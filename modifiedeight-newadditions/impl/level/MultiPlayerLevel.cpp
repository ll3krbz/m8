#include <level/MultiPlayerLevel.hpp>
#include <entity/Entity.hpp>
#include <entity/Player.hpp>
#include <level/biome/Biome.hpp>
#include <math/Mth.hpp>
#include <algorithm>
MultiPlayerLevel::MultiPlayerLevel(LevelStorage* a2, const std::string& a3, const LevelSettings& a4, bool a5,int a6,Dimension * a7) : Level(a2, a3, a4, a5, a6, a7){

}
bool_t MultiPlayerLevel::addEntity(Entity* a2) {
	bool_t v3 = Level::addEntity(a2);
	this->field_BC4.insert(a2);
	if(!v3) {
		this->field_BDC.insert(a2);
	}
	return v3;
}
void MultiPlayerLevel::putEntity(int32_t eid, Entity* ent) {
	//TODO check
	auto&& p = this->field_BAC.find(eid);
	if(p != this->field_BAC.end()) {
		this->removeEntity(p->second);
		this->field_BC4.erase(p->second);
	}
	this->field_BC4.insert(ent);
	ent->entityId = eid;
	if(!this->addEntity(ent)) {
		this->field_BDC.insert(ent);
	}
	this->field_BAC.insert({eid, ent});
}

MultiPlayerLevel::~MultiPlayerLevel() {
}
void MultiPlayerLevel::tick() {
	if(this->isClientMaybe) {
		if(this->adventureSettings.daylightCycle) {
			this->setTime(this->getTime() + 1);
		}
		this->updateSkyDarken();
		this->gameTickCounter++;
		if (this->weatherType != 0) {
			if (this->rainLevel < 1.0f) this->rainLevel += 0.003f;
			if (this->rainLevel > 1.0f) this->rainLevel = 1.0f;
		} else {
			if (this->rainLevel > 0.0f) {
				this->rainLevel -= 0.003f;
				if (this->rainLevel < 0.0f) this->rainLevel = 0.0f;
			}
		}
		if (this->rainLevel > 0.35f) {
			if (this->gameTickCounter % 10 == 0 && !this->playersMaybe.empty() && this->playersMaybe[0]) {
				Player* p = this->playersMaybe[0];
				Biome* b = this->getBiome(Mth::floor(p->posX), Mth::floor(p->posZ));
				bool isSnowy = (b == Biome::taiga || b == Biome::tundra || b == Biome::icePeaks || b == Biome::iceDesert);
				if (!isSnowy) {
					float dropAlpha = (this->rainLevel - 0.35f) / 0.65f;
					this->playSound(p->posX, p->posY, p->posZ, "ambient.weather.rain", dropAlpha * 0.7f, 1.0f);
				}
			}
			if (this->weatherType == 2 && this->gameTickCounter % 120 == 0 && (this->random.genrand_int32() % 2 == 0)) {
				if (!this->playersMaybe.empty() && this->playersMaybe[0]) {
					Player* p = this->playersMaybe[0];
					float ox = (this->random.nextFloat() * 80.0f - 40.0f);
					float oz = (this->random.nextFloat() * 80.0f - 40.0f);
					this->playSound(p->posX + ox, p->posY, p->posZ + oz, "ambient.weather.thunder", 1.0f, 0.9f + this->random.nextFloat() * 0.2f);
				}
			}
		}
		int32_t v3 = 10;
		do {
			//TODO check
			if(this->field_BDC.size() == 0) break;
			auto&& e = this->field_BDC.rbegin();
			this->field_BDC.erase(*e);
			if(this->entities.end() == std::find(this->entities.begin(), this->entities.end(), *e)) {
				this->addEntity(*e);
			}
			--v3;
		} while(v3);

		//TODO check
		for(auto&& it = this->field_BA4.begin(); it != this->field_BA4.end();) {
			int32_t v4 = it->field_C - 1;
			it->field_C = v4;
			if(v4) {
				++it;
			} else {
				this->setTileAndDataNoUpdate(it->x, it->y, it->z, it->id, it->meta);
				this->sendTileUpdated(it->x, it->y, it->z);
				auto&& svd = it;
				++it;
				this->field_BA4.erase(svd);
			}
		}
	} else {
		Level::tick();
	}
}
bool_t MultiPlayerLevel::tickPendingTicks(bool_t a2) {
	if(!this->isClientMaybe) {
		return Level::tickPendingTicks(a2);
	}
	return 0;
}
void MultiPlayerLevel::addToTickNextTick(int32_t x, int32_t y, int32_t z, int32_t id, int32_t delay) {
	if(!this->isClientMaybe) {
		Level::addToTickNextTick(x, y, z, id, delay);
	}
}
ChunkSource* MultiPlayerLevel::createChunkSource() {
	return 0;
}
void MultiPlayerLevel::tickTiles() {
}
