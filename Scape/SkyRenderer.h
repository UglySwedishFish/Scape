#pragma once

#include "FrameBuffer.h"
#include "Shader.h"
#include "Camera.h"
#include "WorldManager.h"

namespace Scape {


	namespace Rendering {

		const float Ranges[5] = { 15.0, 45.0, 135., 405., 1215. };
		const unsigned char UpdateQueue[] = { 0,1,0,2,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,2,0,1,0,3,0,4 };
		const int SHADOWMAP_RES = 2048; 


		struct SkyRendering {


			FrameBufferObject SkyIncident; 
			CubeMultiPassFrameBufferObject SkyCube; 
			FrameBufferObject ShadowMaps[5]; 
			Matrix4f ViewMatrices[5], ProjectionMatrices[5]; 

			Shader SkyIncidentShader, SkyCubeShader; 

			void PrepareSkyRenderer(Window & Window); 
			void RenderSky(Window& Window, Camera & Camera, WorldManager & World); 





			//TimeOfDay is measured in seconds 
			void SetTimeOfDay(float TimeOfDay); 

			Vector2f Direction; 
			Vector3f Orientation; 
			Vector3f SunColor; 

		private: 

			void UpdateShadowMap(Window & Window, WorldManager& World);


		};

	}

}