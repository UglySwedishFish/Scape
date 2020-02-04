/*
Idea: for primary GI tracing - use screen space
Then, if the ray goes outside of the screen - switch to cubemap data fallback
If the ray ends up not hitting something (or recording a very clearly false hit), switch to the pre-baked indirect lighting

should give almost RT quality ~90-75% of the time -> at a fraction of the actual cost. And, when they miss, we have a pretty damn good fallback

worth noting that this will at first be done at FULL RES. So, its not expected to be particularily cheap. If it becomes to expensive,
temporal / spatial upscaling from 1/4th res might be viable. Would like to avoid at all cost however. Much rather deal with some inaccuracies.
might be worth just not tracing for further away objects. (considering the baked indirect likely has more quality)

*/

#version 330
in vec2 TexCoord;

layout(location = 0) out float Diffuse;
layout(location = 1) out float Specular;

uniform sampler2D ViewSpaceZ;
uniform sampler2D Depth;
uniform sampler2D DirectInput;

uniform samplerCube CubeLighting;
uniform samplerCube CubeLinearZ;

uniform sampler2D Normal;
uniform sampler2D WorldPos;
uniform sampler2D BakedIndirectDiffuse;
uniform sampler2D Albedo;

uniform sampler2D Sobol;
uniform sampler2D Ranking;
uniform sampler2D Scrambling;

uniform samplerCube Sky;

uniform sampler2D DiffuseDirection; 
uniform sampler2D SpecularDirection; 

uniform int Frame;

uniform mat4 InverseView;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform vec3 CameraPosition;
uniform bool UseNewMethod;

uniform float zFar;
uniform float zNear;

int FetchFromTexture(sampler2D Texture, int Index) {

	int Width = 16384;
	int Height = textureSize(Texture, 0).y;

	ivec2 Pixel = ivec2(Index % Width, Index / Width);

	return clamp(int(texelFetch(Texture, ivec2(Pixel), 0).x * 255. + .1), 0, 255);

}

float LinearlizeDepth(float z) {
	return 2.0 * zNear * zFar / (zFar + zNear - (z * 2.0 - 1.0) * (zFar - zNear));
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


ivec2 Resolution;


float rsi(vec3 r0, vec3 rd, vec3 s0, float sr) {

	float a = dot(rd, rd);
	vec3 s0_r0 = r0 - s0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sr * sr);
	if (b * b - 4.0 * a * c < 0.0) {
		return -1.0;
	}
	return (-b - sqrt((b * b) - 4.0 * a * c)) / (2.0 * a);
}

float TraceCubeMap(vec3 Direction, vec3 Origin, int Steps) {


	//figure out the max traversal distance 



	/*float TraversalDistance = rsi(Origin, Direction, CameraPosition, 100.f); //gives us max traversal distance :)

	if(TraversalDistance < 0.0)
		return vec3(-5.0f); */

	float TraversalDistance = 64.f;


	vec3 Step = (Direction * TraversalDistance) / float(Steps);

	float TraversalStep = TraversalDistance / float(Steps); 

	float Traversal =  clamp(hash(ivec2(gl_FragCoord.xy), Frame, 4), .0, 1.0); 


	vec3 Position = Origin + Step * Traversal;

	Traversal *= TraversalStep; 



	float PreviousZ = length(Position - CameraPosition);

	for (int i = 1; i < Steps; i++) {

		Position += Step;
		Traversal += TraversalStep; 

		vec3 Incident = Position - CameraPosition;
		float L = length(Incident);
		float Depth = texture(CubeLinearZ, Incident / L).x;

		if (Depth < L) {

			vec3 StepLBinarySearch = Step * 0.5; 
			float TraversalStepBinarySearch = TraversalStep * 0.5; 
			Position -= StepLBinarySearch; 
			Traversal -= TraversalStepBinarySearch; 


			for (int StepBinarySearch = 0; StepBinarySearch < 6; StepBinarySearch++) {
				
				StepLBinarySearch *= 0.5; 
				TraversalStepBinarySearch *= 0.5; 

				Incident = Position - CameraPosition; 

				L = length(Incident); 

				Depth = texture(CubeLinearZ, Incident / L).x; 

				if(Depth < L) {
					Traversal -= TraversalStepBinarySearch; 
					Position -= StepLBinarySearch; 
				}
				else {
					Traversal += TraversalStepBinarySearch; 
					Position += StepLBinarySearch; 
				}
			}

			if (abs(Depth - L) < abs(PreviousZ - L) * 12.0) {
				return Traversal;
			}

			return -10.0f;

		}

		PreviousZ = L;

	}

	//are we ok still? 

	return -5.0;





}

