#pragma once
#include <entity/Animal.hpp>

struct Ocelot: Animal {
	bool_t isTrusting;
	bool_t isSitting;
	bool_t isAngry;
	bool_t isInterested;
	std::string ownerName;
	int32_t ownerEntityId;
	int32_t repathTicks;
	int32_t angerTicks;
	int32_t sitLookMode;
	int32_t sitLookTicks;
	float sitYawOffset;
	float sitPitchTarget;
	float curHeadYawOffset;
	float curHeadPitchTarget;

	Ocelot(Level*);
	void updateSynchedFlags();
	bool_t isFishItem(const ItemInstance*) const;
	bool_t isOwnerPlayer(const Player*) const;
	Player* resolveOwner();
	virtual ~Ocelot();
	virtual int32_t getEntityTypeId() const;
	virtual std::string* getTexture();
	virtual int32_t getMaxHealth();
	virtual float getBaseSpeed();
	virtual int32_t getAmbientSoundInterval();
	virtual const char_t* getAmbientSound();
	virtual std::string getHurtSound();
	virtual std::string getDeathSound();
	virtual void travel(float, float);
	virtual float getWalkTargetValue(int32_t, int32_t, int32_t);
	virtual void updateAi();
	virtual void aiStep();
	virtual bool_t interactWithPlayer(Player*);
	virtual bool_t hurt(Entity*, int32_t);
	virtual bool_t isFood(const ItemInstance*);
	virtual Entity* findAttackTarget();
	virtual void checkHurtTarget(Entity*, float);
	virtual bool_t doHurtTarget(Entity*);
	virtual void readAdditionalSaveData(CompoundTag*);
	virtual void addAdditonalSaveData(CompoundTag*);
	virtual bool_t canSpawn();
	virtual bool_t removeWhenFarAway();
	virtual Mob* getBreedOffspring(Animal*);
};
