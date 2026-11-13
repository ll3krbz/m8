#include <gui/screens/ChatScreen.hpp>
#include <Minecraft.hpp>
#include <cpputils.hpp>
#include <entity/LocalPlayer.hpp>
#include <gui/buttons/BlankButton.hpp>
#include <gui/buttons/Button.hpp>
#include <gui/buttons/ImageWithBackground.hpp>
#include <gui/buttons/Touch_TButton.hpp>
#include <network/RakNetInstance.hpp>
#include <network/packet/MessagePacket.hpp>
#include <network/packet/MovePlayerPacket.hpp>
#include <network/packet/AddPlayerPacket.hpp>
#include <network/ServerSideNetworkHandler.hpp>
#include <java/JavaBridge.hpp>
#include <rendering/Font.hpp>
#include <util/Util.hpp>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include <level/Level.hpp>
#include <level/biome/Biome.hpp>
#include <level/BiomeSource.hpp>
#include <tile/Tile.hpp>
#include <item/Item.hpp>
#include <item/ItemInstance.hpp>
#include <inventory/Inventory.hpp>
#include <map>

ChatScreen::ChatScreen()
	: Screen() {
	this->field_54 = "";
	this->keyboardVisible = 0;
	this->field_59 = 0;
	this->closeButton = 0;
	this->field_60 = 0;
	this->field_64 = 0;
	this->sendChatMessageButton = 0;
}
void ChatScreen::closeWindow() {
	this->minecraft->platform()->hideKeyboard();
	this->minecraft->setScreen(0);
}
void ChatScreen::drawChatMessages(int32_t a2) {
	int32_t v20 = a2;
	for(auto& msg: this->chatMessages) {
		Util::stringSplit(msg.field_10, this->width, this->font->charLength, [&v20](const std::string&, float) {
			v20 -= 9;
		});

		if(msg.field_8[0] == '/') {
			this->font->drawWordWrap(msg.field_10, 2.0, v20, this->width, Color4::GREY.toARGB(), 1, 0);
		} else {
			this->font->drawWordWrap(msg.field_10, 2.0, v20, this->width, Color4::WHITE.toARGB(), 1, 0);
		}
	}
}
bool_t ChatScreen::guiMessagesUpdated() {
	int32_t sz = this->chatMessages.size();
	if(sz != this->minecraft->gui.chatMessages.size()) {
		return 1;
	}
	for(int32_t i = 0; i != sz; ++i) {
		if(!(this->minecraft->gui.chatMessages[i].field_10 == this->chatMessages[i].field_10)) {
			return 1;
		}
	}
	return 0;
}
template<typename T>
static inline std::string toStr(const T& val) {
	std::ostringstream oss;
	oss << val;
	return oss.str();
}
static inline float parseF(const std::string& s) {
	return (float)atof(s.c_str());
}
static inline int parseI(const std::string& s) {
	return atoi(s.c_str());
}

static std::string normalizeItemQuery(const std::string& raw) {
	std::string s = raw;
	for (auto& c : s) c = tolower(c);
	if (s.rfind("minecraft:", 0) == 0) s = s.substr(10);
	if (s.rfind("tile.", 0) == 0) s = s.substr(5);
	if (s.rfind("item.", 0) == 0) s = s.substr(5);
	if (s.size() > 5 && s.rfind(".name") == s.size() - 5) s = s.substr(0, s.size() - 5);
	std::string clean;
	for (char c : s) {
		if (isalnum(c)) clean += c;
	}
	return clean;
}

static void getItemCandidates(int id, std::string& outDesc, std::string& outLocalized, std::string& outDisplay) {
	outDesc.clear();
	outLocalized.clear();
	outDisplay.clear();
	if (id >= 0 && id < 256 && Tile::tiles[id]) {
		outDesc = Tile::tiles[id]->getDescriptionId();
	}
	if (id >= 0 && id < 512 && Item::items[id]) {
		if (outDesc.empty()) outDesc = Item::items[id]->getDescriptionId();
		ItemInstance inst(id, 1, 0);
		outLocalized = Item::items[id]->getName(&inst);
		outDisplay = outLocalized;
	}
	if (outDisplay.empty()) outDisplay = outDesc;
}

static bool resolveItem(const std::string& rawName, int& outId, std::string& outName) {
	if (rawName.empty()) return false;
	std::string name = rawName;
	for (auto& c : name) c = tolower(c);
	if (name.rfind("minecraft:", 0) == 0) name = name.substr(10);

	bool isNum = true;
	for (char c : name) {
		if (!isdigit(c)) { isNum = false; break; }
	}
	if (isNum) {
		int id = atoi(name.c_str());
		if (id > 0) {
			std::string desc, loc, disp;
			getItemCandidates(id, desc, loc, disp);
			outId = id;
			outName = disp.empty() ? ("item_" + toStr(id)) : disp;
			return true;
		}
	}

	std::string cleanName = normalizeItemQuery(rawName);
	if (cleanName.empty()) return false;

	int bestPartialId = -1;
	size_t bestPartialLen = (size_t)-1;
	std::string bestPartialName;

	for (int pass = 0; pass < 2; ++pass) {
		int startId = (pass == 0) ? 256 : 1;
		int endId = (pass == 0) ? 512 : 256;
		for (int i = startId; i < endId; ++i) {
			if (i < 256 && !Tile::tiles[i] && !Item::items[i]) continue;
			if (i >= 256 && !Item::items[i]) continue;

			std::string desc, loc, disp;
			getItemCandidates(i, desc, loc, disp);
			std::string cDesc = normalizeItemQuery(desc);
			std::string cLoc = normalizeItemQuery(loc);

			if ((!cDesc.empty() && cDesc == cleanName) || (!cLoc.empty() && cLoc == cleanName)) {
				outId = i;
				outName = disp.empty() ? ("item_" + toStr(i)) : disp;
				return true;
			}

			if (!cDesc.empty() && cDesc.find(cleanName) != std::string::npos) {
				if (cDesc.size() < bestPartialLen) {
					bestPartialLen = cDesc.size();
					bestPartialId = i;
					bestPartialName = disp.empty() ? ("item_" + toStr(i)) : disp;
				}
			}
			if (!cLoc.empty() && cLoc.find(cleanName) != std::string::npos) {
				if (cLoc.size() < bestPartialLen) {
					bestPartialLen = cLoc.size();
					bestPartialId = i;
					bestPartialName = disp.empty() ? ("item_" + toStr(i)) : disp;
				}
			}
		}
	}

	if (bestPartialId > 0) {
		outId = bestPartialId;
		outName = bestPartialName;
		return true;
	}

	return false;
}

