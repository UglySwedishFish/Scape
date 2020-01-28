#pragma once

#include "Camera.h"
#include "TemporalAntiAliasing.h"
namespace Scape {

	using namespace Rendering; 

	struct Pipeline {

		WorldManager World; 
		SkyRendering Sky; 
		DirectRenderer Direct; 
		FoliageRenderer FoliageRenderer; 
		TemporalAntiAliasing TAA; 
		Indirect IndirectLighting; 
		CubeMapHandler CubeMap; 


		void PreparePipeline(Camera& Camera, Window& Window);
		void RunPipeline(Camera& Camera, Window& Window);
		


	};

}
