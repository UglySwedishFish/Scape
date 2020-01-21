#version 330

layout(location = 0) out vec4 OutAlbedo; 
layout(location = 1) out vec4 OutNormal; 
layout(location = 2) out vec4 WorldPosition; 
layout(location = 3) out float Emission; 


in vec2 TexCoord;
in vec3 Position;
in vec3 Normal; 
in vec3 Tang; 
in vec2 LightMapTC; 

uniform sampler2D AlbedoMap; 
uniform sampler2D NormalMap; 
uniform sampler2D RoughnessMap; 
uniform sampler2D MetalnessMap; 
uniform sampler2D EmissiveMap; 
uniform sampler2D LightMap; 
uniform samplerCube Sky; 


uniform bool HasAlbedo; 
uniform bool HasNormal; 
uniform bool HasRough; 
uniform bool HasMet; 
uniform bool HasEmissive; 

uniform vec3 Albedo; 
uniform float Metalness; 
uniform float Roughness; 
uniform float InputEmission; 

vec3 CalcNormalMappedNormal(float intensity,mat3 TBN)  { //for normal maps 
    return mix(Normal,(TBN * (texture(NormalMap,TexCoord).rgb * 2. -1.)),intensity);
}

void main() {


	if(HasAlbedo) 
		OutAlbedo.xyz = pow(texture(AlbedoMap, TexCoord).xyz,vec3(2.2)); 
	else 
		OutAlbedo.xyz = Albedo; 


	if(HasMet) 
		OutAlbedo.w = texture(MetalnessMap, TexCoord).x; 
	else
		OutAlbedo.w = Metalness; 

	OutNormal = vec4(Normal, 0.0); 
	OutNormal.xyz = normalize(OutNormal.xyz); 

	vec3 Tangent = normalize(Tang - dot(Tang, Normal) * Normal);
    vec3 Bitangent = normalize(cross(Tang, Normal));

	mat3 TBN = mat3(Tangent, Bitangent, Normal); 

	if(HasNormal) 
		OutNormal.xyz = normalize(CalcNormalMappedNormal(0.5, TBN)); 


	if(HasRough) 
		OutNormal.w = texture(RoughnessMap, TexCoord).x;
	else
		OutNormal.w = Roughness; 
		
	

	WorldPosition = vec4(Position, OutAlbedo.w);
	Emission = InputEmission; 
	//OutAlbedo.xyz = OutNormal.xyz; 

	vec3 LightMap = texture(LightMap, LightMapTC).xyz; 

	OutAlbedo.xyz *= LightMap.xxx * LightMap.y; 
	OutAlbedo.xyz *= textureLod(Sky, OutNormal.xyz,4.0).xyz; 
	OutAlbedo.xyz = pow(OutAlbedo.xyz, vec3(0.45454545)); 


}