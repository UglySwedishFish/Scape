#include "MinifoliageBaker.h"
#include <fstream>
#include <sstream>


//we do not particularily care for storage when it comes to caching the geometry for the baker
//since its a lot less than the actual cached results 

//So here's the syntax: 

//Type: [CIRCLE/RECTANGLE] Position: [X] [Y] Size: [X] [Y] Rotation [Axis] Shift [X] [Y] 

namespace Scape {

	namespace Rendering {

		void FoliageBaker::BakeFoliage(const std::string& FileName)
		{
			Shader FoliageBakerShader = Shader("Shaders/FoliageBaker"); 
			FrameBufferObject FoliageOutPutBuffer = FrameBufferObject(Vector2i(FoliageResolution * FoliageDirections, FoliageResolution), GL_RGBA8, false); 
		

			Vector3f* FoliageData = new Vector3f[Geometry.size()*3];

			for (int GeometryIdx = 0; GeometryIdx < Geometry.size(); GeometryIdx++) {

				auto& Geometry = this->Geometry[GeometryIdx]; 

				FoliageData[GeometryIdx * 3] = Vector3f(static_cast<int>(Geometry.Type), Geometry.Size);
				FoliageData[GeometryIdx * 3 + 1] = Vector3f(Geometry.RotationPosition);
				FoliageData[GeometryIdx * 3 + 2] = Vector3f(Geometry.Shift, 0.0);

			}


			unsigned int FoliageDataTexture;
			glGenTextures(1, &FoliageDataTexture);

			glBindTexture(GL_TEXTURE_1D, FoliageDataTexture);
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, Geometry.size() * 3, 0, GL_RGB, GL_FLOAT, FoliageData);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_1D, 0);

			glFinish();

			float TimeAddon = 6.28318531 / 16.f; 

			unsigned char* Pixels = new unsigned char[FoliageDirections * FoliageResolution * FoliageResolution * 4];

			for (int i = 0; i < 16; i++) {


				


				FoliageBakerShader.Bind();

				FoliageOutPutBuffer.Bind();

				FoliageBakerShader.SetUniform("TextureSize", FoliageResolution);
				FoliageBakerShader.SetUniform("RayCount", FoliageDirections);
				FoliageBakerShader.SetUniform("MinStepSize", MinStepSize);
				FoliageBakerShader.SetUniform("MaxStepLength", MaxLenght);
				FoliageBakerShader.SetUniform("Primitives", 0);
				FoliageBakerShader.SetUniform("PrimitiveCount", static_cast<int>(Geometry.size()));
				FoliageBakerShader.SetUniform("Time", TimeAddon * i);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_1D, FoliageDataTexture);

				DrawPostProcessQuad();

				FoliageOutPutBuffer.UnBind();

				FoliageBakerShader.UnBind();

				glFinish();


				glBindTexture(GL_TEXTURE_2D, FoliageOutPutBuffer.ColorBuffer);
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
				glBindTexture(GL_TEXTURE_2D, 0);

				glFinish();

				sf::Image* OutPutImage = new sf::Image();
				OutPutImage->create(FoliageResolution * FoliageDirections, FoliageResolution, Pixels);
				OutPutImage->saveToFile(FileName + std::to_string(i) + ".png");

				delete OutPutImage; 

			}

			

			glDeleteTextures(1, &FoliageDataTexture); 
			delete[] FoliageData; 
			delete[] Pixels; 



		}

		void FoliageBaker::AddFoliageGeometry(FoliageGeometry Geometry)
		{
			this->Geometry.push_back(Geometry); 
		}

		void FoliageBaker::SaveFoliageGeometrySetup(const std::string& Path)
		{

			std::ofstream FileOut; 
			FileOut.open(Path); 

			for (auto& Geometry : Geometry) {
				FileOut << "Type: "; 
				switch (Geometry.Type) {

				case FoliageGeometryType::RECTANGLE: 
					FileOut << "RECTANGLE"; 
				break; 

				case FoliageGeometryType::CIRCLE:
					FileOut << "CIRCLE"; 
				break; 

				default: 
					FileOut << "Unknown"; 
					break; 

				}
				FileOut << " Position: " << Geometry.RotationPosition.y << ' ' << Geometry.RotationPosition.z; 
				FileOut << " Size: " << Geometry.Size.x << ' ' << Geometry.Size.y; 
				FileOut << " Rotation: " << Geometry.RotationPosition.x; 
				FileOut << " Shift:" << Geometry.Shift.x << ' ' << Geometry.Shift.y; 
				FileOut << '\n';	
			}
			FileOut.close(); 
			
		}

		void FoliageBaker::LoadFoliageGeometrySetup(const std::string& Path)
		{
		}

	}


}