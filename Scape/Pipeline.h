#pragma once

#include "WorldManager.h"
#include "SkyRenderer.h"
#include "Camera.h"
namespace Scape {

	using namespace Rendering; 

	struct Pipeline {

		WorldManager World; 
		SkyRendering Sky; 



		void PreparePipeline(Camera& Camera, Window& Window);
		void RunPipeline(Camera& Camera, Window& Window);
		


	};

}
