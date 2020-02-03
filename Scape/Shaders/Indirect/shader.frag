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

layout(location = 0) out vec3 Diffuse; 
layout(location = 1) out vec3 Specular; 

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

	return clamp(int(texelFetch(Texture, ivec2(Pixel), 0).x * 255. + .1),0,255); 
	
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


	int rankedSampleIndex = sampleIndex ^ FetchFromTexture(Ranking,sampleDimension + (pixel_i + pixel_j*128)*8);

	// fetch value in sequence
	int value = FetchFromTexture(Sobol,sampleDimension + rankedSampleIndex*256);

	// If the dimension is optimized, xor sequence value based on optimized scrambling
	value = value ^ FetchFromTexture(Scrambling,(sampleDimension%8) + (pixel_i + pixel_j*128)*8);

	// convert to float and return
	float v = (0.5f+value)/256.0f;
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
    } else if (abs(normal.y) < 0.57735026919f /* 1 / sqrt(3) */) {
        majorAxis = vec3(0, 1, 0);
    } else {
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


	for(int Try = 0; Try < 3; Try++) {
		
		vec2 Xi = hash2(ivec2(gl_FragCoord.xy), Frame, 8+Try) * vec2(1.f, 0.2f); 

		vec3 rho = ImportanceGGX(Xi, clamp(sqrt(Roughness), 0.001f, 1.0f)); 

		vec3 TryDirection = normalize(0.001f + rho.x * Tangent + rho.y * Bitangent + rho.z * RawDirection); 

		if(dot(TryDirection, Normal) > 0.0005f) {
			return TryDirection; 
		}

	}
	return RawDirection; 
}

ivec2 Resolution; 


float rsi(vec3 r0, vec3 rd, vec3 s0, float sr) {

    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b*b - 4.0*a*c < 0.0) {
        return -1.0;
    }
    return (-b - sqrt((b*b) - 4.0*a*c))/(2.0*a);
}

vec3 TraceCubeMap(vec3 Direction, vec3 Origin, int Steps) {
	

	//figure out the max traversal distance 


	
	/*float TraversalDistance = rsi(Origin, Direction, CameraPosition, 100.f); //gives us max traversal distance :)

	if(TraversalDistance < 0.0) 
		return vec3(-5.0f); */

		float TraversalDistance = 100.f; 


	vec3 Step = (Direction * TraversalDistance) / float(Steps); 

	vec3 Position = Origin + Step * clamp(hash(ivec2(gl_FragCoord.xy), Frame, 4),.0,1.0); 

	float PreviousZ = length(Position - CameraPosition); 

	for(int i = 1; i < Steps; i++) {
		
		Position += Step; 

		vec3 Incident = Position - CameraPosition; 
		float L = length(Incident); 
		float Depth = texture(CubeLinearZ, Incident/L).x; 
		
		if(Depth < L) {
			
			if(abs(Depth - L) < abs(PreviousZ - L) * 4.0) {
				return Incident / L; 
			}

			return vec3(-10.0f); 

		}

		PreviousZ = L; 

	}

	//are we ok still? 

	return vec3(-5.0f); 





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

	if(RayDirScreen.x > 0.0) {
		TraversalDistance = (1.0 - RayStartScreen.x) / RayDirScreen.x; 

	}
	else {
		TraversalDistance = (-RayStartScreen.x) / RayDirScreen.x; 
	}


	if(RayDirScreen.y > 0.0) {
		TraversalDistance = min(TraversalDistance,(1.0 - RayStartScreen.y) / RayDirScreen.y); 
	}
	else {
		TraversalDistance = min(TraversalDistance,(-RayStartScreen.y) / RayDirScreen.y); 
	}


	if(RayDirScreen.z > 0.0) {
		//TraversalDistance = max(TraversalDistance,(zFar - RayStartScreen.z) / RayDirScreen.z); 
	}
	//else {
	//	TraversalDistance = min(TraversalDistance,abs((zNear -RayStartScreen.z) / RayDirScreen.z)); 
	//}


	//binary search the required traversal distance (there must be an algebraic way to solve this, but I am not smart enough to do so) 

	TraversalDistance = 100.f; 

	float TraversalStepSize = 100.f; 

	float BaseDistance = distance(Origin, CameraPosition); 

	for(int BinarySearchIteration = 0; BinarySearchIteration < 16; BinarySearchIteration++) {
		
		TraversalStepSize *= 0.5; 

		vec3 Position = ViewSpacePosition + ViewSpaceDirection * TraversalDistance; 

		vec4 ClipSpace = ProjectionMatrix * vec4(Position, 1.0) ; 
		ClipSpace.xyz /= ClipSpace.w; 



		if(abs(ClipSpace).x >= .99 || abs(ClipSpace.y) >= .99 || (LinearlizeDepth(ClipSpace.z*0.5+0.5)-BaseDistance) >= 30.0)
			TraversalDistance -= TraversalStepSize; 
		else 
			TraversalDistance += TraversalStepSize; 
			



	}

	TraversalStepSize = min(100.f, TraversalStepSize);  

	//figure out a good step count based on all of this! 

	Steps = clamp(int((Steps / 8.0f) * TraversalDistance), Steps / 8, Steps); 


	vec3 RayStep = (RayDirScreen.xyz*TraversalDistance) / float(Steps); 

	vec3 RayStepView = (ViewSpaceDirection * TraversalDistance) / float(Steps); 
	vec3 RayPosView = ViewSpacePosition + RayStepView * clamp(hash(ivec2(gl_FragCoord.xy), Frame, 4),.0,1.0); 

	vec3 RayPos = RayStartScreen.xyz + RayStep * clamp(hash(ivec2(gl_FragCoord.xy), Frame, 4),.0,1.0); 
	
	ivec2 PreviousPixel = ivec2(gl_FragCoord.xy); 

	float PreviousZ = RayPos.z; 

	for(int i = 1; i < Steps; i++) {
		
		//have we moved at least one pixel? 

		PreviousZ = RayPos.z; 
		
		RayPosView += RayStepView; 

		vec4 Fetch = ProjectionMatrix * vec4(RayPosView, 1.0); 
		Fetch.xyz /= Fetch.w; 
		RayPos = Fetch.xyz * 0.5 + 0.5; 
		RayPos.z = LinearlizeDepth(RayPos.z); 


		


		if(RayPos.z < zNear || RayPos.z > zFar) 
			break; 

		if(abs(RayPos.x * 2. - 1.) >= (1.0-1.0/Resolution.x) || abs(RayPos.y * 2. - 1.) >= (1.0-1.0/Resolution.y)) 
			break; 

		float DepthSample = LinearlizeDepth(texture(Depth, vec2(ivec2(RayPos.xy*Resolution))/Resolution).x); 

		if(DepthSample+0.0025 < RayPos.z) {
			if(abs(DepthSample - RayPos.z) < abs(RayPos.z-PreviousZ) * 2.0)
				return vec3(RayPos.xy,0.0); 
			return vec3(-10.0); 
		}
	
	}

	return vec3(-1.0); 


}

