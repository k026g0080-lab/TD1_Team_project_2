#include "player.h"
#include <Novice.h>
#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <cmath>

// 初期化
void Player::Initialize() {

    x_ = 200.0f;
    y_ = 500.0f;

    velocityY_ = 0.0f;
    isGround_ = false;

    // 照準の初期位置（1920x1080の中央）
    aimX_ = 960.0f;
    aimY_ = 540.0f;
    Novice::GetMousePosition(&preMouseX_, &preMouseY_);
    isMouseWarped_ = false;

    // 弾を初期化
    for (int i = 0; i < kMaxBullets; i++) {
        bullets_[i] = {};
    }

    shotTimer_ = 0;

    beamTexture_ = Novice::LoadTexture("./NoviceResources/white1x1.png");
}

// 更新
void Player::Update(const char* keys, const char* preKeys, const Map& map) {

    while (map.IsSolidWorld(x_ + width_ / 2.0f, y_ + height_ / 2.0f) && y_ > kMapChipSize * 2) {
        y_ -= static_cast<float>(kMapChipSize);
        velocityY_ = 0.0f;
    }

    //入力の取得
    int lStickX = 0;
    int lStickY = 0;
    Novice::GetAnalogInputLeft(0, &lStickX, &lStickY);

    int rStickX = 0;
    int rStickY = 0;
    Novice::GetAnalogInputRight(0, &rStickX, &rStickY);

    int mouseX = 0;
    int mouseY = 0;
    Novice::GetMousePosition(&mouseX, &mouseY);

    // -----------------------------------------
    // 1. プレイヤーの左右移動 & マップ（壁）との当たり判定
    // -----------------------------------------
    float moveX = 0.0f;
    if (keys[DIK_A] || lStickX < -8000 || Novice::IsPressButton(0, kPadButton2)) {
        moveX -= speed_;
    }
    if (keys[DIK_D] || lStickX > 8000 || Novice::IsPressButton(0, kPadButton3)) {
        moveX += speed_;
    }

    x_ += moveX;

    // 画面外に出ないようにする
    x_ = std::clamp(x_, 0.0f, 1920.0f - width_);

    // 上下ギリギリで床・天井に引っかからないよう 1.0f だけ内側にマージンを取る
    float topY = y_ + 1.0f;
    float midY = y_ + height_ / 2.0f;
    float bottomY = y_ + height_ - 1.0f;

    if (moveX > 0.0f) {
        // 右移動中：右辺の 上・中・下 をチェック
        float rightX = x_ + width_;
        if (map.IsSolidWorld(rightX, topY) ||
            map.IsSolidWorld(rightX, midY) ||
            map.IsSolidWorld(rightX, bottomY)) {

            int mapX = static_cast<int>(std::floor(rightX / kMapChipSize));
            x_ = static_cast<float>(mapX * kMapChipSize) - width_;
        }
    }
    else if (moveX < 0.0f) {
        // 左移動中：左辺の 上・中・下 をチェック
        float leftX = x_;
        if (map.IsSolidWorld(leftX, topY) ||
            map.IsSolidWorld(leftX, midY) ||
            map.IsSolidWorld(leftX, bottomY)) {

            int mapX = static_cast<int>(std::floor(leftX / kMapChipSize));
            x_ = static_cast<float>((mapX + 1) * kMapChipSize);
        }
    }

    // ジャンプ
    if (((keys[DIK_SPACE] && !preKeys[DIK_SPACE]) || Novice::IsTriggerButton(0, kPadButton10)) && isGround_) {
        velocityY_ = jumpPower_;
        isGround_ = false;
    }

    // -----------------------------------------
    // 2. 重力 & マップ（床・天井）との当たり判定
    // -----------------------------------------
    velocityY_ += gravity_;
    y_ += velocityY_;

    float leftX = x_ + 1.0f;
    float midX = x_ + width_ / 2.0f;
    float rightX = x_ + width_ - 1.0f;

    if (velocityY_ >= 0.0f) {
        // 落下中・接地中：下辺の 左・中・右 をチェック
        float footY = y_ + height_;
        if (map.IsSolidWorld(leftX, footY) ||
            map.IsSolidWorld(midX, footY) ||
            map.IsSolidWorld(rightX, footY)) {

            // 着地した地面の上端を取得
            int mapY = static_cast<int>(std::floor(footY / kMapChipSize));
            y_ = static_cast<float>(mapY * kMapChipSize) - height_;

            velocityY_ = 0.0f;
            isGround_ = true;
        }
        else {
            isGround_ = false;
        }
    }
    else {
        // ジャンプ上昇中：上辺の 左・中・右 をチェック（天井判定）
        float headY = y_;
        if (map.IsSolidWorld(leftX, headY) ||
            map.IsSolidWorld(midX, headY) ||
            map.IsSolidWorld(rightX, headY)) {

            // 当たった天井の下端に合わせる
            int mapY = static_cast<int>(std::floor(headY / kMapChipSize));
            y_ = static_cast<float>((mapY + 1) * kMapChipSize);

            velocityY_ = 0.0f;
        }
    }

    // 画面上端
    if (y_ < 0.0f) {
        y_ = 0.0f;
        velocityY_ = 0.0f;
    }

    // -----------------------------------------
    // 3. 照準の操作
    // -----------------------------------------
    if (isMouseWarped_) {
        // カーソルを中央へ戻した直後のフレームは、新しい中央座標を基準として記憶するだけにする
        preMouseX_ = mouseX;
        preMouseY_ = mouseY;
        isMouseWarped_ = false;
    }
    else {
        // 前フレームからの移動量だけを照準に足す
        int deltaMouseX = mouseX - preMouseX_;
        int deltaMouseY = mouseY - preMouseY_;

        if (deltaMouseX != 0 || deltaMouseY != 0) {
            aimX_ += static_cast<float>(deltaMouseX);
            aimY_ += static_cast<float>(deltaMouseY);
        }

        preMouseX_ = mouseX;
        preMouseY_ = mouseY;

        // 見えないカーソルが画面端（端から150px以内）に近づいたら中央へ戻す
        if (mouseX < 150 || mouseX > 1770 || mouseY < 150 || mouseY > 930) {
            HWND hwnd = GetActiveWindow();
            if (hwnd != nullptr) {
                POINT pt = { 960, 540 };
                ClientToScreen(hwnd, &pt);
                SetCursorPos(pt.x, pt.y);
                isMouseWarped_ = true; // 次の1フレームはワープ後の座標合わせに使う
            }
        }
    }

    // 右スティックの傾きで照準を移動（デッドゾーン: 10000）
    if (std::abs(rStickX) > 10000) {
        aimX_ += (rStickX / 32767.0f) * aimSpeed_;
    }
    if (std::abs(rStickY) > 10000) {
        aimY_ += (rStickY / 32767.0f) * aimSpeed_;
    }

    // 照準を画面内に制限（1920x1080）
    aimX_ = std::clamp(aimX_, 0.0f, 1920.0f);
    aimY_ = std::clamp(aimY_, 0.0f, 1080.0f);

    // -----------------------------------------
    // 4. 射撃
    // -----------------------------------------
    if (shotTimer_ > 0) {
        shotTimer_--;
    }

    if ((Novice::IsPressMouse(0) || Novice::IsPressButton(0, kPadButton9)) && shotTimer_ <= 0) {

        float centerX = x_ + width_ / 2.0f;
        float centerY = y_ + height_ / 2.0f;

        // 緑の照準（aimX_, aimY_）の方向へ撃つ
        float directionX = aimX_ - centerX;
        float directionY = aimY_ - centerY;

        float length = sqrtf(
            directionX * directionX +
            directionY * directionY
        );

        if (length > 0.0f) {

            // 正規化
            directionX /= length;
            directionY /= length;

            for (int i = 0; i < kMaxBullets; i++) {

                if (bullets_[i].isActive) {
                    continue;
                }

                Bullet& bullet = bullets_[i];

                bullet.segmentCount = 0;
                bullet.timer = beamDuration_;
                bullet.isActive = true;

                float currentX = centerX;
                float currentY = centerY;

                float remainingRange = beamRange_;
                int reflectionCount = 0;

                // 最大反射回数までループ
                while (bullet.segmentCount < Bullet::kMaxReflections + 1) {

                    BeamSegment& segment = bullet.segments[bullet.segmentCount];

                    segment.startX = currentX;
                    segment.startY = currentY;
                    segment.endX = currentX;
                    segment.endY = currentY;

                    bool hitWall = false;
                    bool hitScreenEdge = false;
                    float traveled = 0.0f;

                    // ビームの進行方向を調べる
                    while (traveled < remainingRange) {

                        float step = 2.0f;
                        if (step > remainingRange - traveled) {
                            step = remainingRange - traveled;
                        }

                        float nextX = currentX + directionX * step;
                        float nextY = currentY + directionY * step;

                        // 画面外なら終了
                        if (nextX < 0.0f || nextX >= 1920.0f ||
                            nextY < 0.0f || nextY >= 1080.0f) {
                            hitScreenEdge = true;
                            break;
                        }

                        // 壁との当たり判定
                        if (map.IsSolidWorld(nextX, nextY)) {
                            hitWall = true;
                            break;
                        }

                        currentX = nextX;
                        currentY = nextY;
                        traveled += step;

                        segment.endX = currentX;
                        segment.endY = currentY;
                    }

                    bullet.segmentCount++;

                    // 反射するたびに射程をリセット
                    remainingRange = beamRange_;

                    // 画面外、または射程終了
                    if (hitScreenEdge || !hitWall) {
                        break;
                    }

                    // 最大反射回数に達したら終了
                    if (reflectionCount >= maxReflections_) {
                        break;
                    }

                    // X方向とY方向の衝突を確認
                    bool hitX = map.IsSolidWorld(currentX + directionX * 2.0f, currentY);
                    bool hitY = map.IsSolidWorld(currentX, currentY + directionY * 2.0f);

                    // 左右の壁
                    if (hitX) {
                        directionX *= -1.0f;
                    }

                    // 床・天井
                    if (hitY) {
                        directionY *= -1.0f;
                    }

                    // 角に当たった場合
                    if (!hitX && !hitY) {
                        directionX *= -1.0f;
                        directionY *= -1.0f;
                    }

                    reflectionCount++;
                }

                shotTimer_ = shotInterval_;
                break;
            }
        }
    }

    // -----------------------------------------
    // 5. ビームの更新
    // -----------------------------------------
    for (int i = 0; i < kMaxBullets; i++) {

        if (!bullets_[i].isActive) {
            continue;
        }

        // 表示時間を減らす
        bullets_[i].timer--;

        // 時間切れで消す
        if (bullets_[i].timer <= 0) {
            bullets_[i].isActive = false;
        }
    }
}

