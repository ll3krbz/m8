#include <level/LevelHeight.hpp>
#include <rendering/model/HumanoidModel.hpp>
#include <rendering/model/MobAnimUtils.hpp>
#include <chrono>
#include <map>
#include <entity/Entity.hpp>
#include <entity/Mob.hpp>
#include <entity/Player.hpp>
#include <entity/LocalPlayer.hpp>
#include <math/Mth.hpp>
#include <level/Level.hpp>
#include <tile/material/Material.hpp>
#include <tile/Tile.hpp>
#include <cmath>
#include <math.h>

HumanoidModel::HumanoidModel(float a2, float a3)
	: headModel(0, 0, 64, 32)
	, bodyModel(16, 16, 64, 32)
	, rightArmModel(40, 16, 64, 32)
	, leftArmModel(40, 16, 64, 32)
	, rightLegModel(0, 16, 64, 32)
	, leftLegModel(0, 16, 64, 32) {
	this->field_0 = 0;
	this->field_14 = 1;
	this->field_4 = 0;
	this->field_5 = 0;
	AppPlatform::_singleton->addListener(this, 1);
	this->field_318 = 0;
	this->field_319 = 0;
	this->field_31A = 0;
	this->isUsingBow = 0;
	this->isSwimming = 0;
	this->curEntity = nullptr;
	this->field_31C = 0;
	this->crouchAmount = 0.0f;
	this->swimAmount = 0.0f;
	this->headModel.setModel(this);
	this->bodyModel.setModel(this);
	this->rightArmModel.setModel(this);
	this->leftArmModel.setModel(this);
	this->rightLegModel.setModel(this);
	this->leftLegModel.setModel(this);
	this->headModel.addBox(-4.0, -8.0, -4.0, 8, 8, 8, a2);
	this->headModel.setPos(0.0, a3 + 0.0, 0.0);
	this->bodyModel.addBox(-4.0, 0.0, -2.0, 8, 12, 4, a2);
	this->bodyModel.setPos(0.0, a3 + 0.0, 0.0);
	this->rightArmModel.addBox(-3.0, -2.0, -2.0, 4, 12, 4, a2);
	this->rightArmModel.setPos(-5.0, a3 + 2.0, 0.0);
	this->leftArmModel.mirror = 1;
	this->leftArmModel.addBox(-1.0, -2.0, -2.0, 4, 12, 4, a2);
	this->leftArmModel.setPos(5.0, a3 + 2.0, 0.0);
	this->rightLegModel.addBox(-2.0, 0.0, -2.0, 4, 12, 4, a2);
	float v8 = a3 + 12.0;
	this->rightLegModel.setPos(-2.0, v8, 0.0);
	this->leftLegModel.mirror = 1;
	this->leftLegModel.addBox(-2.0, 0.0, -2.0, 4, 12, 4, a2);
	this->leftLegModel.setPos(2.0, v8, 0.0);
}

HumanoidModel::~HumanoidModel() {
}
static bool computeEntitySwimming(Entity* a2, bool_t fallback) {
	if (!a2 || a2->isRiding()) return false;
	if (!a2->isMob()) return fallback != 0;
	Mob* m = (Mob*)a2;
	if (m->entityHeight < 0.8f || m->isUnderLiquid(Material::water)) return true;
	if (m->isInWater() && !m->onGround && m->level) {
		return m->level->getMaterial(Mth::floor(m->posX), Mth::floor(m->boundingBox.minY + 0.9f), Mth::floor(m->posZ)) == Material::water;
	}
	return false;
}

void HumanoidModel::render(Entity* a2, float a3, float a4, float a5, float a6, float a7, float a8) {
	ItemInstance* item;

	this->curEntity = a2;
	this->field_31A = 0;
	this->isSwimming = 0;
	if(a2) {
		this->field_4 = a2->isRiding();
		this->field_31A = a2->isSneaking();
		if(a2->isMob()) {
			Mob* m = (Mob*)a2;
			item = m->getCarriedItem();
			if(item) {
				if(m->getUseItemDuration() > 0 && item->getUseAnimation() == 4) {
					this->isUsingBow = 1;
				}
			}
			this->isSwimming = computeEntitySwimming(a2, 0);
		}
	}
	this->setupAnim(a3, a4, a5, a6, a7, a8);
	this->headModel.render(a8);
	this->bodyModel.render(a8);
	this->rightArmModel.render(a8);
	this->leftArmModel.render(a8);
	this->rightLegModel.render(a8);
	this->leftLegModel.render(a8);
	this->isUsingBow = 0;
	this->field_31A = 0;
	this->isSwimming = 0;
	this->curEntity = nullptr;
}

