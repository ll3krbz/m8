#include <entity/Animal.hpp>
#include <entity/Player.hpp>
#include <entity/player/Abilities.hpp>
#include <entity/path/Path.hpp>
#include <inventory/Inventory.hpp>
#include <item/Item.hpp>
#include <level/Level.hpp>
#include <nbt/CompoundTag.hpp>
#include <tile/Tile.hpp>
#include <math/Mth.hpp>
#include <cmath>

Animal::Animal(Level* a2)
	: AgableMob(a2) {
	this->inLove = 0;
}
bool_t Animal::canMate(const Animal* a2) {
	int32_t v4; // r6

	if(a2 != this && (v4 = a2->getEntityTypeId(), v4 == this->getEntityTypeId()) && this->isInLove()) {
		return a2->isInLove();
	} else {
		return 0;
	}
}
bool_t Animal::isInLove() const {
	return this->inLove > 0;
}
void Animal::resetLove() {
	this->inLove = 0;
}

Animal::~Animal() {
}
bool_t Animal::interactWithPlayer(Player* a2) {
	ItemInstance* sel; // r5
	int32_t v5;		   // r6
	float g;		   // s21
	float g2;		   // s20
	float v8;		   // r0
	float posX;		   // s16
	Level* level;	   // r7
	float v11;		   // s19
	float f;		   // r0
	float entityWidth; // s15
	float v14;		   // s18
	float posY;		   // s16
	float ff;		   // r0
	float posZ;		   // s22
	float v18;		   // s16
	float v19;		   // r0

	sel = a2->inventory->getSelected();
	if(!sel || !this->isFood(sel) || this->isBaby() || this->getAge()) {
		return Entity::interactWithPlayer(a2);
	}
	if(!a2->abilities.instabuild) {
		--sel->count;
	}
	v5 = 7;
	this->inLove = 600;
	this->attackTarget = 0;
	do {
		g = this->random.nextGaussian();
		g2 = this->random.nextGaussian();
		v8 = this->random.nextGaussian();
		posX = this->posX;
		level = this->level;
		v11 = v8;
		f = this->random.nextFloat();
		entityWidth = this->entityWidth;
		v14 = posX + (float)((float)(f * entityWidth) + (float)(f * entityWidth));
		posY = this->posY;
		ff = this->random.nextFloat();
		posZ = this->posZ;
		v18 = (float)(posY + 0.5) + (float)(ff * this->entityHeight);
		v19 = this->random.nextFloat();
		level->addParticle(PT_HEART, v14 - entityWidth, v18, (float)(posZ + (float)((float)(v19 * this->entityWidth) + (float)(v19 * this->entityWidth))) - this->entityWidth, g * 0.02, g2 * 0.02, v11 * 0.02, 0);
		--v5;
	} while(v5);
	return 1;
}

