
#pragma once

#include "player.h"

class Boss {
public:
	void Initialize();
	void Update(Player& player);
	void Draw();

	bool IsAlive() const { return isAlive_; }

private:
	enum class State {
		Normal,
		Charge,
		Attack,
		Cooldown
	};

	// ボスの座標・サイズ
	float x_ = 1500.0f;
	float y_ = 400.0f;
	float width_ = 160.0f;
	float height_ = 160.0f;

	// 移動
	float speedY_ = 4.0f;

	// ステータス
	int maxHp_ = 1000;
	int hp_ = 1000;
	bool isAlive_ = true;

	int damageTimer_ = 0;

	// ステート
	State state_ = State::Normal;
	int stateTimer_ = 0;

	// 追尾弾
	struct HomingBullet {
		float x;
		float y;
		float velocityX;
		float velocityY;
		float radius;
		bool isActive;
	};

	static const int kMaxHomingBullets = 10;
	HomingBullet bullets_[kMaxHomingBullets] = {};

	// 弾の設定
	float bulletSpeed_ = 8.0f;
	float bulletRadius_ = 20.0f;
	float homingStrength_ = 0.08f;

	// 攻撃間隔
	int shootInterval_ = 15;
	int shootTimer_ = 0;

	// チャージ攻撃用
	float targetX_ = 0.0f;
	float targetY_ = 0.0f;
};
