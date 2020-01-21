#ifdef __cplusplus

#pragma once



#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>
#include <DependenciesRendering.h>
#include <iostream>


//^ this is for improved cross compatability with GLSL (as we don't want to have to write the same code twice)

namespace Scape {
    namespace Rendering {
        namespace GLSLComp {
            using namespace glm;
#endif

#define WORLD_SIZE 128.
#define WORLD_HEIGHT 100.

#ifdef __cplusplus

			using sampler2D = sf::Image*; 
		

			ivec2 textureSize(sampler2D Sampler, int MipLevel) {

				return ivec2(Sampler->getSize().x, Sampler->getSize().y); 

			}

			vec4 texelFetch(sampler2D Sampler, ivec2 Texel, int mip) { //mip does not do anything!!!
				//assume wrapping mode is set to GL_REPEAT 
				Texel = Texel % textureSize(Sampler,0);
			
				auto Pixel = Sampler->getPixel(Texel.x, Texel.y); 
				return vec4(Pixel.r, Pixel.g, Pixel.b, Pixel.a) / 255.f; 
			}

			vec4 texture(sampler2D Sampler, vec2 TextureCoordinate) {
				//assume this is only linear interpolation 

				TextureCoordinate = fract(TextureCoordinate); 
			//	TextureCoordinate.y = 1.0 - TextureCoordinate.y; 
				ivec2 Size = textureSize(Sampler, 0); 

				vec2 Fraction = fract(TextureCoordinate * vec2(Size)); 

				ivec2 Texel00 = clamp(ivec2(TextureCoordinate * vec2(Size)), ivec2(0), Size - 1); 
				ivec2 Texel11 = Texel00 + 1;
				ivec2 Texel10 = ivec2(Texel11.x, Texel00.y); 
				ivec2 Texel01 = ivec2(Texel00.x, Texel11.y); 

				vec4 Sample1 = texelFetch(Sampler, Texel00, 0); 
				vec4 Sample2 = texelFetch(Sampler, Texel10, 0); 
				vec4 Sample3 = texelFetch(Sampler, Texel01, 0); 
				vec4 Sample4 = texelFetch(Sampler, Texel11, 0); 

				return mix(mix(Sample1, Sample2, Fraction.x), mix(Sample3, Sample4, Fraction.x), Fraction.y); 
				
			}

			vec4 textureLod(sampler2D Sampler, vec2 TextureCoordinate, float MipLevel) {
				return texture(Sampler, TextureCoordinate); 
			}

			

#endif



			float GetFractalNoise(vec2 TexCoord, sampler2D NoiseSampler, int Iter, float Multiplier) {

				float WeightNow = 0.5; 
				float TotalHeight = 0.0; 
				float TotalWeight = 0.0; 


				for (int Iteration = 0; Iteration < Iter; Iteration++) {
					TotalWeight += WeightNow; 
					TotalHeight += texture(NoiseSampler, TexCoord).x * WeightNow;
					TexCoord *= Multiplier; 
					WeightNow *= 0.5f; 
				}

				if (Iter == 0) //avoid division by 0!
					return 0.0; 
				else 
					return TotalHeight / TotalWeight; 
			}





#define BIOME_PLAINS 2
#define BIOME_BEACH 1
#define BIOME_UNDERWATER 0


#define BIOME_DESERT_DENSEROCKS 0
#define BIOME_DESERT_ROCKY 1
#define BIOME_DESERT_SAND 2
#define BIOME_DESERT_HARD 3

#define BIOME_FOREST_STICKS 4
#define BIOME_FOREST_MOSS 5
#define BIOME_FOREST_MUD 6
#define BIOME_FOREST_ROCK 7

#define PRIMARY_BIOME_DESERT 0


			struct TerrainData {

				float Height; 
				vec4 TextureMixes; 
				vec4 Textures; 

			};


			float VoroniDunesFunction(vec2 Coord, sampler2D VoroniNoise) {
				float Height = pow(max(texture(VoroniNoise, Coord).x - texture(VoroniNoise, -vec2(Coord.y, Coord.x) * 1.7f).x * 0.65, 0.0), 4.0); 
				Height = pow(min(max(Height - 0.001, 0.0) * 10.0, 1.0), 0.125); 
				return max(Height - 0.65, 0.0) * 2.85714286; 
			}

			float PathFunction(vec2 Coord, sampler2D PerlinNoise) {
				float Height = texture(PerlinNoise, Coord * 0.000834375f).x; 
				return pow(1.0 - abs(Height * 2. - 1.), 8.0f); 
			}


			TerrainData GetActualHeight(vec2 Pos, sampler2D VoroniNoise, sampler2D PerlinNoise, sampler2D FractalNoise) {
				
				TerrainData Returner; 

				//figure out our primary biome 

				int PrimaryBiome = PRIMARY_BIOME_DESERT; //todo: implement multiple biome types 



				if (PrimaryBiome == PRIMARY_BIOME_DESERT) {

					//what about our secondary biome? (how are we doing in this context) 
					float BiomeFactorHard = 1.0;

					//sandy dunes! 
					float BiomeFactorSandDunes = VoroniDunesFunction(Pos * 0.0015f, VoroniNoise); 
					float BiomeFactorSticks = PathFunction(Pos, PerlinNoise) * pow( 1.0 - BiomeFactorSandDunes, 2.0f);


					//to start of with, lets begin with our base desert (which is just sand) 



					BiomeFactorHard = max(BiomeFactorHard - min(BiomeFactorSandDunes*2.0f,1.0f) - BiomeFactorSticks, 0.0f);

					float BiomeFactorSand = 1.0; 
					BiomeFactorSand -= BiomeFactorHard; 
					BiomeFactorSand -= BiomeFactorSticks; 
					BiomeFactorSand = max(BiomeFactorSand, 0.0f); 
					Returner.Textures.x = float(BIOME_DESERT_SAND);
					Returner.Textures.y = float(BIOME_DESERT_HARD);
					Returner.Textures.z = float(BIOME_FOREST_STICKS);

					Returner.TextureMixes = vec4(BiomeFactorSand, BiomeFactorHard, BiomeFactorSticks, 0.0);

					Returner.Height = 41.0 + max(BiomeFactorSandDunes * 10.0f - texture(PerlinNoise,Pos*0.001f).x * 3.0f,BiomeFactorSandDunes * 0.2f) - pow(BiomeFactorSticks,2.0f) * 5.0;





				}


				return Returner; 

			//	return 40.0 + (1.0- GetFractalNoise(Pos * 0.0004f,VoroniNoise,4,1.71)) * 80.0f;









			}


#ifdef __cplusplus
        }
    }
}
#endif