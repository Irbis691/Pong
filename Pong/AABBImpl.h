#pragma once

#include "IAABB.h"

class AABBImpl : public IAABB {
	float MinX;
	float MinY;
	float MaxX;
	float MaxY;

public:
	AABBImpl(float MinX, float MinY, float MaxX, float MaxY)
		: MinX(MinX),
		MinY(MinY),
		MaxX(MaxX),
		MaxY(MaxY) {
	}

	AABB GetAABB() const override;
};
