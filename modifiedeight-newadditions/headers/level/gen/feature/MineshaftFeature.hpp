#pragma once

#include <level/gen/feature/Feature.hpp>
#include <vector>

struct MineshaftBox {
	int32_t x0, y0, z0;
	int32_t x1, y1, z1;
};

class MineshaftFeature : public Feature {
public:
	MineshaftFeature(bool_t update = 0);
	virtual ~MineshaftFeature();

	virtual bool_t place(Level* level, Random* random, int32_t x, int32_t y, int32_t z);

private:
	struct Piece {
		int32_t type;
		int32_t x, y, z;
		int32_t dir;
		int32_t length;
		MineshaftBox box;
	};
	std::vector<MineshaftBox> occupied;
	void addBox(const MineshaftBox& b);
	bool_t intersects(const MineshaftBox& b);
	bool_t canPlace(const MineshaftBox& b);
	void carve(Level* level, int32_t x, int32_t y, int32_t z);
	void fillAir(Level* level, int32_t x0, int32_t y0, int32_t z0, int32_t x1, int32_t y1, int32_t z1);
	void placeFloor(Level* level, Random* random, int32_t x, int32_t y, int32_t z);
	void placeSupport(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir);
	void placeCorridorPiece(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir, int32_t depth);
	void placeStairsPiece(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir, int32_t depth);
	void placeCrossingPiece(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir, int32_t depth);
	void placeRoom(Level* level, Random* random, int32_t x, int32_t y, int32_t z);
	void tryExpand(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t dir, int32_t depth);
	void placeChestWithLoot(Level* level, Random* random, int32_t x, int32_t y, int32_t z, int32_t data);
};