static bool executeCommand(Minecraft* mc, const std::string& line) {
	if (!mc || !mc->level || mc->isOnlineClient() || (mc->level && mc->level->isClientMaybe) || JavaBridge::isActive()) return false;
	if (line.empty() || line[0] != '/') return false;

	std::stringstream ss(line.substr(1));
	std::string cmd;
	ss >> cmd;
	std::vector<std::string> args;
	std::string arg;
	while (ss >> arg) {
		args.emplace_back(arg);
	}

	for (auto& c : cmd) c = tolower(c);

	if (cmd == "xyz" || cmd == "pos" || cmd == "coords" || cmd == "where") {
		if (mc->player) {
			float px = mc->player->posX;
			float py = mc->player->posY;
			float pz = mc->player->posZ;
			char buf[128];
			snprintf(buf, sizeof(buf), "XYZ: %.1f / %.1f / %.1f (Block: %d, %d, %d)", px, py, pz, (int)floorf(px), (int)floorf(py), (int)floorf(pz));
			mc->gui.addMessage("", buf, 200);
		} else {
			mc->gui.addMessage("", "Player not available", 200);
		}
		return true;
	}

	if (cmd == "tp") {
		auto findPlayer = [&](const std::string& name) -> Player* {
			std::string lower = name;
			for (auto& c : lower) c = tolower(c);
			if (mc->player) {
				std::string pName = mc->player->username;
				for (auto& c : pName) c = tolower(c);
				if (pName == lower) return mc->player;
			}
			if (mc->level) {
				for (auto* p : mc->level->playersMaybe) {
					if (p) {
						std::string pName = p->username;
						for (auto& c : pName) c = tolower(c);
						if (pName == lower) return p;
					}
				}
			}
			return nullptr;
		};

		auto teleportPlayer = [&](Player* p, float x, float y, float z) {
			if (!p) return;
			p->moveTo(x, y - p->ridingHeight, z, p->yaw, p->pitch);
			p->motionX = 0.0f;
			p->motionY = 0.0f;
			p->motionZ = 0.0f;
			if (mc->rakNetInstance) {
				MovePlayerPacket pk(p->entityId, x, y - p->ridingHeight, z, p->pitch, p->yaw, p->headYaw);
				mc->rakNetInstance->send(pk);
			}
			if (mc->rakNetInstance && mc->rakNetInstance->isServer() && mc->level) {
				RakNet::RakPeer* peer = mc->rakNetInstance->getPeer();
				if (peer) {
					for (auto* other : mc->level->playersMaybe) {
						if (other && other != p) {
							if (other != mc->player && other->rakNetGUID.g != 0) {
								RakNet::BitStream stream;
								AddPlayerPacket(p).write(&stream);
								peer->Send(&stream, PacketPriority::HIGH_PRIORITY, PacketReliability::RELIABLE_ORDERED, 0, RakNet::AddressOrGUID(other->rakNetGUID), 0, 0);
							}
							if (p != mc->player && p->rakNetGUID.g != 0) {
								RakNet::BitStream stream;
								AddPlayerPacket(other).write(&stream);
								peer->Send(&stream, PacketPriority::HIGH_PRIORITY, PacketReliability::RELIABLE_ORDERED, 0, RakNet::AddressOrGUID(p->rakNetGUID), 0, 0);
							}
						}
					}
				}
			}
		};

		if (args.size() == 1) {
			Player* target = findPlayer(args[0]);
			if (!target) {
				mc->gui.addMessage("", "Player not found: " + args[0], 200);
				return true;
			}
			if (mc->player) {
				teleportPlayer(mc->player, target->posX, target->posY, target->posZ);
				mc->gui.addMessage("", "Teleported to " + target->username, 200);
			}
			return true;
		}
		if (args.size() == 2) {
			Player* p1 = findPlayer(args[0]);
			Player* p2 = findPlayer(args[1]);
			if (!p1) {
				mc->gui.addMessage("", "Player not found: " + args[0], 200);
				return true;
			}
			if (!p2) {
				mc->gui.addMessage("", "Player not found: " + args[1], 200);
				return true;
			}
			teleportPlayer(p1, p2->posX, p2->posY, p2->posZ);
			mc->gui.addMessage("", "Teleported " + p1->username + " to " + p2->username, 200);
			return true;
		}
		if (args.size() == 3) {
			try {
				float px = (args[0] == "~") ? mc->player->posX : parseF(args[0]);
				float py = (args[1] == "~") ? mc->player->posY : parseF(args[1]);
				float pz = (args[2] == "~") ? mc->player->posZ : parseF(args[2]);
				teleportPlayer(mc->player, px, py, pz);
				mc->gui.addMessage("", "Teleported to " + toStr((int)px) + ", " + toStr((int)py) + ", " + toStr((int)pz), 200);
			} catch (...) {
				mc->gui.addMessage("", "Invalid coordinates", 200);
			}
			return true;
		}
		if (args.size() == 4) {
			Player* target = findPlayer(args[0]);
			if (!target) {
				mc->gui.addMessage("", "Player not found: " + args[0], 200);
				return true;
			}
			try {
				float px = (args[1] == "~") ? target->posX : parseF(args[1]);
				float py = (args[2] == "~") ? target->posY : parseF(args[2]);
				float pz = (args[3] == "~") ? target->posZ : parseF(args[3]);
				teleportPlayer(target, px, py, pz);
				mc->gui.addMessage("", "Teleported " + target->username + " to " + toStr((int)px) + ", " + toStr((int)py) + ", " + toStr((int)pz), 200);
			} catch (...) {
				mc->gui.addMessage("", "Invalid coordinates", 200);
			}
			return true;
		}
		mc->gui.addMessage("", "Usage: /tp <player> or /tp [player] <x> <y> <z> or /tp <player1> <player2>", 200);
		return true;
	}

	if (cmd == "locate") {
		if (args.empty() || args[0] == "list" || args[0] == "help") {
			mc->gui.addMessage("", "Structures: village, temple, swamp_hut, igloo", 200);
			mc->gui.addMessage("", "Biomes: plains, desert, jungle, taiga, forest, birch_forest, swamp, mountains, tundra, savanna, shrubland, mushroom", 200);
			mc->gui.addMessage("", "Usage: /locate <structure|biome> [radius]", 200);
			return true;
		}
		std::string targetName = args[0];
		for (auto& c : targetName) c = tolower(c);

		int maxRadius = 3000;
		if (args.size() >= 2) {
			try {
				maxRadius = parseI(args[1]);
				if (maxRadius < 100) maxRadius = 100;
				if (maxRadius > 50000) maxRadius = 50000;
			} catch (...) {
			}
		}

		if (!mc->player || !mc->level || !mc->level->getBiomeSource()) {
			mc->gui.addMessage("", "Level or player not loaded", 200);
			return true;
		}

		int px = (int)mc->player->posX;
		int pz = (int)mc->player->posZ;
		BiomeSource* bs = mc->level->getBiomeSource();

		if (targetName == "village" || targetName == "villages") {
			int curChunkX = (int)floorf((float)px / 16.0f);
			int curChunkZ = (int)floorf((float)pz / 16.0f);
			int curGX = (int)floorf((float)curChunkX / 24.0f);
			int curGZ = (int)floorf((float)curChunkZ / 24.0f);
			int maxG = (maxRadius / 384) + 1;
			int bestDistSq = 999999999;
			int foundX = 0, foundZ = 0;
			bool found = false;

			for (int gr = 0; gr <= maxG; ++gr) {
				for (int dgx = -gr; dgx <= gr; ++dgx) {
					for (int dgz = -gr; dgz <= gr; ++dgz) {
						if (abs(dgx) == gr || abs(dgz) == gr) {
							int gx = curGX + dgx;
							int gz = curGZ + dgz;
							uint64_t vSeed = ((uint64_t)gx * 341873128712ULL + (uint64_t)gz * 132897987541ULL) ^ (uint64_t)mc->level->getSeed();
							Random vRand(vSeed);
							int targetChunkX = gx * 24 + (vRand.genrand_int32() % 16);
							int targetChunkZ = gz * 24 + (vRand.genrand_int32() % 16);
							int vx = targetChunkX * 16 + 8;
							int vz = targetChunkZ * 16 + 8;
							Biome* b = bs->getBiome(targetChunkX * 16 + 8, targetChunkZ * 16 + 8);
							if (b == Biome::plains || b == Biome::desert || b == Biome::taiga || b == Biome::tundra || b == Biome::icePeaks) {
								int dx = vx - px;
								int dz = vz - pz;
								int distSq = dx * dx + dz * dz;
								if (distSq <= maxRadius * maxRadius && distSq < bestDistSq) {
									bestDistSq = distSq;
									foundX = vx;
									foundZ = vz;
									found = true;
								}
							}
						}
					}
				}
				if (found) break;
			}

			if (found) {
				int dist = (int)sqrtf((float)bestDistSq);
				int foundY = mc->level->getHeightmap(foundX, foundZ);
				if (foundY <= 0) foundY = 70;
				else foundY += 1;
				mc->gui.addMessage("", "Located Village at X: " + toStr(foundX) + ", Y: " + toStr(foundY) + ", Z: " + toStr(foundZ) + " (" + toStr(dist) + " blocks away)", 200);
			} else {
				mc->gui.addMessage("", "Village not found within " + toStr(maxRadius) + " blocks", 200);
			}
			return true;
		}

		if (targetName == "temple" || targetName == "desert_temple" || targetName == "deserttemple") {
			int curChunkX = (int)floorf((float)px / 16.0f);
			int curChunkZ = (int)floorf((float)pz / 16.0f);
			int curGX = (int)floorf((float)curChunkX / 20.0f);
			int curGZ = (int)floorf((float)curChunkZ / 20.0f);
			int maxG = (maxRadius / 320) + 1;
			int bestDistSq = 999999999;
			int foundX = 0, foundZ = 0;
			bool found = false;

			for (int gr = 0; gr <= maxG; ++gr) {
				for (int dgx = -gr; dgx <= gr; ++dgx) {
					for (int dgz = -gr; dgz <= gr; ++dgz) {
						if (abs(dgx) == gr || abs(dgz) == gr) {
							int gx = curGX + dgx;
							int gz = curGZ + dgz;
							uint64_t tSeed = ((uint64_t)gx * 241873128712ULL + (uint64_t)gz * 332897987541ULL) ^ (uint64_t)mc->level->getSeed();
							Random tRand(tSeed);
							int targetChunkX = gx * 20 + (tRand.genrand_int32() % 12);
							int targetChunkZ = gz * 20 + (tRand.genrand_int32() % 12);
							int tx = targetChunkX * 16 + 8;
							int tz = targetChunkZ * 16 + 8;
							Biome* b = bs->getBiome(targetChunkX * 16 + 16, targetChunkZ * 16 + 16);
							if (b == Biome::desert) {
								int dx = tx - px;
								int dz = tz - pz;
								int distSq = dx * dx + dz * dz;
								if (distSq <= maxRadius * maxRadius && distSq < bestDistSq) {
									bestDistSq = distSq;
									foundX = tx;
									foundZ = tz;
									found = true;
								}
							}
						}
					}
				}
				if (found) break;
			}

			if (found) {
				int dist = (int)sqrtf((float)bestDistSq);
				int foundY = mc->level->getHeightmap(foundX + 10, foundZ + 10);
				if (foundY <= 0) foundY = 70;
				else foundY += 1;
				mc->gui.addMessage("", "Located Desert Temple at X: " + toStr(foundX) + ", Y: " + toStr(foundY) + ", Z: " + toStr(foundZ) + " (" + toStr(dist) + " blocks away)", 200);
			} else {
				mc->gui.addMessage("", "Desert Temple not found within " + toStr(maxRadius) + " blocks", 200);
			}
			return true;
		}

		if (targetName == "swamp_hut" || targetName == "swamphut" || targetName == "witch_hut" || targetName == "witchhut") {
			int curChunkX = (int)floorf((float)px / 16.0f);
			int curChunkZ = (int)floorf((float)pz / 16.0f);
			int curGX = (int)floorf((float)curChunkX / 32.0f);
			int curGZ = (int)floorf((float)curChunkZ / 32.0f);
			int maxG = (maxRadius / 512) + 1;
			int bestDistSq = 999999999;
			int foundX = 0, foundZ = 0;
			bool found = false;

			for (int gr = 0; gr <= maxG; ++gr) {
				for (int dgx = -gr; dgx <= gr; ++dgx) {
					for (int dgz = -gr; dgz <= gr; ++dgz) {
						if (abs(dgx) == gr || abs(dgz) == gr) {
							int gx = curGX + dgx;
							int gz = curGZ + dgz;
							uint64_t hSeed = ((uint64_t)gx * 341873128712ULL + (uint64_t)gz * 132897987541ULL) ^ (uint64_t)mc->level->getSeed() ^ 14357620ULL;
							Random hRand(hSeed);
							int targetChunkX = gx * 32 + (hRand.genrand_int32() % 24);
							int targetChunkZ = gz * 32 + (hRand.genrand_int32() % 24);
							int hx = targetChunkX * 16 + 4;
							int hz = targetChunkZ * 16 + 4;
							Biome* b = bs->getBiome(hx + 8, hz + 8);
							if (b == Biome::swampland) {
								int dx = hx - px;
								int dz = hz - pz;
								int distSq = dx * dx + dz * dz;
								if (distSq <= maxRadius * maxRadius && distSq < bestDistSq) {
									bestDistSq = distSq;
									foundX = hx;
									foundZ = hz;
									found = true;
								}
							}
						}
					}
				}
				if (found) break;
			}

			if (found) {
				int dist = (int)sqrtf((float)bestDistSq);
				int foundY = mc->level->getHeightmap(foundX + 3, foundZ + 4);
				if (foundY <= 0) foundY = 64;
				else foundY += 1;
				mc->gui.addMessage("", "Located Swamp Hut at X: " + toStr(foundX) + ", Y: " + toStr(foundY) + ", Z: " + toStr(foundZ) + " (" + toStr(dist) + " blocks away)", 200);
			} else {
				mc->gui.addMessage("", "Swamp Hut not found within " + toStr(maxRadius) + " blocks", 200);
			}
			return true;
		}

		if (targetName == "igloo") {
			int curChunkX = (int)floorf((float)px / 16.0f);
			int curChunkZ = (int)floorf((float)pz / 16.0f);
			int curGX = (int)floorf((float)curChunkX / 32.0f);
			int curGZ = (int)floorf((float)curChunkZ / 32.0f);
			int maxG = (maxRadius / 512) + 1;
			int bestDistSq = 999999999;
			int foundX = 0, foundZ = 0;
			bool found = false;

			for (int gr = 0; gr <= maxG; ++gr) {
				for (int dgx = -gr; dgx <= gr; ++dgx) {
					for (int dgz = -gr; dgz <= gr; ++dgz) {
						if (abs(dgx) == gr || abs(dgz) == gr) {
							int gx = curGX + dgx;
							int gz = curGZ + dgz;
							uint64_t iSeed = ((uint64_t)gx * 341873128712ULL + (uint64_t)gz * 132897987541ULL) ^ (uint64_t)mc->level->getSeed() ^ 14357618ULL;
							Random iRand(iSeed);
							int targetChunkX = gx * 32 + (iRand.genrand_int32() % 24);
							int targetChunkZ = gz * 32 + (iRand.genrand_int32() % 24);
							int ix = targetChunkX * 16 + 4;
							int iz = targetChunkZ * 16 + 4;
							Biome* b = bs->getBiome(ix + 8, iz + 8);
							if (b == Biome::tundra || b == Biome::icePeaks || b == Biome::taiga) {
								int dx = ix - px;
								int dz = iz - pz;
								int distSq = dx * dx + dz * dz;
								if (distSq <= maxRadius * maxRadius && distSq < bestDistSq) {
									bestDistSq = distSq;
									foundX = ix;
									foundZ = iz;
									found = true;
								}
							}
						}
					}
				}
				if (found) break;
			}

			if (found) {
				int dist = (int)sqrtf((float)bestDistSq);
				int foundY = mc->level->getHeightmap(foundX + 3, foundZ + 3);
				if (foundY <= 0) foundY = 64;
				else foundY += 1;
				mc->gui.addMessage("", "Located Igloo at X: " + toStr(foundX) + ", Y: " + toStr(foundY) + ", Z: " + toStr(foundZ) + " (" + toStr(dist) + " blocks away)", 200);
			} else {
				mc->gui.addMessage("", "Igloo not found within " + toStr(maxRadius) + " blocks", 200);
			}
			return true;
		}

		/* mineshaft locate removed */

		Biome* target1 = nullptr;
		Biome* target2 = nullptr;

		if (targetName == "plains") {
			target1 = Biome::plains;
		} else if (targetName == "desert") {
			target1 = Biome::desert;
		} else if (targetName == "jungle" || targetName == "rainforest") {
			target1 = Biome::jungle;
			target2 = Biome::rainForest;
		} else if (targetName == "taiga") {
			target1 = Biome::taiga;
		} else if (targetName == "forest") {
			target1 = Biome::forest;
		} else if (targetName == "birch" || targetName == "birch_forest") {
			target1 = Biome::birchForest;
		} else if (targetName == "swamp" || targetName == "swampland") {
			target1 = Biome::swampland;
		} else if (targetName == "mountain" || targetName == "mountains" || targetName == "extreme_hills") {
			target1 = Biome::mountain;
		} else if (targetName == "snow" || targetName == "tundra" || targetName == "ice" || targetName == "ice_plains") {
			target1 = Biome::tundra;
			target2 = Biome::icePeaks;
		} else if (targetName == "savanna") {
			target1 = Biome::savanna;
		} else if (targetName == "shrubland") {
			target1 = Biome::shrubland;
		} else if (targetName == "mushroom" || targetName == "mushroom_island" || targetName == "mushroom_fields") {
			target1 = Biome::mushroom;
		} else {
			mc->gui.addMessage("", "Unknown target: " + args[0], 200);
			mc->gui.addMessage("", "Structures: village, temple, swamp_hut, igloo", 200);
			mc->gui.addMessage("", "Biomes: plains, desert, jungle, taiga, forest, birch_forest, swamp, mountains, tundra, savanna, shrubland, mushroom", 200);
			return true;
		}

		int foundX = 0, foundZ = 0, bestDistSq = 999999999;
		bool found = false;

		for (int r = 16; r <= maxRadius; r += 32) {
			for (int sx = -r; sx <= r; sx += 32) {
				for (int sz = -r; sz <= r; sz += 32) {
					if (abs(sx) == r || abs(sz) == r) {
						Biome* b = bs->getBiome(px + sx, pz + sz);
						if (b && (b == target1 || (target2 && b == target2))) {
							int distSq = sx * sx + sz * sz;
							if (distSq < bestDistSq) {
								bestDistSq = distSq;
								foundX = px + sx;
								foundZ = pz + sz;
								found = true;
							}
						}
					}
				}
			}
			if (found) break;
		}

		if (found) {
			int dist = (int)sqrtf((float)bestDistSq);
			int foundY = mc->level->getHeightmap(foundX, foundZ);
			if (foundY <= 0) foundY = 70;
			else foundY += 1;
			mc->gui.addMessage("", "Located " + args[0] + " at X: " + toStr(foundX) + ", Y: " + toStr(foundY) + ", Z: " + toStr(foundZ) + " (" + toStr(dist) + " blocks away)", 200);
		} else {
			mc->gui.addMessage("", "Biome " + args[0] + " not found within " + toStr(maxRadius) + " blocks", 200);
		}
		return true;
	}

	if (cmd == "time") {
		if (args.size() >= 2 && args[0] == "set") {
			int timeVal = 0;
			if (args[1] == "day") timeVal = 1000;
			else if (args[1] == "noon") timeVal = 6000;
			else if (args[1] == "night") timeVal = 13000;
			else if (args[1] == "midnight") timeVal = 18000;
			else {
				try {
					timeVal = parseI(args[1]);
				} catch (...) {
					mc->gui.addMessage("", "Invalid time value", 200);
					return true;
				}
			}
			if (mc->level) {
				mc->level->levelData.stopTime = -1;
				mc->level->setTime(timeVal);
				mc->level->_syncTime(timeVal);
			}
			mc->gui.addMessage("", "Set time to " + toStr(timeVal), 200);
			return true;
		}
		if (args.size() >= 2 && args[0] == "add") {
			try {
				int addVal = parseI(args[1]);
				if (mc->level) {
					mc->level->levelData.stopTime = -1;
					int newTime = mc->level->getTime() + addVal;
					mc->level->setTime(newTime);
					mc->level->_syncTime(newTime);
				}
				mc->gui.addMessage("", "Added " + toStr(addVal) + " to time", 200);
			} catch (...) {
				mc->gui.addMessage("", "Invalid time value", 200);
			}
			return true;
		}
		mc->gui.addMessage("", "Usage: /time set <day|night|noon|midnight|number> or /time add <number>", 200);
		return true;
	}

	if (cmd == "weather") {
		if (args.empty()) {
			mc->gui.addMessage("", "Usage: /weather set <clear|rain|thunder|snow> or /weather clear", 200);
			return true;
		}
		if (args[0] == "clear") {
			if (mc->level) {
				mc->level->weatherType = 0;
				mc->level->weatherTicks = 24000;
				extern int g_morningFogTicks;
				mc->level->levelEvent(0, 9810, 0, (int16_t)(mc->level->rainLevel * 1000.0f), (int16_t)g_morningFogTicks, mc->level->weatherTicks);
			}
			mc->gui.addMessage("", "Set weather to clear", 200);
			return true;
		}
		if (args.size() >= 2 && args[0] == "set") {
			std::string wtype = args[1];
			for (auto& c : wtype) c = tolower(c);
			if (wtype == "clear") {
				if (mc->level) {
					mc->level->weatherType = 0;
					mc->level->weatherTicks = 24000;
					extern int g_morningFogTicks;
					mc->level->levelEvent(0, 9810, 0, (int16_t)(mc->level->rainLevel * 1000.0f), (int16_t)g_morningFogTicks, mc->level->weatherTicks);
				}
				mc->gui.addMessage("", "Set weather to clear", 200);
			} else if (wtype == "rain" || wtype == "snow" || wtype == "rainfrogs") {
				if (mc->level) {
					mc->level->weatherType = 1;
					mc->level->isRainFrogs = (wtype == "rainfrogs");
					mc->level->weatherTicks = 24000;
					extern int g_morningFogTicks;
					mc->level->levelEvent(0, 9810, 1, (int16_t)(mc->level->rainLevel * 1000.0f), (int16_t)g_morningFogTicks, mc->level->weatherTicks);
				}
				mc->gui.addMessage("", "Set weather to rain", 200);
			} else if (wtype == "thunder" || wtype == "storm") {
				if (mc->level) {
					mc->level->weatherType = 2;
					mc->level->weatherTicks = 24000;
					extern int g_morningFogTicks;
					mc->level->levelEvent(0, 9810, 2, (int16_t)(mc->level->rainLevel * 1000.0f), (int16_t)g_morningFogTicks, mc->level->weatherTicks);
				}
				mc->gui.addMessage("", "Set weather to thunder storm", 200);
			} else {
				mc->gui.addMessage("", "Unknown weather type: " + wtype, 200);
			}
			return true;
		}
		mc->gui.addMessage("", "Usage: /weather set <clear|rain|thunder|snow> or /weather clear", 200);
		return true;
	}

	if (cmd == "list") {
		std::vector<std::string> playerNames;
		if (mc->player && !mc->player->username.empty()) {
			playerNames.emplace_back(mc->player->username);
		}
		if (mc->level) {
			for (auto* p : mc->level->playersMaybe) {
				if (p && !p->username.empty() && (playerNames.empty() || p->username != playerNames[0])) {
					playerNames.emplace_back(p->username);
				}
			}
		}
		std::string result = "Players online (" + toStr(playerNames.size()) + "): ";
		for (size_t i = 0; i < playerNames.size(); ++i) {
			result += playerNames[i];
			if (i + 1 < playerNames.size()) result += ", ";
		}
		mc->gui.addMessage("", result, 200);
		return true;
	}

	if (cmd == "give") {
		if (args.empty() || args[0] == "list" || args[0] == "help" || args[0] == "?") {
			int page = 1;
			if (args.size() > 1) {
				try {
					page = parseI(args[1]);
				} catch (...) {
					page = 1;
				}
			}
			if (page < 1) page = 1;

			std::vector<std::pair<int, std::string>> allEntries;
			for (int i = 1; i < 256; ++i) {
				if (Tile::tiles[i]) {
					std::string n = Tile::tiles[i]->getDescriptionId();
					if (n.rfind("tile.", 0) == 0) n = n.substr(5);
					allEntries.emplace_back(i, n);
				}
			}
			for (int i = 256; i < 512; ++i) {
				if (Item::items[i]) {
					std::string n = Item::items[i]->getDescriptionId();
					if (n.rfind("item.", 0) == 0) n = n.substr(5);
					allEntries.emplace_back(i, n);
				}
			}

			int pageSize = 12;
			int totalPages = ((int)allEntries.size() + pageSize - 1) / pageSize;
			if (totalPages < 1) totalPages = 1;
			if (page > totalPages) page = totalPages;

			mc->gui.addMessage("", "--- Item IDs (" + toStr(page) + "/" + toStr(totalPages) + ") ---", 200);
			std::string line = "";
			int startIdx = (page - 1) * pageSize;
			int endIdx = std::min((int)allEntries.size(), startIdx + pageSize);
			for (int i = startIdx; i < endIdx; ++i) {
				std::string entry = toStr(allEntries[i].first) + ":" + allEntries[i].second;
				if (!line.empty()) line += ", ";
				line += entry;
				if ((i - startIdx + 1) % 4 == 0 || i == endIdx - 1) {
					mc->gui.addMessage("", line, 200);
					line = "";
				}
			}
			mc->gui.addMessage("", "Usage: /give <item> [amount] [data] | /give list [page]", 200);
			return true;
		}

		Player* target = nullptr;
		size_t itemArgIdx = 0;

		if (args[0] == "@p" || args[0] == "@s" || args[0] == "@a") {
			target = mc->player;
			itemArgIdx = 1;
		} else if (mc->player && mc->player->username == args[0]) {
			target = mc->player;
			itemArgIdx = 1;
		} else if (mc->level) {
			for (auto* p : mc->level->playersMaybe) {
				if (p && p->username == args[0]) {
					target = p;
					itemArgIdx = 1;
					break;
				}
			}
		}

		if (!target) {
			int testId = 0;
			std::string testName;
			if (resolveItem(args[0], testId, testName)) {
				target = mc->player;
				itemArgIdx = 0;
			} else if (args.size() > 1) {
				mc->gui.addMessage("", "Player not found: " + args[0], 200);
				return true;
			} else {
				target = mc->player;
				itemArgIdx = 0;
			}
		}

		if (itemArgIdx >= args.size()) {
			mc->gui.addMessage("", "Usage: /give [player] <item> [amount] [data]", 200);
			return true;
		}

		int itemId = 0;
		std::string resolvedName;
		if (!resolveItem(args[itemArgIdx], itemId, resolvedName)) {
			mc->gui.addMessage("", "Unknown item: " + args[itemArgIdx], 200);
			return true;
		}

		int count = 1;
		if (itemArgIdx + 1 < args.size()) {
			try {
				count = parseI(args[itemArgIdx + 1]);
				if (count <= 0) count = 1;
				if (count > 6400) count = 6400;
			} catch (...) {
				count = 1;
			}
		}

		int meta = 0;
		if (itemArgIdx + 2 < args.size()) {
			try {
				meta = parseI(args[itemArgIdx + 2]);
				if (meta < 0) meta = 0;
			} catch (...) {
				meta = 0;
			}
		}

		if (!target || !target->inventory) {
			mc->gui.addMessage("", "Cannot give item: player inventory not available", 200);
			return true;
		}

		ItemInstance inst(itemId, count, meta);
		target->inventory->add(&inst);
		std::string recipient = (target == mc->player) ? "yourself" : target->username;
		mc->gui.addMessage("", "Given [" + resolvedName + "] * " + toStr(count) + " to " + recipient, 200);
		return true;
	}

	if (cmd == "seed") {
		mc->gui.addMessage("", "Seed: " + toStr((long long)mc->level->getSeed()), 200);
		return true;
	}

	mc->gui.addMessage("", "Unknown command: /" + cmd + ". Available: /give, /tp, /locate, /time, /list, /seed", 200);
	return true;
}

