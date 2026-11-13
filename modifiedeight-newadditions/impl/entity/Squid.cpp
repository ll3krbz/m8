#include <entity/Squid.hpp>
#include <level/Level.hpp>
#include <tile/material/Material.hpp>
#include <item/Item.hpp>
#include <math/Mth.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void normalizeSquidDir(float& x, float& y, float& z) {
	float lenSq = x * x + y * y + z * z;
	if (lenSq > 0.000001f) {
		float inv = 1.0f / sqrtf(lenSq);
		x *= inv;
		y *= inv;
		z *= inv;
	} else {
		x = 1.0f;
		y = 0.0f;
		z = 0.0f;
	}
}

Squid::Squid(Level* level)
	: WaterAnimal(level) {
	this->setSize(0.95f, 0.95f);
	float initAngle = this->random.nextFloat() * (float)(M_PI * 2.0);
	this->tx = cosf(initAngle);
	this->ty = 0.0f;
	this->tz = sinf(initAngle);
	this->targetTx = this->tx;
	this->targetTy = 0.0f;
	this->targetTz = this->tz;
	this->swimPulse = 0.55f;
	this->wanderTicks = 40 + (this->random.genrand_int32() % 60);
	this->yaw = -(float)(atan2f(this->tx, this->tz) * 180.0 / M_PI);
	this->prevYaw = this->yaw;
	this->field_124 = this->yaw;
	this->field_128 = this->yaw;
	this->headYaw = this->yaw;
	this->prevHeadYaw = this->yaw;
	this->xBodyRot = -90.0f;
	this->xBodyRotO = -90.0f;
	this->zBodyRot = 0.0f;
	this->zBodyRotO = 0.0f;
	this->tentacleMovement = this->random.nextFloat() * (float)(M_PI * 2.0);
	this->oldTentacleMovement = this->tentacleMovement;
	this->tentacleAngle = 0.0f;
	this->oldTentacleAngle = 0.0f;
	this->rotateSpeed = 0.0f;
	this->tentacleSpeed = 1.0f / (this->random.nextFloat() + 1.0f) * 0.14f;
	this->entityRenderId = SQUID;
}

Squid::~Squid() {
}

int32_t Squid::getMaxHealth() {
	return 10;
}

int32_t Squid::getEntityTypeId() const {
	return 17;
}

static std::string _squidTex = "mob/squid.png";
std::string* Squid::getTexture() {
	return &_squidTex;
}

int32_t Squid::getAmbientSoundInterval() {
	return 120;
}

const char_t* Squid::getAmbientSound() {
	return "mob.squid.ambient";
}

std::string Squid::getHurtSound() {
	return "mob.squid.hurt";
}

std::string Squid::getDeathSound() {
	return "mob.squid.death";
}

