#include <level/gen/feature/MineshaftFeature.hpp>
#include <level/Level.hpp>
#include <level/LightLayer.hpp>
#include <tile/Tile.hpp>
#include <tile/entity/ChestTileEntity.hpp>
#include <item/Item.hpp>
#include <item/ItemInstance.hpp>
#include <util/Random.hpp>
#include <algorithm>

MineshaftFeature::MineshaftFeature(bool_t update) : Feature(update) {}
MineshaftFeature::~MineshaftFeature() {}

static int32_t minI(int32_t a, int32_t b) { return a < b ? a : b; }
static int32_t maxI(int32_t a, int32_t b) { return a > b ? a : b; }

void MineshaftFeature::addBox(const MineshaftBox& b) { occupied.push_back(b); }
bool_t MineshaftFeature::intersects(const MineshaftBox& b) {
	for (const auto& o : occupied) {
		if (b.x1 < o.x0 || b.x0 > o.x1) continue;
		if (b.y1 < o.y0 || b.y0 > o.y1) continue;
		if (b.z1 < o.z0 || b.z0 > o.z1) continue;
		return 1;
	}
	return 0;
}
bool_t MineshaftFeature::canPlace(const MineshaftBox& b) { return !intersects(b); }

void MineshaftFeature::carve(Level* level, int32_t x, int32_t y, int32_t z) {
	if (y <= 1 || y >= 126) return;
	int32_t t = level->getTile(x, y, z);
	if (t == Tile::unbreakable->blockID) return;
	this->placeBlock(level, x, y, z, 0, 0);
}
void MineshaftFeature::fillAir(Level* level, int32_t x0, int32_t y0, int32_t z0, int32_t x1, int32_t y1, int32_t z1) {
	for (int32_t x = x0; x <= x1; ++x)
		for (int32_t y = y0; y <= y1; ++y)
			for (int32_t z = z0; z <= z1; ++z)
				carve(level, x, y, z);
}

void MineshaftFeature::placeFloor(Level* level, Random* random, int32_t x, int32_t y, int32_t z) {
	if (y <= 1) return;
	int32_t below = level->getTile(x, y - 1, z);
	if (below == 0 || (Tile::tiles[below] && !Tile::tiles[below]->isSolidRender())) {
		int32_t plank = Tile::wood ? Tile::wood->blockID : 5;
		this->placeBlock(level, x, y - 1, z, plank, 0);
	}
}

void MineshaftFeature::placeSupport(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir) {
	int32_t plank = Tile::wood ? Tile::wood->blockID : 5;
	int32_t fence = Tile::fence ? Tile::fence->blockID : 85;
	int32_t lx = x, lz = z, rx = x, rz = z;
	if (dir == 0 || dir == 2) { lx = x - 1; rx = x + 1; }
	else { lz = z - 1; rz = z + 1; }
	bool damaged = (random->genrand_int32() % 6) == 0;
	for (int32_t dy = 0; dy <= 2; ++dy) {
		if (!(damaged && dy == 1 && (random->genrand_int32() % 2) == 0)) this->placeBlock(level, lx, y + dy, lz, fence, 0);
		if (!(damaged && dy == 2 && (random->genrand_int32() % 2) == 0)) this->placeBlock(level, rx, y + dy, rz, fence, 0);
	}
	for (int32_t w = -1; w <= 1; ++w) {
		int32_t bx = x, bz = z;
		if (dir == 0 || dir == 2) bx = x + w; else bz = z + w;
		if (damaged && w == 0 && (random->genrand_int32() % 2) == 0) continue;
		this->placeBlock(level, bx, y + 3, bz, plank, 0);
	}
	if (damaged && Tile::web && (random->genrand_int32() % 2) == 0) {
		this->placeBlock(level, x, y + 2, z, Tile::web->blockID, 0);
	}
}

