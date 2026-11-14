#include <entity/Wolf.hpp>
#include <entity/Fox.hpp>
#include <entity/Ocelot.hpp>
#include <entity/Arrow.hpp>
#include <entity/Player.hpp>
#include <entity/player/Abilities.hpp>
#include <entity/path/Path.hpp>
#include <inventory/Inventory.hpp>
#include <item/Item.hpp>
#include <item/ItemInstance.hpp>
#include <level/Level.hpp>
#include <level/biome/Biome.hpp>
#include <nbt/CompoundTag.hpp>
#include <tile/Tile.hpp>
#include <tile/material/Material.hpp>
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
	this->isInterested = 0;
	this->angerTicks = 0;
	this->pacifiedTicks = 0;
	this->ownerName = "";
	this->ownerEntityId = 0;
	this->repathTicks = 0;
	this->sitLookMode = 0;
	this->sitLookTicks = 0;
	this->sitYawOffset = 0.0f;
	this->sitPitchTarget = 0.0f;
	this->synchedEntityData.define<int8_t>(16, 0);
}

bool_t Wolf::isMeatItem(const ItemInstance* item) const {
	if (!item || !item->itemClass) return 0;
	Item* cls = item->itemClass;
	return cls == Item::porkChop_raw || cls == Item::porkChop_cooked || cls == Item::beef_raw || cls == Item::beef_cooked || cls == Item::chicken_raw || cls == Item::chicken_cooked;
}

bool_t Wolf::isTemptFood(const ItemInstance* item) const {
	if (!item || !item->itemClass) return 0;
	return item->itemClass == Item::bone || this->isMeatItem(item);
}

bool_t Wolf::isOwnerPlayer(const Player* player) const {
	if (!player || !this->isTame) return 0;
	if (!this->ownerName.empty()) {
		return player->username == this->ownerName;
	}
	if (this->ownerEntityId != 0) {
		return player->entityId == this->ownerEntityId;
	}
	return (this->level && this->level->playersMaybe.size() == 1);
}

Player* Wolf::resolveOwner() {
	if (!this->level || !this->isTame) return 0;
	if (this->ownerEntityId != 0) {
		Entity* e = this->level->getEntity(this->ownerEntityId);
		if (e && e->isPlayer()) {
			return (Player*)e;
		}
		this->ownerEntityId = 0;
	}
	if (!this->ownerName.empty()) {
		for (size_t i = 0; i < this->level->playersMaybe.size(); ++i) {
			Player* p = this->level->playersMaybe[i];
			if (p && p->username == this->ownerName) {
				this->ownerEntityId = p->entityId;
				return p;
			}
		}
	} else if (this->level->playersMaybe.size() == 1) {
		Player* p = this->level->playersMaybe[0];
		if (p) {
			this->ownerName = p->username;
			this->ownerEntityId = p->entityId;
			return p;
		}
	}
	return 0;
}

static bool_t isWolfValidTargetForOwner(Wolf* self, Player* owner, Entity* candidate) {
	if (!candidate || !candidate->isAlive() || candidate == self || candidate == owner) {
		return 0;
	}
	if (candidate->isPlayer() && (((Player*)candidate)->abilities.instabuild || ((Player*)candidate)->abilities.invulnerable)) {
		return 0;
	}
	if (candidate->getEntityTypeId() == 14) {
		Wolf* other = (Wolf*)candidate;
		if (other->isTame && ((owner && other->isOwnerPlayer(owner)) || (!self->ownerName.empty() && other->ownerName == self->ownerName))) {
			return 0;
		}
	}
	if (candidate->getEntityTypeId() == 38) {
		Fox* otherFox = (Fox*)candidate;
		if (otherFox->isTame && ((owner && otherFox->isOwnerPlayer(owner)) || (!self->ownerName.empty() && otherFox->ownerName == self->ownerName))) {
			return 0;
		}
	}
	if (candidate->getEntityTypeId() == 22) {
		Ocelot* otherOcelot = (Ocelot*)candidate;
		if (otherOcelot->isTrusting && ((owner && otherOcelot->isOwnerPlayer(owner)) || (!self->ownerName.empty() && otherOcelot->ownerName == self->ownerName))) {
			return 0;
		}
	}
	return 1;
}

static Entity* findOwnerCombatTarget(Wolf* self, Player* owner) {
	if (!self || !self->level || !owner || !owner->isAlive()) {
		return 0;
	}
	Entity* ownerVictim = (owner->field_C58 > 0 && owner->lastHurtMobTicks > 0) ? self->level->getEntity(owner->field_C58) : 0;
	Entity* ownerAttacker = (owner->field_C5C > 0 && owner->field_C60 > 0) ? self->level->getEntity(owner->field_C5C) : (Entity*)owner->getLastHurtByMob();
	bool_t validVictim = ownerVictim && isWolfValidTargetForOwner(self, owner, ownerVictim) && ownerVictim->distanceToSqr(self) <= 576.0f;
	bool_t validAttacker = ownerAttacker && isWolfValidTargetForOwner(self, owner, ownerAttacker) && ownerAttacker->distanceToSqr(self) <= 576.0f;
	if (validVictim && validAttacker) {
		return (owner->lastHurtMobTicks >= owner->field_C60) ? ownerVictim : ownerAttacker;
	}
	if (validVictim) {
		return ownerVictim;
	}
	if (validAttacker) {
		return ownerAttacker;
	}
	return 0;
}

