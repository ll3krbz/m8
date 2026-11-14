#include <rendering/model/WolfModel.hpp>
#include <rendering/model/MobAnimUtils.hpp>
#include <entity/Wolf.hpp>
#include <math/Mth.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

WolfModel::WolfModel()
	: head(0, 0, 64, 32)
	, body(18, 14, 64, 32)
	, leg0(0, 18, 64, 32)
	, leg1(0, 18, 64, 32)
	, leg2(0, 18, 64, 32)
	, leg3(0, 18, 64, 32)
	, tail(9, 18, 64, 32)
	, mane(21, 0, 64, 32) {
	this->head.setModel(this);
	this->head.addBox(-2.0f, -3.0f, -2.0f, 6, 6, 4);
	this->head.setPos(-1.0f, 13.5f, -7.0f);

	this->head.texOffsetX = 16;
	this->head.texOffsetY = 14;
	this->head.addBox(-2.0f, -5.0f, 0.0f, 2, 2, 1);
	this->head.addBox(2.0f, -5.0f, 0.0f, 2, 2, 1);

	this->head.texOffsetX = 0;
	this->head.texOffsetY = 10;
	this->head.addBox(-0.5f, 0.0f, -5.0f, 3, 3, 4);

	this->body.setModel(this);
	this->body.addBox(-3.0f, -2.0f, -3.0f, 6, 9, 6);
	this->body.setPos(0.0f, 14.0f, 2.0f);
	this->body.xRotAngle = (float)(M_PI / 2.0);

	this->mane.setModel(this);
	this->mane.addBox(-3.0f, -3.0f, -3.0f, 8, 6, 7);
	this->mane.setPos(-1.0f, 14.0f, -3.0f);
	this->mane.xRotAngle = (float)(M_PI / 2.0);

	this->leg0.setModel(this);
	this->leg0.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2);
	this->leg0.setPos(-1.5f, 16.0f, 7.0f);

	this->leg1.setModel(this);
	this->leg1.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2);
	this->leg1.setPos(1.5f, 16.0f, 7.0f);

	this->leg2.setModel(this);
	this->leg2.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2);
	this->leg2.setPos(-1.5f, 16.0f, -4.0f);

	this->leg3.setModel(this);
	this->leg3.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2);
	this->leg3.setPos(1.5f, 16.0f, -4.0f);

	this->tail.setModel(this);
	this->tail.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2);
	this->tail.setPos(0.0f, 12.0f, 8.0f);
	this->tail.xRotAngle = (float)(M_PI / 3.0);
}

WolfModel::~WolfModel() {
}

void WolfModel::setupAnim(float a2, float a3, float a4, float a5, float a6, float a7) {
	this->head.setPos(-1.0f, 13.5f, -7.0f);
	this->body.setPos(0.0f, 14.0f, 2.0f);
	this->body.xRotAngle = (float)(M_PI / 2.0);
	this->mane.setPos(-1.0f, 14.0f, -3.0f);
	this->mane.xRotAngle = (float)(M_PI / 2.0);
	this->leg0.setPos(-1.5f, 16.0f, 7.0f);
	this->leg1.setPos(1.5f, 16.0f, 7.0f);
	this->leg2.setPos(-1.5f, 16.0f, -4.0f);
	this->leg3.setPos(1.5f, 16.0f, -4.0f);
	this->tail.setPos(0.0f, 12.0f, 8.0f);
	this->tail.xRotAngle = (float)(M_PI / 3.0);

	this->head.xRotAngle = a6 * (float)(M_PI / 180.0);
	this->head.yRotAngle = a5 * (float)(M_PI / 180.0);
	this->leg0.xRotAngle = Mth::cos(a2 * 0.6662f) * 1.4f * a3;
	this->leg1.xRotAngle = Mth::cos(a2 * 0.6662f + (float)M_PI) * 1.4f * a3;
	this->leg2.xRotAngle = Mth::cos(a2 * 0.6662f + (float)M_PI) * 1.4f * a3;
	this->leg3.xRotAngle = Mth::cos(a2 * 0.6662f) * 1.4f * a3;

	this->tail.yRotAngle = animBurstWave(a4, 42.0f, 141U, 128U, 2.8f) * 0.28f;
	if (a3 > 0.001f) {
		float bodySway = Mth::sin(a2 * 0.6662f) * a3 * 0.06f;
		this->body.zRotAngle = bodySway;
		this->mane.zRotAngle = bodySway;
		this->head.zRotAngle = -bodySway * 0.5f;
	} else {
		this->body.zRotAngle = 0.0f;
		this->mane.zRotAngle = 0.0f;
		this->head.zRotAngle = 0.0f;
	}
}

