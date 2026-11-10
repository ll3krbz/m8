#include <rendering/entity/ItemFrameRenderer.hpp>
#include <entity/ItemFrame.hpp>
#include <rendering/Tesselator.hpp>
#include <rendering/EntityRenderDispatcher.hpp>
#include <rendering/entity/ItemRenderer.hpp>
#include <rendering/TileRenderer.hpp>
#include <item/Item.hpp>
#include <tile/Tile.hpp>
#include <tile/MobHeadTile.hpp>
#include <rendering/tileentity/MobHeadRenderer.hpp>
#include <unigl.h>

static bool_t canRenderFrameTile(int32_t shape) {
	switch (shape) {
		case 0:
		case 1:
		case 6:
		case 10:
		case 11:
		case 13:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 31:
		case 32:
			return 1;
	}
	return 0;
}

ItemFrameRenderer::ItemFrameRenderer()
	: EntityRenderer() {
}

ItemFrameRenderer::~ItemFrameRenderer() {
}

void ItemFrameRenderer::render(Entity* entity, float x, float y, float z, float rot, float a6) {
	ItemFrame* frame = (ItemFrame*)entity;
	if (!frame) return;

	float yaw = rot;
	switch (frame->direction) {
		case 0:
			yaw = 0.0f;
			break;
		case 1:
			yaw = 270.0f;
			break;
		case 2:
			yaw = 180.0f;
			break;
		case 3:
			yaw = 90.0f;
			break;
	}

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(yaw, 0.0f, 1.0f, 0.0f);
	if (frame->direction == 4) {
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	} else if (frame->direction == 5) {
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	}

	GLboolean lightingWas = glIsEnabled(GL_LIGHTING);
	GLboolean cullWas = glIsEnabled(GL_CULL_FACE);
	GLboolean blendWas = glIsEnabled(GL_BLEND);
	GLboolean alphaTestWas = glIsEnabled(GL_ALPHA_TEST);
	GLboolean depthTestWas = glIsEnabled(GL_DEPTH_TEST);
	GLboolean colorArrayWas = glIsEnabled(GL_COLOR_ARRAY);
	GLboolean normalArrayWas = glIsEnabled(GL_NORMAL_ARRAY);
	GLboolean texArrayWas = glIsEnabled(GL_TEXTURE_COORD_ARRAY);

	if (lightingWas) glDisable(GL_LIGHTING);
	if (cullWas) glDisable(GL_CULL_FACE);
	if (!depthTestWas) glEnable(GL_DEPTH_TEST);
	if (colorArrayWas) glDisableClientState(GL_COLOR_ARRAY);
	if (normalArrayWas) glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	Tesselator& t = Tesselator::instance;
	this->bindTexture("item_frame.png");

	t.begin(4);
	t.vertexUV(-0.5f, -0.5f, -0.0625f, 0.0f, 1.0f);
	t.vertexUV(0.5f, -0.5f, -0.0625f, 1.0f, 1.0f);
	t.vertexUV(0.5f, 0.5f, -0.0625f, 1.0f, 0.0f);
	t.vertexUV(-0.5f, 0.5f, -0.0625f, 0.0f, 0.0f);

	t.vertexUV(-0.5f, 0.5f, 0.0f, 0.0f, 0.0f);
	t.vertexUV(0.5f, 0.5f, 0.0f, 1.0f, 0.0f);
	t.vertexUV(0.5f, 0.5f, -0.0625f, 1.0f, 0.0625f);
	t.vertexUV(-0.5f, 0.5f, -0.0625f, 0.0f, 0.0625f);

	t.vertexUV(-0.5f, -0.5f, -0.0625f, 0.0f, 0.9375f);
	t.vertexUV(0.5f, -0.5f, -0.0625f, 1.0f, 0.9375f);
	t.vertexUV(0.5f, -0.5f, 0.0f, 1.0f, 1.0f);
	t.vertexUV(-0.5f, -0.5f, 0.0f, 0.0f, 1.0f);

	t.vertexUV(-0.5f, -0.5f, 0.0f, 0.0f, 1.0f);
	t.vertexUV(-0.5f, 0.5f, 0.0f, 0.0f, 0.0f);
	t.vertexUV(-0.5f, 0.5f, -0.0625f, 0.0625f, 0.0f);
	t.vertexUV(-0.5f, -0.5f, -0.0625f, 0.0625f, 1.0f);

	t.vertexUV(0.5f, -0.5f, -0.0625f, 0.9375f, 1.0f);
	t.vertexUV(0.5f, 0.5f, -0.0625f, 0.9375f, 0.0f);
	t.vertexUV(0.5f, 0.5f, 0.0f, 1.0f, 0.0f);
	t.vertexUV(0.5f, -0.5f, 0.0f, 1.0f, 1.0f);

	t.vertexUV(0.5f, -0.5f, 0.0f, 1.0f, 1.0f);
	t.vertexUV(-0.5f, -0.5f, 0.0f, 0.0f, 1.0f);
	t.vertexUV(-0.5f, 0.5f, 0.0f, 0.0f, 0.0f);
	t.vertexUV(0.5f, 0.5f, 0.0f, 1.0f, 0.0f);
	t.draw(1);

	ItemInstance dispItem = frame->getDisplayedItem();
	if (!dispItem.isNull() && dispItem.count > 0) {
		int32_t id = dispItem.getId();
		int32_t aux = dispItem.getAuxValue();

		glPushMatrix();
		glTranslatef(0.0f, 0.0f, 0.01f);
		glRotatef((float)frame->getRotation() * 45.0f, 0.0f, 0.0f, 1.0f);

		float s = 0.28f;
		if (MobHeadTile::isHeadBlock(id)) {
			int htype = MobHeadTile::getHeadType(id);
			if (MobHeadRenderer::instance) {
				MobHeadRenderer::instance->renderHead(htype, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f);
			}
		} else if (id > 0 && id < 256 && Tile::tiles[id]) {
			Tile* tile = Tile::tiles[id];
			int32_t shape = tile->getRenderShape();
			this->bindTexture("terrain-atlas.tga");
			if (canRenderFrameTile(shape)) {
				float scale = (shape == 0 || shape == 1) ? 0.5f : 0.25f;
				glScalef(scale, scale, scale);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				ItemRenderer::tileRenderer->renderTile(tile, aux, 0);
			} else {
				TextureUVCoordinateSet* uvs = tile->getTexture(2, aux);
				if (uvs) {
					t.begin(4);
					t.colorABGR(0xFFFFFFFF);
					t.vertexUV(-s, -s, 0.0f, uvs->minX, uvs->maxY);
					t.vertexUV(s, -s, 0.0f, uvs->maxX, uvs->maxY);
					t.vertexUV(s, s, 0.0f, uvs->maxX, uvs->minY);
					t.vertexUV(-s, s, 0.0f, uvs->minX, uvs->minY);
					t.draw(1);
				}
			}
		} else if (id >= 256 && id < 512 && Item::items[id]) {
			Item* it = Item::items[id];
			this->bindTexture(it->itemTexture.empty() ? "gui/items.png" : it->itemTexture);
			TextureUVCoordinateSet* uvs = it->getIcon(aux, 0, 0);
			if (uvs) {
				t.begin(4);
				t.colorABGR(0xFFFFFFFF);
				t.vertexUV(-s, -s, 0.0f, uvs->minX, uvs->maxY);
				t.vertexUV(s, -s, 0.0f, uvs->maxX, uvs->maxY);
				t.vertexUV(s, s, 0.0f, uvs->maxX, uvs->minY);
				t.vertexUV(-s, s, 0.0f, uvs->minX, uvs->minY);
				t.draw(1);
			}
		}
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glPopMatrix();
	}

	if (!alphaTestWas) glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);
	if (!blendWas) glDisable(GL_BLEND);
	if (cullWas) glEnable(GL_CULL_FACE);
	if (lightingWas) glEnable(GL_LIGHTING);
	if (!depthTestWas) glDisable(GL_DEPTH_TEST);
	if (colorArrayWas) glEnableClientState(GL_COLOR_ARRAY);
	if (normalArrayWas) glEnableClientState(GL_NORMAL_ARRAY);
	if (!texArrayWas) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();
}
