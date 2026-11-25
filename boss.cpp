#include "boss.h"
#include <Novice.h>
#include <algorithm>
#include <cmath>

namespace {
    // 太さ(radius)を持つ線分 (x1, y1)-(x2, y2) と 矩形 (rx, ry, rw, rh) の正確な当たり判定（スラブ法）
    bool CheckSegmentToBox(float x1, float y1, float x2, float y2, float radius, float rx, float ry, float rw, float rh) {
        float minX = rx - radius;
        float maxX = rx + rw + radius;
        float minY = ry - radius;
        float maxY = ry + rh + radius;

        float dx = x2 - x1;
        float dy = y2 - y1;

        float tMin = 0.0f;
        float tMax = 1.0f;

        if (std::abs(dx) < 1e-6f) {
            if (x1 < minX || x1 > maxX) return false;
        } else {
            float invDx = 1.0f / dx;
            float t1 = (minX - x1) * invDx;
            float t2 = (maxX - x1) * invDx;
            if (t1 > t2) std::swap(t1, t2);
            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);
            if (tMin > tMax) return false;
        }

        if (std::abs(dy) < 1e-6f) {
            if (y1 < minY || y1 > maxY) return false;
        } else {
            float invDy = 1.0f / dy;
            float t1 = (minY - y1) * invDy;
            float t2 = (maxY - y1) * invDy;
            if (t1 > t2) std::swap(t1, t2);
            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);
            if (tMin > tMax) return false;
        }

        return true;
    }

    // 矩形同士の当たり判定（ボスビームとプレイヤー用）
    bool CheckBoxToBox(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
        return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
    }
}

// 初期化
void Boss::Initialize() {
    x_ = 1500.0f;
    y_ = 400.0f;
    hp_ = maxHp_;
    isAlive_ = true;
    damageTimer_ = 0;

    // ステートの初期化
    state_ = State::Normal;
    stateTimer_ = 180; // 最初の攻撃まで3秒（60fps想定）
}

// 更新
void Boss::Update(Player& player) {
    if (!isAlive_) {
        return;
    }

    // 1. 状態（ステート）ごとの行動処理
    stateTimer_--;

    switch (state_) {
    case State::Normal:
        // 上下に浮遊移動（画面端で折り返し）
        y_ += speedY_;
        if (y_ < 100.0f || y_ + height_ > 900.0f) {
            speedY_ *= -1.0f;
        }

        // 時間が来たらチャージ開始
        if (stateTimer_ <= 0) {
            state_ = State::Charge;
            stateTimer_ = 90; // 1.5秒チャージする
        }
        break;

    case State::Charge:
        // チャージ中はその場に停止して狙いを定める
        if (stateTimer_ <= 0) {
            state_ = State::Attack;
            stateTimer_ = 60; // 1.0秒間ビームを撃ち続ける
        }
        break;

    case State::Attack:
        // ボスビームとプレイヤーの当たり判定
    {
        float beamX = 0.0f;
        float beamW = x_;
        float beamY = (y_ + height_ / 2.0f) - (beamHeight_ / 2.0f);
        float beamH = beamHeight_;

        if (CheckBoxToBox(
            beamX, beamY, beamW, beamH,
            player.GetX(), player.GetY(), player.GetWidth(), player.GetHeight())) {

           
        }
    }

    if (stateTimer_ <= 0) {
        state_ = State::Cooldown;
        stateTimer_ = 60; // 1.0秒の硬直
    }
    break;

    case State::Cooldown:
        // 攻撃後の隙
        if (stateTimer_ <= 0) {
            state_ = State::Normal;
            stateTimer_ = 180; // 再び通常移動へ（3秒後にまた攻撃）
        }
        break;
    }

    // 被弾フラッシュタイマー減少
    if (damageTimer_ > 0) {
        damageTimer_--;
    }

    // 2. プレイヤーのビームとの当たり判定
    const float beamRadius = 10.0f;

    for (int i = 0; i < Player::kMaxBullets; i++) {
        Bullet& bullet = player.bullets_[i];

        if (!bullet.isActive) {
            continue;
        }

        for (int j = 0; j < bullet.segmentCount; j++) {
            const BeamSegment& seg = bullet.segments[j];

            if (CheckSegmentToBox(seg.startX, seg.startY, seg.endX, seg.endY, beamRadius, x_, y_, width_, height_)) {
                hp_ -= 1;
                damageTimer_ = 2;

                if (hp_ <= 0) {
                    hp_ = 0;
                    isAlive_ = false;
                }
                break;
            }
        }
    }
}

// 描画
void Boss::Draw() {
    if (!isAlive_) {
        return;
    }

    float centerY = y_ + height_ / 2.0f;

    // チャージ中（予兆）の描画 
    if (state_ == State::Charge) {
        // 1. 攻撃範囲の予測線（半透明の赤）
        int warnY = static_cast<int>(centerY - beamHeight_ / 2.0f);
        Novice::DrawBox(0, warnY, static_cast<int>(x_), static_cast<int>(beamHeight_), 0.0f, 0xFF000044, kFillModeSolid);

        // 2. 発射口に集まるエネルギー球（残り時間が少ないほど大きくなる）
        float progress = 1.0f - (static_cast<float>(stateTimer_) / 90.0f);
        int chargeRadius = static_cast<int>(60.0f * progress);
        Novice::DrawEllipse(static_cast<int>(x_), static_cast<int>(centerY), chargeRadius, chargeRadius, 0.0f, 0xFFFF00FF, kFillModeSolid);
    }

    // ビーム発射中の描画
    if (state_ == State::Attack) {
        int beamY = static_cast<int>(centerY - beamHeight_ / 2.0f);
        int beamH = static_cast<int>(beamHeight_);
        // 外側の赤いオーラ
        Novice::DrawBox(0, beamY, static_cast<int>(x_), beamH, 0.0f, 0xFF2222FF, kFillModeSolid);
        // 中心の白い高エネルギー芯
        Novice::DrawBox(0, beamY + beamH / 4, static_cast<int>(x_), beamH / 2, 0.0f, WHITE, kFillModeSolid);
    }

    // ボス本体の描画 
    unsigned int color = 0x8800FFFF; // 通常：紫
    if (damageTimer_ > 0) {
        color = WHITE;               // 被弾：白
    } else if (state_ == State::Charge) {
        // チャージ中は黄色く点滅させる
        color = (stateTimer_ % 6 < 3) ? 0xFFFF00FF : 0xFF8800FF;
    }

    Novice::DrawBox(
        static_cast<int>(x_),
        static_cast<int>(y_),
        static_cast<int>(width_),
        static_cast<int>(height_),
        0.0f,
        color,
        kFillModeSolid
    );

    // HPバーの描画（画面上部中央）
    int barMaxWidth = 800;
    int barHeight = 24;
    int barX = (1920 - barMaxWidth) / 2;
    int barY = 40;

    Novice::DrawBox(barX - 4, barY - 4, barMaxWidth + 8, barHeight + 8, 0.0f, BLACK, kFillModeSolid);
    int currentBarWidth = static_cast<int>(barMaxWidth * (static_cast<float>(hp_) / maxHp_));
    Novice::DrawBox(barX, barY, currentBarWidth, barHeight, 0.0f, RED, kFillModeSolid);
}