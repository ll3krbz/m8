#include <entity/Squid.hpp>
#include <level/Level.hpp>
#include <tile/material/Material.hpp>
#include <item/Item.hpp>
#include <math/Mth.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Squid::Squid(Level* level)
	: WaterAnimal(level) {
	this->setSize(0.95f, 0.95f);
	this->xBodyRot = 0.0f;
	this->xBodyRotO = 0.0f;
	this->zBodyRot = 0.0f;
	this->zBodyRotO = 0.0f;
	this->tentacleMovement = 0.0f;
	this->oldTentacleMovement = 0.0f;
	this->tentacleAngle = 0.0f;
	this->oldTentacleAngle = 0.0f;
	this->rotateSpeed = 0.0f;
	this->tx = 0.0f;
	this->ty = 0.0f;
	this->tz = 0.0f;
	this->tentacleSpeed = 1.0f / (this->random.nextFloat() + 1.0f) * 0.2f;
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
	WaterAnimal::aiStep();

	this->xBodyRotO = this->xBodyRot;
	this->zBodyRotO = this->zBodyRot;
	this->oldTentacleMovement = this->tentacleMovement;
	this->oldTentacleAngle = this->tentacleAngle;
	this->tentacleMovement += this->tentacleSpeed;

	if (this->tentacleMovement > (float)(M_PI * 2.0)) {
		this->tentacleMovement -= (float)(M_PI * 2.0);
		if (this->random.genrand_int32() % 10 == 0) {
			this->tentacleSpeed = 1.0f / (this->random.nextFloat() + 1.0f) * 0.2f;
		}
	}

	if (this->isInWater() || this->isUnderLiquid(Material::water)) {
		this->air = 300;
		if (this->tentacleMovement < (float)M_PI) {
			float f = this->tentacleMovement / (float)M_PI;
			this->tentacleAngle = Mth::sin(f * f * (float)M_PI) * (float)M_PI * 0.25f;
			if ((double)f > 0.75) {
				this->speed = 1.0f;
				this->rotateSpeed = 1.0f;
			} else {
				this->rotateSpeed *= 0.8f;
			}
		} else {
			this->tentacleAngle = 0.0f;
			this->speed *= 0.9f;
			this->rotateSpeed *= 0.99f;
		}

		if (this->tx * this->tx + this->ty * this->ty + this->tz * this->tz < 0.0001f || (this->random.genrand_int32() % 50 == 0)) {
			float angle = this->random.nextFloat() * (float)M_PI * 2.0f;
			this->tx = Mth::cos(angle) * 0.12f;
			this->ty = -0.03f + this->random.nextFloat() * 0.06f;
			this->tz = Mth::sin(angle) * 0.12f;
		}

		int bx = (int)floor(this->posX);
		int by = (int)floor(this->posY + 0.5f);
		int bz = (int)floor(this->posZ);
		bool waterAbove = this->level && this->level->getMaterial(bx, by + 1, bz)->isLiquid();

		this->motionX = this->tx * this->speed;
		this->motionY = this->ty * this->speed;
		this->motionZ = this->tz * this->speed;

		if (!waterAbove && this->motionY > 0.0f) {
			this->motionY = -0.02f;
		}

		float horizontalDist = sqrtf(this->motionX * this->motionX + this->motionZ * this->motionZ);
		this->yaw += (-(float)(atan2f(this->motionX, this->motionZ) * 180.0 / M_PI) - this->yaw) * 0.1f;
		this->zBodyRot += (float)((double)(float)M_PI * (double)this->rotateSpeed * 1.5 - (double)this->zBodyRot) * 0.1f;
		this->xBodyRot += ((-(float)(atan2f(horizontalDist, this->motionY) * 180.0 / M_PI) + 90.0f) - this->xBodyRot) * 0.1f;
		this->isJumping = 0;
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
							int t = this->level->getTile(bx + dx, by + dy, bz + dz);
							Material* m = this->level->getMaterial(bx + dx, by + dy, bz + dz);
							if ((m && m->isLiquid()) || t == 8 || t == 9) {
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
					this->motionX = (bestDx / len) * 0.18f;
					this->motionZ = (bestDz / len) * 0.18f;
					this->yaw = (float)(atan2f(bestDz, bestDx) * 180.0 / M_PI) - 90.0f;
				}
				if (this->onGround) {
					this->motionY = 0.28f;
				}
			} else if (this->onGround && this->random.nextFloat() < 0.2f) {
				this->motionX = (this->random.nextFloat() - 0.5f) * 0.25f;
				this->motionZ = (this->random.nextFloat() - 0.5f) * 0.25f;
				this->motionY = 0.25f;
			}
		}
		this->motionY = (this->motionY - 0.08f) * 0.98f;
		this->xBodyRot += (-90.0f - this->xBodyRot) * 0.02f;
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
		this->tx = -this->motionX * 2.0f;
		this->ty = 0.2f;
		this->tz = -this->motionZ * 2.0f;
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
