#include "FoliageRenderer.h"

namespace Scape {

	namespace Rendering {




		void FoliageRenderer::PrepareFoligeRenderer(Window& Window)
		{

			FoliageShadowProject = Core::ShadowOrthoMatrix(7.5, 10.0f, 50.0f); 
			

			FoliageRenderer = Shader("Shaders/FoliageRenderer"); 

			CombinedDeferred = MultiPassFrameBufferObjectPreviousData(Window.GetResolution(), 5, { GL_RGBA16F, GL_RGBA16F, GL_RGBA32F, GL_RGB16F,GL_RGBA16F });
			FoliageShadowMap = FrameBufferObject(Vector2i(4096), GL_R8);

			for (int i = 0; i < 256; i++) {
				std::string Title = "Foliage/FoliageData" + std::to_string(i) + ".png"; 
				RayData[i] = LoadTextureGL3D(Title.c_str(), Vector3i(FoliageResolution, FoliageDirections, FoliageResolution));
				
			}

			WindTexture = LoadTextureGL("Foliage/Wind.jpg"); 
			GrassTexture = LoadTextureGL("Textures/Grass Blade.png"); 
			GrassSurfaceTexture = LoadTextureGL("Textures/Grass Surface.jpg"); 

			FoliageRenderer.Bind(); 
			FoliageRenderer.SetUniform("MaxLength", MaxLenght); 
			for (int i = 0; i < 16; i++) {
				std::string Title = "RayData[" + std::to_string(i) + "]"; 
				FoliageRenderer.SetUniform(Title.c_str(), i);
			}

			FoliageRenderer.SetUniform("GrassTexture", 21);
			FoliageRenderer.SetUniform("GrassSurfaceTexture", 22);
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

		void FoliageRenderer::RenderFoliage(Camera& Camera, Window& Window, WorldManager& World, SkyRendering& Sky)
		{

			glEnable(GL_DEPTH_TEST); 

			RenderFoliageShadowMap(Camera, Window, World, Sky); 

			CombinedDeferred.Bind(); 

			FoliageRenderer.Bind(); 

			FoliageRenderer.SetUniform("CameraPosition", Camera.Position); 
			FoliageRenderer.SetUniform("IdentityMatrix", Camera.Project * Camera.View);
			FoliageRenderer.SetUniform("Time", Window.GetTimeOpened());
			FoliageRenderer.SetUniform("RandTexCoord", !sf::Keyboard::isKeyPressed(sf::Keyboard::R));
			FoliageRenderer.SetUniform("LightDirection", Sky.Orientation);
			FoliageRenderer.SetUniform("IdentityMatrix", Camera.Project * Camera.View);

			World.DeferredFBO.BindDepthImage(0); 
			World.DeferredFBOTerrain.BindDepthImage(1); 

			for (int i = 0; i < 4; i++) {

				World.DeferredFBO.BindImage(i, 2 + i); 
				World.DeferredFBOTerrain.BindImage(i, 6 + i);

			}
			
			WindTexture.Bind(16); 

			RayData[int(Window.GetTimeOpened() * 30) & 255].Bind(20); 

			GrassTexture.Bind(21); 

			GrassSurfaceTexture.Bind(22); 

			DrawPostProcessQuad(); 

			FoliageRenderer.UnBind(); 

			CombinedDeferred.UnBind(); 

			glDisable(GL_DEPTH_TEST);

		}

		void FoliageRenderer::RenderFoliageShadowMap(Camera& Camera, Window& Window, WorldManager& World, SkyRendering& Sky)
		{

			FoliageShadowView = Core::ViewMatrix(Camera.Position + Sky.Orientation * 25.f, Vector3f(Sky.Direction.x, Sky.Direction.y, 0.f)); 

			Scape::Camera ShadowCamera; 
			ShadowCamera.Project = FoliageShadowProject; 
			ShadowCamera.View = FoliageShadowView; 


			Sky.ShadowDeferred.Bind();

			FoliageShadowMap.Bind();

			World.RenderWorld(ShadowCamera, Sky.SkyCube, &Sky.ShadowDeferred);

			FoliageShadowMap.UnBind(Window);

			Sky.ShadowDeferred.UnBind();



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