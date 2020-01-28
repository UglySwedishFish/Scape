#version 330

layout(location = 0) out vec4 Lighting; 
layout(location = 1) out float Depth; 

in vec2 TexCoord;   


uniform sampler2D ShadowMaps[5]; 
uniform mat4 ShadowMatrices[5]; 

uniform sampler2D WorldPosition; 
uniform sampler2D Normal; 
uniform sampler2D Albedo; 
uniform sampler2D LightMap; 

uniform vec3 SunColor; 

uniform vec3 LightDirection; 
uniform vec3 CameraPosition; 
uniform int Frame; 
uniform float Time; 



sampler2D GetShadowCascade(int Index) {
	return Index == 0 ? ShadowMaps[0] : 
			Index == 1 ? ShadowMaps[1] :
			ShadowMaps[2]; 
}


float seed; 

vec2 hash2() {
    return fract(sin(vec2(seed+=0.1,seed+=0.1))*vec2(43758.5453123,22578.1459123));
}

#define SHADOW_MAP_RES 1024
#define SHADOW_TAPS 2

float Multipliers[] = float[3](2.0,4.0,8.0); 
float OffsetMultiplier[] = float[3](1.0, 3.0, 9.0); 

float WaterHeight; 


//computes the caustic factor used for water shading
//this is an approximation 
//we approximate the light lost due to surface refraction to be equal to the dot product between the refracted water light direction and the light direction 



vec4 SampleInterpolatied(sampler2DArray Sampler,vec3 Coord) {


	float BaseTime = mod(Coord.z+max(Coord.x,Coord.y),120.); 

	int Coord1 = int(floor(BaseTime)); 
	int Coord2 = int(ceil(BaseTime)); 

	if(Coord2 == 120) 
		Coord2 = 0; 

	return mix(texture(Sampler, vec3(Coord.xy, Coord1)), texture(Sampler,vec3(Coord.xy, Coord2)), fract(BaseTime)); 


}

float MultiShadowPass(vec3 WorldPos, float Offset, out int Pass) {


	float HashOffset = 0.9 + 0.1 * hash2().x; 

	for(Pass = 0; Pass < 5; Pass++) {
		
		vec4 ShadowCoord = ShadowMatrices[Pass] * vec4(WorldPos, 1.0); 

		vec3 ShadowNDC = ShadowCoord.xyz / ShadowCoord.w; 


		if(abs(ShadowNDC.x) < HashOffset && abs(ShadowNDC.y) < HashOffset) {
			
			float TotalShadow = 0.0; 

			vec3 ShadowScreenCoord = ShadowNDC * .5 + .5; 

			for(int Tap = 0; Tap < SHADOW_TAPS; Tap++) {

				vec2 Hash = hash2(); 

				float Angle = Hash.x * 6.28; 
				vec2 Direction = vec2(cos(Angle), sin(Angle)); 
				float Length = Hash.y * Multipliers[Pass];

				TotalShadow += (texture(ShadowMaps[Pass],ShadowScreenCoord.xy + Direction * Length / SHADOW_MAP_RES).x > ShadowScreenCoord.z-0.000003*(1+Length + OffsetMultiplier[Pass]) ? 1.0 : 0.0);

			}
			return TotalShadow / float(SHADOW_TAPS); 

		}

	} 

	return 1.; 

}

void main() {
	
	seed = ((TexCoord.x * TexCoord.y)*1000.0) + float(Frame+1) * SHADOW_TAPS * 2; 

	vec3 WorldPos = texture(WorldPosition, TexCoord).xyz; 
	vec4 NormalSample = texture(Normal, TexCoord);

	int Pass = 0;
	float ShadowSample = MultiShadowPass(WorldPos, 0.003, Pass);  

	Lighting.xyz = texture(Albedo, TexCoord).xyz * (ShadowSample * max(dot(NormalSample.xyz, LightDirection), 0.0) * SunColor + texture(LightMap, TexCoord).xyz); 
	Lighting.w = 1.0; 
	float L = length(NormalSample.xyz); 

	Depth = min(distance(CameraPosition, WorldPos), 100.f); 

	if(L < 0.5 || L > 1.5) {
		Lighting = vec4(0.0); 
		Depth = 100.f; 
	}
	//Lighting.xyz = texture(Albedo, TexCoord).xyz; 
}