#pragma once

#include "Generator.h"
#include <vector>
#include "Kernel.h"
#include "Kernels/sharedstructs.h"
#include <set>
#include "Mesh.h"
#include "Camera.h"

namespace Scape {
	namespace Rendering {



		enum class LightBakingStatus { InActive, Active, Finished };


		struct Chunk {

			friend struct LightBaker; 

			//each chunk is a model manager -> why? cuz fuck u, thats why (also means frustum culling becomes super easy -> still only a few draw calls per chunk) 

			int PosX = 0, PosZ = 0; 

			~Chunk(); 
			Chunk(int PosX, int PosZ); 
			Chunk() {}
			void Generate(Generator & Generator, struct LightBaker & Baker, Camera & Camera); 
			void DrawChunk(const Shader & ChunkShader, const std::array<Model, static_cast<int>(Objects::Size)> & Models) const;


			std::vector<ModelEntity> Entities = {};


		private: 

			//light baking data: 
			LightBakingStatus BakingStatus = LightBakingStatus::InActive;
			unsigned char BakingSampleCount = 0;
			unsigned int LightBakingImage = 0, LightBakingImageGI = 0;
			unsigned int WorldPositionDataImage = 0, NormalPositionDataImage = 0; 
			unsigned int LightMapWidth = 0; 
			std::vector<unsigned int> EntityData; 
			std::unordered_map<int,int> ModelIndicies; 

			KernelBuffer<KernelModel> KernelModelDataStructure; 


			void CreateModelStructure(KernelGlobals& GlobalKernelData);
			void GenerateChunkMesh(Generator& Generator);


		};




	}
}


