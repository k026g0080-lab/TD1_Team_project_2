#pragma once

#include "player.h" // プレイヤーのビーム情報を受け取るため
#include "map.h"    // マップの当たり判定を受け取るため

class Boss {
public:
    // 初期化
    void Initialize();

    // 更新（マップの当たり判定とプレイヤーのビーム判定を行う）
    void Update(Player& player, const Map& map);

    // 描画
    void Draw();

    // 生存しているか
    bool IsAlive() const { return isAlive_; }

    // バリア展開中か取得
    bool HasBarrier() const { return hasBarrier_; }

private:
    // 矩形とマップ壁の当たり判定・押し戻し（X軸・Y軸）
    bool ResolveMapCollisionX(float& boxX, float& boxY, float boxW, float boxH, float velX, const Map& map);
    bool ResolveMapCollisionY(float& boxX, float& boxY, float boxW, float boxH, float velY, const Map& map);

    // 地殻変動などで静止中・重なり中に壁へめり込んだ場合の押し出し処理
    void ResolveOverlapWithMap(float& boxX, float& boxY, float boxW, float boxH, const Map& map);

    // 内部状態の整合性チェック（NaN/inf・負のHP・不正サイズ防止）
    void SanitizeState();

private:
    // ボスの座標・サイズ
    float x_ = 1500.0f;
    float y_ = 400.0f;
    float width_ = 160.0f;
    float height_ = 160.0f;

    // 移動用
    float speedX_ = 0.0f;
    float speedY_ = 4.0f;

    // ステータス
    int maxHp_ = 1000;
    int hp_ = 1000;
    bool isAlive_ = true;

    // バリア・弱点ギミック用
    bool hasBarrier_ = false;          // バリア展開中フラグ
    bool barrierTriggered_ = false;    // HP5分の3到達済みフラグ
    int barrierTimer_ = 0;             // バリア経過フレームカウンタ

    float weakX_ = 900.0f;             // 弱点のX座標
    float weakY_ = 500.0f;             // 弱点のY座標
    float weakBaseX_ = 900.0f;         // 弱点の移動基準X座標
    float weakBaseY_ = 500.0f;         // 弱点の移動基準Y座標
    float weakWidth_ = 64.0f;          // 弱点の幅
    float weakHeight_ = 64.0f;         // 弱点の高さ
    float weakPrevX_ = 900.0f;         // 前フレームの弱点X座標
    float weakPrevY_ = 500.0f;         // 前フレームの弱点Y座標

    int weakMaxHp_ = 40;               // 弱点の最大耐久値（大きくするほど破壊に時間がかかる）
    int weakHp_ = 40;                  // 弱点の現在耐久値
    int weakHitFlashTimer_ = 0;        // 弱点被弾フラッシュタイマー
    int weakHitCooldown_ = 0;          // 弱点連続ヒット抑制タイマー

    // ダメージ演出（被弾時に一瞬白く光らせる用）
    int damageTimer_ = 0;
};