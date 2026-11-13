#include <entity/WaterAnimal.hpp>
#include <level/Level.hpp>
#include <tile/material/Material.hpp>

WaterAnimal::WaterAnimal(Level* level)
	: PathfinderMob(level) {
}
int32_t WaterAnimal::getExperienceReward(Player*) {
	return this->random.genrand_int32() % 3 + 1;
}
WaterAnimal::~WaterAnimal() {
}
int32_t WaterAnimal::getCreatureBaseType() {
	return 3;
}
void WaterAnimal::readAdditionalSaveData(CompoundTag* t) {
	PathfinderMob::readAdditionalSaveData(t);
}
void WaterAnimal::addAdditonalSaveData(CompoundTag* t) {
	PathfinderMob::addAdditonalSaveData(t);
}
bool_t WaterAnimal::isWaterMob() {
	return 1;
}
int32_t WaterAnimal::getAmbientSoundInterval() {
	return 120;
}
float WaterAnimal::getBaseSpeed() {
	return 0.2;
}
bool_t WaterAnimal::canSpawn() {
	return this->level->isUnobstructed(this->boundingBox);
}
bool_t WaterAnimal::removeWhenFarAway() {
	return 1;
}
bool_t WaterAnimal::isWaterAt(int32_t x, int32_t y, int32_t z) const {
	if (!this->level || y <= 0 || y >= 127) {
		return 0;
	}
	int32_t t = this->level->getTile(x, y, z);
	if (t == 8 || t == 9) {
		return 1;
	}
	Material* m = this->level->getMaterial(x, y, z);
	return m == Material::water;
}
void WaterAnimal::updateAi() {
}
void WaterAnimal::tick() {
	PathfinderMob::tick();
	this->field_124 = this->yaw;
	this->headYaw = this->yaw;
	while(this->field_124 - this->field_128 < -180.0f) this->field_128 -= 360.0f;
	while(this->field_124 - this->field_128 >= 180.0f) this->field_128 += 360.0f;
	while(this->headYaw - this->prevHeadYaw < -180.0f) this->prevHeadYaw -= 360.0f;
	while(this->headYaw - this->prevHeadYaw >= 180.0f) this->prevHeadYaw += 360.0f;
}
