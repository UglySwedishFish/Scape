#include "FoliageRenderer.h"

namespace Scape {

	namespace Rendering {




		void FoliageRenderer::PrepareFoligeRenderer(Window& Window)
		{
			FoliageRenderer = Shader("Shaders/FoliageRenderer"); 

			CombinedDeferred = MultiPassFrameBufferObject(Window.GetResolution(), 4, { GL_RGBA16F, GL_RGBA16F, GL_RGBA32F, GL_RGB16F });

			for (int i = 0; i < 256; i++) {
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

			FoliageRenderer.SetUniform("CurrentRayData", 20); 

			FoliageRenderer.SetUniform("EntityDepth", 0); 
			FoliageRenderer.SetUniform("TerrainDepth", 1);

			FoliageRenderer.SetUniform("EntityAlbedo", 2); 
			FoliageRenderer.SetUniform("EntityNormal", 3);
			FoliageRenderer.SetUniform("EntityWorldPosition", 4);
			FoliageRenderer.SetUniform("EntityLighting", 5);

			FoliageRenderer.SetUniform("TerrainAlbedo", 6);
			FoliageRenderer.SetUniform("TerrainNormal", 7);
			FoliageRenderer.SetUniform("TerrainWorldPosition", 8);
			FoliageRenderer.SetUniform("TerrainLighting", 9);

			FoliageRenderer.SetUniform("Wind", 16); 
			FoliageRenderer.UnBind();

		}

		void FoliageRenderer::RenderFoliage(Camera& Camera, Window& Window, WorldManager& World, Vector3f SunDirection)
		{

			CombinedDeferred.Bind(); 

			FoliageRenderer.Bind(); 

			FoliageRenderer.SetUniform("CameraPosition", Camera.Position); 
			FoliageRenderer.SetUniform("IdentityMatrix", Camera.Project * Camera.View);
			FoliageRenderer.SetUniform("Time", Window.GetTimeOpened());
			FoliageRenderer.SetUniform("RandTexCoord", !sf::Keyboard::isKeyPressed(sf::Keyboard::R));
			FoliageRenderer.SetUniform("LightDirection", SunDirection);
			FoliageRenderer.SetUniform("IdentityMatrix", Camera.Project * Camera.View);

			World.DeferredFBO.BindDepthImage(0); 
			World.DeferredFBOTerrain.BindDepthImage(1); 

			for (int i = 0; i < 4; i++) {

				World.DeferredFBO.BindImage(i, 2 + i); 
				World.DeferredFBOTerrain.BindImage(i, 6 + i);

			}
			
			WindTexture.Bind(16); 

			RayData[int(Window.GetTimeOpened() * 30) & 255].Bind(20); 

			DrawPostProcessQuad(); 

			FoliageRenderer.UnBind(); 

			CombinedDeferred.UnBind(); 

		}

		void FoliageRenderer::ReloadFoliage()
		{
			FoliageRenderer.Reload("Shaders/FoliageRenderer"); 
			FoliageRenderer.Bind();
			FoliageRenderer.SetUniform("MaxLength", MaxLenght);
			
			FoliageRenderer.SetUniform("EntityDepth", 0);
			FoliageRenderer.SetUniform("TerrainDepth", 1);

			FoliageRenderer.SetUniform("EntityAlbedo", 2);
			FoliageRenderer.SetUniform("EntityNormal", 3);
			FoliageRenderer.SetUniform("EntityWorldPosition", 4);
			FoliageRenderer.SetUniform("EntityLighting", 5);

			FoliageRenderer.SetUniform("TerrainAlbedo", 6);
			FoliageRenderer.SetUniform("TerrainNormal", 7);
			FoliageRenderer.SetUniform("TerrainWorldPosition", 8);
			FoliageRenderer.SetUniform("TerrainLighting", 9);

			FoliageRenderer.SetUniform("Wind", 16);
			FoliageRenderer.SetUniform("CurrentRayData", 20);
			FoliageRenderer.UnBind();
		}

	}


}