#include <level/LevelHeight.hpp>
#include <entity/Player.hpp>
#include <item/ArmorItem.hpp>
#include <level/Level.hpp>
#include <map>
#include <math.h>
#include <math/Mth.hpp>
#include <mutex>
#include <rendering/EntityRenderDispatcher.hpp>
#include <rendering/Font.hpp>
#include <rendering/Tesselator.hpp>
#include <rendering/Textures.hpp>
#include <rendering/entity/PlayerRenderer.hpp>
#include <rendering/model/HumanoidModel.hpp>
#include <stb_image.h>
#include <string.h>
#include <stdlib.h>
#include <thread>
#include <tile/Tile.hpp>
#include <tile/material/Material.hpp>
#include <util/CrossPlatformWeb.hpp>
#include <Options.hpp>
#include <rendering/Tesselator.hpp>
#include <math.h>

#include <chrono>
#include <unigl.h>

static bool isPlayerSwimming(Player *p) {
  if (!p || !p->isAlive() || p->isSleeping() || p->isRiding())
    return false;
  if (p->entityHeight < 0.8f)
    return true;
  if (p->isInWater() || p->isUnderLiquid(Material::water))
    return true;
  if (p->level) {
    int px = (int)floorf(p->posX);
    int pz = (int)floorf(p->posZ);
    for (float yo = -0.5f; yo <= 1.4f; yo += 0.35f) {
      int py = (int)floorf(p->posY + yo);
      if (LevelHeight::inRange(py)) {
        int tile = p->level->getTile(px, py, pz);
        Material *mat = p->level->getMaterial(px, py, pz);
        if (tile == Tile::water->blockID || tile == Tile::calmWater->blockID ||
            (Tile::seagrass && tile == Tile::seagrass->blockID) ||
            (mat && mat == Material::water)) {
          return true;
        }
      }
    }
  }
  return false;
}

PlayerRenderer::PlayerRenderer(HumanoidModel *a2, float a3)
    : HumanoidMobRenderer(a2, a3) {
  this->hmodel1 = new HumanoidModel(1.0, 0.0);
  this->hmodel2 = new HumanoidModel(0.5, 0.0);
}

PlayerRenderer::~PlayerRenderer() {
  if (this->hmodel1)
    delete this->hmodel1;
  if (this->hmodel2)
    delete this->hmodel2;
}

static std::string _dword_D6E083C8_armorTextures[] = {
    "armor/cloth_1.png",   "armor/cloth_2.png",   "armor/chain_1.png",
    "armor/chain_2.png",   "armor/iron_1.png",    "armor/iron_2.png",
    "armor/diamond_1.png", "armor/diamond_2.png", "armor/gold_1.png",
    "armor/gold_2.png"};

