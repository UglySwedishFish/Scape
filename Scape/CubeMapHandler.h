#pragma once

#include "WorldManager.h"
#include "SkyRenderer.h"

namespace Scape {

	

	namespace Rendering {

		const int CubeMapRes = 128; 

		struct CubeMapHandler {
			
			CubeFrameBufferObject CubeBuffer; //r, g, b -> lighting, a -> linearlized traversal distance
			
			Shader EntityCubeSimplified, TerrainCubeSimplified; 

			void PrepareCubeMapHandler(); 
			void RenderToCubeMap(Camera& Camera, WorldManager& World, SkyRendering& Sky); 

		private: 

			MultiPassFrameBufferObject TemporaryCubeBuffer; //<- this is were all the actual geometry is rendered onto. 

		};

	}

}