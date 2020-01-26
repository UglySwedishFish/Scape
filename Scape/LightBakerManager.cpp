#include "LightBakerManager.h"
#include "External/BlueNoiseData.h"
#include <iostream>
#include "SkyRenderer.h"

namespace Scape {

	namespace Rendering {

		void LightBaker::PrepareLightBakingSystem(SkyRendering& Sky)
		{

			glGenTextures(1, &SobolTexture);
			glBindTexture(GL_TEXTURE_2D, SobolTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 16384, 4, 0, GL_RED, GL_UNSIGNED_BYTE, sobol_256spp_256d);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);

			glGenTextures(1, &RankingTexture);
			glBindTexture(GL_TEXTURE_2D, RankingTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 16384, 8, 0, GL_RED, GL_UNSIGNED_BYTE, rankingTile);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);

			glGenTextures(1, &ScramblingTexture);
			glBindTexture(GL_TEXTURE_2D, ScramblingTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 16384, 8, 0, GL_RED, GL_UNSIGNED_BYTE, scramblingTile);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


			RayGenerationBuffer = MultiPassFrameBufferObject(Vector2i(MAX_LIGHTMAP_RES, LightBakingTextureResolutions[static_cast<int>(LightBakingQuality::TERRAIN)]), 2, { GL_RGBA16F, GL_RGBA32F }, false); 
			LightMapRayGenerator = Shader("Shaders/RayGenerator"); 
			LightMapShadeHandler = Shader("Shaders/HitHandler"); 

			LightMapRayGenerator.Bind();

			LightMapRayGenerator.SetUniform("WorldPosition", 0);
			LightMapRayGenerator.SetUniform("Normal", 1);
			LightMapRayGenerator.SetUniform("Sobol", 2);
			LightMapRayGenerator.SetUniform("Ranking", 3);
			LightMapRayGenerator.SetUniform("Scrambling", 4);

			LightMapRayGenerator.UnBind();

			LightMapShadeHandler.Bind(); 

			LightMapShadeHandler.SetUniform("RayDirection", 0); 
			LightMapShadeHandler.SetUniform("RayOrigin", 1);
			LightMapShadeHandler.SetUniform("RayHitDataUVWM", 2);
			LightMapShadeHandler.SetUniform("RayHitDataNormal", 3);
			LightMapShadeHandler.SetUniform("PreviousImageDataAO", 4);
			LightMapShadeHandler.SetUniform("AOImage", 5);
			LightMapShadeHandler.SetUniform("LightingImages", 6);

			LightMapShadeHandler.SetUniform("MaterialData", 30);
			LightMapShadeHandler.SetUniform("Textures", 7);
			LightMapShadeHandler.SetUniform("PreviousImageDataLighting", 8);
			LightMapShadeHandler.SetUniform("ShadowCount", LIGHT_BAKING_LIGHTING_ZONES-2);

			for (int ShadowMap = 0; ShadowMap < LIGHT_BAKING_LIGHTING_ZONES-2; ShadowMap++) {
				std::string Title = "ShadowMaps[" + std::to_string(ShadowMap) + "]"; 
				LightMapShadeHandler.SetUniform(Title, 10 + ShadowMap); 
			}

			
			LightMapShadeHandler.UnBind(); 


			std::vector<cl::Platform> Platforms;
			cl::Platform::get(&Platforms);

			for (int i = 0; i < Platforms.size(); i++) {
				std::string s = Platforms[i].getInfo<CL_PLATFORM_NAME>();
				std::cout << "Platform name: " << s << "\n";
			}

			std::cout << "Choose platform: ";
			int Platform = 1;

			std::cin >> Platform;


			std::vector<cl::Device> Devices;
			Platforms[Platform].getDevices(CL_DEVICE_TYPE_ALL, &Devices);

			for (int i = 0; i < Devices.size(); i++) {
				std::string s = Devices[i].getInfo<CL_DEVICE_NAME>();
				std::cout << "Device name: " << s << "\n";
			}

			int Device = 0;

			std::cin >> Device;
			GlobalKernelData = KernelGlobals(Platforms[Platform], Devices[Device]);
			QueueSystem.Create(0, GlobalKernelData);

