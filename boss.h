#pragma once
#include "player.h" // プレイヤーのビーム情報を受け取るため

// マップ上に孤立して配置される弱点データ
struct WeakPoint {
	float x = 960.0f;
	float y = 540.0f;
	float width = 64.0f;
	float height = 64.0f;
	int maxHp = 120;
	int hp = 120;
	bool isActive = false;
	int damageTimer = 0;

	// 指定したマップ座標に孤立配置して有効化
	void Spawn(float posX, float posY) {
		x = posX;
		y = posY;
		hp = maxHp;
		isActive = true;
		damageTimer = 0;
	}

	void Deactivate() {
		isActive = false;
		hp = 0;
		damageTimer = 0;
	}
};

class Boss {
public:
	// 初期化
	void Initialize();

	// 更新（プレイヤーのビームとの当たり判定もここで行う）
	void Update(Player& player);

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

	// ボスの座標・サイズ
	float x_ = 1500.0f;
	float y_ = 400.0f;
	float width_ = 160.0f;
	float height_ = 160.0f;

	// 移動用
	float speedY_ = 4.0f;

	// ステータス
	int maxHp_ = 1000;
	int hp_ = 1000;
	bool isAlive_ = true;

	// バリアギミック用（HPが3/5以下で発動、孤立弱点の破壊で解除）
	bool hasBarrier_ = false;
	bool hasTriggeredBarrier_ = false;
	float barrierPadding_ = 24.0f;

	// マップ上に孤立配置する弱点
	WeakPoint weakPoint_;
	float isolatedWeakPointX_ = 920.0f; // マップ上の孤立位置X
	float isolatedWeakPointY_ = 500.0f; // マップ上の孤立位置Y

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