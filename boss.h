#pragma once
#include "player.h" // プレイヤーのビーム情報を受け取るため
#include "map.h"    // マップの当たり判定を受け取るため

class Boss {
public:
	// 初期化
	void Initialize();

	// 更新（マップの当たり判定とプレイヤーのビーム判定を行う）
	void Update(Player& player, const Map& map);

	// 描画
	void Draw();

	// 生存しているか
	bool IsAlive() const { return isAlive_; }

	// バリア展開中か
	bool HasBarrier() const { return hasBarrier_; }

private:
	// ボスの行動パターン（ステート）
	enum class State {
		Normal,   // 通常移動
		Charge,   // チャージ中（予兆）
		Attack,   // 攻撃中
		Cooldown  // 攻撃後の硬直
	};

	// 矩形とマップ壁の当たり判定・押し戻し（X軸・Y軸）
	bool ResolveMapCollisionX(float& boxX, float& boxY, float boxW, float boxH, float velX, const Map& map);
	bool ResolveMapCollisionY(float& boxX, float& boxY, float boxW, float boxH, float velY, const Map& map);

	// 静止中・重なり中に壁へめり込んだ場合の押し出し処理
	void ResolveOverlapWithMap(float& boxX, float& boxY, float boxW, float boxH, const Map& map);

	// 内部状態の整合性チェック
	void SanitizeState();

	// ボスの座標・サイズ
	float x_ = 1500.0f;
	float y_ = 400.0f;
	float width_ = 160.0f;
	float height_ = 160.0f;

	// 移動用
	float speedX_ = 0.0f;
	float speedY_ = 0.0f;

	// ステータス
	int maxHp_ = 1000;
	int hp_ = 1000;
	bool isAlive_ = true;

	// バリア・弱点ギミック用（HPが3/5以下で発動、マップ上に孤立した弱点破壊で解除）
	bool hasBarrier_ = false;
	bool barrierTriggered_ = false;
	int barrierTimer_ = 0;
	float barrierPadding_ = 24.0f;

	// マップ上に孤立配置する弱点
	float weakX_ = 900.0f;
	float weakY_ = 500.0f;
	float weakBaseX_ = 900.0f;
	float weakBaseY_ = 500.0f;
	float weakWidth_ = 64.0f;
	float weakHeight_ = 64.0f;
	float weakPrevX_ = 900.0f;
	float weakPrevY_ = 500.0f;

	int weakMaxHp_ = 40;
	int weakHp_ = 40;
	int weakHitFlashTimer_ = 0;
	int weakHitCooldown_ = 0;

	// ダメージ演出（被弾時に一瞬白く光らせる用）
	int damageTimer_ = 0;

	// ステート管理
	State state_ = State::Normal; // 現在の状態
	int stateTimer_ = 0;          // 状態ごとの残り時間フレーム

	// ビーム攻撃用
	float beamHeight_ = 100.0f;   // ボスビームの太さ（高さ）

	// 追尾弾用
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

	// 追尾弾の設定
	float bulletSpeed_ = 8.0f;
	float bulletRadius_ = 20.0f;
	float homingStrength_ = 0.08f;

	// 追尾弾の発射間隔
	int shootInterval_ = 15;
	int shootTimer_ = 0;

	// チャージ時のターゲット座標記録用
	float targetX_ = 0.0f;
	float targetY_ = 0.0f;
};