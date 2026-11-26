#include <Novice.h>
#include "player.h"
#include "map.h"
#include "boss.h"
#include "enemy.h"
#include <cstdlib>
#include <cstring>

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