#include "player.h"
#include <Novice.h>
#include <algorithm>

// 初期化
void Player::Initialize() {

    x_ = 200.0f;
    y_ = 500.0f;

    velocityY_ = 0.0f;
    isGround_ = false;
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
}