#pragma once
#include "AABB.h"
#include <memory>
#include <vector>
#include <map>
#include <forward_list>
#include "IAABB.h"

#define AABB_NULL_NODE 0xffffffff

struct AABBNode
{
	AABB aabb;
	std::shared_ptr<IAABB> Object;
	// tree links
	int ParentNodeIndex;
	int LeftNodeIndex;
	int RightNodeIndex;
	// node linked list link
	int NextNodeIndex;

	bool IsLeaf() const { return LeftNodeIndex == AABB_NULL_NODE; }

	AABBNode() : Object(nullptr), ParentNodeIndex(AABB_NULL_NODE), LeftNodeIndex(AABB_NULL_NODE),
	             RightNodeIndex(AABB_NULL_NODE), NextNodeIndex(AABB_NULL_NODE)
	{
	}
};

class AABBTree
{
public:
	AABBTree(int InitialSize);
	~AABBTree();

	bool IsLeaf(const std::shared_ptr<IAABB>& Object) const;
	std::vector<AABB> GetNodes() const;
	void InsertObject(const std::shared_ptr<IAABB>& Object);
	void RemoveObject(const std::shared_ptr<IAABB>& Object);
	void UpdateObject(const std::shared_ptr<IAABB>& Object);
	std::forward_list<std::shared_ptr<IAABB>> QueryOverlaps(const std::shared_ptr<IAABB>& Object) const;

private:
	std::map<std::shared_ptr<IAABB>, int> ObjectNodeToIndex;
	std::vector<AABBNode> Nodes;
	int RootNodeIndex;
	int AllocatedNodeCount;
	int NextFreeNodeIndex;
	int NodeCapacity;
	int GrowthSize;

	int AllocateNode();
	void DeallocateNode(int NodeIndex);
	void InsertLeaf(int LeafNodeIndex);
	void RemoveLeaf(int LeafNodeIndex);
	void UpdateLeaf(int LeafNodeIndex, const AABB& NewAABB);
	void FixUpwardsTree(int TreeNodeIndex);
};
