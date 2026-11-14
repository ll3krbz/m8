#include <rendering/model/OcelotModel.hpp>
#include <rendering/model/MobAnimUtils.hpp>
#include <entity/Ocelot.hpp>
#include <math/Mth.hpp>
#include <cmath>

OcelotModel::OcelotModel()
	: head(0, 0, 64, 32)
	, body(20, 0, 64, 32)
	, leg0(8, 13, 64, 32)
	, leg1(8, 13, 64, 32)
	, leg2(40, 0, 64, 32)
	, leg3(40, 0, 64, 32)
	, tail1(0, 15, 64, 32)
	, tail2(4, 15, 64, 32) {
	this->head.setModel(this);
	this->head.addBox(-2.5f, -2.0f, -3.0f, 5, 4, 5);
	this->head.addBox(-1.5f, 0.0f, -4.0f, 3, 2, 2);
	this->head.addBox(-2.0f, -3.0f, 0.0f, 1, 1, 2);
	this->head.addBox(1.0f, -3.0f, 0.0f, 1, 1, 2);
	this->head.setPos(0.0f, 15.0f, -9.0f);

	this->body.setModel(this);
	this->body.addBox(-2.0f, 3.0f, -8.0f, 4, 16, 6);
	this->body.setPos(0.0f, 12.0f, -10.0f);

	this->tail1.setModel(this);
	this->tail1.addBox(-0.5f, 0.0f, 0.0f, 1, 8, 1);
	this->tail1.setPos(0.0f, 15.0f, 8.0f);

	this->tail2.setModel(this);
	this->tail2.addBox(-0.5f, 0.0f, 0.0f, 1, 8, 1);
	this->tail2.setPos(0.0f, 20.0f, 14.0f);

	this->leg0.setModel(this);
	this->leg0.addBox(-1.0f, 0.0f, 1.0f, 2, 6, 2);
	this->leg0.setPos(0.9f, 18.0f, 5.0f);

	this->leg1.setModel(this);
	this->leg1.addBox(-1.0f, 0.0f, 1.0f, 2, 6, 2);
	this->leg1.setPos(-0.9f, 18.0f, 5.0f);

	this->leg2.setModel(this);
	this->leg2.addBox(-1.0f, 0.0f, 0.0f, 2, 9, 2);
	this->leg2.setPos(0.9f, 15.0f, -5.0f);

	this->leg3.setModel(this);
	this->leg3.addBox(-1.0f, 0.0f, 0.0f, 2, 9, 2);
	this->leg3.setPos(-0.9f, 15.0f, -5.0f);
}

OcelotModel::~OcelotModel() {
}

void OcelotModel::render(Entity* entity, float a3, float a4, float a5, float a6, float a7, float a8) {
	this->setupAnim(a3, a4, a5, a6, a7, a8);
	Ocelot* ocelot = (entity && entity->getEntityTypeId() == 22) ? (Ocelot*)entity : 0;
	if (ocelot) {
		uint32_t seed = (uint32_t)ocelot->entityId;
		float t = a5 + (float)((seed & 63U) * 19U);
		if (ocelot->isInterested) {
			float side = ((seed & 1U) != 0) ? 1.0f : -1.0f;
			this->head.zRotAngle = side * 0.32f + Mth::sin(t * 0.34f) * 0.05f;
			this->head.xRotAngle -= 0.10f;
			this->tail1.yRotAngle = Mth::sin(t * 0.52f) * 0.28f;
			this->tail2.yRotAngle = Mth::sin(t * 0.52f - 0.5f) * 0.42f;
		}
		if (ocelot->isSitting) {
			this->head.setPos(0.0f, 11.7f, -8.0f);
			this->body.xRotAngle = 0.7853982f;
			this->body.setPos(0.0f, 8.0f, -5.0f);
			this->tail1.xRotAngle = 1.7278761f;
			this->tail1.setPos(0.0f, 22.0f, 6.0f);
			this->tail2.xRotAngle = 2.45f;
			this->leg2.xRotAngle = -0.15707964f;
			this->leg2.setPos(0.9f, 15.2f, -7.0f);
			this->leg3.xRotAngle = -0.15707964f;
			this->leg3.setPos(-0.9f, 15.2f, -7.0f);
			this->leg0.xRotAngle = -1.5707964f;
			this->leg0.setPos(0.9f, 21.0f, 1.0f);
			this->leg1.xRotAngle = -1.5707964f;
			this->leg1.setPos(-0.9f, 21.0f, 1.0f);
		} else if (ocelot->isInWater()) {
			float swimSpeed = (a4 > 1.0f) ? 1.0f : a4;
			float paddlePhase = a5 * 0.48f + a3 * 0.85f;
			float paddleAmp = 0.38f + swimSpeed * 0.42f;
			this->head.setPos(0.0f, 14.2f, -9.0f);
			this->head.xRotAngle -= 0.16f;
			this->head.zRotAngle = 0.0f;
			this->leg2.xRotAngle = -0.32f + Mth::cos(paddlePhase) * paddleAmp;
			this->leg3.xRotAngle = -0.32f + Mth::cos(paddlePhase + 3.14159f) * paddleAmp;
			this->leg0.xRotAngle = 0.30f + Mth::cos(paddlePhase + 3.14159f) * (paddleAmp * 0.85f);
			this->leg1.xRotAngle = 0.30f + Mth::cos(paddlePhase) * (paddleAmp * 0.85f);
			this->tail1.xRotAngle = 1.35f;
			this->tail2.xRotAngle = 1.48f;
			this->tail1.yRotAngle = Mth::cos(a5 * 0.36f) * 0.18f;
			this->tail2.yRotAngle = Mth::cos(a5 * 0.36f - 0.5f) * 0.28f;
		}
	}
	float t1Len = 7.5f;
	this->tail2.setPos(
		this->tail1.xOffset + t1Len * Mth::sin(this->tail1.xRotAngle) * Mth::sin(this->tail1.yRotAngle),
		this->tail1.yOffset + t1Len * Mth::cos(this->tail1.xRotAngle),
		this->tail1.zOffset + t1Len * Mth::sin(this->tail1.xRotAngle) * Mth::cos(this->tail1.yRotAngle)
	);
	if (this->field_14) {
		glPushMatrix();
		glTranslatef(0.0f, 8.0f * a8, 2.5f * a8);
		glScalef(0.75f, 0.75f, 0.75f);
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
		this->tail1.render(a8);
		this->tail2.render(a8);
		glPopMatrix();
		return;
	}
	this->head.render(a8);
	this->body.render(a8);
	this->leg0.render(a8);
	this->leg1.render(a8);
	this->leg2.render(a8);
	this->leg3.render(a8);
	this->tail1.render(a8);
	this->tail2.render(a8);
}

