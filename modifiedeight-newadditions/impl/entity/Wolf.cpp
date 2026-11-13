#include <entity/Wolf.hpp>
#include <entity/Arrow.hpp>
#include <entity/Player.hpp>
#include <entity/player/Abilities.hpp>
#include <inventory/Inventory.hpp>
#include <item/Item.hpp>
#include <item/ItemInstance.hpp>
#include <level/Level.hpp>
#include <level/biome/Biome.hpp>
#include <nbt/CompoundTag.hpp>
#include <tile/Tile.hpp>
#include <math/Mth.hpp>
#include <cmath>

Wolf::Wolf(Level* level)
	: Animal(level) {
	this->entityRenderId = WOLF;
	this->setSize(0.6f, 0.8f);
	this->wolfType = 0;
	this->isAngry = 0;
	this->isTame = 0;
	this->isSitting = 0;
	this->angerTicks = 0;
	this->pacifiedTicks = 0;
	this->synchedEntityData.define<int8_t>(16, 0);
}

void Wolf::updateSynchedFlags() {
	int8_t flags = 0;
	if (this->isSitting) flags |= 1;
	if (this->isAngry) flags |= 2;
	if (this->isTame) flags |= 4;
	this->synchedEntityData.set<int8_t>(16, flags);
}

void Wolf::setAngry(bool_t angry) {
	this->isAngry = angry ? 1 : 0;
	this->updateSynchedFlags();
}

Wolf::~Wolf() {
}

int32_t Wolf::getEntityTypeId() const {
	return 14;
}

static std::string _wolfTex = "mob/wolf.png";
static std::string _wolfAngryTex = "mob/wolf_angry.png";
static std::string _wolfTameTex = "mob/wolf_tame.png";
static std::string _snowWolfTex = "mob/snow_wolf.png";
static std::string _snowWolfAngryTex = "mob/snow_wolf_angry.png";
static std::string _snowWolfTameTex = "mob/snow_wolf_tame.png";

std::string* Wolf::getTexture() {
	if (this->wolfType == 1) {
		if (this->isAngry) return &_snowWolfAngryTex;
		if (this->isTame) return &_snowWolfTameTex;
		return &_snowWolfTex;
	}
	if (this->isAngry) return &_wolfAngryTex;
	if (this->isTame) return &_wolfTameTex;
	return &_wolfTex;
}

int32_t Wolf::getMaxHealth() {
	return this->isTame ? 20 : 8;
}

float Wolf::getBaseSpeed() {
	return 0.3f;
}

float Wolf::getWalkingSpeedModifier() {
	float base = Animal::getWalkingSpeedModifier();
	if (this->isAngry || this->attackTarget != 0) {
		return base * 1.4f;
	}
	return base;
}

int32_t Wolf::getAmbientSoundInterval() {
	return 120;
}

const char_t* Wolf::getAmbientSound() {
	if (this->isAngry) return "mob.wolf.growl";
	if (this->random.genrand_int32() % 3 == 0) {
		return (this->isTame && this->health < 10) ? "mob.wolf.whine" : "mob.wolf.panting";
	}
	return "mob.wolf.bark";
}

std::string Wolf::getHurtSound() {
	return "mob.wolf.hurt";
}

std::string Wolf::getDeathSound() {
	return "mob.wolf.death";
}

void Wolf::aiStep() {
	Animal::aiStep();
	if (!this->level) {
		return;
	}
	if (this->level->isClientMaybe) {
		uint8_t flags = (uint8_t)this->synchedEntityData.getByte(16);
		this->isSitting = (flags & 1) != 0;
		this->isAngry = (flags & 2) != 0;
		this->isTame = (flags & 4) != 0;
		return;
	}
	if (this->pacifiedTicks > 0) {
		--this->pacifiedTicks;
	}
	if (this->isAngry) {
		if (this->attackTarget != 0) {
			Entity* tgt = this->level->getEntity(this->attackTarget);
			if (!tgt || !tgt->isAlive() || (tgt->isPlayer() && (((Player*)tgt)->abilities.instabuild || ((Player*)tgt)->abilities.invulnerable))) {
				this->attackTarget = 0;
				this->angerTicks = 0;
				this->setAngry(0);
				return;
			}
			if (this->angerTicks > 0) {
				--this->angerTicks;
			}
			if (this->angerTicks <= 0) {
				this->attackTarget = 0;
				this->setAngry(0);
			}
		} else {
			if (this->angerTicks > 20) {
				this->angerTicks = 20;
			}
			if (this->angerTicks > 0) {
				--this->angerTicks;
			}
			if (this->angerTicks <= 0) {
				this->setAngry(0);
			}
		}
	}
}

bool_t Wolf::interactWithPlayer(Player* player) {
	if (!player || !player->inventory) {
		return Animal::interactWithPlayer(player);
	}
	ItemInstance* sel = player->inventory->getSelected();
	if (sel && sel->itemClass == Item::bone) {
		if (!player->abilities.instabuild) {
			--sel->count;
			if (sel->count <= 0) {
				player->inventory->clearSlot(player->inventory->selectedSlot);
			}
		}
		if (!this->level->isClientMaybe) {
			this->setAngry(0);
			this->attackTarget = 0;
			this->angerTicks = 0;
			this->inPanicTicksMaybe = 0;
			this->pacifiedTicks = 200;
			this->setLastHurtByMob(0);
			if (this->health > 0 && this->health < this->getMaxHealth()) {
				int32_t nextHp = this->health + 4;
				int32_t maxHp = this->getMaxHealth();
				this->health = (nextHp > maxHp) ? maxHp : nextHp;
			}
			this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
		}
		for (int32_t i = 0; i < 7; ++i) {
			float gx = this->random.nextGaussian() * 0.02f;
			float gy = this->random.nextGaussian() * 0.02f;
			float gz = this->random.nextGaussian() * 0.02f;
			float px = this->posX + (this->random.nextFloat() * this->entityWidth * 2.0f) - this->entityWidth;
			float py = this->posY + 0.5f + (this->random.nextFloat() * this->entityHeight);
			float pz = this->posZ + (this->random.nextFloat() * this->entityWidth * 2.0f) - this->entityWidth;
			this->level->addParticle(PT_HEART, px, py, pz, gx, gy, gz, 0);
		}
		return 1;
	}
	return Animal::interactWithPlayer(player);
}

