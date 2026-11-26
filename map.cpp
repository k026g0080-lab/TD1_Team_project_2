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
    // 地殻変動の回数を増やす
    shiftCount_++;

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