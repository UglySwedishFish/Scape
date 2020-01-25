#include "FoliageRenderer.h"

namespace Scape {

	namespace Rendering {




		void FoliageRenderer::PrepareFoligeRenderer(Window& Window)
		{
			FoliageRenderer = Shader("Shaders/FoliageRenderer"); 
			RayData = LoadTextureGL3D("Foliage/FoliageData.png", Vector3i(FoliageResolution, FoliageDirections, FoliageResolution));

			FoliageRenderer.Bind(); 
			FoliageRenderer.SetUniform("MaxLength", MaxLenght); 
			FoliageRenderer.SetUniform("RayData", 0);
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

			RayData.Bind(0); 

			DrawPostProcessQuad(); 

			FoliageRenderer.UnBind(); 


		}

	}


}