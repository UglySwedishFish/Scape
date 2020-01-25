#pragma once

//injects micro-foliage in rendering! 
//foliage can be rendered onto anything, as long as it has tangents and bi-tangents 

#include "FrameBuffer.h"
#include "Shader.h"
#include "Camera.h"
#include "MinifoliageBaker.h"
#include "Texture.h"

namespace Scape {

	namespace Rendering {

		struct FoliageRenderer {

			Shader FoliageRenderer; 
			TextureGL3D RayData; 


			void PrepareFoligeRenderer(Window & Window); 
			void RenderFoliage(Camera& Camera, Window& Window); 

		};


	}


}



