#include "Direct.h"

namespace Scape {

	namespace Rendering {

		


		void DirectRenderer::PrepareDirectLighting(Window& Window)
		{
			DirectLighting = MultiPassFrameBufferObject(Window.GetResolution(), 2, { GL_RGB16F, GL_RGB16F }, false); 
			DirectShader = Shader("Shaders/Direct"); 
			
			DirectShader.Bind();

			DirectShader.SetUniform("ShadowMaps[0]", 0);
			DirectShader.SetUniform("ShadowMaps[1]", 1);
			DirectShader.SetUniform("ShadowMaps[2]", 2);

			DirectShader.SetUniform("WorldPosition", 3);
			DirectShader.SetUniform("Normal", 4);
			DirectShader.SetUniform("Albedo", 5);
			DirectShader.SetUniform("LightMap", 6);
			DirectShader.SetUniform("GrassDirect", 7);
			DirectShader.SetUniform("SkyIncident", 8);

			DirectShader.UnBind();
		}

		void DirectRenderer::RenderDirectLighting(Window& Window, Camera& Camera, FoliageRenderer& World, SkyRendering& Sky)
		{

			DirectLighting.Bind(); 

			DirectShader.Bind();
			
			DirectShader.SetUniform("SunColor", Sky.SunColor); 
			DirectShader.SetUniform("Frame", Window.GetFrameCount()); 
			DirectShader.SetUniform("LightDirection", Sky.Orientation);
			DirectShader.SetUniform("UseAlbedo", !sf::Keyboard::isKeyPressed(sf::Keyboard::U));

			for (int i = 0; i < 3; i++) {
				std::string Title = "ShadowMatrices[" + std::to_string(i) + "]"; 

				DirectShader.SetUniform(Title.c_str(), Sky.ProjectionMatrices[i] * Sky.ViewMatrices[i]); 
				Sky.ShadowMaps[i].BindDepthImage(i); 

			}

			
			
			World.CombinedDeferred.BindImage(2, 3);
			World.CombinedDeferred.BindImage(1, 4);
			World.CombinedDeferred.BindImage(0, 5);
			World.CombinedDeferred.BindImage(3, 6);
			World.CombinedDeferred.BindImage(4, 7);
			Sky.SkyIncident.BindImage(8); 

			DrawPostProcessQuad(); 

			DirectShader.UnBind();

			DirectLighting.UnBind(); 
			


		}

	}


}