void PlayerRenderer::renderName(Entity *a2_, float a3) {
  Player *a2 = (Player *)a2_;
  if (!a2 || a2->isLocalPlayer() || a2->username.empty()) return;
  Mob *cameraEntity = EntityRenderer::entityRenderDispatcher->cameraEntity;
  if (a2 != cameraEntity && a2->distanceToSqr(cameraEntity) <= 1024.0) {
    float v7 =
        (float)(a2->prevPosX + (float)((float)(a2->posX - a2->prevPosX) * a3)) -
        EntityRenderDispatcher::xOff;
    float v8 =
        (float)(a2->prevPosY + (float)((float)(a2->posY - a2->prevPosY) * a3)) -
        EntityRenderDispatcher::yOff;
    float v9 =
        (float)(a2->prevPosZ + (float)((float)(a2->posZ - a2->prevPosZ) * a3)) -
        EntityRenderDispatcher::zOff;
    Font *font = this->getFont();
    if (!font) return;
    glPushMatrix();
    float nameY = a2->isSneaking() ? 0.5f : 0.7f;
    glTranslatef(v7, v8 + nameY, v9);
    glRotatef(-EntityRenderer::entityRenderDispatcher->field_14, 0.0f, 1.0f, 0.0f);
    glRotatef(EntityRenderer::entityRenderDispatcher->field_18, 1.0f, 0.0f, 0.0f);
    glScalef(-0.026667f, -0.026667f, 0.026667f);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    int32_t v11 = font->width(a2->username);
    Tesselator::instance.begin(7);
    Tesselator::instance.color(0.0f, 0.0f, 0.0f, 0.35f);
    Tesselator::instance.vertex((float)(-v11 / 2 - 1), -1.0f, 0.0f);
    Tesselator::instance.vertex((float)(-v11 / 2 - 1), 8.0f, 0.0f);
    Tesselator::instance.vertex((float)(v11 / 2 + 1), 8.0f, 0.0f);
    Tesselator::instance.vertex((float)(v11 / 2 + 1), -1.0f, 0.0f);
    Tesselator::instance.draw(1);
    glEnable(GL_TEXTURE_2D);
    float v12 = (float)font->width(a2->username) * -0.5f;
    font->draw(a2->username, v12, 0.0f, 553648127, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    font->draw(a2->username, v12, 0.0f, -1, 0);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();
  }
}
int32_t PlayerRenderer::prepareArmor(Mob *a2_, int32_t armour, float a4) {
  Player *a2 = (Player *)a2_;
  ItemInstance *armor;
  HumanoidModel *hmodel2;
  bool_t v9;
  bool_t v10;

  armor = a2->getArmor(armour);
  if (!ItemInstance::isArmorItem(armor)) {
    return -1;
  }
  this->bindTexture(
      _dword_D6E083C8_armorTextures
          [2 * ((ArmorItem *)armor->itemClass)->field_50 + (armour == 2)]);
  if (armour == 2) {
    hmodel2 = this->hmodel2;
  } else {
    hmodel2 = this->hmodel1;
  }
  hmodel2->headModel.field_1D = armour == 0;
  hmodel2->bodyModel.field_1D = (uint32_t)(armour - 1) <= 1;
  v9 = armour == 1;
  hmodel2->rightArmModel.field_1D = armour == 1;
  v10 = (uint32_t)(armour - 2) <= 1;
  hmodel2->leftArmModel.field_1D = v9;
  hmodel2->rightLegModel.field_1D = v10;
  hmodel2->leftLegModel.field_1D = v10;
  this->setArmor(hmodel2);
  hmodel2->field_4 = a2->isRiding();
  hmodel2->field_31A = a2->isSneaking();
  hmodel2->isSwimming = isPlayerSwimming(a2);
  hmodel2->field_0 = this->getAttackAnim(a2, a4);
  return 1;
}
#include <Minecraft.hpp>
#include <cstring>
#include <entity/LocalPlayer.hpp>
#ifdef _WIN32
#include <direct.h>
#define mkdirPortable(p) _mkdir(p)
#else
#include <sys/stat.h>
#define mkdirPortable(p) mkdir(p, 0777)
#endif

static std::map<std::string, unsigned char *> g_pendingSkinsData;
static std::map<std::string, std::pair<int, int>> g_pendingSkinsDims;
static std::map<std::string, unsigned char *> g_pendingCapesData;
static std::map<std::string, std::pair<int, int>> g_pendingCapesDims;
static std::map<std::string, uint64_t> g_lastSkinCheckTime;
static std::mutex skinMutex;
static std::map<int32_t, std::pair<float, float>> g_capeRotMap;

static bool isBase64Char(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string decodeBase64(const std::string &encoded) {
  static const std::string b64 =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  int in_len = (int)encoded.size();
  int i = 0, in_ = 0;
  unsigned char a4[4], a3[3];
  while (in_len-- && (encoded[in_] != '=') && isBase64Char(encoded[in_])) {
    a4[i++] = encoded[in_++];
    if (i == 4) {
      for (i = 0; i < 4; i++)
        a4[i] = (unsigned char)b64.find(a4[i]);
      a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
      a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
      a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
      for (i = 0; i < 3; i++)
        out += a3[i];
      i = 0;
    }
  }
  if (i) {
    for (int j = i; j < 4; j++)
      a4[j] = 0;
    for (int j = 0; j < 4; j++)
      a4[j] = (unsigned char)b64.find(a4[j]);
    a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
    a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
    a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
    for (int j = 0; j < i - 1; j++)
      out += a3[j];
  }
  return out;
}

static std::string extractJsonStr(const std::string &json,
                                  const std::string &key) {
  std::string search = "\"" + key + "\"";
  size_t pos = json.find(search);
  if (pos == std::string::npos)
    return "";
  pos = json.find(':', pos + search.size());
  if (pos == std::string::npos)
    return "";
  pos++;
  while (pos < json.size() && isspace((unsigned char)json[pos]))
    pos++;
  if (pos >= json.size() || json[pos] != '"')
    return "";
  pos++;
  size_t end = json.find('"', pos);
  if (end == std::string::npos)
    return "";
  return json.substr(pos, end - pos);
}

static std::string getMojangTextureUrl(const std::string &username,
                                       const std::string &textureKey) {
  if (username.empty())
    return "";
  std::string resp = CrossPlatform_DownloadString(
      "https://api.mojang.com/users/profiles/minecraft/" + username);
  if (resp.empty()) {
    resp = CrossPlatform_DownloadString(
        "http://api.mojang.com/users/profiles/minecraft/" + username);
  }
  if (resp.empty())
    return "";
  std::string uuid = extractJsonStr(resp, "id");
  if (uuid.empty())
    return "";
  std::string prof = CrossPlatform_DownloadString(
      "https://sessionserver.mojang.com/session/minecraft/profile/" + uuid);
  if (prof.empty()) {
    prof = CrossPlatform_DownloadString(
        "http://sessionserver.mojang.com/session/minecraft/profile/" + uuid);
  }
  if (prof.empty())
    return "";
  std::string encoded = extractJsonStr(prof, "value");
  if (encoded.empty())
    return "";
  std::string decoded = decodeBase64(encoded);
  size_t texPos = decoded.find("\"" + textureKey + "\"");
  if (texPos == std::string::npos)
    return "";
  size_t urlPos = decoded.find("\"url\"", texPos);
  if (urlPos == std::string::npos)
    return "";
  return extractJsonStr(decoded.substr(urlPos), "url");
}

static uint64_t getCurrentTimeMillis() {
  return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

static unsigned char *processSkinImage(const unsigned char *src, int w, int h,
                                       int &outW, int &outH) {
  outW = 64;
  outH = 32;
  unsigned char *out = (unsigned char *)calloc(64 * 32 * 4, 1);
  if (!out)
    return nullptr;

  int srcHalfH = (h == w) ? (h / 2) : h;

  for (int dy = 0; dy < 32; ++dy) {
    int sy = dy * srcHalfH / 32;
    if (sy >= h)
      sy = h - 1;
    for (int dx = 0; dx < 64; ++dx) {
      int sx = dx * w / 64;
      if (sx >= w)
        sx = w - 1;
      int srcIdx = (sy * w + sx) * 4;
      int dstIdx = (dy * 64 + dx) * 4;
      out[dstIdx + 0] = src[srcIdx + 0];
      out[dstIdx + 1] = src[srcIdx + 1];
      out[dstIdx + 2] = src[srcIdx + 2];
      out[dstIdx + 3] = src[srcIdx + 3];

      if (!(dx >= 32 && dx < 64 && dy < 16)) {
        out[dstIdx + 3] = 255;
      }
    }
  }
  return out;
}

static void uploadSkinTexture(const std::string &texName,
                              const unsigned char *px, int w, int h) {
  int outW = 64, outH = 32;
  unsigned char *uploadPx = processSkinImage(px, w, h, outW, outH);
  if (!uploadPx)
    return;

  Textures *tex = EntityRenderer::entityRenderDispatcher->textures;
  if (!tex) {
    free(uploadPx);
    return;
  }

  auto itTex = tex->textures.find(texName);
  GLuint skinTexId = 0;
  if (itTex != tex->textures.end() && itTex->second.glTexId != 0) {
    skinTexId = itTex->second.glTexId;
    glBindTexture(GL_TEXTURE_2D, skinTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outW, outH, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, uploadPx);
    itTex->second.width = outW;
    itTex->second.height = outH;
  } else {
    glGenTextures(1, &skinTexId);
    glBindTexture(GL_TEXTURE_2D, skinTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outW, outH, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, uploadPx);
    TextureData td;
    td.width = outW;
    td.height = outH;
    td.pixels = nullptr;
    td.glTexId = skinTexId;
    tex->textures.erase(texName);
    tex->textures.insert(
        std::pair<std::string, TextureData>(texName, std::move(td)));
  }
  tex->currentTexture = 0;
  free(uploadPx);
}

static std::vector<unsigned char>
downloadSkinData(const std::string &username, int skinSource) {
  if (username.empty())
    return {};

  if (skinSource == 1) {
    std::vector<std::string> urls = {
        "https://raw.githubusercontent.com/gameherobrine2/test123/refs/heads/"
        "main/" +
            username + ".png",
        "https://raw.githubusercontent.com/gameherobrine2/test123/main/" +
            username + ".png",
        "https://raw.githubusercontent.com/gameherobrine2/test123/master/" +
            username + ".png",
        "https://github.com/gameherobrine2/test123/raw/main/" + username + ".png",
    };

    std::string lowerName = username;
    for (char &c : lowerName)
      c = (char)tolower((unsigned char)c);
    if (lowerName != username) {
      urls.push_back("https://raw.githubusercontent.com/gameherobrine2/test123/"
                     "refs/heads/main/" +
                     lowerName + ".png");
      urls.push_back(
          "https://raw.githubusercontent.com/gameherobrine2/test123/main/" +
          lowerName + ".png");
      urls.push_back(
          "https://raw.githubusercontent.com/gameherobrine2/test123/master/" +
          lowerName + ".png");
    }

    for (const auto &url : urls) {
      auto data = CrossPlatform_DownloadBinary(url);
      if (!data.empty() && data.size() > 64) {
        return data;
      }
    }
    return {};
  }

  std::string mojangSkinUrl = getMojangTextureUrl(username, "SKIN");
  if (!mojangSkinUrl.empty()) {
    auto data = CrossPlatform_DownloadBinary(mojangSkinUrl);
    if (!data.empty() && data.size() > 64) {
      return data;
    }
  }
  return {};
}

static std::vector<unsigned char>
downloadCapeData(const std::string &username) {
  if (username.empty())
    return {};

  std::string mojangCapeUrl = getMojangTextureUrl(username, "CAPE");
  if (!mojangCapeUrl.empty()) {
    auto data = CrossPlatform_DownloadBinary(mojangCapeUrl);
    if (!data.empty() && data.size() > 64) {
      return data;
    }
  }
  return {};
}

static void uploadCapeTexture(const std::string &username,
                              const unsigned char *px, int w, int h) {
  int outW = 64, outH = 32;
  unsigned char *uploadPx = (unsigned char *)calloc(outW * outH * 4, 1);
  if (!uploadPx) return;
  for (int y = 0; y < outH && y < h; ++y) {
    for (int x = 0; x < outW && x < w; ++x) {
      int si = (y * w + x) * 4;
      int di = (y * outW + x) * 4;
      uploadPx[di + 0] = px[si + 0];
      uploadPx[di + 1] = px[si + 1];
      uploadPx[di + 2] = px[si + 2];
      uploadPx[di + 3] = 255;
    }
  }
  std::string texName = username + "_cape";
  Textures *tex = EntityRenderer::entityRenderDispatcher->textures;
  if (!tex) { free(uploadPx); return; }
  auto itTex = tex->textures.find(texName);
  GLuint texId = 0;
  if (itTex != tex->textures.end() && itTex->second.glTexId != 0) {
    texId = itTex->second.glTexId;
    glBindTexture(GL_TEXTURE_2D, texId);
    itTex->second.width = outW;
    itTex->second.height = outH;
  } else {
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    TextureData td;
    td.width = outW; td.height = outH; td.pixels = nullptr; td.glTexId = texId;
    tex->textures.erase(texName);
    tex->textures.insert(std::pair<std::string, TextureData>(texName, std::move(td)));
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outW, outH, 0, GL_RGBA, GL_UNSIGNED_BYTE, uploadPx);
  tex->currentTexture = 0;
  free(uploadPx);
}

void PlayerRenderer::updateSkin(Player *a2) {
  if (!a2) return;
  std::string nick = a2->username;
  if (nick.empty() && EntityRenderer::entityRenderDispatcher &&
      EntityRenderer::entityRenderDispatcher->minecraft) {
    if (a2 == EntityRenderer::entityRenderDispatcher->minecraft->player) {
      nick =
          EntityRenderer::entityRenderDispatcher->minecraft->options.username;
      if (a2->username.empty())
        a2->username = nick;
    }
  }

  if (!nick.empty()) {
    int skinSource = (Options::instance && Options::instance->skinSource == 1) ? 1 : 0;
    std::string sourceTag = (skinSource == 1) ? "local" : "mojang";
    std::string texName = nick + "_skin_" + sourceTag;
    std::string cachePath = "skin_cache/" + nick + "_" + sourceTag + ".png";
    std::string capeTexName = nick + "_cape";
    Textures *tex = EntityRenderer::entityRenderDispatcher ? EntityRenderer::entityRenderDispatcher->textures : nullptr;

    if (tex && (tex->textures.find(texName) == tex->textures.end() ||
                tex->textures[texName].glTexId == 0)) {
      mkdirPortable("skin_cache");
      int cw = 0, ch = 0, cch = 0;
      unsigned char *cachedPx =
          stbi_load(cachePath.c_str(), &cw, &ch, &cch, STBI_rgb_alpha);
      if (cachedPx) {
        uploadSkinTexture(texName, cachedPx, cw, ch);
        stbi_image_free(cachedPx);
      }
    }

    if (tex && (tex->textures.find(capeTexName) == tex->textures.end() ||
                tex->textures[capeTexName].glTexId == 0)) {
      std::string capeCache = "skin_cache/" + nick + "_cape.png";
      int cw2 = 0, ch2 = 0, cch2 = 0;
      unsigned char *capePx =
          stbi_load(capeCache.c_str(), &cw2, &ch2, &cch2, STBI_rgb_alpha);
      if (capePx) {
        uploadCapeTexture(nick, capePx, cw2, ch2);
        stbi_image_free(capePx);
      }
    }

    uint64_t now = getCurrentTimeMillis();
    bool doSkinCheck = false;
    bool doCapeCheck = false;
    {
      std::lock_guard<std::mutex> lock(skinMutex);
      if (g_lastSkinCheckTime.find(texName) == g_lastSkinCheckTime.end() ||
          now - g_lastSkinCheckTime[texName] >= 15000) {
        g_lastSkinCheckTime[texName] = now;
        doSkinCheck = true;
      }
      if (g_lastSkinCheckTime.find(capeTexName) == g_lastSkinCheckTime.end() ||
          now - g_lastSkinCheckTime[capeTexName] >= 15000) {
        g_lastSkinCheckTime[capeTexName] = now;
        doCapeCheck = true;
      }
    }

    if (doSkinCheck || doCapeCheck) {
      std::thread([nick, skinSource, texName, cachePath, doSkinCheck, doCapeCheck]() {
        if (doSkinCheck) {
          auto bin = downloadSkinData(nick, skinSource);
          if (!bin.empty()) {
            mkdirPortable("skin_cache");
            FILE *wf = fopen(cachePath.c_str(), "wb");
            if (wf) {
              fwrite(bin.data(), 1, bin.size(), wf);
              fclose(wf);
            }

            int w, h, ch;
            unsigned char *px = stbi_load_from_memory(
                bin.data(), (int)bin.size(), &w, &h, &ch, STBI_rgb_alpha);
            if (px) {
              std::lock_guard<std::mutex> lock(skinMutex);
              if (g_pendingSkinsData.find(texName) != g_pendingSkinsData.end()) {
                stbi_image_free(g_pendingSkinsData[texName]);
              }
              g_pendingSkinsData[texName] = px;
              g_pendingSkinsDims[texName] = {w, h};
            }
          }
        }

        if (doCapeCheck) {
          auto capeBin = downloadCapeData(nick);
          if (!capeBin.empty()) {
            mkdirPortable("skin_cache");
            std::string capeCachePath = "skin_cache/" + nick + "_cape.png";
            FILE *cwf = fopen(capeCachePath.c_str(), "wb");
            if (cwf) {
              fwrite(capeBin.data(), 1, capeBin.size(), cwf);
              fclose(cwf);
            }

            int cw, ch, cch;
            unsigned char *cpx = stbi_load_from_memory(
                capeBin.data(), (int)capeBin.size(), &cw, &ch, &cch,
                STBI_rgb_alpha);
            if (cpx) {
              std::lock_guard<std::mutex> lock(skinMutex);
              if (g_pendingCapesData.find(nick) != g_pendingCapesData.end()) {
                stbi_image_free(g_pendingCapesData[nick]);
              }
              g_pendingCapesData[nick] = cpx;
              g_pendingCapesDims[nick] = {cw, ch};
            }
          }
        }
      }).detach();
    }

    {
      std::lock_guard<std::mutex> lock(skinMutex);
      auto it = g_pendingSkinsData.find(texName);
      if (it != g_pendingSkinsData.end()) {
        unsigned char *px = it->second;
        int w = g_pendingSkinsDims[texName].first;
        int h = g_pendingSkinsDims[texName].second;
        uploadSkinTexture(texName, px, w, h);
        stbi_image_free(px);
        g_pendingSkinsData.erase(it);
      }
      auto cit = g_pendingCapesData.find(nick);
      if (cit != g_pendingCapesData.end()) {
        unsigned char *cpx = cit->second;
        int cw = g_pendingCapesDims[nick].first;
        int ch = g_pendingCapesDims[nick].second;
        uploadCapeTexture(nick, cpx, cw, ch);
        stbi_image_free(cpx);
        g_pendingCapesData.erase(cit);
      }
    }

    if (tex && tex->textures.count(texName) &&
        tex->textures[texName].glTexId != 0) {
      a2->skin = texName;
    } else {
      a2->skin = "mob/char.png";
    }

    if (tex && tex->textures.count(capeTexName) &&
        tex->textures[capeTexName].glTexId != 0) {
      a2->setCapeTexture(capeTexName);
    }
  }
}

void PlayerRenderer::setupPosition(Entity *a2_, float a3, float a4, float a5) {
  Player *a2 = (Player *)a2_;
  if (a2) {
    updateSkin(a2);
  }

  float v11, v12;
  if (a2->isAlive() && a2->isSleeping()) {
    v11 = a3 + a2->field_CC4;
    v12 = a4 + a2->field_CC8;
    a5 = a5 + a2->field_CCC;
  } else {
    v11 = a3;
    v12 = a4;
  }
  MobRenderer::setupPosition(a2, v11, v12, a5);
}
void PlayerRenderer::setupRotations(Entity *a2_, float a3, float a4, float a5) {
  Player *a2 = (Player *)a2_;
  float sleepRot, v10;
  if (a2->isAlive() && a2->isSleeping()) {
    sleepRot = a2->getSleepRotation();
    glRotatef(sleepRot, 0.0, 1.0, 0.0);
    v10 = this->getFlipDegrees(a2);
    glRotatef(v10, 0.0, 0.0, 1.0);
    glRotatef(270.0, 0.0, 1.0, 0.0);
  } else {
    MobRenderer::setupRotations(a2, a3, a4, a5);
  }
}
int32_t PlayerRenderer::getArmorTransparencyMode() { return 3008; }


void PlayerRenderer::additionalRendering(Mob* a2, float a3) {
	HumanoidMobRenderer::additionalRendering(a2, a3);
	if (!a2 || !a2->isPlayer()) return;
	Player* player = (Player*)a2;
	if (!Options::instance || !Options::instance->showCape) return;
	if (player->capeTexture.empty()) return;

	this->bindTexture(player->capeTexture);

	glPushMatrix();
	this->hmodel->bodyModel.translateTo(0.0625f);
	glScalef(0.0625f, 0.0625f, 0.0625f);
	ItemInstance* chestArmor = player->getArmor(1);
	float zOff = (chestArmor && !chestArmor->isNull()) ? 3.2f : 2.15f;
	glTranslatef(0.0f, 1.0f, zOff);

	if (player->isSneaking()) {
		glTranslatef(0.0f, 1.5f, 0.0f);
	}

	float pt = a3;
	double capeX = player->xc + (player->xCape - player->xc) * pt;
	double capeY = player->yc + (player->yCape - player->yc) * pt;
	double capeZ = player->zc + (player->zCape - player->zc) * pt;

	double px = player->prevPosX + (player->posX - player->prevPosX) * pt;
	double py = player->prevPosY + (player->posY - player->prevPosY) * pt;
	double pz = player->prevPosZ + (player->posZ - player->prevPosZ) * pt;

	double dx = capeX - px;
	double dy = capeY - py;
	double dz = capeZ - pz;

	float bodyYaw = player->field_128 + Mth::wrapDegrees(player->field_124 - player->field_128) * pt;
	float rad = bodyYaw * 3.1415926535f / 180.0f;
	double sinYaw = sinf(rad);
	double cosYaw = -cosf(rad);

	float forward = (float)(dx * sinYaw + dz * cosYaw) * 100.0f;
	float sideways = (float)(dx * cosYaw - dz * sinYaw) * 100.0f;
	if (forward < 0.0f) forward = 0.0f;
	if (forward > 165.0f) forward = 165.0f;
	if (sideways < -20.0f) sideways = -20.0f;
	if (sideways > 20.0f) sideways = 20.0f;

	float lift = (float)dy * 10.0f;
	if (lift < -6.0f) lift = -6.0f;
	if (lift > 32.0f) lift = 32.0f;

	float walkSpeed = player->field_168 + (player->field_16C - player->field_168) * pt;
	if (walkSpeed > 1.0f) walkSpeed = 1.0f;
	float walkPos = player->field_170 - (1.0f - pt) * player->field_16C;
	float walk = sinf(walkPos * 0.6662f * 2.0f) * 6.0f * walkSpeed;

	float capeXRot = 6.0f + forward / 2.0f + lift + walk + (player->isSneaking() ? 25.0f : 0.0f);
	float capeZRot = sideways / 2.0f;
	if (capeXRot < 0.0f) capeXRot = 0.0f;
	if (capeXRot > 150.0f) capeXRot = 150.0f;
	if (capeZRot < -15.0f) capeZRot = -15.0f;
	if (capeZRot > 15.0f) capeZRot = 15.0f;

	static std::map<int32_t, std::pair<float, float>> s_capeSmooth;
	auto& smoothState = s_capeSmooth[player->entityId];
	const float smooth = 0.3f;
	capeXRot = smoothState.first + (capeXRot - smoothState.first) * smooth;
	capeZRot = smoothState.second + (capeZRot - smoothState.second) * smooth;
	smoothState.first = capeXRot;
	smoothState.second = capeZRot;

	glRotatef(capeXRot, 1.0f, 0.0f, 0.0f);
	glRotatef(capeZRot, 0.0f, 0.0f, 1.0f);

	GLboolean cullWas = glIsEnabled(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	Tesselator& t = Tesselator::instance;
	t.begin(24);

	const float u0 = 1.0f / 64.0f;
	const float u1 = 11.0f / 64.0f;
	const float u2 = 12.0f / 64.0f;
	const float u3 = 22.0f / 64.0f;

	const float uL0 = 0.0f / 64.0f;
	const float uL1 = 1.0f / 64.0f;

	const float uR0 = 11.0f / 64.0f;
	const float uR1 = 12.0f / 64.0f;

	const float v0 = 0.0f / 32.0f;
	const float v1 = 1.0f / 32.0f;

	const float vTop = 1.0f / 32.0f;
	const float vBottom = 17.0f / 32.0f;

	const float halfW = 5.0f;
	const float height = 16.0f;
	const float depth = 1.0f;

	t.vertexUV(-halfW, 0.0f, 0.0f, u2, vTop);
	t.vertexUV(halfW, 0.0f, 0.0f, u3, vTop);
	t.vertexUV(halfW, height, 0.0f, u3, vBottom);
	t.vertexUV(-halfW, height, 0.0f, u2, vBottom);

	t.vertexUV(halfW, 0.0f, depth, u0, vTop);
	t.vertexUV(-halfW, 0.0f, depth, u1, vTop);
	t.vertexUV(-halfW, height, depth, u1, vBottom);
	t.vertexUV(halfW, height, depth, u0, vBottom);

	t.vertexUV(-halfW, 0.0f, depth, uL0, vTop);
	t.vertexUV(-halfW, 0.0f, 0.0f, uL1, vTop);
	t.vertexUV(-halfW, height, 0.0f, uL1, vBottom);
	t.vertexUV(-halfW, height, depth, uL0, vBottom);

	t.vertexUV(halfW, 0.0f, 0.0f, uR0, vTop);
	t.vertexUV(halfW, 0.0f, depth, uR1, vTop);
	t.vertexUV(halfW, height, depth, uR1, vBottom);
	t.vertexUV(halfW, height, 0.0f, uR0, vBottom);

	t.vertexUV(-halfW, 0.0f, depth, u0, v0);
	t.vertexUV(halfW, 0.0f, depth, u1, v0);
	t.vertexUV(halfW, 0.0f, 0.0f, u1, v1);
	t.vertexUV(-halfW, 0.0f, 0.0f, u0, v1);

	t.vertexUV(halfW, height, 0.0f, u2, v0);
	t.vertexUV(-halfW, height, 0.0f, u3, v0);
	t.vertexUV(-halfW, height, depth, u3, v1);
	t.vertexUV(halfW, height, depth, u2, v1);

	t.draw(1);
	if (cullWas) glEnable(GL_CULL_FACE);
	glPopMatrix();
}
