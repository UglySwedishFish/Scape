#include "IndirectLighting.h"

namespace Scape {

	namespace Rendering {



		void Indirect::PrepareIndirect(Window& Window)
		{
			IndirectPrep = MultiPassFrameBufferObject(Window.GetResolution() / 2, 2, { GL_R16F,GL_RGBA16F }, false);
			IndirectOutPut = MultiPassFrameBufferObject(Window.GetResolution() / 2, 2, { GL_R16F, GL_R16F }, false);
			IndirectUpscaled = MultiPassFrameBufferObject(Window.GetResolution(), 2, { GL_RGBA16F, GL_RGB16F }, false);
			IndirectRays = MultiPassFrameBufferObject(Window.GetResolution(), 2, { GL_RGB16F, GL_RGB16F }, false); 

			IndirectShader = Shader("Shaders/Indirect");
			IndirectPrepShader = Shader("Shaders/IndirectPrep"); 
			IndirectUpscalingShader = Shader("Shaders/IndirectUpscaling"); 
			IndirectRayGenerator = Shader("Shaders/IndirectDirections"); 
			
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

			IndirectShader.SetUniform("DiffuseDirection", 13);
			IndirectShader.SetUniform("SpecularDirection", 14);

			IndirectShader.UnBind(); 

			IndirectUpscalingShader.Bind(); 

			IndirectUpscalingShader.SetUniform("NormalHighRes", 0); 
			IndirectUpscalingShader.SetUniform("DepthHighRes", 1);
			IndirectUpscalingShader.SetUniform("WorldPosition", 2);

			IndirectUpscalingShader.SetUniform("Direct", 3);
			IndirectUpscalingShader.SetUniform("BakedIndirect", 4);

			IndirectUpscalingShader.SetUniform("Normal", 5);
			IndirectUpscalingShader.SetUniform("Depth", 6);

			IndirectUpscalingShader.SetUniform("CubeLighting", 7);
			IndirectUpscalingShader.SetUniform("CubeDepth", 8);
			IndirectUpscalingShader.SetUniform("Sky", 9);

			IndirectUpscalingShader.SetUniform("RawDiffuse", 10);
			IndirectUpscalingShader.SetUniform("RawSpecular", 11);

			IndirectUpscalingShader.SetUniform("DiffuseDirection", 12);
			IndirectUpscalingShader.SetUniform("SpecularDirection", 13);
			IndirectUpscalingShader.SetUniform("Albedo", 14);

			IndirectUpscalingShader.SetUniform("Divisor", 2); //might actually use a divisor of 4 in the future :thonk: 



			IndirectUpscalingShader.UnBind(); 

			IndirectRayGenerator.Bind(); 

			IndirectRayGenerator.SetUniform("Normal", 0);
			IndirectRayGenerator.SetUniform("WorldPos", 1);

			IndirectRayGenerator.SetUniform("Sobol", 2);
			IndirectRayGenerator.SetUniform("Ranking", 3);
			IndirectRayGenerator.SetUniform("Scrambling", 4);

			IndirectRayGenerator.UnBind(); 


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

			FoliageDeferred.CombinedDeferred.BindDepthImage(0); 
			FoliageDeferred.CombinedDeferred.BindImage(1, 1);

			IndirectPrepShader.SetUniform("zNear", Camera.znear);
			IndirectPrepShader.SetUniform("zFar", Camera.zfar);

			DrawPostProcessQuad(); 

			IndirectPrepShader.UnBind(); 

			IndirectPrep.UnBind(); 


			IndirectRays.Bind(); 

			IndirectRayGenerator.Bind(); 

			FoliageDeferred.CombinedDeferred.BindImage(1, 0);
			FoliageDeferred.CombinedDeferred.BindImage(2, 1);

			IndirectRayGenerator.SetUniform("CameraPosition", Camera.Position);
			IndirectRayGenerator.SetUniform("Frame", Window.GetFrameCount() & 63);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, LightBaker.SobolTexture);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, LightBaker.RankingTexture);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, LightBaker.ScramblingTexture);

			DrawPostProcessQuad(); 

			IndirectRayGenerator.UnBind(); 

			IndirectRays.UnBind(); 

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

			IndirectPrep.BindImage(0, 0); 
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

			IndirectPrep.BindImage(0, 11); 

			glActiveTexture(GL_TEXTURE12);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Sky.SkyCube.Texture[0]);

			IndirectRays.BindImage(0, 13); 
			IndirectRays.BindImage(1, 14);

			DrawPostProcessQuad(); 

			IndirectShader.UnBind();

			IndirectOutPut.UnBind(); 


			if (!sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
				IndirectUpscaled.Bind();

				IndirectUpscalingShader.Bind();

				FoliageDeferred.CombinedDeferred.BindImage(1, 0);
				FoliageDeferred.CombinedDeferred.BindDepthImage(1);
				FoliageDeferred.CombinedDeferred.BindImage(2, 2);

				Direct.DirectLighting.BindImage(0, 3);
				FoliageDeferred.CombinedDeferred.BindImage(3, 4);

				IndirectPrep.BindImage(1, 5);
				IndirectPrep.BindImage(0, 6);

				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap.CubeBuffer.Texture[0]);

				glActiveTexture(GL_TEXTURE8);
				glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap.CubeBuffer.Texture[1]);

				glActiveTexture(GL_TEXTURE9);
				glBindTexture(GL_TEXTURE_CUBE_MAP, Sky.SkyCube.Texture[0]);

				IndirectOutPut.BindImage(0, 10);
				IndirectOutPut.BindImage(1, 11);

				IndirectRays.BindImage(0, 12); 
				IndirectRays.BindImage(1, 13);

				FoliageDeferred.CombinedDeferred.BindImage(0, 14);


				IndirectUpscalingShader.SetUniform("zNear", Camera.znear);
				IndirectUpscalingShader.SetUniform("zFar", Camera.zfar);
				IndirectUpscalingShader.SetUniform("CameraPosition", Camera.Position);
				IndirectUpscalingShader.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
				IndirectUpscalingShader.SetUniform("InverseView", glm::inverse(Camera.View));
				IndirectUpscalingShader.SetUniform("ProjectionMatrix", Camera.Project);
				IndirectUpscalingShader.SetUniform("ViewMatrix", Camera.View);
				IndirectUpscalingShader.SetUniform("UseHalfRes", sf::Keyboard::isKeyPressed(sf::Keyboard::U));


				DrawPostProcessQuad();

				IndirectUpscalingShader.UnBind();

				IndirectUpscaled.UnBind();
			}
		}

	}

}