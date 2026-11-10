#include <rendering/model/ChestModel.hpp>

ChestModel::ChestModel(bool_t a2)
	: field_18(0, 0, 64, 32)
	, field_98(0, 0, 64, 32)
	, field_118(0, 0, 64, 32) {
	float v4 = a2 ? -8.0f : 0.0f;
	int32_t v11 = a2 ? 30 : 14;
	float v8 = v4 + 1.0f;
	int32_t v5 = a2 ? 128 : 64;

	this->field_198 = a2;

	this->field_18.texOffsetX = 0;
	this->field_18.texOffsetY = 0;
	this->field_18.texWidth = (float)v5;
	this->field_18.texHeight = 64.0f;
	this->field_18.addBox(0.0f, -5.0f, -14.0f, v11, 5, 14, 0.0f);
	this->field_18.yOffset = 7.0f;
	this->field_18.zOffset = 15.0f;
	this->field_18.xOffset = v8;

	this->field_118.texOffsetX = 0;
	this->field_118.texOffsetY = 0;
	this->field_118.texWidth = (float)v5;
	this->field_118.texHeight = 64.0f;
	this->field_118.addBox(-1.0f, -2.0f, -15.0f, 2, 4, 1, 0.0f);
	this->field_118.yOffset = 7.0f;
	this->field_118.xOffset = 8.0f;
	this->field_118.zOffset = 15.0f;

	this->field_98.texOffsetX = 0;
	this->field_98.texOffsetY = 19;
	this->field_98.texWidth = (float)v5;
	this->field_98.texHeight = 64.0f;
	this->field_98.addBox(0.0f, 0.0f, 0.0f, v11, 10, 14, 0.0f);
	this->field_98.xOffset = v8;
	this->field_98.zOffset = 1.0f;
	this->field_98.yOffset = 6.0f;
}

ChestModel::~ChestModel() {
}

void ChestModel::render() {
	this->field_118.xRotAngle = this->field_18.xRotAngle;
	this->field_18.render(0.0625f);
	this->field_118.render(0.0625f);
	this->field_98.render(0.0625f);
}
