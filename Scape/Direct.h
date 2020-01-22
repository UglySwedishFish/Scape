#pragma once

#include "SkyRenderer.h"


namespace Scape {

	namespace Rendering {

		struct DirectRenderer {

			MultiPassFrameBufferObject DirectLighting; 
			Shader DirectShader; 

			void PrepareDirectLighting(Window& Window); 
			void RenderDirectLighting(Window& Window, Camera& Camera, WorldManager & World, SkyRendering & Sky); 

		};



	}


}
