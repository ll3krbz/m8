#include <entity/Fox.hpp>
#include <entity/Arrow.hpp>
#include <entity/Player.hpp>
#include <entity/player/Abilities.hpp>
#include <entity/path/Path.hpp>
#include <inventory/Inventory.hpp>
#include <item/Item.hpp>
#include <item/ItemInstance.hpp>
#include <level/Level.hpp>
#include <nbt/CompoundTag.hpp>
#include <tile/Tile.hpp>
#include <math/Mth.hpp>
#include <cmath>

Fox::Fox(Level* level)
	: Animal(level) {
	this->entityRenderId = FOX;
	this->setSize(0.6f, 0.7f);
	this->foxType = 0;
	this->isFoxSleeping = 0;
	this->isFoxSitting = 0;
	this->isFoxCrouching = 0;
	this->isTame = 0;
	this->trustedPlayerId = 0;
	this->fleeTicks = 0;
	this->fleeTargetId = 0;
	this->repathTicks = 0;
	this->synchedEntityData.define<int8_t>(16, 0);
}

void Fox::updateSynchedFlags() {
	int8_t flags = 0;
	if (this->isFoxSitting) flags |= 1;
	if (this->isFoxSleeping) flags |= 2;
	if (this->isTame) flags |= 4;
	if (this->isFoxCrouching) flags |= 8;
	this->synchedEntityData.set<int8_t>(16, flags);
}

bool_t Fox::isFishItem(const ItemInstance* item) const {
	if (!item || !item->itemClass) return 0;
	Item* cls = item->itemClass;
	return cls == Item::fish_raw || cls == Item::fish_cooked || cls == Item::salmon_raw || cls == Item::salmon_cooked || cls == Item::clownfish || cls == Item::pufferfish;
}

Fox::~Fox() {
}

int32_t Fox::getEntityTypeId() const {
	return 38;
}

static std::string _foxTex = "mob/fox.png";
static std::string _snowFoxTex = "mob/snow_fox.png";

std::string* Fox::getTexture() {
	if (this->foxType == 1) {
		return &_snowFoxTex;
	}
	return &_foxTex;
}

int32_t Fox::getMaxHealth() {
	return 10;
}

float Fox::getBaseSpeed() {
	return 0.3f;
}

float Fox::getWalkingSpeedModifier() {
	if (this->fleeTicks > 0) {
		return Mob::getWalkingSpeedModifier() * 1.65f;
	}
	return Animal::getWalkingSpeedModifier();
}

int32_t Fox::getAmbientSoundInterval() {
	return 120;
}

const char_t* Fox::getAmbientSound() {
	if (this->isFoxSleeping) return "mob.fox.sleep";
	if (this->random.genrand_int32() % 4 == 0) return "mob.fox.sniff";
	return "mob.fox.idle";
}

std::string Fox::getHurtSound() {
	return "mob.fox.hurt";
}

std::string Fox::getDeathSound() {
	return "mob.fox.death";
}

bool_t Fox::hurt(Entity* attacker, int32_t dmg) {
	if (!Animal::hurt(attacker, dmg)) {
		return 0;
	}
	this->isFoxSleeping = 0;
	this->isFoxSitting = 0;
	this->isFoxCrouching = 0;
	if (attacker) {
		Entity* threat = attacker;
		if (attacker->getEntityTypeId() == 80 && ((Arrow*)attacker)->shooterEID != 0 && this->level) {
			Entity* shooter = this->level->getEntity(((Arrow*)attacker)->shooterEID);
			if (shooter) threat = shooter;
		}
		this->fleeTargetId = threat->entityId;
		this->fleeTicks = 100;
		this->repathTicks = 0;
	}
	this->updateSynchedFlags();
	return 1;
}

void Fox::updateAi() {
	if (this->isFoxSitting || this->isFoxSleeping) {
		this->setPath(0);
		this->moveForward = 0.0f;
		this->moveStrafe = 0.0f;
		this->isJumping = 0;
		return;
	}
	PathfinderMob::updateAi();
}

