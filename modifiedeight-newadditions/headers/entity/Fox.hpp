#pragma once
#include <entity/Animal.hpp>

struct Fox: Animal {
	int32_t foxType;
	bool_t isFoxSleeping;
	bool_t isFoxSitting;
	bool_t isFoxCrouching;
	bool_t isTame;
	int32_t trustedPlayerId;
	int32_t fleeTicks;
	int32_t fleeTargetId;
	int32_t repathTicks;

	Fox(Level*);
	void updateSynchedFlags();
	bool_t isFishItem(const ItemInstance*) const;
	virtual ~Fox();
	virtual int32_t getEntityTypeId() const;
	virtual std::string* getTexture();
	virtual int32_t getMaxHealth();
	virtual float getBaseSpeed();
	virtual float getWalkingSpeedModifier();
	virtual int32_t getAmbientSoundInterval();
	virtual const char_t* getAmbientSound();
	virtual std::string getHurtSound();
	virtual std::string getDeathSound();
	virtual void updateAi();
	virtual void aiStep();
	virtual bool_t interactWithPlayer(Player*);
	virtual bool_t hurt(Entity*, int32_t);
	virtual bool_t isFood(const ItemInstance*);
	virtual void readAdditionalSaveData(CompoundTag*);
	virtual void addAdditonalSaveData(CompoundTag*);
	virtual bool_t canSpawn();
	virtual bool_t removeWhenFarAway();
	virtual Mob* getBreedOffspring(Animal*);
};
