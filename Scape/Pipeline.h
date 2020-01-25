#pragma once

#include "Direct.h"
#include "Camera.h"
#include "FoliageRenderer.h"
namespace Scape {

	using namespace Rendering; 

	struct Pipeline {

		WorldManager World; 
		SkyRendering Sky; 
		DirectRenderer Direct; 
		FoliageRenderer FoliageRenderer; 


		void PreparePipeline(Camera& Camera, Window& Window);
		void RunPipeline(Camera& Camera, Window& Window);
		


	};

}