void MineshaftFeature::placeChestWithLoot(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t data) {
	if (!Tile::chest) return;
	level->setTileAndData(x, y, z, Tile::chest->blockID, data, 3);
	ChestTileEntity* te = (ChestTileEntity*)level->getTileEntity(x, y, z);
	if (!te) {
		te = new ChestTileEntity();
		level->setTileEntity(x, y, z, te);
	}
	if (!te) return;
	int32_t numItems = 3 + (random->genrand_int32() % 5);
	for (int32_t i = 0; i < numItems; ++i) {
		int32_t slot = random->genrand_int32() % 27;
		int32_t r = random->genrand_int32() % 24;
		ItemInstance inst(Item::coal, 3 + (random->genrand_int32() % 6), 0);
		if (r <= 2) inst = ItemInstance(Item::ironIngot, 1 + (random->genrand_int32() % 5), 0);
		else if (r <= 4) inst = ItemInstance(Item::goldIngot, 1 + (random->genrand_int32() % 3), 0);
		else if (r == 5 && Item::redStone) inst = ItemInstance(Item::redStone, 4 + (random->genrand_int32() % 5), 0);
		else if (r <= 7) inst = ItemInstance(Item::bread, 1 + (random->genrand_int32() % 3), 0);
		else if (r <= 9 && Item::seeds_wheat) inst = ItemInstance(Item::seeds_wheat, 2 + (random->genrand_int32() % 4), 0);
		else if (r <= 11 && Tile::rail) inst = ItemInstance(Tile::rail->blockID, 4 + (random->genrand_int32() % 5), 0);
		else if (r == 12 && Item::pickAxe_iron) inst = ItemInstance(Item::pickAxe_iron, 1, 0);
		else if (r == 13 && Item::apple) inst = ItemInstance(Item::apple, 1, 0);
		else if (r == 14 && Item::emerald) inst = ItemInstance(Item::emerald, 1, 0);
		te->setItem(slot, &inst);
	}
}

static void stepDir(int32_t dir, int32_t step, int32_t& x, int32_t& z) {
	if (dir == 0) z -= step;
	else if (dir == 1) x += step;
	else if (dir == 2) z += step;
	else x -= step;
}

void MineshaftFeature::placeRoom(Level* level, Random* random, int32_t x, int32_t y, int32_t z) {
	MineshaftBox box{x - 5, y - 1, z - 5, x + 5, y + 5, z + 5};
	addBox(box);
	int32_t plank = Tile::wood ? Tile::wood->blockID : 5;
	int32_t fence = Tile::fence ? Tile::fence->blockID : 85;
	fillAir(level, x - 4, y, z - 4, x + 4, y + 3, z + 4);
	for (int32_t dx = -4; dx <= 4; ++dx) {
		for (int32_t dz = -4; dz <= 4; ++dz) {
			placeFloor(level, random, x + dx, y, z + dz);
			if ((abs(dx) == 4 || abs(dz) == 4) && (random->genrand_int32() % 3) != 0) {
				this->placeBlock(level, x + dx, y + 4, z + dz, plank, 0);
			}
		}
	}
	for (int32_t dy = 0; dy <= 3; ++dy) {
		this->placeBlock(level, x - 4, y + dy, z - 4, fence, 0);
		this->placeBlock(level, x + 4, y + dy, z - 4, fence, 0);
		this->placeBlock(level, x - 4, y + dy, z + 4, fence, 0);
		this->placeBlock(level, x + 4, y + dy, z + 4, fence, 0);
	}
	if (Tile::torch) this->placeBlock(level, x, y + 2, z, Tile::torch->blockID, 5);
}