struct HumanoidPoseState {
	float crouch;
	float swim;
	float lastBob;
	uint64_t lastMs;
};
static std::map<int32_t, HumanoidPoseState> s_poseStates;

static uint64_t getPoseNowMs() {
	return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
}

static void updatePoseBlend(Entity* ent, float targetCrouch, float targetSwim, float bob, float& outCrouch, float& outSwim) {
	if (!ent) {
		outCrouch = targetCrouch;
		outSwim = targetSwim;
		return;
	}
	uint64_t nowMs = getPoseNowMs();
	auto it = s_poseStates.find(ent->entityId);
	if (it == s_poseStates.end()) {
		HumanoidPoseState st;
		st.crouch = targetCrouch;
		st.swim = targetSwim;
		st.lastBob = bob;
		st.lastMs = nowMs;
		s_poseStates[ent->entityId] = st;
		outCrouch = targetCrouch;
		outSwim = targetSwim;
		return;
	}
	HumanoidPoseState& st = it->second;
	if (fabsf(bob - st.lastBob) > 1e-4f || (nowMs >= st.lastMs && (nowMs - st.lastMs) >= 16)) {
		float dt = (nowMs > st.lastMs) ? (float)(nowMs - st.lastMs) * 0.001f : 0.0f;
		if (dt > 0.05f) dt = 0.05f;
		float cAlpha = 1.0f - expf(-18.0f * dt);
		float sAlpha = 1.0f - expf(-10.0f * dt);
		st.crouch += (targetCrouch - st.crouch) * cAlpha;
		st.swim += (targetSwim - st.swim) * sAlpha;
		if (fabsf(targetCrouch - st.crouch) < 0.001f) st.crouch = targetCrouch;
		if (fabsf(targetSwim - st.swim) < 0.001f) st.swim = targetSwim;
		st.lastBob = bob;
		st.lastMs = nowMs;
	}
	outCrouch = st.crouch;
	outSwim = st.swim;
}

void HumanoidModel::renderHorrible(float a2, float a3, float a4, float a5, float a6, float a7) {
	this->setupAnim(a2, a3, a4, a5, a6, a7);
	this->headModel.renderHorrible(a7);
	this->bodyModel.renderHorrible(a7);
	this->rightArmModel.renderHorrible(a7);
	this->leftArmModel.renderHorrible(a7);
	this->rightLegModel.renderHorrible(a7);
	this->leftLegModel.renderHorrible(a7);
}

