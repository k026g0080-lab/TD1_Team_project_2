#include "boss.h"
#include <Novice.h>
#include <algorithm>
#include <cmath>

namespace {
    // 線分 (x1, y1)-(x2, y2) と 矩形 (rx, ry, rw, rh) の簡易当たり判定
    bool CheckSegmentToBox(float x1, float y1, float x2, float y2, float rx, float ry, float rw, float rh) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float length = std::sqrt(dx * dx + dy * dy);
        if (length <= 0.0f) return false;

        // 線分上を細かくチェック（ステップ幅 8.0f）
        int steps = static_cast<int>(length / 8.0f) + 1;
        for (int i = 0; i <= steps; i++) {
            float t = static_cast<float>(i) / steps;
            float px = x1 + dx * t;
            float py = y1 + dy * t;

            if (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh) {
                return true;
            }
        }
        return false;
    }
}

// 初期化
void Boss::Initialize() {
    x_ = 1500.0f;
    y_ = 400.0f;
    hp_ = maxHp_;
    isAlive_ = true;
    damageTimer_ = 0;
}

// 更新
void Boss::Update(Player& player) {
    if (!isAlive_) {
        return;
    }

    // 1. 上下に浮遊移動（画面端で折り返し）
    y_ += speedY_;
    if (y_ < 100.0f || y_ + height_ > 900.0f) {
        speedY_ *= -1.0f;
    }

    // 被弾フラッシュタイマー減少
    if (damageTimer_ > 0) {
        damageTimer_--;
    }

    // 2. プレイヤーのビームとの当たり判定
    for (int i = 0; i < Player::kMaxBullets; i++) {
        Bullet& bullet = player.bullets_[i];

        // ビームが出た最初の1フレーム（timer == beamDuration_ - 1）だけダメージを与える
        if (!bullet.isActive || bullet.timer != player.beamDuration_ - 1) {
            continue;
        }

        // 反射したすべての線分をチェック
        for (int j = 0; j < bullet.segmentCount; j++) {
            const BeamSegment& seg = bullet.segments[j];

            if (CheckSegmentToBox(seg.startX, seg.startY, seg.endX, seg.endY, x_, y_, width_, height_)) {
                hp_ -= 5; // 1ヒットあたりのダメージ
                damageTimer_ = 5; // 5フレーム白く光る

                if (hp_ <= 0) {
                    hp_ = 0;
                    isAlive_ = false;
                }
                break; // 1本のビームで多重ヒットしないよう抜ける
            }
        }
    }
}

// 描画
void Boss::Draw() {
    if (!isAlive_) {
        return;
    }

    // ボス本体（被弾中は白、通常は紫）
    unsigned int color = (damageTimer_ > 0) ? WHITE : 0x8800FFFF;
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

    // 背景（黒）
    Novice::DrawBox(barX - 4, barY - 4, barMaxWidth + 8, barHeight + 8, 0.0f, BLACK, kFillModeSolid);
    // 現在のHP（赤）
    int currentBarWidth = static_cast<int>(barMaxWidth * (static_cast<float>(hp_) / maxHp_));
    Novice::DrawBox(barX, barY, currentBarWidth, barHeight, 0.0f, RED, kFillModeSolid);
}