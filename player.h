#pragma once

#include "map.h"

class Player {
public:

    // 初期化
    void Initialize();

    // 更新
    void Update(const char* keys, const char* preKeys, const Map& map);

    // 描画
    void Draw();

private:

    // プレイヤーの座標
    float x_ = 200.0f;
    float y_ = 500.0f;

    // プレイヤーのサイズ
    float width_ = 64.0f;
    float height_ = 64.0f;

    // 移動速度
    float speed_ = 6.0f;

    // ジャンプ・重力
    float velocityY_ = 0.0f;
    float gravity_ = 0.8f;
    float jumpPower_ = -18.0f;

    // 地面にいるか
    bool isGround_ = false;
};