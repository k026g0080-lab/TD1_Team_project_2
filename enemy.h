
#pragma once

#include "player.h"

class Enemy {
public:

	void Initialize(float x, float y);

	void Update(Player& player);

	void Draw();

	bool IsAlive() const {
		return isAlive_;
	}
	// 指定座標が敵の内部にあるか調べる
	bool ContainsPoint(float x, float y) const {
		return isAlive_ &&
			x >= x_ &&
			x <= x_ + width_ &&
			y >= y_ &&
			y <= y_ + height_;
	}
private:

	// 敵の位置とサイズ
	float x_ = 0.0f;
	float y_ = 0.0f;

	float width_ = 64.0f;
	float height_ = 64.0f;

	// HP
	int hp_ = 30;
	bool isAlive_ = true;

	// 被弾演出
	int damageTimer_ = 0;

	// 攻撃用の弾
	struct EnemyBullet {
		float x;
		float y;
		float velocityX;
		float velocityY;
		float radius;
		bool isActive;
	};

	static const int kMaxBullets = 5;

	EnemyBullet bullets_[kMaxBullets] = {};

	// 攻撃設定
	float bulletSpeed_ = 6.0f;
	float bulletRadius_ = 12.0f;

	int shootInterval_ = 90;
	int shootTimer_ = 0;

	// ビームとの当たり判定
	bool CheckBeamCollision(
		float x1,
		float y1,
		float x2,
		float y2
	) const;
};