bool_t Fox::isFood(const ItemInstance* item) {
	if (!item || !item->itemClass) return 0;
	return this->isFishItem(item) || item->itemClass == Item::sweetBerries;
}

bool_t Fox::interactWithPlayer(Player* player) {
	if (!player || !player->inventory) {
		return Animal::interactWithPlayer(player);
	}
	ItemInstance* sel = player->inventory->getSelected();
	if (this->isFishItem(sel)) {
		if (!player->abilities.instabuild) {
			--sel->count;
			if (sel->count <= 0) {
				player->inventory->clearSlot(player->inventory->selectedSlot);
			}
		}
		if (this->level && !this->level->isClientMaybe) {
			bool_t wasTame = this->isTame;
			this->isTame = 1;
			this->trustedPlayerId = player->entityId;
			this->fleeTicks = 0;
			this->fleeTargetId = 0;
			this->inPanicTicksMaybe = 0;
			this->attackTarget = 0;
			this->isFoxSleeping = 0;
			this->isFoxCrouching = 0;
			this->setLastHurtByMob(0);
			if (!wasTame) {
				this->isFoxSitting = 0;
				this->health = this->getMaxHealth();
			} else if (this->health > 0 && this->health < this->getMaxHealth()) {
				int32_t nextHp = this->health + 4;
				int32_t maxHp = this->getMaxHealth();
				this->health = (nextHp > maxHp) ? maxHp : nextHp;
			} else if (this->getAge() == 0 && !this->isInLove()) {
				this->inLove = 600;
				this->setPath(0);
			} else if (this->isBaby()) {
				int32_t age = this->getAge();
				if (age < 0) {
					int32_t boost = (-age) / 10;
					if (boost < 20) boost = 20;
					int32_t nextAge = age + boost;
					if (nextAge > 0) nextAge = 0;
					this->setAge(nextAge);
				}
			}
			this->updateSynchedFlags();
			this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
		}
		if (this->level) {
			for (int32_t i = 0; i < 7; ++i) {
				float gx = this->random.nextGaussian() * 0.02f;
				float gy = this->random.nextGaussian() * 0.02f;
				float gz = this->random.nextGaussian() * 0.02f;
				float px = this->posX + (this->random.nextFloat() * this->entityWidth * 2.0f) - this->entityWidth;
				float py = this->posY + 0.5f + (this->random.nextFloat() * this->entityHeight);
				float pz = this->posZ + (this->random.nextFloat() * this->entityWidth * 2.0f) - this->entityWidth;
				this->level->addParticle(PT_HEART, px, py, pz, gx, gy, gz, 0);
			}
		}
		return 1;
	}
	if (this->isTame && !this->isFood(sel)) {
		if (this->level && !this->level->isClientMaybe) {
			this->isFoxSitting = this->isFoxSitting ? 0 : 1;
			this->isFoxSleeping = 0;
			this->isJumping = 0;
			this->setPath(0);
			this->trustedPlayerId = player->entityId;
			this->updateSynchedFlags();
		}
		return 1;
	}
	return Animal::interactWithPlayer(player);
}

