#pragma once

#include "LightBakerManager.h"
#include "Camera.h"
#include <memory>
#include <unordered_map>

namespace Scape {

	namespace Rendering {

		struct WorldManager {

			std::unordered_map<int, std::unordered_map<int, std::unique_ptr<Chunk>>> Chunks; 
			std::array<Model, static_cast<int>(Objects::Size)> Models; 
			MultiPassFrameBufferObject DeferredFBO; 
			
			void PrepareWorldManager(Window& Window); 
			void HandleWorldGeneration(Camera & Camera); 
			void RenderWorld(Window& Window, Camera& Camera, CubeMultiPassFrameBufferObject & SkyCube, Shader * OverideShader = nullptr); 

		private: 
			LightBaker LightBaker;
			Generator Generator; 
			Shader EntityDeferredShader; 
			std::vector<const Chunk*> ChunkIterator;
			std::unique_ptr<Assimp::Importer> Importer; 

		};

	}

}