void Squid::aiStep() {
	this->xBodyRotO = this->xBodyRot;
	this->zBodyRotO = this->zBodyRot;
	this->oldTentacleMovement = this->tentacleMovement;
	this->oldTentacleAngle = this->tentacleAngle;
	this->tentacleMovement += this->tentacleSpeed;

	if (this->tentacleMovement > (float)(M_PI * 2.0)) {
		this->tentacleMovement -= (float)(M_PI * 2.0);
		if (this->random.genrand_int32() % 10 == 0) {
			this->tentacleSpeed = 1.0f / (this->random.nextFloat() + 1.0f) * 0.14f;
		}
	}

	bool_t inWater = this->isInWater() || this->isUnderLiquid(Material::water);

	if (this->level && this->level->isClientMaybe) {
		if (inWater) {
			if (this->tentacleMovement < (float)M_PI) {
				float f = this->tentacleMovement / (float)M_PI;
				this->tentacleAngle = Mth::sin(f * f * (float)M_PI) * (float)M_PI * 0.25f;
				if (f > 0.65f) {
					this->rotateSpeed += (1.0f - this->rotateSpeed) * 0.25f;
				} else {
					this->rotateSpeed *= 0.92f;
				}
			} else {
				this->tentacleAngle = 0.0f;
				this->rotateSpeed *= 0.96f;
			}
			float vx = this->posX - this->prevX;
			float vy = this->posY - this->prevY;
			float vz = this->posZ - this->prevZ;
			float h = sqrtf(vx * vx + vz * vz);
			if (h > 0.002f || fabsf(vy) > 0.002f) {
				float targetXRot = -(float)(atan2f(h, vy) * 180.0 / M_PI);
				this->xBodyRot += Mth::clamp(Mth::wrapDegrees(targetXRot - this->xBodyRot) * 0.14f, -6.0f, 6.0f);
			}
			this->zBodyRot += this->rotateSpeed * 2.0f;
			while (this->zBodyRot >= 180.0f) {
				this->zBodyRot -= 360.0f;
				this->zBodyRotO -= 360.0f;
			}
			while (this->zBodyRot < -180.0f) {
				this->zBodyRot += 360.0f;
				this->zBodyRotO += 360.0f;
			}
		} else {
			this->tentacleAngle = fabsf(Mth::sin(this->tentacleMovement)) * (float)M_PI * 0.25f;
			this->xBodyRot += Mth::wrapDegrees(-90.0f - this->xBodyRot) * 0.04f;
		}
		this->isJumping = 0;
		WaterAnimal::aiStep();
		return;
	}

	if (inWater) {
		this->air = 300;
		if (this->tentacleMovement < (float)M_PI) {
			float f = this->tentacleMovement / (float)M_PI;
			this->tentacleAngle = Mth::sin(f * f * (float)M_PI) * (float)M_PI * 0.25f;
			if (f > 0.65f) {
				float pulseRamp = (f - 0.65f) / 0.35f;
				float targetPulse = 0.55f + 0.45f * Mth::sin(pulseRamp * (float)M_PI * 0.5f);
				this->swimPulse += (targetPulse - this->swimPulse) * 0.35f;
				this->rotateSpeed += (1.0f - this->rotateSpeed) * 0.25f;
			} else {
				this->swimPulse = this->swimPulse * 0.94f + 0.02f;
				this->rotateSpeed *= 0.92f;
			}
		} else {
			this->tentacleAngle = 0.0f;
			this->swimPulse *= 0.94f;
			if (this->swimPulse < 0.32f) {
				this->swimPulse = 0.32f;
			}
			this->rotateSpeed *= 0.96f;
		}

		if (--this->wanderTicks <= 0) {
			this->wanderTicks = 50 + (this->random.genrand_int32() % 60);
			float curAngle = atan2f(this->targetTz, this->targetTx);
			float deltaAngle = (this->random.nextFloat() * 2.0f - 1.0f) * 1.15f;
			float nextAngle = curAngle + deltaAngle;
			this->targetTx = cosf(nextAngle);
			this->targetTz = sinf(nextAngle);
			this->targetTy = -0.18f + this->random.nextFloat() * 0.36f;
			normalizeSquidDir(this->targetTx, this->targetTy, this->targetTz);
		}

		if (this->level) {
			int32_t probeX = Mth::floor(this->posX + this->tx * 1.5f);
			int32_t probeY = Mth::floor(this->posY + 0.05f);
			int32_t probeZ = Mth::floor(this->posZ + this->tz * 1.5f);
			if (!this->isWaterAt(probeX, probeY, probeZ) || this->isCollidedHorizontally) {
				float baseAng = atan2f(this->tz, this->tx);
				float sideSign = (this->entityId & 1) ? 1.0f : -1.0f;
				static const float _turnOffsets[7] = {0.7f, -0.7f, 1.4f, -1.4f, 2.2f, -2.2f, 3.14159f};
				bool_t steered = 0;
				for (int32_t i = 0; i < 7; ++i) {
					float tryAng = baseAng + _turnOffsets[i] * sideSign;
					int32_t txCheck = Mth::floor(this->posX + cosf(tryAng) * 1.8f);
					int32_t tzCheck = Mth::floor(this->posZ + sinf(tryAng) * 1.8f);
					if (this->isWaterAt(txCheck, probeY, tzCheck)) {
						this->targetTx = cosf(tryAng);
						this->targetTz = sinf(tryAng);
						steered = 1;
						break;
					}
				}
				if (!steered) {
					this->targetTx = -this->targetTx;
					this->targetTz = -this->targetTz;
				}
				this->wanderTicks = 35;
			}
		}

		int32_t bx = Mth::floor(this->posX);
		int32_t bz = Mth::floor(this->posZ);
		bool_t waterAboveClose = this->isWaterAt(bx, Mth::floor(this->posY + 1.15f), bz);
		bool_t waterAboveNear = this->isWaterAt(bx, Mth::floor(this->posY + 1.9f), bz);
		bool_t waterBelowClose = this->isWaterAt(bx, Mth::floor(this->posY - 0.55f), bz);

		if (!waterAboveClose && !waterBelowClose) {
			float centerY = (float)Mth::floor(this->posY) + 0.45f;
			this->targetTy = Mth::clamp((centerY - this->posY) * 0.25f, -0.15f, 0.15f);
		} else if (!waterAboveClose) {
			if (this->targetTy > -0.25f) this->targetTy = -0.25f;
		} else if (!waterAboveNear && this->targetTy > -0.05f) {
			this->targetTy = -0.08f;
		} else if (!waterBelowClose) {
			if (this->targetTy < 0.25f) this->targetTy = 0.25f;
		}

		normalizeSquidDir(this->targetTx, this->targetTy, this->targetTz);
		this->tx += (this->targetTx - this->tx) * 0.08f;
		this->ty += (this->targetTy - this->ty) * 0.08f;
		this->tz += (this->targetTz - this->tz) * 0.08f;
		normalizeSquidDir(this->tx, this->ty, this->tz);

		float moveSpeed = 0.115f * this->swimPulse;
		float desiredVx = this->tx * moveSpeed;
		float desiredVy = this->ty * moveSpeed;
		float desiredVz = this->tz * moveSpeed;

		if (!waterAboveClose && desiredVy > 0.0f) {
			desiredVy = 0.0f;
		}
		if (!waterBelowClose && desiredVy < 0.0f) {
			desiredVy = 0.0f;
		}

		this->motionX += (desiredVx - this->motionX) * 0.16f;
		this->motionY += (desiredVy - this->motionY) * 0.16f;
		this->motionZ += (desiredVz - this->motionZ) * 0.16f;

		float horizontalDist = sqrtf(this->motionX * this->motionX + this->motionZ * this->motionZ);
		if (horizontalDist > 0.002f) {
			float targetYaw = -(float)(atan2f(this->motionX, this->motionZ) * 180.0 / M_PI);
			float dYaw = Mth::wrapDegrees(targetYaw - this->yaw);
			this->yaw += Mth::clamp(dYaw * 0.14f, -8.0f, 8.0f);
		}
		float targetXRot = -(float)(atan2f(horizontalDist, this->motionY) * 180.0 / M_PI);
		float dXRot = Mth::wrapDegrees(targetXRot - this->xBodyRot);
		this->xBodyRot += Mth::clamp(dXRot * 0.12f, -6.0f, 6.0f);

		this->zBodyRot += this->rotateSpeed * 2.0f;
		while (this->zBodyRot >= 180.0f) {
			this->zBodyRot -= 360.0f;
			this->zBodyRotO -= 360.0f;
		}
		while (this->zBodyRot < -180.0f) {
			this->zBodyRot += 360.0f;
			this->zBodyRotO += 360.0f;
		}

		this->isJumping = 0;
		this->headYaw = this->yaw;
		WaterAnimal::aiStep();
	} else {
		this->tentacleAngle = fabsf(Mth::sin(this->tentacleMovement)) * (float)M_PI * 0.25f;
		if (this->level) {
			bool foundWater = false;
			float bestDx = 0.0f, bestDz = 0.0f;
			float bestDistSq = 99999.0f;
			int bx = (int)floorf(this->posX);
			int by = (int)floorf(this->posY);
			int bz = (int)floorf(this->posZ);
			for (int r = 1; r <= 16 && !foundWater; ++r) {
				for (int dx = -r; dx <= r; ++dx) {
					for (int dz = -r; dz <= r; ++dz) {
						if (abs(dx) != r && abs(dz) != r) continue;
						for (int dy = -4; dy <= 2; ++dy) {
							if (this->isWaterAt(bx + dx, by + dy, bz + dz)) {
								float dSq = (float)(dx * dx + dz * dz);
								if (dSq < bestDistSq) {
									bestDistSq = dSq;
									bestDx = (float)dx;
									bestDz = (float)dz;
									foundWater = true;
								}
							}
						}
					}
				}
			}
			if (foundWater) {
				float len = sqrtf(bestDx * bestDx + bestDz * bestDz);
				if (len > 0.001f) {
					this->motionX = (bestDx / len) * 0.16f;
					this->motionZ = (bestDz / len) * 0.16f;
					float targetYaw = (float)(atan2f(bestDz, bestDx) * 180.0 / M_PI) - 90.0f;
					this->yaw += Mth::wrapDegrees(targetYaw - this->yaw) * 0.35f;
				}
				if (this->onGround) {
					this->motionY = 0.26f;
				}
			} else if (this->onGround && this->random.nextFloat() < 0.15f) {
				this->motionX = (this->random.nextFloat() - 0.5f) * 0.22f;
				this->motionZ = (this->random.nextFloat() - 0.5f) * 0.22f;
				this->motionY = 0.24f;
			}
		}
		this->motionY = (this->motionY - 0.08f) * 0.98f;
		this->xBodyRot += Mth::wrapDegrees(-90.0f - this->xBodyRot) * 0.05f;
		this->isJumping = 0;
		this->headYaw = this->yaw;
		WaterAnimal::aiStep();
	}
}