void Fox::aiStep() {
	if (!this->level) {
		Animal::aiStep();
		return;
	}
	if (this->level->isClientMaybe) {
		uint8_t flags = (uint8_t)this->synchedEntityData.getByte(16);
		this->isFoxSitting = (flags & 1) != 0;
		this->isFoxSleeping = (flags & 2) != 0;
		this->isTame = (flags & 4) != 0;
		this->isFoxCrouching = (flags & 8) != 0;
		Animal::aiStep();
		return;
	}
	if (!this->isTame) {
		Player* threatPlayer = 0;
		float closestThreatDist = 11.5f;
		for (size_t i = 0; i < this->level->playersMaybe.size(); ++i) {
			Player* p = this->level->playersMaybe[i];
			if (!p || !p->isAlive() || p->isSneaking()) {
				continue;
			}
			float dx = this->posX - p->posX;
			float dy = this->posY - p->posY;
			float dz = this->posZ - p->posZ;
			float distSq = dx * dx + dy * dy + dz * dz;
			if (distSq > closestThreatDist * closestThreatDist) {
				continue;
			}
			float horizDist = sqrtf(dx * dx + dz * dz);
			float vx = p->posX - p->prevX;
			float vz = p->posZ - p->prevZ;
			float moveSpeed = sqrtf(vx * vx + vz * vz);
			float motSpeed = sqrtf(p->motionX * p->motionX + p->motionZ * p->motionZ);
			float totalSpeed = (moveSpeed > motSpeed) ? moveSpeed : motSpeed;
			float approach = totalSpeed;
			if (horizDist > 0.001f) {
				float invH = 1.0f / horizDist;
				float cMove = (vx * dx + vz * dz) * invH;
				float cMot = (p->motionX * dx + p->motionZ * dz) * invH;
				approach = (cMove > cMot) ? cMove : cMot;
			}
			bool_t approachingTooFast = (horizDist < 10.5f && approach > 0.11f) || (horizDist < 8.5f && totalSpeed > 0.145f) || (horizDist < 3.2f && totalSpeed > 0.05f);
			if (approachingTooFast && this->canSee(p)) {
				closestThreatDist = sqrtf(distSq);
				threatPlayer = p;
			}
		}
		if (threatPlayer) {
			this->isFoxSleeping = 0;
			this->isFoxSitting = 0;
			this->isFoxCrouching = 0;
			this->attackTarget = 0;
			this->fleeTargetId = threatPlayer->entityId;
			this->fleeTicks = 80;
			this->updateSynchedFlags();
		}
	} else if (this->fleeTicks <= 0 && this->isFoxSitting) {
		this->setPath(0);
		this->moveForward = 0.0f;
		this->moveStrafe = 0.0f;
		this->isJumping = 0;
		this->motionX *= 0.6f;
		this->motionZ *= 0.6f;
	} else if (this->fleeTicks <= 0 && this->trustedPlayerId != 0) {
		Entity* owner = this->level->getEntity(this->trustedPlayerId);
		if (owner && owner->isPlayer() && owner->isAlive()) {
			float distSq = owner->distanceToSqr(this);
			if (distSq > 36.0f && distSq < 400.0f) {
				if (this->repathTicks > 0) {
					--this->repathTicks;
				}
				if (!this->path || this->path->isDone() || this->repathTicks <= 0) {
					this->setPath(this->level->findPath(this, owner, 16.0f, 1, 0, 0, 1));
					this->repathTicks = 10;
				}
				this->lookAt(owner, 25.0f, 25.0f);
			} else if (distSq <= 9.0f) {
				if (this->path) {
					this->setPath(0);
				}
				this->lookAt(owner, 20.0f, 20.0f);
			}
		}
	}
	if (!this->isTame && this->fleeTicks <= 0) {
		for (size_t i = 0; i < this->level->playersMaybe.size(); ++i) {
			Player* p = this->level->playersMaybe[i];
			if (!p || !p->isAlive() || !p->inventory) continue;
			if (this->isFishItem(p->inventory->getSelected()) && p->distanceToSqr(this) <= 64.0f && this->canSee(p)) {
				this->lookAt(p, 15.0f, 15.0f);
				break;
			}
		}
	}
	if (this->fleeTicks > 0) {
		--this->fleeTicks;
		if (this->repathTicks > 0) {
			--this->repathTicks;
		}
		Entity* threat = this->fleeTargetId ? this->level->getEntity(this->fleeTargetId) : 0;
		if (threat && threat->isAlive() && threat->isPlayer() && ((Player*)threat)->isSneaking() && threat->distanceTo(this) > 4.0f) {
			threat = 0;
		}
		if (!threat || !threat->isAlive()) {
			this->fleeTicks = 0;
			this->fleeTargetId = 0;
		} else if (!this->path || this->path->isDone() || this->repathTicks <= 0) {
			float baseAng = atan2f(this->posZ - threat->posZ, this->posX - threat->posX);
			static const float _angleOffsets[7] = {0.0f, 0.4f, -0.4f, 0.8f, -0.8f, 1.2f, -1.2f};
			static const float _radii[2] = {10.0f, 6.0f};
			float bestScore = -99999.0f;
			int32_t bestX = 0;
			int32_t bestY = 0;
			int32_t bestZ = 0;
			bool_t found = 0;
			for (int32_t rIdx = 0; rIdx < 2; ++rIdx) {
				float distStep = _radii[rIdx];
				for (int32_t aIdx = 0; aIdx < 7; ++aIdx) {
					float ang = baseAng + _angleOffsets[aIdx];
					int32_t tx = Mth::floor(this->posX + cosf(ang) * distStep);
					int32_t tz = Mth::floor(this->posZ + sinf(ang) * distStep);
					for (int32_t dy = -3; dy <= 4; ++dy) {
						int32_t ty = Mth::floor(this->boundingBox.minY + 0.5f) + dy;
						if (ty <= 1 || ty >= 127) continue;
						int32_t tAt = this->level->getTile(tx, ty, tz);
						int32_t tBelow = this->level->getTile(tx, ty - 1, tz);
						if ((tAt > 0 && Tile::solid[tAt]) || tBelow <= 0 || !Tile::solid[tBelow]) {
							continue;
						}
						float val = this->getWalkTargetValue(tx, ty, tz);
						if (val >= 0.0f) {
							float tdx = (float)tx + 0.5f - threat->posX;
							float tdz = (float)tz + 0.5f - threat->posZ;
							float score = val + sqrtf(tdx * tdx + tdz * tdz);
							if (score > bestScore) {
								bestScore = score;
								bestX = tx;
								bestY = ty;
								bestZ = tz;
								found = 1;
							}
							break;
						}
					}
				}
				if (found) break;
			}
			if (found) {
				this->setPath(this->level->findPath(this, bestX, bestY, bestZ, 16.0f, 1, 0, 0, 1));
			} else {
				this->findRandomStrollLocation();
			}
			this->repathTicks = 12;
		}
	}
	Animal::aiStep();
}

