#pragma once

#include "IndirectLighting.h"

namespace Scape {

	namespace Rendering {

		//32 sample TAA. used for denoising more stochastic elements (such as screen-space + cubemap-space global illumination, AO and reflections) as well as severely lowering any form of aliasing 
		struct TemporalAntiAliasing {

			FrameBufferObjectPreviousData TemporalAntiAliasing; 
			Shader TAAShader, BlitToScreenShader; 

			void PrepareTemporalAntiAliasing(Window& Window);
			void DoTemporalAntiAliasing(Window& Window, Camera& Camera, DirectRenderer& Direct, Indirect& Indirect, FoliageRenderer & Foliage); 

			 

		};

	}

}