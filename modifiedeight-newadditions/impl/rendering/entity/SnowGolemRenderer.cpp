#include <rendering/entity/SnowGolemRenderer.hpp>
#include <entity/Snowman.hpp>
#include <item/ItemInstance.hpp>
#include <rendering/EntityRenderDispatcher.hpp>
#include <rendering/ItemInHandRenderer.hpp>
#include <rendering/Tesselator.hpp>
#include <rendering/TextureUVCoordinateSet.hpp>
#include <rendering/model/SnowGolemModel.hpp>
#include <tile/Tile.hpp>
#include <unigl.h>

SnowGolemRenderer::SnowGolemRenderer(SnowGolemModel* model, float shadow)
	: MobRenderer(model, shadow) {
	this->snowmanModel = model;
}

SnowGolemRenderer::~SnowGolemRenderer() {
}

void SnowGolemRenderer::additionalRendering(Mob* mob, float pt) {
	MobRenderer::additionalRendering(mob, pt);
	if (!mob) return;
	Snowman* snowman = (Snowman*)mob;
	if (snowman->hasPumpkin()) {
		glPushMatrix();
		this->snowmanModel->head.translateTo(0.0625f);
		glTranslatef(0.0f, -0.34375f, 0.0f);
		glScalef(0.625f, -0.625f, -0.625f);
		this->bindTexture("terrain-atlas.tga");
		glDisable(GL_CULL_FACE);
		Tesselator& t = Tesselator::instance;
		t.begin(24);
		t.color(1.0f, 1.0f, 1.0f, 1.0f);
		TextureUVCoordinateSet* uv0 = Tile::pumpkin->getTexture(0, 0);
		if (uv0) {
			t.vertexUV(-0.5f, -0.5f, -0.5f, uv0->minX, uv0->minY);
			t.vertexUV(0.5f, -0.5f, -0.5f, uv0->maxX, uv0->minY);
			t.vertexUV(0.5f, -0.5f, 0.5f, uv0->maxX, uv0->maxY);
			t.vertexUV(-0.5f, -0.5f, 0.5f, uv0->minX, uv0->maxY);
		}
		TextureUVCoordinateSet* uv1 = Tile::pumpkin->getTexture(1, 0);
		if (uv1) {
			t.vertexUV(-0.5f, 0.5f, 0.5f, uv1->minX, uv1->minY);
			t.vertexUV(0.5f, 0.5f, 0.5f, uv1->maxX, uv1->minY);
			t.vertexUV(0.5f, 0.5f, -0.5f, uv1->maxX, uv1->maxY);
			t.vertexUV(-0.5f, 0.5f, -0.5f, uv1->minX, uv1->maxY);
		}
		TextureUVCoordinateSet* uv2 = Tile::pumpkin->getTexture(2, 0);
		if (uv2) {
			t.vertexUV(0.5f, 0.5f, -0.5f, uv2->minX, uv2->minY);
			t.vertexUV(-0.5f, 0.5f, -0.5f, uv2->maxX, uv2->minY);
			t.vertexUV(-0.5f, -0.5f, -0.5f, uv2->maxX, uv2->maxY);
			t.vertexUV(0.5f, -0.5f, -0.5f, uv2->minX, uv2->maxY);
		}
		TextureUVCoordinateSet* uv3 = Tile::pumpkin->getTexture(3, 0);
		if (uv3) {
			t.vertexUV(-0.5f, 0.5f, 0.5f, uv3->minX, uv3->minY);
			t.vertexUV(0.5f, 0.5f, 0.5f, uv3->maxX, uv3->minY);
			t.vertexUV(0.5f, -0.5f, 0.5f, uv3->maxX, uv3->maxY);
			t.vertexUV(-0.5f, -0.5f, 0.5f, uv3->minX, uv3->maxY);
		}
		TextureUVCoordinateSet* uv4 = Tile::pumpkin->getTexture(4, 0);
		if (uv4) {
			t.vertexUV(-0.5f, 0.5f, -0.5f, uv4->minX, uv4->minY);
			t.vertexUV(-0.5f, 0.5f, 0.5f, uv4->maxX, uv4->minY);
			t.vertexUV(-0.5f, -0.5f, 0.5f, uv4->maxX, uv4->maxY);
			t.vertexUV(-0.5f, -0.5f, -0.5f, uv4->minX, uv4->maxY);
		}
		TextureUVCoordinateSet* uv5 = Tile::pumpkin->getTexture(5, 0);
		if (uv5) {
			t.vertexUV(0.5f, 0.5f, 0.5f, uv5->minX, uv5->minY);
			t.vertexUV(0.5f, 0.5f, -0.5f, uv5->maxX, uv5->minY);
			t.vertexUV(0.5f, -0.5f, -0.5f, uv5->maxX, uv5->maxY);
			t.vertexUV(0.5f, -0.5f, 0.5f, uv5->minX, uv5->maxY);
		}
		t.draw(1);
		glEnable(GL_CULL_FACE);
		glPopMatrix();
	}
}
