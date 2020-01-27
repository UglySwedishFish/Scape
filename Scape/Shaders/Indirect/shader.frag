/*
Idea: for primary GI tracing - use screen space 
Then, if the ray goes outside of the screen - switch to cubemap data fallback 
If the ray ends up not hitting something (or recording a very clearly false hit), switch to the pre-baked indirect lighting 

should give almost RT quality ~90-75% of the time -> at a fraction of the actual cost. And, when they miss, we have a pretty damn good fallback 

worth noting that this will at first be done at FULL RES. So, its not expected to be particularily cheap. If it becomes to expensive, 
temporal / spatial upscaling from 1/4th res might be viable. Would like to avoid at all cost however. Much rather deal with some inaccuracies. 
might be worth just not tracing for further away objects. (considering the baked indirect likely has more quality) 

*/

#version 330
in vec2 TexCoord; 

layout(location = 0) out vec3 Diffuse; 
layout(location = 1) out vec3 Specular; 

uniform sampler2D LinearZ; 
uniform sampler2D DirectInput; 

uniform samplerCube CubeLighting;
uniform samplerCube CubeLinearZ; 

uniform sampler2D Normal; 
uniform sampler2D WorldPos; 
uniform sampler2D BakedIndirectDiffuse; 

uniform sampler2D Sobol; 
uniform sampler2D Ranking; 
uniform sampler2D Scrambling; 

uniform int Frame; 

int FetchFromTexture(sampler2D Texture, int Index) {
	
	int Width = 16384; 
	int Height = textureSize(Texture, 0).y; 

	ivec2 Pixel = ivec2(Index % Width, Index / Width); 

	return clamp(int(texelFetch(Texture, ivec2(Pixel), 0).x * 255. + .1),0,255); 
	
}


float samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_32spp(int pixel_i, int pixel_j, int sampleIndex, int sampleDimension)
{
	// wrap arguments
	pixel_i = pixel_i & 127;
	pixel_j = pixel_j & 127;
	sampleIndex = sampleIndex & 255;
	sampleDimension = sampleDimension & 255;

	// xor index based on optimized ranking


	int rankedSampleIndex = sampleIndex ^ FetchFromTexture(Ranking,sampleDimension + (pixel_i + pixel_j*128)*8);

	// fetch value in sequence
	int value = FetchFromTexture(Sobol,sampleDimension + rankedSampleIndex*256);

	// If the dimension is optimized, xor sequence value based on optimized scrambling
	value = value ^ FetchFromTexture(Scrambling,(sampleDimension%8) + (pixel_i + pixel_j*128)*8);

	// convert to float and return
	float v = (0.5f+value)/256.0f;
	return v;
}

float hash(ivec2 Pixel, int Index, int Dimension) {

    return samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_32spp(Pixel.x, Pixel.y, Index, Dimension); 

}

vec2 hash2(ivec2 Pixel, int Index, int Dimension) {

    vec2 Returnhash; 
    Returnhash.x = hash(Pixel, Index, 1); 
    Returnhash.y = hash(Pixel, Index, 2);

    return Returnhash;  

}

vec3 TransformToWorld(float x, float y, float z, vec3 normal) {
    // Find an axis that is not parallel to normal
    vec3 majorAxis;
    if (abs(normal.x) < 0.57735026919f /* 1 / sqrt(3) */) {
        majorAxis = vec3(1, 0, 0);
    } else if (abs(normal.y) < 0.57735026919f /* 1 / sqrt(3) */) {
        majorAxis = vec3(0, 1, 0);
    } else {
        majorAxis = vec3(0, 0, 1);
    }

    // Use majorAxis to create a coordinate system relative to world space
    vec3 u = normalize(cross(normal, majorAxis));
    vec3 v = cross(normal, u);
    vec3 w = normal;


    // Transform from local coordinates to world coordinates
    return u * x +
           v * y +
           w * z;
}

vec3 LambertBRDF(vec3 Normal, vec2 Hash) {



    float r = sqrt(Hash.x); 

    float theta = Hash.y * 6.2831;

    float x = r * cos(theta);
    float y = r * sin(theta);

    // Project z up to the unit hemisphere
    float z = sqrt(1.0f - x * x - y * y);

    return normalize(TransformToWorld(x, y, z, Normal));

}

vec2 ScreenSpaceTrace(vec3 ViewSpaceDirection, vec3 ViewSpacePosition) {
	
}

vec3 GetDiffuseIndirect(vec3 Normal, vec3 WorldPos) {

	//First, attempt to utilize existing screen-space data

	//If this fails, try instead to use cubemap data 

	
	//If all fails, just rely on pre-baked indirect lighting (which is still really good quality) 


	//Final fallback -> indirect diffuse 
	return texture(BakedIndirectDiffuse, TexCoord).xyz; 
	
}



void main() {

	vec4 NormRoughness = texture(Normal, TexCoord); 
	vec3 WorldPosition = texture(WorldPos, TexCoord).xyz; 
	







	
	

	

}