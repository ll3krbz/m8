#include <entity/AbstractFish.hpp>
#include <level/Level.hpp>
#include <tile/material/Material.hpp>
#include <math/Mth.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float blendFishAngle(float a, float b, float t) {
	return Mth::wrapDegrees(a + Mth::wrapDegrees(b - a) * t);
}

AbstractFish::AbstractFish(Level* level)
	: WaterAnimal(level) {
	this->setSize(0.5f, 0.4f);
	this->fishAngle = this->random.nextFloat() * 360.0f - 180.0f;
	this->targetAngle = this->fishAngle;
	this->yaw = this->fishAngle;
	this->prevYaw = this->yaw;
	this->field_124 = this->yaw;
	this->field_128 = this->yaw;
	this->headYaw = this->yaw;
	this->prevHeadYaw = this->yaw;
	this->swimSpeed = 0.055f;
	this->targetSwimSpeed = 0.06f;
	this->targetVy = 0.0f;
	this->swimTimer = 30 + (this->random.genrand_int32() % 50);
	this->leaderEntityId = 0;
}

AbstractFish::~AbstractFish() {
}

int32_t AbstractFish::getMaxHealth() {
	return 3;
}

int32_t AbstractFish::getAmbientSoundInterval() {
	return 120;
}

const char_t* AbstractFish::getAmbientSound() {
	return "mob.fish.swim";
}

std::string AbstractFish::getHurtSound() {
	return "mob.fish.hurt";
}

std::string AbstractFish::getDeathSound() {
	return "mob.fish.hurt";
}

int32_t AbstractFish::getDeathLoot() {
	return 0;
}

void AbstractFish::dropDeathLoot() {
	int32_t loot = this->getDeathLoot();
	if (loot > 0) {
		this->spawnAtLocation(loot, 1, 0);
	}
}

void AbstractFish::startFollowing(AbstractFish* target) {
	this->leaderEntityId = target ? target->entityId : 0;
}

