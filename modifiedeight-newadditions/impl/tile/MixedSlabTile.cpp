#include <tile/MixedSlabTile.hpp>
#include <tile/entity/MixedSlabTileEntity.hpp>
#include <level/Level.hpp>
#include <entity/Player.hpp>
#include <entity/LocalPlayer.hpp>
#include <item/ItemInstance.hpp>
#include <input/Keyboard.hpp>
#include <tile/material/Material.hpp>
#include <tile/SoulSandTile.hpp>
#include <entity/Entity.hpp>
#include <level/LightLayer.hpp>
#include <tile/CustomSlabTile.hpp>
#include <tile/CustomSlabTile.hpp>
#include <math/HitResult.hpp>

MixedSlabTile::MixedSlabTile(int32_t id)
	: EntityTile(id, "stoneSlab", Material::stone) {
	this->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	this->setDestroyTime(2.0f);
	this->setExplodeable(10.0f);
	this->setSoundType(Tile::SOUND_STONE);
}

MixedSlabTile::~MixedSlabTile() {
}

TileEntity* MixedSlabTile::newTileEntity() {
	return new MixedSlabTileEntity();
}

int32_t MixedSlabTile::getTileEntityType() {
	return 5;
}

int32_t MixedSlabTile::getRenderShape() {
	return 40;
}

static bool_t isSneakingForPlacement(Player* player) {
	if (Keyboard::_states[10] == 1) return 1;
	if (player && player->isLocalPlayer()) return ((LocalPlayer*)player)->isSneaking();
	return 0;
}

bool_t MixedSlabTile::tryPlaceVerticalAdjacent(Level* level, Player* player, ItemInstance* item, int32_t halfId, int32_t x, int32_t y, int32_t z, int32_t face) {
	if (!level || !item || item->count == 0 || !Tile::mixedSlab) return 0;
	if (face < 0 || face > 5) return 0;
	if (!isSneakingForPlacement(player)) return 0;
	if (level->getTile(x, y, z) != Tile::mixedSlab->blockID) return 0;

	MixedSlabTileEntity* te = (MixedSlabTileEntity*)level->getTileEntity(x, y, z);
	if (!te || (te->mode != 1 && te->mode != 2)) return 0;

	bool_t hasBottom = te->bottomTileId > 0;
	bool_t hasTop = te->topTileId > 0;
	if (hasBottom == hasTop) return 0;

	int32_t targetX = x;
	int32_t targetY = y;
	int32_t targetZ = z;
	switch (face) {
		case 0: --targetY; break;
		case 1: ++targetY; break;
		case 2: --targetZ; break;
		case 3: ++targetZ; break;
		case 4: --targetX; break;
		case 5: ++targetX; break;
	}

	if (!level->mayPlace(Tile::mixedSlab->blockID, targetX, targetY, targetZ, 0, face)) return 0;
	if (!level->setTileAndData(targetX, targetY, targetZ, Tile::mixedSlab->blockID, 0, 3)) return 0;

	MixedSlabTileEntity* nte = (MixedSlabTileEntity*)level->getTileEntity(targetX, targetY, targetZ);
	if (!nte) return 0;

	nte->mode = te->mode;
	if (hasTop) {
		nte->topTileId = halfId;
		nte->topAux = item->getAuxValue() & 7;
	} else {
		nte->bottomTileId = halfId;
		nte->bottomAux = item->getAuxValue() & 7;
	}
	level->sendTileUpdated(targetX, targetY, targetZ);

	Tile* placedTile = Tile::tiles[halfId];
	if (placedTile && placedTile->soundType) {
		level->playSound((float)targetX + 0.5f, (float)targetY + 0.5f, (float)targetZ + 0.5f, placedTile->soundType->field_C, (float)(placedTile->soundType->field_0 + 1.0f) * 0.5f, placedTile->soundType->field_4 * 0.8f);
	}
	if (!player || !player->abilities.instabuild) --item->count;
	return 1;
}

