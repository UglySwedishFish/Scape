#version 420

in vec2 TexCoord; 
layout(location = 0) out vec4 Height;
layout(location = 1) out vec4 Textures;
layout(location = 2) out vec3 Mixes; //last mix can be computed as 1.0 - (mix.x+mix.y+mix.z) 
//as they all have to equal 1 once combined 


uniform vec2 Offset; 

#include generator/NoiseGenerator.h


uniform sampler2D Voroni; 
uniform sampler2D Perlin; 
uniform sampler2D Fractal; 



void main() {
    TerrainData Data = GetActualHeight((TexCoord * 2.0 - 1.0) * WORLD_SIZE + Offset, Voroni, Perlin, Fractal);    
	Height = vec4(Data.Height);
	Textures = Data.Textures; 
	Mixes = Data.TextureMixes.xyz; 
}
