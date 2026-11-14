#include <entity/Ocelot.hpp>
#include <entity/Wolf.hpp>
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
#include <tile/material/Material.hpp>
#include <math/Mth.hpp>
#include <cmath>

static std::string _ocelotTex = "mob/ocelot.png";

Ocelot::Ocelot(Level* level)
	: Animal(level) {
	this->entityRenderId = OCELOT;
	this->setSize(0.6f, 0.7f);
	this->isTrusting = 0;
	this->isSitting = 0;
	this->isAngry = 0;
	this->isInterested = 0;
	this->ownerName = "";
	this->ownerEntityId = 0;
	this->repathTicks = 0;
	this->angerTicks = 0;
	this->sitLookMode = 0;
	this->sitLookTicks = 0;
	this->sitYawOffset = 0.0f;
	this->sitPitchTarget = 0.0f;
	this->curHeadYawOffset = 0.0f;
	this->curHeadPitchTarget = 0.0f;
	this->synchedEntityData.define<int8_t>(16, 0);
}

void Ocelot::updateSynchedFlags() {
	int8_t flags = 0;
	if (this->isSitting) flags |= 1;
	if (this->isAngry) flags |= 2;
	if (this->isTrusting) flags |= 4;
	if (this->isInterested) flags |= 8;
	this->synchedEntityData.set<int8_t>(16, flags);
}

bool_t Ocelot::isFishItem(const ItemInstance* item) const {
	if (!item || !item->itemClass) return 0;
	Item* cls = item->itemClass;
	return cls == Item::fish_raw || cls == Item::fish_cooked || cls == Item::salmon_raw || cls == Item::salmon_cooked || cls == Item::clownfish || cls == Item::pufferfish;
}

bool_t Ocelot::isOwnerPlayer(const Player* p) const {
	if (!p || !this->isTrusting) return 0;
	if (this->ownerEntityId != 0 && p->entityId == this->ownerEntityId) return 1;
	if (!this->ownerName.empty() && p->username == this->ownerName) return 1;
	return 0;
}

static bool_t isOcelotFishEntity(Entity* e) {
	if (!e || !e->isAlive()) return 0;
	int32_t t = e->getEntityTypeId();
	return t >= 27 && t <= 30;
}

static bool_t isOcelotValidTargetForOwner(Ocelot* ocelot, Player* owner, Entity* target) {
	if (!ocelot || !owner || !target || !target->isAlive() || target == ocelot || target == owner) return 0;
	int32_t typeId = target->getEntityTypeId();
	if (!target->isMob() && !target->isPlayer()) return 0;
	if (typeId == 33) return 0;
	if (typeId == 14) {
		Wolf* otherWolf = (Wolf*)target;
		if (otherWolf->isTame && otherWolf->resolveOwner() == owner) return 0;
	}
	if (typeId == 38) {
		Fox* otherFox = (Fox*)target;
		if (otherFox->isTame && otherFox->resolveOwner() == owner) return 0;
	}
	if (typeId == 22) {
		Ocelot* otherOcelot = (Ocelot*)target;
		if (otherOcelot->isTrusting && otherOcelot->resolveOwner() == owner) return 0;
	}
	if (target->isPlayer() && ((Player*)target)->abilities.invulnerable) return 0;
	return 1;
}

