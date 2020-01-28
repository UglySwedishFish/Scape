#include "Generator.h"
#include "Shaders/Generator/NoiseGenerator.h"

namespace Scape {

	void Generator::PrepareGenerator()
	{
		PerlinNoise.loadFromFile("Resources/Perlin.png"); 
		FractalNoise.loadFromFile("Resources/Fractal.png"); 
		VoroniNoise.loadFromFile("Resources/Voroni.png"); 
	}
	std::vector<ModelEntity> Generator::GetGeneratedModels(int PositionX, int PositionZ)
	{
		
		//for now, let's just put a few entities all around the chunk 

		std::vector<ModelEntity> FinalEntities; 

		for (int Entity = 0; Entity < 8; Entity++) {

			ModelEntity ThisEntity; 

			Vector2f PositionXZ = -(Vector2f(PositionX, PositionZ) + Vector2f(rand(), rand()) / Vector2f(RAND_MAX)) * Vector2f(CHUNK_SIZE); 
			float Height = GetHeightAt(PositionXZ); 

			ThisEntity.Position = Vector3f(PositionXZ.x, 0.0, PositionXZ.y); 
			ThisEntity.Rotation = Vector3f(0.0); 
			ThisEntity.Scale = Vector3f(1.0); 
			ThisEntity.ModelMatrix = Core::ModelMatrix(ThisEntity.Position, ThisEntity.Rotation, ThisEntity.Scale); 
			ThisEntity.ModelIndex = 0; 

			FinalEntities.push_back(ThisEntity); 

			if (ThisEntity.ModelIndex == 0) {

				ThisEntity.ModelIndex = 1; 
				FinalEntities.push_back(ThisEntity);

			}

		}

		return FinalEntities; 

	}

	float Generator::GetHeightAt(Vector2f Position)
	{
		return 0.f; 
		return Rendering::GLSLComp::GetActualHeight(Position, &VoroniNoise, &PerlinNoise, &FractalNoise).Height; 
	}
}