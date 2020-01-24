#pragma once

#include "Shader.h"
#include "FrameBuffer.h"



namespace Scape {

	namespace Rendering {


		unsigned short FoliageDirections = 64; 
		unsigned short FoliageResolution = 64; 
		//this results in a 64^3 res texture (costing just about 1 mb of vram) 


		enum class FoliageGeometryType : std::uint8_t {CIRCLE, RECTANGLE };


		struct FoliageGeometry {

			FoliageGeometryType Type = FoliageGeometryType::CIRCLE; 
			Vector2f Size;
			Vector3f RotationPosition; 

			FoliageGeometry(FoliageGeometryType Type, Vector2f Size, float Rotation, Vector2f Position): 
				Type(Type), Size(Size), RotationPosition(Vector3f(Rotation, Position))
			{
			}

			FoliageGeometry() = default; 


		};

		struct FoliageBaker {

			std::vector<FoliageGeometry> Geometry; 
			Shader FoliageBaker; 



			


		};


	}

}
