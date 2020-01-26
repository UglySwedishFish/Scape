#include "FoliageRenderer.h"

namespace Scape {

	namespace Rendering {




		void FoliageRenderer::PrepareFoligeRenderer(Window& Window)
		{
			FoliageRenderer = Shader("Shaders/FoliageRenderer"); 

			for (int i = 0; i < 16; i++) {
				std::string Title = "Foliage/FoliageData" + std::to_string(i) + ".png"; 
				RayData[i] = LoadTextureGL3D(Title.c_str(), Vector3i(FoliageResolution, FoliageDirections, FoliageResolution));
				
			}

			WindTexture = LoadTextureGL("Foliage/Wind.jpg"); 

			FoliageRenderer.Bind(); 
			FoliageRenderer.SetUniform("MaxLength", MaxLenght); 
			for (int i = 0; i < 16; i++) {
				std::string Title = "RayData[" + std::to_string(i) + "]"; 
				FoliageRenderer.SetUniform(Title.c_str(), i);
			}
			FoliageRenderer.SetUniform("Wind", 16); 
			FoliageRenderer.UnBind();

		}

		void FoliageRenderer::RenderFoliage(Camera& Camera, Window& Window)
		{

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

			FoliageRenderer.Bind(); 

			FoliageRenderer.SetUniform("CameraPosition", Camera.Position); 
			FoliageRenderer.SetUniform("IdentityMatrix", Camera.Project * Camera.View);
			FoliageRenderer.SetUniform("Time", Window.GetTimeOpened());
			FoliageRenderer.SetUniform("RandTexCoord", !sf::Keyboard::isKeyPressed(sf::Keyboard::R));

			for(int i = 0; i < 16; i++)
				RayData[i].Bind(i); 

			WindTexture.Bind(16); 

			DrawPostProcessQuad(); 

			FoliageRenderer.UnBind(); 


		}

		void FoliageRenderer::ReloadFoliage()
		{
			FoliageRenderer.Reload("Shaders/FoliageRenderer"); 
			FoliageRenderer.Bind();
			FoliageRenderer.SetUniform("MaxLength", MaxLenght);
			for (int i = 0; i < 16; i++) {
				std::string Title = "RayData[" + std::to_string(i) + "]";
				FoliageRenderer.SetUniform(Title.c_str(), i);
			}
			FoliageRenderer.SetUniform("Wind", 16);
			FoliageRenderer.UnBind();
		}

	}


}