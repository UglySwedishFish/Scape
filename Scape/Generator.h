#pragma once

#include <vector>
#include "Entity.h"
#include "DependenciesRendering.h"
#include <array>
#include <tuple> 

namespace Scape {
		

		const int CHUNK_SIZE = 48; 


		const std::array<int, 5> LightBakingTextureResolutions = { 8,16,32,64,128 };
		enum class LightBakingQuality : std::uint8_t { LOW, MEDIUM, HIGH, ULTRA, TERRAIN };

		const enum class Objects : int {MonkeyHead, Torus, Sphere, Size};
		const std::array<LightBakingQuality, static_cast<int>(Objects::Size)> ObjectLightMapQuality{
			LightBakingQuality::HIGH,
			LightBakingQuality::HIGH,
			LightBakingQuality::MEDIUM
		}; 
		const std::array<std::tuple<std::string, std::string>, static_cast<int>(Objects::Size)> ObjectLocations = {
			std::make_tuple("Monkey.obj","Monkey.obj"),
			std::make_tuple("Plane.obj", "Plane.obj"),
			std::make_tuple("Sphere.obj", "Sphere.obj")
		}; 


		struct Generator {

			//handles generation of ALL models (from trees to larger grass and everything inbetween) 
			
			void PrepareGenerator(); 
			std::vector<ModelEntity> GetGeneratedModels(int PositionX, int PositionZ);
			float GetHeightAt(Vector2f Position); 

		private: 
			sf::Image PerlinNoise, FractalNoise, VoroniNoise; 
			
		};

}

