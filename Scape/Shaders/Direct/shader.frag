#version 420
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 
layout(location = 0) out vec3 Lighting;

uniform sampler2D ShadowMaps[3]; 
uniform mat4 ShadowMatrices[3]; 

uniform sampler2D WorldPosition; 
uniform sampler2D Normal; 
uniform sampler2D Albedo; 
uniform sampler2D LocalLighting; 
uniform sampler2D HeightMap; 
uniform samplerCube Sky; 
uniform sampler2DArray HemisphericalShadowMap; 
uniform sampler2DArray WaterTexture; 
uniform mat4 HemisphericalMatrix[24]; 
uniform vec3 HemiCameraPosition[24]; 

uniform vec3 CameraPosition; 
uniform vec3 FullCameraPosition; 
uniform vec3 LightDirection; 
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

float ComputeCausticFactor(vec3 WorldPos) {
	
	//ray-intersection with an infinite plane 

	//ROy + RDy * x = 40.0 
	//RDy * x = 40.0 - ROy
	//x = (40.0 - ROy) / RDy

	//"technically" approximating the water surface as a solid plane is not correct but... well, meh

	if(WorldPos.y >= WaterHeight || LightDirection.y <= 0.0) 
		return 1.0; 

	vec3 WaterCoordinate = WorldPos + LightDirection * ((40.0 - WorldPos.y) / LightDirection.y); 

	vec4 Sample = SampleInterpolatied(WaterTexture, vec3(WaterCoordinate.xz * 0.05, mod(Time*10.0, 119.))); 

	Sample.xyz = normalize(Sample.xyz); 

	return pow(max(dot(refract(vec3(0.0,-1.0,0.0), Sample.xyz, 1.0/1.33), vec3(0.0,-1.0,0.0)),0.0),48.0); 

}

vec3 ComputeWaterLightLossFactor(vec3 Lighting, vec3 Direction, vec3 Origin, float Traversal) {

	float ActualTraversal = Traversal; 

	if(Origin.y > WaterHeight && Direction.y >= 0.0) {
		return Lighting; 
	}
	else if(Origin.y > WaterHeight && Direction.y < 0.0) {
		float ToWaterTraversal = (40.0-Origin.y) / Direction.y; 
		if(ToWaterTraversal < Traversal) {
			ActualTraversal = ActualTraversal - ToWaterTraversal; 
		}
	}
	else if(Direction.y > 0.0) {
		float ToWaterTraversal = (40.0-Origin.y) / Direction.y; 
		ActualTraversal = min(ToWaterTraversal, ActualTraversal); 
	}

	vec3 Absorption = vec3(0.8,0.08,0.01) + vec3(0.01, 0.025, 0.05) * 2.0; 

	vec3 Transmission = vec3(exp(-Absorption.x * ActualTraversal),exp(-Absorption.y * ActualTraversal),exp(-Absorption.z * ActualTraversal)); 

	return mix(vec3(0.0), Lighting, Transmission); 
	
} 

float MultiShadowPass(vec3 WorldPos, float Offset, out int Pass) {


	float Caustic = ComputeCausticFactor(WorldPos); 


	float HashOffset = 0.9 + 0.1 * hash2().x; 

	for(Pass = 0; Pass < 3; Pass++) {
		
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

				TotalShadow += Caustic * (texture(ShadowMaps[Pass],ShadowScreenCoord.xy + Direction * Length / SHADOW_MAP_RES).x > ShadowScreenCoord.z-0.000003*(1+Length + OffsetMultiplier[Pass]) ? 1.0 : 0.0);

			}
			return TotalShadow / float(SHADOW_TAPS); 

		}

	} 

	return 1.; 

}


vec3 LightColor = vec3(1.0,0.9,0.7);  
float LightStrenght = 100.; 

vec3 Colors[3] = vec3[](vec3(1.0,0.5,0.25), vec3(0.25,1.0,0.5), vec3(0.5,0.25,1.0));


void main() {
	
	seed = ((TexCoord.x * TexCoord.y)*1000.0) + float(Frame+1) * SHADOW_TAPS * 2; 

	vec3 WorldPos = texture(WorldPosition, TexCoord).xyz; 
	vec4 NormalSample = texture(Normal, TexCoord);
	
	vec3 Incident = normalize(CameraPosition - WorldPos); 

	int Pass = 0; 

	WaterHeight = 37.9 + 2.1 * SampleInterpolatied(WaterTexture,vec3(WorldPos.xz * 0.05, mod(Time*10.0, 119.))).w; 



	float ShadowSample = MultiShadowPass(WorldPos, 0.0003, Pass);  
	Lighting = ShadowSample * max(dot(NormalSample.xyz, LightDirection), 0.0) * 3.0 * texture(Sky, LightDirection).xyz;
	Lighting = ComputeWaterLightLossFactor(Lighting, LightDirection, WorldPos, 10000.); 
}