static Entity* findOcelotOwnerCombatTarget(Ocelot* ocelot, Player* owner) {
	if (!ocelot || !owner || !ocelot->level) return 0;
	int32_t tickNow = ocelot->level->levelData.getTime();
	if (owner->field_C58 > 0 && (tickNow - owner->lastHurtMobTicks) < 200) {
		Entity* hurtTarget = ocelot->level->getEntity(owner->field_C58);
		if (isOcelotValidTargetForOwner(ocelot, owner, hurtTarget) && hurtTarget->distanceToSqr(ocelot) <= 576.0f) {
			return hurtTarget;
		}
	}
	if (owner->field_C5C > 0 && (tickNow - owner->field_C60) < 200) {
		Entity* attacker = ocelot->level->getEntity(owner->field_C5C);
		if (isOcelotValidTargetForOwner(ocelot, owner, attacker) && attacker->distanceToSqr(ocelot) <= 576.0f) {
			return attacker;
		}
	}
	Entity* lastBy = owner->getLastHurtByMob();
	if (isOcelotValidTargetForOwner(ocelot, owner, lastBy) && lastBy->distanceToSqr(ocelot) <= 576.0f) {
		return lastBy;
	}
	std::vector<Entity*>* nearby = ocelot->level->getEntities(ocelot, {ocelot->boundingBox.minX - 18.0f, ocelot->boundingBox.minY - 8.0f, ocelot->boundingBox.minZ - 18.0f, ocelot->boundingBox.maxX + 18.0f, ocelot->boundingBox.maxY + 8.0f, ocelot->boundingBox.maxZ + 18.0f});
	if (nearby) {
		for (size_t i = 0; i < nearby->size(); ++i) {
			Entity* e = nearby->at(i);
			if (!isOcelotValidTargetForOwner(ocelot, owner, e)) continue;
			Mob* m = (Mob*)e;
			if (m->getLastHurtByMob() == owner && m->hurtTime > 0) {
				return m;
			}
			if (m->getEntityTypeId() >= 32 && m->getEntityTypeId() < 64) {
				PathfinderMob* pm = (PathfinderMob*)m;
				if (pm->attackTarget == owner->entityId && pm->distanceToSqr(owner) <= 256.0f) {
					return m;
				}
			}
		}
	}
	return 0;
}

static float steerOcelotYawToward(float curYaw, float dx, float dz, float maxTurn) {
	float targetYaw = (atan2f(dz, dx) * 180.0f / 3.14159265f) - 90.0f;
	float diff = Mth::wrapDegrees(targetYaw - curYaw);
	return Mth::wrapDegrees(curYaw + Mth::clamp(diff, -maxTurn, maxTurn));
}

