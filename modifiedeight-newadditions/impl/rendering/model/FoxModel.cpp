#include <rendering/model/FoxModel.hpp>
#include <rendering/model/MobAnimUtils.hpp>
#include <entity/Fox.hpp>
#include <math/Mth.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FoxModel::FoxModel()
	: head(1, 5, 48, 32)
	, earL(8, 1, 48, 32)
	, earR(15, 1, 48, 32)
	, nose(6, 18, 48, 32)
	, body(24, 15, 48, 32)
	, leg0(13, 24, 48, 32)
	, leg1(4, 24, 48, 32)
	, leg2(13, 24, 48, 32)
	, leg3(4, 24, 48, 32)
	, tail(30, 0, 48, 32) {
	this->head.setModel(this);
	this->head.addBox(-3.0f, -2.0f, -5.0f, 8, 6, 6);
	this->head.setPos(-1.0f, 16.5f, -3.0f);

	this->earL.setModel(this);
	this->earL.addBox(-3.0f, -4.0f, -4.0f, 2, 2, 1);
	this->head.addChild(&this->earL);

	this->earR.setModel(this);
	this->earR.addBox(3.0f, -4.0f, -4.0f, 2, 2, 1);
	this->head.addChild(&this->earR);

	this->nose.setModel(this);
	this->nose.addBox(-1.0f, 2.01f, -8.0f, 4, 2, 3);
	this->head.addChild(&this->nose);

	this->body.setModel(this);
	this->body.addBox(-3.0f, 3.999f, -3.5f, 6, 11, 6);
	this->body.setPos(0.0f, 16.0f, -6.0f);
	this->body.xRotAngle = (float)(M_PI / 2.0);

	this->leg0.setModel(this);
	this->leg0.addBox(2.0f, 0.5f, -1.0f, 2, 6, 2);
	this->leg0.setPos(-5.0f, 17.5f, 7.0f);

	this->leg1.setModel(this);
	this->leg1.addBox(2.0f, 0.5f, -1.0f, 2, 6, 2);
	this->leg1.setPos(-1.0f, 17.5f, 7.0f);

	this->leg2.setModel(this);
	this->leg2.addBox(2.0f, 0.5f, -1.0f, 2, 6, 2);
	this->leg2.setPos(-5.0f, 17.5f, 0.0f);

	this->leg3.setModel(this);
	this->leg3.addBox(2.0f, 0.5f, -1.0f, 2, 6, 2);
	this->leg3.setPos(-1.0f, 17.5f, 0.0f);

	this->tail.setModel(this);
	this->tail.addBox(-2.0f, 0.0f, -1.0f, 4, 9, 5);
	this->tail.setPos(0.0f, 14.2f, -1.0f);
	this->tail.xRotAngle = -0.05235988f;
	this->body.addChild(&this->tail);
}

FoxModel::~FoxModel() {
}