vec3 SSRT(vec3 Direction, vec3 Origin, int Steps) {

	//initially -> figure out how FAR the ray should actually go! 

	vec3 ViewSpaceDirection = vec3(vec4(Direction.xyz, 0.0) * InverseView);
	vec3 ViewSpacePosition = (ViewMatrix * vec4(Origin, 1.0)).xyz;

	float PixelDistance = distance(Origin, CameraPosition);

	vec4 RayStartScreen = ProjectionMatrix * vec4(ViewSpacePosition, 1.0);
	RayStartScreen.xyz /= RayStartScreen.w; RayStartScreen = RayStartScreen * 0.5 + 0.5;

	vec4 RayEndScreen = ProjectionMatrix * vec4(ViewSpacePosition + ViewSpaceDirection * .01, 1.0);
	RayEndScreen.xyz /= RayEndScreen.w; RayEndScreen = RayEndScreen * 0.5 + 0.5;

	RayEndScreen.z = LinearlizeDepth(RayEndScreen.z);
	RayStartScreen.z = LinearlizeDepth(RayStartScreen.z);


	vec4 RayDirScreen = (RayEndScreen - RayStartScreen);
	//RayDirScreen.xy = max(abs(RayDirScreen.xy), 1.0 / vec2(Resolution)) * sign(RayDirScreen.xy); 




	//figure out how long you have to travel to hit the edge of the screen 

	float TraversalDistance = -1.0f;


	//approximates the required traversal distance (not 100% accurate) 

	if (RayDirScreen.x > 0.0) {
		TraversalDistance = (1.0 - RayStartScreen.x) / RayDirScreen.x;

	}
	else {
		TraversalDistance = (-RayStartScreen.x) / RayDirScreen.x;
	}


	if (RayDirScreen.y > 0.0) {
		TraversalDistance = min(TraversalDistance, (1.0 - RayStartScreen.y) / RayDirScreen.y);
	}
	else {
		TraversalDistance = min(TraversalDistance, (-RayStartScreen.y) / RayDirScreen.y);
	}


	if (RayDirScreen.z > 0.0) {
		//TraversalDistance = max(TraversalDistance,(zFar - RayStartScreen.z) / RayDirScreen.z); 
	}
	//else {
	//	TraversalDistance = min(TraversalDistance,abs((zNear -RayStartScreen.z) / RayDirScreen.z)); 
	//}


	//binary search the required traversal distance (there must be an algebraic way to solve this, but I am not smart enough to do so) 

	TraversalDistance = 100.f;

	float TraversalStepSize = 100.f;

	float BaseDistance = distance(Origin, CameraPosition);

	for (int BinarySearchIteration = 0; BinarySearchIteration < 16; BinarySearchIteration++) {

		TraversalStepSize *= 0.5;

		vec3 Position = ViewSpacePosition + ViewSpaceDirection * TraversalDistance;

		vec4 ClipSpace = ProjectionMatrix * vec4(Position, 1.0);
		ClipSpace.xyz /= ClipSpace.w;



		if (abs(ClipSpace).x >= .99 || abs(ClipSpace.y) >= .99 || (LinearlizeDepth(ClipSpace.z * 0.5 + 0.5) - BaseDistance) >= 30.0)
			TraversalDistance -= TraversalStepSize;
		else
			TraversalDistance += TraversalStepSize;




	}

	TraversalStepSize = min(100.f, TraversalStepSize);

	//figure out a good step count based on all of this! 

	Steps = clamp(int((Steps / 8.0f) * TraversalDistance), Steps / 8, Steps);


	vec3 RayStep = (RayDirScreen.xyz * TraversalDistance) / float(Steps);

	vec3 RayStepView = (ViewSpaceDirection * TraversalDistance) / float(Steps);

	float Traversal = clamp(hash(ivec2(gl_FragCoord.xy), Frame, 4), .0, 1.0);
	float TraversalSize = (TraversalDistance) / float(Steps); 

	vec3 RayPosView = ViewSpacePosition + RayStepView * Traversal;



	vec3 RayPos = RayStartScreen.xyz + RayStep * Traversal;

	ivec2 PreviousPixel = ivec2(gl_FragCoord.xy);

	float PreviousZ = RayPos.z;

	Traversal *= TraversalSize; 

	for (int i = 1; i < Steps; i++) {

		//have we moved at least one pixel? 

		PreviousZ = RayPos.z;

		RayPosView += RayStepView;
		Traversal += TraversalSize; 

		vec4 Fetch = ProjectionMatrix * vec4(RayPosView, 1.0);
		Fetch.xyz /= Fetch.w;
		RayPos = Fetch.xyz * 0.5 + 0.5;
		RayPos.z = LinearlizeDepth(RayPos.z);





		if (RayPos.z < zNear || RayPos.z > zFar)
			break;

		if (abs(RayPos.x * 2. - 1.) >= (1.0 - 1.0 / Resolution.x) || abs(RayPos.y * 2. - 1.) >= (1.0 - 1.0 / Resolution.y))
			break;

		float DepthSample = (texture(Depth, vec2(ivec2(RayPos.xy * Resolution)) / Resolution).x);

		if (DepthSample + 0.0025 < RayPos.z) {
			if (abs(DepthSample - RayPos.z) < abs(RayPos.z - PreviousZ) * 2.0)
				return vec3(RayPos.xy, Traversal);
			return vec3(-10.0);
		}

	}

	return vec3(-1.0);


}

