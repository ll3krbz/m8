#pragma once
#include <rendering/entity/MobRenderer.hpp>

struct SnowGolemModel;

struct SnowGolemRenderer: MobRenderer {
	SnowGolemModel* snowmanModel;

	SnowGolemRenderer(SnowGolemModel*, float);
	virtual ~SnowGolemRenderer();

	virtual void additionalRendering(Mob*, float);
};
