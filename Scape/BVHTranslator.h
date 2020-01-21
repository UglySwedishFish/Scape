#pragma once
/**********************************************************************
Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
#ifndef FATNODE_BVH_TRANSLATOR_H
#define FATNODE_BVH_TRANSLATOR_H

#include <map>
#include "BVH.h"

namespace Scape
{
	/// Fatnode translator transforms regular binary BVH into the form where:
	/// * Each node contains bounding boxes of its children
	/// * Both children follow parent node in the layuot (breadth first)
	/// * No parent informantion is stored for the node => stacked traversal only
	///
	class BVHTranslator
	{
	public:
		struct Face
		{
			// Up to 3 indices
			int idx[3];
			// Shape index
			int shapeidx;
			// Primitive ID within the mesh
			int id;
			// Shape mask
			int shape_mask;
		};

		// Constructor
		BVHTranslator()
			: nodecnt_(0)
			, root_(0)
		{
		}

		// Fat BVH node
		// Encoding:
		// xbound.pmin.w == -1.f if x-child is an internal node otherwise triangle index
		//

		struct smallnode {

			BBox bounds[2];

		};

		struct Node
		{
			union
			{
				struct
				{
					// Node's bounding box
					BBox bounds[2];
				}s0;

				struct
				{
					// If node is a leaf we keep vertex indices here
					int i0, i1, i2;
					// Address of a left child
					int child0;
					// Shape mask
					int shape_mask;
					// Shape ID
					int shape_id;
					// Primitive ID
					int prim_id;
					// Address of a right child
					int child1;
					
					int Padding[8]; 

				}s1;
			};

			Node()
				: s0()
			{

			}
		};

		void Flush();
		void Process(BVH& BVH);
		void InjectIndices(Face const* faces);
		//void Process(BVH const** BVHs, int const* offsets, int numBVHs);
		//void UpdateTopLevel(BVH const& BVH);

		std::vector<Node> nodes_;
		std::vector<int> extra_;
		std::vector<int> roots_;
		std::vector<int> indices_;
		std::vector<int> addresses_;
		int nodecnt_;
		int root_;
		int max_idx_;

	private:
		int ProcessRootNode(BVH::Node const* node);
		//int ProcessNode(BVH::Node const* n, int offset);

		BVHTranslator(BVHTranslator const&);
		BVHTranslator& operator =(BVHTranslator const&);
	};
}


#endif // PLAIN_BVH_TRANSLATOR_H