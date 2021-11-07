#pragma once

#include <vector>
#include "AABBTree.h"

struct Float2
{
	float X;
	float Y;
};

struct Bullet
{
	Float2 Position;
	Float2 Direction;
	float BirthTime;
	float LifeTime;
	float Speed;

	bool IsAlive(float CurrentTime) const
	{
		return BirthTime + LifeTime > CurrentTime;
	}
};

class BulletManager
{
public:
	BulletManager(const AABBTree& InWallsTree) : WallsTree(InWallsTree)
	{
	};

	void Update(float Time);
	void Fire(const Float2& StartPosition, const Float2& StartDirection, float StartTime, float LifeTime, float Speed);
	const std::vector<Bullet>& GetBullets(float Time) const;
	std::vector<float> GetWalls();

private:
	std::vector<Bullet> Bullets;
	AABBTree WallsTree;

	float Area(Float2 A, Float2 B, Float2 C);
	bool Intersect(float a, float b, float c, float d);
	bool Intersect(Float2 A, Float2 B, Float2 C, Float2 D);
	bool DoesPointHitTheWall(const AABB& AABB, const Float2& Point, const Float2& PrevPoint);
	static Float2 CalculateReflectedDirection(Bullet& b, const Float2& PrevPos, AABB AABB);
	void RemoveWallIfHit(const std::shared_ptr<IAABB>& InAABB, Bullet& b, const Float2& PrevPos);
};
