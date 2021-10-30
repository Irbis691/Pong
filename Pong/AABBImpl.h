#pragma once


class AABBImpl : public IAABB {
	float minX;
	float minY;
	float maxX;
	float maxY;

public:
	AABBImpl(float min_x, float min_y, float max_x, float max_y)
		: minX(min_x),
		minY(min_y),
		maxX(max_x),
		maxY(max_y) {
	}

	AABB getAABB() const override;
};