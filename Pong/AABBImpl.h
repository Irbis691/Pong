#pragma once

#include <algorithm>
#include "IAABB.h"

class AABBImpl : public IAABB
{
	float MinX;
	float MinY;
	float MaxX;
	float MaxY;

public:
	AABBImpl(float FirstX, float SecondX, float FirstY, float SecondY)
		: MinX(std::min(FirstX, SecondX)),
		  MinY(std::min(FirstY, SecondY)),
		  MaxX(std::max(FirstX, SecondX)),
		  MaxY(std::max(FirstY, SecondY))
	{
	}

	AABB GetAABB() const override;
};
