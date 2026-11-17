#include "map.h"
#include <Novice.h>
#include <cmath>

// マップの描画
void Map::Draw() {

    for (int y = 0; y < kMapHeight; y++) {

        for (int x = 0; x < kMapWidth; x++) {

            if (mapData_[y][x] == kGround) {

                Novice::DrawBox(
                    x * kMapChipSize,
                    y * kMapChipSize,
                    kMapChipSize,
                    kMapChipSize,
                    0.0f,
                    WHITE,
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