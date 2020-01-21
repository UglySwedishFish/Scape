#version 420

in vec2 TexCoord; 
layout(location = 0) out vec4 DiffuseRay;
layout(location = 1) out vec4 Origin;

uniform sampler2D WorldPosition; 
uniform sampler2D Normal; 
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


void main() {
	
	ivec2 Pixel = ivec2(gl_FragCoord.xy); 
	
	vec3 WorldPosition = texelFetch(WorldPosition, Pixel, 0).xyz; 
	vec4 NormalRoughness = texelFetch(Normal, Pixel, 0); 

	float L = length(NormalRoughness.xyz); 
	DiffuseRay = vec4(0.0,1.0,0.0,-1.0); 
	Origin = vec4(0.0,1.0,0.0,-1.0); 

	if(L >= 0.2 && L <= 1.75) {
		NormalRoughness.xyz /= L; 
		Origin = vec4(vec3(WorldPosition.x,WorldPosition.y, WorldPosition.z) + NormalRoughness.xyz * 0.025, 0.); 
		vec2 BlueHash = hash2(Pixel, Frame, 1); 
		DiffuseRay.xyz = LambertBRDF(NormalRoughness.xyz, BlueHash); 
		DiffuseRay.w = 1.0; 
	}

	if(abs(DiffuseRay.x) < 0.01) 
		DiffuseRay.x = 0.01; 

	if(abs(DiffuseRay.y) < 0.01) 
		DiffuseRay.y = 0.01; 

	if(abs(DiffuseRay.z) < 0.01) 
		DiffuseRay.z = 0.01; 

	DiffuseRay.xyz = normalize(DiffuseRay.xyz); 


}