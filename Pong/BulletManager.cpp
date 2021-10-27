#include "BulletManager.h"

#define SEGMENT_COORDS_AMOUNT 4

float BulletManager::Area(Float2 A, Float2 B, Float2 C) {
	return (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
}

bool BulletManager::Intersect(float a, float b, float c, float d) {
	if (a > b) std::swap(a, b);
	if (c > d) std::swap(c, d);
	return std::max(a, c) <= std::min(b, d);
}

bool BulletManager::Intersect(Float2 A, Float2 B, Float2 C, Float2 D) {
	return Intersect(A.X, B.X, C.X, D.X)
		&& Intersect(A.Y, B.Y, C.Y, D.Y)
		&& Area(A, B, C) * Area(A, B, D) <= 0
		&& Area(C, D, A) * Area(C, D, B) <= 0;
}

bool BulletManager::DoesPointHitTheWall(const std::vector<float>& Walls, const Float2& Point, const Float2& PrevPoint, size_t i) {
	return Intersect(Float2{ Walls[i], Walls[i + 1] }, Float2{ Walls[i + 2], Walls[i + 3] },
		Float2{ PrevPoint.X, PrevPoint.Y },
		Float2{ Point.X, Point.Y });
}

int BulletManager::DoesHitTheWall(const Float2& Point, const Float2& PrevPoint) {
	for (size_t i = 0; i < Walls.size(); i += SEGMENT_COORDS_AMOUNT) {
		if (DoesPointHitTheWall(Walls, Point, PrevPoint, i)) {
			return i;
		}
	}
	return -1;
}

const std::vector<float>& BulletManager::GetWalls()
{
	return Walls;
}

const std::vector<Bullet>& BulletManager::GetBullets()
{
	return Bullets;
}

void BulletManager::Update(float Time)
{
	for (Bullet& b : Bullets) {
		if (b.IsAlive(Time)) {
			Float2 PrevPos = { b.Position.X, b.Position.Y };
			b.Position.X += b.Direction.X * b.Speed;
			b.Position.Y += b.Direction.Y * b.Speed;

			const int HitCoord = DoesHitTheWall(b.Position, PrevPos);
			if (HitCoord != -1) {
				const Float2 FallingVector = { b.Position.X - PrevPos.X, b.Position.Y - PrevPos.Y };
				const Float2 Normal = {
					Walls[HitCoord + 1] - Walls[HitCoord + 3],
					Walls[HitCoord + 2] - Walls[HitCoord]
				};
				const float NormalLength = sqrt(Normal.X * Normal.X + Normal.Y * Normal.Y);
				const Float2 NormalNormalized = { Normal.X / NormalLength, Normal.Y / NormalLength };
				const float floatDotProduct = 2 * (FallingVector.X * NormalNormalized.X + FallingVector.Y *
					NormalNormalized.Y);
				const Float2 Subtrahend = {
					floatDotProduct * NormalNormalized.X, floatDotProduct * NormalNormalized.Y
				};
				const Float2 Reflected = { FallingVector.X - Subtrahend.X, FallingVector.Y - Subtrahend.Y };
				const float ReflectedLength = sqrt(Reflected.X * Reflected.X + Reflected.Y * Reflected.Y);
				const Float2 NormalizedReflected = { Reflected.X / ReflectedLength, Reflected.Y / ReflectedLength };
				b.Direction.X = NormalizedReflected.X;
				b.Direction.Y = NormalizedReflected.Y;

				for (int i = 0; i < SEGMENT_COORDS_AMOUNT; ++i) {
					Walls.erase(Walls.begin() + HitCoord);
				}
			}
		}
	}
}

void BulletManager::Fire(const Float2& StartPosistion, const Float2& StartDirection, float Speed, float StartTime, float LifeTime)
{
	const float StartDirectionLenqgth = sqrt(StartDirection.X * StartDirection.X + StartDirection.Y * StartDirection.Y);
	const Float2 StartDirectionNormalized = { StartDirection.X / StartDirectionLenqgth, StartDirection.Y / StartDirectionLenqgth };
	Bullets.push_back({ StartPosistion, StartDirectionNormalized, StartTime, LifeTime, Speed });
}