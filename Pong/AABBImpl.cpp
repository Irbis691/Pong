#include "AABBImpl.h"
#include "AABB.h"

AABB AABBImpl::getAABB() const {
	return { minX, minY, maxX, maxY };
}