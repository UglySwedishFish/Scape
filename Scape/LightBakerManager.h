#pragma once

#include "Chunks.h"
#include "Mesh.h"
#include <vector>
#include <array>
#include "Kernel.h"
#include "FrameBuffer.h"
#include "Intersecter.h"

namespace Scape {

	namespace Rendering {

		const int LIGHT_BAKING_SAMPLES = 32; //todo: theoretically lightmap rendering can converge in O(log N) time rather than O(N), which is a neat little property that has to be researched further
		const int LIGHT_BAKING_LIGHTING_ZONES = 16; 
		const int MAX_LIGHTMAP_RES = 768; 

		

		struct LightBaker {
			
			friend struct Chunk; 

			struct MeshLightBakingData {

				unsigned short Resolution = LightBakingTextureResolutions[static_cast<int>(LightBakingQuality::HIGH)]; 
				std::vector<Vector3f> Normals = {}, WorldPositions = {};
				MeshLightBakingData() {}
			};

			struct LightMapImageData {
				std::vector<std::vector<Vector3f>> NormalData, WorldPositionData;
				void SetInitialHeight(int Height); 
				void ResizeToFit(int NewResX); 
				void SetPixel(int PixelX, int PixelY, Vector3f Normal, Vector3f WorldPosition); 
			};

			std::vector<Chunk*> Chunks; 
			std::vector<MeshLightBakingData> Data; 

			void PrepareLightBakingSystem();
			void AddToLightBakingQueue(Chunk& Chunk); 
			bool IsInLightBakingQueue(Chunk& Chunk);
			void ConstructBakedMeshData(Model& Model, LightBakingQuality Quality = LightBakingQuality::HIGH);
			void UpdateLightBaking();

		private: 

			void ConstructLightBakingDataImage(Chunk& Chunk); 
			void RayTrace(Chunk& Chunk, unsigned int Width, unsigned int Height); 

			Shader LightMapRayGenerator, LightMapImageCopy, LightMapShadeHandler; 
			MultiPassFrameBufferObject RayGenerationBuffer; 
			Kernel RayTraceKernel; 
			KernelBuffer<float2> UVs; 
			KernelBuffer<float4> Normals, Vertices; 

			std::vector<float2> UVsV; 
			std::vector<float4> NormalsV, VerticesV; 

			KernelGlobals GlobalKernelData;
			Queue QueueSystem;

			KernelInterop2DImageList InputData, OutPutData; 
			
			std::unique_ptr<Intersecter> RayIntersecter; 

			unsigned int SobolTexture, RankingTexture, ScramblingTexture; 



		};


		
	}


}

