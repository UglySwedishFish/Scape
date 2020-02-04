#version 330
in vec2 TexCoord;

layout(location = 0) out vec3 DiffuseDirection;
layout(location = 1) out vec3 SpecularDirection;

uniform sampler2D Normal;
uniform sampler2D WorldPos;
uniform sampler2D Sobol;
uniform sampler2D Ranking;
uniform sampler2D Scrambling;


uniform int Frame; 
uniform vec3 CameraPosition;

int FetchFromTexture(sampler2D Texture, int Index) {

	int Width = 16384;
	int Height = textureSize(Texture, 0).y;

	ivec2 Pixel = ivec2(Index % Width, Index / Width);

	return clamp(int(texelFetch(Texture, ivec2(Pixel), 0).x * 255. + .1), 0, 255);

}



float samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_32spp(int pixel_i, int pixel_j, int sampleIndex, int sampleDimension)
{
	// wrap arguments
	pixel_i = pixel_i & 127;
	pixel_j = pixel_j & 127;
	sampleIndex = sampleIndex & 255;
	sampleDimension = sampleDimension & 255;

	// xor index based on optimized ranking


	int rankedSampleIndex = sampleIndex ^ FetchFromTexture(Ranking, sampleDimension + (pixel_i + pixel_j * 128) * 8);

	// fetch value in sequence
	int value = FetchFromTexture(Sobol, sampleDimension + rankedSampleIndex * 256);

	// If the dimension is optimized, xor sequence value based on optimized scrambling
	value = value ^ FetchFromTexture(Scrambling, (sampleDimension % 8) + (pixel_i + pixel_j * 128) * 8);

	// convert to float and return
	float v = (0.5f + value) / 256.0f;
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
	}
	else if (abs(normal.y) < 0.57735026919f /* 1 / sqrt(3) */) {
		majorAxis = vec3(0, 1, 0);
	}
	else {
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

vec3 ImportanceGGX(vec2 xi, float roughness)
{
	float r_square = roughness * roughness;
	float phi = 6.2831 * xi.x;
	float cos_theta = sqrt((1 - xi.y) / (1 + (r_square * r_square - 1) * xi.y));
	float sin_theta = sqrt(1 - cos_theta * cos_theta);

	return vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
}

vec3 GetSpecularRayDirection(vec3 RawDirection, vec3 Normal, vec3 Incident, float Roughness) {

	vec3 v0 = abs(Normal.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(0.f, 1.f, 0.f);

	vec3 Tangent = normalize(cross(v0, Normal));
	vec3 Bitangent = normalize(cross(Tangent, Normal));


	for (int Try = 0; Try < 3; Try++) {

		vec2 Xi = hash2(ivec2(gl_FragCoord.xy), Frame, 8 + Try) * vec2(1.f, 0.2f);

		vec3 rho = ImportanceGGX(Xi, clamp(sqrt(Roughness), 0.001f, 1.0f));

		vec3 TryDirection = normalize(0.001f + rho.x * Tangent + rho.y * Bitangent + rho.z * RawDirection);

		if (dot(TryDirection, Normal) > 0.0005f) {
			return TryDirection;
		}

	}
	return RawDirection;
}

void main() {

	ivec2 Pixel = ivec2(gl_FragCoord.xy); 

	vec4 NormRoughness = texture(Normal, TexCoord);
	vec4 WorldPosition = texture(WorldPos, TexCoord);


	vec3 Incident = normalize(WorldPosition.xyz - CameraPosition);

	DiffuseDirection = LambertBRDF(NormRoughness.xyz, hash2(Pixel, Frame, 0)).xyz; 
	SpecularDirection = GetSpecularRayDirection(reflect(Incident, NormRoughness.xyz), NormRoughness.xyz, Incident, NormRoughness.w); 
	DiffuseDirection = SpecularDirection; 
}