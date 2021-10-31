#include "AABBTree.h"
#include <cassert>
#include <stack>
#include "IAABB.h"

AABBTree::AABBTree(int InitialSize) : RootNodeIndex(AABB_NULL_NODE), AllocatedNodeCount(0),
                                      NextFreeNodeIndex(0), NodeCapacity(InitialSize), GrowthSize(InitialSize)
{
	Nodes.resize(InitialSize);
	for (int NodeIndex = 0; NodeIndex < InitialSize; NodeIndex++)
	{
		AABBNode& Node = Nodes[NodeIndex];
		Node.NextNodeIndex = NodeIndex + 1;
	}
	Nodes[InitialSize - 1].NextNodeIndex = AABB_NULL_NODE;
}

AABBTree::~AABBTree()
{
}

std::vector<AABB> AABBTree::GetNodes() const
{
	std::vector<AABB> ResultNodes;
	for (auto It = ObjectNodeToIndex.begin(); It != ObjectNodeToIndex.end(); ++It)
	{
		if (Nodes[It->second].IsLeaf())
		{
			ResultNodes.push_back((*It->first).GetAABB());
		}
	}
	return ResultNodes;
}

int AABBTree::AllocateNode()
{
	// if we have no free tree GetNodes then grow the pool
	if (NextFreeNodeIndex == AABB_NULL_NODE)
	{
		assert(AllocatedNodeCount == NodeCapacity);

		NodeCapacity += GrowthSize;
		Nodes.resize(NodeCapacity);
		for (int NodeIndex = AllocatedNodeCount; NodeIndex < NodeCapacity; NodeIndex++)
		{
			AABBNode& Node = Nodes[NodeIndex];
			Node.NextNodeIndex = NodeIndex + 1;
		}
		Nodes[NodeCapacity - 1].NextNodeIndex = AABB_NULL_NODE;
		NextFreeNodeIndex = AllocatedNodeCount;
	}

	int NodeIndex = NextFreeNodeIndex;
	AABBNode& AllocatedNode = Nodes[NodeIndex];
	AllocatedNode.ParentNodeIndex = AABB_NULL_NODE;
	AllocatedNode.LeftNodeIndex = AABB_NULL_NODE;
	AllocatedNode.RightNodeIndex = AABB_NULL_NODE;
	NextFreeNodeIndex = AllocatedNode.NextNodeIndex;
	AllocatedNodeCount++;

	return NodeIndex;
}

void AABBTree::DeallocateNode(int NodeIndex)
{
	AABBNode& DeallocatedNode = Nodes[NodeIndex];
	DeallocatedNode.NextNodeIndex = NextFreeNodeIndex;
	NextFreeNodeIndex = NodeIndex;
	AllocatedNodeCount--;
}

void AABBTree::InsertObject(const std::shared_ptr<IAABB>& Object)
{
	int NodeIndex = AllocateNode();
	AABBNode& Node = Nodes[NodeIndex];

	Node.aabb = Object->GetAABB();
	Node.Object = Object;

	InsertLeaf(NodeIndex);
	ObjectNodeToIndex[Object] = NodeIndex;
}

void AABBTree::RemoveObject(const std::shared_ptr<IAABB>& Object)
{
	int NodeIndex = ObjectNodeToIndex[Object];
	RemoveLeaf(NodeIndex);
	DeallocateNode(NodeIndex);
	ObjectNodeToIndex.erase(Object);
}

std::forward_list<std::shared_ptr<IAABB>> AABBTree::QueryOverlaps(const std::shared_ptr<IAABB>& Object) const
{
	std::forward_list<std::shared_ptr<IAABB>> Overlaps;
	std::stack<int> Stack;
	AABB TestAabb = Object->GetAABB();

	Stack.push(RootNodeIndex);
	while (!Stack.empty())
	{
		int NodeIndex = Stack.top();
		Stack.pop();

		if (NodeIndex == AABB_NULL_NODE)
		{
			continue;
		}

		const AABBNode& Node = Nodes[NodeIndex];
		if (Node.aabb.Overlaps(TestAabb))
		{
			if (Node.IsLeaf() && Node.Object != Object)
			{
				Overlaps.push_front(Node.Object);
			}
			else
			{
				Stack.push(Node.LeftNodeIndex);
				Stack.push(Node.RightNodeIndex);
			}
		}
	}

	return Overlaps;
}

