#version 330

layout(location = 0) out vec4 OutAlbedo; 
layout(location = 1) out vec4 OutNormal; 
layout(location = 2) out vec4 WorldPosition; 
layout(location = 3) out vec3 Lighting; 


in vec2 TexCoord;
in vec3 Position;
in vec3 Normal; 
in vec3 Tang; 
in vec2 LightMapTC; 

uniform sampler2D LightMap; 
uniform samplerCube Sky; 
uniform sampler2DArray LightMapGI; 
uniform sampler2D DirtTexture; 

uniform bool HasAlbedo; 
uniform bool HasNormal; 
uniform bool HasRough; 
uniform bool HasMet; 
uniform bool HasEmissive; 

uniform vec3 Albedo; 
uniform float Metalness; 
uniform float Roughness; 
uniform float InputEmission; 

uniform float TimeOfDay; 
uniform vec3 SunColor; 
uniform int LightingZones; 


void main() {


	OutAlbedo.xyz = pow(texture(DirtTexture, TexCoord * 2.0).xyz,vec3(2.2)); 
	OutNormal.xyz = Normal; 
	WorldPosition.xyz = Position; 


	vec3 LightMap = texture(LightMap, LightMapTC).xyz; 

	//whats our GI lookin like?= 

	vec3 LightGIA, LightGIB, LightGIFinal = vec3(0.0); 
	float LightGIMixFactor; 


	if(TimeOfDay < 43200. && TimeOfDay > 0.0) {
		
		float TimeAddon = 43200 / float(LightingZones); 

		if(TimeOfDay < TimeAddon) {
			LightGIA = vec3(0.0); 
			LightGIB = texture(LightMapGI, vec3(LightMapTC, 0.0)).xyz; 
		}
		else if(TimeOfDay + TimeAddon > 43200.) {
			LightGIA = texture(LightMapGI, vec3(LightMapTC, float(LightingZones-3))).xyz; 
			LightGIB = vec3(0.0); 
		}
		else {
			int Clamped = int((TimeOfDay / 43200.f) * LightingZones);
			
			LightGIA = texture(LightMapGI, vec3(LightMapTC, Clamped-1)).xyz; 
			LightGIB = texture(LightMapGI, vec3(LightMapTC, Clamped)).xyz; 

		}

		LightGIMixFactor = fract(TimeOfDay / TimeAddon); 

		LightGIFinal = mix(LightGIA, LightGIB, LightGIMixFactor); 

	}
	



	Lighting = LightMap.x * textureLod(Sky, OutNormal.xyz,4.0).xyz; 
	Lighting += LightGIFinal * SunColor; 
	Lighting *= LightMap.y; 

	//OutAlbedo.xyz = Lighting; 
	
	//Lighting = texture(LightMapGI, vec3(LightMapTC, 4.0)).xyz; 
}