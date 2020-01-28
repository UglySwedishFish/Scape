#version 420

in vec2 TexCoord; 
layout(location = 0) out vec4 Lighting;

uniform sampler2D CombinedLighting;
uniform sampler2D PreviousLighting; 
uniform sampler2D WorldPosPrevious;
uniform sampler2D PreviousWorldPos; 
uniform sampler2D CurrentNormal; 
uniform sampler2D PreviousNormal; 
uniform vec3 CameraPosition; 

uniform mat4 MotionMatrix; 


vec2 Resolution = vec2(1920, 1080); 

vec2 FindBestPixel(vec2 Coord, vec3 BaseWorldPosition, float Distance) {
	

	float BestDistance = 10000000.0; 
	
	vec3 BestWorldPosition = vec3(-100000.0); 

	vec2 BestCoord = ivec2(-1); 

	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			
			vec2 CurrentCoord = vec2(Coord) + vec2(x,y)/Resolution; 
			vec3 CurrentWorldPosition = texture(PreviousWorldPos, CurrentCoord, 0).xyz; 

			float CurrentDistance = distance(CurrentWorldPosition,BaseWorldPosition) * (12.0 + length(vec2(x,y))); 

			if(CurrentDistance < BestDistance) {
				BestDistance = CurrentDistance; 
				BestWorldPosition = CurrentWorldPosition; 
				BestCoord = CurrentCoord; 
			}


		}
	}

	if(abs(BestWorldPosition.x - BaseWorldPosition.x) / Distance < 0.1 && abs(BestWorldPosition.y - BaseWorldPosition.y) / Distance < 0.1 && abs(BestWorldPosition.z - BaseWorldPosition.z) / Distance < 0.1)
		return BestCoord; 
	return vec2(-1); 

}



vec4 GetClamped(sampler2D Current, sampler2D Previous, vec2 PreviousTexCoord) {
	
	return texture(Previous, PreviousTexCoord); 

	ivec2 Coord = ivec2(gl_FragCoord.xy); 

	vec4 Min = vec4(10000.0); 
	vec4 Max = vec4(-10000.0); 

	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {

			vec4 Fetch = texelFetch(Current,Coord + ivec2(x,y), 0); 

			Min = min(Min, Fetch); 
			Max = max(Max, Fetch); 
		}
	}
	
	return clamp(texture(Previous, PreviousTexCoord), Min, Max); 

}

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

	
	vec3 WorldSpace = texture(WorldPosPrevious, TexCoord).xyz; 

	vec4 ClipSpace = MotionMatrix * vec4(WorldSpace, 1.0); 
	ClipSpace.xyz /= ClipSpace.w; 

	vec2 LookUpCoordinate = vec2(-1.0); 
	float ClampFactor = .0; 


	if(abs(ClipSpace.x) < 1.0 && abs(ClipSpace.y) < 1.0) {
		
		LookUpCoordinate = ClipSpace.xy * 0.5 + 0.5; 
		vec2 Best = FindBestPixel(LookUpCoordinate, WorldSpace,max(distance(WorldSpace, CameraPosition.xyz),1.0)); 

		if(Best.x > 0.0 && Best.y > 0.0) {
			ClampFactor = 0.9667; 
			//LookUpCoordinate = Best; 
		}

		
	}


	

	float PreviousFrame = texture(PreviousLighting, TexCoord).w + 1.0; 

	vec3 CurrentLighting = ACESFitted(texture(CombinedLighting, TexCoord).xyz,1.0); 

	Lighting.xyz = mix(CurrentLighting, GetClamped(CombinedLighting, PreviousLighting, LookUpCoordinate).xyz,min(PreviousFrame / (PreviousFrame+1.0), ClampFactor)); 
	Lighting.w = PreviousFrame; 

	
}

