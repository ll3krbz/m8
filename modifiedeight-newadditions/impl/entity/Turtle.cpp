#include <entity/Turtle.hpp>
#include <entity/path/Path.hpp>
#include <level/Level.hpp>
#include <tile/Tile.hpp>
#include <math/Mth.hpp>
#include <math.h>

Turtle::Turtle(Level* level)
	: Animal(level) {
	this->entityRenderId = TURTLE;
	this->setSize(1.2f, 0.4f);
	this->stepHeight = 1.0f;
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

void Turtle::updateAi() {
	PathfinderMob::updateAi();
	if (this->isInWater()) {
		bool_t climbShore = this->isCollidedHorizontally;
		bool_t needUp = (this->path && !this->path->isDone() && this->path->currentPos(this).y > (float)Mth::floor(this->boundingBox.minY + 0.5f));
		this->isJumping = (climbShore || needUp || this->air < 120) ? 1 : 0;
	}
}

void Turtle::aiStep() {
	this->stepHeight = 1.0f;
	if (this->isInWater()) {
		this->isJumping = 0;
	}
	Animal::aiStep();
}

void Turtle::travel(float strafe, float forward) {
	if (this->isInWater()) {
		this->moveRelative(strafe, forward, 0.032f);
		this->move(this->motionX, this->motionY, this->motionZ);
		this->motionX *= 0.86f;
		this->motionZ *= 0.86f;
		if (this->isCollidedHorizontally) {
			this->motionY = 0.14f;
		} else if (this->air < 120) {
			this->motionY += (0.04f - this->motionY) * 0.2f;
		} else if (this->path && !this->path->isDone()) {
			float dy = this->path->currentPos(this).y - this->posY;
			float wantVy = Mth::clamp(dy * 0.05f, -0.025f, 0.025f);
			this->motionY += (wantVy - this->motionY) * 0.2f;
		} else {
			this->motionY *= 0.82f;
		}
		return;
	}
	Animal::travel(strafe, forward);
}

void Turtle::tick() {
	Animal::tick();
	if (this->isInWater()) {
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
