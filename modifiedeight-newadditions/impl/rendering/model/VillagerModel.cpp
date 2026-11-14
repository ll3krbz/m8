#include <rendering/model/VillagerModel.hpp>
#include <rendering/model/MobAnimUtils.hpp>
#include <math/Mth.hpp>
#include <cmath>

VillagerModel::VillagerModel(float f)
	: head(0, 0, 64, 64)
	, hat(32, 0, 64, 64)
	, hatRim(30, 47, 64, 64)
	, body(16, 20, 64, 64)
	, jacket(0, 38, 64, 64)
	, arms(40, 38, 64, 64)
	, leg0(0, 22, 64, 64)
	, leg1(0, 22, 64, 64)
	, nose(24, 0, 64, 64) {
	this->head.setPos(0.0f, 0.0f, 0.0f);
	this->head.addBox(-4.0f, -10.0f, -4.0f, 8, 10, 8, f);

	this->nose.setPos(0.0f, -2.0f, 0.0f);
	this->nose.addBox(-1.0f, -1.0f, -6.0f, 2, 4, 2, f);

	this->body.setPos(0.0f, 0.0f, 0.0f);
	this->body.addBox(-4.0f, 0.0f, -3.0f, 8, 12, 6, f);

	this->jacket.setPos(0.0f, 0.0f, 0.0f);
	this->jacket.addBox(-4.0f, 0.0f, -3.0f, 8, 18, 6, f + 0.5f);

	this->arms.setPos(0.0f, 3.0f, -1.0f);
	this->arms.addBox(-4.0f, 2.0f, -2.0f, 8, 4, 4, f);
	this->arms.texOffs(44, 22);
	this->arms.addBox(-8.0f, -2.0f, -2.0f, 4, 8, 4, f);
	this->arms.addBox(4.0f, -2.0f, -2.0f, 4, 8, 4, f);

	this->leg0.setPos(-2.0f, 12.0f, 0.0f);
	this->leg0.addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, f);

	this->leg1.mirror = 1;
	this->leg1.setPos(2.0f, 12.0f, 0.0f);
	this->leg1.addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, f);
}

VillagerModel::~VillagerModel() {
}

void VillagerModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) {
	float headTilt = animWindowHold(bob, 44.0f, 151U, 92U, true) * 0.20f;
	float headNod = animWindowHold(bob, 50.0f, 157U, 84U, false) * 0.14f + animBurstWave(bob, 52.0f, 163U, 72U, 2.2f) * 0.08f;
	float sideGlance = animWindowHold(bob, 48.0f, 167U, 86U, true) * 0.15f;
	float noseTwitch = animBurstWave(bob, 42.0f, 173U, 78U, 3.0f) * 0.08f;
	float armShrug = animWindowHold(bob, 54.0f, 179U, 80U, false) * 0.12f + animBurstWave(bob, 60.0f, 181U, 64U, 2.0f) * 0.06f;
	float bodySway = (r > 0.001f) ? (Mth::sin(time * 0.6662f) * r * 0.045f) : (animWindowHold(bob, 64.0f, 191U, 68U, true) * 0.03f);

	this->head.yRotAngle = yRot * (3.14159265f / 180.0f) + sideGlance;
	this->head.xRotAngle = xRot * (3.14159265f / 180.0f) + headNod;
	this->head.zRotAngle = headTilt;
	this->nose.yRotAngle = this->head.yRotAngle;
	this->nose.xRotAngle = this->head.xRotAngle + noseTwitch * 0.5f;
	this->nose.zRotAngle = headTilt + noseTwitch;

	this->body.zRotAngle = bodySway;
	this->jacket.zRotAngle = bodySway;

	this->arms.setPos(0.0f, 3.0f - armShrug * 2.5f, -1.0f);
	this->arms.xRotAngle = -0.75f - armShrug;
	this->arms.zRotAngle = bodySway * 0.8f;

	this->leg0.xRotAngle = Mth::cos(time * 0.6662f) * 1.4f * r * 0.5f;
	this->leg1.xRotAngle = Mth::cos(time * 0.6662f + 3.14159265f) * 1.4f * r * 0.5f;
	this->leg0.yRotAngle = 0.0f;
	this->leg1.yRotAngle = 0.0f;
}

void VillagerModel::render(Entity* ent, float time, float r, float bob, float yRot, float xRot, float scale) {
	this->setupAnim(time, r, bob, yRot, xRot, scale);
	this->head.render(scale);
	this->nose.render(scale);
	this->body.render(scale);
	this->jacket.render(scale);
	this->arms.render(scale);
	this->leg0.render(scale);
	this->leg1.render(scale);
}
