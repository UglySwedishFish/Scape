#version 420
#extension GL_ARB_bindless_texture : enable

uniform int Frame; 

uniform sampler2D RayDirection; 
uniform sampler2D RayOrigin; 
uniform sampler2D RayHitDataUVWM; 
uniform sampler2D RayHitDataNormal; 
uniform sampler2D PreviousImageDataAO; 
layout(RGBA8) uniform writeonly image2D AOImage;

uniform sampler2D MaterialData; 
uniform sampler2DArray Textures; 

uniform sampler2DArray PreviousImageDataLighting; 
layout(RGBA8) uniform writeonly image2DArray LightingImages; 

uniform sampler2D ShadowMaps[16]; 
uniform mat4 ShadowMatrices[16]; 
uniform int ShadowCount; 
uniform vec3 LightDirection[16]; 

void main() {
	


	ivec2 Pixel = ivec2(gl_FragCoord.xy); 

	vec4 Origin = texelFetch(RayOrigin, Pixel, 0); 
	vec4 Direction = texelFetch(RayDirection, Pixel, 0); 
	vec4 HitUVWM = texelFetch(RayHitDataUVWM, Pixel, 0); 
	vec4 HitNormal = texelFetch(RayHitDataNormal, Pixel, 0); 
	vec4 Previous = texelFetch(PreviousImageDataAO, Pixel, 0); 

	vec3 Current = vec3(1.0); 

	if(HitUVWM.z > 0.0) {


		vec3 HitLocation = Origin.xyz + Direction.xyz * HitUVWM.z; 
		
		//handle hit! 

		vec4 MaterialData = texelFetch(MaterialData, ivec2(int(HitUVWM.w),0),0); 

		vec3 Color = MaterialData.xyz; 

		if(MaterialData.x < -0.5) {
			int Texture = int(MaterialData.y + .1); 
			Color = pow(texture(Textures, vec3(HitUVWM.xy, Texture & 255)).xyz,vec3(2.2)); 
		}

		for(int GIImage = 0; GIImage < ShadowCount; GIImage++) {
		
			vec4 ShadowClip = ShadowMatrices[GIImage] * vec4(HitLocation.xyz, 1.0); 
			vec3 ShadowNDC = ShadowClip.xyz / ShadowClip.w; 

			vec3 CurrentLighting = vec3(0.0); 
			vec3 PreviousLighting = texelFetch(PreviousImageDataLighting, ivec3(Pixel, GIImage), 0).xyz; 

			if(abs(ShadowNDC.x) < 1.0 && abs(ShadowNDC.y) < 1.0) {
				CurrentLighting = Color * max(dot(LightDirection[GIImage], HitNormal.xyz),0.0)  * (texture(ShadowMaps[GIImage], ShadowNDC.xy * 0.5 + 0.5).x > (ShadowNDC.z * 0.5 + 0.5)-0.000003 ? 1.0 : 0.0); 
			}
			
			imageStore(LightingImages, ivec3(Pixel, GIImage), vec4(mix(CurrentLighting.xyz, PreviousLighting.xyz, float(Frame) / float(Frame+1)), 0.0));

			

		}
 
		Current.x = 0.0; 
		Current.y = pow(clamp(HitUVWM.z * 0.75, 0.0,1.0), 2.0); 
		Current.z = 0.; 
	}
	else {
		for(int GIImage = 0; GIImage < ShadowCount; GIImage++) {
	
			vec3 CurrentLighting = vec3(0.); 
			vec3 PreviousLighting = texelFetch(PreviousImageDataLighting, ivec3(Pixel, GIImage), 0).xyz; 

			imageStore(LightingImages, ivec3(Pixel, GIImage), vec4(mix(CurrentLighting.xyz, PreviousLighting.xyz, float(Frame) / float(Frame+1)), 0.0));

		}
	}
	imageStore(AOImage, Pixel, vec4(mix(Current.xyz, Previous.xyz, float(Frame) / float(Frame+1)), 0.0)); 

}