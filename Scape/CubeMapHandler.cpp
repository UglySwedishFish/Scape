#include "CubeMapHandler.h"

namespace Scape {

	namespace Rendering {

		void CubeMapHandler::PrepareCubeMapHandler()
		{
			
			CubeBuffer = CubeFrameBufferObject(Vector2i(CubeMapRes), GL_RGBA16F, false);
			TemporaryCubeBuffer = MultiPassFrameBufferObject(Vector2i(CubeMapRes), 2, { GL_RGB16F, GL_RGB16F }); 




		}

		void CubeMapHandler::RenderToCubeMap(Camera& Camera, WorldManager& World, SkyRendering& Sky)
		{

		}

	}

}