bool_t Fox::canSpawn() {
	if (!this->level) return 0;
	int32_t x = (int32_t)floorf(this->posX);
	int32_t y = (int32_t)floorf(this->posY);
	int32_t z = (int32_t)floorf(this->posZ);
	if (y <= 1 || y >= 127) return 0;
	int32_t tileBelow = this->level->getTile(x, y - 1, z);
	if (tileBelow == Tile::snow->blockID || tileBelow == Tile::topSnow->blockID || tileBelow == Tile::ice->blockID) {
		this->foxType = 1;
	}
	return (tileBelow == Tile::grass->blockID || tileBelow == Tile::dirt->blockID || tileBelow == Tile::snow->blockID || tileBelow == Tile::topSnow->blockID || tileBelow == Tile::ice->blockID) && Mob::canSpawn();
}

void Fox::readAdditionalSaveData(CompoundTag* tag) {
	Animal::readAdditionalSaveData(tag);
	this->foxType = tag->getInt("FoxType");
	this->isTame = tag->getByte("Tame") != 0;
	this->isFoxSitting = tag->getByte("Sitting") != 0;
	this->isFoxSleeping = tag->getByte("Sleeping") != 0;
	this->updateSynchedFlags();
}

void Fox::addAdditonalSaveData(CompoundTag* tag) {
	Animal::addAdditonalSaveData(tag);
	tag->putInt("FoxType", this->foxType);
	tag->putBoolean("Tame", this->isTame);
	tag->putBoolean("Sitting", this->isFoxSitting);
	tag->putBoolean("Sleeping", this->isFoxSleeping);
}

bool_t Fox::removeWhenFarAway() {
	return !this->isTame;
}

Mob* Fox::getBreedOffspring(Animal*) {
	Fox* baby = new Fox(this->level);
	baby->foxType = this->foxType;
	baby->isTame = this->isTame;
	baby->trustedPlayerId = this->trustedPlayerId;
	baby->updateSynchedFlags();
	return baby;
}
