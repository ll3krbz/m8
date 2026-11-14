#include <rendering/model/PolarBearModel.hpp>
#include <rendering/model/MobAnimUtils.hpp>
#include <entity/PolarBear.hpp>
#include <math/Mth.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PolarBearModel::PolarBearModel()
	: head(0, 0, 128, 64)
	, body(0, 19, 128, 64)
	, leg0(50, 22, 128, 64)
	, leg1(50, 22, 128, 64)
	, leg2(50, 40, 128, 64)
	, leg3(50, 40, 128, 64) {
	this->head.setModel(this);
	this->head.addBox(-3.5f, -3.0f, -3.0f, 7, 7, 7);
	this->head.setPos(0.0f, 10.0f, -16.0f);

	this->head.texOffsetX = 0;
	this->head.texOffsetY = 44;
	this->head.addBox(-2.5f, 1.0f, -6.0f, 5, 3, 3);

	this->head.texOffsetX = 26;
	this->head.texOffsetY = 0;
	this->head.addBox(-4.5f, -4.0f, -1.0f, 2, 2, 1);
	this->head.addBox(2.5f, -4.0f, -1.0f, 2, 2, 1);

	this->body.setModel(this);
	this->body.texOffsetX = 0;
	this->body.texOffsetY = 19;
	this->body.addBox(-5.0f, -13.0f, -7.0f, 14, 14, 11);

	this->body.texOffsetX = 39;
	this->body.texOffsetY = 0;
	this->body.addBox(-4.0f, -25.0f, -7.0f, 12, 12, 10);
	this->body.setPos(-2.0f, 9.0f, 12.0f);
	this->body.xRotAngle = (float)(M_PI / 2.0);

	this->leg0.setModel(this);
	this->leg0.addBox(-2.0f, 0.0f, -2.0f, 4, 10, 8);
	this->leg0.setPos(-4.5f, 14.0f, 6.0f);

	this->leg1.setModel(this);
	this->leg1.addBox(-2.0f, 0.0f, -2.0f, 4, 10, 8);
	this->leg1.setPos(4.5f, 14.0f, 6.0f);

	this->leg2.setModel(this);
	this->leg2.addBox(-2.0f, 0.0f, -2.0f, 4, 10, 6);
	this->leg2.setPos(-3.5f, 14.0f, -8.0f);

	this->leg3.setModel(this);
	this->leg3.addBox(-2.0f, 0.0f, -2.0f, 4, 10, 6);
	this->leg3.setPos(3.5f, 14.0f, -8.0f);
}

PolarBearModel::~PolarBearModel() {
}

void PolarBearModel::setupAnim(float a2, float a3, float a4, float a5, float a6, float a7) {
	float scentLift = -animWindowHold(a4, 60.0f, 261U, 76U, false) * 0.22f;
	float scentSweep = animBurstWave(a4, 60.0f, 263U, 72U, 1.6f) * 0.16f + animWindowHold(a4, 52.0f, 269U, 84U, true) * 0.14f;
	float headTilt = animWindowHold(a4, 48.0f, 271U, 80U, true) * 0.18f;
	float bodyShift = animWindowHold(a4, 68.0f, 277U, 68U, true) * 0.035f;
	this->head.xRotAngle = a6 * (float)(M_PI / 180.0) + scentLift + (a3 > 0.001f ? Mth::cos(a2 * 0.6662f * 2.0f) * a3 * 0.06f : 0.0f);
	this->head.yRotAngle = a5 * (float)(M_PI / 180.0) + scentSweep;
	this->head.zRotAngle = headTilt - (a3 > 0.001f ? Mth::sin(a2 * 0.6662f) * a3 * 0.04f : 0.0f);
	this->body.zRotAngle = bodyShift + (a3 > 0.001f ? Mth::sin(a2 * 0.6662f) * a3 * 0.055f : 0.0f);
	this->leg0.xRotAngle = Mth::cos(a2 * 0.6662f) * 1.4f * a3;
	this->leg1.xRotAngle = Mth::cos(a2 * 0.6662f + (float)M_PI) * 1.4f * a3;
	this->leg2.xRotAngle = Mth::cos(a2 * 0.6662f + (float)M_PI) * 1.4f * a3;
	this->leg3.xRotAngle = Mth::cos(a2 * 0.6662f) * 1.4f * a3;
}

void PolarBearModel::render(Entity* a2, float a3, float a4, float a5, float a6, float a7, float a8) {
	this->setupAnim(a3, a4, a5, a6, a7, a8);
	PolarBear* bear = (a2 && a2->getEntityTypeId() == 26) ? (PolarBear*)a2 : 0;
	if (bear && bear->isInterested) {
		uint32_t seed = (uint32_t)bear->entityId;
		float t = a5 + (float)((seed & 31U) * 19U);
		float side = ((seed & 1U) != 0) ? 1.0f : -1.0f;
		this->head.zRotAngle = side * 0.24f + Mth::sin(t * 0.28f) * 0.05f;
		this->head.xRotAngle -= 0.14f + Mth::sin(t * 0.42f) * 0.04f;
		this->body.zRotAngle = Mth::sin(t * 0.22f) * 0.04f;
	}
	if (this->field_14) {
		glPushMatrix();
		glTranslatef(0.0f, 11.5f * a8, 3.0f * a8);
		glScalef(0.65f, 0.65f, 0.65f);
		this->head.render(a8);
		glPopMatrix();
		glPushMatrix();
		glScalef(0.5f, 0.5f, 0.5f);
		glTranslatef(0.0f, 24.0f * a8, 0.0f);
		this->body.render(a8);
		this->leg0.render(a8);
		this->leg1.render(a8);
		this->leg2.render(a8);
		this->leg3.render(a8);
		glPopMatrix();
		return;
	}
	this->head.render(a8);
	this->body.render(a8);
	this->leg0.render(a8);
	this->leg1.render(a8);
	this->leg2.render(a8);
	this->leg3.render(a8);
}