uniform float Roughness; 

vec3 GetDiffuseIndirect(vec3 Normal, vec3 WorldPos, bool Metal) {

	vec3 LightMapSample = texture(BakedIndirectDiffuse, TexCoord).xyz; 

	//First, attempt to utilize existing screen-space data

	//return texture(DirectInput, TexCoord).xyz; 

	vec3 Direction = LambertBRDF(Normal, hash2(ivec2(gl_FragCoord.xy), Frame,0)).xyz; 

	vec3 Incident = normalize(WorldPos - CameraPosition); 

	if(Metal) 
		Direction = GetSpecularRayDirection(reflect(Incident, Normal), Normal, Incident, Roughness); 

	

	float Traversal = 0.0; 

	//vec2 ScreenSpaceTraceResult = ScreenSpaceTrace(vec3(vec4(Direction.xyz, 0.0) * InverseView),(ViewMatrix * vec4(WorldPos, 1.0)).xyz, 0.1, 1.0, 64, 5, Traversal); 
	//if(UseNewMethod) 
	vec2 ScreenSpaceTraceResult = SSRT(Direction, WorldPos, 32).xy; 

	if(ScreenSpaceTraceResult.x > 0.0 && ScreenSpaceTraceResult.y > 0.0) {
		//return vec3(Traversal); 

		return texture(Albedo,ScreenSpaceTraceResult.xy).xyz * (texture(DirectInput, ScreenSpaceTraceResult.xy).xyz + texture(BakedIndirectDiffuse, ScreenSpaceTraceResult).xyz); 
	}

	if(ScreenSpaceTraceResult.x < -9.0) 
		return LightMapSample; 

	vec3 Traced = TraceCubeMap(Direction, WorldPos, 32); 

	if(Traced.x < -8.0) 
		return LightMapSample; 

	if(Traced.x < -2.0) 
		Traced = Direction; 

	vec4 CubeRawSample = texture(CubeLighting, Traced); 
	return mix(texture(Sky, Direction).xyz, CubeRawSample.xyz, CubeRawSample.w); ; 

	//If this fails, try instead to use cubemap data 

	
	//If all fails, just rely on pre-baked indirect lighting (which is still really good quality) 


	//Final fallback -> baked indirect diffuse 

	
}



void main() {

	vec4 NormRoughness = texture(Normal, TexCoord); 
	vec4 WorldPosition = texture(WorldPos, TexCoord); 
	
	Resolution = textureSize(WorldPos, 0).xy; 

	bool Metal = WorldPosition.w > .7; 

	vec3 Indirect = GetDiffuseIndirect(NormRoughness.xyz, WorldPosition.xyz,Metal); 



	Diffuse = (Indirect); 

}