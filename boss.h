
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



	// ステータス
	int maxHp_ = 1000;
	int hp_ = 1000;
	bool isAlive_ = true;