// 描画
void Player::Draw() {

    // プレイヤー本体
    Novice::DrawBox(
        static_cast<int>(x_),
        static_cast<int>(y_),
        static_cast<int>(width_),
        static_cast<int>(height_),
        0.0f,
        RED,
        kFillModeSolid
    );

    // 照準
    Novice::DrawEllipse(
        static_cast<int>(aimX_),
        static_cast<int>(aimY_),
        10, 10, 0.0f, GREEN, kFillModeSolid
    );

    // ビームの描画
    for (int i = 0; i < kMaxBullets; i++) {

        if (!bullets_[i].isActive) {
            continue;
        }

        Bullet& bullet = bullets_[i];

        // 残り時間の割合
        float t = static_cast<float>(bullet.timer) / beamDuration_;

        // 徐々に細くなる
        float width = beamWidth_ * t;

        // 反射した線分をすべて描画
        for (int j = 0; j < bullet.segmentCount; j++) {

            BeamSegment& segment = bullet.segments[j];

            float dx = segment.endX - segment.startX;
            float dy = segment.endY - segment.startY;
            float length = sqrtf(dx * dx + dy * dy);

            if (length <= 0.0f) {
                continue;
            }

            // 垂直方向
            float normalX = -dy / length;
            float normalY = dx / length;
            float halfWidth = width / 2.0f;

            int leftTopX = static_cast<int>(segment.startX + normalX * halfWidth);
            int leftTopY = static_cast<int>(segment.startY + normalY * halfWidth);
            int rightTopX = static_cast<int>(segment.endX + normalX * halfWidth);
            int rightTopY = static_cast<int>(segment.endY + normalY * halfWidth);
            int leftBottomX = static_cast<int>(segment.startX - normalX * halfWidth);
            int leftBottomY = static_cast<int>(segment.startY - normalY * halfWidth);
            int rightBottomX = static_cast<int>(segment.endX - normalX * halfWidth);
            int rightBottomY = static_cast<int>(segment.endY - normalY * halfWidth);

            Novice::DrawQuad(
                leftTopX, leftTopY,
                rightTopX, rightTopY,
                leftBottomX, leftBottomY,
                rightBottomX, rightBottomY,
                0, 0,
                1, 1,
                beamTexture_,
                RED
            );
        }
    }
}