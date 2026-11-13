#include <tile/SlimeBlockTile.hpp>
#include <tile/material/Material.hpp>
#include <entity/Entity.hpp>
#include <entity/Mob.hpp>
#include <entity/Player.hpp>
#include <entity/player/Abilities.hpp>
#include <level/Level.hpp>
#include <cmath>

SlimeBlockTile::SlimeBlockTile(int32_t id, const std::string& texName)
	: Tile(id, Material::cloth) {
	this->tex = *this->getTextureItem(texName);
	this->slipperiness = 0.8f;
}

SlimeBlockTile::~SlimeBlockTile() {
}

TextureUVCoordinateSet* SlimeBlockTile::getTexture(int32_t side, int32_t data) {
	TextureAtlasTextureItem* ti = this->getTextureItem("slime");
	if (ti) return ti->getUV(0);
	ti = this->getTextureItem("slime_block");
	if (ti) return ti->getUV(0);
	return this->tex.getUV(0);
}

TextureUVCoordinateSet* SlimeBlockTile::getCarriedTexture(int32_t side, int32_t data) {
	return this->getTexture(side, data);
}

void SlimeBlockTile::fallOn(Level* level, int32_t x, int32_t y, int32_t z, Entity* entity, float damage) {
	if (!entity) {
		return;
	}
	entity->fallDistance = 0.0f;
	if (entity->isSneaking() || entity->isInWater() || entity->isInLava() || entity->ridingAt) {
		if (entity->motionY < -0.25f && level) {
			level->playSound(entity, "mob.slime.small", 0.4f, 0.9f);
		}
		return;
	}
	if (entity->isPlayer() && ((Player*)entity)->abilities.flying) {
		return;
	}
	float impactSpeed = (entity->motionY < 0.0f) ? -entity->motionY : 0.0f;
	float heightSpeed = (damage > 0.25f) ? sqrtf(2.0f * 0.08f * damage) * 0.78f : 0.0f;
	float effectiveSpeed = (impactSpeed > heightSpeed) ? impactSpeed : heightSpeed;
	bool_t jumping = entity->isMob() && ((Mob*)entity)->isJumping;
	float rebound = jumping ? (effectiveSpeed * 0.93f + 0.06f) : (effectiveSpeed * 0.88f - 0.05f);
	if (rebound >= 0.20f) {
		if (rebound > 3.6f) {
			rebound = 3.6f;
		}
		entity->motionY = rebound / 0.98f + 0.08f;
		if (level) {
			float vol = 0.35f + rebound * 0.4f;
			if (vol > 1.0f) vol = 1.0f;
			float pitch = 0.9f + (Entity::sharedRandom.nextFloat() - Entity::sharedRandom.nextFloat()) * 0.15f;
			level->playSound(entity, "mob.slime.big", vol, pitch);
			for (int32_t i = 0; i < 4; ++i) {
				float px = entity->posX + (Entity::sharedRandom.nextFloat() - 0.5f) * 0.4f;
				float pz = entity->posZ + (Entity::sharedRandom.nextFloat() - 0.5f) * 0.4f;
				float vx = (Entity::sharedRandom.nextFloat() - 0.5f) * 0.12f;
				float vz = (Entity::sharedRandom.nextFloat() - 0.5f) * 0.12f;
				level->addParticle(PT_TERRAIN, px, (float)y + 1.05f, pz, vx, 0.12f, vz, this->blockID);
			}
		}
	}
}

void SlimeBlockTile::stepOn(Level* level, int32_t x, int32_t y, int32_t z, Entity* entity) {
	if (entity && !entity->isSneaking() && entity->motionY <= 0.0f) {
		entity->motionX *= 0.85f;
		entity->motionZ *= 0.85f;
	}
}

int32_t SlimeBlockTile::getRenderLayer() {
	return 1;
}

bool_t SlimeBlockTile::isSolidRender() {
	return 0;
}