void AABBTree::InsertLeaf(int LeafNodeIndex)
{
	// make sure we're inserting a new leaf
	assert(Nodes[LeafNodeIndex].ParentNodeIndex == AABB_NULL_NODE);
	assert(Nodes[LeafNodeIndex].LeftNodeIndex == AABB_NULL_NODE);
	assert(Nodes[LeafNodeIndex].RightNodeIndex == AABB_NULL_NODE);

	// if the tree is empty then we make the root the leaf
	if (RootNodeIndex == AABB_NULL_NODE)
	{
		RootNodeIndex = LeafNodeIndex;
		return;
	}

	// search for the best place to put the new leaf in the tree
	// we use surface area and depth as search heuristics
	int TreeNodeIndex = RootNodeIndex;
	AABBNode& LeafNode = Nodes[LeafNodeIndex];
	while (!Nodes[TreeNodeIndex].IsLeaf())
	{
		// because of the test in the while loop above we know we are never a leaf inside it
		const AABBNode& TreeNode = Nodes[TreeNodeIndex];
		int LeftNodeIndex = TreeNode.LeftNodeIndex;
		int RightNodeIndex = TreeNode.RightNodeIndex;
		const AABBNode& LeftNode = Nodes[LeftNodeIndex];
		const AABBNode& RightNode = Nodes[RightNodeIndex];

		const AABB CombinedAABB = TreeNode.aabb.Merge(LeafNode.aabb);

		float NewParentNodeCost = 2.0f * CombinedAABB.SurfaceArea;
		float MinimumPushDownCost = 2.0f * (CombinedAABB.SurfaceArea - TreeNode.aabb.SurfaceArea);

		// use the costs to figure out whether to create a new parent here or descend
		float CostLeft;
		float CostRight;
		if (LeftNode.IsLeaf())
		{
			CostLeft = LeafNode.aabb.Merge(LeftNode.aabb).SurfaceArea + MinimumPushDownCost;
		}
		else
		{
			AABB NewLeftAabb = LeafNode.aabb.Merge(LeftNode.aabb);
			CostLeft = (NewLeftAabb.SurfaceArea - LeftNode.aabb.SurfaceArea) + MinimumPushDownCost;
		}
		if (RightNode.IsLeaf())
		{
			CostRight = LeafNode.aabb.Merge(RightNode.aabb).SurfaceArea + MinimumPushDownCost;
		}
		else
		{
			AABB NewRightAABB = LeafNode.aabb.Merge(RightNode.aabb);
			CostRight = (NewRightAABB.SurfaceArea - RightNode.aabb.SurfaceArea) + MinimumPushDownCost;
		}

		// if the cost of creating a new parent node here is less than descending in either direction then
		// we know we need to create a new parent node, errrr, here and attach the leaf to that
		if (NewParentNodeCost <= CostLeft && NewParentNodeCost <= CostRight)
		{
			break;
		}

		// otherwise descend in the cheapest direction
		if (CostLeft < CostRight)
		{
			TreeNodeIndex = LeftNodeIndex;
		}
		else
		{
			TreeNodeIndex = RightNodeIndex;
		}
	}

	// the leafs sibling is going to be the node we found above and we are going to create a new
	// parent node and attach the leaf and this item
	const int LeafSiblingIndex = TreeNodeIndex;
	AABBNode& LeafSibling = Nodes[LeafSiblingIndex];
	const int OldParentIndex = LeafSibling.ParentNodeIndex;
	const int NewParentIndex = AllocateNode();
	AABBNode& NewParent = Nodes[NewParentIndex];
	NewParent.ParentNodeIndex = OldParentIndex;
	NewParent.aabb = LeafNode.aabb.Merge(LeafSibling.aabb);
	// the new parents aabb is the leaf aabb combined with it's siblings aabb
	NewParent.LeftNodeIndex = LeafSiblingIndex;
	NewParent.RightNodeIndex = LeafNodeIndex;
	LeafNode.ParentNodeIndex = NewParentIndex;
	LeafSibling.ParentNodeIndex = NewParentIndex;

	if (OldParentIndex == AABB_NULL_NODE)
	{
		// the old parent was the root and so this is now the root
		RootNodeIndex = NewParentIndex;
	}
	else
	{
		// the old parent was not the root and so we need to patch the left or right index to
		// point to the new node
		AABBNode& OldParent = Nodes[OldParentIndex];
		if (OldParent.LeftNodeIndex == LeafSiblingIndex)
		{
			OldParent.LeftNodeIndex = NewParentIndex;
		}
		else
		{
			OldParent.RightNodeIndex = NewParentIndex;
		}
	}

	// finally we need to walk back up the tree fixing heights and areas
	TreeNodeIndex = LeafNode.ParentNodeIndex;
	FixUpwardsTree(TreeNodeIndex);
}