bool_t Wolf::hurt(Entity* attacker, int32_t dmg) {
	if (!Mob::hurt(attacker, dmg)) {
		return 0;
	}
	this->inLove = 0;
	this->inPanicTicksMaybe = 0;
	Player* playerAttacker = 0;
	if (attacker) {
		if (attacker->isPlayer()) {
			playerAttacker = (Player*)attacker;
		} else if (attacker->getEntityTypeId() == 80 && ((Arrow*)attacker)->shooterEID != 0 && this->level) {
			Entity* shooter = this->level->getEntity(((Arrow*)attacker)->shooterEID);
			if (shooter && shooter->isPlayer()) {
				playerAttacker = (Player*)shooter;
			}
		}
	}
	if (playerAttacker && playerAttacker->isAlive()) {
		this->isSitting = 0;
		this->pacifiedTicks = 0;
		this->angerTicks = 600;
		this->setAngry(1);
		this->attackTarget = playerAttacker->entityId;
		if (this->level) {
			std::vector<Entity*>* nearby = this->level->getEntities(this, {this->boundingBox.minX - 20.0f, this->boundingBox.minY - 10.0f, this->boundingBox.minZ - 20.0f, this->boundingBox.maxX + 20.0f, this->boundingBox.maxY + 10.0f, this->boundingBox.maxZ + 20.0f});
			if (nearby) {
				for (size_t i = 0; i < nearby->size(); ++i) {
					Entity* ent = nearby->at(i);
					if (ent && ent != this && ent->getEntityTypeId() == 14 && ent->isAlive()) {
						Wolf* mate = (Wolf*)ent;
						if (!mate->isSitting && mate->pacifiedTicks <= 0) {
							mate->inPanicTicksMaybe = 0;
							mate->inLove = 0;
							mate->angerTicks = 600;
							mate->setAngry(1);
							mate->attackTarget = playerAttacker->entityId;
						}
					}
				}
			}
		}
	}
	return 1;
}

Entity* Wolf::findAttackTarget() {
	if (!this->isAngry || this->pacifiedTicks > 0 || !this->level || this->level->isClientMaybe) {
		return 0;
	}
	if (this->angerTicks > 0) {
		Player* np = this->level->getNearestPlayer(this, 16.0f);
		if (np && np->isAlive() && !np->abilities.instabuild && !np->abilities.invulnerable && this->canSee(np)) {
			return np;
		}
	}
	return 0;
}

void Wolf::checkHurtTarget(Entity* target, float dist) {
	if (target && target->isAlive() && this->attackTime <= 0 && dist < 2.0f && target->boundingBox.maxY > this->boundingBox.minY && target->boundingBox.minY < this->boundingBox.maxY) {
		this->attackTime = 20;
		this->doHurtTarget(target);
	}
}

bool_t Wolf::doHurtTarget(Entity* target) {
	if (!target) {
		return 0;
	}
	this->swing();
	return target->hurt(this, this->isBaby() ? 2 : 4);
}

void Wolf::readAdditionalSaveData(CompoundTag* tag) {
	Animal::readAdditionalSaveData(tag);
	this->wolfType = tag->getInt("WolfType");
	this->isAngry = tag->getByte("Angry") != 0;
	this->isTame = tag->getByte("Tame") != 0;
	this->isSitting = tag->getByte("Sitting") != 0;
	this->angerTicks = this->isAngry ? 600 : 0;
	this->updateSynchedFlags();
}

void Wolf::addAdditonalSaveData(CompoundTag* tag) {
	Animal::addAdditonalSaveData(tag);
	tag->putInt("WolfType", this->wolfType);
	tag->putBoolean("Angry", this->isAngry);
	tag->putBoolean("Tame", this->isTame);
	tag->putBoolean("Sitting", this->isSitting);
}

bool_t Wolf::canSpawn() {
	if (!this->level) return 0;
	int32_t x = Mth::floor(this->posX);
	int32_t y = Mth::floor(this->posY);
	int32_t z = Mth::floor(this->posZ);
	if (y <= 1 || y >= 127) return 0;
	int32_t tileBelow = this->level->getTile(x, y - 1, z);
	if (tileBelow == 78 || tileBelow == 80 || tileBelow == 79) {
		this->wolfType = 1;
	}
	Biome* b = this->level->getBiome(x, z);
	if (b == Biome::taiga || b == Biome::tundra || b == Biome::icePeaks || b == Biome::iceDesert) {
		this->wolfType = 1;
	}
	return (tileBelow == 2 || tileBelow == 3 || tileBelow == 78 || tileBelow == 79 || tileBelow == 80 || tileBelow == Tile::grass->blockID || tileBelow == Tile::dirt->blockID || tileBelow == Tile::snow->blockID || tileBelow == Tile::topSnow->blockID || tileBelow == Tile::ice->blockID) && Mob::canSpawn();
}

bool_t Wolf::removeWhenFarAway() {
	return !this->isTame;
}

Mob* Wolf::getBreedOffspring(Animal*) {
	Wolf* baby = new Wolf(this->level);
	baby->wolfType = this->wolfType;
	return baby;
}
