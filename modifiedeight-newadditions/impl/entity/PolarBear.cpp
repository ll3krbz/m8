#include <entity/PolarBear.hpp>
#include <entity/Arrow.hpp>
#include <entity/Player.hpp>
#include <entity/player/Abilities.hpp>
#include <inventory/Inventory.hpp>
#include <item/Item.hpp>
#include <item/ItemInstance.hpp>
#include <level/Level.hpp>
#include <nbt/CompoundTag.hpp>
#include <tile/Tile.hpp>
#include <math/Mth.hpp>
#include <cmath>

PolarBear::PolarBear(Level* level)
	: Animal(level) {
	this->entityRenderId = POLAR_BEAR;
	this->setSize(1.3f, 1.4f);
	this->angerTicks = 0;
	this->stareTicks = 0;
	this->friendlyTicks = 0;
	this->trustedPlayerId = 0;
}

bool_t PolarBear::isFishItem(const ItemInstance* inst) const {
	if (!inst || !inst->itemClass) return 0;
	Item* cls = inst->itemClass;
	return cls == Item::fish_raw || cls == Item::fish_cooked || cls == Item::salmon_raw || cls == Item::salmon_cooked || cls == Item::clownfish || cls == Item::pufferfish;
}

PolarBear::~PolarBear() {
}

int32_t PolarBear::getEntityTypeId() const {
	return 26;
}

static std::string _polarBearTex = "mob/polarbear.png";
std::string* PolarBear::getTexture() {
	return &_polarBearTex;
}

int32_t PolarBear::getMaxHealth() {
	return 30;
}

float PolarBear::getBaseSpeed() {
	return 0.25f;
}

float PolarBear::getWalkingSpeedModifier() {
	float base = Animal::getWalkingSpeedModifier();
	if (!this->isBaby() && this->attackTarget != 0) {
		return base * 1.45f;
	}
	return base;
}

int32_t PolarBear::getAmbientSoundInterval() {
	return 120;
}

const char_t* PolarBear::getAmbientSound() {
	return "mob.polarbear.idle";
}

std::string PolarBear::getHurtSound() {
	return "mob.polarbear.hurt";
}

std::string PolarBear::getDeathSound() {
	return "mob.polarbear.death";
}

bool_t PolarBear::isFood(const ItemInstance* inst) {
	return this->isFishItem(inst);
}

