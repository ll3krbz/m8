#include <entity/Frog.hpp>
#include <level/Level.hpp>
#include <level/biome/Biome.hpp>
#include <tile/Tile.hpp>
#include <tile/material/Material.hpp>
#include <math/Mth.hpp>
#include <math.h>

Frog::Frog(Level* level)
	: Animal(level) {
	this->entityRenderId = FROG;
	this->setSize(0.5f, 0.5f);
	this->stepHeight = 0.6f;
	this->swimAngle = this->random.nextFloat() * 360.0f;
	this->swimSpeed = 0.07f;
	this->swimTargetVy = 0.0f;
	this->swimTimer = 0;
}

Frog::~Frog() {
}

int32_t Frog::getEntityTypeId() const {
	return 40;
}

static std::string _frogTex = "mob/frog.png";
std::string* Frog::getTexture() {
	return &_frogTex;
}

int32_t Frog::getMaxHealth() {
	return 10;
}

float Frog::getBaseSpeed() {
	return 0.25f;
}

int32_t Frog::getAmbientSoundInterval() {
	return 120;
}

const char_t* Frog::getAmbientSound() {
	return "mob.frog.ambient";
}

std::string Frog::getHurtSound() {
	return "mob.frog.hurt";
}

std::string Frog::getDeathSound() {
	return "mob.frog.death";
}

static bool_t isFrogWaterAt(Level* level, int32_t x, int32_t y, int32_t z) {
	if (!level || y <= 1 || y >= 127) return 0;
	return level->getMaterial(x, y, z) == Material::water;
}

float Frog::getWalkTargetValue(int32_t x, int32_t y, int32_t z) {
	if (this->level && (isFrogWaterAt(this->level, x, y, z) || isFrogWaterAt(this->level, x, y - 1, z))) {
		return 6.0f;
	}
	return Animal::getWalkTargetValue(x, y, z);
}

void Frog::updateAi() {
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
		if (this->swimTimer <= 0 || this->isCollidedHorizontally || (this->random.genrand_int32() % 45) == 0) {
			if (this->isCollidedHorizontally) {
				this->swimAngle = Mth::wrapDegrees(this->swimAngle + 115.0f + this->random.nextFloat() * 130.0f);
			} else {
				this->swimAngle = Mth::wrapDegrees(this->swimAngle + (this->random.nextFloat() - 0.5f) * 140.0f);
			}
			this->swimSpeed = 0.055f + this->random.nextFloat() * 0.045f;
			this->swimTargetVy = (this->random.nextFloat() - 0.48f) * 0.032f;
			this->swimTimer = 25 + (this->random.genrand_int32() % 45);
		}

		float rad = this->swimAngle * (float)(3.14159265 / 180.0);
		int32_t curY = Mth::floor(this->posY + 0.25f);
		int32_t aheadX = Mth::floor(this->posX - Mth::sin(rad) * 1.6f);
		int32_t aheadZ = Mth::floor(this->posZ + Mth::cos(rad) * 1.6f);
		if (!isFrogWaterAt(this->level, aheadX, curY, aheadZ)) {
			float leftAngle = Mth::wrapDegrees(this->swimAngle - 65.0f);
			float rightAngle = Mth::wrapDegrees(this->swimAngle + 65.0f);
			float lRad = leftAngle * (float)(3.14159265 / 180.0);
			float rRad = rightAngle * (float)(3.14159265 / 180.0);
			bool_t hasLeft = isFrogWaterAt(this->level, Mth::floor(this->posX - Mth::sin(lRad) * 1.6f), curY, Mth::floor(this->posZ + Mth::cos(lRad) * 1.6f));
			bool_t hasRight = isFrogWaterAt(this->level, Mth::floor(this->posX - Mth::sin(rRad) * 1.6f), curY, Mth::floor(this->posZ + Mth::cos(rRad) * 1.6f));
			if (hasLeft && !hasRight) {
				this->swimAngle = leftAngle;
			} else if (hasRight && !hasLeft) {
				this->swimAngle = rightAngle;
			} else {
				this->swimAngle = Mth::wrapDegrees(this->swimAngle + ((this->random.genrand_int32() & 1) ? 145.0f : -145.0f));
			}
			this->swimTimer = 20;
		}

		bool_t waterAbove = isFrogWaterAt(this->level, Mth::floor(this->posX), Mth::floor(this->posY + 0.7f), Mth::floor(this->posZ));
		bool_t waterBelow = isFrogWaterAt(this->level, Mth::floor(this->posX), Mth::floor(this->posY - 0.5f), Mth::floor(this->posZ));
		if (!waterAbove && this->swimTargetVy > -0.004f) {
			this->swimTargetVy = -0.015f;
		} else if (!waterBelow && this->swimTargetVy < 0.005f) {
			this->swimTargetVy = 0.022f;
		}

		float diff = Mth::wrapDegrees(this->swimAngle - this->yaw);
		this->yaw = Mth::wrapDegrees(this->yaw + Mth::clamp(diff * 0.14f, -8.5f, 8.5f));
		this->pitch = 0.0f;
		return;
	}
	PathfinderMob::updateAi();
}