void AbstractFish::aiStep() {
	try {
		bool_t inWater = this->isInWater() || this->isUnderLiquid(Material::water);
		if (inWater) {
			this->air = 300;
		}
		if (!this->level || this->level->isClientMaybe) {
			this->fishAngle += 0.12f;
			this->isJumping = 0;
			WaterAnimal::aiStep();
			return;
		}

		if (inWater) {
			if (this->leaderEntityId == 0 && (this->random.genrand_int32() % 40 == 0)) {
				AABB searchBox{this->posX - 9.0f, this->posY - 4.0f, this->posZ - 9.0f, this->posX + 9.0f, this->posY + 4.0f, this->posZ + 9.0f};
				std::vector<Entity*>* nearby = this->level->getEntities(this, searchBox);
				if (nearby) {
					for (Entity* e : *nearby) {
						if (e && e->isAlive() && e->getEntityTypeId() == this->getEntityTypeId() && e->entityId != this->entityId && e->isInWater()) {
							AbstractFish* otherFish = (AbstractFish*)e;
							if (otherFish->leaderEntityId == 0 || otherFish->leaderEntityId == otherFish->entityId) {
								this->leaderEntityId = otherFish->entityId;
								break;
							} else if (otherFish->leaderEntityId != this->entityId) {
								this->leaderEntityId = otherFish->leaderEntityId;
								break;
							}
						}
					}
				}
			}

			Entity* leaderEnt = (this->leaderEntityId != 0) ? this->level->getEntity(this->leaderEntityId) : nullptr;
			if (leaderEnt && leaderEnt->isAlive() && leaderEnt->isInWater() && leaderEnt->entityId != this->entityId && leaderEnt->distanceToSqr(this) < 256.0f) {
				AbstractFish* leaderFish = (AbstractFish*)leaderEnt;
				float dx = leaderEnt->posX - this->posX;
				float dy = leaderEnt->posY - this->posY;
				float dz = leaderEnt->posZ - this->posZ;
				float dsq = dx * dx + dz * dz;
				float toLeaderAngle = (float)(atan2f(dz, dx) * 180.0 / M_PI) - 90.0f;
				if (dsq > 14.0f) {
					this->targetAngle = blendFishAngle(this->targetAngle, toLeaderAngle, 0.25f);
					this->targetSwimSpeed = 0.09f;
				} else if (dsq > 2.25f) {
					float desired = blendFishAngle(leaderEnt->yaw, toLeaderAngle, 0.35f);
					this->targetAngle = blendFishAngle(this->targetAngle, desired, 0.30f);
					this->targetSwimSpeed = leaderFish->swimSpeed * 1.05f;
				} else {
					float fanOffset = (float)((this->entityId % 7) - 3) * 6.0f;
					float desired = leaderEnt->yaw + fanOffset;
					if (dsq < 0.7f && dsq > 0.001f) {
						desired = blendFishAngle(desired, toLeaderAngle + 180.0f, 0.20f);
					}
					this->targetAngle = blendFishAngle(this->targetAngle, desired, 0.25f);
					this->targetSwimSpeed = leaderFish->swimSpeed;
				}
				this->targetVy = Mth::clamp(dy * 0.04f, -0.025f, 0.025f);
			} else {
				this->leaderEntityId = 0;
				if (--this->swimTimer <= 0) {
					this->swimTimer = 45 + (this->random.genrand_int32() % 55);
					float delta = (this->random.nextFloat() * 2.0f - 1.0f) * 75.0f;
					this->targetAngle = Mth::wrapDegrees(this->targetAngle + delta);
					this->targetSwimSpeed = 0.05f + this->random.nextFloat() * 0.03f;
					this->targetVy = (this->random.nextFloat() * 2.0f - 1.0f) * 0.015f;
				}
			}

			float yawRad = this->yaw * (float)M_PI / 180.0f;
			int32_t fwdX = Mth::floor(this->posX - Mth::sin(yawRad) * 1.5f);
			int32_t fwdY = Mth::floor(this->posY + 0.05f);
			int32_t fwdZ = Mth::floor(this->posZ + Mth::cos(yawRad) * 1.5f);
			if (!this->isWaterAt(fwdX, fwdY, fwdZ) || this->isCollidedHorizontally) {
				float sideSign = (this->entityId & 1) ? 1.0f : -1.0f;
				static const float _fishTurns[9] = {35.0f, -35.0f, 70.0f, -70.0f, 110.0f, -110.0f, 150.0f, -150.0f, 180.0f};
				bool_t foundOpen = 0;
				for (int32_t i = 0; i < 9; ++i) {
					float tryAngle = Mth::wrapDegrees(this->yaw + _fishTurns[i] * sideSign);
					float tryRad = tryAngle * (float)M_PI / 180.0f;
					int32_t cx = Mth::floor(this->posX - Mth::sin(tryRad) * 1.8f);
					int32_t cz = Mth::floor(this->posZ + Mth::cos(tryRad) * 1.8f);
					if (this->isWaterAt(cx, fwdY, cz)) {
						this->targetAngle = tryAngle;
						foundOpen = 1;
						break;
					}
				}
				if (!foundOpen) {
					this->targetAngle = Mth::wrapDegrees(this->yaw + 180.0f);
				}
				this->swimTimer = 30;
			}

			int32_t bx = Mth::floor(this->posX);
			int32_t bz = Mth::floor(this->posZ);
			bool_t waterAbove = this->isWaterAt(bx, Mth::floor(this->posY + 0.85f), bz);
			bool_t waterBelow = this->isWaterAt(bx, Mth::floor(this->posY - 0.5f), bz);

			float desiredVy = this->targetVy;
			if (!waterAbove && !waterBelow) {
				float centerY = (float)Mth::floor(this->posY) + 0.35f;
				desiredVy = Mth::clamp((centerY - this->posY) * 0.08f, -0.015f, 0.015f);
			} else if (!waterAbove) {
				desiredVy = -0.02f;
			} else if (!waterBelow) {
				desiredVy = 0.02f;
			}

			float diff = Mth::wrapDegrees(this->targetAngle - this->yaw);
			this->yaw += Mth::clamp(diff * 0.14f, -8.5f, 8.5f);
			this->swimSpeed += (this->targetSwimSpeed - this->swimSpeed) * 0.12f;

			float rad = this->yaw * (float)M_PI / 180.0f;
			float dvx = -Mth::sin(rad) * this->swimSpeed;
			float dvz = Mth::cos(rad) * this->swimSpeed;

			this->motionX += (dvx - this->motionX) * 0.22f;
			this->motionZ += (dvz - this->motionZ) * 0.22f;
			this->motionY += (desiredVy - this->motionY) * 0.18f;
			this->fishAngle += 0.12f;
			this->isJumping = 0;
			this->headYaw = this->yaw;
		} else {
			if (this->onGround) {
				this->motionY = 0.22f;
				float bestDistSq = 9999.0f;
				float targetDx = 0.0f, targetDz = 0.0f;
				int32_t bx = Mth::floor(this->posX);
				int32_t by = Mth::floor(this->posY);
				int32_t bz = Mth::floor(this->posZ);
				for (int32_t ox = -3; ox <= 3; ++ox) {
					for (int32_t oz = -3; oz <= 3; ++oz) {
						for (int32_t oy = -1; oy <= 1; ++oy) {
							if (this->isWaterAt(bx + ox, by + oy, bz + oz)) {
								float distSq = (float)(ox * ox + oz * oz);
								if (distSq < bestDistSq) {
									bestDistSq = distSq;
									targetDx = (float)ox;
									targetDz = (float)oz;
								}
							}
						}
					}
				}
				float goalYaw = this->yaw;
				if (bestDistSq < 9000.0f && bestDistSq > 0.01f) {
					float len = sqrtf(targetDx * targetDx + targetDz * targetDz);
					this->motionX = (targetDx / len) * 0.18f;
					this->motionZ = (targetDz / len) * 0.18f;
					goalYaw = (float)(atan2f(targetDz, targetDx) * 180.0 / M_PI) - 90.0f;
				} else {
					this->motionX = (this->random.nextFloat() * 2.0f - 1.0f) * 0.15f;
					this->motionZ = (this->random.nextFloat() * 2.0f - 1.0f) * 0.15f;
					goalYaw = this->random.nextFloat() * 360.0f - 180.0f;
				}
				this->yaw += Mth::wrapDegrees(goalYaw - this->yaw) * 0.4f;
				this->level->playSound(this->posX, this->posY, this->posZ, "mob.fish.flop", 0.4f, 1.0f + (this->random.nextFloat() - this->random.nextFloat()) * 0.2f);
			}
			this->motionY = (this->motionY - 0.08f) * 0.98f;
			this->isJumping = 0;
			this->headYaw = this->yaw;

			if (--this->air <= -20) {
				this->air = 0;
				this->hurt(0, 1);
			}
		}
		WaterAnimal::aiStep();
	} catch (...) {
	}
}

void AbstractFish::travel(float a2, float a3) {
	this->move(this->motionX, this->motionY, this->motionZ);
}
