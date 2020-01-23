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
			ShadowDeferred = Shader("Shaders/EntityDeferred/shader.vert", std::string("Shaders/EntityDeferred/shader.frag")); 

			ShadowDeferred.Bind(); 

			ShadowDeferred.SetUniform("InstanceData", 0);
			ShadowDeferred.SetUniform("AlbedoMap", 2);
			ShadowDeferred.SetUniform("NormalMap", 3);
			ShadowDeferred.SetUniform("RoughnessMap", 4);
			ShadowDeferred.SetUniform("MetalnessMap", 5);
			ShadowDeferred.SetUniform("EmissiveMap", 6);
			ShadowDeferred.SetUniform("LightMap", 7);
			ShadowDeferred.SetUniform("Sky", 8);

			ShadowDeferred.UnBind(); 

			for (int i = 0; i < 5; i++) {
				ShadowMaps[i] = FrameBufferObject(Vector2i(SHADOWMAP_RES), GL_RGB16F);
				ProjectionMatrices[i] = Core::ShadowOrthoMatrix(Ranges[i], 100.f, 2500.f);
			}


		}

		void SkyRendering::RenderSky(Window& Window, Camera& Camera, WorldManager& World)
		{

			glEnable(GL_DEPTH_TEST); 

			UpdateShadowMap(Window, Camera, World);

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



		}

		void SkyRendering::SetTimeOfDay(float TimeOfDay)
		{

			float TimeNormalized = glm::fract(TimeOfDay / 86400.); 

			//TimeNormalized = int(TimeNormalized * 32) / 32.f; 

			float TimeAngles = 360. * TimeNormalized;
		
			Direction.x = TimeAngles; 
			Direction.y = TimeAngles / 2.f;

			Orientation = Vector3f(0.0); 

			Move(Orientation, 1.0, Direction.x, Direction.y - 90.0);

			SunColor = Atmospheric::GetSunColor(Orientation); 

		}
		void SkyRendering::GetTimeOfDayDirection(float TimeOfDay, Vector2f& Direction, Vector3f& Orientation)
		{
			float TimeNormalized = glm::fract(TimeOfDay / 86400.);

			float TimeAngles = 360. * TimeNormalized;

			Direction.x = TimeAngles;
			Direction.y = TimeAngles / 2.f;

			Orientation = Vector3f(0.0);

			Move(Orientation, 1.0, Direction.x, Direction.y - 90.0);
		}
		void SkyRendering::UpdateShadowMap(Window& Window, Camera& Camera, WorldManager& World)
		{

			Scape::Camera ShadowCamera; 

			int ToUpdate = UpdateQueue[Window.GetFrameCount() % 26];

			ViewMatrices[ToUpdate] = Core::ViewMatrix(Camera.Position + Orientation * 500.0f, Vector3f(Direction.x, Direction.y, 0.));

			ShadowCamera.Project = ProjectionMatrices[ToUpdate]; 
			ShadowCamera.View = ViewMatrices[ToUpdate]; 
			


			ShadowDeferred.Bind(); 

			ShadowMaps[ToUpdate].Bind(); 

			World.RenderWorld(ShadowCamera, SkyCube, &ShadowDeferred);

			ShadowMaps[ToUpdate].UnBind(Window); 

			ShadowDeferred.UnBind(); 

		}
	}
}

