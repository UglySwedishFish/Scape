#include "MinifoliageBaker.h"
#include <fstream>
#include <sstream>


//we do not particularily care for storage when it comes to caching the geometry for the baker
//since its a lot less than the actual cached results 

//So here's the syntax: 

//Type: [CIRCLE/RECTANGLE] Position: [X] [Y] Size: [X] [Y] Rotation [Axis] Shift [X] [Y] 

namespace Scape {

	namespace Rendering {

		void FoliageBaker::BakeFoliage(const std::string& ExportDir, const std::string& FileName)
		{

			





		}

		void FoliageBaker::AddFoliageGeometry(FoliageGeometry Geometry)
		{
		}

		void FoliageBaker::SaveFoliageGeometrySetup(const std::string& Path)
		{

			std::fstream FileOut; 
			FileOut.open(Path); 

			for (auto& Geometry : Geometry) {
				FileOut << "Type: "; 
				switch (Geometry.Type) {

				case FoliageGeometryType::RECTANGLE: 
					FileOut << "RECTANGLE"; 
				break; 

				case FoliageGeometryType::CIRCLE:
					FileOut << "CIRCLE"; 
				break; 

				default: 
					FileOut << "Unknown"; 
					break; 

				}
				FileOut << " Position: " << Geometry.RotationPosition.y << ' ' << Geometry.RotationPosition.z; 
				FileOut << " Size: " << Geometry.Size.x << ' ' << Geometry.Size.y; 
				FileOut << " Rotation: " << Geometry.RotationPosition.x; 
				FileOut << " Shift:" << Geometry.Shift.x << ' ' << Geometry.Shift.y; 
				FileOut << '\n';	
			}
			FileOut.close(); 
			
		}

		void FoliageBaker::LoadFoliageGeometrySetup(const std::string& Path)
		{
		}

	}


}