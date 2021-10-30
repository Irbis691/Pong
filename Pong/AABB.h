#pragma once
#include <algorithm>

struct AABB
{
private:
	float calculateSurfaceArea() const
	{
		return getWidth() * getHeight();
	}

public:
	float minX;
	float minY;
	float maxX;
	float maxY;
	float surfaceArea;

	AABB() : minX(0.0f), minY(0.0f), maxX(0.0f), maxY(0.0f), surfaceArea(0.0f)
	{
	}

	AABB(unsigned minX, unsigned minY, unsigned maxX, unsigned maxY) :
		AABB(static_cast<float>(minX), static_cast<float>(minY), static_cast<float>(maxX), static_cast<float>(maxY))
	{
	}

	AABB(float minX, float minY, float maxX, float maxY) :
		minX(minX), minY(minY), maxX(maxX), maxY(maxY)
	{
		surfaceArea = calculateSurfaceArea();
	}

	friend bool operator==(const AABB& lhs, const AABB& rhs)
	{
		return lhs.minX == rhs.minX
			&& lhs.minY == rhs.minY
			&& lhs.maxX == rhs.maxX
			&& lhs.maxY == rhs.maxY;
	}

	friend bool operator!=(const AABB& lhs, const AABB& rhs)
	{
		return !(lhs == rhs);
	}

	bool overlaps(const AABB& other) const
	{
		// y is deliberately first in the list of checks below as it is seen as more likely than things
		// collide on x,z but not on y than they do on y thus we drop out sooner on a y fail
		return maxX >= other.minX &&
			minX <= other.maxX &&
			maxY >= other.minY &&
			minY <= other.maxY;
	}

	bool contains(const AABB& other) const
	{
		return other.minX >= minX &&
			other.maxX <= maxX &&
			other.minY >= minY &&
			other.maxY <= maxY;
	}

	AABB merge(const AABB& other) const
	{
		return AABB(
			std::min(minX, other.minX), std::min(minY, other.minY),
			std::max(maxX, other.maxX), std::max(maxY, other.maxY)
		);
	}

	AABB intersection(const AABB& other) const
	{
		return AABB(
			std::max(minX, other.minX), std::max(minY, other.minY),
			std::min(maxX, other.maxX), std::min(maxY, other.maxY)
		);
	}

	float getWidth() const
	{
		return maxX - minX;
	}

	float getHeight() const
	{
		return maxY - minY;
	}
};