			RayTraceKernel.LoadKernel("Kernels/CL/RayTrace.cl", GlobalKernelData, "RayTrace");

			RayIntersecter = std::make_unique<Intersecter>(&QueueSystem, GlobalKernelData); 
			
			InputData.ReserveImageList(2); 
			for (int i = 0; i < 2; i++)
				InputData.AssignGLImage(i, RayGenerationBuffer.ColorBuffers[i], GlobalKernelData.Context);

			unsigned int Height = LightBakingTextureResolutions[static_cast<int>(LightBakingQuality::TERRAIN)]; //constant! 

			OutPutData.CreateGLImage(GlobalKernelData.Context, { MAX_LIGHTMAP_RES, (int)Height  }, true);
			OutPutData.CreateGLImage(GlobalKernelData.Context, { MAX_LIGHTMAP_RES, (int)Height }, false);

			float TimeMax = 43200. / float(LIGHT_BAKING_LIGHTING_ZONES);

			ShadowMaps.resize(LIGHT_BAKING_LIGHTING_ZONES - 2); 
			ShadowMapOrientations.resize(LIGHT_BAKING_LIGHTING_ZONES - 2); 
			ShadowMapRotations.resize(LIGHT_BAKING_LIGHTING_ZONES - 2); 
			ShadowViewMatrices.resize(LIGHT_BAKING_LIGHTING_ZONES - 2); 

