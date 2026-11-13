#include <level/gen/feature/IglooFeature.hpp>
#include <level/Level.hpp>
#include <level/LightLayer.hpp>
#include <tile/Tile.hpp>
#include <tile/entity/ChestTileEntity.hpp>
#include <tile/material/Material.hpp>
#include <item/Item.hpp>
#include <item/ItemInstance.hpp>
#include <util/Random.hpp>
#include <stdlib.h>

IglooFeature::IglooFeature(bool_t update)
	: Feature(update) {
}

IglooFeature::~IglooFeature() {
}

static int32_t rotateDataIG(int32_t blockId, int32_t data, int32_t rotation) {
	if (blockId == Tile::stairs_wood->blockID || blockId == Tile::stairs_stone->blockID ||
	    (Tile::stairs_stoneBrickSmooth && blockId == Tile::stairs_stoneBrickSmooth->blockID) ||
	    blockId == Tile::stairs_brick->blockID || (Tile::woodStairsDark && blockId == Tile::woodStairsDark->blockID)) {
		int32_t dir = data & 3, ud = data & 4, nd = dir;
		if (rotation == 1) { if (dir==0) nd=2; else if (dir==1) nd=3; else if (dir==2) nd=1; else if (dir==3) nd=0; }
		else if (rotation == 2) { if (dir==0) nd=1; else if (dir==1) nd=0; else if (dir==2) nd=3; else if (dir==3) nd=2; }
		else if (rotation == 3) { if (dir==0) nd=3; else if (dir==1) nd=2; else if (dir==2) nd=0; else if (dir==3) nd=1; }
		return nd | ud;
	}
	if (blockId == Tile::ladder->blockID) {
		int32_t nd = data;
		if (rotation == 1) { if (data==2) nd=5; else if (data==3) nd=4; else if (data==4) nd=2; else if (data==5) nd=3; }
		else if (rotation == 2) { if (data==2) nd=3; else if (data==3) nd=2; else if (data==4) nd=5; else if (data==5) nd=4; }
		else if (rotation == 3) { if (data==2) nd=4; else if (data==3) nd=5; else if (data==4) nd=3; else if (data==5) nd=2; }
		return nd;
	}
	if (blockId == Tile::furnace->blockID || blockId == Tile::chest->blockID) {
		int32_t nd = data;
		if (rotation == 1) { if (data==2) nd=5; else if (data==3) nd=4; else if (data==4) nd=2; else if (data==5) nd=3; }
		else if (rotation == 2) { if (data==2) nd=3; else if (data==3) nd=2; else if (data==4) nd=5; else if (data==5) nd=4; }
		else if (rotation == 3) { if (data==2) nd=4; else if (data==3) nd=5; else if (data==4) nd=3; else if (data==5) nd=2; }
		return nd;
	}
	return data;
}

void IglooFeature::placeBlockRotated(Level* level, int32_t originX, int32_t originY, int32_t originZ, int32_t lx, int32_t ly, int32_t lz, int32_t blockId, int32_t data, int32_t rotation) {
	int32_t rx = lx, rz = lz;
	if (rotation == 1) { rx = 6 - lz; rz = lx; }
	else if (rotation == 2) { rx = 6 - lx; rz = 6 - lz; }
	else if (rotation == 3) { rx = lz; rz = 6 - lx; }
	int32_t wx = originX + rx;
	int32_t wy = originY + ly;
	int32_t wz = originZ + rz;
	if (wy > 0 && wy < 127) {
		data = rotateDataIG(blockId, data, rotation);
		this->placeBlock(level, wx, wy, wz, blockId, data);
	}
}

static bool_t isWaterAt(Level* level, int32_t x, int32_t y, int32_t z) {
	if (!level) return 0;
	int32_t t = level->getTile(x, y, z);
	if (t == Tile::water->blockID || t == Tile::calmWater->blockID) return 1;
	Material* m = level->getMaterial(x, y, z);
	return m && m->isLiquid();
}