void MixedSlabTile::updateShape(LevelSource* level, int32_t x, int32_t y, int32_t z) {
	MixedSlabTileEntity* te = (MixedSlabTileEntity*)level->getTileEntity(x, y, z);
	if (!te) {
		this->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		return;
	}
	bool hasB = (te->bottomTileId > 0);
	bool hasT = (te->topTileId > 0);
	if (hasB && hasT) {
		this->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		return;
	}
	if (te->mode == 1) {
		if (hasB) {
			this->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
		} else if (hasT) {
			this->setShape(0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f);
		}
	} else if (te->mode == 2) {
		if (hasB) {
			this->setShape(0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f);
		} else if (hasT) {
			this->setShape(0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		}
	} else {
		if (hasB) {
			this->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
		} else if (hasT) {
			this->setShape(0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}

void MixedSlabTile::addAABBs(Level* level, int32_t x, int32_t y, int32_t z, const AABB* a6, std::vector<AABB>& a7) {
	this->updateShape(level, x, y, z);
	Tile::addAABBs(level, x, y, z, a6, a7);
}

bool_t MixedSlabTile::isSolidRender() {
	return 0;
}

bool_t MixedSlabTile::isCubeShaped() {
	return 0;
}

void MixedSlabTile::entityInside(Level* level, int32_t x, int32_t y, int32_t z, Entity* entity) {
	if (!entity) {
		return;
	}
	MixedSlabTileEntity* te = (MixedSlabTileEntity*)level->getTileEntity(x, y, z);
	if (!te) {
		return;
	}
	if ((Tile::soulSand && (te->bottomTileId == Tile::soulSand->blockID || (Tile::soulSandSlabHalf && te->bottomTileId == Tile::soulSandSlabHalf->blockID))) ||
	    (Tile::soulSand && (te->topTileId == Tile::soulSand->blockID || (Tile::soulSandSlabHalf && te->topTileId == Tile::soulSandSlabHalf->blockID)))) {
		entity->motionX *= 0.4f;
		entity->motionZ *= 0.4f;
	}
}

int32_t MixedSlabTile::getResource(int32_t meta, Random* random) {
	return 0;
}

int32_t MixedSlabTile::getResourceCount(Random* random) {
	return 0;
}

static int32_t fullBlockResourceForSlabTile(int32_t slabId, int32_t aux) {
	if (slabId <= 0 || slabId >= 256) return slabId;
	Tile* component = Tile::tiles[slabId];
	if (!component) return slabId;
	CustomSlabTile* custom = dynamic_cast<CustomSlabTile*>(component);
	if (custom && custom->isFull) return slabId;
	if (custom && custom->partnerSlabId > 0) return custom->partnerSlabId;
	if (slabId == Tile::stoneSlabHalf->blockID) return Tile::stoneSlab->blockID;
	if (slabId == Tile::woodSlabHalf->blockID) return Tile::woodSlab->blockID;
	return slabId;
}

void MixedSlabTile::playerDestroy(Level* level, Player* player, int32_t x, int32_t y, int32_t z, int32_t meta) {
	MixedSlabTileEntity* te = (MixedSlabTileEntity*)level->getTileEntity(x, y, z);
	if (!te) {
		return;
	}

	int32_t mode = te->mode;
	int32_t bTileId = te->bottomTileId;
	int32_t bAux = te->bottomAux;
	int32_t tTileId = te->topTileId;
	int32_t tAux = te->topAux;

	float hitCoord = 0.5f;
	if (player) {
		Vec3 eyePos = player->getPos(1.0f);
		eyePos.y += player->getHeadHeight();
		Vec3 viewVec = player->getViewVector(1.0f);
		Vec3 reachEnd(eyePos.x + viewVec.x * 7.0f, eyePos.y + viewVec.y * 7.0f, eyePos.z + viewVec.z * 7.0f);
		AABB fullBox = {(float)x, (float)y, (float)z, (float)x + 1.0f, (float)y + 1.0f, (float)z + 1.0f};
		HitResult hit = fullBox.clip(eyePos, reachEnd);
		if (hit.hitType == 0) {
			if (mode == 1) hitCoord = hit.hitVec.z - (float)z;
			else if (mode == 2) hitCoord = hit.hitVec.x - (float)x;
			else hitCoord = hit.hitVec.y - (float)y;
		}
	}

	bool hitTop = (hitCoord >= 0.5f);
	if (bTileId > 0 && tTileId == 0) hitTop = false;
	if (tTileId > 0 && bTileId == 0) hitTop = true;

	if (hitTop) {
		Tile* topTile = (tTileId > 0 && tTileId < 256) ? Tile::tiles[tTileId] : nullptr;
		int32_t dropId = topTile ? (bTileId > 0 ? (tTileId == Tile::redstoneLampSlabHalfOn->blockID ? Tile::redstoneLampSlabHalfOn->blockID : (tTileId == Tile::redstoneLampSlabOn->blockID ? Tile::redstoneLampSlabOn->blockID : topTile->getResource(tAux, &level->random))) : (fullBlockResourceForSlabTile(tTileId, tAux))) : fullBlockResourceForSlabTile(tTileId, tAux);
		int32_t dropAux = topTile ? topTile->getSpawnResourcesAuxValue(tAux) : tAux;
		if (dropId > 0) {
			this->popResource(level, x, y, z, ItemInstance(dropId, 1, dropAux));
		}
		if (bTileId > 0) {
			te->topTileId = 0;
			te->topAux = 0;
			level->sendTileUpdated(x, y, z);
		} else {
			level->removeTileEntity(x, y, z);
			level->setTile(x, y, z, 0, 3);
		}
	} else {
		Tile* bottomTile = (bTileId > 0 && bTileId < 256) ? Tile::tiles[bTileId] : nullptr;
		int32_t dropId = bottomTile ? (tTileId > 0 ? (bTileId == Tile::redstoneLampSlabHalfOn->blockID ? Tile::redstoneLampSlabHalfOn->blockID : (bTileId == Tile::redstoneLampSlabOn->blockID ? Tile::redstoneLampSlabOn->blockID : bottomTile->getResource(bAux, &level->random))) : (fullBlockResourceForSlabTile(bTileId, bAux))) : fullBlockResourceForSlabTile(bTileId, bAux);
		int32_t dropAux = bottomTile ? bottomTile->getSpawnResourcesAuxValue(bAux) : bAux;
		if (dropId > 0) {
			this->popResource(level, x, y, z, ItemInstance(dropId, 1, dropAux));
		}
		if (tTileId > 0) {
			te->bottomTileId = 0;
			te->bottomAux = 0;
			level->sendTileUpdated(x, y, z);
		} else {
			level->removeTileEntity(x, y, z);
			level->setTile(x, y, z, 0, 3);
		}
	}
}

void MixedSlabTile::playerWillDestroy(Level* level, int32_t x, int32_t y, int32_t z, int32_t meta, Player* player) {
	MixedSlabTileEntity* te = (MixedSlabTileEntity*)level->getTileEntity(x, y, z);
	if (!te) return;
	if (player && player->abilities.instabuild) return;

	int32_t remTileId = (te->bottomTileId > 0) ? te->bottomTileId : te->topTileId;
	int32_t remAux = (te->bottomTileId > 0) ? te->bottomAux : te->topAux;
	if (remTileId > 0) {
		Tile* remTile = (remTileId < 256) ? Tile::tiles[remTileId] : nullptr;
		int32_t dropId = remTile ? remTile->getResource(remAux, &level->random) : remTileId;
		int32_t dropAux = remTile ? remTile->getSpawnResourcesAuxValue(remAux) : remAux;
		if (dropId > 0) {
			this->popResource(level, x, y, z, ItemInstance(dropId, 1, dropAux));
		}
	}
}


void MixedSlabTile::neighborChanged(Level* level, int32_t x, int32_t y, int32_t z, int32_t ox, int32_t oy, int32_t oz, int32_t tileId) {
	if (!level || level->isClientMaybe) return;
	MixedSlabTileEntity* te = (MixedSlabTileEntity*)level->getTileEntity(x, y, z);
	if (!te) return;
	bool hasSignal = level->hasNeighborSignal(x, y, z);
	int32_t ids[2] = {te->bottomTileId, te->topTileId};
	int32_t auxs[2] = {te->bottomAux, te->topAux};
	bool changed = false;
	for (int i = 0; i < 2; i++) {
		int32_t componentId = ids[i];
		Tile* component = (componentId > 0 && componentId < 256) ? Tile::tiles[componentId] : nullptr;
		CustomSlabTile* slab = component ? dynamic_cast<CustomSlabTile*>(component) : nullptr;
		if (!slab || slab->slabType != 7) continue;
		bool isOn = (Tile::redstoneLampSlabHalfOn && componentId == Tile::redstoneLampSlabHalfOn->blockID) || (Tile::redstoneLampSlabOn && componentId == Tile::redstoneLampSlabOn->blockID);
		if (hasSignal && !isOn && slab->lampOnTileId > 0 && slab->lampOnTileId != slab->blockID) {
			ids[i] = slab->lampOnTileId;
			changed = true;
		} else if (!hasSignal && isOn) {
			if (Tile::redstoneLampSlabHalf && componentId == Tile::redstoneLampSlabHalfOn->blockID) ids[i] = Tile::redstoneLampSlabHalf->blockID;
			else if (Tile::redstoneLampSlab && componentId == Tile::redstoneLampSlabOn->blockID) ids[i] = Tile::redstoneLampSlab->blockID;
			changed = true;
		}
	}
	if (changed) {
		te->bottomTileId = ids[0];
		te->topTileId = ids[1];
		te->bottomAux = auxs[0];
		te->topAux = auxs[1];
		level->sendTileUpdated(x, y, z);
		level->updateLight(LightLayer::Block, x - 15, y - 15, z - 15, x + 15, y + 15, z + 15);
	}
}

void MixedSlabTile::onPlace(Level* level, int32_t x, int32_t y, int32_t z) {
	this->neighborChanged(level, x, y, z, x, y, z, 0);
}
