#include <entity/Snowman.hpp>
#include <entity/Arrow.hpp>
#include <entity/Player.hpp>
#include <entity/Snowball.hpp>
#include <entity/Monster.hpp>
#include <inventory/Inventory.hpp>
#include <item/Item.hpp>
#include <item/ItemInstance.hpp>
#include <level/Level.hpp>
#include <level/biome/Biome.hpp>
#include <math/Mth.hpp>
#include <nbt/CompoundTag.hpp>
#include <tile/Tile.hpp>
#include <math.h>

Snowman::Snowman(Level* level)
	: PathfinderMob(level) {
	this->entityRenderId = SNOW_GOLEM;
	this->skin = "mob/snowman.png";
	this->setSize(0.7f, 1.9f);
	this->synchedEntityData.define<int8_t>(16, 16);
	this->revengeTargetId = 0;
	this->revengeTimer = 0;
}

Snowman::~Snowman() {
}

int32_t Snowman::getEntityTypeId() const {
	return 21;
}

float Snowman::getBaseSpeed() {
	return 0.2f;
}

int32_t Snowman::getMaxHealth() {
	return 4;
}

bool_t Snowman::hasPumpkin() const {
	return (const_cast<SynchedEntityData&>(this->synchedEntityData).getByte(16) & 16) != 0;
}

void Snowman::setPumpkin(bool_t has) {
	int8_t b = this->synchedEntityData.getByte(16);
	if (has) {
		this->synchedEntityData.set<int8_t>(16, (int8_t)(b | 16));
	} else {
		this->synchedEntityData.set<int8_t>(16, (int8_t)(b & ~16));
	}
}

bool_t Snowman::hasLitPumpkin() const {
	return (const_cast<SynchedEntityData&>(this->synchedEntityData).getByte(16) & 32) != 0;
}

void Snowman::setLitPumpkin(bool_t lit) {
	int8_t b = this->synchedEntityData.getByte(16);
	if (lit) {
		this->synchedEntityData.set<int8_t>(16, (int8_t)(b | 32));
	} else {
		this->synchedEntityData.set<int8_t>(16, (int8_t)(b & ~32));
	}
}

void Snowman::readAdditionalSaveData(CompoundTag* tag) {
	PathfinderMob::readAdditionalSaveData(tag);
	if (tag->contains("Pumpkin")) {
		this->setPumpkin(tag->getByte("Pumpkin") != 0);
	}
	if (tag->contains("LitPumpkin")) {
		this->setLitPumpkin(tag->getByte("LitPumpkin") != 0);
	}
}

void Snowman::addAdditonalSaveData(CompoundTag* tag) {
	PathfinderMob::addAdditonalSaveData(tag);
	tag->putBoolean("Pumpkin", this->hasPumpkin());
	tag->putBoolean("LitPumpkin", this->hasLitPumpkin());
}

bool_t Snowman::interactWithPlayer(Player* player) {
	if (!player || !player->inventory) return false;
	ItemInstance* sel = player->inventory->getSelected();
	if (sel && sel->itemClass == Item::shears && this->hasPumpkin()) {
		if (this->level && !this->level->isClientMaybe) {
			int32_t dropId = this->hasLitPumpkin() ? Tile::litPumpkin->blockID : Tile::pumpkin->blockID;
			this->setPumpkin(false);
			this->setLitPumpkin(false);
			Entity* v10 = this->spawnAtLocation(ItemInstance(dropId, 1, 0), 1.0f);
			if (v10) {
				v10->motionY += (float)(this->random.nextFloat() * 0.05f);
				v10->motionX += (float)((this->random.nextFloat() - this->random.nextFloat()) * 0.1f);
				v10->motionZ += (float)((this->random.nextFloat() - this->random.nextFloat()) * 0.1f);
			}
			this->level->playSound(this, "mob.sheep.shear", 1.0f, 1.0f);
		}
		if (!player->abilities.instabuild) {
			sel->hurtAndBreak(1, player);
		}
		return true;
	}
	return PathfinderMob::interactWithPlayer(player);
}

bool_t Snowman::hurt(Entity* attacker, int32_t damage) {
	bool_t res = PathfinderMob::hurt(attacker, damage);
	if (res && this->isAlive() && attacker && this->level && !this->level->isClientMaybe) {
		Entity* target = attacker;
		if (attacker->getEntityTypeId() == 80) {
			Arrow* arrow = (Arrow*)attacker;
			if (arrow->shooterEID) {
				Entity* shooter = this->level->getEntity(arrow->shooterEID);
				if (shooter) {
					target = shooter;
				}
			}
		}
		if (target && target != this && target != this->rider && target != this->ridingAt && target->isAlive() && target->isMob()) {
			bool wasRevenge = (this->revengeTimer > 0);
			this->attackTarget = target->entityId;
			this->revengeTargetId = target->entityId;
			this->revengeTimer = 200;
			if (!wasRevenge && this->attackTime > 10) {
				this->attackTime = 10;
			}
		}
	}
	return res;
}