void MineshaftFeature::placeCorridorPiece(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir, int32_t depth) {
	int32_t length = 5 + (random->genrand_int32() % 10);
	int32_t ex = x, ez = z; stepDir(dir, length - 1, ex, ez);
	MineshaftBox box{minI(x,ex) - 2, y - 1, minI(z,ez) - 2, maxI(x,ex) + 2, y + 4, maxI(z,ez) + 2};
	if (!canPlace(box)) return;
	addBox(box);

	int32_t plank = Tile::wood ? Tile::wood->blockID : 5;
	bool spider = (depth >= 2 && (random->genrand_int32() % 100) < 6);
	for (int32_t i = 0; i < length; ++i) {
		int32_t cx = x, cz = z; stepDir(dir, i, cx, cz);
		for (int32_t w = -1; w <= 1; ++w) {
			int32_t bx = cx, bz = cz;
			if (dir == 0 || dir == 2) bx = cx + w; else bz = cz + w;
			for (int32_t dy = 0; dy <= 2; ++dy) carve(level, bx, y + dy, bz);
			placeFloor(level, random, bx, y, bz);
		}
		if (i % 5 == 0 && (random->genrand_int32() % 6) != 0) {
			placeSupport(level, random, cx, y, cz, dir);
			if (Tile::torch && (random->genrand_int32() % 3) == 0) {
				int32_t tx = cx, tz = cz;
				if (dir == 0 || dir == 2) tx = cx - 1; else tz = cz - 1;
				this->placeBlock(level, tx, y + 2, tz, Tile::torch->blockID, 5);
			}
		}
		if (Tile::rail && (random->genrand_int32() % 100) < 70) {
			if (!spider || (random->genrand_int32() % 2) == 0)
				this->placeBlock(level, cx, y, cz, Tile::rail->blockID, (dir == 0 || dir == 2) ? 0 : 1);
		}
		if (Tile::web) {
			int32_t webChance = spider ? 45 : 10;
			if ((random->genrand_int32() % 100) < webChance) {
				int32_t wx = cx, wz = cz;
				if ((random->genrand_int32() % 2) == 0) { if (dir == 0 || dir == 2) wx = cx + ((random->genrand_int32()%2)?1:-1); else wz = cz + ((random->genrand_int32()%2)?1:-1); }
				int32_t wy = (random->genrand_int32() % 2) ? y + 2 : y + 1;
				this->placeBlock(level, wx, wy, wz, Tile::web->blockID, 0);
			}
		}
		if (!spider && i >= 3 && i < length - 2 && (i % 7 == 0) && (random->genrand_int32() % 100) < 20) {
			bool_t left = (random->genrand_int32() % 2) == 0;
			int32_t sx = cx, sz = cz;
			if (dir == 0 || dir == 2) sx = cx + (left ? -1 : 1);
			else sz = cz + (left ? -1 : 1);
			int32_t data = 0;
			if (dir == 0) data = left ? 5 : 4;
			else if (dir == 1) data = left ? 3 : 2;
			else if (dir == 2) data = left ? 4 : 5;
			else data = left ? 2 : 3;
			placeChestWithLoot(level, random, sx, y, sz, data);
		}
	}
	if (spider) {
		int32_t mx = x, mz = z; stepDir(dir, length / 2, mx, mz);
		for (int32_t dx = -1; dx <= 1; ++dx) for (int32_t dz = -1; dz <= 1; ++dz) for (int32_t dy = 0; dy <= 2; ++dy)
			if (Tile::web && (random->genrand_int32() % 100) < 70) this->placeBlock(level, mx + dx, y + dy, mz + dz, Tile::web->blockID, 0);
	}

	int32_t nx = x, nz = z; stepDir(dir, length, nx, nz);
	if (depth < 7) {
		int32_t roll = random->genrand_int32() % 100;
		if (roll < 55) {
			tryExpand(level, random, nx, y, nz, dir, depth + 1);
		} else if (roll < 70) {
			tryExpand(level, random, nx, y, nz, (dir + 1) % 4, depth + 1);
		} else if (roll < 85) {
			tryExpand(level, random, nx, y, nz, (dir + 3) % 4, depth + 1);
		} else if (roll < 93) {
			placeCrossingPiece(level, random, nx, y, nz, dir, depth + 1);
		} else {
			placeStairsPiece(level, random, nx, y, nz, dir, depth + 1);
		}
		if ((random->genrand_int32() % 100) < 18) {
			int32_t bx = x, bz = z; stepDir(dir, 3 + (random->genrand_int32() % (length > 4 ? length - 4 : 1)), bx, bz);
			int32_t bdir = (random->genrand_int32() % 2) ? (dir + 1) % 4 : (dir + 3) % 4;
			int32_t sx = bx, sz = bz; stepDir(bdir, 1, sx, sz);
			tryExpand(level, random, sx, y, sz, bdir, depth + 1);
		}
	}
}

