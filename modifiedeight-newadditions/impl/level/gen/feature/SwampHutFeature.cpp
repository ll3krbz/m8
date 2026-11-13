#include <level/gen/feature/SwampHutFeature.hpp>
#include <level/Level.hpp>
#include <level/LightLayer.hpp>
#include <tile/Tile.hpp>
#include <util/Random.hpp>

SwampHutFeature::SwampHutFeature(bool_t update)
	: Feature(update) {
}

SwampHutFeature::~SwampHutFeature() {
}

static int32_t rotateDataSH(int32_t blockId, int32_t data, int32_t rotation) {
	if (blockId == Tile::stairs_wood->blockID || blockId == Tile::stairs_stone->blockID ||
	    (Tile::stairs_stoneBrickSmooth && blockId == Tile::stairs_stoneBrickSmooth->blockID) ||
	    blockId == Tile::stairs_brick->blockID || (Tile::woodStairsDark && blockId == Tile::woodStairsDark->blockID)) {
		int32_t dir = data & 3;
		int32_t ud = data & 4;
		int32_t nd = dir;
		if (rotation == 1) { if (dir==0) nd=2; else if (dir==1) nd=3; else if (dir==2) nd=1; else if (dir==3) nd=0; }
		else if (rotation == 2) { if (dir==0) nd=1; else if (dir==1) nd=0; else if (dir==2) nd=3; else if (dir==3) nd=2; }
		else if (rotation == 3) { if (dir==0) nd=3; else if (dir==1) nd=2; else if (dir==2) nd=0; else if (dir==3) nd=1; }
		return nd | ud;
	}
	if (blockId == Tile::chest->blockID || blockId == Tile::furnace->blockID) {
		int32_t nd = data;
		if (rotation == 1) { if (data==2) nd=5; else if (data==3) nd=4; else if (data==4) nd=2; else if (data==5) nd=3; }
		else if (rotation == 2) { if (data==2) nd=3; else if (data==3) nd=2; else if (data==4) nd=5; else if (data==5) nd=4; }
		else if (rotation == 3) { if (data==2) nd=4; else if (data==3) nd=5; else if (data==4) nd=3; else if (data==5) nd=2; }
		return nd;
	}
	return data;
}

void SwampHutFeature::placeBlockRotated(Level* level, int32_t originX, int32_t originY, int32_t originZ, int32_t lx, int32_t ly, int32_t lz, int32_t blockId, int32_t data, int32_t rotation) {
	int32_t rx = lx, rz = lz;
	if (rotation == 1) { rx = 6 - lz; rz = lx; }
	else if (rotation == 2) { rx = 6 - lx; rz = 8 - lz; }
	else if (rotation == 3) { rx = lz; rz = 8 - lx; }
	int32_t wx = originX + rx;
	int32_t wy = originY + ly;
	int32_t wz = originZ + rz;
	if (wy > 0 && wy < 127) {
		data = rotateDataSH(blockId, data, rotation);
		this->placeBlock(level, wx, wy, wz, blockId, data);
	}
}