void Snowman::aiStep() {
	PathfinderMob::aiStep();

	if (this->level && !this->level->isClientMaybe) {
		if (this->revengeTimer > 0) {
			--this->revengeTimer;
			Entity* target = this->level->getEntity(this->revengeTargetId);
			if (target && target->isAlive()) {
				float dist = target->distanceTo(this);
				if (dist > 20.0f) {
					if (this->attackTarget == this->revengeTargetId) {
						this->attackTarget = 0;
					}
					this->revengeTargetId = 0;
					this->revengeTimer = 0;
				} else if (!this->attackTarget && dist < 10.0f && this->canSee(target)) {
					this->checkHurtTarget(target, dist);
					this->headYaw = this->yaw;
				}
			} else {
				if (this->attackTarget == this->revengeTargetId) {
					this->attackTarget = 0;
				}
				this->revengeTargetId = 0;
				this->revengeTimer = 0;
			}
			if (this->revengeTimer == 0 && this->revengeTargetId) {
				if (this->attackTarget == this->revengeTargetId) {
					this->attackTarget = 0;
				}
				this->revengeTargetId = 0;
			}
		}
		int32_t ix = Mth::floor(this->posX);
		int32_t iy = Mth::floor(this->posY);
		int32_t iz = Mth::floor(this->posZ);

		bool inRain = (this->level->weatherType > 0 && this->level->canSeeSky(ix, iy, iz));
		if (this->isInWater() || inRain) {
			this->hurt(0, 1);
		}

		Biome* b = this->level->getBiome(ix, iz);
		if (b && b->temperature > 1.0f) {
			this->hurt(0, 1);
		}

		if (Tile::topSnow) {
			for (int32_t l = 0; l < 4; ++l) {
				int32_t x = Mth::floor(this->posX + (float)(l % 2 * 2 - 1) * 0.25f);
				int32_t y = Mth::floor(this->posY);
				int32_t z = Mth::floor(this->posZ + (float)(l / 2 % 2 * 2 - 1) * 0.25f);

				if (this->level->getTile(x, y, z) == 0) {
					Biome* blockBiome = this->level->getBiome(x, z);
					if (!blockBiome || blockBiome->temperature < 0.8f) {
						if (this->level->isTopSolidBlocking(x, y - 1, z) && Tile::topSnow->mayPlace(this->level, x, y, z)) {
							this->level->setTile(x, y, z, Tile::topSnow->blockID, 3);
						}
					}
				}
			}
		}
	}
}

Entity* Snowman::findAttackTarget() {
	if (!this->level) return nullptr;
	AABB searchBox{this->posX - 10.0f, this->posY - 4.0f, this->posZ - 10.0f,
	               this->posX + 10.0f, this->posY + 4.0f, this->posZ + 10.0f};
	std::vector<Entity*>* ents = this->level->getEntities(this, searchBox);
	if (!ents) return nullptr;

	float closestDist = 100.0f;
	Entity* closest = nullptr;
	for (Entity* e : *ents) {
		if (e && !e->isDead && e->isMob() && ((Mob*)e)->getCreatureBaseType() == 1 && this->canSee(e)) {
			float d = this->distanceToSqr(e);
			if (d < closestDist) {
				closestDist = d;
				closest = e;
			}
		}
	}
	return closest;
}

void Snowman::checkHurtTarget(Entity* target, float dist) {
	if (!target || !this->level) return;
	if (dist < 10.0f) {
		float dx = target->posX - this->posX;
		float dz = target->posZ - this->posZ;
		if (this->attackTime <= 0) {
			Snowball* snowball = new Snowball(this->level, this);
			float dy = (target->boundingBox.minY + target->entityHeight * 0.6f) - snowball->posY;
			float hdist = sqrtf(dx * dx + dz * dz);
			snowball->damage = 2;
			snowball->shoot(dx, dy + 0.03f * hdist * hdist, dz, 0.75f, 10.0f);
			this->level->playSound(this, "random.bow", 1.0f, 1.0f / (this->random.nextFloat() * 0.4f + 0.8f));
			this->level->addEntity(snowball);
			this->attackTime = 40;
		}
		this->_shouldHoldGround = 1;
		this->yaw = atan2f(dz, dx) * 57.29578f - 90.0f;
	}
}

void Snowman::dropDeathLoot() {
	if (!this->level || this->level->isClientMaybe) return;
	int32_t count = this->random.genrand_int32() % 6 + 1;
	for (int32_t i = 0; i < count; ++i) {
		this->spawnAtLocation(Item::snowBall->itemID, 1);
	}
}

const char_t* Snowman::getAmbientSound() {
	return "mob.snowgolem.ambient";
}

std::string Snowman::getHurtSound() {
	return "mob.snowgolem.hurt";
}

std::string Snowman::getDeathSound() {
	return "mob.snowgolem.death";
}
