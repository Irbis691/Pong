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
	const std::vector<Bullet>& GetBullets(float Time);
	std::vector<float> GetWalls();

private:
	std::vector<Bullet> Bullets;
	AABBTree WallsTree;
};
