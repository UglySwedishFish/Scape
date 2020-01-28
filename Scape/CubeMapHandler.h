#pragma once

#include "WorldManager.h"
#include "SkyRenderer.h"

namespace Scape {

	

	namespace Rendering {

		const int CubeMapRes = 256; 

		struct CubeMapHandler {
			
			CubeMultiPassFrameBufferObject CubeBuffer; //r, g, b -> lighting, a -> linearlized traversal distance
			

			void PrepareCubeMapHandler(); 
			void RenderToCubeMap(Camera& Camera, WorldManager& World, SkyRendering& Sky); 

		private: 

			Shader CubeFinal;
			MultiPassFrameBufferObject TemporaryCubeBuffer; //<- this is were all the actual geometry is rendered onto. 

		};

	}

}