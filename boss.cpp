#include "boss.h"
#include <Novice.h>
#include <algorithm>
#include <cmath>

namespace {
    // 有限な浮動小数点数かどうかを確認
    bool IsValidFloat(float v) {
        return std::isfinite(v) != 0;
    }

    // マップ定数が有効か検証
    bool IsValidMapConfig() {
        return (kMapChipSize > 0) && (kMapWidth > 0) && (kMapHeight > 0);
    }

    // 太さ(radius)を持つ線分 (x1, y1)-(x2, y2) と 矩形 (rx, ry, rw, rh) の正確な当たり判定（スラブ法）
    bool CheckSegmentToBox(float x1, float y1, float x2, float y2, float radius, float rx, float ry, float rw, float rh) {
        if (!IsValidFloat(x1) || !IsValidFloat(y1) || !IsValidFloat(x2) || !IsValidFloat(y2) ||
            !IsValidFloat(radius) || !IsValidFloat(rx) || !IsValidFloat(ry) || !IsValidFloat(rw) || !IsValidFloat(rh)) {
            return false;
        }
        if (radius < 0.0f || rw <= 0.0f || rh <= 0.0f) {
            return false;
        }

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
    width_ = 160.0f;
    height_ = 160.0f;

    speedX_ = 0.0f;
    speedY_ = 4.0f;

    maxHp_ = (maxHp_ > 0) ? maxHp_ : 1000;
    hp_ = maxHp_;
    isAlive_ = true;
    damageTimer_ = 0;

    hasBarrier_ = false;
    barrierTriggered_ = false;
    barrierTimer_ = 0;

    weakBaseX_ = 900.0f;
    weakBaseY_ = 500.0f;
    weakX_ = weakBaseX_;
    weakY_ = weakBaseY_;
    weakPrevX_ = weakX_;
    weakPrevY_ = weakY_;
    weakWidth_ = 64.0f;
    weakHeight_ = 64.0f;

    weakMaxHp_ = (weakMaxHp_ > 0) ? weakMaxHp_ : 40;
    weakHp_ = weakMaxHp_;
    weakHitFlashTimer_ = 0;
    weakHitCooldown_ = 0;
}

// 内部状態の整合性チェック
void Boss::SanitizeState() {
    if (!IsValidFloat(x_)) x_ = 1500.0f;
    if (!IsValidFloat(y_)) y_ = 400.0f;
    if (!IsValidFloat(width_) || width_ <= 0.0f) width_ = 160.0f;
    if (!IsValidFloat(height_) || height_ <= 0.0f) height_ = 160.0f;

    if (!IsValidFloat(speedX_)) speedX_ = 0.0f;
    if (!IsValidFloat(speedY_)) speedY_ = 4.0f;

    if (!IsValidFloat(weakX_)) weakX_ = 900.0f;
    if (!IsValidFloat(weakY_)) weakY_ = 500.0f;
    if (!IsValidFloat(weakBaseX_)) weakBaseX_ = 900.0f;
    if (!IsValidFloat(weakBaseY_)) weakBaseY_ = 500.0f;
    if (!IsValidFloat(weakWidth_) || weakWidth_ <= 0.0f) weakWidth_ = 64.0f;
    if (!IsValidFloat(weakHeight_) || weakHeight_ <= 0.0f) weakHeight_ = 64.0f;
    if (!IsValidFloat(weakPrevX_)) weakPrevX_ = weakX_;
    if (!IsValidFloat(weakPrevY_)) weakPrevY_ = weakY_;

    if (maxHp_ <= 0) maxHp_ = 1;
    hp_ = (std::max)(0, (std::min)(hp_, maxHp_));

    if (weakMaxHp_ <= 0) weakMaxHp_ = 1;
    weakHp_ = (std::max)(0, (std::min)(weakHp_, weakMaxHp_));

    if (hp_ <= 0) {
        isAlive_ = false;
    }
    if (barrierTimer_ < 0) barrierTimer_ = 0;
    if (damageTimer_ < 0) damageTimer_ = 0;
    if (weakHitFlashTimer_ < 0) weakHitFlashTimer_ = 0;
    if (weakHitCooldown_ < 0) weakHitCooldown_ = 0;
}

// X軸方向のマップ当たり判定と押し戻し
bool Boss::ResolveMapCollisionX(float& boxX, float& boxY, float boxW, float boxH, float velX, const Map& map) {
    if (!IsValidMapConfig()) {
        return false;
    }
    if (!IsValidFloat(boxX) || !IsValidFloat(boxY) || !IsValidFloat(boxW) || !IsValidFloat(boxH) || !IsValidFloat(velX)) {
        return false;
    }
    if (boxW <= 0.0f || boxH <= 0.0f || velX == 0.0f) {
        return false;
    }

    bool hit = false;
    const float fTile = static_cast<float>(kMapChipSize);
    const float eps = 0.001f;

    int leftCol = static_cast<int>(std::floor(boxX / fTile));
    int rightCol = static_cast<int>(std::floor((boxX + boxW - eps) / fTile));
    int topRow = static_cast<int>(std::floor(boxY / fTile));
    int bottomRow = static_cast<int>(std::floor((boxY + boxH - eps) / fTile));

    leftCol = (std::max)(0, leftCol);
    rightCol = (std::min)(kMapWidth - 1, rightCol);
    topRow = (std::max)(0, topRow);
    bottomRow = (std::min)(kMapHeight - 1, bottomRow);

    if (leftCol > rightCol || topRow > bottomRow) {
        return false;
    }

    for (int r = topRow; r <= bottomRow; ++r) {
        for (int c = leftCol; c <= rightCol; ++c) {
            if (map.IsSolid(c, r)) {
                float tileLeft = static_cast<float>(c) * fTile;
                float tileRight = static_cast<float>(c + 1) * fTile;

                if (velX > 0.0f) {
                    float candidateX = tileLeft - boxW;
                    if (candidateX < boxX) {
                        boxX = candidateX;
                    }
                    hit = true;
                }
                else if (velX < 0.0f) {
                    float candidateX = tileRight;
                    if (candidateX > boxX) {
                        boxX = candidateX;
                    }
                    hit = true;
                }
            }
        }
    }

    return hit;
}

// Y軸方向のマップ当たり判定と押し戻し
bool Boss::ResolveMapCollisionY(float& boxX, float& boxY, float boxW, float boxH, float velY, const Map& map) {
    if (!IsValidMapConfig()) {
        return false;
    }
    if (!IsValidFloat(boxX) || !IsValidFloat(boxY) || !IsValidFloat(boxW) || !IsValidFloat(boxH) || !IsValidFloat(velY)) {
        return false;
    }
    if (boxW <= 0.0f || boxH <= 0.0f || velY == 0.0f) {
        return false;
    }

    bool hit = false;
    const float fTile = static_cast<float>(kMapChipSize);
    const float eps = 0.001f;

    int leftCol = static_cast<int>(std::floor(boxX / fTile));
    int rightCol = static_cast<int>(std::floor((boxX + boxW - eps) / fTile));
    int topRow = static_cast<int>(std::floor(boxY / fTile));
    int bottomRow = static_cast<int>(std::floor((boxY + boxH - eps) / fTile));

    leftCol = (std::max)(0, leftCol);
    rightCol = (std::min)(kMapWidth - 1, rightCol);
    topRow = (std::max)(0, topRow);
    bottomRow = (std::min)(kMapHeight - 1, bottomRow);

    if (leftCol > rightCol || topRow > bottomRow) {
        return false;
    }

    for (int r = topRow; r <= bottomRow; ++r) {
        for (int c = leftCol; c <= rightCol; ++c) {
            if (map.IsSolid(c, r)) {
                float tileTop = static_cast<float>(r) * fTile;
                float tileBottom = static_cast<float>(r + 1) * fTile;

                if (velY > 0.0f) {
                    float candidateY = tileTop - boxH;
                    if (candidateY < boxY) {
                        boxY = candidateY;
                    }
                    hit = true;
                }
                else if (velY < 0.0f) {
                    float candidateY = tileBottom;
                    if (candidateY > boxY) {
                        boxY = candidateY;
                    }
                    hit = true;
                }
            }
        }
    }

    return hit;
}

// 地殻変動などで静止中にブロックが重なった場合の最小めり込み押し出し
void Boss::ResolveOverlapWithMap(float& boxX, float& boxY, float boxW, float boxH, const Map& map) {
    if (!IsValidMapConfig()) {
        return;
    }
    if (!IsValidFloat(boxX) || !IsValidFloat(boxY) || !IsValidFloat(boxW) || !IsValidFloat(boxH)) {
        return;
    }
    if (boxW <= 0.0f || boxH <= 0.0f) {
        return;
    }

    const float fTile = static_cast<float>(kMapChipSize);
    const float eps = 0.001f;

    int leftCol = static_cast<int>(std::floor(boxX / fTile));
    int rightCol = static_cast<int>(std::floor((boxX + boxW - eps) / fTile));
    int topRow = static_cast<int>(std::floor(boxY / fTile));
    int bottomRow = static_cast<int>(std::floor((boxY + boxH - eps) / fTile));

    leftCol = (std::max)(0, leftCol);
    rightCol = (std::min)(kMapWidth - 1, rightCol);
    topRow = (std::max)(0, topRow);
    bottomRow = (std::min)(kMapHeight - 1, bottomRow);

    for (int r = topRow; r <= bottomRow; ++r) {
        for (int c = leftCol; c <= rightCol; ++c) {
            if (map.IsSolid(c, r)) {
                float tileLeft = static_cast<float>(c) * fTile;
                float tileRight = static_cast<float>(c + 1) * fTile;
                float tileTop = static_cast<float>(r) * fTile;
                float tileBottom = static_cast<float>(r + 1) * fTile;

                float overlapLeft = (boxX + boxW) - tileLeft;
                float overlapRight = tileRight - boxX;
                float overlapTop = (boxY + boxH) - tileTop;
                float overlapBottom = tileBottom - boxY;

                if (overlapLeft <= 0.0f || overlapRight <= 0.0f || overlapTop <= 0.0f || overlapBottom <= 0.0f) {
                    continue;
                }

                float minOverlap = overlapLeft;
                int dir = 0; // 0:左へ, 1:右へ, 2:上へ, 3:下へ

                if (overlapRight < minOverlap) {
                    minOverlap = overlapRight;
                    dir = 1;
                }
                if (overlapTop < minOverlap) {
                    minOverlap = overlapTop;
                    dir = 2;
                }
                if (overlapBottom < minOverlap) {
                    minOverlap = overlapBottom;
                    dir = 3;
                }

                if (dir == 0) boxX -= overlapLeft;
                else if (dir == 1) boxX += overlapRight;
                else if (dir == 2) boxY -= overlapTop;
                else if (dir == 3) boxY += overlapBottom;
            }
        }
    }
}

// 更新
void Boss::Update(Player& player, const Map& map) {
    SanitizeState();

    if (!isAlive_) {
        return;
    }

    const bool hasValidMap = IsValidMapConfig();

    // 1. ボス本体の移動とマップ当たり判定（X軸・Y軸）
    if (speedX_ != 0.0f) {
        x_ += speedX_;
        if (hasValidMap && ResolveMapCollisionX(x_, y_, width_, height_, speedX_, map)) {
            speedX_ *= -1.0f;
        }
    }

    y_ += speedY_;
    if (hasValidMap && ResolveMapCollisionY(x_, y_, width_, height_, speedY_, map)) {
        speedY_ *= -1.0f;
    }

    // 地殻変動などで重なった場合の押し出し
    if (hasValidMap) {
        ResolveOverlapWithMap(x_, y_, width_, height_, map);
    }

    // 画面端での折り返し
    if (y_ < 100.0f) {
        y_ = 100.0f;
        speedY_ = std::abs(speedY_);
    }
    else if (y_ + height_ > 900.0f) {
        y_ = 900.0f - height_;
        speedY_ = -std::abs(speedY_);
    }

    // 各種タイマー減少
    if (damageTimer_ > 0) {
        damageTimer_--;
    }
    if (weakHitFlashTimer_ > 0) {
        weakHitFlashTimer_--;
    }
    if (weakHitCooldown_ > 0) {
        weakHitCooldown_--;
    }

    // HPが約5分の3（60%）以下になったらバリアと弱点を展開
    if (!barrierTriggered_ && hp_ <= (maxHp_ * 3) / 5) {
        barrierTriggered_ = true;
        hasBarrier_ = true;
        barrierTimer_ = 0;
        weakHp_ = weakMaxHp_;
        weakHitCooldown_ = 0;
        weakX_ = weakBaseX_;
        weakY_ = weakBaseY_;
        weakPrevX_ = weakX_;
        weakPrevY_ = weakY_;
    }

    // バリア中は弱点を上下に動かしつつマップ当たり判定を行う
    if (hasBarrier_) {
        barrierTimer_++;

        float nextX = weakBaseX_;
        float nextY = weakBaseY_ + std::sinf(static_cast<float>(barrierTimer_) * 0.05f) * 120.0f;
        if (!IsValidFloat(nextX)) nextX = weakBaseX_;
        if (!IsValidFloat(nextY)) nextY = weakBaseY_;

        float weakVelX = nextX - weakPrevX_;
        float weakVelY = nextY - weakPrevY_;

        weakX_ = nextX;
        if (hasValidMap && weakVelX != 0.0f) {
            ResolveMapCollisionX(weakX_, weakY_, weakWidth_, weakHeight_, weakVelX, map);
        }

        weakY_ = nextY;
        if (hasValidMap && weakVelY != 0.0f) {
            ResolveMapCollisionY(weakX_, weakY_, weakWidth_, weakHeight_, weakVelY, map);
        }

        if (hasValidMap) {
            ResolveOverlapWithMap(weakX_, weakY_, weakWidth_, weakHeight_, map);
        }

        weakPrevX_ = weakX_;
        weakPrevY_ = weakY_;
    }
    else {
        barrierTimer_ = 0;
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

        // segmentCount の範囲外アクセス防止
        int segCount = bullet.segmentCount;
        if (segCount <= 0) {
            continue;
        }
        int maxSegs = static_cast<int>(sizeof(bullet.segments) / sizeof(bullet.segments[0]));
        if (segCount > maxSegs) {
            segCount = maxSegs;
        }

        // 反射したすべての線分をチェック
        for (int j = 0; j < segCount; j++) {
            const BeamSegment& seg = bullet.segments[j];

            if (hasBarrier_) {
                // クールタイム中なら弱点への連続ヒットを少し抑えて破壊時間を稼ぐ
                if (weakHitCooldown_ <= 0 &&
                    CheckSegmentToBox(seg.startX, seg.startY, seg.endX, seg.endY, beamRadius, weakX_, weakY_, weakWidth_, weakHeight_)) {
                    weakHp_ -= 1;
                    weakHitFlashTimer_ = 4;
                    weakHitCooldown_ = 4; // 4フレームごとに削る（0にすると毎フレーム削れる）

                    if (weakHp_ <= 0) {
                        weakHp_ = 0;
                        hasBarrier_ = false;
                    }
                    break;
                }
            }
            else {
                // バリアが無い時はボス本体にダメージ
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

    SanitizeState();
}

// 描画
void Boss::Draw() {
    SanitizeState();

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

    // バリアと弱点の描画
    if (hasBarrier_) {
        const float pad = 18.0f;
        Novice::DrawBox(
            static_cast<int>(x_ - pad),
            static_cast<int>(y_ - pad),
            static_cast<int>(width_ + pad * 2.0f),
            static_cast<int>(height_ + pad * 2.0f),
            0.0f,
            0x00FFFFFF,
            kFillModeWireFrame
        );
        Novice::DrawBox(
            static_cast<int>(x_ - pad + 4.0f),
            static_cast<int>(y_ - pad + 4.0f),
            static_cast<int>(width_ + (pad - 4.0f) * 2.0f),
            static_cast<int>(height_ + (pad - 4.0f) * 2.0f),
            0.0f,
            0x66FFFFAA,
            kFillModeWireFrame
        );

        // 弱点オブジェクトの描画
        unsigned int weakColor = (weakHitFlashTimer_ > 0) ? WHITE : 0xFFDD00FF;
        Novice::DrawBox(
            static_cast<int>(weakX_),
            static_cast<int>(weakY_),
            static_cast<int>(weakWidth_),
            static_cast<int>(weakHeight_),
            0.0f,
            weakColor,
            kFillModeSolid
        );
        Novice::DrawBox(
            static_cast<int>(weakX_ - 3.0f),
            static_cast<int>(weakY_ - 3.0f),
            static_cast<int>(weakWidth_ + 6.0f),
            static_cast<int>(weakHeight_ + 6.0f),
            0.0f,
            RED,
            kFillModeWireFrame
        );

        // 弱点の上部に耐久ゲージを表示（ゼロ除算・負幅防止）
        int weakBarW = static_cast<int>(weakWidth_);
        int weakBarH = 8;
        int weakBarX = static_cast<int>(weakX_);
        int weakBarY = static_cast<int>(weakY_) - 16;
        float weakRatio = (weakMaxHp_ > 0)
            ? (static_cast<float>(weakHp_) / static_cast<float>(weakMaxHp_))
            : 0.0f;
        weakRatio = (std::max)(0.0f, (std::min)(1.0f, weakRatio));
        int curWeakW = static_cast<int>(static_cast<float>(weakBarW) * weakRatio);

        Novice::DrawBox(weakBarX - 2, weakBarY - 2, weakBarW + 4, weakBarH + 4, 0.0f, BLACK, kFillModeSolid);
        Novice::DrawBox(weakBarX, weakBarY, curWeakW, weakBarH, 0.0f, 0xFFAA00FF, kFillModeSolid);
    }

    // HPバーの描画（画面上部中央）
    int barMaxWidth = 800;
    int barHeight = 24;
    int barX = (1920 - barMaxWidth) / 2;
    int barY = 40;

    // 背景（黒）
    Novice::DrawBox(barX - 4, barY - 4, barMaxWidth + 8, barHeight + 8, 0.0f, BLACK, kFillModeSolid);
    // 現在のHP（赤・ゼロ除算・範囲外防止）
    float hpRatio = (maxHp_ > 0)
        ? (static_cast<float>(hp_) / static_cast<float>(maxHp_))
        : 0.0f;
    hpRatio = (std::max)(0.0f, (std::min)(1.0f, hpRatio));
    int currentBarWidth = static_cast<int>(static_cast<float>(barMaxWidth) * hpRatio);
    Novice::DrawBox(barX, barY, currentBarWidth, barHeight, 0.0f, RED, kFillModeSolid);

    // 5分の3（60%）ラインの目印
    int thresholdX = barX + (barMaxWidth * 3) / 5;
    Novice::DrawLine(thresholdX, barY - 4, thresholdX, barY + barHeight + 4, 0xFFD400);
}