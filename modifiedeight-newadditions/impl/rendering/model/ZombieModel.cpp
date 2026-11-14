#include <rendering/model/ZombieModel.hpp>
#include <rendering/model/MobAnimUtils.hpp>
#include <math/Mth.hpp>

ZombieModel::ZombieModel()
	: HumanoidModel(0, 0) {
}

ZombieModel::~ZombieModel() {
}
void ZombieModel::setupAnim(float a2, float a3, float a4, float a5, float a6, float a7) {
	float v9;  // s16
	float v10; // s19
	float v11; // r0
	float v12; // s15
	float v13; // s16
	float v14; // s19
	float v15; // r0

	HumanoidModel::setupAnim(a2, a3, a4, a5, a6, a7);
	if(!this->field_318 && !this->field_319) {
		v9 = this->field_0;
		v10 = Mth::sin(v9 * 3.1416);
		v11 = Mth::sin((float)(1.0 - (float)((float)(1.0 - v9) * (float)(1.0 - v9))) * 3.1416);
		v12 = 0.1 - (float)(v10 * 0.6);
		this->rightArmModel.zRotAngle = 0.0;
		this->leftArmModel.zRotAngle = 0.0;
		this->leftArmModel.yRotAngle = v12;
		this->rightArmModel.yRotAngle = -v12;
		v13 = (float) - (float)(3.1416 * 0.5) - (float)((float)(v10 * 1.2) - (float)(v11 * 0.4));
		this->rightArmModel.xRotAngle = v13;
		this->leftArmModel.xRotAngle = v13;
		v14 = (float)(Mth::cos(a4 * 0.09) * 0.05) + 0.05;
		v15 = Mth::sin(a4 * 0.067);
		float reachR = animWindowHold(a4, 46.0f, 321U, 88U, false) * 0.22f + animBurstWave(a4, 58.0f, 323U, 62U, 2.2f) * 0.09f;
		float reachL = animWindowHold(a4, 52.0f, 329U, 88U, false) * 0.22f + animBurstWave(a4, 62.0f, 331U, 62U, 2.2f) * 0.09f;
		float armSway = animWindowHold(a4, 48.0f, 337U, 80U, true) * 0.12f;
		float neckLoll = animWindowHold(a4, 42.0f, 347U, 92U, true) * 0.22f + animBurstWave(a4, 68.0f, 349U, 48U, 2.8f) * 0.08f;
		this->headModel.zRotAngle += neckLoll;
		this->rightArmModel.zRotAngle = v14 + fabsf(armSway) * 0.5f;
		this->leftArmModel.zRotAngle = 0.0f - v14 - fabsf(armSway) * 0.5f;
		this->rightArmModel.yRotAngle += armSway;
		this->leftArmModel.yRotAngle += armSway;
		this->rightArmModel.xRotAngle = v13 + (float)(v15 * 0.05) - reachR;
		this->leftArmModel.xRotAngle = v13 - (float)(v15 * 0.05) - reachL;
	}
}