void ChatScreen::sendChatMessage() {
	if(this->field_54.size()) {
		/*
		 * A Java Edition session takes the line verbatim, slash commands
		 * included: on a Java server those belong to the server, not to the
		 * local handler below.  sendChat returns 0 when no session is up, so
		 * MCPE servers and single player keep the original behaviour.
		 */
		if(JavaBridge::sendChat(this->field_54)) {
		} else if (this->minecraft->isOnlineClient() || (this->minecraft->level && this->minecraft->level->isClientMaybe)) {
			MessagePacket v7(this->field_54, this->minecraft->player->username);
			this->minecraft->rakNetInstance->send(v7);
		} else if (this->field_54[0] == '/') {
			if (!executeCommand(this->minecraft, this->field_54)) {
				MessagePacket v7(this->field_54, this->minecraft->player->username);
				this->minecraft->rakNetInstance->send(v7);
			}
		} else {
			MessagePacket v7(this->field_54, this->minecraft->player->username);
			this->minecraft->rakNetInstance->send(v7);
			this->minecraft->gui.addMessage(this->minecraft->player->username, this->field_54, 200);
		}
		this->field_78.emplace_back(this->field_54);
		this->field_84 = this->field_78.size();
		this->field_54 = "";
		this->minecraft->platform()->updateTextBoxText(this->field_54);
	}
}
void ChatScreen::updateGuiMessages() {
	this->chatMessages = this->minecraft->gui.chatMessages;
}
void ChatScreen::updateKeyboardVisibility() {
	if(this->keyboardVisible) {
		this->minecraft->platform()->showKeyboard(&this->field_54, 160, 0);
		this->updateToggleKeyboardButton();
		this->sendChatMessageButton->setActiveAndVisibility(1, 1);
		this->field_60->active = 0;
	} else {
		this->minecraft->platform()->hideKeyboard();
		this->updateToggleKeyboardButton();
		this->sendChatMessageButton->setActiveAndVisibility(0, 0);
		this->field_60->active = 1;
	}
}
void ChatScreen::updateToggleKeyboardButton() {
	ImageDef v12;
	v12.field_0 = "gui/spritesheet.png";
	v12.field_14 = 36;
	int32_t v5;
	if(this->keyboardVisible) {
		v5 = 9;
	} else {
		v5 = 0;
	}
	v12.field_24 = 1;
	v12.field_18 = v5;
	float v6 = 13.0;
	if(this->keyboardVisible) {
		v6 = 12.0;
	}
	v12.width = v6;
	float v7 = 8.0;
	if(!this->keyboardVisible) v7 = 11;
	v12.height = v7;
	v12.field_1C = (int32_t)v6;
	v12.field_20 = (int32_t)v7;
	this->field_64->setImageDef(v12, 0);
	int32_t height = this->height;
	int32_t v11;
	if(this->keyboardVisible) {
		v11 = height / 2 - this->field_64->height;
	} else {
		v11 = height - this->field_64->height;
	}
	this->field_64->posY = v11;
}