void FoxModel::setupAnim(float a2, float a3, float a4, float a5, float a6, float a7) {
	this->head.setPos(-1.0f, 16.5f, -3.0f);
	this->body.setPos(0.0f, 16.0f, -6.0f);
	this->body.xRotAngle = (float)(M_PI / 2.0);
	this->body.yRotAngle = 0.0f;
	this->body.zRotAngle = 0.0f;
	this->leg0.setPos(-5.0f, 17.5f, 7.0f);
	this->leg1.setPos(-1.0f, 17.5f, 7.0f);
	this->leg2.setPos(-5.0f, 17.5f, 0.0f);
	this->leg3.setPos(-1.0f, 17.5f, 0.0f);
	this->leg0.yRotAngle = 0.0f;
	this->leg1.yRotAngle = 0.0f;
	this->leg2.yRotAngle = 0.0f;
	this->leg3.yRotAngle = 0.0f;
	this->leg0.zRotAngle = 0.0f;
	this->leg1.zRotAngle = 0.0f;
	this->leg2.zRotAngle = 0.0f;
	this->leg3.zRotAngle = 0.0f;
	this->tail.setPos(0.0f, 14.2f, -1.0f);
	this->tail.yRotAngle = 0.0f;

	float tailSwish = animWindowHold(a4, 42.0f, 509U, 112U, true) * 0.22f + animBurstWave(a4, 48.0f, 521U, 96U, 2.4f) * 0.20f;
	float tailLift = animWindowHold(a4, 54.0f, 523U, 84U, false) * 0.14f;
	this->head.xRotAngle = a6 * (float)(M_PI / 180.0);
	this->head.yRotAngle = a5 * (float)(M_PI / 180.0);
	this->head.zRotAngle = 0.0f;
	this->earL.xRotAngle = 0.0f;
	this->earL.yRotAngle = 0.0f;
	this->earL.zRotAngle = 0.0f;
	this->earR.xRotAngle = 0.0f;
	this->earR.yRotAngle = 0.0f;
	this->earR.zRotAngle = 0.0f;
	this->nose.xRotAngle = 0.0f;
	this->nose.yRotAngle = 0.0f;
	this->nose.zRotAngle = 0.0f;
	this->body.zRotAngle = 0.0f;
	this->tail.xRotAngle = -0.05235988f + tailLift + (a3 > 0.001f ? Mth::cos(a2 * 0.6662f) * a3 * 0.12f : 0.0f);
	this->tail.zRotAngle = tailSwish + (a3 > 0.001f ? Mth::sin(a2 * 0.6662f) * a3 * 0.18f : 0.0f);
	this->leg0.xRotAngle = Mth::cos(a2 * 0.6662f) * 1.4f * a3;
	this->leg1.xRotAngle = Mth::cos(a2 * 0.6662f + (float)M_PI) * 1.4f * a3;
	this->leg2.xRotAngle = Mth::cos(a2 * 0.6662f + (float)M_PI) * 1.4f * a3;
	this->leg3.xRotAngle = Mth::cos(a2 * 0.6662f) * 1.4f * a3;
}

