#include "player.h"
#include <Novice.h>
#include <algorithm>
#include <cmath>

// 初期化
void Player::Initialize() {

    x_ = 200.0f;
    y_ = 500.0f;

    velocityY_ = 0.0f;
    isGround_ = false;

    // 弾を初期化
    for (int i = 0; i < kMaxBullets; i++) {
        bullets_[i] = {};
    }

    shotTimer_ = 0;

    beamTexture_ = Novice::LoadTexture("./NoviceResources/white1x1.png");
}

// 更新
void Player::Update(const char* keys, const char* preKeys, const Map& map) {

    // 左移動
    if (keys[DIK_A]) {
        x_ -= speed_;
    }

    // 右移動
    if (keys[DIK_D]) {
        x_ += speed_;
    }

    // 画面外に出ないようにする
    x_ = std::clamp(x_, 0.0f, 1920.0f - width_);

    // ジャンプ
    if (keys[DIK_W] && !preKeys[DIK_W] && isGround_) {

        velocityY_ = jumpPower_;
        isGround_ = false;
    }

    // 射撃クールタイム
    if (shotTimer_ > 0) {
        shotTimer_--;
    }

    // マウス座標
    int mouseX = 0;
    int mouseY = 0;

    Novice::GetMousePosition(&mouseX, &mouseY);

    // 左クリック長押しで射撃
    if (Novice::IsPressMouse(0) && shotTimer_ <= 0) {

        float centerX = x_ + width_ / 2.0f;
        float centerY = y_ + height_ / 2.0f;

        float directionX = mouseX - centerX;
        float directionY = mouseY - centerY;

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

                // 最大5回反射
                while (bullet.segmentCount < Bullet::kMaxReflections + 1) {

                    BeamSegment& segment =
                        bullet.segments[bullet.segmentCount];

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
                    bool hitX = map.IsSolidWorld(
                        currentX + directionX * 2.0f,
                        currentY
                    );

                    bool hitY = map.IsSolidWorld(
                        currentX,
                        currentY + directionY * 2.0f
                    );

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

    // ビームの更新
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

    // 重力
    velocityY_ += gravity_;

    // 縦移動
    y_ += velocityY_;

    // 地面との当たり判定
    if (velocityY_ >= 0.0f) {

        float leftX = x_ + 1.0f;
        float rightX = x_ + width_ - 1.0f;
        float bottomY = y_ + height_;

        if (map.IsSolidWorld(leftX, bottomY) ||
            map.IsSolidWorld(rightX, bottomY)) {

            // 着地した地面の上端を取得
            int mapY = static_cast<int>(bottomY) / kMapChipSize;

            y_ = static_cast<float>(mapY * kMapChipSize) - height_;

            velocityY_ = 0.0f;
            isGround_ = true;

        }
        else {

            isGround_ = false;
        }
    }

    // 画面上端
    if (y_ < 0.0f) {
        y_ = 0.0f;
        velocityY_ = 0.0f;
    }
}

// 描画
void Player::Draw() {

    Novice::DrawBox(
        static_cast<int>(x_),
        static_cast<int>(y_),
        static_cast<int>(width_),
        static_cast<int>(height_),
        0.0f,
        RED,
        kFillModeSolid
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

            int leftTopX = static_cast<int>(
                segment.startX + normalX * halfWidth
                );

            int leftTopY = static_cast<int>(
                segment.startY + normalY * halfWidth
                );

            int rightTopX = static_cast<int>(
                segment.endX + normalX * halfWidth
                );

            int rightTopY = static_cast<int>(
                segment.endY + normalY * halfWidth
                );

            int leftBottomX = static_cast<int>(
                segment.startX - normalX * halfWidth
                );

            int leftBottomY = static_cast<int>(
                segment.startY - normalY * halfWidth
                );

            int rightBottomX = static_cast<int>(
                segment.endX - normalX * halfWidth
                );

            int rightBottomY = static_cast<int>(
                segment.endY - normalY * halfWidth
                );

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