#include "Intersecter.h"
#include <iostream>
#include <fstream>
//#include "ImGui/ImGuizmo.h"

namespace Scape {
	namespace Rendering {

		Intersecter::Intersecter(Queue* QueueSystem, KernelGlobals& GlobalKernelData) :
			GlobalKernelData(GlobalKernelData),
			QueueSystem(QueueSystem)
		{
			//IntersectionKernel.LoadKernel("Kernels/CL/Intersect.cl", GlobalKernelData, "intersect_main");
			//OcclusionKernel.LoadKernel("Kernels/CL/Intersect.cl", GlobalKernelData, "occluded_main");

		}
		Intersecter::Intersecter(bool Cuda)
		{
			UsingCuda = true; 
		}
		void Intersecter::Process(Model& Model)
		{

			BVH.reset(new Scape::BVH(100.0f, 64, true));

			int numfaces = Model.Vertices.size() / 3;

			std::vector<Core::BoundingBox> Bounds(numfaces);

			for (int i = 0; i < Model.Vertices.size(); i += 3) {
				for (int j = 0; j < 3; j++) {
					Bounds[i / 3].ResizeToFit(Model.Vertices[i + j]);
				}

				//std::cout << Bounds[i / 3].Max.x << ' ' << Bounds[i / 3].Max.y << ' ' << Bounds[i / 3].Max.z << '\n'; 
				//std::cout << Bounds[i / 3].Min.x << ' ' << Bounds[i / 3].Min.y << ' ' << Bounds[i / 3].Min.z << '\n';

			}



			BVH->Build(&Bounds[0], Bounds.size());

			if (BVH->GetHeight() > MaxStackSize) {
				std::cout << "BVH is too tall!\n";
				std::cin.get();
			}

			BVHTranslator Translator;
			Translator.Process(*BVH);

			auto NumIndicies = BVH->GetNumIndices();

			std::vector<BVHTranslator::Face> FaceData(NumIndicies);

			int const* Reordering = BVH->GetIndices();

			for (int i = 0; i < NumIndicies; i++) {

				int IndexToLookFor = Reordering[i];

				FaceData[i].idx[0] = Model.Indicies[IndexToLookFor * 3];
				FaceData[i].idx[1] = Model.Indicies[IndexToLookFor * 3 + 1];
				FaceData[i].idx[2] = Model.Indicies[IndexToLookFor * 3 + 2];
				FaceData[i].id = IndexToLookFor;


			}






			Translator.InjectIndices(&FaceData[0]);

			for (auto Node : Translator.nodes_) {

				if (Node.s1.child0 == -1) {
					Node.s1.i0 += TriangleOffset;
					Node.s1.i1 += TriangleOffset;
					Node.s1.i2 += TriangleOffset;

				}
				else {
					Node.s1.child0 += NodeOffset;
					Node.s1.child1 += NodeOffset;
				}


				Nodes.push_back(Node);

			}

			for (auto& Node : Nodes) {

				//std::cout << Node.s0.bounds[0].Max.x << ' ' << Node.s0.bounds[0].Max.y << ' ' << Node.s0.bounds[0].Max.z << '\n';

			}




			BVHData = KernelBuffer<BVHTranslator::Node>::Create(GlobalKernelData, CL_MEM_READ_ONLY, Nodes.size(), Nodes.data());

			QueueSystem->Finish();

			MeshesVector.push_back(KernelMesh(TriangleOffset, NodeOffset, MaterialOffset));

			Meshes = KernelBuffer<KernelMesh>::Create(GlobalKernelData, CL_MEM_READ_ONLY, MeshesVector.size(), &MeshesVector[0]);

			TriangleOffset += Model.Vertices.size();
			NodeOffset += Translator.nodes_.size();
			MaterialOffset += Model.Materials.size();

			Translator.nodes_.clear();

			Stack = KernelBuffer<int>::Create(GlobalKernelData, CL_MEM_READ_WRITE, 50331648 / 4);

		}
		void Intersecter::UpdateIntersectionModels(std::vector<Core::KernelInstance>& ModelVector)
		{

			std::vector<KernelModel> KernelModels = std::vector<KernelModel>(ModelVector.size());

			for (int i = 0; i < ModelVector.size(); i++) {

				KernelModels[i].Mesh = ModelVector[i].Model;

				//Matrix3f RotationMatrix = Matrix3f(ModelVector[i].ModelMatrix);

				Vector3f Position, Rotation, Scale;


				//ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(ModelVector[i].ModelMatrix), glm::value_ptr(Position), glm::value_ptr(Rotation), glm::value_ptr(Scale));

				Matrix3f ScaleMatrix = Matrix3f(Core::ModelMatrix(Vector3f(0.0), Vector3f(0.0), 1.f / Scale));
				Matrix3f RotationMatrix = Matrix3f(ModelVector[i].ModelMatrix);
				RotationMatrix = RotationMatrix * ScaleMatrix * ScaleMatrix;

				KernelModels[i].RotationMatrix1PositionX = { RotationMatrix[0].x,RotationMatrix[0].y,RotationMatrix[0].z,Position.x };
				KernelModels[i].RotationMatrix2PositionY = { RotationMatrix[1].x,RotationMatrix[1].y,RotationMatrix[1].z,Position.y };
				KernelModels[i].RotationMatrix3PositionZ = { RotationMatrix[2].x,RotationMatrix[2].y,RotationMatrix[2].z,Position.z };

				KernelModels[i].ScaleX = Scale.x;
				KernelModels[i].ScaleY = Scale.y;
				KernelModels[i].ScaleZ = Scale.z;


			}

			Models = KernelBuffer<KernelModel>::Create(GlobalKernelData, CL_MEM_READ_ONLY, KernelModels.size(), KernelModels.data());


		}
		void Intersecter::Intersect(KernelBuffer<Ray>& Rays, unsigned int RayCount, KernelBuffer<Intersection>& Hits, KernelBuffer<float4>& Vertices)
		{


			if (Models.Size > 0) {
				unsigned int StackSize = RayCount * MaxStackSize;

				if (StackSize > Stack.Size)
				{
					Stack.Memory = nullptr;
					Stack = KernelBuffer<int>::Create(GlobalKernelData, CL_MEM_READ_WRITE, StackSize * 2);
					Stack.Size = StackSize * 2;
					std::cout << "hmmm...\n";
				}

				auto& Func = this->IntersectionKernel;

				int arg = 0;

				Func.SetArgumentMemory(arg++, BVHData.Memory);

				Func.SetArgumentMemory(arg++, Vertices.Memory);

				Func.SetArgumentMemory(arg++, Rays.Memory);

				Func.SetArgument(arg++, int(RayCount));

				Func.SetArgumentMemory(arg++, Hits.Memory);

				Func.SetArgumentMemory(arg++, Meshes.Memory);

				Func.SetArgumentMemory(arg++, Models.Memory);

				Func.SetArgumentMemory(arg++, Stack.Memory);

				Func.SetArgument(arg++, int(Models.Size));

				size_t localsize = 64;
				size_t globalsize = ((RayCount + 63) / 64) * 64;

				Func.RunKernel(QueueSystem->Get(0), RayCount, localsize);

				QueueSystem->Finish();

			}
		}
		void Intersecter::Occluded(KernelBuffer<Ray>& Rays, unsigned int RayCount, KernelBuffer<int>& Hits)
		{


			unsigned int StackSize = RayCount * MaxStackSize;

			if (StackSize > Stack.Size)
			{
				Stack.Memory = nullptr;
				Stack = KernelBuffer<int>::Create(GlobalKernelData, CL_MEM_READ_WRITE, StackSize);
			}

			auto& Func = this->OcclusionKernel;

			int arg = 0;

			Func.SetArgumentMemory(arg++, BVHData.Memory);

			Func.SetArgumentMemory(arg++, Vertices.Memory);

			Func.SetArgumentMemory(arg++, Rays.Memory);

			Func.SetArgument(arg++, int(RayCount));

			Func.SetArgumentMemory(arg++, Stack.Memory);

			Func.SetArgumentMemory(arg++, Hits.Memory);

			size_t localsize = 64;
			size_t globalsize = ((RayCount + 63) / 64) * 64;

			Func.RunKernel(QueueSystem->Get(0), globalsize, localsize);

			QueueSystem->Finish();
		}
	}
}