void FoxModel::render(Entity* a2, float a3, float a4, float a5, float a6, float a7, float a8) {
	this->setupAnim(a3, a4, a5, a6, a7, a8);
	Fox* fox = (a2 && (a2->getEntityTypeId() == 38 || a2->getEntityTypeId() == 46)) ? (Fox*)a2 : 0;
	if (fox) {
		uint32_t seed = (uint32_t)fox->entityId;
		float t = a5 + (float)((seed & 63U) * 17U);
		float headTilt = animWindowHold(t, 44.0f, seed + 13U, 92U, true) * 0.24f;
		float foxGlance = animWindowHold(t, 52.0f, seed + 29U, 88U, true) * 0.14f;
		float sniffPitch = animWindowHold(t, 58.0f, seed + 41U, 74U, false) * 0.12f;
		if (fox->isInterested) {
			float side = ((seed & 1U) != 0) ? 1.0f : -1.0f;
			headTilt = side * 0.34f + Mth::sin(t * 0.35f) * 0.05f;
			sniffPitch = -0.10f;
			this->tail.zRotAngle = Mth::sin(t * 0.55f) * 0.32f;
			this->tail.xRotAngle = 0.18f;
		}
		if (fox->isFoxSleeping) {
			this->body.setPos(0.0f, 19.5f, -5.0f);
			this->body.xRotAngle = (float)(M_PI / 2.0);
			this->body.zRotAngle = 0.0f;
			this->head.setPos(-1.0f, 18.5f, -2.5f);
			this->head.xRotAngle = 0.04f + animWindowHold(t, 56.0f, seed + 19U, 72U, true) * 0.05f;
			this->head.yRotAngle = (a6 * (float)(M_PI / 180.0)) * 0.45f;
			this->head.zRotAngle = 0.06f;
			this->leg0.setPos(-5.0f, 22.8f, 7.5f);
			this->leg0.xRotAngle = -(float)(M_PI / 2.0);
			this->leg0.yRotAngle = 0.12f;
			this->leg1.setPos(-1.0f, 22.8f, 7.5f);
			this->leg1.xRotAngle = -(float)(M_PI / 2.0);
			this->leg1.yRotAngle = -0.12f;
			this->leg2.setPos(-5.0f, 22.8f, -0.5f);
			this->leg2.xRotAngle = -(float)(M_PI / 2.0);
			this->leg3.setPos(-1.0f, 22.8f, -0.5f);
			this->leg3.xRotAngle = -(float)(M_PI / 2.0);
			this->tail.setPos(0.0f, 14.2f, -1.0f);
			this->tail.xRotAngle = 0.05f;
			this->tail.zRotAngle = 0.35f + animBurstWave(t, 54.0f, seed + 59U, 88U, 2.0f) * 0.14f;
		} else if (fox->isFoxSitting) {
			this->body.setPos(0.0f, 12.0f, -6.0f);
			this->body.xRotAngle = (float)(M_PI * 0.30);
			this->head.setPos(-1.0f, 13.5f, -3.2f);
			this->head.zRotAngle = headTilt;
			this->head.xRotAngle += sniffPitch;
			this->head.yRotAngle += foxGlance;
			this->leg0.setPos(-5.0f, 22.4f, 4.5f);
			this->leg0.xRotAngle = -1.52f;
			this->leg0.yRotAngle = 0.15f;
			this->leg1.setPos(-1.0f, 22.4f, 4.5f);
			this->leg1.xRotAngle = -1.52f;
			this->leg1.yRotAngle = -0.15f;
			this->leg2.setPos(-5.0f, 17.5f, -2.5f);
			this->leg2.xRotAngle = -0.15f;
			this->leg3.setPos(-1.0f, 17.5f, -2.5f);
			this->leg3.xRotAngle = -0.15f;
			this->tail.setPos(0.0f, 14.2f, -1.0f);
			this->tail.xRotAngle = 0.72f;
			this->tail.zRotAngle = animBurstWave(t, 40.0f, seed + 71U, 140U, 2.8f) * 0.28f;
		} else if (fox->isInWater()) {
			float swimSpeed = (a4 > 1.0f) ? 1.0f : a4;
			float paddlePhase = a5 * 0.48f + a3 * 0.85f;
			float paddleAmp = 0.42f + swimSpeed * 0.48f;
			float roll = Mth::sin(paddlePhase) * 0.04f;
			this->head.setPos(-1.0f, 15.6f, -3.0f);
			this->head.xRotAngle -= 0.16f;
			this->head.zRotAngle = -roll * 0.35f;
			this->body.zRotAngle = 0.0f;
			this->leg2.xRotAngle = -0.35f + Mth::cos(paddlePhase) * paddleAmp;
			this->leg3.xRotAngle = -0.35f + Mth::cos(paddlePhase + (float)M_PI) * paddleAmp;
			this->leg0.xRotAngle = 0.32f + Mth::cos(paddlePhase + (float)M_PI) * (paddleAmp * 0.85f);
			this->leg1.xRotAngle = 0.32f + Mth::cos(paddlePhase) * (paddleAmp * 0.85f);
			this->tail.xRotAngle = 0.12f;
			this->tail.zRotAngle = Mth::cos(a5 * 0.38f) * 0.22f;
		} else {
			float walkDamp = 1.0f - Mth::clamp(a4 * 0.35f, 0.0f, 0.45f);
			this->head.zRotAngle = headTilt * walkDamp;
			this->head.xRotAngle += sniffPitch * walkDamp;
			this->head.yRotAngle += foxGlance * walkDamp;
			if (fox->isFoxCrouching) {
				this->body.setPos(0.0f, 17.2f, -6.0f);
				this->head.setPos(-1.0f, 17.8f, -3.5f);
				this->leg0.setPos(-5.0f, 17.5f, 7.0f);
				this->leg0.xRotAngle *= 0.6f;
				this->leg1.setPos(-1.0f, 17.5f, 7.0f);
				this->leg1.xRotAngle *= 0.6f;
				this->leg2.setPos(-5.0f, 17.5f, 0.0f);
				this->leg2.xRotAngle *= 0.6f;
				this->leg3.setPos(-1.0f, 17.5f, 0.0f);
				this->leg3.xRotAngle *= 0.6f;
			}
		}
	}
	if (this->field_14) {
		float babyHeadY = (fox && fox->isFoxSleeping) ? 6.2f : 8.0f;
		glPushMatrix();
		glTranslatef(0.0f, babyHeadY * a8, 1.0f * a8);
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