ChatScreen::~ChatScreen() {
	safeRemove<Button>(this->closeButton);
	safeRemove<Button>(this->field_60);
	safeRemove<ImageWithBackground>(this->field_64);
	safeRemove<ImageWithBackground>(this->sendChatMessageButton);
}
void ChatScreen::render(int32_t a2, int32_t a3, float a4) {
	int32_t height; // r6
	int32_t v9;		// kr00_4
	int32_t v10;	// r5
	int32_t v11;	// r0
	int32_t v12;	// r7
	int32_t v13;	// r6

	this->renderBackground(0);
	height = this->height;
	if(this->keyboardVisible) {
		v9 = this->height;
		height /= 2;
		v10 = this->width - this->field_64->width - this->sendChatMessageButton->width;
	} else {
		v10 = this->width - this->field_64->width;
	}
	v11 = this->font->height(this->field_54, v10 - 4) + 4;
	if(v11 < this->field_64->height) {
		v12 = this->field_64->height;
	} else {
		v12 = v11;
	}
	v13 = height - v12;
	this->drawChatMessages(v13);
	Screen::render(a2, a3, a4);
	this->field_60->posX = 0;
	this->field_60->posY = this->height - v12;
	this->field_60->width = v10;
	this->field_60->height = v12;
	this->fill(0, v13, v10, this->height, -16777216);
	{
		std::string ss = this->field_54;
		if(this->keyboardVisible) {
			ss += '_';
		}
		this->font->drawWordWrap(ss, 2.0, (float)v13 + 2.0, (float)v10 - 4.0, -1, 1, 0);
	}
}
void ChatScreen::init() {
	AppPlatform* v2 = this->minecraft->platform();
	v2->showDialog(2);
	v2->createUserInput();
	this->field_60 = new BlankButton(1);
	this->field_64 = new ImageWithBackground(10);
	this->field_64->init(this->minecraft->texturesPtr, 32, 32, {112, 0, 8, 67}, {120, 0, 8, 67}, 2, 2, "gui/spritesheet.png");
	this->sendChatMessageButton = new ImageWithBackground(2);
	this->sendChatMessageButton->init(this->minecraft->texturesPtr, 32, 32, {112, 0, 8, 67}, {120, 0, 8, 67}, 2, 2, "gui/spritesheet.png");
	ImageDef v27;
	v27.field_0 = "gui/spritesheet.png";
	v27.field_24 = 1;
	v27.width = 12.0;
	v27.height = 12.0;
	v27.field_14 = 48;
	v27.field_18 = 4;
	v27.field_1C = 12;
	v27.field_20 = 12;
	this->sendChatMessageButton->setImageDef(v27, 0);
	this->closeButton = new Touch::TButton(2, "Back", 0);
	this->closeButton->width = 38;
	this->closeButton->height = 18;
	((Touch::TButton*)this->closeButton)->init(this->minecraft);
	this->buttons.push_back(this->closeButton);
	this->buttons.push_back(this->field_60);
	this->buttons.emplace_back(this->field_64);
	this->buttons.push_back(this->sendChatMessageButton);

	this->updateKeyboardVisibility();
	this->field_84 = 0;
	this->keyboardVisible = 1;
	this->updateKeyboardVisibility();
}
void ChatScreen::setupPositions() {
	this->closeButton->posX = this->width - this->closeButton->width - 4;
	this->closeButton->posY = 8;
	this->field_64->width = 20;
	this->field_64->height = 20;
	this->field_64->posX = this->width - this->field_64->width;
	this->sendChatMessageButton->width = 24;
	this->sendChatMessageButton->height = 20;
	this->sendChatMessageButton->posX = this->field_64->posX - this->sendChatMessageButton->width;
	this->sendChatMessageButton->posY = this->height / 2 - this->sendChatMessageButton->height;
	this->updateToggleKeyboardButton();
	this->field_64->setSize((float)this->field_64->width, (float)this->field_64->height);
	this->sendChatMessageButton->setSize((float)this->sendChatMessageButton->width, (float)this->sendChatMessageButton->height);
	this->minecraft->gui.field_A94 = 1;
}
bool_t ChatScreen::handleBackEvent(bool_t a2) {
	if(!a2) {
		if(this->keyboardVisible) {
			this->keyboardVisible = 0;
			this->updateKeyboardVisibility();
		} else {
			this->closeWindow();
		}
	}
	return 1;
}
void ChatScreen::tick() {
	Screen::tick();
	if(this->guiMessagesUpdated()) {
		this->updateGuiMessages();
	}
}
void ChatScreen::removed() {
	this->minecraft->gui.field_A94 = 0;
	this->keyboardVisible = 0;
	this->updateKeyboardVisibility();
}
bool_t ChatScreen::renderGameBehind() {
	return 1;
}
bool_t ChatScreen::isPauseScreen() {
	return 0;
}
bool_t ChatScreen::isErrorScreen() {
	return 0;
}
bool_t ChatScreen::isInGameScreen() {
	return 1;
}
bool_t ChatScreen::closeOnPlayerHurt() {
	return 1;
}
void ChatScreen::setTextboxText(const std::string& a2) {
	this->field_54 = a2;
}
void ChatScreen::buttonClicked(Button* a2) {
	if(a2 == this->closeButton) {
		this->closeWindow();
	} else if(a2 == this->field_60 || a2 == (Button*)this->field_64) {
		this->keyboardVisible ^= 1u;
		this->updateKeyboardVisibility();
	} else if(a2 == (Button*)this->sendChatMessageButton) {
		this->sendChatMessage();
	}
}
void ChatScreen::keyPressed(int32_t a2) {
	int32_t v7, sz, v9, v11;
	switch(a2) {
		case 8:
			if(!this->field_54.size()) {
				return;
			}
			this->field_54 = Util::utf8substring(this->field_54, 0, Util::utf8len(this->field_54) - 1);
			goto LABEL_17;
		case 13:
			this->sendChatMessage();
			return;
		case 38:
			v7 = (this->field_84 - 1);
			sz = this->field_78.size();
			v9 = sz - 1;
			if(v9 >= v7) {
				v9 = v7;
			}
			this->field_84 = v9 & ~(v9 >> 31);
			goto LABEL_15;
		case 40:
			v11 = this->field_84 + 1;
			sz = this->field_78.size();
			if(sz < v11) {
				v11 = this->field_78.size();
			}
			this->field_84 = v11 & ~(v11 >> 31);
			if(this->field_84 == sz) {
				this->field_54 = "";
LABEL_17:
				this->minecraft->platform()->updateTextBoxText(this->field_54);
				return;
			}
LABEL_15:
			if(!sz) {
				return;
			}
			this->field_54 = this->field_78[this->field_84];
			goto LABEL_17;
	}
	Screen::keyPressed(a2);
}
void ChatScreen::keyboardNewChar(const std::string& a2, bool_t a3) {
	std::string v8(this->field_54);
	if(this->field_59) {
		v8 = Util::utf8substring(v8, 0, Util::utf8len(v8) - 1);
	}
	v8 += a2;
	this->field_59 = a3;
	if(Util::utf8len(v8) <= 160) {
		this->field_54 = v8;
	}
}