void Squid::baseTick() {
	int prevAir = this->air;
	WaterAnimal::baseTick();
	if (this->isInWater() || this->isUnderLiquid(Material::water)) {
		this->air = 300;
	} else {
		this->air = prevAir - 1;
		if (this->air <= -20) {
			this->air = 0;
			this->field_DC = 0;
			this->hurt(0, 1);
		}
	}
}

void Squid::travel(float a2, float a3) {
	this->move(this->motionX, this->motionY, this->motionZ);
}

bool_t Squid::hurt(Entity* source, int32_t damage) {
	bool_t result = WaterAnimal::hurt(source, damage);
	if (this->health > 0) {
		if (source) {
			this->targetTx = this->posX - source->posX;
			this->targetTy = 0.2f;
			this->targetTz = this->posZ - source->posZ;
		} else {
			this->targetTx = -this->tx;
			this->targetTy = 0.2f;
			this->targetTz = -this->tz;
		}
		normalizeSquidDir(this->targetTx, this->targetTy, this->targetTz);
		this->swimPulse = 1.0f;
		this->wanderTicks = 40;
	}
	return result;
}

void Squid::die(Entity* source) {
	WaterAnimal::die(source);
}

void Squid::dropDeathLoot() {
	int32_t count = this->random.genrand_int32() % 3 + 1;
	for (int32_t i = 0; i < count; ++i) {
		this->spawnAtLocation(Item::dye_powder->itemID, 1, 0);
	}
}
