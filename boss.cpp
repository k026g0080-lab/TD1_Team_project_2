
#include "boss.h"
#include <Novice.h>
#include <algorithm>
#include <cmath>

// 初期化
void Boss::Initialize() {
	x_ = 1500.0f;
	y_ = 400.0f;

	hp_ = maxHp_;
	isAlive_ = true;
	damageTimer_ = 0;

	state_ = State::Normal;
	stateTimer_ = 180;

	shootTimer_ = 0;

	for (int i = 0; i < kMaxHomingBullets; i++) {
		bullets_[i].isActive = false;
	}
}

// 更新
void Boss::Update(Player& player) {

	if (!isAlive_) {
		return;
	}

	stateTimer_--;

	switch (state_) {

	case State::Normal:

		// 上下移動
		y_ += speedY_;

		if (y_ < 100.0f || y_ + height_ > 900.0f) {
			speedY_ *= -1.0f;
		}

		// チャージ開始
		if (stateTimer_ <= 0) {

			state_ = State::Charge;
			stateTimer_ = 90;

			// プレイヤーの位置を記録
			targetX_ = player.GetX() + player.GetWidth() / 2.0f;
			targetY_ = player.GetY() + player.GetHeight() / 2.0f;
		}

		break;

	case State::Charge:

		// チャージ終了
		if (stateTimer_ <= 0) {

			state_ = State::Attack;
			stateTimer_ = 120;
			shootTimer_ = 0;
		}

		break;

	case State::Attack:

		// 一定間隔で追尾弾を発射
		shootTimer_--;

		if (shootTimer_ <= 0) {

			for (int i = 0; i < kMaxHomingBullets; i++) {

				if (!bullets_[i].isActive) {

					// ボスの中心
					float startX = x_;
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

					// 弾を初期化
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

		// 攻撃終了
		if (stateTimer_ <= 0) {

			state_ = State::Cooldown;
			stateTimer_ = 60;
		}

		break;

	case State::Cooldown:

		if (stateTimer_ <= 0) {

			state_ = State::Normal;
			stateTimer_ = 180;
		}

		break;
	}

	// 追尾弾の更新
	for (int i = 0; i < kMaxHomingBullets; i++) {

		HomingBullet& bullet = bullets_[i];

		if (!bullet.isActive) {
			continue;
		}

		// プレイヤーの現在位置
		float playerX =
			player.GetX() + player.GetWidth() / 2.0f;

		float playerY =
			player.GetY() + player.GetHeight() / 2.0f;

		// プレイヤーへの方向
		float dx = playerX - bullet.x;
		float dy = playerY - bullet.y;

		float length = std::sqrt(dx * dx + dy * dy);

		if (length > 0.0f) {

			dx /= length;
			dy /= length;

			// 現在の速度と追尾方向を混ぜる
			bullet.velocityX += dx * homingStrength_;
			bullet.velocityY += dy * homingStrength_;

			// 速度を一定に保つ
			float speed = std::sqrt(
				bullet.velocityX * bullet.velocityX +
				bullet.velocityY * bullet.velocityY
			);

			if (speed > 0.0f) {

				bullet.velocityX =
					bullet.velocityX / speed * bulletSpeed_;

				bullet.velocityY =
					bullet.velocityY / speed * bulletSpeed_;
			}
		}

		// 移動
		bullet.x += bullet.velocityX;
		bullet.y += bullet.velocityY;

		// 画面外に出たら消す
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

			// 命中時の処理
			bullet.isActive = false;
		}
	}

	// 被弾タイマー
	if (damageTimer_ > 0) {
		damageTimer_--;
	}

	// プレイヤーのビームとの当たり判定
	for (int i = 0; i < Player::kMaxBullets; i++) {

		Bullet& bullet = player.bullets_[i];

		if (!bullet.isActive) {
			continue;
		}

		for (int j = 0; j < bullet.segmentCount; j++) {

			const BeamSegment& seg = bullet.segments[j];

			// 線分とボスの矩形の簡易判定
			float minX = (std::min)(seg.startX, seg.endX);
			float maxX = (std::max)(seg.startX, seg.endX);
			float minY = (std::min)(seg.startY, seg.endY);
			float maxY = (std::max)(seg.startY, seg.endY);

			if (maxX + 10.0f >= x_ &&
				minX - 10.0f <= x_ + width_ &&
				maxY + 10.0f >= y_ &&
				minY - 10.0f <= y_ + height_) {

				hp_--;
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

	// チャージ中
	if (state_ == State::Charge) {

		Novice::DrawEllipse(
			static_cast<int>(x_),
			static_cast<int>(centerY),
			60,
			60,
			0.0f,
			0xFFFF00FF,
			kFillModeSolid
		);
	}

	// 追尾弾の描画
	for (int i = 0; i < kMaxHomingBullets; i++) {

		const HomingBullet& bullet = bullets_[i];

		if (!bullet.isActive) {
			continue;
		}

		// 外側
		Novice::DrawEllipse(
			static_cast<int>(bullet.x),
			static_cast<int>(bullet.y),
			static_cast<int>(bullet.radius),
			static_cast<int>(bullet.radius),
			0.0f,
			RED,
			kFillModeSolid
		);

		// 中心
		Novice::DrawEllipse(
			static_cast<int>(bullet.x),
			static_cast<int>(bullet.y),
			8,
			8,
			0.0f,
			WHITE,
			kFillModeSolid
		);
	}

	// ボスの色
	unsigned int color = 0x8800FFFF;

	if (damageTimer_ > 0) {
		color = WHITE;
	} else if (state_ == State::Charge) {
		color = 0xFFFF00FF;
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

	// HPバー
	int barMaxWidth = 800;
	int barHeight = 24;
	int barX = (1920 - barMaxWidth) / 2;
	int barY = 40;

	Novice::DrawBox(
		barX - 4,
		barY - 4,
		barMaxWidth + 8,
		barHeight + 8,
		0.0f,
		BLACK,
		kFillModeSolid
	);

	int currentBarWidth =
		static_cast<int>(
			barMaxWidth * (static_cast<float>(hp_) / maxHp_)
			);

	Novice::DrawBox(
		barX,
		barY,
		currentBarWidth,
		barHeight,
		0.0f,
		RED,
		kFillModeSolid
	);
}
