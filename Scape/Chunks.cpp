#include "Chunks.h"
#include "LightBakerManager.h"

namespace Scape {

	namespace Rendering {




		Chunk::~Chunk()
		{



		}

		Chunk::Chunk(int PosX, int PosZ) : PosX(PosX), PosZ(PosZ)
		{



		}

		void Chunk::Generate(Generator& Generator, LightBaker& Baker)
		{
			Entities = Generator.GetGeneratedModels(PosX, PosZ); 
			Baker.AddToLightBakingQueue(*this);
			CreateModelStructure(Baker.GlobalKernelData);
			GenerateChunkMesh(Generator); 
		}

		void Chunk::DrawChunk(const Shader& ChunkShader, const std::array<Model, static_cast<int>(Objects::Size)>& Models) const
		{

			int CurrentImage = 0; 

			for (auto& ModelIndex : ModelIndicies) {

				glActiveTexture(GL_TEXTURE0); 
				glBindTexture(GL_TEXTURE_2D, EntityData[CurrentImage]); 

				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, LightBakingImage);

				Models[ModelIndex.first].DrawWithMaterialsInstanced(ChunkShader, 2, ModelIndex.second); 

				CurrentImage++; 
			}








		}

		void Chunk::CreateModelStructure(KernelGlobals& GlobalKernelData)
		{
			for (auto& Entity : Entities) {
				ModelIndicies[Entity.ModelIndex]++;
			}
			EntityData.resize(ModelIndicies.size());

			glGenTextures(ModelIndicies.size(), EntityData.data());

			int CurrentImage = 0;

			for (auto& ModelIndex : ModelIndicies) {

				std::vector<float> Data;

				int CurrentPixel = 0; 

				for (auto& Entity : Entities) {

					if (Entity.ModelIndex != ModelIndex.first)
						continue;

					Data.resize(Data.size() + 20);

					Matrix4f Mat = Entity.ModelMatrix;

					for (int pixel = 0; pixel < 16; pixel++) {
						Data[CurrentPixel * 20 + pixel] = Mat[pixel % 4][pixel / 4];
					}

					Data[CurrentPixel * 20 + 16] = Entity.LightMapTexCoord.x; 
					Data[CurrentPixel * 20 + 17] = Entity.LightMapTexCoord.y;
					Data[CurrentPixel * 20 + 18] = Entity.LightMapSize.x;
					Data[CurrentPixel * 20 + 19] = Entity.LightMapSize.y;


					CurrentPixel++; 
				}

				glBindTexture(GL_TEXTURE_2D, EntityData[CurrentImage++]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Data.size()/4, 1, 0, GL_RGBA, GL_FLOAT, Data.data());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glBindTexture(GL_TEXTURE_2D, 0);

			}

			

			std::vector<KernelModel> KernelModels = std::vector<KernelModel>(Entities.size()); 

			for (int i = 0; i < KernelModels.size(); i++) {

				auto& Entity = Entities[i]; 


				KernelModels[i].Mesh = Entity.ModelIndex; 

				Matrix3f ScaleMatrix = Matrix3f(Core::ModelMatrix(Vector3f(0.0), Vector3f(0.0), 1.f / Entity.Scale));
				Matrix3f RotationMatrix = Matrix3f(Entity.ModelMatrix);
				RotationMatrix = RotationMatrix * ScaleMatrix * ScaleMatrix;

				KernelModels[i].RotationMatrix1PositionX = { RotationMatrix[0].x,RotationMatrix[0].y,RotationMatrix[0].z,-Entity.Position.x };
				KernelModels[i].RotationMatrix2PositionY = { RotationMatrix[1].x,RotationMatrix[1].y,RotationMatrix[1].z,-Entity.Position.y };
				KernelModels[i].RotationMatrix3PositionZ = { RotationMatrix[2].x,RotationMatrix[2].y,RotationMatrix[2].z,-Entity.Position.z };

				KernelModels[i].ScaleX = Entity.Scale.x;
				KernelModels[i].ScaleY = Entity.Scale.y;
				KernelModels[i].ScaleZ = Entity.Scale.z;


			}

			KernelModelDataStructure = KernelBuffer<KernelModel>::Create(GlobalKernelData, CL_MEM_READ_ONLY, KernelModels.size(), KernelModels.data());


		}

		void Chunk::GenerateChunkMesh(Generator& Generator)
		{


			




		}






	}

}