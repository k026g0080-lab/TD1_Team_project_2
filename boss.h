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
    // ボスの行動パターン（ステート）
    enum class State {
        Normal,   // 通常移動
        Charge,   // チャージ中（予兆）
        Attack,   // ビーム発射中
        Cooldown  // 攻撃後の硬直
    };

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

    // チャージ攻撃用 
    State state_ = State::Normal; // 現在の状態
    int stateTimer_ = 0;          // 状態ごとの経過時間フレーム
    float beamHeight_ = 100.0f;   // ボスビームの太さ（高さ）
};