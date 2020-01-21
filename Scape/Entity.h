#pragma once

#include "Core.h"

namespace Scape {
	struct ModelEntity {
		int ModelIndex = 0; 
		Vector3f Position, Rotation, Scale; 
		unsigned short LightMapPositionX, LightMapPositionY; 
		Vector2f LightMapTexCoord, LightMapSize; 
		Matrix4f ModelMatrix; 
	};
}