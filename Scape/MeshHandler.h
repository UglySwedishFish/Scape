/*#pragma once

#include "Shader.h"
#include "FrameBuffer.h"
#include "Camera.h"
#include "Kernel.h"
#include "Kernels/sharedstructs.h"
#include "Pathtracer.h"
#include "Mesh.h"

namespace Scape {
	namespace Rendering {

		struct MeshHandler {


			std::unique_ptr<Assimp::Importer> Importer;

			std::vector<std::vector<Core::Instance>> Instances;
			std::vector<Model> Meshes;
			std::vector<Core::KernelInstance> AllInstances;

			KernelBuffer<KernelModel> KernelInstanceData;

			std::vector<unsigned int> GLInstanceData;
			std::vector<unsigned int> GLInstanceDataPrevious;
			std::vector<bool> RecentlyUpdated;

			PathTracer* CurrentPathTracer = nullptr;


			void PrepareMeshHandler(PathTracer& PathTracer);
			void AddMesh(const std::string& FileName);
			void AddInstance(unsigned short MeshIndex, Vector3f Position, Vector3f Rotation, Vector3f Scale);
			void EditInstance(unsigned short MeshIndex, unsigned short InstanceIndex, Vector3f Position, Vector3f Rotation, Vector3f Scale, bool GenerateModelMatrix = true);
			void UpdateAllInstanceData(unsigned short MeshIndex);
			void DrawAllModelsMaterials(Shader& Shader, int InstanceImage, int BaseImage);
			void DrawAllModelsNoMaterials(int InstanceImage);
			void EndOfFrame();

		};

	}
}*/