#include "boss.h"
#include <Novice.h>
#include <algorithm>
#include <cmath>

namespace {
    // 太さ(radius)を持つ線分 (x1, y1)-(x2, y2) と 矩形 (rx, ry, rw, rh) の正確な当たり判定（スラブ法）
    bool CheckSegmentToBox(float x1, float y1, float x2, float y2, float radius, float rx, float ry, float rw, float rh) {
        // ビームの太さ（半径）分だけ、ボスの当たり判定矩形を上下左右に広げる
        float minX = rx - radius;
        float maxX = rx + rw + radius;
        float minY = ry - radius;
        float maxY = ry + rh + radius;

        float dx = x2 - x1;
        float dy = y2 - y1;

        float tMin = 0.0f;
        float tMax = 1.0f;

        // X軸の交差範囲を計算
        if (std::abs(dx) < 1e-6f) {
            if (x1 < minX || x1 > maxX) {
                return false;
            }
        }
        else {
            float invDx = 1.0f / dx;
            float t1 = (minX - x1) * invDx;
            float t2 = (maxX - x1) * invDx;
            if (t1 > t2) {
                std::swap(t1, t2);
            }
            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);
            if (tMin > tMax) {
                return false;
            }
        }

        // Y軸の交差範囲を計算
        if (std::abs(dy) < 1e-6f) {
            if (y1 < minY || y1 > maxY) {
                return false;
            }
        }
        else {
            float invDy = 1.0f / dy;
            float t1 = (minY - y1) * invDy;
            float t2 = (maxY - y1) * invDy;
            if (t1 > t2) {
                std::swap(t1, t2);
            }
            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);
            if (tMin > tMax) {
                return false;
            }
        }

        return true;
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
    // ★ビームの見た目の太さの半分を指定（例：太さ20pxなら10.0f）
    const float beamRadius = 10.0f;

    for (int i = 0; i < Player::kMaxBullets; i++) {
        Bullet& bullet = player.bullets_[i];

        // ビームが出ている間は常に当たり判定を行う
        if (!bullet.isActive) {
            continue;
        }

        // 反射したすべての線分をチェック
        for (int j = 0; j < bullet.segmentCount; j++) {
            const BeamSegment& seg = bullet.segments[j];

            if (CheckSegmentToBox(seg.startX, seg.startY, seg.endX, seg.endY, beamRadius, x_, y_, width_, height_)) {
                hp_ -= 1;         // 毎フレーム当たるためダメージ量を調整（必要に応じて変更してください）
                damageTimer_ = 2; // 白く光る時間

                if (hp_ <= 0) {
                    hp_ = 0;
                    isAlive_ = false;
                }
                break; // 1本のビーム（複数反射）で同フレームに多重ヒットしないよう抜ける
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