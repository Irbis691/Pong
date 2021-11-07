#include "BulletManager.h"
#include <future>
#include "AABBImpl.h"

float BulletManager::Area(Float2 A, Float2 B, Float2 C)
{
	return (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
}

bool BulletManager::Intersect(float a, float b, float c, float d)
{
	if (a > b) std::swap(a, b);
	if (c > d) std::swap(c, d);
	return std::max(a, c) <= std::min(b, d);
}

bool BulletManager::Intersect(Float2 A, Float2 B, Float2 C, Float2 D)
{
	return Intersect(A.X, B.X, C.X, D.X)
		&& Intersect(A.Y, B.Y, C.Y, D.Y)
		&& Area(A, B, C) * Area(A, B, D) <= 0
		&& Area(C, D, A) * Area(C, D, B) <= 0;
}

bool BulletManager::DoesPointHitTheWall(const AABB& AABB, const Float2& Point, const Float2& PrevPoint)
{
	return Intersect(Float2{AABB.MinX, AABB.MinY}, Float2{AABB.MaxX, AABB.MaxY},
	                 Float2{PrevPoint.X, PrevPoint.Y},
	                 Float2{Point.X, Point.Y});
}

Float2 BulletManager::CalculateReflectedDirection(Bullet& b, const Float2& PrevPos, const AABB AABB)
{
	const Float2 FallingVector = {b.Position.X - PrevPos.X, b.Position.Y - PrevPos.Y};
	const Float2 Normal = {AABB.MinY - AABB.MaxY, AABB.MaxX - AABB.MinX};
	const float NormalLength = sqrt(Normal.X * Normal.X + Normal.Y * Normal.Y);
	const Float2 NormalNormalized = {Normal.X / NormalLength, Normal.Y / NormalLength};
	const float floatDotProduct = 2 * (FallingVector.X * NormalNormalized.X + FallingVector.Y * NormalNormalized.Y);
	const Float2 Subtrahend = {floatDotProduct * NormalNormalized.X, floatDotProduct * NormalNormalized.Y};
	const Float2 Reflected = {FallingVector.X - Subtrahend.X, FallingVector.Y - Subtrahend.Y};
	const float ReflectedLength = sqrt(Reflected.X * Reflected.X + Reflected.Y * Reflected.Y);
	return {Reflected.X / ReflectedLength, Reflected.Y / ReflectedLength};
}

void BulletManager::RemoveWallIfHit(const std::shared_ptr<IAABB>& InAABB, Bullet& b, const Float2& PrevPos)
{
	const auto AABB = InAABB->GetAABB();
	const bool DoesHit = DoesPointHitTheWall(AABB, b.Position, PrevPos);
	if (DoesHit)
	{
		WallsTree.RemoveObject(InAABB);
		const Float2 NormalizedReflected = CalculateReflectedDirection(b, PrevPos, AABB);
		b.Direction.X = NormalizedReflected.X;
		b.Direction.Y = NormalizedReflected.Y;
	}
}

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

const std::vector<Bullet>& BulletManager::GetBullets(float Time) const
{
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

			auto OverlapedAABB = WallsTree.QueryOverlaps(
				std::make_shared<AABBImpl>(PrevPos.X, b.Position.X, PrevPos.Y, b.Position.Y));

			for (const auto AABB : OverlapedAABB)
			{
				RemoveWallIfHit(AABB, b, PrevPos);
			}
		}
	}
	Bullets.erase(std::remove_if(Bullets.begin(), Bullets.end(), [Time](const Bullet& b)
	{
		return !b.IsAlive(Time);
	}), Bullets.end());
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
