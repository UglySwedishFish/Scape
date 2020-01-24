#include "WorldManager.h"
#include "LightBakerManager.h"

namespace Scape {

	namespace Rendering {



		void WorldManager::PrepareWorldManager(Window& Window)
		{
			Importer = std::make_unique<Assimp::Importer>(); 
			DeferredFBO = MultiPassFrameBufferObject(Window.GetResolution(), 4, { GL_RGBA16F, GL_RGBA16F, GL_RGBA32F, GL_RGB16F }); 
			EntityDeferredShader = Shader("Shaders/EntityDeferred"); 

			EntityDeferredShader.Bind();

			EntityDeferredShader.SetUniform("InstanceData", 0);
			EntityDeferredShader.SetUniform("AlbedoMap", 2);
			EntityDeferredShader.SetUniform("NormalMap", 3);
			EntityDeferredShader.SetUniform("RoughnessMap", 4);
			EntityDeferredShader.SetUniform("MetalnessMap", 5);
			EntityDeferredShader.SetUniform("EmissiveMap", 6);
			EntityDeferredShader.SetUniform("LightMap", 7);
			EntityDeferredShader.SetUniform("Sky", 8);
			EntityDeferredShader.SetUniform("LightMapGI", 9);

			EntityDeferredShader.UnBind();

			Generator.PrepareGenerator(); 

			int ModelIdx = 0; 

			for (auto& ModelID : ObjectLocations) {
				std::string ModelPath = "Models/" + std::get<0>(ModelID); 
				std::string ModelLightMapUVPath = "Models/" + std::get<1>(ModelID); 
				Models[ModelIdx] = Model(ModelPath.c_str(), ModelLightMapUVPath.c_str(), Importer); 
				LightBaker.ConstructBakedMeshData(Models[ModelIdx], ObjectLightMapQuality[ModelIdx++]); 
			}
			UpdateMaterialContainer();
			ConstructCombinedTextures(); 

		}

		void WorldManager::HandleWorldGeneration(Camera& Camera)
		{
			
			bool HasGenerated = false; 

			if (Chunks.find(0) == Chunks.end()) {
				if (Chunks[0].find(0) == Chunks[0].end()) {
					Chunks[0][0] = std::make_unique<Chunk>(0,0); 
					Chunks[0][0]->Generate(Generator, LightBaker, Camera); 
					HasGenerated = true; 
				}

			}

			if (HasGenerated) {
				ChunkIterator.clear(); 

				for (auto &x : Chunks) {

					for (auto &y : x.second) {
						ChunkIterator.push_back(y.second.get()); 
					}

				}


			}
			

		}

		void WorldManager::SetSunDetail(Vector4f Detail)
		{
			SunDetail = Detail; 
		}

		void WorldManager::RenderWorld(Camera& Camera, CubeMultiPassFrameBufferObject& SkyCube, Shader* OverideShader)
		{

			Shader* CurrentShader = &EntityDeferredShader;
			if (OverideShader != nullptr) {
				CurrentShader = OverideShader; 
				//std::cout << "Using custom shader!\n"; 
			}



			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

			//glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y); 




			CurrentShader->Bind();
			CurrentShader->SetUniform("IdentityMatrix", Camera.Project * Camera.View);
			CurrentShader->SetUniform("TimeOfDay", SunDetail.w);
			CurrentShader->SetUniform("SunColor", Vector3f(SunDetail));
			CurrentShader->SetUniform("LightingZones", LIGHT_BAKING_LIGHTING_ZONES);

			glActiveTexture(GL_TEXTURE8); 
			glBindTexture(GL_TEXTURE_CUBE_MAP, SkyCube.Texture[1]);
			



			for (int ChunkIdx = 0; ChunkIdx < ChunkIterator.size(); ChunkIdx++) {
				ChunkIterator[ChunkIdx]->DrawChunk(*CurrentShader, Models);
			}

			CurrentShader->UnBind();


		}

	}

}