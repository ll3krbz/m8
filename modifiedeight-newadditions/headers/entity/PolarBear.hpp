#pragma once
#include <entity/Animal.hpp>

struct PolarBear: Animal {
	int32_t angerTicks;
	int32_t stareTicks;
	int32_t friendlyTicks;
	int32_t trustedPlayerId;

	PolarBear(Level*);
	bool_t isFishItem(const ItemInstance*) const;
	virtual ~PolarBear();
	virtual int32_t getEntityTypeId() const;
	virtual std::string* getTexture();
	virtual int32_t getMaxHealth();
	virtual float getBaseSpeed();
	virtual float getWalkingSpeedModifier();
	virtual int32_t getAmbientSoundInterval();
	virtual const char_t* getAmbientSound();
	virtual std::string getHurtSound();
	virtual std::string getDeathSound();
	virtual void aiStep();
	virtual bool_t canSpawn();
	virtual bool_t isFood(const ItemInstance*);
	virtual bool_t interactWithPlayer(Player*);
	virtual bool_t hurt(Entity*, int32_t);
	virtual Entity* findAttackTarget();
	virtual void checkHurtTarget(Entity*, float);
	virtual bool_t doHurtTarget(Entity*);
	virtual void readAdditionalSaveData(CompoundTag*);
	virtual void addAdditonalSaveData(CompoundTag*);
	virtual Mob* getBreedOffspring(Animal*);
};