Player* Ocelot::resolveOwner() {
	if (!this->level || !this->isTrusting) return 0;
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

Ocelot::~Ocelot() {
}

int32_t Ocelot::getEntityTypeId() const {
	return 22;
}

std::string* Ocelot::getTexture() {
	return &_ocelotTex;
}

int32_t Ocelot::getMaxHealth() {
	return this->isTrusting ? 20 : 10;
}

float Ocelot::getBaseSpeed() {
	return 0.35f;
}

int32_t Ocelot::getAmbientSoundInterval() {
	return 160;
}

const char_t* Ocelot::getAmbientSound() {
	if (this->isAngry) {
		return "mob.cat.hiss";
	}
	if (this->random.genrand_int32() % 2 == 0) {
		return "mob.cat.meow";
	}
	return "mob.cat.purr";
}

std::string Ocelot::getHurtSound() {
	return "mob.cat.hitt";
}

std::string Ocelot::getDeathSound() {
	return "mob.cat.hitt";
}

void Ocelot::travel(float strafe, float forward) {
	if (this->isInWater() && this->level) {
		float yawRad = this->yaw * (3.14159265f / 180.0f);
		float targetSpeed = 0.0f;
		if (!this->isSitting && forward > 0.01f) {
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
		if (!this->isSitting && fabsf(strafe) > 0.01f) {
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
				float targetY = surfaceY - 0.46f + Mth::sin((float)this->field_D8 * 0.24f) * 0.01f;
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

float Ocelot::getWalkTargetValue(int32_t x, int32_t y, int32_t z) {
	if (this->isTrusting && this->level) {
		Player* owner = this->resolveOwner();
		if ((owner && owner->isInWater()) || this->isInWater() || this->attackTarget != 0) {
			int32_t tileAt = this->level->getTile(x, y, z);
			int32_t tileBelow = this->level->getTile(x, y - 1, z);
			if (tileAt == Tile::water->blockID || tileAt == Tile::calmWater->blockID || tileBelow == Tile::water->blockID || tileBelow == Tile::calmWater->blockID) {
				return 10.0f;
			}
		}
	}
	return Animal::getWalkTargetValue(x, y, z);
}

void Ocelot::updateAi() {
	if (this->isSitting) {
		this->setPath(0);
		this->attackTarget = 0;
		this->moveForward = 0.0f;
		this->moveStrafe = 0.0f;
		this->isJumping = 0;
		if (!this->level) return;

		Player* focusPlayer = this->resolveOwner();
		if (!focusPlayer || !focusPlayer->isAlive() || focusPlayer->distanceToSqr(this) > 256.0f) {
			Player* nearPlayer = this->level->getNearestPlayer(this, 10.0f);
			if (nearPlayer && nearPlayer->isAlive()) {
				focusPlayer = nearPlayer;
			}
		}

		if (this->sitLookTicks > 0) {
			--this->sitLookTicks;
		}
		if (this->sitLookTicks <= 0) {
			int32_t roll = this->random.genrand_int32() % 100;
			if (focusPlayer && roll < 65) {
				this->sitLookMode = 0;
				this->sitLookTicks = 28 + (this->random.genrand_int32() % 45);
				this->sitYawOffset = ((this->random.nextFloat() - 0.5f) * 12.0f);
				this->sitPitchTarget = ((this->random.nextFloat() - 0.5f) * 8.0f);
			} else if (roll < 88) {
				this->sitLookMode = 1;
				this->sitLookTicks = 18 + (this->random.genrand_int32() % 32);
				float sideSign = (this->random.genrand_int32() & 1) ? 1.0f : -1.0f;
				this->sitYawOffset = sideSign * (20.0f + this->random.nextFloat() * 30.0f);
				this->sitPitchTarget = -6.0f + this->random.nextFloat() * 16.0f;
			} else {
				this->sitLookMode = 2;
				this->sitLookTicks = 16 + (this->random.genrand_int32() % 24);
				this->sitYawOffset = (this->random.nextFloat() - 0.5f) * 18.0f;
				this->sitPitchTarget = (this->random.genrand_int32() & 1) ? (-14.0f - this->random.nextFloat() * 8.0f) : (10.0f + this->random.nextFloat() * 10.0f);
			}
		}

		float desiredYaw = this->field_124;
		float desiredPitch = this->sitPitchTarget;
		if (focusPlayer && this->sitLookMode == 0) {
			float dx = focusPlayer->posX - this->posX;
			float dz = focusPlayer->posZ - this->posZ;
			float dy = (focusPlayer->boundingBox.minY + focusPlayer->getHeadHeight()) - (this->posY + this->getHeadHeight());
			float horiz = Mth::sqrt(dx * dx + dz * dz);
			float playerYaw = (atan2f(dz, dx) * 180.0f / 3.14159265f) - 90.0f;
			float playerPitch = -(atan2f(dy, horiz) * 180.0f / 3.14159265f);
			desiredYaw = playerYaw + this->sitYawOffset;
			desiredPitch = Mth::clamp(playerPitch + this->sitPitchTarget * 0.5f, -35.0f, 35.0f);

			float bodyDiff = Mth::wrapDegrees(playerYaw - this->field_124);
			if (fabsf(bodyDiff) > 42.0f) {
				this->field_124 = Mth::wrapDegrees(this->field_124 + Mth::clamp(bodyDiff * 0.14f, -4.0f, 4.0f));
			}
		} else if (focusPlayer) {
			float dx = focusPlayer->posX - this->posX;
			float dz = focusPlayer->posZ - this->posZ;
			float baseYaw = (atan2f(dz, dx) * 180.0f / 3.14159265f) - 90.0f;
			desiredYaw = baseYaw + this->sitYawOffset;
		} else {
			desiredYaw = this->field_124 + this->sitYawOffset;
		}

		float clampedHeadRel = Mth::clamp(Mth::wrapDegrees(desiredYaw - this->field_124), -56.0f, 56.0f);
		float finalYaw = this->field_124 + clampedHeadRel;
		float yawStep = Mth::wrapDegrees(finalYaw - this->yaw);
		this->yaw = Mth::wrapDegrees(this->yaw + Mth::clamp(yawStep * 0.25f, -7.5f, 7.5f));
		this->headYaw = this->yaw;

		float pitchStep = desiredPitch - this->pitch;
		this->pitch = Mth::clamp(this->pitch + Mth::clamp(pitchStep * 0.22f, -5.0f, 5.0f), -35.0f, 35.0f);
		return;
	}

	if (this->isInWater() && this->level) {
		Entity* swimTarget = 0;
		if (this->attackTarget != 0) {
			Entity* tgt = this->level->getEntity(this->attackTarget);
			if (tgt && tgt->isAlive()) {
				swimTarget = tgt;
			}
		} else if (this->isTrusting) {
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
			this->yaw = steerOcelotYawToward(this->yaw, dx, dz, 16.0f);
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

	if (this->isTrusting && this->level) {
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
			this->yaw = steerOcelotYawToward(this->yaw, dx, dz, 30.0f);
			this->moveForward = this->getSpeed();
			this->lookAt(directTgt, 30.0f, 30.0f);
			if (this->isCollidedHorizontally) {
				this->isJumping = 1;
			}
		}
	}

	if (this->attackTarget != 0 && this->level) {
		Entity* tgt = this->level->getEntity(this->attackTarget);
		if (isOcelotFishEntity(tgt)) {
			if (!this->path) {
				float dx = tgt->posX - this->posX;
				float dz = tgt->posZ - this->posZ;
				this->yaw = steerOcelotYawToward(this->yaw, dx, dz, 30.0f);
				this->moveForward = this->getSpeed();
				this->lookAt(tgt, 30.0f, 30.0f);
				if (this->isInWater()) {
					float dy = tgt->posY - this->posY;
					this->motionY += Mth::clamp(dy * 0.08f, -0.05f, 0.08f);
				}
			}
		} else if (tgt && tgt->isAlive()) {
			float dist = tgt->distanceTo(this);
			if (dist > 2.0f && dist < 5.5f && this->onGround && !this->isInWater() && (this->random.genrand_int32() % 10) == 0) {
				float dx = tgt->posX - this->posX;
				float dz = tgt->posZ - this->posZ;
				float d = Mth::sqrt(dx * dx + dz * dz);
				if (d > 0.001f) {
					this->motionX = (dx / d) * 0.45f + this->motionX * 0.2f;
					this->motionZ = (dz / d) * 0.45f + this->motionZ * 0.2f;
					this->motionY = 0.36f;
				}
			}
		}
	}
}

void Ocelot::aiStep() {
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
		this->isTrusting = (flags & 4) != 0;
		this->isInterested = (flags & 8) != 0;
		return;
	}

	if (this->angerTicks > 0) {
		--this->angerTicks;
		if (this->angerTicks == 0) {
			this->isAngry = 0;
			this->updateSynchedFlags();
		}
	}

	Player* temptPlayer = 0;
	if (!this->isAngry && this->attackTarget == 0 && !this->isInLove()) {
		float bestTemptDistSq = 144.0f;
		for (size_t i = 0; i < this->level->playersMaybe.size(); ++i) {
			Player* p = this->level->playersMaybe[i];
			if (!p || !p->isAlive() || !p->inventory) continue;
			ItemInstance* held = p->inventory->getSelected();
			if (!this->isFood(held)) continue;
			float dSq = p->distanceToSqr(this);
			if (dSq < bestTemptDistSq && this->canSee(p)) {
				bestTemptDistSq = dSq;
				temptPlayer = p;
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
			if (this->repathTicks > 0) --this->repathTicks;
			if (!this->path || this->path->isDone() || this->repathTicks <= 0) {
				this->setPath(this->level->findPath(this, temptPlayer, 16.0f, 1, 0, 0, 1));
				this->repathTicks = 8;
			}
			if (!this->path && !this->isInWater()) {
				float dx = temptPlayer->posX - this->posX;
				float dz = temptPlayer->posZ - this->posZ;
				this->yaw = steerOcelotYawToward(this->yaw, dx, dz, 25.0f);
				this->moveForward = this->getSpeed();
			}
		} else {
			if (this->path) {
				this->setPath(0);
			}
			float dx = temptPlayer->posX - this->posX;
			float dz = temptPlayer->posZ - this->posZ;
			this->yaw = steerOcelotYawToward(this->yaw, dx, dz, 20.0f);
		}
	}

	if (this->isTrusting) {
		Player* owner = this->resolveOwner();
		if (!this->isSitting && owner && owner->isAlive()) {
			Entity* ownerTarget = findOcelotOwnerCombatTarget(this, owner);
			if (ownerTarget) {
				if (this->attackTarget != ownerTarget->entityId) {
					this->attackTarget = ownerTarget->entityId;
					this->setPath(this->level->findPath(this, ownerTarget, 20.0f, 1, 0, 0, 1));
					this->repathTicks = 8;
				}
				if (!this->isAngry) {
					this->isAngry = 1;
				}
				this->angerTicks = 200;
				this->updateSynchedFlags();
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
		} else if (owner && owner->isAlive()) {
			float distSq = owner->distanceToSqr(this);
			if (this->attackTarget != 0) {
				Entity* curTarget = this->level->getEntity(this->attackTarget);
				if (!isOcelotValidTargetForOwner(this, owner, curTarget) && !isOcelotFishEntity(curTarget)) {
					this->attackTarget = 0;
					this->setPath(0);
					if (this->isAngry) {
						this->isAngry = 0;
						this->angerTicks = 0;
						this->updateSynchedFlags();
					}
				} else if (distSq > 576.0f) {
					this->attackTarget = 0;
					this->setPath(0);
				} else if (!this->isInWater()) {
					if (this->repathTicks > 0) --this->repathTicks;
					if (!this->path || this->path->isDone() || this->repathTicks <= 0) {
						this->setPath(this->level->findPath(this, curTarget, 20.0f, 1, 0, 0, 1));
						this->repathTicks = 8;
					}
				}
			}
			if (this->attackTarget == 0 && !this->isInLove() && !temptPlayer) {
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
				} else if (!this->isInWater() && distSq > (owner->isInWater() ? 9.0f : 16.0f)) {
					if (this->repathTicks > 0) {
						--this->repathTicks;
					}
					if (!this->path || this->path->isDone() || this->repathTicks <= 0) {
						this->setPath(this->level->findPath(this, owner, 16.0f, 1, 0, 0, 1));
						this->repathTicks = 8;
					}
				} else if (!this->isInWater() && distSq <= 6.25f) {
					if (this->path) {
						this->setPath(0);
					}
				}
			}
		}
	}

	if (!this->isSitting && !this->isInWater() && this->attackTarget == 0 && this->health > 0) {
		Player* owner = this->isTrusting ? this->resolveOwner() : 0;
		Player* watchPlayer = (owner && owner->isAlive() && owner->distanceToSqr(this) <= 256.0f) ? owner : this->level->getNearestPlayer(this, 10.0f);
		ItemInstance* held = (watchPlayer && watchPlayer->inventory) ? watchPlayer->inventory->getSelected() : 0;
		bool_t holdingTreat = (held && this->isFood(held)) ? 1 : 0;
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
					this->sitPitchTarget = 14.0f + this->random.nextFloat() * 10.0f;
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
					this->sitYawOffset = side * (24.0f + this->random.nextFloat() * 32.0f);
					this->sitPitchTarget = (this->random.nextFloat() - 0.45f) * 14.0f;
				} else {
					this->sitLookMode = 3;
					this->sitLookTicks = 16 + (this->random.genrand_int32() % 26);
					this->sitYawOffset = (this->random.nextFloat() - 0.5f) * 24.0f;
					this->sitPitchTarget = 14.0f + this->random.nextFloat() * 10.0f;
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
				targetYawOff = Mth::clamp(relYaw + (holdingTreat ? 0.0f : this->sitYawOffset), -54.0f, 54.0f);
				targetPitchOff = Mth::clamp(ownerPitch + (holdingTreat ? -3.0f : this->sitPitchTarget), -30.0f, 30.0f);
			}
		} else if (!isMoving && fabsf(this->sitYawOffset) > 34.0f) {
			this->yaw = Mth::wrapDegrees(this->yaw + Mth::clamp(this->sitYawOffset * 0.08f, -5.0f, 5.0f));
		}
		this->curHeadYawOffset += Mth::clamp((targetYawOff - this->curHeadYawOffset) * 0.25f, -10.0f, 10.0f);
		this->curHeadPitchTarget += Mth::clamp((targetPitchOff - this->curHeadPitchTarget) * 0.25f, -6.5f, 6.5f);
		float desiredHeadYaw = Mth::wrapDegrees(this->yaw + this->curHeadYawOffset);
		float bodyRel = Mth::clamp(Mth::wrapDegrees(desiredHeadYaw - this->field_124), -54.0f, 54.0f);
		this->headYaw = Mth::wrapDegrees(this->field_124 + bodyRel);
		this->pitch = Mth::clamp(this->curHeadPitchTarget, -30.0f, 30.0f);
	}
}

bool_t Ocelot::hurt(Entity* attacker, int32_t dmg) {
	if (attacker && attacker->getEntityTypeId() == 80 && ((Arrow*)attacker)->shooterEID != 0 && this->level) {
		Entity* shooter = this->level->getEntity(((Arrow*)attacker)->shooterEID);
		if (shooter) attacker = shooter;
	}
	if (!Animal::hurt(attacker, dmg)) {
		return 0;
	}
	this->isSitting = 0;
	this->isInterested = 0;
	if (attacker && attacker != this && this->isTrusting) {
		Player* owner = this->resolveOwner();
		if (attacker != owner && (!attacker->isPlayer() || !this->isOwnerPlayer((Player*)attacker))) {
			this->attackTarget = attacker->entityId;
			this->isAngry = 1;
			this->angerTicks = 400;
			this->inPanicTicksMaybe = 0;
		}
	}
	this->updateSynchedFlags();
	return 1;
}

bool_t Ocelot::isFood(const ItemInstance* item) {
	return this->isFishItem(item);
}

Entity* Ocelot::findAttackTarget() {
	if (this->isBaby() || this->isSitting || this->inPanicTicksMaybe > 0 || !this->level || this->level->isClientMaybe) {
		return 0;
	}
	if (this->isTrusting) {
		Player* owner = this->resolveOwner();
		Entity* combatTarget = findOcelotOwnerCombatTarget(this, owner);
		if (combatTarget) {
			return combatTarget;
		}
	}
	std::vector<Entity*>* nearby = this->level->getEntities(this, {this->boundingBox.minX - 16.0f, this->boundingBox.minY - 8.0f, this->boundingBox.minZ - 16.0f, this->boundingBox.maxX + 16.0f, this->boundingBox.maxY + 8.0f, this->boundingBox.maxZ + 16.0f});
	Entity* bestFish = 0;
	float bestDistSq = 256.0f;
	if (nearby) {
		for (size_t i = 0; i < nearby->size(); ++i) {
			Entity* ent = nearby->at(i);
			if (!isOcelotFishEntity(ent)) {
				continue;
			}
			float dSq = ent->distanceToSqr(this);
			if (dSq < bestDistSq) {
				bestDistSq = dSq;
				bestFish = ent;
			}
		}
	}
	return bestFish;
}

void Ocelot::checkHurtTarget(Entity* target, float dist) {
	if (target && target->isAlive() && this->attackTime <= 0 && dist < 2.2f && target->boundingBox.maxY > this->boundingBox.minY - 0.6f && target->boundingBox.minY < this->boundingBox.maxY + 0.6f) {
		this->attackTime = 20;
		this->doHurtTarget(target);
	}
}

bool_t Ocelot::doHurtTarget(Entity* target) {
	if (!target) {
		return 0;
	}
	this->swing();
	int32_t dmg = this->isBaby() ? 2 : (this->isTrusting ? 5 : 4);
	return target->hurt(this, dmg);
}

static void consumeOcelotSelectedItem(Player* player, ItemInstance* sel) {
	if (!player || !sel || player->abilities.instabuild) return;
	--sel->count;
	if (sel->count <= 0) {
		player->inventory->clearSlot(player->inventory->selectedSlot);
	}
}

static void spawnOcelotHeartParticles(Ocelot* ocelot) {
	if (!ocelot || !ocelot->level) return;
	for (int32_t i = 0; i < 7; ++i) {
		float gx = ocelot->random.nextGaussian() * 0.02f;
		float gy = ocelot->random.nextGaussian() * 0.02f;
		float gz = ocelot->random.nextGaussian() * 0.02f;
		float px = ocelot->posX + (ocelot->random.nextFloat() * ocelot->entityWidth * 2.0f) - ocelot->entityWidth;
		float py = ocelot->posY + 0.5f + (ocelot->random.nextFloat() * ocelot->entityHeight);
		float pz = ocelot->posZ + (ocelot->random.nextFloat() * ocelot->entityWidth * 2.0f) - ocelot->entityWidth;
		ocelot->level->addParticle(PT_HEART, px, py, pz, gx, gy, gz, 0);
	}
}

static void spawnOcelotSmokeParticles(Ocelot* ocelot) {
	if (!ocelot || !ocelot->level) return;
	for (int32_t i = 0; i < 7; ++i) {
		float gx = ocelot->random.nextGaussian() * 0.02f;
		float gy = ocelot->random.nextGaussian() * 0.02f;
		float gz = ocelot->random.nextGaussian() * 0.02f;
		float px = ocelot->posX + (ocelot->random.nextFloat() * ocelot->entityWidth * 2.0f) - ocelot->entityWidth;
		float py = ocelot->posY + 0.5f + (ocelot->random.nextFloat() * ocelot->entityHeight);
		float pz = ocelot->posZ + (ocelot->random.nextFloat() * ocelot->entityWidth * 2.0f) - ocelot->entityWidth;
		ocelot->level->addParticle(PT_SMOKE, px, py, pz, gx, gy, gz, 0);
	}
}

bool_t Ocelot::interactWithPlayer(Player* player) {
	if (!player || !player->inventory) return Animal::interactWithPlayer(player);
	ItemInstance* held = player->inventory->getSelected();
	bool_t isOcelotFood = this->isFood(held);
	if (isOcelotFood) {
		bool_t usedFood = 0;
		if (!this->isTrusting && !this->isAngry) {
			usedFood = 1;
			consumeOcelotSelectedItem(player, held);
			bool_t tamedNow = 1;
			if (this->level && !this->level->isClientMaybe) {
				if (tamedNow) {
					this->isTrusting = 1;
					this->isSitting = 0;
					this->isAngry = 0;
					this->angerTicks = 0;
					this->ownerName = player->username;
					this->ownerEntityId = player->entityId;
					this->inPanicTicksMaybe = 0;
					this->attackTarget = 0;
					this->setPath(0);
					this->setLastHurtByMob(0);
					this->health = this->getMaxHealth();
					this->updateSynchedFlags();
				}
				this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
			}
			if (tamedNow) {
				spawnOcelotHeartParticles(this);
			} else {
				spawnOcelotSmokeParticles(this);
			}
			return 1;
		} else if (this->health > 0 && this->health < this->getMaxHealth()) {
			usedFood = 1;
			consumeOcelotSelectedItem(player, held);
			if (this->level && !this->level->isClientMaybe) {
				int32_t nextHp = this->health + 4;
				int32_t maxHp = this->getMaxHealth();
				this->health = (nextHp > maxHp) ? maxHp : nextHp;
				this->inPanicTicksMaybe = 0;
				this->isSitting = 0;
				if (this->isTrusting) {
					this->ownerName = player->username;
					this->ownerEntityId = player->entityId;
				}
				this->updateSynchedFlags();
				this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
			}
		} else if (!this->isBaby() && this->getAge() == 0 && !this->isInLove()) {
			usedFood = 1;
			consumeOcelotSelectedItem(player, held);
			if (this->level && !this->level->isClientMaybe) {
				this->inLove = 600;
				this->attackTarget = 0;
				this->isSitting = 0;
				this->setPath(0);
				this->updateSynchedFlags();
				this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() - this->random.nextFloat()) * 0.2f + 1.0f);
			}
		} else if (this->isBaby()) {
			usedFood = 1;
			consumeOcelotSelectedItem(player, held);
			if (this->level && !this->level->isClientMaybe) {
				int32_t age = this->getAge();
				if (age < 0) {
					int32_t boost = (-age) / 10;
					if (boost < 20) boost = 20;
					int32_t nextAge = age + boost;
					if (nextAge > 0) nextAge = 0;
					this->setAge(nextAge);
				}
				this->isSitting = 0;
				this->updateSynchedFlags();
				this->level->playSound(this, "random.eat", 0.7f, (this->random.nextFloat() * 0.2f) + 0.9f);
			}
		}
		if (usedFood) {
			spawnOcelotHeartParticles(this);
			return 1;
		}
	}
	if (this->isTrusting) {
		if (this->level && !this->level->isClientMaybe) {
			Player* owner = this->resolveOwner();
			if (!owner || owner == player || this->isOwnerPlayer(player)) {
				this->isSitting = this->isSitting ? 0 : 1;
				this->isJumping = 0;
				this->setPath(0);
				this->attackTarget = 0;
				this->ownerName = player->username;
				this->ownerEntityId = player->entityId;
				this->updateSynchedFlags();
			}
		}
		return 1;
	}
	return Animal::interactWithPlayer(player);
}

void Ocelot::readAdditionalSaveData(CompoundTag* tag) {
	Animal::readAdditionalSaveData(tag);
	this->isTrusting = (tag->getByte("Trusting") != 0) || (tag->getByte("Tame") != 0);
	this->isSitting = tag->getByte("Sitting") != 0;
	this->ownerName = tag->getString("Owner");
	this->updateSynchedFlags();
}

void Ocelot::addAdditonalSaveData(CompoundTag* tag) {
	Animal::addAdditonalSaveData(tag);
	tag->putBoolean("Trusting", this->isTrusting);
	tag->putBoolean("Sitting", this->isSitting);
	tag->putString("Owner", this->ownerName);
}

bool_t Ocelot::canSpawn() {
	if (!this->level) return 0;
	int32_t x = (int32_t)floorf(this->posX);
	int32_t y = (int32_t)floorf(this->posY);
	int32_t z = (int32_t)floorf(this->posZ);
	if (y <= 1 || y >= 127) return 0;
	int32_t tileBelow = this->level->getTile(x, y - 1, z);
	return (tileBelow == Tile::grass->blockID || tileBelow == Tile::leaves->blockID || tileBelow == Tile::dirt->blockID || tileBelow == Tile::sand->blockID || tileBelow == Tile::wood->blockID) && Mob::canSpawn();
}

bool_t Ocelot::removeWhenFarAway() {
	return !this->isTrusting;
}

Mob* Ocelot::getBreedOffspring(Animal* mate) {
	Ocelot* baby = new Ocelot(this->level);
	baby->isTrusting = this->isTrusting;
	baby->ownerName = this->ownerName;
	baby->ownerEntityId = this->ownerEntityId;
	baby->updateSynchedFlags();
	return baby;
}