void Wolf::updateSynchedFlags() {
	int8_t flags = 0;
	if (this->isSitting) flags |= 1;
	if (this->isAngry) flags |= 2;
	if (this->isTame) flags |= 4;
	if (this->isInterested) flags |= 8;
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

static float steerWolfYawToward(float curYaw, float dx, float dz, float maxTurn) {
	float targetYaw = (atan2f(dz, dx) * 180.0f / 3.14159265f) - 90.0f;
	float diff = Mth::wrapDegrees(targetYaw - curYaw);
	return Mth::wrapDegrees(curYaw + Mth::clamp(diff, -maxTurn, maxTurn));
}

void Wolf::updateAi() {
	if (this->isSitting) {
		this->setPath(0);
		this->attackTarget = 0;
		this->moveForward = 0.0f;
		this->moveStrafe = 0.0f;
		this->isJumping = 0;
		if (this->level) {
			Player* owner = this->isTame ? this->resolveOwner() : 0;
			Player* watchPlayer = (owner && owner->isAlive() && owner->distanceToSqr(this) <= 256.0f) ? owner : this->level->getNearestPlayer(this, 10.0f);
			ItemInstance* held = (watchPlayer && watchPlayer->inventory) ? watchPlayer->inventory->getSelected() : 0;
			bool_t holdingTreat = (held && (held->itemClass == Item::bone || this->isMeatItem(held))) ? 1 : 0;
			if (this->sitLookTicks > 0) {
				--this->sitLookTicks;
			}
			if (this->sitLookTicks <= 0) {
				if (holdingTreat && watchPlayer) {
					this->sitLookMode = 0;
					this->sitLookTicks = 15 + (this->random.genrand_int32() % 20);
					this->sitYawOffset = 0.0f;
					this->sitPitchTarget = -4.0f;
				} else if (watchPlayer) {
					int32_t roll = this->random.genrand_int32() % 100;
					if (this->sitLookMode != 0 && roll < 44) {
						this->sitLookMode = 0;
						this->sitLookTicks = 28 + (this->random.genrand_int32() % 48);
						this->sitYawOffset = (this->random.nextFloat() - 0.5f) * 8.0f;
						this->sitPitchTarget = (this->random.nextFloat() - 0.5f) * 5.0f;
					} else if (roll < 64) {
						this->sitLookMode = 4;
						this->sitLookTicks = 24 + (this->random.genrand_int32() % 40);
						this->sitYawOffset = 0.0f;
						this->sitPitchTarget = 0.0f;
					} else if (roll < 82) {
						this->sitLookMode = 1;
						this->sitLookTicks = 18 + (this->random.genrand_int32() % 28);
						float side = (this->random.genrand_int32() & 1) ? 1.0f : -1.0f;
						this->sitYawOffset = side * (26.0f + this->random.nextFloat() * 34.0f);
						this->sitPitchTarget = (this->random.nextFloat() - 0.45f) * 14.0f;
					} else if (roll < 92) {
						this->sitLookMode = 2;
						this->sitLookTicks = 14 + (this->random.genrand_int32() % 22);
						float side = (this->sitYawOffset >= 0.0f) ? -1.0f : 1.0f;
						this->sitYawOffset = side * (22.0f + this->random.nextFloat() * 34.0f);
						this->sitPitchTarget = -8.0f + this->random.nextFloat() * 18.0f;
					} else {
						this->sitLookMode = 3;
						this->sitLookTicks = 16 + (this->random.genrand_int32() % 24);
						this->sitYawOffset = (this->random.nextFloat() - 0.5f) * 28.0f;
						this->sitPitchTarget = 16.0f + this->random.nextFloat() * 12.0f;
					}
				} else {
					int32_t roll = this->random.genrand_int32() % 100;
					if (this->sitLookMode != 4 && roll < 42) {
						this->sitLookMode = 4;
						this->sitLookTicks = 28 + (this->random.genrand_int32() % 46);
						this->sitYawOffset = 0.0f;
						this->sitPitchTarget = 0.0f;
					} else if (roll < 78) {
						this->sitLookMode = 1;
						this->sitLookTicks = 22 + (this->random.genrand_int32() % 38);
						float side = (this->sitYawOffset >= 0.0f) ? -1.0f : 1.0f;
						this->sitYawOffset = side * (24.0f + this->random.nextFloat() * 42.0f);
						this->sitPitchTarget = (this->random.nextFloat() - 0.5f) * 16.0f;
					} else {
						this->sitLookMode = 3;
						this->sitLookTicks = 18 + (this->random.genrand_int32() % 28);
						this->sitYawOffset = (this->random.nextFloat() - 0.5f) * 30.0f;
						this->sitPitchTarget = 15.0f + this->random.nextFloat() * 12.0f;
					}
				}
			}
			float wantYaw = this->yaw;
			float wantPitch = 0.0f;
			float anchorYaw = this->field_124;
			if (watchPlayer && watchPlayer->isAlive()) {
				float dx = watchPlayer->posX - this->posX;
				float dz = watchPlayer->posZ - this->posZ;
				float dy = (watchPlayer->boundingBox.minY + 1.35f) - (this->posY + this->getHeadHeight());
				float horiz = Mth::sqrt(dx * dx + dz * dz);
				if (horiz < 0.001f) horiz = 0.001f;
				float ownerYaw = (atan2f(dz, dx) * 180.0f / 3.14159265f) - 90.0f;
				float ownerPitch = -(atan2f(dy, horiz) * 180.0f / 3.14159265f);
				ownerPitch = Mth::clamp(ownerPitch, -35.0f, 35.0f);
				anchorYaw = ownerYaw;
				if (holdingTreat || this->sitLookMode == 0) {
					wantYaw = Mth::wrapDegrees(ownerYaw + (holdingTreat ? 0.0f : this->sitYawOffset));
					wantPitch = Mth::clamp(ownerPitch + (holdingTreat ? -3.0f : this->sitPitchTarget), -35.0f, 35.0f);
				} else {
					wantYaw = Mth::wrapDegrees(ownerYaw + this->sitYawOffset);
					wantPitch = Mth::clamp(ownerPitch * 0.35f + this->sitPitchTarget, -30.0f, 30.0f);
				}
			} else {
				wantYaw = Mth::wrapDegrees(this->field_124 + this->sitYawOffset);
				wantPitch = Mth::clamp(this->sitPitchTarget, -25.0f, 25.0f);
			}
			float yawDiff = Mth::wrapDegrees(wantYaw - this->yaw);
			this->yaw = Mth::wrapDegrees(this->yaw + Mth::clamp(yawDiff * 0.28f, -14.0f, 14.0f));
			float pitchDiff = wantPitch - this->pitch;
			this->pitch = Mth::clamp(this->pitch + Mth::clamp(pitchDiff * 0.25f, -8.0f, 8.0f), -35.0f, 35.0f);
			this->headYaw = this->yaw;
			float bodyDiff = Mth::wrapDegrees(anchorYaw - this->field_124);
			if (fabsf(bodyDiff) > 28.0f) {
				this->field_124 = Mth::wrapDegrees(this->field_124 + Mth::clamp(bodyDiff * 0.14f, -8.0f, 8.0f));
			}
		}
		return;
	}
	if (this->isInWater() && this->level) {
		Entity* swimTarget = 0;
		if (this->attackTarget != 0) {
			Entity* tgt = this->level->getEntity(this->attackTarget);
			if (tgt && tgt->isAlive()) {
				swimTarget = tgt;
			}
		} else if (this->isTame) {
			Player* owner = this->resolveOwner();
			if (owner && owner->isAlive()) {
				swimTarget = owner;
			}
		}
		if (swimTarget) {
			if (this->path) {
				this->setPath(0);
			}
			this->moveStrafe = 0.0f;
			this->isJumping = 0;
			float dx = swimTarget->posX - this->posX;
			float dz = swimTarget->posZ - this->posZ;
			float horizDistSq = dx * dx + dz * dz;
			float distSq = swimTarget->distanceToSqr(this);
			float stopDistSq = (this->attackTarget != 0) ? 1.5f : (swimTarget->isInWater() ? 9.0f : 3.0f);
			this->yaw = steerWolfYawToward(this->yaw, dx, dz, 16.0f);
			if (this->isCollidedHorizontally && horizDistSq > stopDistSq) {
				float rad = this->yaw * (3.14159265f / 180.0f);
				int32_t by = Mth::floor(this->boundingBox.minY + 0.55f);
				int32_t bx = Mth::floor(this->posX - Mth::sin(rad) * 0.75f);
				int32_t bz = Mth::floor(this->posZ + Mth::cos(rad) * 0.75f);
				int32_t tLow = this->level->getTile(bx, by, bz);
				int32_t tHigh = this->level->getTile(bx, by + 1, bz);
				if (tLow > 0 && Tile::solid[tLow] && tHigh > 0 && Tile::solid[tHigh]) {
					float leftYaw = Mth::wrapDegrees(this->yaw - 55.0f);
					float rightYaw = Mth::wrapDegrees(this->yaw + 55.0f);
					float lRad = leftYaw * (3.14159265f / 180.0f);
					float rRad = rightYaw * (3.14159265f / 180.0f);
					int32_t lLow = this->level->getTile(Mth::floor(this->posX - Mth::sin(lRad) * 0.9f), by, Mth::floor(this->posZ + Mth::cos(lRad) * 0.9f));
					int32_t rLow = this->level->getTile(Mth::floor(this->posX - Mth::sin(rRad) * 0.9f), by, Mth::floor(this->posZ + Mth::cos(rRad) * 0.9f));
					bool_t leftBlocked = (lLow > 0 && Tile::solid[lLow]);
					bool_t rightBlocked = (rLow > 0 && Tile::solid[rLow]);
					if (!leftBlocked && rightBlocked) {
						this->yaw = leftYaw;
					} else if (!rightBlocked && leftBlocked) {
						this->yaw = rightYaw;
					} else {
						this->yaw = ((this->entityId & 1) ? leftYaw : rightYaw);
					}
				}
			}
			this->lookAt(swimTarget, 30.0f, 30.0f);
			this->headYaw = this->yaw;
			if (horizDistSq > stopDistSq && distSq > stopDistSq) {
				this->moveForward = (distSq > 64.0f || this->attackTarget != 0) ? 1.35f : 1.0f;
			} else {
				this->moveForward = 0.0f;
			}
			if (this->attackTarget != 0 && this->canSee(swimTarget)) {
				this->checkHurtTarget(swimTarget, swimTarget->distanceTo(this));
			}
			return;
		}
	}
	PathfinderMob::updateAi();
	if (this->isTame && this->level) {
		Entity* directTgt = 0;
		if (this->attackTarget != 0 && !this->path) {
			Entity* tgt = this->level->getEntity(this->attackTarget);
			if (tgt && tgt->isAlive()) {
				directTgt = tgt;
			}
		} else if (this->attackTarget == 0) {
			Player* owner = this->resolveOwner();
			if (owner && owner->isAlive()) {
				float distSq = owner->distanceToSqr(this);
				if ((!this->path && distSq > (owner->isInWater() ? 9.0f : 16.0f)) || (owner->isInWater() && distSq > 9.0f && fabsf(owner->posY - this->posY) <= 4.0f && this->canSee(owner))) {
					directTgt = owner;
				}
			}
		}
		if (directTgt) {
			if (this->path) {
				this->setPath(0);
			}
			float dx = directTgt->posX - this->posX;
			float dz = directTgt->posZ - this->posZ;
			this->yaw = steerWolfYawToward(this->yaw, dx, dz, 30.0f);
			this->moveForward = this->getSpeed();
			this->lookAt(directTgt, 30.0f, 30.0f);
			if (this->isCollidedHorizontally) {
				this->isJumping = 1;
			}
		}
	}
}

void Wolf::travel(float strafe, float forward) {
	if (this->isInWater() && this->level) {
		float yawRad = this->yaw * (3.14159265f / 180.0f);
		float targetSpeed = 0.0f;
		if (forward > 0.01f) {
			if (forward >= 1.15f) {
				targetSpeed = 0.19f;
			} else if (forward >= 0.7f) {
				targetSpeed = 0.15f;
			} else {
				targetSpeed = forward * 0.45f;
			}
		}
		float wantVx = -Mth::sin(yawRad) * targetSpeed;
		float wantVz = Mth::cos(yawRad) * targetSpeed;
		if (fabsf(strafe) > 0.01f) {
			float strafeSpeed = (fabsf(strafe) >= 0.7f) ? (strafe > 0.0f ? 0.12f : -0.12f) : strafe * 0.35f;
			wantVx += Mth::cos(yawRad) * strafeSpeed;
			wantVz += Mth::sin(yawRad) * strafeSpeed;
		}
		this->motionX += (wantVx - this->motionX) * 0.28f;
		this->motionZ += (wantVz - this->motionZ) * 0.28f;

		if (this->isSitting) {
			this->motionY = (this->motionY * 0.8f) - 0.02f;
		} else if (this->motionY <= 0.14f) {
			int32_t ix = Mth::floor(this->posX);
			int32_t iz = Mth::floor(this->posZ);
			int32_t wy = Mth::floor(this->posY + 0.15f);
			if (this->level->getMaterial(ix, wy, iz) != Material::water) {
				wy = Mth::floor(this->posY + 0.45f);
			}
			int32_t topWaterY = -1;
			for (int32_t step = 0; step < 8 && wy < 127; ++step, ++wy) {
				if (this->level->getMaterial(ix, wy, iz) == Material::water) {
					topWaterY = wy;
				} else if (topWaterY >= 0) {
					break;
				}
			}
			if (topWaterY >= 0) {
				int32_t wdata = this->level->getData(ix, topWaterY, iz);
				float fill = (wdata >= 8) ? 1.0f : (1.0f - (float)(wdata + 1) / 9.0f);
				float surfaceY = (float)topWaterY + fill;
				float targetY = surfaceY - 0.52f + Mth::sin((float)this->field_D8 * 0.24f) * 0.012f;
				float dy = targetY - this->posY;
				float wantVy = Mth::clamp(dy * 0.35f, -0.07f, 0.18f);
				this->motionY += (wantVy - this->motionY) * 0.35f;
			} else {
				this->motionY = (this->motionY * 0.85f) + 0.02f;
			}
		}

		this->move(this->motionX, this->motionY, this->motionZ);
		this->motionX *= 0.88f;
		this->motionY *= 0.86f;
		this->motionZ *= 0.88f;

		if (!this->isSitting && this->isCollidedHorizontally && targetSpeed > 0.01f) {
			int32_t bx = Mth::floor(this->posX - Mth::sin(yawRad) * 0.7f);
			int32_t bz = Mth::floor(this->posZ + Mth::cos(yawRad) * 0.7f);
			int32_t by = Mth::floor(this->boundingBox.minY + 0.45f);
			int32_t tAhead = this->level->getTile(bx, by, bz);
			int32_t tAbove = this->level->getTile(bx, by + 1, bz);
			bool_t solidAhead = (tAhead > 0 && Tile::solid[tAhead]);
			bool_t passableAbove = (tAbove <= 0 || !Tile::solid[tAbove]);
			if (solidAhead && passableAbove) {
				this->motionY = 0.32f;
			} else {
				this->motionY = 0.08f;
			}
		}
		return;
	}
	Animal::travel(strafe, forward);
}

void Wolf::aiStep() {
	Animal::aiStep();
	if (!this->level) {
		return;
	}
	if (this->isInWater()) {
		this->fallDistance = 0.0f;
		if (!this->isSitting) {
			this->field_124 += Mth::wrapDegrees(this->yaw - this->field_124) * 0.25f;
			while (this->field_124 - this->field_128 < -180.0f) this->field_128 -= 360.0f;
			while (this->field_124 - this->field_128 >= 180.0f) this->field_128 += 360.0f;
		}
	}
	if (this->level->isClientMaybe) {
		uint8_t flags = (uint8_t)this->synchedEntityData.getByte(16);
		this->isSitting = (flags & 1) != 0;
		this->isAngry = (flags & 2) != 0;
		this->isTame = (flags & 4) != 0;
		this->isInterested = (flags & 8) != 0;
		return;
	}
	if (this->pacifiedTicks > 0) {
		--this->pacifiedTicks;
	}
	Player* temptPlayer = 0;
	if (this->attackTarget == 0 && !this->isAngry) {
		float bestTemptDistSq = 121.0f;
		for (size_t i = 0; i < this->level->playersMaybe.size(); ++i) {
			Player* p = this->level->playersMaybe[i];
			if (!p || !p->isAlive() || !p->inventory) continue;
			if (this->isTemptFood(p->inventory->getSelected())) {
				float dSq = p->distanceToSqr(this);
				if (dSq < bestTemptDistSq && this->canSee(p)) {
					bestTemptDistSq = dSq;
					temptPlayer = p;
				}
			}
		}
	}
	bool_t wantInterested = (temptPlayer != 0) ? 1 : 0;
	if (wantInterested != this->isInterested) {
		this->isInterested = wantInterested;
		this->updateSynchedFlags();
	}
	if (temptPlayer && !this->isSitting && this->attackTarget == 0 && !this->isInLove()) {
		float dSq = temptPlayer->distanceToSqr(this);
		if (dSq > 5.0f) {
			if (this->repathTicks > 0) {
				--this->repathTicks;
			}
			if (!this->path || this->path->isDone() || this->repathTicks <= 0) {
				this->setPath(this->level->findPath(this, temptPlayer, 16.0f, 1, 0, 0, 1));
				this->repathTicks = 8;
			}
			if (!this->path && !this->isInWater()) {
				float dx = temptPlayer->posX - this->posX;
				float dz = temptPlayer->posZ - this->posZ;
				this->yaw = steerWolfYawToward(this->yaw, dx, dz, 25.0f);
				this->moveForward = this->getSpeed();
			}
		} else {
			if (this->path) {
				this->setPath(0);
			}
			float dx = temptPlayer->posX - this->posX;
			float dz = temptPlayer->posZ - this->posZ;
			this->yaw = steerWolfYawToward(this->yaw, dx, dz, 20.0f);
		}
	}
	if (this->isTame) {
		Player* owner = this->resolveOwner();
		if (this->attackTarget != 0) {
			Entity* tgt = this->level->getEntity(this->attackTarget);
			if (!isWolfValidTargetForOwner(this, owner, tgt) || tgt->distanceToSqr(this) > 1024.0f) {
				this->attackTarget = 0;
				this->angerTicks = 0;
				this->setAngry(0);
			} else if (!this->isAngry) {
				this->setAngry(1);
			}
		} else {
			if (this->isAngry) {
				this->angerTicks = 0;
				this->setAngry(0);
			}
			if (!this->isSitting && owner && owner->isAlive()) {
				Entity* ownerTarget = findOwnerCombatTarget(this, owner);
				if (ownerTarget) {
					this->attackTarget = ownerTarget->entityId;
					this->angerTicks = 600;
					this->setAngry(1);
					this->setPath(this->level->findPath(this, ownerTarget, 16.0f, 1, 0, 0, 1));
				}
			}
		}
		if (this->isSitting) {
			this->setPath(0);
			this->attackTarget = 0;
			this->moveForward = 0.0f;
			this->moveStrafe = 0.0f;
			this->isJumping = 0;
			this->motionX *= 0.6f;
			this->motionZ *= 0.6f;
		} else if (this->attackTarget == 0 && owner && owner->isAlive()) {
			float distSq = owner->distanceToSqr(this);
			float tpDistSq = this->isInWater() ? 576.0f : 144.0f;
			if (distSq > tpDistSq && owner->onGround && !owner->isInWater()) {
				int32_t ox = Mth::floor(owner->posX);
				int32_t oy = Mth::floor(owner->boundingBox.minY + 0.5f);
				int32_t oz = Mth::floor(owner->posZ);
				bool_t teleported = 0;
				for (int32_t dx = -2; dx <= 2 && !teleported; ++dx) {
					for (int32_t dz = -2; dz <= 2 && !teleported; ++dz) {
						if (dx >= -1 && dx <= 1 && dz >= -1 && dz <= 1) continue;
						int32_t tx = ox + dx;
						int32_t tz = oz + dz;
						int32_t tBelow = this->level->getTile(tx, oy - 1, tz);
						int32_t tAt = this->level->getTile(tx, oy, tz);
						int32_t tAbove = this->level->getTile(tx, oy + 1, tz);
						if (tBelow > 0 && Tile::solid[tBelow] && (tAt <= 0 || !Tile::solid[tAt]) && (tAbove <= 0 || !Tile::solid[tAbove]) && this->level->getMaterial(tx, oy, tz) != Material::water && this->level->getMaterial(tx, oy, tz) != Material::lava) {
							this->moveTo((float)tx + 0.5f, (float)oy, (float)tz + 0.5f, this->yaw, this->pitch);
							this->setPath(0);
							teleported = 1;
						}
					}
				}
			} else if (!this->isInWater() && !this->isInLove() && !temptPlayer && distSq > (owner->isInWater() ? 9.0f : 16.0f)) {
				if (this->repathTicks > 0) {
					--this->repathTicks;
				}
				if (!this->path || this->path->isDone() || this->repathTicks <= 0) {
					this->setPath(this->level->findPath(this, owner, 16.0f, 1, 0, 0, 1));
					this->repathTicks = 8;
				}
			} else if (!this->isInWater() && !this->isInLove() && !temptPlayer && distSq <= 6.25f) {
				if (this->path) {
					this->setPath(0);
				}
			}
		}
	}
	if (!this->isSitting && this->attackTarget == 0 && this->health > 0) {
		Player* owner = this->isTame ? this->resolveOwner() : 0;
		Player* watchPlayer = (owner && owner->isAlive() && owner->distanceToSqr(this) <= 256.0f) ? owner : this->level->getNearestPlayer(this, 10.0f);
		ItemInstance* held = (watchPlayer && watchPlayer->inventory) ? watchPlayer->inventory->getSelected() : 0;
		bool_t holdingTreat = (held && (held->itemClass == Item::bone || this->isMeatItem(held))) ? 1 : 0;
		bool_t isMoving = (this->path != 0 || fabsf(this->moveForward) > 0.02f || ((this->posX - this->prevX) * (this->posX - this->prevX) + (this->posZ - this->prevZ) * (this->posZ - this->prevZ)) > 0.0008f) ? 1 : 0;
		if (this->sitLookTicks > 0) {
			--this->sitLookTicks;
		}
		if (this->sitLookTicks <= 0) {
			if (holdingTreat && watchPlayer) {
				this->sitLookMode = 0;
				this->sitLookTicks = 14 + (this->random.genrand_int32() % 18);
				this->sitYawOffset = 0.0f;
				this->sitPitchTarget = -4.0f;
			} else if (isMoving) {
				int32_t roll = this->random.genrand_int32() % 100;
				if (watchPlayer && this->sitLookMode != 0 && roll < 38) {
					this->sitLookMode = 0;
					this->sitLookTicks = 16 + (this->random.genrand_int32() % 24);
					this->sitYawOffset = (this->random.nextFloat() - 0.5f) * 8.0f;
					this->sitPitchTarget = (this->random.nextFloat() - 0.5f) * 5.0f;
				} else if (roll < 64) {
					this->sitLookMode = 1;
					this->sitLookTicks = 20 + (this->random.genrand_int32() % 32);
					this->sitYawOffset = 0.0f;
					this->sitPitchTarget = 0.0f;
				} else if (roll < 85) {
					this->sitLookMode = 2;
					this->sitLookTicks = 14 + (this->random.genrand_int32() % 20);
					float side = (this->sitYawOffset >= 0.0f) ? -1.0f : 1.0f;
					this->sitYawOffset = side * (22.0f + this->random.nextFloat() * 26.0f);
					this->sitPitchTarget = (this->random.nextFloat() - 0.45f) * 12.0f;
				} else {
					this->sitLookMode = 3;
					this->sitLookTicks = 14 + (this->random.genrand_int32() % 20);
					this->sitYawOffset = (this->random.nextFloat() - 0.5f) * 20.0f;
					this->sitPitchTarget = 16.0f + this->random.nextFloat() * 11.0f;
				}
			} else {
				int32_t roll = this->random.genrand_int32() % 100;
				if (watchPlayer && this->sitLookMode != 0 && roll < 42) {
					this->sitLookMode = 0;
					this->sitLookTicks = 24 + (this->random.genrand_int32() % 42);
					this->sitYawOffset = (this->random.nextFloat() - 0.5f) * 10.0f;
					this->sitPitchTarget = (this->random.nextFloat() - 0.5f) * 6.0f;
				} else if (roll < 66) {
					this->sitLookMode = 1;
					this->sitLookTicks = 24 + (this->random.genrand_int32() % 40);
					this->sitYawOffset = 0.0f;
					this->sitPitchTarget = 0.0f;
				} else if (roll < 86) {
					this->sitLookMode = 2;
					this->sitLookTicks = 16 + (this->random.genrand_int32() % 28);
					float side = (this->sitYawOffset >= 0.0f) ? -1.0f : 1.0f;
					this->sitYawOffset = side * (25.0f + this->random.nextFloat() * 35.0f);
					this->sitPitchTarget = (this->random.nextFloat() - 0.45f) * 14.0f;
				} else {
					this->sitLookMode = 3;
					this->sitLookTicks = 16 + (this->random.genrand_int32() % 26);
					this->sitYawOffset = (this->random.nextFloat() - 0.5f) * 26.0f;
					this->sitPitchTarget = 16.0f + this->random.nextFloat() * 12.0f;
				}
			}
		}
		float targetYawOff = this->sitYawOffset;
		float targetPitchOff = this->sitPitchTarget;
		if ((holdingTreat || this->sitLookMode == 0) && watchPlayer && watchPlayer->isAlive()) {
			float dx = watchPlayer->posX - this->posX;
			float dz = watchPlayer->posZ - this->posZ;
			float dy = (watchPlayer->boundingBox.minY + 1.35f) - (this->posY + this->getHeadHeight());
			float horiz = Mth::sqrt(dx * dx + dz * dz);
			if (horiz < 0.001f) horiz = 0.001f;
			float ownerYaw = (atan2f(dz, dx) * 180.0f / 3.14159265f) - 90.0f;
			float ownerPitch = -(atan2f(dy, horiz) * 180.0f / 3.14159265f);
			ownerPitch = Mth::clamp(ownerPitch, -32.0f, 32.0f);
			float relYaw = Mth::wrapDegrees(ownerYaw - this->yaw);
			if (!isMoving && fabsf(relYaw) > 38.0f) {
				this->yaw = Mth::wrapDegrees(this->yaw + Mth::clamp(relYaw * 0.22f, -12.0f, 12.0f));
				relYaw = Mth::wrapDegrees(ownerYaw - this->yaw);
			}
			if (fabsf(relYaw) <= 75.0f || !isMoving) {
				targetYawOff = Mth::clamp(relYaw + (holdingTreat ? 0.0f : this->sitYawOffset), -56.0f, 56.0f);
				targetPitchOff = Mth::clamp(ownerPitch + (holdingTreat ? -3.0f : this->sitPitchTarget), -32.0f, 32.0f);
			}
		} else if (!isMoving && fabsf(this->sitYawOffset) > 34.0f) {
			this->yaw = Mth::wrapDegrees(this->yaw + Mth::clamp(this->sitYawOffset * 0.08f, -5.0f, 5.0f));
		}
		this->curHeadYawOffset += Mth::clamp((targetYawOff - this->curHeadYawOffset) * 0.25f, -11.0f, 11.0f);
		this->curHeadPitchOffset += Mth::clamp((targetPitchOff - this->curHeadPitchOffset) * 0.25f, -7.0f, 7.0f);
		float desiredHeadYaw = Mth::wrapDegrees(this->yaw + this->curHeadYawOffset);
		float bodyRel = Mth::wrapDegrees(desiredHeadYaw - this->field_124);
		bodyRel = Mth::clamp(bodyRel, -58.0f, 58.0f);
		this->headYaw = Mth::wrapDegrees(this->field_124 + bodyRel);
		this->pitch = Mth::clamp(this->curHeadPitchOffset, -32.0f, 32.0f);
	}
	if (this->isTame && (this->isSitting || this->attackTarget == 0)) {
		return;
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

static void consumeWolfSelectedItem(Player* player, ItemInstance* sel) {
	if (!player || !sel || player->abilities.instabuild) return;
	--sel->count;
	if (sel->count <= 0) {
		player->inventory->clearSlot(player->inventory->selectedSlot);
	}
}

static void spawnWolfHeartParticles(Wolf* wolf) {
	if (!wolf || !wolf->level) return;
	for (int32_t i = 0; i < 7; ++i) {
		float gx = wolf->random.nextGaussian() * 0.02f;
		float gy = wolf->random.nextGaussian() * 0.02f;
		float gz = wolf->random.nextGaussian() * 0.02f;
		float px = wolf->posX + (wolf->random.nextFloat() * wolf->entityWidth * 2.0f) - wolf->entityWidth;
		float py = wolf->posY + 0.5f + (wolf->random.nextFloat() * wolf->entityHeight);
		float pz = wolf->posZ + (wolf->random.nextFloat() * wolf->entityWidth * 2.0f) - wolf->entityWidth;
		wolf->level->addParticle(PT_HEART, px, py, pz, gx, gy, gz, 0);
	}
}

bool_t Wolf::isFood(const ItemInstance* item) {
	return this->isMeatItem(item);
}

bool_t Wolf::interactWithPlayer(Player* player) {
	if (!player || !player->inventory) {
		return Animal::interactWithPlayer(player);
	}
	ItemInstance* sel = player->inventory->getSelected();
	if (!this->isTame) {
		if (sel && sel->itemClass == Item::bone) {
			consumeWolfSelectedItem(player, sel);
			if (this->level && !this->level->isClientMaybe) {
				this->isTame = 1;
				this->isSitting = 0;
				this->setAngry(0);
				this->attackTarget = 0;
				this->angerTicks = 0;
				this->inPanicTicksMaybe = 0;
				this->pacifiedTicks = 0;
				this->setLastHurtByMob(0);
				this->ownerName = player->username;
				this->ownerEntityId = player->entityId;
				this->health = this->getMaxHealth();
				this->setPath(0);
				this->updateSynchedFlags();
				this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
			}
			spawnWolfHeartParticles(this);
			return 1;
		}
		if (sel && this->isMeatItem(sel)) {
			bool_t usedUntamedFood = 0;
			if (this->health > 0 && this->health < this->getMaxHealth()) {
				usedUntamedFood = 1;
				consumeWolfSelectedItem(player, sel);
				if (this->level && !this->level->isClientMaybe) {
					int32_t nextHp = this->health + 6;
					int32_t maxHp = this->getMaxHealth();
					this->health = (nextHp > maxHp) ? maxHp : nextHp;
					this->inPanicTicksMaybe = 0;
					if (!this->isBaby() && this->getAge() == 0 && !this->isInLove()) {
						this->inLove = 600;
						this->attackTarget = 0;
						this->setPath(0);
					}
					this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
				}
			} else if (!this->isBaby() && this->getAge() == 0 && !this->isInLove()) {
				usedUntamedFood = 1;
				consumeWolfSelectedItem(player, sel);
				if (this->level && !this->level->isClientMaybe) {
					this->inLove = 600;
					this->inPanicTicksMaybe = 0;
					this->attackTarget = 0;
					this->setPath(0);
					this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
				}
			}
			if (usedUntamedFood) {
				spawnWolfHeartParticles(this);
				return 1;
			}
		}
		return Animal::interactWithPlayer(player);
	}

	bool_t isMeat = (sel && this->isMeatItem(sel)) ? 1 : 0;
	bool_t isBone = (sel && sel->itemClass == Item::bone) ? 1 : 0;
	if (isMeat || isBone) {
		bool_t usedFood = 0;
		if (this->health > 0 && this->health < this->getMaxHealth()) {
			usedFood = 1;
			consumeWolfSelectedItem(player, sel);
			if (this->level && !this->level->isClientMaybe) {
				int32_t healAmount = isBone ? 4 : 6;
				int32_t nextHp = this->health + healAmount;
				int32_t maxHp = this->getMaxHealth();
				this->health = (nextHp > maxHp) ? maxHp : nextHp;
				if (isMeat && !this->isBaby() && this->getAge() == 0 && !this->isInLove()) {
					this->inLove = 600;
					this->isSitting = 0;
					this->attackTarget = 0;
					this->setPath(0);
					this->updateSynchedFlags();
				}
				this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
			}
		} else if (!this->isBaby() && this->getAge() == 0 && !this->isInLove()) {
			usedFood = 1;
			consumeWolfSelectedItem(player, sel);
			if (this->level && !this->level->isClientMaybe) {
				this->inLove = 600;
				this->isSitting = 0;
				this->attackTarget = 0;
				this->setPath(0);
				this->updateSynchedFlags();
				this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
			}
		} else if (this->isBaby()) {
			usedFood = 1;
			consumeWolfSelectedItem(player, sel);
			if (this->level && !this->level->isClientMaybe) {
				int32_t age = this->getAge();
				if (age < 0) {
					int32_t boost = (-age) / 10;
					if (boost < 20) boost = 20;
					int32_t nextAge = age + boost;
					if (nextAge > 0) nextAge = 0;
					this->setAge(nextAge);
				}
				this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
			}
		}
		if (usedFood) {
			spawnWolfHeartParticles(this);
			return 1;
		}
	}

	if (this->isOwnerPlayer(player)) {
		if (this->level && !this->level->isClientMaybe) {
			this->isSitting = this->isSitting ? 0 : 1;
			this->isJumping = 0;
			this->setPath(0);
			this->attackTarget = 0;
			this->angerTicks = 0;
			this->setAngry(0);
			this->ownerName = player->username;
			this->ownerEntityId = player->entityId;
			this->updateSynchedFlags();
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
	Entity* threat = attacker;
	if (attacker && attacker->getEntityTypeId() == 80 && ((Arrow*)attacker)->shooterEID != 0 && this->level) {
		Entity* shooter = this->level->getEntity(((Arrow*)attacker)->shooterEID);
		if (shooter) {
			threat = shooter;
		}
	}
	if (this->isTame) {
		this->isSitting = 0;
		this->setPath(0);
		Player* owner = this->resolveOwner();
		bool_t hitByOwner = threat && threat->isPlayer() && this->isOwnerPlayer((Player*)threat);
		if (!hitByOwner && threat && isWolfValidTargetForOwner(this, owner, threat)) {
			this->angerTicks = 600;
			this->setAngry(1);
			this->attackTarget = threat->entityId;
		} else {
			this->updateSynchedFlags();
		}
		return 1;
	}
	Player* playerAttacker = (threat && threat->isPlayer()) ? (Player*)threat : 0;
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
						if (!mate->isTame && !mate->isSitting && mate->pacifiedTicks <= 0) {
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
	if (!this->level || this->level->isClientMaybe || this->isSitting) {
		return 0;
	}
	if (this->isTame) {
		Player* owner = this->resolveOwner();
		Entity* ownerTarget = findOwnerCombatTarget(this, owner);
		if (ownerTarget) {
			this->angerTicks = 600;
			this->setAngry(1);
			return ownerTarget;
		}
		return 0;
	}
	if (!this->isAngry || this->pacifiedTicks > 0) {
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
	if (target && target->isAlive() && this->attackTime <= 0 && dist < 2.1f && target->boundingBox.maxY > this->boundingBox.minY - 0.5f && target->boundingBox.minY < this->boundingBox.maxY + 0.5f) {
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
	this->ownerName = tag->getString("Owner");
	if (this->isTame && this->isAngry) {
		this->isAngry = 0;
	}
	this->angerTicks = this->isAngry ? 600 : 0;
	this->updateSynchedFlags();
}

void Wolf::addAdditonalSaveData(CompoundTag* tag) {
	Animal::addAdditonalSaveData(tag);
	tag->putInt("WolfType", this->wolfType);
	tag->putBoolean("Angry", this->isAngry);
	tag->putBoolean("Tame", this->isTame);
	tag->putBoolean("Sitting", this->isSitting);
	tag->putString("Owner", this->ownerName);
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
	if (this->isTame) {
		baby->isTame = 1;
		baby->ownerName = this->ownerName;
		baby->ownerEntityId = this->ownerEntityId;
		baby->health = baby->getMaxHealth();
		baby->updateSynchedFlags();
	}
	return baby;
}
