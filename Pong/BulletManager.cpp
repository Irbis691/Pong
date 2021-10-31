#include "BulletManager.h"
#include "AABBImpl.h"

#define SEGMENT_COORDS_AMOUNT 4

std::vector<float> BulletManager::GetWalls()
{
	std::vector<float> Result;

	for (const auto& AABBWall : WallsTree.GetNodes())
	{
		Result.push_back(AABBWall.MinX);
		Result.push_back(AABBWall.MinY);
		Result.push_back(AABBWall.MaxX);
		Result.push_back(AABBWall.MaxY);
	}

	return Result;
}

const std::vector<Bullet>& BulletManager::GetBullets(float Time)
{
	Bullets.erase(std::remove_if(Bullets.begin(), Bullets.end(), [Time](const Bullet& b)
	{
		return !b.IsAlive(Time);
	}), Bullets.end());
	return Bullets;
}

void BulletManager::Update(float Time)
{
	for (Bullet& b : Bullets)
	{
		if (b.IsAlive(Time))
		{
			Float2 PrevPos = {b.Position.X, b.Position.Y};
			b.Position.X += b.Direction.X * b.Speed;
			b.Position.Y += b.Direction.Y * b.Speed;

			auto WallsToRemove = WallsTree.QueryOverlaps(
				std::make_shared<AABBImpl>(PrevPos.X, b.Position.X, PrevPos.Y, b.Position.Y));

			for (const auto Wall : WallsToRemove)
			{
				WallsTree.RemoveObject(Wall);
				const Float2 FallingVector = {b.Position.X - PrevPos.X, b.Position.Y - PrevPos.Y};
				const auto AABB = Wall->GetAABB();
				const Float2 Normal = {AABB.MaxY - AABB.MaxY, AABB.MaxX - AABB.MinX};
				const float NormalLength = sqrt(Normal.X * Normal.X + Normal.Y * Normal.Y);
				const Float2 NormalNormalized = {Normal.X / NormalLength, Normal.Y / NormalLength};
				const float floatDotProduct = 2 * (FallingVector.X * NormalNormalized.X + FallingVector.Y *
					NormalNormalized.Y);
				const Float2 Subtrahend = {
					floatDotProduct * NormalNormalized.X, floatDotProduct * NormalNormalized.Y
				};
				const Float2 Reflected = {FallingVector.X - Subtrahend.X, FallingVector.Y - Subtrahend.Y};
				const float ReflectedLength = sqrt(Reflected.X * Reflected.X + Reflected.Y * Reflected.Y);
				const Float2 NormalizedReflected = {Reflected.X / ReflectedLength, Reflected.Y / ReflectedLength};
				b.Direction.X = NormalizedReflected.X;
				b.Direction.Y = NormalizedReflected.Y;
			}
		}
	}
}

void BulletManager::Fire(const Float2& StartPosition, const Float2& StartDirection, float StartTime, float LifeTime,
                         float Speed)
{
	const float StartDirectionLenqgth = sqrt(StartDirection.X * StartDirection.X + StartDirection.Y * StartDirection.Y);
	const Float2 StartDirectionNormalized = {
		StartDirection.X / StartDirectionLenqgth, StartDirection.Y / StartDirectionLenqgth
	};
	Bullets.push_back({StartPosition, StartDirectionNormalized, StartTime, LifeTime, Speed});
}
