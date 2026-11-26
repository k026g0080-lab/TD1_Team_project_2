#include "boss.h"
#include <Novice.h>
#include <algorithm>
#include <cmath>

namespace {
	// 太さ(radius)を持つ線分 (x1, y1)-(x2, y2) と 矩形 (rx, ry, rw, rh) の当たり判定（スラブ法）
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
		}
		else {
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
		}
		else {
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

	// バリア・孤立弱点の初期化
	hasBarrier_ = false;
	hasTriggeredBarrier_ = false;
	weakPoint_.Deactivate();

	state_ = State::Normal;
	stateTimer_ = 180; // 最初の攻撃まで3秒（60fps想定）

	shootTimer_ = 0;
	targetX_ = 0.0f;
	targetY_ = 0.0f;

	for (int i = 0; i < kMaxHomingBullets; i++) {
		bullets_[i].isActive = false;
	}
}

// 更新
void Boss::Update(Player& player) {
	if (!isAlive_) {
		return;
	}

	// HPが5分の3以下になったらバリアを張り、マップ上の離れた位置に弱点を孤立配置（1回のみ）
	if (!hasTriggeredBarrier_ && hp_ <= (maxHp_ * 3) / 5) {
		hasTriggeredBarrier_ = true;
		hasBarrier_ = true;
		weakPoint_.Spawn(isolatedWeakPointX_, isolatedWeakPointY_);
	}

	// 孤立弱点のタイマー更新（ボスには追従せずマップ上に固定）
	if (weakPoint_.isActive && weakPoint_.damageTimer > 0) {
		weakPoint_.damageTimer--;
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
			targetX_ = player.GetX() + player.GetWidth() / 2.0f;
			targetY_ = player.GetY() + player.GetHeight() / 2.0f;
		}
		break;

	case State::Charge:
		// チャージ中はその場に停止して狙いを定める
		if (stateTimer_ <= 0) {
			state_ = State::Attack;
			stateTimer_ = 90; // 攻撃継続時間
			shootTimer_ = 0;
		}
		break;

	case State::Attack:
		// ① ボスビームとプレイヤーの当たり判定
	{
		float beamX = 0.0f;
		float beamW = x_;
		float beamY = (y_ + height_ / 2.0f) - (beamHeight_ / 2.0f);
		float beamH = beamHeight_;

		if (CheckBoxToBox(
			beamX, beamY, beamW, beamH,
			player.GetX(), player.GetY(), player.GetWidth(), player.GetHeight())) {
			// プレイヤー被弾時の処理が必要ならここに記述
		}
	}

	// ② 一定間隔で追尾弾を発射
	shootTimer_--;
	if (shootTimer_ <= 0) {
		for (int i = 0; i < kMaxHomingBullets; i++) {
			if (!bullets_[i].isActive) {
				float startX = x_;
				float startY = y_ + height_ / 2.0f;

				float playerX = player.GetX() + player.GetWidth() / 2.0f;
				float playerY = player.GetY() + player.GetHeight() / 2.0f;

				float dx = playerX - startX;
				float dy = playerY - startY;
				float length = std::sqrt(dx * dx + dy * dy);
				if (length > 0.0f) {
					dx /= length;
					dy /= length;
				}

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

	// 2. 追尾弾の更新とプレイヤーとの当たり判定
	for (int i = 0; i < kMaxHomingBullets; i++) {
		HomingBullet& bullet = bullets_[i];
		if (!bullet.isActive) {
			continue;
		}

		// プレイヤーへの追尾方向を計算
		float playerX = player.GetX() + player.GetWidth() / 2.0f;
		float playerY = player.GetY() + player.GetHeight() / 2.0f;

		float dx = playerX - bullet.x;
		float dy = playerY - bullet.y;
		float length = std::sqrt(dx * dx + dy * dy);
		if (length > 0.0f) {
			dx /= length;
			dy /= length;

			bullet.velocityX += dx * homingStrength_;
			bullet.velocityY += dy * homingStrength_;

			// 速度を一定に保つ
			float speed = std::sqrt(
				bullet.velocityX * bullet.velocityX +
				bullet.velocityY * bullet.velocityY
			);
			if (speed > 0.0f) {
				bullet.velocityX = bullet.velocityX / speed * bulletSpeed_;
				bullet.velocityY = bullet.velocityY / speed * bulletSpeed_;
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

	// 3. 被弾フラッシュタイマー減少
	if (damageTimer_ > 0) {
		damageTimer_--;
	}

	// 4. プレイヤーのビームとの当たり判定
	const float beamRadius = 10.0f;
	for (int i = 0; i < Player::kMaxBullets; i++) {
		Bullet& bullet = player.bullets_[i];
		if (!bullet.isActive) {
			continue;
		}

		for (int j = 0; j < bullet.segmentCount; j++) {
			const BeamSegment& seg = bullet.segments[j];

			// バリア展開中：マップ上の孤立弱点に当たったら弱点HPを減らし、破壊でバリア解除
			if (hasBarrier_) {
				if (weakPoint_.isActive &&
					CheckSegmentToBox(seg.startX, seg.startY, seg.endX, seg.endY, beamRadius,
						weakPoint_.x, weakPoint_.y, weakPoint_.width, weakPoint_.height)) {
					weakPoint_.hp -= 1;
					weakPoint_.damageTimer = 2;
					if (weakPoint_.hp <= 0) {
						weakPoint_.Deactivate();
						hasBarrier_ = false; // 孤立弱点破壊でバリア解除
					}
					break;
				}
			}
			else {
				// バリアなし：ボス本体にダメージ
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
}

// 描画
void Boss::Draw() {
	if (!isAlive_) {
		return;
	}

	float centerY = y_ + height_ / 2.0f;

	// 1. チャージ中（予兆）の描画
	if (state_ == State::Charge) {
		// 攻撃範囲の予測線（半透明の赤）
		int warnY = static_cast<int>(centerY - beamHeight_ / 2.0f);
		Novice::DrawBox(0, warnY, static_cast<int>(x_), static_cast<int>(beamHeight_), 0.0f, 0xFF000044, kFillModeSolid);

		// 発射口に集まるエネルギー球（残り時間が少ないほど大きくなる）
		float progress = 1.0f - (static_cast<float>(stateTimer_) / 90.0f);
		int chargeRadius = static_cast<int>(60.0f * progress);
		Novice::DrawEllipse(static_cast<int>(x_), static_cast<int>(centerY), chargeRadius, chargeRadius, 0.0f, 0xFFFF00FF, kFillModeSolid);
	}

	// 2. ビーム発射中の描画
	if (state_ == State::Attack) {
		int beamY = static_cast<int>(centerY - beamHeight_ / 2.0f);
		int beamH = static_cast<int>(beamHeight_);

		// 外側の赤いオーラ
		Novice::DrawBox(0, beamY, static_cast<int>(x_), beamH, 0.0f, 0xFF2222FF, kFillModeSolid);
		// 中心の白い高エネルギー芯
		Novice::DrawBox(0, beamY + beamH / 4, static_cast<int>(x_), beamH / 2, 0.0f, WHITE, kFillModeSolid);
	}

	// 3. 追尾弾の描画
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

	// 4. ボス本体の描画
	unsigned int color = 0x8800FFFF; // 通常：紫
	if (damageTimer_ > 0) {
		color = WHITE;               // 被弾：白
	}
	else if (state_ == State::Charge) {
		// チャージ中は黄色とオレンジで点滅
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

	// 5. バリアの描画
	if (hasBarrier_) {
		int bx = static_cast<int>(x_ - barrierPadding_);
		int by = static_cast<int>(y_ - barrierPadding_);
		int bw = static_cast<int>(width_ + barrierPadding_ * 2.0f);
		int bh = static_cast<int>(height_ + barrierPadding_ * 2.0f);

		// 半透明の水色バリア
		Novice::DrawBox(bx, by, bw, bh, 0.0f, 0x00FFFF44, kFillModeSolid);
		Novice::DrawBox(bx, by, bw, bh, 0.0f, 0x00FFFFFF, kFillModeWireFrame);
	}

	// 6. マップ上に孤立配置された弱点の描画
	if (weakPoint_.isActive) {
		unsigned int wpColor = (weakPoint_.damageTimer > 0) ? WHITE : 0x00FF88FF;
		Novice::DrawBox(
			static_cast<int>(weakPoint_.x),
			static_cast<int>(weakPoint_.y),
			static_cast<int>(weakPoint_.width),
			static_cast<int>(weakPoint_.height),
			0.0f,
			wpColor,
			kFillModeSolid
		);
		Novice::DrawBox(
			static_cast<int>(weakPoint_.x),
			static_cast<int>(weakPoint_.y),
			static_cast<int>(weakPoint_.width),
			static_cast<int>(weakPoint_.height),
			0.0f,
			WHITE,
			kFillModeWireFrame
		);
	}

	// 7. HPバーの描画（画面上部中央）
	int barMaxWidth = 800;
	int barHeight = 24;
	int barX = (1920 - barMaxWidth) / 2;
	int barY = 40;

	// 背景（黒）
	Novice::DrawBox(barX - 4, barY - 4, barMaxWidth + 8, barHeight + 8, 0.0f, BLACK, kFillModeSolid);

	// 現在HP（赤）
	int currentBarWidth = static_cast<int>(barMaxWidth * (static_cast<float>(hp_) / maxHp_));
	Novice::DrawBox(barX, barY, currentBarWidth, barHeight, 0.0f, RED, kFillModeSolid);

	// 孤立弱点のHPゲージ（バリア展開中のみ表示）
	if (hasBarrier_ && weakPoint_.isActive) {
		int wpBarY = barY + barHeight + 8;
		int wpBarHeight = 10;
		Novice::DrawBox(barX - 2, wpBarY - 2, barMaxWidth + 4, wpBarHeight + 4, 0.0f, BLACK, kFillModeSolid);
		int currentWpWidth = static_cast<int>(barMaxWidth * (static_cast<float>(weakPoint_.hp) / weakPoint_.maxHp));
		Novice::DrawBox(barX, wpBarY, currentWpWidth, wpBarHeight, 0.0f, 0x00FF88FF, kFillModeSolid);
	}
}