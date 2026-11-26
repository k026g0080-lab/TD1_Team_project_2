
#include "map.h"
#include <Novice.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

// 初期化
void Map::Initialize() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (int y = 0; y < kMapHeight; y++) {
        for (int x = 0; x < kMapWidth; x++) {
            mapData_[y][x] = initialMapData_[y][x];
        }
    }

    shiftTimer_ = shiftInterval_;
    shiftEffectTimer_ = 0;
}

// 更新
void Map::Update() {
    if (shiftEffectTimer_ > 0) {
        shiftEffectTimer_--;
    }

    // タイマーを減らし、0になったら地殻変動を起こす
    shiftTimer_--;
    if (shiftTimer_ <= 0) {
        TriggerCrustalShift();
        shiftTimer_ = shiftInterval_;
    }
}

// ランダムに地殻変動を起こす
void Map::TriggerCrustalShift() {
    shiftEffectTimer_ = 15; // 15フレームだけブロックの色を変えて変動を分かりやすくする

    // 1. まず外周（上2行・下2行・左右1列）以外の内部を一旦リセット
    for (int y = 2; y < kMapHeight - 2; y++) {
        for (int x = 1; x < kMapWidth - 1; x++) {
            mapData_[y][x] = kEmpty;
        }
    }

    // 2. ランダムな位置に「地面からせり上がる柱」を生成（3〜5本）
    int pillarCount = 3 + (std::rand() % 3);
    for (int i = 0; i < pillarCount; i++) {
        // ボスのいる右端エリア(x >= 23)やプレイヤー初期位置付近を避けつつランダム配置
        int px = 3 + (std::rand() % 20);
        int height = 2 + (std::rand() % 7); // 高さ 2〜8マス

        for (int h = 0; h < height; h++) {
            int py = (kMapHeight - 3) - h; // 床(y=14)から上へ積み上げる
            if (py >= 2) {
                mapData_[py][px] = kGround;
            }
        }
    }

    // 3. ランダムな位置に「天井から垂れ下がる鍾乳石（反射壁）」を生成（2〜4本）
    int ceilingCount = 2 + (std::rand() % 3);
    for (int i = 0; i < ceilingCount; i++) {
        int cx = 2 + (std::rand() % 23);
        int length = 2 + (std::rand() % 6); // 長さ 2〜7マス

        for (int l = 0; l < length; l++) {
            int cy = 2 + l; // 天井(y=2)から下へ伸ばす
            if (cy < kMapHeight - 4) { // プレイヤーが通れる隙間は最低限残す
                mapData_[cy][cx] = kGround;
            }
        }
    }

    // 4. 空中に浮かぶ「反射用の浮島ブロック」をランダム生成（1〜2個）
    int islandCount = 1 + (std::rand() % 2);
    for (int i = 0; i < islandCount; i++) {
        int ix = 5 + (std::rand() % 16);
        int iy = 5 + (std::rand() % 6);
        int width = 2 + (std::rand() % 3); // 横幅 2〜4マス

        for (int w = 0; w < width; w++) {
            if (ix + w < kMapWidth - 2) {
                mapData_[iy][ix + w] = kGround;
            }
        }
    }
}

// マップの描画
void Map::Draw() {
    // 地殻変動の直前はブロックを少し黄色っぽく点滅させて予兆を出す
    unsigned int blockColor = WHITE;
    if (shiftEffectTimer_ > 0) {
        blockColor = 0xFFCC66FF; // 変動直後はオレンジ色に光る
    }
    else if (shiftTimer_ < 60 && (shiftTimer_ / 5) % 5 == 0) {
        blockColor = 0xFFFF88FF; // 変動1秒前から黄色く点滅して予兆を知らせる
    }

    for (int y = 0; y < kMapHeight; y++) {
        for (int x = 0; x < kMapWidth; x++) {
            if (mapData_[y][x] == kGround) {
                Novice::DrawBox(
                    x * kMapChipSize,
                    y * kMapChipSize,
                    kMapChipSize,
                    kMapChipSize,
                    0.0f,
                    blockColor,
                    kFillModeSolid
                );
            }
        }
    }
}

