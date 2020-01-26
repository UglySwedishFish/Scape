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

		void Chunk::Generate(Generator& Generator, LightBaker& Baker, Camera& Camera)
		{
			Entities = Generator.GetGeneratedModels(PosX, PosZ); 
			GenerateChunkMesh(Generator); 
			Baker.AddToLightBakingQueue(*this, Camera);
			CreateModelStructure(Baker.GlobalKernelData);
		}

		void Chunk::DrawChunk(const Shader& ChunkShader, const std::array<Model, static_cast<int>(Objects::Size)>& Models) const
		{

			int CurrentImage = 0; 

			for (auto& ModelIndex : ModelIndicies) {

				glActiveTexture(GL_TEXTURE0); 
				glBindTexture(GL_TEXTURE_2D, EntityData[CurrentImage]); 

				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, LightBakingImage);

				glActiveTexture(GL_TEXTURE9);
				glBindTexture(GL_TEXTURE_2D_ARRAY, LightBakingImageGI);


				Models[ModelIndex.first].DrawWithMaterialsInstanced(ChunkShader, 2, ModelIndex.second); 

				CurrentImage++; 
			}








		}

		void Chunk::DrawChunkTerrain(const Shader& ChunkShader) const
		{

			ChunkShader.SetUniform("LightMapWidth", (int)LightMapWidth); 

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, LightBakingImage);

			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D_ARRAY, LightBakingImageGI);

			glBindVertexArray(TerrainVAO);

			glDrawElements(GL_TRIANGLES, VertexCount, GL_UNSIGNED_INT, nullptr);

			glBindVertexArray(0);


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

			Vector3f Normal = Vector3f(0.0, 1.0, 0.0); 
			Vector3f Tangent = Vector3f(1.0, 0.0, 0.0); 

			std::array<Vector3f, 4> QuadVertices = { Vector3f(0.0,0.0,0.0), Vector3f(1.0,0.0,0.0), Vector3f(1.0,0.0,1.0), Vector3f(0.0,0.0,1.0) }; 
			std::array<unsigned char, 6> TriangleIndicies = { 0,1,2,2,3,0 }; 


			std::vector<Vector3f> Normals, Tangents, Vertices;
			std::vector<Vector2f> UVs, LightMapUVs; 
			std::vector<unsigned int> Indices;

			unsigned int CurrentIndex = 0; 

			LightMapData.resize(CHUNK_SIZE * CHUNK_SIZE);

			for (int BlockX = 0; BlockX < CHUNK_SIZE; BlockX++) {

				for (int BlockY = 0; BlockY < CHUNK_SIZE; BlockY++) {
						
					//the two triangles! 

					for (int Index = 0; Index < 6; Index++) {

						unsigned char SubIndex = TriangleIndicies[Index]; 

						Vector2f UV = Vector2f(QuadVertices[SubIndex].x, QuadVertices[SubIndex].z);
						Vector3f Vertice = Vector3f(QuadVertices[SubIndex]) + Vector3f(BlockX, 0.0, BlockY);
						Vector2f LightMapUV = (UV + Vector2f(BlockX, BlockY)) / Vector2f(CHUNK_SIZE); 

						Normals.push_back(Normal); 
						Tangents.push_back(Tangent); 
						LightMapUVs.push_back(LightMapUV); 
						UVs.push_back(UV); 
						Vertices.push_back(Vertice); 
						Indices.push_back(CurrentIndex++);

						LightMapData[BlockX * CHUNK_SIZE + BlockY] = Vector4f(Normal, 0.0); 

						VertexCount++; 

					}
					
				}
				

			}

			glGenVertexArrays(1, &TerrainVAO); 
			glBindVertexArray(TerrainVAO); 

			glGenBuffers(6, TerrainVBO); 

			glBindBuffer(GL_ARRAY_BUFFER, TerrainVBO[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, TerrainVBO[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(UVs[0]) * UVs.size(), &UVs[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, TerrainVBO[3]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(LightMapUVs[0]) * LightMapUVs.size(), &LightMapUVs[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, TerrainVBO[4]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, TerrainVBO[5]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Tangents[0]) * Tangents.size(), &Tangents[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);


			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TerrainVBO[0]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

			glBindVertexArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			
		}

	}

}