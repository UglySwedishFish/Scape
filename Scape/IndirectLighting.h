#pragma once

#include "Direct.h"
#include "CubeMapHandler.h"

namespace Scape {

	namespace Rendering {




		struct Indirect {


			FrameBufferObject IndirectPrep; 
			MultiPassFrameBufferObject IndirectOutPut; 
			Shader IndirectShader, IndirectPrepShader; 
			
			void PrepareIndirect(Window& Window); 
			void RenderIndirectLighting(Window& Window, Camera& Camera, FoliageRenderer& FoliageDeferred, DirectRenderer& Direct, LightBaker & LightBaker, CubeMapHandler & CubeMap, SkyRendering & Sky); 


		};


	}

}