// マップチップの当たり判定
bool Map::IsSolid(int mapX, int mapY) const {
    // マップの範囲外
    if (mapX < 0 || mapX >= kMapWidth ||
        mapY < 0 || mapY >= kMapHeight) {
        return false;
    }

    // 地面ならtrue
    return mapData_[mapY][mapX] == kGround;
}

// ワールド座標から当たり判定を取得
bool Map::IsSolidWorld(float x, float y) const {
    int mapX = static_cast<int>(floorf(x / kMapChipSize));
    int mapY = static_cast<int>(floorf(y / kMapChipSize));

    return IsSolid(mapX, mapY);
}
=======

#include <Novice.h>
#include "player.h"
#include "map.h"
#include "boss.h"
#include "enemy.h"
#include <cstdlib>
const char kWindowTitle[] = "タイトル";

// 雑魚敵の数
const int kEnemyCount = 3;

// 雑魚敵をランダムな位置に出現させる
void SpawnEnemies(Enemy enemies[], int enemyCount, Map& map, Player& player) {

	for (int i = 0; i < enemyCount; i++) {

		float spawnX = 0.0f;
		float spawnY = 0.0f;

		bool foundPosition = false;

		// 出現場所が見つかるまで探す
		for (int attempt = 0; attempt < 100; attempt++) {

			// マップの範囲内からランダムに選択
			int mapX = 2 + rand() % 21;
			int mapY = 3 + rand() % 11;

			float x = static_cast<float>(mapX * kMapChipSize);
			float y = static_cast<float>(mapY * kMapChipSize);

			// 敵の位置がブロックと重ならないか確認
			if (map.IsSolidWorld(x + 32.0f, y + 32.0f)) {
				continue;
			}

			// プレイヤーから離れた場所に出現させる
			float dx = x - player.GetX();
			float dy = y - player.GetY();

			if (dx * dx + dy * dy < 250.0f * 250.0f) {
				continue;
			}

			// ボスのいる右端付近を避ける
			if (x > 1400.0f) {
				continue;
			}

			spawnX = x;
			spawnY = y;
			foundPosition = true;
			break;
		}

		// 出現場所が見つかった場合
		if (foundPosition) {
			enemies[i].Initialize(spawnX, spawnY);
		}
	}
}
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Novice::Initialize(kWindowTitle, 1920, 1080);

	Novice::SetMouseCursorVisibility(0);

	Map map;
	map.Initialize();

	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Player player;
	player.Initialize();

	Boss boss;
	boss.Initialize();

	// 雑魚敵
	Enemy enemies[kEnemyCount];

	// 最初の雑魚敵を出現させる
	SpawnEnemies(enemies, kEnemyCount, map, player);

	// 地殻変動の回数を記録
	int previousShiftCount = map.GetShiftCount();

	

	while (Novice::ProcessMessage() == 0) {

		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// 更新処理
		player.Update(
    keys,
    preKeys,
    map,
    enemies,
    kEnemyCount
);

		boss.Update(player, map);

		// 雑魚敵の更新
		for (int i = 0; i < kEnemyCount; i++) {
			enemies[i].Update(player);
		}

		
		map.Update();

		// 地殻変動が発生したか確認
		if (previousShiftCount != map.GetShiftCount()) {

			// 地殻変動の回数を更新
			previousShiftCount = map.GetShiftCount();

			// 雑魚敵を新しく出現させる
			SpawnEnemies(enemies, kEnemyCount, map, player);
		}
		// 描画処理
		map.Draw();

		player.Draw();

		// 雑魚敵の描画
		for (int i = 0; i < kEnemyCount; i++) {
			enemies[i].Draw();
		}

		boss.Draw();

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}

