#pragma once
#include <entity/PathfinderMob.hpp>

struct Snowman: PathfinderMob {
	int32_t revengeTargetId;
	int32_t revengeTimer;

	Snowman(Level*);
	virtual ~Snowman();

	virtual int32_t getEntityTypeId() const;
	virtual float getBaseSpeed();
	virtual int32_t getMaxHealth();
	virtual void aiStep();
	virtual void dropDeathLoot();
	virtual bool_t hurt(Entity*, int32_t);
	virtual bool_t interactWithPlayer(Player*);
	virtual void checkHurtTarget(Entity*, float);
	virtual Entity* findAttackTarget();
	virtual void readAdditionalSaveData(CompoundTag*);
	virtual void addAdditonalSaveData(CompoundTag*);
	virtual const char_t* getAmbientSound();
	virtual std::string getHurtSound();
	virtual std::string getDeathSound();

	bool_t hasPumpkin() const;
	void setPumpkin(bool_t);
	bool_t hasLitPumpkin() const;
	void setLitPumpkin(bool_t);
};