void MineshaftFeature::placeStairsPiece(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir, int32_t depth) {
	int32_t steps = 4 + (random->genrand_int32() % 5);
	int32_t ex = x, ez = z; stepDir(dir, steps - 1, ex, ez);
	int32_t ey = y - (steps - 1);
	MineshaftBox box{minI(x,ex) - 2, ey - 1, minI(z,ez) - 2, maxI(x,ex) + 2, y + 4, maxI(z,ez) + 2};
	if (!canPlace(box) || ey < 8) return;
	addBox(box);
	for (int32_t i = 0; i < steps; ++i) {
		int32_t cx = x, cz = z; stepDir(dir, i, cx, cz);
		int32_t cy = y - i;
		for (int32_t w = -1; w <= 1; ++w) {
			int32_t bx = cx, bz = cz;
			if (dir == 0 || dir == 2) bx = cx + w; else bz = cz + w;
			for (int32_t dy = 0; dy <= 2; ++dy) carve(level, bx, cy + dy, bz);
			int32_t below = level->getTile(bx, cy - 1, bz);
			if (below == 0 || (Tile::tiles[below] && !Tile::tiles[below]->isSolidRender())) {
				this->placeBlock(level, bx, cy - 1, bz, Tile::rock ? Tile::rock->blockID : 1, 0);
			}
		}
		if (i % 3 == 0) placeSupport(level, random, cx, cy, cz, dir);
	}
	int32_t nx = x, nz = z; stepDir(dir, steps, nx, nz);
	tryExpand(level, random, nx, ey, nz, dir, depth + 1);
}

void MineshaftFeature::placeCrossingPiece(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir, int32_t depth) {
	MineshaftBox box{x - 2, y - 1, z - 2, x + 2, y + 4, z + 2};
	if (!canPlace(box)) return;
	addBox(box);
	int32_t plank = Tile::wood ? Tile::wood->blockID : 5;
	fillAir(level, x - 1, y, z - 1, x + 1, y + 2, z + 1);
	for (int32_t dx = -1; dx <= 1; ++dx) for (int32_t dz = -1; dz <= 1; ++dz) {
		placeFloor(level, random, x + dx, y, z + dz);
		this->placeBlock(level, x + dx, y + 3, z + dz, plank, 0);
	}
	if (Tile::torch) this->placeBlock(level, x, y + 2, z, Tile::torch->blockID, 5);
	for (int32_t d = 0; d < 4; ++d) {
		if (depth < 7 && (random->genrand_int32() % 100) < 75) {
			int32_t nx = x, nz = z; stepDir(d, 2, nx, nz);
			tryExpand(level, random, nx, y, nz, d, depth + 1);
		}
	}
}

void MineshaftFeature::tryExpand(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir, int32_t depth) {
	if (depth > 7 || y < 10 || y > 50) return;
	if ((random->genrand_int32() % 100) < 8) return;
	int32_t r = random->genrand_int32() % 100;
	if (r < 70) placeCorridorPiece(level, random, x, y, z, dir, depth);
	else if (r < 82) placeStairsPiece(level, random, x, y, z, dir, depth);
	else placeCrossingPiece(level, random, x, y, z, dir, depth);
}

bool_t MineshaftFeature::place(Level* level, Random* random, int32_t x, int32_t y, int32_t z) {
	if (!level) return 0;
	occupied.clear();
	int32_t baseY = 18 + (int32_t)(random->genrand_int32() % 22);
	if (baseY < 12) baseY = 12;
	if (baseY > 40) baseY = 40;
	placeRoom(level, random, x, baseY, z);

	int32_t exits = 1 + (random->genrand_int32() % 4);
	bool used[4] = {0,0,0,0};
	for (int32_t i = 0; i < exits; ++i) {
		int32_t dir = random->genrand_int32() % 4;
		int tries = 0;
		while (used[dir] && tries++ < 4) dir = (dir + 1) % 4;
		used[dir] = 1;
		int32_t sx = x, sz = z; stepDir(dir, 5, sx, sz);
		tryExpand(level, random, sx, baseY, sz, dir, 0);
	}
	level->updateLight(LightLayer::Sky, x - 64, baseY - 8, z - 64, x + 64, baseY + 16, z + 64, 1);
	level->updateLight(LightLayer::Block, x - 64, baseY - 8, z - 64, x + 64, baseY + 16, z + 64, 1);
	return 1;
}