bool_t Animal::hurt(Entity* a2, int32_t a3) {
	this->inPanicTicksMaybe = 60;
	this->attackTarget = 0;
	this->inLove = 0;
	return Mob::hurt(a2, a3);
}
int32_t Animal::getCreatureBaseType() {
	return 2;
}
void Animal::readAdditionalSaveData(CompoundTag* a2) {
	AgableMob::readAdditionalSaveData(a2);
	this->inLove = a2->getInt("InLove");
}
void Animal::addAdditonalSaveData(CompoundTag* a2) {
	AgableMob::addAdditonalSaveData(a2);
	a2->putInt("InLove", this->inLove);
}
int32_t Animal::getAmbientSoundInterval() {
	return 240;
}
void Animal::aiStep() {
	int32_t inLove; // r3
	int32_t v3;		// r3
	float g;		// s20
	float v5;		// s19
	float v6;		// r0
	float posX;		// s16
	Level* level;	// r6
	float v9;		// s18
	float f;		// r0
	float v11;		// s17
	float v12;		// s16
	float v13;		// r0
	float posZ;		// s21
	float v15;		// s16
	float v16;		// r0

	AgableMob::aiStep();
	if(this->getAge()) {
		this->inLove = 0;
	}
	inLove = this->inLove;
	if(inLove > 0) {
		v3 = inLove - 1;
		this->inLove = v3;
		if((v3 & 0xF) == 0) {
			g = this->random.nextGaussian();
			v5 = this->random.nextGaussian();
			v6 = this->random.nextGaussian();
			posX = this->posX;
			level = this->level;
			v9 = v6;
			f = this->random.nextFloat();
			v11 = (float)(posX + (float)((float)(f * this->entityWidth) + (float)(f * this->entityWidth))) - this->entityWidth;
			v12 = this->posY + 0.5;
			v13 = this->random.nextFloat();
			posZ = this->posZ;
			v15 = v12 + (float)(v13 * this->entityHeight);
			v16 = this->random.nextFloat();
			level->addParticle(PT_HEART, v11, v15, (float)(posZ + (float)((float)(v16 * this->entityWidth) + (float)(v16 * this->entityWidth))) - this->entityWidth, g * 0.02, v5 * 0.02, v9 * 0.02, 0);
		}
		if (!this->useNewAi() && this->level && !this->level->isClientMaybe) {
			std::vector<Entity*> candidates;
			AABB searchBox{this->boundingBox.minX - 8.0f, this->boundingBox.minY - 8.0f, this->boundingBox.minZ - 8.0f, this->boundingBox.maxX + 8.0f, this->boundingBox.maxY + 8.0f, this->boundingBox.maxZ + 8.0f};
			this->level->getEntitiesOfType(this->getEntityTypeId(), searchBox, candidates);
			Animal* mate = 0;
			float bestDistSq = 64.0f;
			for (size_t i = 0; i < candidates.size(); ++i) {
				Animal* other = (Animal*)candidates[i];
				if (other && this->canMate(other)) {
					float dSq = this->distanceToSqr(other);
					if (dSq < bestDistSq) {
						bestDistSq = dSq;
						mate = other;
					}
				}
			}
			if (mate) {
				this->attackTarget = 0;
				this->lookAt(mate, 30.0f, 30.0f);
				float breedDistSq = (this->entityWidth > 1.0f) ? 12.25f : 6.25f;
				if (bestDistSq > breedDistSq) {
					if (!this->path || this->path->isDone() || (this->random.genrand_int32() % 10) == 0) {
						this->setPath(this->level->findPath(this, mate, 16.0f, 1, 0, 0, 1));
					}
					if (!this->path) {
						float dx = mate->posX - this->posX;
						float dz = mate->posZ - this->posZ;
						float targetYaw = (atan2f(dz, dx) * 180.0f / 3.14159265f) - 90.0f;
						float diff = Mth::wrapDegrees(targetYaw - this->yaw);
						this->yaw = Mth::wrapDegrees(this->yaw + Mth::clamp(diff, -25.0f, 25.0f));
						this->moveForward = this->getSpeed();
					}
				} else {
					if (this->path) {
						this->setPath(0);
					}
					if (this->inLove <= 555 || mate->inLove <= 555) {
						AgableMob* baby = (AgableMob*)this->getBreedOffspring(mate);
						if (baby) {
							this->setAge(6000);
							mate->setAge(6000);
							this->resetLove();
							mate->resetLove();
							baby->setAge(-24000);
							baby->moveTo(this->posX, this->posY, this->posZ, this->yaw, 0.0f);
							this->level->addEntity(baby);
							for (int32_t p = 0; p < 7; ++p) {
								float gx = this->random.nextGaussian() * 0.02f;
								float gy = this->random.nextGaussian() * 0.02f;
								float gz = this->random.nextGaussian() * 0.02f;
								float px = this->posX + (this->random.nextFloat() * this->entityWidth * 2.0f) - this->entityWidth;
								float py = this->posY + 0.5f + (this->random.nextFloat() * this->entityHeight);
								float pz = this->posZ + (this->random.nextFloat() * this->entityWidth * 2.0f) - this->entityWidth;
								this->level->addParticle(PT_HEART, px, py, pz, gx, gy, gz, 0);
							}
						} else {
							this->resetLove();
							mate->resetLove();
						}
					}
				}
			}
		}
	}
}
bool_t Animal::canSpawn() {
	int32_t v3 = (int32_t)floorf(this->posX);
	int32_t v6 = (int32_t)floorf(this->boundingBox.minY);
	int32_t v11 = (int32_t)floorf(this->posZ);
	int32_t tBelow = this->level->getTile(v3, v6 - 1, v11);
	return (tBelow == Tile::grass->blockID || tBelow == Tile::snow->blockID || tBelow == Tile::topSnow->blockID || tBelow == Tile::dirt->blockID || tBelow == Tile::sand->blockID || tBelow == Tile::leaves->blockID || tBelow == Tile::ice->blockID) && this->level->getRawBrightness(v3, v6, v11) >= 4 && Mob::canSpawn();
}
bool_t Animal::removeWhenFarAway() {
	return !this->isBaby();
}
float Animal::getWalkTargetValue(int32_t x, int32_t y, int32_t z) {
	int tCur = this->level->getTile(x, y, z);
	int tBelow = this->level->getTile(x, y - 1, z);
	if (tCur == 8 || tCur == 9 || tBelow == 8 || tBelow == 9) {
		return -100.0f;
	}
	if (tBelow == Tile::grass->blockID || tBelow == Tile::snow->blockID || tBelow == Tile::topSnow->blockID || tBelow == Tile::leaves->blockID || tBelow == Tile::dirt->blockID || tBelow == Tile::sand->blockID || tBelow == Tile::ice->blockID) {
		return 10.0f;
	} else {
		return this->level->getBrightness(x, y, z) - 0.5f;
	}
}
Entity* Animal::findAttackTarget() {
	return 0;
}
bool_t Animal::isFood(const ItemInstance* a2) {
	return a2->getId() == Item::wheat->itemID;
}
