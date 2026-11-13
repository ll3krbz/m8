#include <rendering/model/SnowGolemModel.hpp>
#include <math/Mth.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SnowGolemModel::SnowGolemModel()
	: piece1(0, 16, 64, 64)
	, piece2(0, 36, 64, 64)
	, head(0, 0, 64, 64)
	, arm1(32, 0, 64, 64)
	, arm2(32, 0, 64, 64) {
	this->head.setModel(this);
	this->head.addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, -0.5f);
	this->head.setPos(0.0f, 4.0f, 0.0f);

	this->arm1.setModel(this);
	this->arm1.addBox(-1.0f, 0.0f, -1.0f, 12, 2, 2, -0.5f);
	this->arm1.setPos(0.0f, 6.0f, 0.0f);

	this->arm2.setModel(this);
	this->arm2.addBox(-1.0f, 0.0f, -1.0f, 12, 2, 2, -0.5f);
	this->arm2.setPos(0.0f, 6.0f, 0.0f);

	this->piece1.setModel(this);
	this->piece1.addBox(-5.0f, -10.0f, -5.0f, 10, 10, 10, -0.5f);
	this->piece1.setPos(0.0f, 13.0f, 0.0f);

	this->piece2.setModel(this);
	this->piece2.addBox(-6.0f, -12.0f, -6.0f, 12, 12, 12, -0.5f);
	this->piece2.setPos(0.0f, 24.0f, 0.0f);
}

SnowGolemModel::~SnowGolemModel() {
}

void SnowGolemModel::setupAnim(float a2, float a3, float a4, float a5, float a6, float a7) {
	this->head.yRotAngle = a5 * (float)(M_PI / 180.0);
	this->head.xRotAngle = a6 * (float)(M_PI / 180.0);
	this->piece1.yRotAngle = a5 * (float)(M_PI / 180.0) * 0.25f;

	float l = Mth::sin(this->piece1.yRotAngle);
	float m = Mth::cos(this->piece1.yRotAngle);

	this->arm1.zRotAngle = 1.0f;
	this->arm2.zRotAngle = -1.0f;
	this->arm1.yRotAngle = this->piece1.yRotAngle;
	this->arm2.yRotAngle = (float)M_PI + this->piece1.yRotAngle;

	this->arm1.xOffset = m * 5.0f;
	this->arm1.zOffset = -l * 5.0f;
	this->arm2.xOffset = -m * 5.0f;
	this->arm2.zOffset = l * 5.0f;
}

void SnowGolemModel::render(Entity* a2, float a3, float a4, float a5, float a6, float a7, float a8) {
	this->setupAnim(a3, a4, a5, a6, a7, a8);
	this->piece1.render(a8);
	this->piece2.render(a8);
	this->head.render(a8);
	this->arm1.render(a8);
	this->arm2.render(a8);
}
