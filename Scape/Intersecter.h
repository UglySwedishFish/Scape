#pragma once
#include "Window.h"
#include "BVHTranslator.h"
#include "Kernel.h"
#include "Mesh.h"

static int const MaxStackSize = 48;
static int const MaxBatchSize = 2048 * 2048;

namespace Scape {
	namespace Rendering {



		struct Intersecter {

			bool UsingCuda = false; 

			KernelBuffer<float3> Vertices;
			KernelBuffer<BVHTranslator::Node> BVHData;
			KernelBuffer<int> Stack;
			KernelBuffer<KernelMesh> Meshes;
			KernelBuffer<KernelModel> Models;
			Kernel IntersectionKernel, OcclusionKernel;

			KernelGlobals GlobalKernelData;
			Queue* QueueSystem;

			std::unique_ptr<BVH> BVH;

			std::vector<BVHTranslator::Node> Nodes;
			std::vector<KernelMesh> MeshesVector;

			unsigned int TriangleOffset = 0, NodeOffset = 0, MaterialOffset = 0;

			Intersecter(Queue* QueueSystem, KernelGlobals& GlobalKernelData);
			Intersecter(bool Cuda); 
			void Process(Model& Model);
			void UpdateIntersectionModels(std::vector<Core::KernelInstance>& ModelVector);
			void Intersect(KernelBuffer<Ray>& Rays, unsigned int RayCount, KernelBuffer<Intersection>& Hits, KernelBuffer<float4>& Vertices);
			void Occluded(KernelBuffer<Ray>& Rays, unsigned int RayCount, KernelBuffer<int>& Hits);

		};
	}
}
