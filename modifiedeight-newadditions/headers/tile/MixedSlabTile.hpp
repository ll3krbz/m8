#pragma once
#include <tile/EntityTile.hpp>

struct ItemInstance;

class MixedSlabTile : public EntityTile {
public:
	static bool_t tryPlaceVerticalAdjacent(Level*, Player*, ItemInstance*, int32_t, int32_t, int32_t, int32_t, int32_t);

	MixedSlabTile(int32_t id);
	virtual ~MixedSlabTile();
	virtual TileEntity* newTileEntity();
	virtual int32_t getTileEntityType();
	virtual int32_t getRenderShape();
	virtual void updateShape(LevelSource*, int32_t, int32_t, int32_t);
	virtual void addAABBs(Level*, int32_t, int32_t, int32_t, const AABB*, std::vector<AABB>&);
	virtual void playerDestroy(Level* level, Player* player, int32_t x, int32_t y, int32_t z, int32_t meta);
	virtual void playerWillDestroy(Level* level, int32_t x, int32_t y, int32_t z, int32_t meta, Player* player);
	virtual void entityInside(Level* level, int32_t x, int32_t y, int32_t z, Entity* entity);
	virtual void neighborChanged(Level* level, int32_t x, int32_t y, int32_t z, int32_t ox, int32_t oy, int32_t oz, int32_t tileId);
	virtual void onPlace(Level* level, int32_t x, int32_t y, int32_t z);

	virtual int32_t getResource(int32_t meta, Random* random);
	virtual int32_t getResourceCount(Random* random);
	virtual bool_t isSolidRender();
	virtual bool_t isCubeShaped();
};
