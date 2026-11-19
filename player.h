#pragma once

#include "map.h"

struct BeamSegment {
    float startX;
    float startY;

    float endX;
    float endY;
};

struct Bullet {

    static const int kMaxReflections = 5;

    // 最初のビーム + 反射後の5本
    BeamSegment segments[kMaxReflections + 1];

    // 実際に使用している線分の数
    int segmentCount;

    // 表示時間
    int timer;

    bool isActive;
};

class Player {
public:

    // 初期化
    void Initialize();

    // 更新
    void Update(const char* keys, const char* preKeys, const Map& map);

    // 描画
    void Draw();

    // 弾の設定
    static const int kMaxBullets = 100;

    Bullet bullets_[kMaxBullets] = {};

    // ビームの最大射程
    float beamRange_ = 2000.0f;

    // ビームの初期の太さ
    float beamWidth_ = 12.0f;

    // ビームの表示時間
    int beamDuration_ = 8;

    // 連射間隔
    int shotInterval_ = 20;

    // 次の発射までの時間
    int shotTimer_ = 0;

    // 最大反射回数
    int maxReflections_ = 5;

private:

    // プレイヤーの座標
    float x_ = 200.0f;
    float y_ = 500.0f;

    // プレイヤーのサイズ
    float width_ = 64.0f;
    float height_ = 64.0f;

    // 移動速度
    float speed_ = 15.0f;

    // ジャンプ・重力
    float velocityY_ = 0.0f;
    float gravity_ = 1.0f;
    float jumpPower_ = -30.0f;

    // 地面にいるか
    bool isGround_ = false;

    int beamTexture_ = -1;

    float aimAngle_ = 0.0f;
    float aimDistance_ = 100.0f; //標準の距離
    float aimSpeed_ = 30.0f;

    int preMouseX_ = 0;
    int preMouseY_ = 0;
    float aimX_ = 960.0f;
    float aimY_ = 540.0f;
    bool isMouseWarped_ = false; 
};