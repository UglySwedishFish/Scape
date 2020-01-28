#include "TemporalAntiAliasing.h"

namespace Scape {
	namespace Rendering {
		void TemporalAntiAliasing::PrepareTemporalAntiAliasing(Window& Window)
		{
			TemporalAntiAliasing = FrameBufferObjectPreviousData(Window.GetResolution(), GL_RGBA16F, false, false); 
			TAAShader = Shader("Shaders/TAA"); 
			BlitToScreenShader = Shader("Shaders/Copy"); 

			TAAShader.Bind(); 

			TAAShader.SetUniform("CombinedLighting", 0); 
			TAAShader.SetUniform("PreviousLighting", 1);
			TAAShader.SetUniform("WorldPosPrevious", 2);
			TAAShader.SetUniform("PreviousWorldPos", 3);
			TAAShader.SetUniform("CurrentNormal", 4);
			TAAShader.SetUniform("PreviousNormal", 5);

			TAAShader.UnBind(); 


		}
		void TemporalAntiAliasing::DoTemporalAntiAliasing(Window& Window, Camera& Camera, DirectRenderer& Direct, Indirect& Indirect, FoliageRenderer& Foliage)
		{
			
			TemporalAntiAliasing.Bind(); 

			TAAShader.Bind(); 

			Indirect.IndirectOutPut.BindImage(0, 0);
			TemporalAntiAliasing.BindImagePrevious(1); 
			Foliage.CombinedDeferred.BindImage(2, 2);
			Foliage.CombinedDeferred.BindImagePrevious(2,3); 
			Foliage.CombinedDeferred.BindImage(1, 4);
			Foliage.CombinedDeferred.BindImagePrevious(1, 5);

			TAAShader.SetUniform("CameraPosition", Camera.Position); 
			TAAShader.SetUniform("MotionMatrix", Camera.Project * Camera.PrevView); 

			DrawPostProcessQuad(); 

			TAAShader.UnBind(); 

			TemporalAntiAliasing.UnBind(Window); 

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

			BlitToScreenShader.Bind(); 

			TemporalAntiAliasing.BindImage(0); 

			DrawPostProcessQuad(); 

			BlitToScreenShader.UnBind(); 


		}
	}
}
