#include <tile/PumpkinTile.hpp>
#include <entity/Mob.hpp>
#include <entity/Snowman.hpp>
#include <level/Level.hpp>
#include <tile/material/Material.hpp>
#include <rendering/TextureAtlasTextureItem.hpp>

PumpkinTile::PumpkinTile(int32_t a2, bool_t lit)
	: DirectionalTile(a2, Material::vegetable) {
	this->field_80 = lit;
	TextureAtlasTextureItem* texItem = this->getTextureItem("pumpkin");
	this->field_84 = *texItem->getUV(0);
	this->field_9C = *texItem->getUV(1);
	this->field_B4 = *texItem->getUV(this->field_80 ? 3 : 2);
}

PumpkinTile::~PumpkinTile() {
}
TextureUVCoordinateSet* PumpkinTile::getTexture(int32_t a2, int32_t a3) {
	bool_t v4; // zf

	if(a2 == 1 || !a2) {
		return &this->field_84;
	}
	if(a3 == 2) {
		v4 = a2 == 2;
	} else if(a3 == 3) {
		v4 = a2 == 5;
	} else if(a3) {
		if(a3 != 1) {
			return &this->field_9C;
		}
		v4 = a2 == 4;
	} else {
		v4 = a2 == 3;
	}
	if(v4) {
		return &this->field_B4;
	}
	return &this->field_9C;
}
bool_t PumpkinTile::mayPlace(Level* level, int32_t x, int32_t y, int32_t z) {
	int32_t v8;				  // r0
	bool_t result;			  // r0
	const Material* material; // r0

	v8 = level->getTile(x, y, z);
	if(!v8) {
		return level->isTopSolidBlocking(x, y - 1, z);
	}
	material = Tile::tiles[v8]->material;
	result = material->isReplaceable();
	if(result) {
		return level->isTopSolidBlocking(x, y - 1, z);
	}
	return result;
}
void PumpkinTile::onPlace(Level* level, int32_t x, int32_t y, int32_t z) {
	if (!level) return;
	if (level->getTile(x, y - 1, z) == Tile::snow->blockID && level->getTile(x, y - 2, z) == Tile::snow->blockID) {
		level->setTile(x, y, z, 0, 2);
		level->setTile(x, y - 1, z, 0, 2);
		level->setTile(x, y - 2, z, 0, 2);

		level->levelEvent(2001, x, y, z, this->blockID);
		level->levelEvent(2001, x, y - 1, z, Tile::snow->blockID);
		level->levelEvent(2001, x, y - 2, z, Tile::snow->blockID);

		if (!level->isClientMaybe) {
			Snowman* golem = new Snowman(level);
			golem->moveTo((float)x + 0.5f, (float)(y - 2), (float)z + 0.5f, 0.0f, 0.0f);
			level->addEntity(golem);
		}
	}
}
int32_t PumpkinTile::getPlacementDataValue(Level*, int32_t, int32_t, int32_t, int32_t, float, float, float, Mob* a10, int32_t) {
	float v11;	 // s15
	int32_t v12; // r0

	v11 = (float)((float)(a10->yaw * 4.0) / 360.0) + 2.5;
	v12 = (int32_t)v11;
	if(v11 < (float)(int32_t)v11) {
		//LOBYTE(v12) = v12 - 1;
		v12 = v12 - 1;
	}
	return v12 & 3;
}
