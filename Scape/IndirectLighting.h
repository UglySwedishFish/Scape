#pragma once

#include "Direct.h"
#include "CubeMapHandler.h"

namespace Scape {

	namespace Rendering {




		struct Indirect {


			MultiPassFrameBufferObject IndirectPrep;
			MultiPassFrameBufferObject IndirectOutPut; 
			MultiPassFrameBufferObject IndirectUpscaled; 
			MultiPassFrameBufferObject IndirectRays; 
			Shader IndirectShader, IndirectPrepShader, IndirectUpscalingShader, IndirectRayGenerator; 
			
			void PrepareIndirect(Window& Window); 
			void RenderIndirectLighting(Window& Window, Camera& Camera, FoliageRenderer& FoliageDeferred, DirectRenderer& Direct, LightBaker & LightBaker, CubeMapHandler & CubeMap, SkyRendering & Sky); 


		};


	}

}