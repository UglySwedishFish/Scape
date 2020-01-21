#version 420
#extension GL_ARB_bindless_texture : enable

uniform int Frame; 

uniform sampler2D RayDirection; 
uniform sampler2D RayOrigin; 
uniform sampler2D RayHitDataUVWM; 
uniform sampler2D RayHitDataNormal; 
uniform sampler2D PreviousImageDataAO; 

//todo: 
//uniform sampler2DArray PreviousImageDataLighting
//uniform sampler2DShadow ShadowMaps[16]; 
//uniform mat4 ShadowMatrices[16]; 
//uniform int ShadowCount; 
//layout(RGBA8) uniform writeonly image2DArray LightingImages

layout(RGBA8) uniform writeonly image2D AOImage;

void main() {

	ivec2 Pixel = ivec2(gl_FragCoord.xy); 

	vec4 Origin = texelFetch(RayOrigin, Pixel, 0); 
	vec4 Direction = texelFetch(RayDirection, Pixel, 0); 
	vec4 HitUVWM = texelFetch(RayHitDataUVWM, Pixel, 0); 
	vec4 HitNormal = texelFetch(RayHitDataNormal, Pixel, 0); 
	vec4 Previous = texelFetch(PreviousImageDataAO, Pixel, 0); 

	vec3 Current = vec3(1.0); 

	if(HitUVWM.z > 0.0) {
		Current.x = 0.0;
		Current.y = pow(clamp(HitUVWM.z * 0.75, 0.0, 1.0), 2.0); 
	}

	imageStore(AOImage, Pixel, vec4(mix(Current.xyz, Previous.xyz, float(Frame) / float(Frame+1)), 0.0)); 

}