void HumanoidModel::setupAnim(float a2, float a3, float a4, float a5, float a6, float a7) {
	float v10;		 // s19
	float v11;		 // s16
	float v12;		 // r0
	float v14;		 // s15
	float v15;		 // s15
	float v16;		 // s15
	float v17;		 // s16
	float v18;		 // r0
	float v19;		 // s21
	float v20;		 // s19
	float v21;		 // s22
	float v22;		 // r0
	float v23;		 // s15
	float v24;		 // s14
	float v25;		 // s16
	float v26;		 // r0
	float v28;		 // s15
	float yRotAngle; // s13
	float v30;		 // s14

	this->headModel.xOffset = 0.0f;
	this->headModel.yOffset = 0.0f;
	this->headModel.zOffset = 0.0f;
	this->headModel.zRotAngle = 0.0f;
	this->bodyModel.xOffset = 0.0f;
	this->bodyModel.yOffset = 0.0f;
	this->bodyModel.zOffset = 0.0f;
	this->bodyModel.xRotAngle = 0.0f;
	this->bodyModel.yRotAngle = 0.0f;
	this->bodyModel.zRotAngle = 0.0f;
	this->rightArmModel.xOffset = -5.0f;
	this->rightArmModel.yOffset = 2.0f;
	this->rightArmModel.zOffset = 0.0f;
	this->leftArmModel.xOffset = 5.0f;
	this->leftArmModel.yOffset = 2.0f;
	this->leftArmModel.zOffset = 0.0f;
	this->rightLegModel.xOffset = -2.0f;
	this->rightLegModel.yOffset = 12.0f;
	this->rightLegModel.zOffset = 0.0f;
	this->rightLegModel.zRotAngle = 0.0f;
	this->leftLegModel.xOffset = 2.0f;
	this->leftLegModel.yOffset = 12.0f;
	this->leftLegModel.zOffset = 0.0f;
	this->leftLegModel.zRotAngle = 0.0f;

	this->headModel.yRotAngle = a5 / (float)(180.0 / 3.1416);
	v10 = a2 * 0.6662;
	this->headModel.xRotAngle = a6 / (float)(180.0 / 3.1416);
	v11 = Mth::cos(a2 * 0.6662) * a3;
	v12 = Mth::cos(v10 + 3.1416);
	this->leftArmModel.xRotAngle = v11;
	this->rightArmModel.zRotAngle = 0.0;
	this->leftArmModel.zRotAngle = 0.0;
	this->rightLegModel.yRotAngle = 0.0;
	this->leftLegModel.yRotAngle = 0.0;
	this->rightArmModel.xRotAngle = v12 * a3;
	this->rightLegModel.xRotAngle = v11 * 1.4;
	this->leftLegModel.xRotAngle = (float)(v12 * a3) * 1.4;
	if(this->field_4) {
		v14 = -(float)(3.1416 * 0.5);
		this->rightArmModel.xRotAngle = (float)(v12 * a3) + (float)(v14 * 0.4);
		this->rightLegModel.xRotAngle = v14 * 0.8;
		this->leftLegModel.xRotAngle = v14 * 0.8;
		this->rightLegModel.yRotAngle = (float)(3.1416 * 0.5) * 0.2;
		this->leftLegModel.yRotAngle = v14 * 0.2;
		this->leftArmModel.xRotAngle = v11 + (float)(v14 * 0.4);
	}
	if(this->field_318) {
		this->leftArmModel.xRotAngle = (float)(this->leftArmModel.xRotAngle * 0.5) - (float)((float)(3.1416 * 0.5) * 0.2);
	}
	if(this->field_319) {
		this->rightArmModel.xRotAngle = (float)(this->rightArmModel.xRotAngle * 0.5) - (float)((float)(3.1416 * 0.5) * 0.2);
	}
	v15 = this->field_0;
	this->rightArmModel.yRotAngle = 0.0;
	this->leftArmModel.yRotAngle = 0.0;
	if(v15 > -9990.0) {
		v16 = sqrt(v15);
		v17 = Mth::sin((float)(v16 * 3.1416) + (float)(v16 * 3.1416)) * 0.2;
		this->bodyModel.yRotAngle = v17;
		this->rightArmModel.zOffset = Mth::sin(v17) * 5.0;
		this->rightArmModel.xOffset = -(float)(Mth::cos(v17) * 5.0);
		this->leftArmModel.zOffset = -(float)(Mth::sin(v17) * 5.0);
		v18 = Mth::cos(v17);
		v19 = this->field_0;
		this->leftArmModel.yRotAngle = this->leftArmModel.yRotAngle + v17;
		this->leftArmModel.xRotAngle = this->leftArmModel.xRotAngle + v17;
		this->leftArmModel.xOffset = v18 * 5.0;
		v20 = v17 + this->rightArmModel.yRotAngle;
		this->rightArmModel.yRotAngle = v20;
		v21 = Mth::sin((float)(1.0 - (float)((float)((float)(1.0 - v19) * (float)(1.0 - v19)) * (float)((float)(1.0 - v19) * (float)(1.0 - v19)))) * 3.1416);
		v22 = Mth::sin(v19 * 3.1416);
		v23 = this->headModel.xRotAngle - 0.7;
		this->rightArmModel.yRotAngle = v20 + (float)(v17 + v17);
		this->rightArmModel.xRotAngle = this->rightArmModel.xRotAngle - (float)((float)((float)-(float)(v23 * v22) * 0.75) + (float)(v21 * 1.2));
		this->rightArmModel.zRotAngle = Mth::sin(v19 * 3.1416) * -0.4;
	}
	bool swimmingNow = this->curEntity ? computeEntitySwimming(this->curEntity, this->isSwimming) : (this->isSwimming != 0);
	float targetCrouch = (this->field_31A && !this->field_4 && !swimmingNow) ? 1.0f : 0.0f;
	float targetSwim = (swimmingNow && !this->field_4) ? 1.0f : 0.0f;
	float crouch = 0.0f;
	float swim = 0.0f;
	updatePoseBlend(this->curEntity, targetCrouch, targetSwim, a4, crouch, swim);
	this->crouchAmount = crouch;
	this->swimAmount = swim;
	this->bodyModel.xRotAngle = 0.5f * crouch;
	this->bodyModel.yOffset = 3.2f * crouch;
	this->rightArmModel.xRotAngle = this->rightArmModel.xRotAngle + 0.4f * crouch;
	this->leftArmModel.xRotAngle = this->leftArmModel.xRotAngle + 0.4f * crouch;
	this->rightArmModel.yOffset = 2.0f + 3.2f * crouch;
	this->leftArmModel.yOffset = 2.0f + 3.2f * crouch;
	this->rightLegModel.zOffset = 4.0f * crouch;
	this->leftLegModel.zOffset = 4.0f * crouch;
	this->rightLegModel.yOffset = 12.0f;
	this->leftLegModel.yOffset = 12.0f;
	this->headModel.yOffset = 4.2f * crouch;
	v25 = (float)(Mth::cos(a4 * 0.09) * 0.05) + 0.05;
	v26 = Mth::sin(a4 * 0.067);
	this->rightArmModel.zRotAngle = this->rightArmModel.zRotAngle + v25;
	v28 = v26 * 0.05;
	this->leftArmModel.zRotAngle = this->leftArmModel.zRotAngle - v25;
	this->rightArmModel.xRotAngle = this->rightArmModel.xRotAngle + (float)(v26 * 0.05);
	this->leftArmModel.xRotAngle = this->leftArmModel.xRotAngle - (float)(v26 * 0.05);
	if (!this->curEntity || !this->curEntity->isPlayer()) {
		uint32_t seed = this->curEntity ? (uint32_t)this->curEntity->entityId : 36U;
		float t = a4 + (float)((seed & 63U) * 17U);
		float headTilt = animWindowHold(t, 44.0f, seed + 13U, 88U, true) * 0.18f;
		float headScan = animWindowHold(t, 52.0f, seed + 29U, 84U, true) * 0.15f;
		float headNod = animWindowHold(t, 58.0f, seed + 43U, 72U, true) * 0.12f;
		float armGestureR = animWindowHold(t, 48.0f, seed + 61U, 76U, true) * 0.14f;
		float armGestureL = animWindowHold(t, 54.0f, seed + 73U, 76U, true) * 0.14f;
		float torsoShift = animWindowHold(t, 64.0f, seed + 89U, 68U, true) * 0.04f;
		this->headModel.zRotAngle += headTilt;
		this->headModel.yRotAngle += headScan;
		this->headModel.xRotAngle += headNod;
		this->bodyModel.zRotAngle += torsoShift + (a3 > 0.001f ? Mth::sin(a2 * 0.6662f) * a3 * 0.045f : 0.0f);
		this->rightArmModel.xRotAngle += armGestureR;
		this->leftArmModel.xRotAngle += armGestureL;
	}
	if(this->isUsingBow) {
		yRotAngle = this->headModel.yRotAngle;
		this->rightArmModel.yRotAngle = yRotAngle - 0.26f;
		this->leftArmModel.yRotAngle = yRotAngle + 0.78f;
		v30 = this->headModel.xRotAngle - (float)(3.1416 * 0.5);
		this->rightArmModel.zRotAngle = v25 + 0.0f;
		this->leftArmModel.zRotAngle = 0.0f - v25;
		this->rightArmModel.xRotAngle = v30 + v28;
		this->leftArmModel.xRotAngle = v30 - v28;
	}
	if(swim > 0.001f) {
		auto lerpF = [](float a, float b, float t) { return a + (b - a) * t; };
		float pitchRad = a6 / 57.29578f;
		float bodyTilt = 1.5708f + pitchRad * 0.35f;
		if (bodyTilt < 1.12f) bodyTilt = 1.12f;
		if (bodyTilt > 2.02f) bodyTilt = 2.02f;
		float phase = a4 * 0.32f;
		float moveFactor = a3 * 1.4f;
		if (moveFactor > 1.0f) moveFactor = 1.0f;
		float amp = 0.28f + 0.72f * moveFactor;
		float swimArm = sinf(phase) * amp;
		float swimArm2 = cosf(phase) * amp;
		float kick = sinf(phase * 1.6f) * (0.18f + 0.25f * moveFactor);
		float bodySin = sinf(bodyTilt);
		float bodyCos = cosf(bodyTilt);
		float baseY = 18.0f;

		this->bodyModel.xRotAngle = lerpF(this->bodyModel.xRotAngle, bodyTilt, swim);
		this->bodyModel.yOffset = lerpF(this->bodyModel.yOffset, baseY, swim);
		this->bodyModel.zOffset = lerpF(this->bodyModel.zOffset, 0.0f, swim);
		this->bodyModel.yRotAngle = lerpF(this->bodyModel.yRotAngle, 0.0f, swim);
		this->bodyModel.zRotAngle = lerpF(this->bodyModel.zRotAngle, swimArm2 * 0.12f * moveFactor, swim);

		this->headModel.yOffset = lerpF(this->headModel.yOffset, baseY, swim);
		this->headModel.zOffset = lerpF(this->headModel.zOffset, 0.0f, swim);
		this->headModel.xRotAngle = lerpF(this->headModel.xRotAngle, -0.25f + pitchRad * 0.45f, swim);

		this->rightArmModel.xOffset = lerpF(this->rightArmModel.xOffset, -5.0f, swim);
		this->rightArmModel.yOffset = lerpF(this->rightArmModel.yOffset, baseY + 2.0f * bodyCos, swim);
		this->rightArmModel.zOffset = lerpF(this->rightArmModel.zOffset, 2.0f * bodySin, swim);
		this->rightArmModel.xRotAngle = lerpF(this->rightArmModel.xRotAngle, bodyTilt + swimArm * 0.65f, swim);
		this->rightArmModel.yRotAngle = lerpF(this->rightArmModel.yRotAngle, -0.25f, swim);
		this->rightArmModel.zRotAngle = lerpF(this->rightArmModel.zRotAngle, 0.35f + swimArm2 * 0.30f, swim);

		this->leftArmModel.xOffset = lerpF(this->leftArmModel.xOffset, 5.0f, swim);
		this->leftArmModel.yOffset = lerpF(this->leftArmModel.yOffset, baseY + 2.0f * bodyCos, swim);
		this->leftArmModel.zOffset = lerpF(this->leftArmModel.zOffset, 2.0f * bodySin, swim);
		this->leftArmModel.xRotAngle = lerpF(this->leftArmModel.xRotAngle, bodyTilt - swimArm * 0.65f, swim);
		this->leftArmModel.yRotAngle = lerpF(this->leftArmModel.yRotAngle, 0.25f, swim);
		this->leftArmModel.zRotAngle = lerpF(this->leftArmModel.zRotAngle, -0.35f - swimArm2 * 0.30f, swim);

		float hipY = baseY + 12.0f * bodyCos;
		float hipZ = 12.0f * bodySin;
		this->rightLegModel.xOffset = lerpF(this->rightLegModel.xOffset, -2.0f, swim);
		this->rightLegModel.yOffset = lerpF(this->rightLegModel.yOffset, hipY, swim);
		this->rightLegModel.zOffset = lerpF(this->rightLegModel.zOffset, hipZ, swim);
		this->rightLegModel.xRotAngle = lerpF(this->rightLegModel.xRotAngle, bodyTilt + kick, swim);
		this->rightLegModel.zRotAngle = lerpF(this->rightLegModel.zRotAngle, 0.05f, swim);
		this->rightLegModel.yRotAngle = lerpF(this->rightLegModel.yRotAngle, 0.0f, swim);

		this->leftLegModel.xOffset = lerpF(this->leftLegModel.xOffset, 2.0f, swim);
		this->leftLegModel.yOffset = lerpF(this->leftLegModel.yOffset, hipY, swim);
		this->leftLegModel.zOffset = lerpF(this->leftLegModel.zOffset, hipZ, swim);
		this->leftLegModel.xRotAngle = lerpF(this->leftLegModel.xRotAngle, bodyTilt - kick, swim);
		this->leftLegModel.zRotAngle = lerpF(this->leftLegModel.zRotAngle, -0.05f, swim);
		this->leftLegModel.yRotAngle = lerpF(this->leftLegModel.yRotAngle, 0.0f, swim);
	}
}
