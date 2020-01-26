#pragma once

#include "Shader.h"
#include "FrameBuffer.h"



namespace Scape {

	namespace Rendering {


		const unsigned short FoliageDirections = 128; 
		const unsigned short FoliageResolution = 128;
		const float MinStepSize = 0.01f; 
		const float MaxLenght = 100.f; //the max distance the ray can travel  

		//this results in a 64^3 res texture (costing just about 1 mb of vram) 
		//bit of a memory exploit. we can assume the y direction of the normal is always going to point upwards, and thus the normal only needs two components (as the Y direction can then be calculated, as we know the normal is a unit vector)

		const enum class FoliageGeometryType : std::uint8_t {CIRCLE, RECTANGLE };


		struct FoliageGeometry {

			FoliageGeometryType Type = FoliageGeometryType::CIRCLE; 
			Vector2f Size = Vector3f(0.1);
			Vector3f RotationPosition = Vector3f(0.0,0.0,0.0); 
			Vector2f Shift = Vector3f(0.0); //decides how to shift the object (gives a more approximatory 3d shape) 

			FoliageGeometry(FoliageGeometryType Type, Vector2f Size, float Rotation, Vector2f Position, Vector2f Shift): 
				Type(Type), Size(Size), RotationPosition(Vector3f(Rotation, Position)), Shift(Shift)
			{
			}

			FoliageGeometry() = default; 


		};

		struct FoliageBaker {

			
			
			std::vector<FoliageGeometry> Geometry; 
			Shader FoliageShader; 

			
			void BakeFoliage(const std::string & FileName); 
			void AddFoliageGeometry(FoliageGeometry Geometry); 
			void SaveFoliageGeometrySetup(const std::string& Path); 
			void LoadFoliageGeometrySetup(const std::string& Path); 


		};


	}

}
