#pragma once

#include <vector>

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
	BulletManager(std::vector<float> InWalls) : Walls(InWalls)
	{
	};

	void Update(float time);
	void Fire(const Float2& StartPosistion, const Float2& StartDirection, float Speed, float StartTime, float LifeTime);
	const std::vector<Bullet>& GetBullets();
	const std::vector<float>& GetWalls();

private:
	std::vector<Bullet> Bullets;
	std::vector<float> Walls;

	float Area(Float2 A, Float2 B, Float2 C);
	bool Intersect(float a, float b, float c, float d);
	bool Intersect(Float2 A, Float2 B, Float2 C, Float2 D);
	bool DoesPointHitTheWall(const std::vector<float>& Walls, const Float2& Point, const Float2& PrevPoint, size_t i);
	int DoesHitTheWall(const Float2& Point, const Float2& PrevPoint);
};
