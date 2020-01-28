#include "IndirectLighting.h"

namespace Scape {

	namespace Rendering {



		void Indirect::PrepareIndirect(Window& Window)
		{
			IndirectPrep = FrameBufferObject(Window.GetResolution(), GL_R32F, false); 
			IndirectOutPut = MultiPassFrameBufferObject(Window.GetResolution(), 2, { GL_RGBA16F, GL_RGB16F }, false); 

			IndirectShader = Shader("Shaders/Indirect");
			IndirectPrepShader = Shader("Shaders/IndirectPrep"); 
			
			IndirectPrepShader.Bind(); 

			IndirectPrepShader.SetUniform("WorldPosition", 0); 
			IndirectPrepShader.SetUniform("Normal", 1);

			IndirectPrepShader.UnBind(); 

			IndirectShader.Bind(); 

			IndirectShader.SetUniform("ViewSpaceZ", 0); 
			IndirectShader.SetUniform("DirectInput", 1);

			IndirectShader.SetUniform("CubeLighting", 2);
			IndirectShader.SetUniform("CubeLinearZ", 3);

			IndirectShader.SetUniform("Albedo", 4);
			IndirectShader.SetUniform("Normal", 5);
			IndirectShader.SetUniform("WorldPos", 6);
			IndirectShader.SetUniform("BakedIndirectDiffuse", 7);

			IndirectShader.SetUniform("Sobol", 8);
			IndirectShader.SetUniform("Ranking", 9);
			IndirectShader.SetUniform("Scrambling", 10);

			IndirectShader.SetUniform("Depth", 11);
			IndirectShader.SetUniform("Sky", 12);

			IndirectShader.UnBind(); 


		}

		float Rough = 0.0f; 

		void Indirect::RenderIndirectLighting(Window& Window, Camera& Camera, FoliageRenderer& FoliageDeferred, DirectRenderer& Direct, LightBaker& LightBaker, CubeMapHandler& CubeMap, SkyRendering& Sky)
		{

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
				Rough += 1.0 * Window.GetFrameTime(); 
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				Rough -= 1.0 * Window.GetFrameTime();
			Rough = glm::clamp(Rough, 0.f, 1.f); 

			IndirectPrep.Bind(); 

			IndirectPrepShader.Bind(); 

			FoliageDeferred.CombinedDeferred.BindImage(2, 0); 
			FoliageDeferred.CombinedDeferred.BindImage(1, 1);

			IndirectPrepShader.SetUniform("ViewMatrix", Camera.View); 

			DrawPostProcessQuad(); 

			IndirectPrepShader.UnBind(); 

			IndirectPrep.UnBind(); 


			IndirectOutPut.Bind(); 

			IndirectShader.Bind(); 

			IndirectShader.SetUniform("InverseView", glm::inverse(Camera.View)); 
			IndirectShader.SetUniform("ProjectionMatrix", Camera.Project); 
			IndirectShader.SetUniform("ViewMatrix", Camera.View); 
			IndirectShader.SetUniform("CameraPosition", Camera.Position); 
			IndirectShader.SetUniform("Frame", Window.GetFrameCount() & 63); 
			IndirectShader.SetUniform("zNear", Camera.znear);
			IndirectShader.SetUniform("zFar", Camera.zfar);
			IndirectShader.SetUniform("UseNewMethod", sf::Keyboard::isKeyPressed(sf::Keyboard::N));
			IndirectShader.SetUniform("Roughness", Rough);

			IndirectPrep.BindImage(0); 
			Direct.DirectLighting.BindImage(0, 1); 

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap.CubeBuffer.Texture[0]);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap.CubeBuffer.Texture[1]);
			

			FoliageDeferred.CombinedDeferred.BindImage(0, 4);
			FoliageDeferred.CombinedDeferred.BindImage(1, 5); 
			FoliageDeferred.CombinedDeferred.BindImage(2, 6);
			FoliageDeferred.CombinedDeferred.BindImage(3, 7);


			glActiveTexture(GL_TEXTURE8); 
			glBindTexture(GL_TEXTURE_2D, LightBaker.SobolTexture); 

			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, LightBaker.RankingTexture);

			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D, LightBaker.ScramblingTexture);

			FoliageDeferred.CombinedDeferred.BindDepthImage(11); 

			glActiveTexture(GL_TEXTURE12);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Sky.SkyCube.Texture[0]);

			DrawPostProcessQuad(); 

			IndirectShader.UnBind();

			IndirectOutPut.UnBind(); 

		}

	}

}