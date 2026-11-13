#include <Minecraft.hpp>
#include <gui/screens/SplashScreen.hpp>
#include <gui/screens/FirstLaunchAcknowledgementScreen.hpp>
#include <math.h>
#include <algorithm>
#include <rendering/Font.hpp>
#include <rendering/Tesselator.hpp>
#include <rendering/Textures.hpp>
#include <unigl.h>

SplashScreen::SplashScreen() : Screen() {
  this->timer = 0.0f;
  this->duration = 1.5f;
  this->fadeDuration = 0.0f;
}

SplashScreen::~SplashScreen() {}

bool_t SplashScreen::isPauseScreen() { return 1; }

bool_t SplashScreen::renderGameBehind() { return 0; }

bool_t SplashScreen::handleBackEvent(bool_t) { return 0; }

void SplashScreen::tick() {
  this->timer += 0.05f;
  if (this->timer >= this->duration) {
    if (!this->minecraft->options.firstLaunchAcknowledged) {
      this->minecraft->setScreen(new FirstLaunchAcknowledgementScreen());
    } else {
      this->minecraft->screenChooser.setScreen(START_MENU_SCREEN);
    }
  }
}

void SplashScreen::render(int32_t mx, int32_t my, float delta) {
  this->fill(0, 0, this->width, this->height, 0xFFE0E0E0);
  if (!this->font) return;

  const char* text = "MODIFIEDEIGHT";
  int32_t tw = this->font->width(text);
  float scale = (this->width >= 360) ? 4.2f : 2.8f;
  if (tw > 0) {
    float maxScale = ((float)this->width * 0.88f) / (float)tw;
    if (scale > maxScale) scale = maxScale;
  }
  float x = floorf(((float)this->width - (float)tw * scale) * 0.5f);
  float y = floorf(((float)this->height - 8.0f * scale) * 0.5f);

  glEnable(GL_TEXTURE_2D);
  glPushMatrix();
  glTranslatef(x, y, 0.0f);
  glScalef(scale, scale, 1.0f);
  this->font->draw(text, 1.0f, 1.0f, 0xFFA0A0A0);
  this->font->draw(text, 0.0f, 0.0f, 0xFF303030);
  glPopMatrix();
}
