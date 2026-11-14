#include <entity/Turtle.hpp>
#include <entity/path/Path.hpp>
#include <level/Level.hpp>
#include <tile/Tile.hpp>
#include <tile/material/Material.hpp>
#include <math/Mth.hpp>
#include <math.h>

Turtle::Turtle(Level* level)
	: Animal(level) {
	this->entityRenderId = TURTLE;
	this->setSize(1.2f, 0.4f);
	this->stepHeight = 1.0f;
	this->swimAngle = this->random.nextFloat() * 360.0f;
	this->swimSpeed = 0.065f;
	this->swimTargetVy = 0.0f;
	this->swimTimer = 0;
}

Turtle::~Turtle() {
}

int32_t Turtle::getEntityTypeId() const {
	return 39;
}

static std::string _turtleTex = "mob/big_sea_turtle.png";
std::string* Turtle::getTexture() {
	return &_turtleTex;
}

int32_t Turtle::getMaxHealth() {
	return 30;
}

float Turtle::getBaseSpeed() {
	return 0.2f;
}

int32_t Turtle::getAmbientSoundInterval() {
	return 200;
}

const char_t* Turtle::getAmbientSound() {
	return "mob.turtle.idle";
}

std::string Turtle::getHurtSound() {
	return "mob.turtle.hurt";
}

std::string Turtle::getDeathSound() {
	return "mob.turtle.death";
}

static bool_t isTurtleWaterAt(Level* level, int32_t x, int32_t y, int32_t z) {
	if (!level || y <= 1 || y >= 127) return 0;
	return level->getMaterial(x, y, z) == Material::water;
}

float Turtle::getWalkTargetValue(int32_t x, int32_t y, int32_t z) {
	if (this->level && (isTurtleWaterAt(this->level, x, y, z) || isTurtleWaterAt(this->level, x, y - 1, z))) {
		return 8.0f;
	}
	return Animal::getWalkTargetValue(x, y, z);
}

void Turtle::updateAi() {
	if (this->isInWater() && this->level) {
		if (this->path) {
			this->setPath(0);
		}
		this->moveStrafe = 0.0f;
		this->moveForward = 0.0f;
		this->isJumping = 0;
		if (this->swimTimer > 0) {
			--this->swimTimer;
		}
		if (this->swimTimer <= 0 || this->isCollidedHorizontally || (this->random.genrand_int32() % 50) == 0) {
			if (this->isCollidedHorizontally) {
				this->swimAngle = Mth::wrapDegrees(this->swimAngle + 110.0f + this->random.nextFloat() * 140.0f);
			} else {
				this->swimAngle = Mth::wrapDegrees(this->swimAngle + (this->random.nextFloat() - 0.5f) * 120.0f);
			}
			this->swimSpeed = 0.052f + this->random.nextFloat() * 0.038f;
			this->swimTargetVy = (this->random.nextFloat() - 0.5f) * 0.028f;
			this->swimTimer = 30 + (this->random.genrand_int32() % 55);
		}

		float rad = this->swimAngle * (float)(3.14159265 / 180.0);
		int32_t curY = Mth::floor(this->posY + 0.25f);
		int32_t aheadX = Mth::floor(this->posX - Mth::sin(rad) * 2.1f);
		int32_t aheadZ = Mth::floor(this->posZ + Mth::cos(rad) * 2.1f);
		if (!isTurtleWaterAt(this->level, aheadX, curY, aheadZ)) {
			float leftAngle = Mth::wrapDegrees(this->swimAngle - 60.0f);
			float rightAngle = Mth::wrapDegrees(this->swimAngle + 60.0f);
			float lRad = leftAngle * (float)(3.14159265 / 180.0);
			float rRad = rightAngle * (float)(3.14159265 / 180.0);
			bool_t hasLeft = isTurtleWaterAt(this->level, Mth::floor(this->posX - Mth::sin(lRad) * 2.1f), curY, Mth::floor(this->posZ + Mth::cos(lRad) * 2.1f));
			bool_t hasRight = isTurtleWaterAt(this->level, Mth::floor(this->posX - Mth::sin(rRad) * 2.1f), curY, Mth::floor(this->posZ + Mth::cos(rRad) * 2.1f));
			if (hasLeft && !hasRight) {
				this->swimAngle = leftAngle;
			} else if (hasRight && !hasLeft) {
				this->swimAngle = rightAngle;
			} else {
				this->swimAngle = Mth::wrapDegrees(this->swimAngle + ((this->random.genrand_int32() & 1) ? 140.0f : -140.0f));
			}
			this->swimTimer = 25;
		}

		bool_t waterAbove = isTurtleWaterAt(this->level, Mth::floor(this->posX), Mth::floor(this->posY + 0.65f), Mth::floor(this->posZ));
		bool_t waterBelow = isTurtleWaterAt(this->level, Mth::floor(this->posX), Mth::floor(this->posY - 0.6f), Mth::floor(this->posZ));
		if (this->air < 120) {
			this->swimTargetVy = 0.042f;
		} else if (!waterAbove && this->swimTargetVy > -0.004f) {
			this->swimTargetVy = -0.015f;
		} else if (!waterBelow && this->swimTargetVy < 0.004f) {
			this->swimTargetVy = 0.02f;
		}

		float diff = Mth::wrapDegrees(this->swimAngle - this->yaw);
		this->yaw = Mth::wrapDegrees(this->yaw + Mth::clamp(diff * 0.1f, -6.5f, 6.5f));
		this->pitch = 0.0f;
		return;
	}
	PathfinderMob::updateAi();
}

