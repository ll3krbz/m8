#pragma once
#include <entity/Animal.hpp>

struct Frog: Animal {
	float swimAngle;
	float swimSpeed;
	float swimTargetVy;
	int32_t swimTimer;

	Frog(Level*);
	virtual ~Frog();
	virtual int32_t getEntityTypeId() const;
	virtual std::string* getTexture();
	virtual int32_t getMaxHealth();
	virtual float getBaseSpeed();
	virtual const char_t* getAmbientSound();
	virtual std::string getHurtSound();
	virtual std::string getDeathSound();
	virtual int32_t getAmbientSoundInterval();
	virtual void updateAi();
	virtual void aiStep();
	virtual void travel(float, float);
	virtual void tick();
	virtual float getWalkTargetValue(int32_t, int32_t, int32_t);
	virtual bool_t canSpawn();
	virtual bool_t removeWhenFarAway();
	virtual int32_t getMaxSpawnClusterSize();
	virtual Mob* getBreedOffspring(Animal*);
};

