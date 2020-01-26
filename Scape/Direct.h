#pragma once

#include "SkyRenderer.h"
#include "FoliageRenderer.h"


namespace Scape {

	namespace Rendering {

		struct DirectRenderer {

			MultiPassFrameBufferObject DirectLighting; 
			Shader DirectShader; 

			void PrepareDirectLighting(Window& Window); 
			void RenderDirectLighting(Window& Window, Camera& Camera, FoliageRenderer & World, SkyRendering & Sky); 

		};



	}


}
