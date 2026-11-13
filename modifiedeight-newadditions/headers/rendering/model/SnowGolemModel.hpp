#pragma once
#include <rendering/model/Model.hpp>
#include <rendering/model/ModelPart.hpp>

struct SnowGolemModel: Model {
	ModelPart piece1;
	ModelPart piece2;
	ModelPart head;
	ModelPart arm1;
	ModelPart arm2;

	SnowGolemModel();
	virtual ~SnowGolemModel();

	virtual void render(Entity*, float, float, float, float, float, float);
	virtual void setupAnim(float, float, float, float, float, float);
};
