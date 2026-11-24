#pragma once

#include "player.h" // プレイヤーのビーム情報を受け取るため

class Boss {
public:
    // 初期化
    void Initialize();

    // 更新（プレイヤーのビームとの当たり判定もここで行う）
    void Update(Player& player);

    // 描画
    void Draw();

    // 生存しているか
    bool IsAlive() const { return isAlive_; }

private:
    // ボスの座標・サイズ
    float x_ = 1500.0f;
    float y_ = 400.0f;
    float width_ = 160.0f;
    float height_ = 160.0f;

    // 移動用
    float speedY_ = 4.0f;

    // ステータス
    int maxHp_ = 1000;
    int hp_ = 1000;
    bool isAlive_ = true;

    // ダメージ演出（被弾時に一瞬白く光らせる用）
    int damageTimer_ = 0;
};