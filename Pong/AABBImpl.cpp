#include "AABBImpl.h"
#include "AABB.h"

AABB AABBImpl::GetAABB() const {
	return { MinX, MinY, MaxX, MaxY };
}