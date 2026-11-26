
#include "enemy.h"
#include <Novice.h>
#include <cmath>
#include <algorithm>

// 初期化
void Enemy::Initialize(float x, float y) {

    x_ = x;
    y_ = y;

    hp_ = 30;
    isAlive_ = true;

    damageTimer_ = 0;
    shootTimer_ = 60;

    for (int i = 0; i < kMaxBullets; i++) {
        bullets_[i].isActive = false;
    }
}

// ビームと敵の当たり判定
bool Enemy::CheckBeamCollision(
    float x1,
    float y1,
    float x2,
    float y2
) const {

    float minX = x_ - 10.0f;
    float maxX = x_ + width_ + 10.0f;

    float minY = y_ - 10.0f;
    float maxY = y_ + height_ + 10.0f;

    float dx = x2 - x1;
    float dy = y2 - y1;

    float tMin = 0.0f;
    float tMax = 1.0f;

    if (std::abs(dx) < 0.00001f) {
        if (x1 < minX || x1 > maxX) {
            return false;
        }
    } else {
        float t1 = (minX - x1) / dx;
        float t2 = (maxX - x1) / dx;

        if (t1 > t2) {
            std::swap(t1, t2);
        }

        tMin = (std::max)(tMin, t1);
        tMax = (std::min)(tMax, t2);

        if (tMin > tMax) {
            return false;
        }
    }

    if (std::abs(dy) < 0.00001f) {
        if (y1 < minY || y1 > maxY) {
            return false;
        }
    } else {
        float t1 = (minY - y1) / dy;
        float t2 = (maxY - y1) / dy;

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

// 更新
void Enemy::Update(Player& player) {

    if (!isAlive_) {
        return;
    }

    if (damageTimer_ > 0) {
        damageTimer_--;
    }

    // --------------------------------
    // プレイヤーへの攻撃
    // --------------------------------

    shootTimer_--;

    if (shootTimer_ <= 0) {

        for (int i = 0; i < kMaxBullets; i++) {

            if (!bullets_[i].isActive) {

                // 敵の中心
                float startX = x_ + width_ / 2.0f;
                float startY = y_ + height_ / 2.0f;

                // プレイヤーの中心
                float playerX =
                    player.GetX() + player.GetWidth() / 2.0f;

                float playerY =
                    player.GetY() + player.GetHeight() / 2.0f;

                // 発射方向
                float dx = playerX - startX;
                float dy = playerY - startY;

                float length = std::sqrt(dx * dx + dy * dy);

                if (length > 0.0f) {
                    dx /= length;
                    dy /= length;
                }

                // 弾の設定
                bullets_[i].x = startX;
                bullets_[i].y = startY;

                bullets_[i].velocityX = dx * bulletSpeed_;
                bullets_[i].velocityY = dy * bulletSpeed_;

                bullets_[i].radius = bulletRadius_;
                bullets_[i].isActive = true;

                break;
            }
        }

        shootTimer_ = shootInterval_;
    }

    // --------------------------------
    // 敵の弾の更新
    // --------------------------------

    for (int i = 0; i < kMaxBullets; i++) {

        EnemyBullet& bullet = bullets_[i];

        if (!bullet.isActive) {
            continue;
        }

        bullet.x += bullet.velocityX;
        bullet.y += bullet.velocityY;

        // 画面外なら消す
        if (bullet.x < -100.0f ||
            bullet.x > 2020.0f ||
            bullet.y < -100.0f ||
            bullet.y > 1180.0f) {

            bullet.isActive = false;
            continue;
        }

        // プレイヤーとの当たり判定
        float playerLeft = player.GetX();
        float playerRight = player.GetX() + player.GetWidth();

        float playerTop = player.GetY();
        float playerBottom = player.GetY() + player.GetHeight();

        if (bullet.x + bullet.radius > playerLeft &&
            bullet.x - bullet.radius < playerRight &&
            bullet.y + bullet.radius > playerTop &&
            bullet.y - bullet.radius < playerBottom) {

            // プレイヤーに当たった
            // プレイヤーのHP・ダメージ処理をここに追加

            bullet.isActive = false;
        }
    }

    // --------------------------------
    // プレイヤーのビームとの当たり判定
    // --------------------------------

    for (int i = 0; i < Player::kMaxBullets; i++) {

        Bullet& bullet = player.bullets_[i];

        if (!bullet.isActive) {
            continue;
        }

        int count = bullet.segmentCount;

        count = (std::max)(0, (std::min)(
            count,
            static_cast<int>(
                sizeof(bullet.segments) / sizeof(bullet.segments[0])
                )
            ));

        for (int j = 0; j < count; j++) {

            BeamSegment& seg = bullet.segments[j];

            if (CheckBeamCollision(
                seg.startX,
                seg.startY,
                seg.endX,
                seg.endY
            )) {

                hp_--;
                damageTimer_ = 4;

                if (hp_ <= 0) {
                    hp_ = 0;
                    isAlive_ = false;

                    // 敵が倒れたら弾も消す
                    for (int k = 0; k < kMaxBullets; k++) {
                        bullets_[k].isActive = false;
                    }
                }

                // 1回のビームにつき1ダメージ
                break;
            }
        }

        if (!isAlive_) {
            break;
        }
    }
}

// 描画
void Enemy::Draw() {

    if (!isAlive_) {
        return;
    }

    // 敵本体
    unsigned int color =
        (damageTimer_ > 0) ? WHITE : 0xFF8800FF;

    Novice::DrawBox(
        static_cast<int>(x_),
        static_cast<int>(y_),
        static_cast<int>(width_),
        static_cast<int>(height_),
        0.0f,
        color,
        kFillModeSolid
    );

    // 敵の中心
    int centerX = static_cast<int>(x_ + width_ / 2.0f);
    int centerY = static_cast<int>(y_ + height_ / 2.0f);

    // 中心のコア
    Novice::DrawEllipse(
        centerX,
        centerY,
        15,
        15,
        0.0f,
        RED,
        kFillModeSolid
    );

    // HPバー
    int barWidth = static_cast<int>(width_);
    int barHeight = 6;

    Novice::DrawBox(
        static_cast<int>(x_),
        static_cast<int>(y_) - 12,
        barWidth,
        barHeight,
        0.0f,
        BLACK,
        kFillModeSolid
    );

    int hpWidth = static_cast<int>(
        barWidth * (static_cast<float>(hp_) / 30.0f)
        );

    Novice::DrawBox(
        static_cast<int>(x_),
        static_cast<int>(y_) - 12,
        hpWidth,
        barHeight,
        0.0f,
        GREEN,
        kFillModeSolid
    );

    // 敵の弾
    for (int i = 0; i < kMaxBullets; i++) {

        const EnemyBullet& bullet = bullets_[i];

        if (!bullet.isActive) {
            continue;
        }

        Novice::DrawEllipse(
            static_cast<int>(bullet.x),
            static_cast<int>(bullet.y),
            static_cast<int>(bullet.radius),
            static_cast<int>(bullet.radius),
            0.0f,
            0xFF8800FF,
            kFillModeSolid
        );

        Novice::DrawEllipse(
            static_cast<int>(bullet.x),
            static_cast<int>(bullet.y),
            5,
            5,
            0.0f,
            WHITE,
            kFillModeSolid
        );
    }
}