			for (int Zone = 0; Zone < LIGHT_BAKING_LIGHTING_ZONES - 2; Zone++) {
				ShadowMaps[Zone] = FrameBufferObject(Vector2i(LIGHTMAP_SHADOW_RES), GL_R8); 
				Sky.GetTimeOfDayDirection(TimeMax * (Zone + 1), ShadowMapRotations[Zone], ShadowMapOrientations[Zone]); 
			}


		}

		void LightBaker::AddToLightBakingQueue(Chunk& Chunk, Camera& Camera)
		{

			PrevCameraPosition = Camera.Position; 
			ShadowSample = 0; 

			Chunks.push_back(&Chunk); 
			ConstructLightBakingDataImage(Chunk); 
		}

		bool LightBaker::IsInLightBakingQueue(Chunk& Chunk)
		{
			for (auto& LookupChunk : Chunks) {
				if (LookupChunk->PosX == Chunk.PosX && LookupChunk->PosZ == Chunk.PosZ)
					return true; 
	
			}
			return false; 
		}

		void LightBaker::ConstructBakedMeshData(Model& Model, LightBakingQuality Quality)
		{

			RayIntersecter->Process(Model); 

			for (int i = 0; i < Model.ModelData.Normals.size(); i++) {
				UVsV.push_back({ Model.TexCoords[i].x, Model.TexCoords[i].y });
				NormalsV.push_back({ Model.Normals[i].x, Model.Normals[i].y, Model.Normals[i].z, Model.TexCoords[i].z });
				VerticesV.push_back({ Model.Vertices[i].x, Model.Vertices[i].y, Model.Vertices[i].z, Model.Vertices[i].z });

			}
			Normals = KernelBuffer<float4>::Create(GlobalKernelData, CL_MEM_READ_ONLY, NormalsV.size(), NormalsV.data());
			UVs = KernelBuffer<float2>::Create(GlobalKernelData, CL_MEM_READ_ONLY, UVsV.size(), UVsV.data());
			Vertices = KernelBuffer<float4>::Create(GlobalKernelData, CL_MEM_READ_ONLY, VerticesV.size(), VerticesV.data());


			Data.push_back(MeshLightBakingData()); 

			auto& CurrentData = Data[Data.size()-1];

			using namespace glm; 

			//to test this out: we just construct it for an sf::image 

			unsigned int Size = LightBakingTextureResolutions[static_cast<int>(Quality)]; 
			unsigned int SuperSize = Size * 4;

			struct PixelData {

				std::vector<Vector3f> Normals = {};
				std::vector<Vector3f> WorldPositions = {};

			};

			std::vector<std::vector<PixelData>> RawImageData; 
			
			auto CrunchedNormalData = new Vector3f[SuperSize * SuperSize];
			auto CrunchedWorldPosData = new Vector3f[SuperSize * SuperSize];

			RawImageData.resize(SuperSize);
			for (int x = 0; x < SuperSize; x++)
				RawImageData[x].resize(SuperSize);

			CurrentData.Normals.resize(Size* Size); 
			CurrentData.WorldPositions.resize(Size* Size); 
			CurrentData.Resolution = Size; 

			//step 1: raw raster

			for (int TriangleIdx = 0; TriangleIdx < Model.ModelDataLightMapUV.TexCoords.size(); TriangleIdx+=3) {

				std::array<Vector2f, 3> UVs; 
				std::array<Vector3f, 3> Vertices;
				std::array<Vector3f, 3> Normals;

				for (int InnerVertice = 0; InnerVertice < 3; InnerVertice++) {
					UVs[InnerVertice] = Model.ModelDataLightMapUV.TexCoords[TriangleIdx + InnerVertice]; 
					Vertices[InnerVertice] = Model.ModelData.Vertices[TriangleIdx + InnerVertice];
					Normals[InnerVertice] = Model.ModelData.Normals[TriangleIdx + InnerVertice];
				}

				Vector2f BoxMin = min(UVs[0], min(UVs[1], UVs[2])); 
				Vector2f BoxMax = max(UVs[0], max(UVs[1], UVs[2]));

				Vector2i BoxMinPixels = max(floor(BoxMin * Vector2f(SuperSize))-1.f,0.f);
				Vector2i BoxMaxPixels = min(BoxMax * Vector2f(SuperSize) + 1.f, Vector2f(SuperSize));

				if (BoxMinPixels.x == BoxMaxPixels.x) {
					BoxMinPixels.x = BoxMinPixels.x - 1; 
					BoxMaxPixels.x = BoxMaxPixels.x + 1;

					BoxMinPixels.x = max(BoxMinPixels.x, 0); 
					BoxMaxPixels.x = min(BoxMinPixels.x, int(SuperSize));

				}

				if (BoxMinPixels.y == BoxMaxPixels.y) {
					BoxMinPixels.y = BoxMinPixels.y - 1;
					BoxMaxPixels.y = BoxMaxPixels.y + 1;

					BoxMinPixels.y = max(BoxMinPixels.y, 0);
					BoxMaxPixels.y = min(BoxMinPixels.y, int(SuperSize));

				}


				for (int X = BoxMinPixels.x; X < BoxMaxPixels.x; X++) {
					for (int Y = BoxMinPixels.y; Y < BoxMaxPixels.y; Y++) {

						Vector2f Coordinate = Vector2f(X, Y) / Vector2f(SuperSize);

						float Divisor = (UVs[1].y - UVs[2].y) * (UVs[0].x - UVs[2].x) + (UVs[2].x - UVs[1].x) * (UVs[0].y - UVs[2].y); 

						Vector3f BaryCentrics; 
						BaryCentrics.x = ((UVs[1].y - UVs[2].y) * (Coordinate.x - UVs[2].x) + (UVs[2].x - UVs[1].x) * (Coordinate.y - UVs[2].y)) / Divisor;
						BaryCentrics.y = ((UVs[2].y - UVs[0].y) * (Coordinate.x - UVs[2].x) + (UVs[0].x - UVs[2].x) * (Coordinate.y - UVs[2].y)) / Divisor;
						BaryCentrics.z = 1 - BaryCentrics.x - BaryCentrics.y; 

						if (BaryCentrics.x > 0.0 && BaryCentrics.y > 0.0 && BaryCentrics.z > 0.0) {

							Vector3f Normal = Normals[0] * BaryCentrics.x + Normals[1] * BaryCentrics.y + Normals[2] * BaryCentrics.z; 
							Vector3f WorldPos = Vertices[0] * BaryCentrics.x + Vertices[1] * BaryCentrics.y + Vertices[2] * BaryCentrics.z;


							RawImageData[X][Y].Normals.push_back(Normal); 
							RawImageData[X][Y].WorldPositions.push_back(WorldPos); 



						}
						



					}
				}
			}

			//step 2: converting the super-sampled data back to raw image data

			for (int X = 0; X < SuperSize; X++) {
				for (int Y = 0; Y < SuperSize; Y++) {

					Vector3f SuperSampledNormal = Vector3f(0.0), SuperSampledWorldPos = Vector3f(0.0); 
					Vector3f AverageNormal = Vector3f(0.0); 
					float TotalWeight = 0.0; 

					for (auto& Norm : RawImageData[X][Y].Normals) {
						AverageNormal += Norm; 
					}

					AverageNormal /= float(RawImageData[X][Y].Normals.size()); 
					AverageNormal = glm::normalize(AverageNormal); 

					for (int NormIdx = 0; NormIdx < RawImageData[X][Y].Normals.size(); NormIdx++) {
						auto& Norm = RawImageData[X][Y].Normals[NormIdx]; 
						auto& WorldPos = RawImageData[X][Y].WorldPositions[NormIdx]; 
						float Weight = 1.0; 
						SuperSampledNormal += Norm * Weight; 
						SuperSampledWorldPos += WorldPos * Weight; 

						TotalWeight += Weight; 
					}

					SuperSampledNormal = normalize(SuperSampledNormal); 
					SuperSampledWorldPos /= TotalWeight; 
					CrunchedNormalData[X * SuperSize + Y] = SuperSampledNormal;
					CrunchedWorldPosData[X * SuperSize + Y] = SuperSampledWorldPos; 
					

				}


			}

			//step 2.5: making sure the data that has no normals has a value of (0,0,0)

			for (int X = 0; X < SuperSize; X++) {
				for (int Y = 0; Y < SuperSize; Y++) {

					if (RawImageData[X][Y].Normals.size() == 0) {
						CrunchedNormalData[X * SuperSize + Y] = Vector3f(0.0);
						CrunchedWorldPosData[X * SuperSize + Y] = Vector3f(0.0); 
					}
				}
			}


			//step 3: filling the blanks! 

			std::vector<std::vector<PixelData>> Temporary;

			Temporary.resize(SuperSize);
			for (int x = 0; x < SuperSize; x++)
				Temporary[x].resize(SuperSize);
			
			for (int X = 0; X < SuperSize; X++) {
				for (int Y = 0; Y < SuperSize; Y++) {
					Temporary[X][Y].Normals = RawImageData[X][Y].Normals;
					Temporary[X][Y].WorldPositions = RawImageData[X][Y].WorldPositions;
				}
			}

			bool HasBeenUpdated = true; 

			int Iter = 0; 

			while(HasBeenUpdated && Iter++ <= 14) {

				HasBeenUpdated = false; 

				for (int X = 0; X < SuperSize; X++) {

					for (int Y = 0; Y < SuperSize; Y++) {

						if (Temporary[X][Y].Normals.size() > 0)
							continue; 

						Vector3f SuperNormal = Vector3f(0.0), SuperWorldPos = Vector3f(0.0); 
						float TotalDivisor = 0.0; 

						int TotalNormalCount = 0; 

						for (int NeighBorX = X-1; NeighBorX <= X+1; NeighBorX++) {

							for (int NeighBorY = Y - 1; NeighBorY <= Y + 1; NeighBorY++) {

								if (NeighBorX < 0 || NeighBorY < 0 || NeighBorX >= SuperSize || NeighBorY >= SuperSize)
									continue; 

								if (Temporary[NeighBorX][NeighBorY].Normals.size() <= 0)
									continue;

								
								SuperNormal += CrunchedNormalData[NeighBorX * SuperSize + NeighBorY] * float(Temporary[NeighBorX][NeighBorY].Normals.size()); 
								SuperWorldPos += CrunchedWorldPosData[NeighBorX * SuperSize + NeighBorY] * float(Temporary[NeighBorX][NeighBorY].Normals.size());
								TotalNormalCount += Temporary[NeighBorX][NeighBorY].Normals.size();
								TotalDivisor += float(Temporary[NeighBorX][NeighBorY].Normals.size());

							}


						}


						if (TotalNormalCount > 0) {
							HasBeenUpdated = true; 
							SuperNormal = normalize(SuperNormal);
							SuperWorldPos /= TotalDivisor; 

							CrunchedNormalData[X * SuperSize + Y] = SuperNormal;
							CrunchedWorldPosData[X * SuperSize + Y] = SuperWorldPos; 

							RawImageData[X][Y].Normals.push_back(SuperNormal);
							RawImageData[X][Y].WorldPositions.push_back(SuperWorldPos); 
						}

					}

				}


				for (int X = 0; X < SuperSize; X++) {
					for (int Y = 0; Y < SuperSize; Y++) {
						Temporary[X][Y].Normals = RawImageData[X][Y].Normals; 
						Temporary[X][Y].WorldPositions = RawImageData[X][Y].WorldPositions;
					}
				}



			}


			//step 4: crunching down the higher res image to this! 

			for (int X = 0; X < Size; X++) {

				for (int Y = 0; Y < Size; Y++) {


					Vector3f AvgNormal = Vector3f(0.0), SuperNormal = Vector3f(0.0), SuperWorldPos = Vector3f(0.0); 

					float TotalWeight = 0.0; 

					for (int SubPixelX = 0; SubPixelX < 4; SubPixelX++) {

						for (int SubPixelY = 0; SubPixelY < 4; SubPixelY++) {

							AvgNormal += CrunchedNormalData[(X * 4 + SubPixelX) * SuperSize + (Y * 4 + SubPixelY)]; 


						}



					}

					AvgNormal = normalize(AvgNormal); 

					for (int SubPixelX = 0; SubPixelX < 4; SubPixelX++) {

						for (int SubPixelY = 0; SubPixelY < 4; SubPixelY++) {

							Vector3f Norm = CrunchedNormalData[(X * 4 + SubPixelX) * SuperSize + (Y * 4 + SubPixelY)]; 
							Vector3f WorldPos = CrunchedWorldPosData[(X * 4 + SubPixelX) * SuperSize + (Y * 4 + SubPixelY)];

							float Weight = 1.0;
							TotalWeight += Weight; 

							SuperNormal += Norm * Weight;
							SuperWorldPos += WorldPos * Weight; 


						}



					}

					SuperNormal = normalize(SuperNormal);
					SuperWorldPos /= TotalWeight; 

					CurrentData.Normals[X * Size + Y] = SuperNormal; 
					CurrentData.WorldPositions[X * Size + Y] = SuperWorldPos; 
					

				}

			}

			delete[] CrunchedNormalData;
			delete[] CrunchedWorldPosData; 
		}

		void LightBaker::UpdateLightBaking(SkyRendering& Sky, WorldManager& World)
		{

			if (ShadowSample != LIGHT_BAKING_LIGHTING_ZONES - 3) {

				glEnable(GL_DEPTH_TEST); 

				//construct matrix 

				Matrix4f ViewMatrix = Core::ViewMatrix(PrevCameraPosition + ShadowMapOrientations[ShadowSample] * 500.f, Vector3f(ShadowMapRotations[ShadowSample], 0.f)); 

				Camera ShadowCamera; 
				ShadowCamera.View = ViewMatrix; 
				ShadowCamera.Project = Sky.ProjectionMatrices[2]; 

				ShadowMaps[ShadowSample].Bind(); 

				Sky.ShadowDeferred.Bind(); 

				World.RenderWorld(ShadowCamera, Sky.SkyCube, &Sky.ShadowDeferred); 

				Sky.ShadowDeferred.UnBind(); 

				ShadowMaps[ShadowSample].UnBind();
				
				glDisable(GL_DEPTH_TEST);

				ShadowViewMatrices[ShadowSample++] = ViewMatrix; 

				return; 
			}



			if (Chunks.empty())
				return; 
			auto& Chunk = Chunks.front(); 

			std::cout << "we're baking!\n";

			unsigned int Height = LightBakingTextureResolutions[static_cast<int>(LightBakingQuality::TERRAIN)]; //constant! 

			//...


			glDisable(GL_DEPTH_TEST); 

			RayGenerationBuffer.Bind(); 

			glViewport(0, 0, Chunk->LightMapWidth, Height); 

			LightMapRayGenerator.Bind(); 

			LightMapRayGenerator.SetUniform("Frame", Chunk->BakingSampleCount); 

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Chunk->WorldPositionDataImage);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, Chunk->NormalPositionDataImage);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, SobolTexture);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, RankingTexture);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, ScramblingTexture);

			DrawPostProcessQuad(); 

			LightMapRayGenerator.UnBind(); 

			RayGenerationBuffer.UnBind(); 

			glFinish(); 

			RayTrace(*Chunk, Chunk->LightMapWidth, Height); 

			LightMapShadeHandler.Bind(); 

			for (int i = 0; i < LIGHT_BAKING_LIGHTING_ZONES - 2; i++) {
				std::string Title = "ShadowMatrices[" + std::to_string(i) + "]"; 
				std::string Title2 = "LightDirection[" + std::to_string(i) + "]";
				LightMapShadeHandler.SetUniform(Title, Sky.ProjectionMatrices[2] * ShadowViewMatrices[i]); 
				LightMapShadeHandler.SetUniform(Title2, ShadowMapOrientations[i]);

			}

			LightMapShadeHandler.SetUniform("Frame", (Chunk->BakingSampleCount));

			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDisable(GL_CULL_FACE);
			glDisable(GL_BLEND);

			RayGenerationBuffer.BindImage(0, 0); 
			RayGenerationBuffer.BindImage(1, 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, OutPutData.glImageList[0]);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, OutPutData.glImageList[1]);

			glActiveTexture(GL_TEXTURE4); 
			glBindTexture(GL_TEXTURE_2D, Chunk->LightBakingImage); 

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, Chunk->LightBakingImage);
			glBindImageTexture(5, Chunk->LightBakingImage, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk->LightBakingImageGI);

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk->LightBakingImageGI);
			glBindImageTexture(6, Chunk->LightBakingImageGI, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
			
			

			glActiveTexture(GL_TEXTURE30);
			glBindTexture(GL_TEXTURE_2D, GetMaterialContainer());

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D_ARRAY, GetCombinedTextures()[0]);

			

			


			for (int i = 0; i < LIGHT_BAKING_LIGHTING_ZONES - 2; i++)
				ShadowMaps[i].BindDepthImage(i + 10); 

			DrawPostProcessQuad(); 

			LightMapShadeHandler.UnBind(); 

			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			glEnable(GL_DEPTH_TEST); 

			glFinish(); 

			if (++(Chunk->BakingSampleCount) == LIGHT_BAKING_SAMPLES)
				Chunks.erase(Chunks.begin()); 

		}

		void LightBaker::ConstructLightBakingDataImage(Chunk& Chunk)
		{



			std::vector<int> XOffsets;
			XOffsets.resize(LightBakingTextureResolutions[4] / LightBakingTextureResolutions[0]); 


			auto& EntitiesSorted = Chunk.Entities;

			std::sort(EntitiesSorted.begin(), EntitiesSorted.end(),
				[=](const ModelEntity& a, const ModelEntity& b) {
					return this->Data[a.ModelIndex].Resolution > this->Data[b.ModelIndex].Resolution;
				}); 

			
			//pre-determine the resolutions required for the lightmap

			unsigned int Height = LightBakingTextureResolutions[static_cast<int>(LightBakingQuality::TERRAIN)]; //constant! 
			unsigned int CurrentPixelX = Height; //we must reserve TERRAIN x TERRAIN resolution for the lightmap 
			unsigned int CurrentPixelY = 0; 


			unsigned int PreviousResolution = Data[EntitiesSorted[0].ModelIndex].Resolution; 

			for (auto& Offset : XOffsets)
				Offset = CurrentPixelX; 

			LightMapImageData FinalLightMapData; 
			FinalLightMapData.SetInitialHeight(Height); 

			for (auto& Entity : EntitiesSorted) {


				auto& Data = this->Data[Entity.ModelIndex]; 

				unsigned int Resolution = Data.Resolution;

				if (CurrentPixelY + Resolution <= Height) {
					
				}
				else {




					CurrentPixelY = 0;


					int MaxOffset = -1;

					for (int Res = 0; Res < Resolution; Res += LightBakingTextureResolutions[0])
						MaxOffset = glm::max(XOffsets[Res / LightBakingTextureResolutions[0]], MaxOffset);

					CurrentPixelX = MaxOffset;

					//essentially we "pop" back one - and we choose the minimum offset in the x direction (laid on shelf for now!)

					/*int GlobalMimimum = 100000; 
					

					for (int Pop = CurrentPixelY-Resolution; Pop >= 0; Pop -= Resolution) {

						int MaxOffset = -1;

						for (int Res = 0; Res < Resolution; Res += LightBakingTextureResolutions[0])
							MaxOffset = glm::max(XOffsets[(Res+Pop) / LightBakingTextureResolutions[0]], MaxOffset);

						if (MaxOffset < GlobalMimimum) {

							CurrentPixelY = Pop; 
							GlobalMimimum = MaxOffset; 

						}


					}
					CurrentPixelX = GlobalMimimum;*/
					

				}

				//"place" the models pixels here! 

				Entity.LightMapPositionX = CurrentPixelX; 
				Entity.LightMapPositionY = CurrentPixelY; 

				FinalLightMapData.ResizeToFit(CurrentPixelX + Resolution); 

				Matrix3f NormalMatrix = glm::transpose(glm::inverse(Matrix3f(Entity.ModelMatrix))); 


				for (int ModelPixelX = 0; ModelPixelX < Resolution; ModelPixelX++) {

					for (int ModelPixelY = 0; ModelPixelY < Resolution; ModelPixelY++) {

						sf::Color TestColor; 

						Vector3f Normal = NormalMatrix * Data.Normals[ModelPixelX * Resolution + ModelPixelY];
						Vector3f WorldPosition = Vector3f(Entity.ModelMatrix * Vector4f(Data.WorldPositions[ModelPixelX * Resolution + ModelPixelY],1.0f));

						FinalLightMapData.SetPixel(CurrentPixelX + ModelPixelX, CurrentPixelY + ModelPixelY, Normal, WorldPosition); 

					}

				}

				for (int Res = 0; Res < Resolution; Res += LightBakingTextureResolutions[0])
					XOffsets[(CurrentPixelY + Res) / LightBakingTextureResolutions[0]] += Resolution;

				CurrentPixelY += Resolution;

				PreviousResolution = Resolution; 


			}


			auto Width = FinalLightMapData.NormalData[0].size(); 

			
			auto GPUNormalData = new Vector3f[Width * Height];
			auto GPUWorldPosData = new Vector3f[Width * Height]; 

			for (int x = 0; x < Width; x++) {

				for (int y = 0; y < Height; y++) {

					if (x < 128) {

						Vector3f Normal = Vector3f(0.0, 1.0, 0.0); 
						Vector3f Vertice = Vector3f(float(x * CHUNK_SIZE) / 128., 0.0, float(y * CHUNK_SIZE) / 128.); 

						GPUNormalData[y * Width + x] = Normal; 
						GPUWorldPosData[y * Width + x] = Vertice; 

					}
					else {
						GPUNormalData[y * Width + x] = FinalLightMapData.NormalData[y][x];
						GPUWorldPosData[y * Width + x] = FinalLightMapData.WorldPositionData[y][x];
					}
				}

			}
			
			for (auto& Entity : EntitiesSorted) {

				auto& Data = this->Data[Entity.ModelIndex];

				unsigned int Resolution = Data.Resolution;

				Entity.LightMapSize.x = float(Resolution) / float(Width); 
				Entity.LightMapSize.y = float(Resolution) / float(Height); 

				Entity.LightMapTexCoord.x = float(Entity.LightMapPositionX) / float(Width); 
				Entity.LightMapTexCoord.y = float(Entity.LightMapPositionY) / float(Height);

			}
			


			glGenTextures(1, &Chunk.NormalPositionDataImage);

			glBindTexture(GL_TEXTURE_2D, Chunk.NormalPositionDataImage); 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, GPUNormalData); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);


			glGenTextures(1, &Chunk.WorldPositionDataImage);

			glBindTexture(GL_TEXTURE_2D, Chunk.WorldPositionDataImage);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, Width, Height, 0, GL_RGB, GL_FLOAT, GPUWorldPosData);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			Chunk.LightMapWidth = Width; 

			unsigned char* LightMapColors = new unsigned char[Width * Height*4];

			for (int x = 0; x < Width * Height * 4; x++)
				LightMapColors[x] = 255; 

			glGenTextures(1, &Chunk.LightBakingImage); 

			glBindTexture(GL_TEXTURE_2D, Chunk.LightBakingImage);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, LightMapColors);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
			glGetError(); 


			glGenTextures(1, &Chunk.LightBakingImageGI); 
			

			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk.LightBakingImageGI);
			
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, Width, Height, LIGHT_BAKING_LIGHTING_ZONES-2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			
			for (int SubImage = 0; SubImage < LIGHT_BAKING_LIGHTING_ZONES - 2; SubImage++) {
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
					0,
					0, 0, SubImage,
					Width, Height, 1,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					LightMapColors);
			}
			

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MIN_FILTER,
				GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			

			delete[] GPUWorldPosData; 
			delete[] GPUNormalData; 
			delete[] LightMapColors; 


		}

		void LightBaker::RayTrace(Chunk& Chunk, unsigned int Width, unsigned int Height)
		{
			unsigned int RayCount = Width * Height; 

			OutPutData.QueueAcquire(QueueSystem.Get(0));
			InputData.QueueAcquire(QueueSystem.Get(0));

			if (Chunk.KernelModelDataStructure.Size > 0) {




				unsigned int StackSize = RayCount * MaxStackSize;

				if (StackSize > RayIntersecter->Stack.Size)
				{
					RayIntersecter->Stack.Memory = nullptr;
					RayIntersecter->Stack = KernelBuffer<int>::Create(GlobalKernelData, CL_MEM_READ_WRITE, StackSize * 2);
					RayIntersecter->Stack.Size = StackSize * 2;
					std::cout << "hmmm...\n";
				}

				

				int arg = 0;

				RayTraceKernel.SetArgumentMemory(arg++, RayIntersecter->BVHData.Memory);

				RayTraceKernel.SetArgumentMemory(arg++, Vertices.Memory);

				RayTraceKernel.SetArgumentMemory(arg++, UVs.Memory);

				RayTraceKernel.SetArgumentMemory(arg++, Normals.Memory);

				RayTraceKernel.SetArgument(arg++, int(RayCount));

				RayTraceKernel.SetArgumentMemory(arg++, RayIntersecter->Meshes.Memory);

				RayTraceKernel.SetArgumentMemory(arg++, Chunk.KernelModelDataStructure.Memory);

				RayTraceKernel.SetArgumentMemory(arg++, RayIntersecter->Stack.Memory);

				RayTraceKernel.SetArgument(arg++, int(Chunk.KernelModelDataStructure.Size));

				RayTraceKernel.SetArgument(arg++, int(Width));

				RayTraceKernel.SetArgumentMemory(arg++, InputData.clImageMemoryList[0]);

				RayTraceKernel.SetArgumentMemory(arg++, InputData.clImageMemoryList[1]);

				RayTraceKernel.SetArgumentMemory(arg++, OutPutData.clImageMemoryList[0]);

				RayTraceKernel.SetArgumentMemory(arg++, OutPutData.clImageMemoryList[1]);


				size_t localsize = 64;
				size_t globalsize = ((RayCount + 63) / 64) * 64;

				RayTraceKernel.RunKernel(QueueSystem.Get(0), globalsize, localsize);

				QueueSystem.Finish();

			}

			OutPutData.QueueRelease(QueueSystem.Get(0));
			InputData.QueueRelease(QueueSystem.Get(0));

			QueueSystem.Finish();

		}

		void LightBaker::LightMapImageData::SetInitialHeight(int Height)
		{
			NormalData.resize(Height); 
			WorldPositionData.resize(Height); 


		}

		void LightBaker::LightMapImageData::ResizeToFit(int NewResX)
		{

			if (NormalData[0].size() >= NewResX)
				return; 

			for (auto& V : NormalData)
				V.resize(NewResX); 
			for (auto& V : WorldPositionData)
				V.resize(NewResX);

		}

		void LightBaker::LightMapImageData::SetPixel(int PixelX, int PixelY, Vector3f Normal, Vector3f WorldPosition)
		{

			if (PixelX >= NormalData[PixelY].size())
				ResizeToFit(PixelX + 1); 

			NormalData[PixelY][PixelX] = Normal; 
			WorldPositionData[PixelY][PixelX] = WorldPosition; 

		}

	}
}