void AABBTree::RemoveLeaf(int LeafNodeIndex)
{
	// if the leaf is the root then we can just clear the root pointer and return
	if (LeafNodeIndex == RootNodeIndex)
	{
		RootNodeIndex = AABB_NULL_NODE;
		return;
	}

	AABBNode& LeafNode = Nodes[LeafNodeIndex];
	int ParentNodeIndex = LeafNode.ParentNodeIndex;
	const AABBNode& ParentNode = Nodes[ParentNodeIndex];
	int GrandParentNodeIndex = ParentNode.ParentNodeIndex;
	int SiblingNodeIndex = ParentNode.LeftNodeIndex == LeafNodeIndex
		                       ? ParentNode.RightNodeIndex
		                       : ParentNode.LeftNodeIndex;
	assert(SiblingNodeIndex != AABB_NULL_NODE); // we must have a sibling
	AABBNode& SiblingNode = Nodes[SiblingNodeIndex];

	if (GrandParentNodeIndex != AABB_NULL_NODE)
	{
		// if we have a grand parent (i.e. the parent is not the root) then destroy the parent and connect the sibling to the grandparent in its
		// place
		AABBNode& GrandParentNode = Nodes[GrandParentNodeIndex];
		if (GrandParentNode.LeftNodeIndex == ParentNodeIndex)
		{
			GrandParentNode.LeftNodeIndex = SiblingNodeIndex;
		}
		else
		{
			GrandParentNode.RightNodeIndex = SiblingNodeIndex;
		}
		SiblingNode.ParentNodeIndex = GrandParentNodeIndex;
		DeallocateNode(ParentNodeIndex);

		FixUpwardsTree(GrandParentNodeIndex);
	}
	else
	{
		// if we have no grandparent then the parent is the root and so our sibling becomes the root and has it's parent removed
		RootNodeIndex = SiblingNodeIndex;
		SiblingNode.ParentNodeIndex = AABB_NULL_NODE;
		DeallocateNode(ParentNodeIndex);
	}

	LeafNode.ParentNodeIndex = AABB_NULL_NODE;
}

void AABBTree::FixUpwardsTree(int TreeNodeIndex)
{
	while (TreeNodeIndex != AABB_NULL_NODE)
	{
		AABBNode& TreeNode = Nodes[TreeNodeIndex];

		// every node should be a parent
		assert(TreeNode.LeftNodeIndex != AABB_NULL_NODE && TreeNode.RightNodeIndex != AABB_NULL_NODE);

		// fix height and area
		AABBNode& LeftNode = Nodes[TreeNode.LeftNodeIndex];
		AABBNode& RightNode = Nodes[TreeNode.RightNodeIndex];
		TreeNode.aabb = LeftNode.aabb.Merge(RightNode.aabb);
		LeftNode.ParentNodeIndex = TreeNodeIndex;
		RightNode.ParentNodeIndex = TreeNodeIndex;
		TreeNodeIndex = TreeNode.ParentNodeIndex;
	}
}