void Frog::aiStep() {
	Animal::aiStep();
}

void Frog::travel(float strafe, float forward) {
	if (this->isInWater()) {
		float yawRad = this->yaw * (float)(3.14159265 / 180.0);
		float wantVx = -Mth::sin(yawRad) * this->swimSpeed;
		float wantVz = Mth::cos(yawRad) * this->swimSpeed;
		this->motionX += (wantVx - this->motionX) * 0.22f;
		this->motionY += (this->swimTargetVy - this->motionY) * 0.2f;
		this->motionZ += (wantVz - this->motionZ) * 0.22f;
		this->move(this->motionX, this->motionY, this->motionZ);
		this->motionX *= 0.92f;
		this->motionY *= 0.9f;
		this->motionZ *= 0.92f;
		if (this->isCollidedHorizontally && this->level) {
			int32_t by = Mth::floor(this->posY + 0.55f);
			int32_t bx = Mth::floor(this->posX - Mth::sin(yawRad) * 0.8f);
			int32_t bz = Mth::floor(this->posZ + Mth::cos(yawRad) * 0.8f);
			if (!isFrogWaterAt(this->level, bx, by, bz) && !isFrogWaterAt(this->level, bx, by + 1, bz)) {
				this->motionY = 0.22f;
			} else {
				this->swimTimer = 0;
			}
		}
		return;
	}
	Animal::travel(strafe, forward);
}

void Frog::tick() {
	Animal::tick();
	if (this->isInWater()) {
		this->air = 300;
		this->fallDistance = 0.0f;
		this->field_124 += Mth::wrapDegrees(this->yaw - this->field_124) * 0.28f;
		this->headYaw = this->yaw;
		while (this->field_124 - this->field_128 < -180.0f) this->field_128 -= 360.0f;
		while (this->field_124 - this->field_128 >= 180.0f) this->field_128 += 360.0f;
		while (this->headYaw - this->prevHeadYaw < -180.0f) this->prevHeadYaw -= 360.0f;
		while (this->headYaw - this->prevHeadYaw >= 180.0f) this->prevHeadYaw += 360.0f;
	}
}

bool_t Frog::canSpawn() {
	if (!this->level) return 0;
	int32_t x = (int32_t)floorf(this->posX);
	int32_t y = (int32_t)floorf(this->posY);
	int32_t z = (int32_t)floorf(this->posZ);
	if (y <= 1 || y >= 127) return 0;
	int32_t tileBelow = this->level->getTile(x, y - 1, z);
	int32_t tileAt = this->level->getTile(x, y, z);
	if (tileAt != 0 && tileAt != Tile::waterLily->blockID && tileAt != Tile::tallgrass->blockID && tileAt != Tile::water->blockID && tileAt != Tile::calmWater->blockID && Tile::tiles[tileAt] && Tile::tiles[tileAt]->isSolidRender()) return 0;
	return (tileBelow == Tile::grass->blockID || tileBelow == Tile::dirt->blockID || tileBelow == Tile::sand->blockID || tileBelow == Tile::leaves->blockID || tileBelow == Tile::waterLily->blockID || tileBelow == Tile::water->blockID || tileBelow == Tile::calmWater->blockID || tileBelow == Tile::clay->blockID || tileBelow == Tile::gravel->blockID || tileBelow == Tile::wood->blockID || tileBelow == Tile::treeTrunk->blockID);
}

bool_t Frog::removeWhenFarAway() {
	return !this->isInLove() && this->getAge() == 0;
}

int32_t Frog::getMaxSpawnClusterSize() {
	return 6;
}

Mob* Frog::getBreedOffspring(Animal*) {
	return new Frog(this->level);
}
