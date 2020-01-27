#pragma once

//injects micro-foliage in rendering! 
//foliage can be rendered onto anything, as long as it has tangents and bi-tangents 

#include "FrameBuffer.h"
#include "Shader.h"
#include "Camera.h"
#include "MinifoliageBaker.h"
#include "Texture.h"
#include "SkyRenderer.h"

namespace Scape {

	namespace Rendering {

		struct FoliageRenderer {

			Matrix4f FoliageShadowView; 
			Matrix4f FoliageShadowProject; 

			Shader FoliageRenderer, TerrainShadowDeferred; 
			TextureGL3D RayData[256]; 
			TextureGL WindTexture; 
			TextureGL GrassTexture, GrassSurfaceTexture;

			MultiPassFrameBufferObjectPreviousData CombinedDeferred; 
			FrameBufferObject FoliageShadowMap; 

			void PrepareFoligeRenderer(Window & Window); 
			void RenderFoliage(Camera& Camera, Window& Window, WorldManager& World, SkyRendering& Sky); 
			void RenderFoliageShadowMap(Camera& Camera, Window& Window, WorldManager& World, SkyRendering& Sky); 
			void ReloadFoliage(); 

		};


	}


}



