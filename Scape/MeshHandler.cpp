/*#include "MeshHandler.h"

namespace Scape {

	namespace Rendering {



		void MeshHandler::PrepareMeshHandler(PathTracer& PathTracer)
		{
			Importer = std::make_unique<Assimp::Importer>();
			CurrentPathTracer = &PathTracer;
		}

		void MeshHandler::AddMesh(const std::string& FileName)
		{

			//if (CurrentPathTracer == nullptr)
			//	return;

			Meshes.push_back(Model(FileName.c_str(), Importer));
			CurrentPathTracer->AddMesh(Meshes[Meshes.size() - 1]);
			Instances.push_back(std::vector<Core::Instance>());
			GLInstanceData.push_back(0);
			glGenTextures(1, &GLInstanceData[GLInstanceData.size() - 1]);

			GLInstanceDataPrevious.push_back(0);
			glGenTextures(1, &GLInstanceDataPrevious[GLInstanceDataPrevious.size() - 1]);

			RecentlyUpdated.push_back(false);

		}

		void MeshHandler::AddInstance(unsigned short MeshIndex, Vector3f Position, Vector3f Rotation, Vector3f Scale)
		{

			if (MeshIndex >= Instances.size())
				return;
			//if (CurrentPathTracer == nullptr)
			//	return;

			Instances[MeshIndex].push_back(Core::Instance{ Core::ModelMatrix(Position,Rotation,Scale),Core::ModelMatrix(Position,Rotation,Scale),Position,Rotation,Scale });

			UpdateAllInstanceData(MeshIndex);

			std::cout << Position.x << ' ' << Position.y << ' ' << Position.z << '\n';

		}

		void MeshHandler::EditInstance(unsigned short MeshIndex, unsigned short InstanceIndex, Vector3f Position, Vector3f Rotation, Vector3f Scale, bool GenerateModelMatrix)
		{

			if (MeshIndex >= Instances.size())
				return;

			if (InstanceIndex >= Instances[MeshIndex].size())
				return;

			//if (CurrentPathTracer == nullptr)
			//	return;

			if (GenerateModelMatrix)
				Instances[MeshIndex][InstanceIndex].ModelMatrixPrevious = Instances[MeshIndex][InstanceIndex].ModelMatrix; 

			Instances[MeshIndex][InstanceIndex] = Core::Instance{ GenerateModelMatrix ? Core::ModelMatrix(Position,Rotation,Scale) : Instances[MeshIndex][InstanceIndex].ModelMatrix, Instances[MeshIndex][InstanceIndex].ModelMatrixPrevious,Position,Rotation,Scale };

			UpdateAllInstanceData(MeshIndex);

		}

		void MeshHandler::UpdateAllInstanceData(unsigned short MeshIndex)
		{

			if (MeshIndex >= Instances.size())
				return;

			//if (CurrentPathTracer == nullptr)
			//	return;


			//hardcoded limit of 2048 instances (yeah, deal with it)
			if (Instances[MeshIndex].size() >= 2048)
				return;

			float* PixelData = new float[Instances[MeshIndex].size() * 16];

			for (int x = 0; x < Instances[MeshIndex].size(); x++) {

				Matrix4f Mat = Instances[MeshIndex][x].ModelMatrix;

				for (int pixel = 0; pixel < 16; pixel++) {
					PixelData[x * 16 + pixel] = Mat[pixel % 4][pixel / 4];
				}
			}



			glBindTexture(GL_TEXTURE_2D, GLInstanceData[MeshIndex]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Instances[MeshIndex].size() * 4, 1, 0, GL_RGBA, GL_FLOAT, PixelData);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);

			float* PixelDataPrevious = new float[Instances[MeshIndex].size() * 16];

			for (int x = 0; x < Instances[MeshIndex].size(); x++) {

				Matrix4f Mat = Instances[MeshIndex][x].ModelMatrixPrevious;

				for (int pixel = 0; pixel < 16; pixel++) {
					PixelDataPrevious[x * 16 + pixel] = Mat[pixel % 4][pixel / 4];
				}
			}



			glBindTexture(GL_TEXTURE_2D, GLInstanceDataPrevious[MeshIndex]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Instances[MeshIndex].size() * 4, 1, 0, GL_RGBA, GL_FLOAT, PixelDataPrevious);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);

			//glFinish();

			AllInstances.clear();

			for (int Meshes = 0; Meshes < Instances.size(); Meshes++) {
				for (int Instance = 0; Instance < Instances[Meshes].size(); Instance++) {
					AllInstances.push_back({ Instances[Meshes][Instance].ModelMatrix, Meshes });
				}
			}

			CurrentPathTracer->RayIntersecter->UpdateIntersectionModels(AllInstances);

			delete[] PixelData;
			delete[] PixelDataPrevious;

		}

		void MeshHandler::DrawAllModelsMaterials(Shader& Shader, int InstanceImage, int BaseImage)
		{

			for (int Mesh = 0; Mesh < Instances.size(); Mesh++) {

				if (Instances[Mesh].size() == 0)
					continue;

				glActiveTexture(GL_TEXTURE0 + InstanceImage);
				glBindTexture(GL_TEXTURE_2D, GLInstanceData[Mesh]);

				glActiveTexture(GL_TEXTURE1 + InstanceImage);
				glBindTexture(GL_TEXTURE_2D, GLInstanceDataPrevious[Mesh]);

				Meshes[Mesh].DrawWithMaterialsInstanced(Shader, BaseImage, Instances[Mesh].size());

			}


		}

		void MeshHandler::DrawAllModelsNoMaterials(int InstanceImage)
		{

			for (int Mesh = 0; Mesh < Instances.size(); Mesh++) {

				if (Instances[Mesh].size() == 0)
					continue;

				glActiveTexture(GL_TEXTURE0 + InstanceImage);
				glBindTexture(GL_TEXTURE_2D, GLInstanceData[Mesh]);

				glActiveTexture(GL_TEXTURE1 + InstanceImage);
				glBindTexture(GL_TEXTURE_2D, GLInstanceDataPrevious[Mesh]);

				Meshes[Mesh].DrawInstanced(Instances[Mesh].size());

			}

		}

		void MeshHandler::EndOfFrame()
		{

			for (int Mesh = 0; Mesh < Instances.size(); Mesh++) {

				if (RecentlyUpdated[Mesh]) {

					RecentlyUpdated[Mesh] = false;

					//we need to re-generate the previous model matrix 

					float* PixelData = new float[Instances[Mesh].size() * 16];

					for (int x = 0; x < Instances[Mesh].size(); x++) {

						Matrix4f Mat = Instances[Mesh][x].ModelMatrix;

						for (int pixel = 0; pixel < 16; pixel++) {
							PixelData[x * 16 + pixel] = Mat[pixel % 4][pixel / 4];
						}
					}



					glBindTexture(GL_TEXTURE_2D, GLInstanceDataPrevious[Mesh]);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Instances[Mesh].size() * 4, 1, 0, GL_RGBA, GL_FLOAT, PixelData);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glBindTexture(GL_TEXTURE_2D, 0);

					//glFinish();

					delete[] PixelData;

				}


			}


		}

	}

}*/