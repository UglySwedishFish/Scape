#include "BVHTranslator.h"
#include <cassert>
#include <queue>
#include <iostream>

namespace Scape
{
	void BVHTranslator::Process(BVH& BVH)
	{
		// WARNING: this is crucial in order for the nodes not to migrate in memory as push_back adds nodes
		nodecnt_ = 0;
		max_idx_ = -1;
		int newsize = BVH.m_nodecnt;
		nodes_.resize(newsize);
		extra_.resize(newsize);
		indices_.resize(newsize);
		addresses_.resize(newsize);

		// Check if we have been initialized

		// Process root
		ProcessRootNode(BVH.m_root);

		nodes_.resize(nodecnt_);
		extra_.resize(nodecnt_);
		indices_.resize(nodecnt_);
		addresses_.resize(nodecnt_);


	}

	void BVHTranslator::InjectIndices(Face const* faces)
	{
		for (auto& node : nodes_)
		{
			if (node.s1.child0 == -1)
			{
				auto idx = node.s1.i0;
				node.s1.i0 = faces[idx].idx[0];
				node.s1.i1 = faces[idx].idx[1];
				node.s1.i2 = faces[idx].idx[2];
				node.s1.shape_id = faces[idx].shapeidx;
				node.s1.prim_id = faces[idx].id;
				node.s1.shape_mask = faces[idx].shape_mask;
			}
		}
	}

	BBox CastFromBoundingBox(Core::BoundingBox& Box) {
		BBox ResultingBox;
		ResultingBox.Min = { Box.Min.x, Box.Min.y, Box.Min.z, 0.0 };
		ResultingBox.Max = { Box.Max.x, Box.Max.y, Box.Max.z, 0.0 };


		return ResultingBox;

	}


	int BVHTranslator::ProcessRootNode(BVH::Node const* root)
	{
		// Keep the nodes to process here
		std::queue<std::pair<BVH::Node const*, int> > workqueue;

		workqueue.push(std::make_pair(root, 0));

		while (!workqueue.empty())
		{
			auto current = workqueue.front();
			workqueue.pop();

			Node& node(nodes_[nodecnt_]);
			indices_[nodecnt_] = current.first->index;
			addresses_[nodecnt_] = nodecnt_;
			++nodecnt_;

			if (current.first->index > max_idx_)
			{
				max_idx_ = current.first->index;
			}

			if (current.first->type == BVH::NodeType::kInternal)
			{
				node.s0.bounds[0] = CastFromBoundingBox(current.first->lc->bounds);
				node.s0.bounds[1] = CastFromBoundingBox(current.first->rc->bounds);
				workqueue.push(std::make_pair(current.first->lc, nodecnt_));
				workqueue.push(std::make_pair(current.first->rc, -nodecnt_));
			}
			else
			{
				node.s1.child0 = node.s1.child1 = -1;
				node.s1.i0 = current.first->startidx;
			}

			if (current.second > 0)
			{
				nodes_[current.second - 1].s1.child0 = nodecnt_ - 1;
			}
			else if (current.second < 0)
			{
				nodes_[-current.second - 1].s1.child1 = nodecnt_ - 1;
			}
		}

		return 0;
	}
}
