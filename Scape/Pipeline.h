#pragma once

#include "Direct.h"
#include "Camera.h"
namespace Scape {

	using namespace Rendering; 

	struct Pipeline {

		WorldManager World; 
		SkyRendering Sky; 
		DirectRenderer Direct; 



		void PreparePipeline(Camera& Camera, Window& Window);
		void RunPipeline(Camera& Camera, Window& Window);
		


	};

}