void OcelotModel::setupAnim(float a2, float a3, float a4, float a5, float a6, float a7) {
	this->head.setPos(0.0f, 15.0f, -9.0f);
	this->body.setPos(0.0f, 12.0f, -10.0f);
	this->tail1.setPos(0.0f, 15.0f, 8.0f);
	this->tail2.setPos(0.0f, 20.0f, 14.0f);
	this->leg0.setPos(0.9f, 18.0f, 5.0f);
	this->leg1.setPos(-0.9f, 18.0f, 5.0f);
	this->leg2.setPos(0.9f, 15.0f, -5.0f);
	this->leg3.setPos(-0.9f, 15.0f, -5.0f);
	this->leg0.yRotAngle = 0.0f;
	this->leg1.yRotAngle = 0.0f;
	this->leg2.yRotAngle = 0.0f;
	this->leg3.yRotAngle = 0.0f;
	this->leg0.zRotAngle = 0.0f;
	this->leg1.zRotAngle = 0.0f;
	this->leg2.zRotAngle = 0.0f;
	this->leg3.zRotAngle = 0.0f;

	this->head.xRotAngle = a6 / 57.29578f;
	this->head.yRotAngle = a5 / 57.29578f;

	float headTilt = animWindowHold(a4, 42.0f, 221U, 90U, true) * 0.24f;
	float catGlance = animWindowHold(a4, 48.0f, 223U, 88U, true) * 0.16f;
	float catPitch = animWindowHold(a4, 54.0f, 227U, 76U, true) * 0.12f;
	float tailSwish1 = animWindowHold(a4, 46.0f, 229U, 108U, true) * 0.22f + animBurstWave(a4, 52.0f, 233U, 84U, 2.0f) * 0.18f;
	float tailSwish2 = animWindowHold(a4, 38.0f, 239U, 124U, true) * 0.34f + animBurstWave(a4, 44.0f, 241U, 102U, 2.6f) * 0.28f;
	float tailPerk = animWindowHold(a4, 50.0f, 251U, 86U, false) * 0.14f;

	this->head.zRotAngle = headTilt;
	this->head.yRotAngle += catGlance;
	this->head.xRotAngle += catPitch;

	this->body.xRotAngle = 1.5707964f;
	this->body.yRotAngle = 0.0f;
	this->body.zRotAngle = 0.0f;

	this->leg0.xRotAngle = Mth::cos(a2 * 0.6662f) * 0.8f * a3;
	this->leg1.xRotAngle = Mth::cos(a2 * 0.6662f + 3.14159f) * 0.8f * a3;
	this->leg2.xRotAngle = Mth::cos(a2 * 0.6662f + 3.14159f) * 0.8f * a3;
	this->leg3.xRotAngle = Mth::cos(a2 * 0.6662f) * 0.8f * a3;

	this->tail1.xRotAngle = 0.9f + tailPerk;
	this->tail1.yRotAngle = tailSwish1;
	this->tail2.xRotAngle = 1.1f + tailPerk * 1.2f + Mth::cos(a2 * 0.6662f) * 0.35f * a3;
	this->tail2.yRotAngle = tailSwish2;
}
