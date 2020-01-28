#include "CubeMapHandler.h"

namespace Scape {

	namespace Rendering {

		void CubeMapHandler::PrepareCubeMapHandler()
		{
			
			CubeBuffer = CubeMultiPassFrameBufferObject(Vector2i(CubeMapRes), 2, { GL_RGBA16F, GL_R32F }, false, { false,false });
			TemporaryCubeBuffer = MultiPassFrameBufferObject(Vector2i(CubeMapRes), 4, { GL_RGBA16F, GL_RGBA16F, GL_RGBA32F, GL_RGB16F });

			CubeFinal = Shader("Shaders/CubeMapCombiner"); 

			CubeFinal.Bind(); 

			CubeFinal.SetUniform("ShadowMaps[0]", 0);
			CubeFinal.SetUniform("ShadowMaps[1]", 1);
			CubeFinal.SetUniform("ShadowMaps[2]", 2);
			CubeFinal.SetUniform("ShadowMaps[3]", 3);
			CubeFinal.SetUniform("ShadowMaps[4]", 4);

			CubeFinal.SetUniform("WorldPosition", 5);
			CubeFinal.SetUniform("Normal", 6);
			CubeFinal.SetUniform("Albedo", 7);
			CubeFinal.SetUniform("LightMap", 8);


			CubeFinal.UnBind(); 

		}

		int Frame = 0; 

		void CubeMapHandler::RenderToCubeMap(Camera& Camera, WorldManager& World, SkyRendering& Sky)
		{

			Frame++; 
			for (int Side = 0; Side < 6; Side++) {

				Matrix4f ViewMatrix = glm::translate(CubeViews[Side], Vector3f(-Camera.Position.x, -Camera.Position.y, -Camera.Position.z));

				Scape::Camera TemporaryCamera; 
				TemporaryCamera.View = ViewMatrix; 
				TemporaryCamera.Project = Core::TAAJitterMatrix(Frame, Vector2i(CubeMapRes))* CubeProjection; 

				glEnable(GL_DEPTH_TEST); 

				TemporaryCubeBuffer.Bind(); 


				World.RenderWorld(TemporaryCamera, Sky.SkyCube); 
				World.RenderWorldTerrain(TemporaryCamera, Sky.SkyCube); 

				TemporaryCubeBuffer.UnBind(); 

				CubeBuffer.Bind();

				glDisable(GL_DEPTH_TEST); 

				CubeFinal.Bind(); 

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + Side, CubeBuffer.Texture[0], 0);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + Side, CubeBuffer.Texture[1], 0);

				CubeFinal.SetUniform("SunColor", Sky.SunColor);
				CubeFinal.SetUniform("Frame", Frame);
				CubeFinal.SetUniform("LightDirection", Sky.Orientation);
				CubeFinal.SetUniform("UseAlbedo", !sf::Keyboard::isKeyPressed(sf::Keyboard::U));
				CubeFinal.SetUniform("CameraPosition", Camera.Position);

				for (int i = 0; i < 5; i++) {
					std::string Title = "ShadowMatrices[" + std::to_string(i) + "]";

					CubeFinal.SetUniform(Title.c_str(), Sky.ProjectionMatrices[i] * Sky.ViewMatrices[i]);
					Sky.ShadowMaps[i].BindDepthImage(i);

				}

				TemporaryCubeBuffer.BindImage(2, 5);
				TemporaryCubeBuffer.BindImage(1, 6);
				TemporaryCubeBuffer.BindImage(0, 7);
				TemporaryCubeBuffer.BindImage(3, 8);

				DrawPostProcessQuad();

				CubeFinal.Bind(); 

				CubeBuffer.UnBind();





			}





		}

	}

}