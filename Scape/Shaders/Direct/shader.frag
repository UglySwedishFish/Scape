#version 420
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 
layout(location = 0) out vec3 Lighting;

uniform sampler2D ShadowMaps[5]; 
uniform mat4 ShadowMatrices[5]; 

uniform sampler2D WorldPosition; 
uniform sampler2D Normal; 
uniform sampler2D Albedo; 
uniform sampler2D LightMap; 
uniform sampler2D GrassDirect; 
uniform sampler2D SkyIncident; 

uniform vec3 SunColor; 

uniform vec3 LightDirection; 
uniform int Frame; 
uniform float Time; 
uniform bool UseAlbedo; 


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

				TotalShadow += (texture(ShadowMaps[Pass],ShadowScreenCoord.xy + Direction * Length / SHADOW_MAP_RES).x > ShadowScreenCoord.z-0.000003*(1+Length + OffsetMultiplier[Pass]) ? 1.0 : 0.0);

			}
			return TotalShadow / float(SHADOW_TAPS); 

		}

	} 

	return 1.; 

}


vec3 LightColor = vec3(1.0,0.9,0.7);  
float LightStrenght = 100.; 

vec3 Colors[3] = vec3[](vec3(1.0,0.5,0.25), vec3(0.25,1.0,0.5), vec3(0.5,0.25,1.0));

mat3 ACESInputMat = mat3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat = mat3(
    1.60475, -0.10208, -0.00327,
    -0.53108, 1.10813, -0.07276,
    -0.07367, -0.00605, 1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 Color, float Exposure)
{
    Color.rgb *= Exposure;
    
    Color.rgb = ACESInputMat * Color.rgb;
    Color.rgb = RRTAndODTFit(Color.rgb);
    Color.rgb = ACESOutputMat * Color.rgb;
    Color.rgb = clamp(Color.rgb, 0.0, 1.0);
    Color.rgb = pow( Color.rgb, vec3( 0.45 ) );

    return Color;
}


void main() {
	
	seed = ((TexCoord.x * TexCoord.y)*1000.0) + float(Frame+1) * SHADOW_TAPS * 2; 

	vec3 WorldPos = texture(WorldPosition, TexCoord).xyz; 
	vec4 NormalSample = texture(Normal, TexCoord);

	int Pass = 0;
	float ShadowSample = MultiShadowPass(WorldPos, 0.003, Pass);  

	vec3 Alb = UseAlbedo ? (texture(Albedo, TexCoord).xyz) : vec3(1.0); 
	float ShadowIntensity = UseAlbedo ? 1.0 : 0.0; 

	Lighting = Alb * (ShadowIntensity * ShadowSample * texture(GrassDirect, TexCoord).w * max(dot(NormalSample.xyz, LightDirection), 0.0) * SunColor + texture(LightMap, TexCoord).xyz);
	//Lighting = texture(LightMap, TexCoord).xyz; 

	float L = length(NormalSample.xyz); 

	if(L < 0.5 || L > 1.5) {
		Lighting = texture(SkyIncident, TexCoord).xyz; 
	}

	Lighting = ACESFitted(Lighting, 1.0); 


	

	//Lighting = pow(texture(ShadowMaps[0],TexCoord).xxx,vec3(1000.)); 
}