bool_t PolarBear::interactWithPlayer(Player* player) {
	if (!player || !player->inventory) {
		return Animal::interactWithPlayer(player);
	}
	ItemInstance* sel = player->inventory->getSelected();
	if (!sel || !this->isFishItem(sel)) {
		return Animal::interactWithPlayer(player);
	}
	if (!player->abilities.instabuild) {
		--sel->count;
		if (sel->count <= 0) {
			player->inventory->clearSlot(player->inventory->selectedSlot);
		}
	}
	if (!this->level->isClientMaybe) {
		this->attackTarget = 0;
		this->angerTicks = 0;
		this->stareTicks = 0;
		this->inPanicTicksMaybe = 0;
		this->setLastHurtByMob(0);
		this->trustedPlayerId = player->entityId;
		this->friendlyTicks = 2400;
		if (this->health > 0 && this->health < this->getMaxHealth()) {
			int32_t nextHp = this->health + 4;
			int32_t maxHp = this->getMaxHealth();
			this->health = (nextHp > maxHp) ? maxHp : nextHp;
		}
		if (this->isBaby()) {
			int32_t curAge = this->getAge();
			if (curAge < 0) {
				this->setAge(curAge - curAge / 10);
			}
		} else if (this->getAge() == 0 && this->inLove <= 0) {
			this->inLove = 600;
		}
		this->level->playSound(this, "random.eat", 0.8f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
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

bool_t PolarBear::hurt(Entity* attacker, int32_t dmg) {
	if (!Mob::hurt(attacker, dmg)) {
		return 0;
	}
	this->inLove = 0;
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
	if (this->isBaby()) {
		this->inPanicTicksMaybe = 80;
		this->attackTarget = 0;
		if (playerAttacker && playerAttacker->isAlive() && this->level) {
			std::vector<Entity*>* nearby = this->level->getEntities(this, {this->boundingBox.minX - 16.0f, this->boundingBox.minY - 8.0f, this->boundingBox.minZ - 16.0f, this->boundingBox.maxX + 16.0f, this->boundingBox.maxY + 8.0f, this->boundingBox.maxZ + 16.0f});
			if (nearby) {
				for (size_t i = 0; i < nearby->size(); ++i) {
					Entity* ent = nearby->at(i);
					if (ent && ent->getEntityTypeId() == 26 && ent->isAlive()) {
						PolarBear* adult = (PolarBear*)ent;
						if (!adult->isBaby()) {
							adult->friendlyTicks = 0;
							adult->trustedPlayerId = 0;
							adult->inPanicTicksMaybe = 0;
							adult->angerTicks = 600;
							adult->stareTicks = 0;
							adult->attackTarget = playerAttacker->entityId;
						}
					}
				}
			}
		}
		return 1;
	}
	if (playerAttacker && playerAttacker->isAlive()) {
		this->friendlyTicks = 0;
		this->trustedPlayerId = 0;
		this->inPanicTicksMaybe = 0;
		this->angerTicks = 600;
		this->stareTicks = 0;
		this->attackTarget = playerAttacker->entityId;
	}
	return 1;
}

Entity* PolarBear::findAttackTarget() {
	if (this->isBaby() || !this->level || this->level->isClientMaybe) {
		return 0;
	}
	bool_t hasNearbyCub = 0;
	std::vector<PolarBear*> cubs;
	std::vector<Entity*>* nearby = this->level->getEntities(this, {this->boundingBox.minX - 12.0f, this->boundingBox.minY - 6.0f, this->boundingBox.minZ - 12.0f, this->boundingBox.maxX + 12.0f, this->boundingBox.maxY + 6.0f, this->boundingBox.maxZ + 12.0f});
	if (nearby) {
		for (size_t i = 0; i < nearby->size(); ++i) {
			Entity* ent = nearby->at(i);
			if (ent && ent->getEntityTypeId() == 26 && ent->isAlive()) {
				PolarBear* other = (PolarBear*)ent;
				if (other->isBaby()) {
					hasNearbyCub = 1;
					cubs.push_back(other);
				}
			}
		}
	}
	if (hasNearbyCub) {
		Player* bestTarget = 0;
		float bestDistSq = 20.0f * 20.0f;
		for (size_t i = 0; i < this->level->playersMaybe.size(); ++i) {
			Player* p = this->level->playersMaybe[i];
			if (!p || !p->isAlive() || p->abilities.instabuild || p->abilities.invulnerable) {
				continue;
			}
			if (this->friendlyTicks > 0 && p->entityId == this->trustedPlayerId) {
				continue;
			}
			float dx = p->posX - this->posX;
			float dy = p->posY - this->posY;
			float dz = p->posZ - this->posZ;
			float dSq = dx * dx + dy * dy + dz * dz;
			bool_t nearBear = dSq <= 16.0f * 16.0f;
			bool_t nearCub = 0;
			for (size_t c = 0; c < cubs.size(); ++c) {
				float cdx = p->posX - cubs[c]->posX;
				float cdy = p->posY - cubs[c]->posY;
				float cdz = p->posZ - cubs[c]->posZ;
				if (cdx * cdx + cdy * cdy + cdz * cdz <= 12.0f * 12.0f) {
					nearCub = 1;
					break;
				}
			}
			if ((nearBear || (nearCub && dSq <= bestDistSq)) && dSq < bestDistSq && this->canSee(p)) {
				bestDistSq = dSq;
				bestTarget = p;
			}
		}
		if (bestTarget) {
			this->angerTicks = 600;
			return bestTarget;
		}
	}
	if (this->angerTicks > 0) {
		Player* np = this->level->getNearestPlayer(this, 16.0f);
		if (np && np->isAlive() && !np->abilities.instabuild && !np->abilities.invulnerable && !(this->friendlyTicks > 0 && np->entityId == this->trustedPlayerId) && this->canSee(np)) {
			return np;
		}
	}
	return 0;
}

void PolarBear::checkHurtTarget(Entity* target, float dist) {
	if (target && target->isAlive() && this->attackTime <= 0 && dist < 2.4f && target->boundingBox.maxY > this->boundingBox.minY && target->boundingBox.minY < this->boundingBox.maxY) {
		this->attackTime = 20;
		this->doHurtTarget(target);
	}
}

bool_t PolarBear::doHurtTarget(Entity* target) {
	if (!target) {
		return 0;
	}
	this->swing();
	return target->hurt(this, this->isBaby() ? 2 : 6);
}

void PolarBear::aiStep() {
	Animal::aiStep();
	if (!this->level || this->level->isClientMaybe) {
		return;
	}
	if (this->friendlyTicks > 0) {
		--this->friendlyTicks;
		if (this->friendlyTicks == 0) {
			this->trustedPlayerId = 0;
		}
	}
	if (this->angerTicks > 0) {
		--this->angerTicks;
	}
	if (this->isBaby()) {
		this->stareTicks = 0;
		this->attackTarget = 0;
		this->angerTicks = 0;
		return;
	}
	if (this->attackTarget != 0) {
		Entity* tgt = this->level->getEntity(this->attackTarget);
		if (!tgt || !tgt->isAlive() || (tgt->isPlayer() && ((Player*)tgt)->entityId == this->trustedPlayerId && this->friendlyTicks > 0)) {
			this->attackTarget = 0;
			this->angerTicks = 0;
		}
		this->stareTicks = 0;
		return;
	}
	Entity* cubTarget = this->findAttackTarget();
	if (cubTarget) {
		this->attackTarget = cubTarget->entityId;
		this->stareTicks = 0;
		return;
	}
	bool_t isBeingStaredAt = 0;
	Player* staringPlayer = 0;
	Player* np = this->level->getNearestPlayer(this, 16.0f);
	if (np && np->isAlive() && !np->abilities.instabuild && !np->abilities.invulnerable && !(this->friendlyTicks > 0 && np->entityId == this->trustedPlayerId)) {
		float eyeX = np->posX;
		float eyeY = np->posY + np->getHeadHeight();
		float eyeZ = np->posZ;
		float targetX = this->posX;
		float targetY = this->posY + this->getHeadHeight() * 0.7f;
		float targetZ = this->posZ;
		float dx = targetX - eyeX;
		float dy = targetY - eyeY;
		float dz = targetZ - eyeZ;
		float dist = sqrtf(dx * dx + dy * dy + dz * dz);
		if (dist > 0.5f && dist <= 16.0f) {
			Vec3 look = np->getViewVector(1.0f);
			float invDist = 1.0f / dist;
			float dot = (dx * look.x + dy * look.y + dz * look.z) * invDist;
			float threshold = 1.0f - (0.14f / (dist > 1.0f ? dist : 1.0f));
			if (dot > threshold && dot > 0.96f && this->canSee(np)) {
				isBeingStaredAt = 1;
				staringPlayer = np;
			}
		}
	}
	if (isBeingStaredAt && staringPlayer) {
		++this->stareTicks;
		if (this->stareTicks >= 60 && (this->random.genrand_int32() % 100) == 0) {
			this->angerTicks = 500;
			this->attackTarget = staringPlayer->entityId;
			this->stareTicks = 0;
			this->level->playSound(this, "mob.polarbear.hurt", 0.9f, 0.7f);
		}
	} else if (this->stareTicks > 0) {
		this->stareTicks = (this->stareTicks >= 2) ? (this->stareTicks - 2) : 0;
	}
}

void PolarBear::readAdditionalSaveData(CompoundTag* tag) {
	Animal::readAdditionalSaveData(tag);
	this->angerTicks = tag->getInt("Anger");
	this->friendlyTicks = tag->getInt("FriendlyTicks");
}

void PolarBear::addAdditonalSaveData(CompoundTag* tag) {
	Animal::addAdditonalSaveData(tag);
	tag->putInt("Anger", this->angerTicks);
	tag->putInt("FriendlyTicks", this->friendlyTicks);
}

bool_t PolarBear::canSpawn() {
	if (!this->level) return 0;
	int32_t x = (int32_t)floorf(this->posX);
	int32_t y = (int32_t)floorf(this->posY);
	int32_t z = (int32_t)floorf(this->posZ);
	if (y <= 1 || y >= 127) return 0;
	int32_t tileBelow = this->level->getTile(x, y - 1, z);
	return (tileBelow == Tile::grass->blockID || tileBelow == Tile::snow->blockID || tileBelow == Tile::ice->blockID || tileBelow == Tile::topSnow->blockID || tileBelow == Tile::dirt->blockID || tileBelow == Tile::rock->blockID) && Mob::canSpawn();
}

Mob* PolarBear::getBreedOffspring(Animal*) {
	return new PolarBear(this->level);
}