bool_t IglooFeature::place(Level* level, Random* random, int32_t x, int32_t y, int32_t z) {
	if (!level) return 0;
	int32_t ground = level->getHeightmap(x + 3, z + 3);
	if (ground <= 0) ground = y;
	if (ground < 55 || ground > 110) return 0;
	y = ground;

	for (int32_t dx = 0; dx <= 6; dx += 2) {
		for (int32_t dz = 0; dz <= 6; dz += 2) {
			int32_t wx = x + dx;
			int32_t wz = z + dz;
			int32_t gy = level->getHeightmap(wx, wz);
			if (gy <= 0) return 0;
			if (abs(gy - y) > 3) return 0;
			int32_t below = level->getTile(wx, gy - 1, wz);
			if (isWaterAt(level, wx, gy - 1, wz) || isWaterAt(level, wx, gy, wz)) return 0;
			if (below == 0) return 0;
			Tile* bt = (below >= 0 && below < 256) ? Tile::tiles[below] : nullptr;
			if (!bt || !bt->isSolidRender()) return 0;
		}
	}

	int32_t rotation = random->genrand_int32() % 4;
	bool hasBasement = (random->genrand_int32() % 2) == 0;
	int32_t snow = Tile::snow ? Tile::snow->blockID : 80;
	int32_t topSnow = Tile::topSnow ? Tile::topSnow->blockID : 0;
	int32_t carpetBlock = Tile::cloth ? Tile::cloth->blockID : 35;

	auto setR = [&](int32_t lx, int32_t ly, int32_t lz, int32_t id, int32_t meta) {
		this->placeBlockRotated(level, x, y, z, lx, ly, lz, id, meta, rotation);
	};
	auto getRotPos = [&](int32_t lx, int32_t lz, int32_t &rx, int32_t &rz) {
		rx = lx; rz = lz;
		if (rotation == 1) { rx = 6 - lz; rz = lx; }
		else if (rotation == 2) { rx = 6 - lx; rz = 6 - lz; }
		else if (rotation == 3) { rx = lz; rz = 6 - lx; }
	};

	for (int32_t lx = 0; lx <= 6; ++lx) {
		for (int32_t lz = 0; lz <= 6; ++lz) {
			int32_t dx = lx - 3, dz = lz - 3;
			int32_t d2 = dx*dx + dz*dz;
			for (int32_t ly = 0; ly <= 4; ++ly) {
				setR(lx, ly, lz, 0, 0);
			}
			if (d2 <= 10) setR(lx, 0, lz, snow, 0);
			if (d2 <= 10 && d2 >= 5) setR(lx, 1, lz, snow, 0);
			if (d2 <= 9 && d2 >= 5) setR(lx, 2, lz, snow, 0);
			if (d2 <= 6 && d2 >= 2) setR(lx, 3, lz, snow, 0);
			if (d2 <= 2) setR(lx, 4, lz, snow, 0);
		}
	}

	for (int32_t lx = 1; lx <= 5; ++lx) {
		for (int32_t lz = 1; lz <= 5; ++lz) {
			int32_t dx = lx - 3, dz = lz - 3;
			int32_t d2 = dx*dx + dz*dz;
			if (d2 <= 4) {
				setR(lx, 1, lz, 0, 0);
				setR(lx, 2, lz, 0, 0);
			}
			if (d2 <= 1) setR(lx, 3, lz, 0, 0);
		}
	}

	for (int32_t lx = 1; lx <= 5; ++lx) {
		for (int32_t lz = 1; lz <= 5; ++lz) {
			int32_t dx = lx - 3, dz = lz - 3;
			if (dx*dx + dz*dz <= 5) setR(lx, 0, lz, snow, 0);
		}
	}

	for (int32_t lz = -1; lz <= 1; ++lz) {
		for (int32_t ly = 1; ly <= 2; ++ly) setR(3, ly, lz, 0, 0);
		setR(2, 1, lz, snow, 0);
		setR(4, 1, lz, snow, 0);
		setR(2, 2, lz, snow, 0);
		setR(4, 2, lz, snow, 0);
		setR(3, 3, lz, snow, 0);
		setR(3, 0, lz, snow, 0);
	}
	setR(3, 1, -1, 0, 0);
	setR(3, 2, -1, 0, 0);
	setR(3, 1, 0, 0, 0);
	setR(3, 2, 0, 0, 0);
	setR(3, 1, 1, 0, 0);
	setR(3, 2, 1, 0, 0);

	setR(3, 0, 2, carpetBlock, 14);
	setR(2, 0, 2, carpetBlock, 14);
	setR(2, 0, 3, carpetBlock, 14);

	if (Tile::workBench) setR(2, 1, 5, Tile::workBench->blockID, 0);
	if (Tile::furnace) setR(4, 1, 5, Tile::furnace->blockID, 3);

	if (topSnow) {
		for (int32_t lx = 0; lx <= 6; ++lx) {
			for (int32_t lz = 0; lz <= 6; ++lz) {
				int32_t dx = lx - 3, dz = lz - 3;
				int32_t d2 = dx*dx + dz*dz;
				if (d2 <= 2) setR(lx, 5, lz, topSnow, 0);
			}
		}
	}

	if (hasBasement) {
		int32_t sbId = Tile::stoneBrick ? Tile::stoneBrick->blockID : (Tile::rock ? Tile::rock->blockID : 1);
		for (int32_t lx = 0; lx <= 6; ++lx) {
			for (int32_t lz = 1; lz <= 7; ++lz) {
				for (int32_t ly = -11; ly <= -6; ++ly) {
					bool wall = (lx==0||lx==6||lz==1||lz==7||ly==-11||ly==-6);
					int32_t id = wall ? sbId : 0;
					setR(lx, ly, lz, id, 0);
				}
			}
		}

		for (int32_t ly = -1; ly >= -10; --ly) {
			if (ly >= -6) {
				setR(2, ly, 3, sbId, 0);
				setR(4, ly, 3, sbId, 0);
				setR(2, ly, 2, sbId, 0);
				setR(4, ly, 2, sbId, 0);
				setR(3, ly, 1, sbId, 0);
			}
			setR(3, ly, 4, sbId, 0);
			setR(3, ly, 2, 0, 0);
			setR(3, ly, 3, 0, 0);
		}

		setR(3, 0, 4, snow, 0);
		setR(3, 0, 2, 0, 0);
		setR(3, 0, 3, 0, 0);

		for (int32_t ly = 0; ly >= -10; --ly) {
			if (Tile::ladder) setR(3, ly, 3, Tile::ladder->blockID, 2);
		}

		if (Tile::bookshelf) {
			for (int32_t ly = -10; ly <= -9; ++ly) {
				for (int32_t lz = 2; lz <= 5; ++lz) setR(1, ly, lz, Tile::bookshelf->blockID, 0);
			}
		}
		if (Tile::workBench) setR(5, -10, 2, Tile::workBench->blockID, 0);
		if (Tile::furnace) setR(5, -10, 6, Tile::furnace->blockID, 3);

		if (Tile::chest) {
			setR(1, -10, 6, Tile::chest->blockID, 5);
			int32_t rx, rz;
			getRotPos(1, 6, rx, rz);
			level->setTileAndData(x + rx, y - 10, z + rz, Tile::chest->blockID, rotateDataIG(Tile::chest->blockID, 5, rotation), 3);
			ChestTileEntity* te = (ChestTileEntity*)level->getTileEntity(x + rx, y - 10, z + rz);
			if (!te) {
				te = new ChestTileEntity();
				level->setTileEntity(x + rx, y - 10, z + rz, te);
			}
			if (te) {
				ItemInstance gold(Item::goldIngot, 1 + (random->genrand_int32() % 4), 0);
				te->setItem(random->genrand_int32() % 27, &gold);
				ItemInstance apple(Item::apple, 1 + (random->genrand_int32() % 2), 0);
				te->setItem(random->genrand_int32() % 27, &apple);
				if (Item::coal) {
					ItemInstance coal(Item::coal, 1 + (random->genrand_int32() % 4), 0);
					te->setItem(random->genrand_int32() % 27, &coal);
				}
				if (Item::emerald) {
					ItemInstance em(Item::emerald, 1, 0);
					te->setItem(random->genrand_int32() % 27, &em);
				}
			}
		}
	}

	level->updateLight(LightLayer::Sky, x - 2, y - 12, z - 2, x + 10, y + 8, z + 10, 1);
	level->updateLight(LightLayer::Block, x - 2, y - 12, z - 2, x + 10, y + 8, z + 10, 1);
	return 1;
}
