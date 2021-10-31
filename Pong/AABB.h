#pragma once
#include <algorithm>

struct AABB
{
private:
	float CalculateSurfaceArea() const
	{
		return GetWidth() * GetHeight();
	}

public:
	float MinX;
	float MinY;
	float MaxX;
	float MaxY;
	float SurfaceArea;

	AABB() : MinX(0.0f), MinY(0.0f), MaxX(0.0f), MaxY(0.0f), SurfaceArea(0.0f)
	{
	}

	AABB(int MinX, int MinY, int MaxX, int MaxY) :
		AABB(static_cast<float>(MinX), static_cast<float>(MinY), static_cast<float>(MaxX), static_cast<float>(MaxY))
	{
	}

	AABB(float MinX, float MinY, float MaxX, float MaxY) :
		MinX(MinX), MinY(MinY), MaxX(MaxX), MaxY(MaxY)
	{
		SurfaceArea = CalculateSurfaceArea();
	}

	friend bool operator==(const AABB& Lhs, const AABB& Rhs)
	{
		return Lhs.MinX == Rhs.MinX
			&& Lhs.MinY == Rhs.MinY
			&& Lhs.MaxX == Rhs.MaxX
			&& Lhs.MaxY == Rhs.MaxY;
	}

	friend bool operator!=(const AABB& Lhs, const AABB& Rhs)
	{
		return !(Lhs == Rhs);
	}

	bool Overlaps(const AABB& Other) const
	{
		// y is deliberately first in the list of checks below as it is seen as more likely than things
		// collide on x,z but not on y than they do on y thus we drop out sooner on a y fail
		return MaxX >= Other.MinX &&
			MinX <= Other.MaxX &&
			MaxY >= Other.MinY &&
			MinY <= Other.MaxY;
	}

	bool Contains(const AABB& Other) const
	{
		return Other.MinX >= MinX &&
			Other.MaxX <= MaxX &&
			Other.MinY >= MinY &&
			Other.MaxY <= MaxY;
	}

	AABB Merge(const AABB& Other) const
	{
		return AABB(
			std::min(MinX, Other.MinX), std::min(MinY, Other.MinY),
			std::max(MaxX, Other.MaxX), std::max(MaxY, Other.MaxY)
		);
	}

	AABB Intersection(const AABB& Other) const
	{
		return AABB(
			std::max(MinX, Other.MinX), std::max(MinY, Other.MinY),
			std::min(MaxX, Other.MaxX), std::min(MaxY, Other.MaxY)
		);
	}

	float GetWidth() const
	{
		return MaxX - MinX;
	}

	float GetHeight() const
	{
		return MaxY - MinY;
	}
};