void WolfModel::render(Entity* a2, float a3, float a4, float a5, float a6, float a7, float a8) {
	this->setupAnim(a3, a4, a5, a6, a7, a8);
	Wolf* wolf = (a2 && a2->getEntityTypeId() == 14) ? (Wolf*)a2 : 0;
	uint32_t seed = wolf ? (uint32_t)wolf->entityId : 14U;
	float t = a5 + (float)((seed & 63U) * 19U);
	float headTilt = animWindowHold(t, 44.0f, seed + 11U, 92U, true) * 0.28f;
	float glanceYaw = animWindowHold(t, 52.0f, seed + 23U, 88U, true) * 0.16f;
	float sniffPitch = animWindowHold(t, 58.0f, seed + 37U, 72U, false) * 0.14f + animBurstWave(t, 58.0f, seed + 41U, 64U, 2.4f) * 0.045f;
	float earPerkPitch = animWindowHold(t, 48.0f, seed + 53U, 76U, true) * 0.08f;
	float maneRuffle = animBurstWave(t, 76.0f, seed + 67U, 44U, 3.0f) * 0.085f;
	float wagBurst = animBurstWave(t, 38.0f, seed + 79U, 148U, 3.2f) * 0.34f + animBurstWave(t, 64.0f, seed + 89U, 96U, 2.0f) * 0.18f;
	float tailLift = animWindowHold(t, 50.0f, seed + 97U, 104U, false) * 0.14f;
	if (wolf && wolf->isInterested) {
		float side = ((seed & 1U) != 0) ? 1.0f : -1.0f;
		headTilt = side * 0.36f + Mth::sin(t * 0.32f) * 0.05f;
		sniffPitch = -0.10f;
		wagBurst = Mth::cos(t * 0.58f) * 0.38f;
		tailLift = 0.20f;
	}
	if (wolf && wolf->isSitting) {
		this->mane.setPos(-1.0f, 16.0f, -3.0f);
		this->mane.xRotAngle = (float)(M_PI * 0.4) + sniffPitch * 0.22f;
		this->body.setPos(0.0f, 18.0f, 0.0f);
		this->body.xRotAngle = (float)(M_PI / 4.0);
		this->tail.setPos(0.0f, 21.0f, 6.0f);
		this->tail.xRotAngle = 1.68f - tailLift * 0.45f;
		this->leg0.setPos(-1.48f, 22.0f, 2.0f);
		this->leg0.xRotAngle = (float)(M_PI * 1.5);
		this->leg1.setPos(1.48f, 22.0f, 2.0f);
		this->leg1.xRotAngle = (float)(M_PI * 1.5);
		this->leg2.setPos(-1.50f, 17.0f, -4.0f);
		this->leg2.xRotAngle = 5.8f;
		this->leg3.setPos(1.50f, 17.0f, -4.0f);
		this->leg3.xRotAngle = 5.8f;
		this->head.zRotAngle = headTilt;
		this->head.xRotAngle += sniffPitch + earPerkPitch;
		this->head.yRotAngle += glanceYaw;
		this->mane.zRotAngle = headTilt * 0.28f + maneRuffle;
		this->body.zRotAngle = headTilt * 0.08f + maneRuffle * 0.35f;
		this->tail.yRotAngle = wagBurst;
	} else if (wolf && !wolf->isInWater()) {
		float walkDamp = 1.0f - Mth::clamp(a4 * 0.35f, 0.0f, 0.45f);
		this->head.zRotAngle += headTilt * walkDamp;
		this->head.xRotAngle += (sniffPitch + earPerkPitch) * walkDamp;
		this->head.yRotAngle += glanceYaw * walkDamp;
		this->mane.zRotAngle += headTilt * 0.24f * walkDamp + maneRuffle;
		this->mane.xRotAngle += sniffPitch * 0.18f * walkDamp;
		if (wolf->isTame) {
			this->tail.xRotAngle = 0.75f + 0.045f * (float)Mth::clamp(wolf->health, 1, 20) + tailLift;
			this->tail.yRotAngle = wagBurst + (a4 > 0.02f ? Mth::cos(a3 * 0.6662f) * a4 * 0.18f : 0.0f);
		} else {
			this->tail.xRotAngle += tailLift * 0.6f;
			this->tail.yRotAngle = wagBurst * 0.65f + (a4 > 0.02f ? Mth::cos(a3 * 0.6662f) * a4 * 0.14f : 0.0f);
		}
	}
	if (wolf && !wolf->isSitting && wolf->isInWater()) {
		float swimSpeed = (a4 > 1.0f) ? 1.0f : a4;
		float paddlePhase = a5 * 0.48f + a3 * 0.85f;
		float paddleAmp = 0.42f + swimSpeed * 0.48f;
		float roll = Mth::sin(paddlePhase) * 0.05f;
		this->head.setPos(-1.0f, 12.6f, -7.0f);
		this->head.xRotAngle -= 0.18f;
		this->head.zRotAngle = -roll * 0.4f;
		this->body.zRotAngle = roll;
		this->mane.zRotAngle = roll;
		this->leg2.xRotAngle = -0.38f + Mth::cos(paddlePhase) * paddleAmp;
		this->leg3.xRotAngle = -0.38f + Mth::cos(paddlePhase + (float)M_PI) * paddleAmp;
		this->leg0.xRotAngle = 0.35f + Mth::cos(paddlePhase + (float)M_PI) * (paddleAmp * 0.85f);
		this->leg1.xRotAngle = 0.35f + Mth::cos(paddlePhase) * (paddleAmp * 0.85f);
		this->tail.xRotAngle = 1.46f;
		this->tail.yRotAngle = Mth::cos(a5 * 0.38f) * 0.25f;
	}
	if (this->field_14) {
		glPushMatrix();
		glTranslatef(0.0f, 5.0f * a8, 2.0f * a8);
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
		this->tail.render(a8);
		this->mane.render(a8);
		glPopMatrix();
		return;
	}
	this->head.render(a8);
	this->body.render(a8);
	this->leg0.render(a8);
	this->leg1.render(a8);
	this->leg2.render(a8);
	this->leg3.render(a8);
	this->tail.render(a8);
	this->mane.render(a8);
}