uniform float Roughness;

float GetDiffuseIndirect(vec3 Normal, vec3 WorldPos, bool Metal, float Rough) {


	//First, attempt to utilize existing screen-space data

	//return texture(DirectInput, TexCoord).xyz; 

	vec3 Direction = texture(DiffuseDirection, TexCoord).xyz;

	

	//if(Metal) 


	



	float Traversal = 0.0;

	//vec2 ScreenSpaceTraceResult = ScreenSpaceTrace(vec3(vec4(Direction.xyz, 0.0) * InverseView),(ViewMatrix * vec4(WorldPos, 1.0)).xyz, 0.1, 1.0, 64, 5, Traversal); 
	//if(UseNewMethod) 
	vec3 ScreenSpaceTraceResult = SSRT(Direction, WorldPos, 16).xyz;

	if (ScreenSpaceTraceResult.x > 0.0 && ScreenSpaceTraceResult.y > 0.0) {
		//return vec3(Traversal); 

		return ScreenSpaceTraceResult.z; 
	}
	
	float Traced = TraceCubeMap(Direction, WorldPos, 48);

	if(Traced > 0.0) {
		return -Traced; 
	}


	return 0.0; 

	//If this fails, try instead to use cubemap data 


	//If all fails, just rely on pre-baked indirect lighting (which is still really good quality) 


	//Final fallback -> baked indirect diffuse 


}



void main() {

	vec4 NormRoughness = texture(Normal, TexCoord);
	vec4 WorldPosition = texture(WorldPos, TexCoord);

	Resolution = textureSize(WorldPos, 0).xy / 2;

	bool Metal = WorldPosition.w > .7;

	Diffuse = GetDiffuseIndirect(NormRoughness.xyz, WorldPosition.xyz, Metal, NormRoughness.w);



}