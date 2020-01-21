#include "SkyRenderer.h"
#include "AtmosphereRenderer.h"


namespace Scape {

	namespace Rendering {

		void Move(Vector3f& Pos, float Speed, float RotationX, float RotationY) {
			Pos.x -= (cos(RotationY * (PI / 180.)) * cos(RotationX * (PI / 180.))) * Speed;
			Pos.y += sin(RotationX * (PI / 180.)) * Speed;
			Pos.z -= (sin(RotationY * (PI / 180.)) * cos(RotationX * (PI / 180.))) * Speed;
		}

		void SkyRendering::PrepareSkyRenderer(Window& Window)
		{
			SkyIncident = FrameBufferObject(Window.GetResolution() / 8, GL_RGB16F, false, false); 
			SkyCube = CubeMultiPassFrameBufferObject(Vector2i(32), 2, { GL_RGB16F, GL_RGB16F }, false, { false, true });

			SkyIncidentShader = Shader("Shaders/SkyIncidentShader");
			SkyCubeShader = Shader("Shaders/SkyCubeShader");

			for (int i = 0; i < 5; i++) {
				ShadowMaps[i] = FrameBufferObject(Vector2i(SHADOWMAP_RES), GL_R8);
				ProjectionMatrices[i] = Core::ShadowOrthoMatrix(Ranges[i], 100.f, 2500.f);
			}


			



		}

		void SkyRendering::RenderSky(Window& Window, Camera& Camera, WorldManager& World)
		{

			glDisable(GL_DEPTH_TEST); 

			SkyCube.Bind();

			SkyCubeShader.Bind();

			SkyCubeShader.SetUniform("SunDirection", Orientation);

			for (int i = 0; i < 6; i++) {


				SkyCubeShader.SetUniform("ViewMatrix", CubeProjection * CubeViews[i]);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, SkyCube.Texture[0], 0);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, SkyCube.Texture[1], 0);

				DrawPostProcessCube();

			}

			SkyCube.UnBind(Window);

			SkyCubeShader.UnBind();

			SkyIncidentShader.Bind();

			SkyIncident.Bind();

			SkyIncidentShader.SetUniform("SunDirection", Orientation);

			SkyIncidentShader.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));

			DrawPostProcessQuad();

			SkyIncident.UnBind(Window);

			SkyIncidentShader.UnBind();

			UpdateShadowMap(Window, World); 


		}

		void SkyRendering::SetTimeOfDay(float TimeOfDay)
		{

			float TimeNormalized = TimeOfDay / 86400.; 

			float TimeAngles = 360. * TimeNormalized;
		
			Direction.x = TimeAngles; 
			Direction.y = TimeAngles;

			Orientation = Vector3f(0.0); 

			Move(Orientation, 1.0, Direction.x, Direction.y - 90.0);

			SunColor = Atmospheric::GetSunColor(Orientation); 

		}
		void SkyRendering::UpdateShadowMap(Window& Window, WorldManager& World)
		{

			int ToUpdate = UpdateQueue[Window.GetFrameCount() % 26];






			ShadowMaps[ToUpdate].Bind(); 



			ShadowMaps[ToUpdate].UnBind(Window); 













		}
	}
}