void Turtle::aiStep() {
	this->stepHeight = 1.0f;
	Animal::aiStep();
}

void Turtle::travel(float strafe, float forward) {
	if (this->isInWater()) {
		float yawRad = this->yaw * (float)(3.14159265 / 180.0);
		float wantVx = -Mth::sin(yawRad) * this->swimSpeed;
		float wantVz = Mth::cos(yawRad) * this->swimSpeed;
		this->motionX += (wantVx - this->motionX) * 0.18f;
		this->motionY += (this->swimTargetVy - this->motionY) * 0.18f;
		this->motionZ += (wantVz - this->motionZ) * 0.18f;
		this->move(this->motionX, this->motionY, this->motionZ);
		this->motionX *= 0.93f;
		this->motionY *= 0.9f;
		this->motionZ *= 0.93f;
		if (this->isCollidedHorizontally && this->level) {
			int32_t by = Mth::floor(this->posY + 0.55f);
			int32_t bx = Mth::floor(this->posX - Mth::sin(yawRad) * 1.1f);
			int32_t bz = Mth::floor(this->posZ + Mth::cos(yawRad) * 1.1f);
			if (!isTurtleWaterAt(this->level, bx, by, bz) && !isTurtleWaterAt(this->level, bx, by + 1, bz) && this->air < 180) {
				this->motionY = 0.16f;
			} else {
				this->swimTimer = 0;
			}
		}
		return;
	}
	Animal::travel(strafe, forward);
}

void Turtle::tick() {
	Animal::tick();
	if (this->isInWater()) {
		this->fallDistance = 0.0f;
		this->field_124 += Mth::wrapDegrees(this->yaw - this->field_124) * 0.25f;
		this->headYaw = this->yaw;
		while (this->field_124 - this->field_128 < -180.0f) this->field_128 -= 360.0f;
		while (this->field_124 - this->field_128 >= 180.0f) this->field_128 += 360.0f;
		while (this->headYaw - this->prevHeadYaw < -180.0f) this->prevHeadYaw -= 360.0f;
		while (this->headYaw - this->prevHeadYaw >= 180.0f) this->prevHeadYaw += 360.0f;
	}
}

bool_t Turtle::canSpawn() {
	if (!this->level) return 0;
	int32_t x = (int32_t)floorf(this->posX);
	int32_t y = (int32_t)floorf(this->posY);
	int32_t z = (int32_t)floorf(this->posZ);
	if (y <= 1 || y >= 127) return 0;
	int32_t tileBelow = this->level->getTile(x, y - 1, z);
	return (tileBelow == Tile::sand->blockID || tileBelow == Tile::grass->blockID || this->isInWater()) && PathfinderMob::canSpawn();
}

bool_t Turtle::isWaterMob() {
	return 1;
}

Mob* Turtle::getBreedOffspring(Animal*) {
	return new Turtle(this->level);
}