bool_t SwampHutFeature::place(Level* level, Random* random, int32_t x, int32_t y, int32_t z) {
	if (!level || y < 55 || y > 100) return 0;

	int32_t ground = level->getHeightmap(x + 3, z + 4);
	if (ground <= 0) ground = y;
	y = ground;

	int32_t rotation = random->genrand_int32() % 4;

	int32_t sprucePlanks = Tile::wood ? Tile::wood->blockID : 5;
	int32_t sprucePlanksMeta = 1;
	int32_t oakLog = Tile::treeTrunk ? Tile::treeTrunk->blockID : 17;
	int32_t oakLogMeta = 0;
	int32_t oakFence = Tile::fence ? Tile::fence->blockID : 85;
	int32_t spruceStairs = Tile::stairs_wood ? Tile::stairs_wood->blockID : 53;

	auto setRot = [&](int32_t lx, int32_t ly, int32_t lz, int32_t id, int32_t meta) {
		this->placeBlockRotated(level, x, y, z, lx, ly, lz, id, meta, rotation);
	};

	for (int32_t lx = 1; lx <= 5; ++lx)
		for (int32_t lz = 1; lz <= 7; ++lz)
			setRot(lx, 1, lz, sprucePlanks, sprucePlanksMeta);

	for (int32_t lx = 1; lx <= 5; ++lx)
		for (int32_t lz = 2; lz <= 7; ++lz)
			setRot(lx, 4, lz, sprucePlanks, sprucePlanksMeta);

	for (int32_t ly = 2; ly <= 3; ++ly)
		for (int32_t lz = 3; lz <= 6; ++lz) {
			setRot(1, ly, lz, sprucePlanks, sprucePlanksMeta);
			setRot(5, ly, lz, sprucePlanks, sprucePlanksMeta);
		}

	for (int32_t ly = 2; ly <= 3; ++ly)
		for (int32_t lx = 2; lx <= 4; ++lx)
			setRot(lx, ly, 7, sprucePlanks, sprucePlanksMeta);

	for (int32_t ly = 2; ly <= 3; ++ly)
		for (int32_t lz = 1; lz <= 7; ++lz)
			for (int32_t lx = 1; lx <= 5; ++lx) {
				bool wall = (lx==1||lx==5||lz==1||lz==7||lz==2);
				if (!wall) setRot(lx, ly, lz, 0, 0);
			}
	for (int32_t ly = 2; ly <= 3; ++ly) {
		for (int32_t lx = 1; lx <= 5; ++lx) {
			setRot(lx, ly, 1, 0, 0);
		}
		setRot(2, ly, 2, 0, 0);
		setRot(3, ly, 2, sprucePlanks, sprucePlanksMeta);
		setRot(4, ly, 2, sprucePlanks, sprucePlanksMeta);
	}

	for (int32_t ly = 0; ly <= 3; ++ly) {
		setRot(1, ly, 2, oakLog, oakLogMeta);
		setRot(5, ly, 2, oakLog, oakLogMeta);
		setRot(1, ly, 7, oakLog, oakLogMeta);
		setRot(5, ly, 7, oakLog, oakLogMeta);
	}

	setRot(4, 3, 2, oakFence, 0);
	setRot(3, 3, 7, oakFence, 0);

	setRot(1, 3, 4, 0, 0);

	if (Tile::flowerPot) {
		int32_t flowers[] = {1, 2, 3, 13, 14, 15};
		int32_t f = flowers[random->genrand_int32() % 6];
		setRot(4, 2, 6, Tile::flowerPot->blockID, f);
	}
	if (Tile::mushroom2) setRot(1, 3, 5, Tile::mushroom2->blockID, 0);
	if (Tile::workBench) setRot(3, 2, 6, Tile::workBench->blockID, 0);

	setRot(1, 2, 1, oakFence, 0);
	setRot(5, 2, 1, oakFence, 0);

	for (int32_t lx = 0; lx <= 6; ++lx) {
		setRot(lx, 4, 1, spruceStairs, 2);
		setRot(lx, 4, 8, spruceStairs, 3);
	}
	for (int32_t lz = 2; lz <= 7; ++lz) {
		setRot(0, 4, lz, spruceStairs, 0);
		setRot(6, 4, lz, spruceStairs, 1);
	}

	for (int32_t i = 2; i <= 7; i += 5) {
		for (int32_t j = 1; j <= 5; j += 4) {
			int32_t rx = j, rz = i;
			if (rotation == 1) { rx = 6 - i; rz = j; }
			else if (rotation == 2) { rx = 6 - j; rz = 8 - i; }
			else if (rotation == 3) { rx = i; rz = 8 - j; }
			int32_t wx = x + rx, wz = z + rz;
			for (int32_t py = y - 1; py >= y - 32; --py) {
				if (py <= 0) break;
				this->placeBlock(level, wx, py, wz, oakLog, oakLogMeta);
				int32_t below = level->getTile(wx, py - 1, wz);
				Tile* bt = (below >= 0 && below < 256) ? Tile::tiles[below] : nullptr;
				if (bt && bt->isSolidRender()) break;
			}
		}
	}

	level->updateLight(LightLayer::Sky, x - 2, y - 2, z - 2, x + 10, y + 8, z + 10, 1);
	return 1;
}
