#version 330
in vec2 TexCoord; 

layout(location = 0) out vec4 IndirectDiffuse; //alpha component contains AO 
layout(location = 1) out vec3 IndirectSpecular; //~ no need for any alpha component 

uniform sampler2D NormalHighRes; 
uniform sampler2D DepthHighRes; 
uniform sampler2D WorldPosition; 

uniform sampler2D Direct; 
uniform sampler2D BakedIndirect; 

uniform sampler2D Normal; 
uniform sampler2D Depth; 

uniform samplerCube CubeLighting; 
uniform samplerCube CubeDepth; 
uniform samplerCube Sky; 

uniform sampler2D RawDiffuse; 
uniform sampler2D RawSpecular; 

uniform sampler2D DiffuseDirection; 
uniform sampler2D SpecularDirection; 
uniform sampler2D Albedo; 

uniform float zNear; 
uniform float zFar; 
uniform vec3 CameraPosition; 

uniform mat4 IncidentMatrix; 
uniform mat4 ProjectionMatrix; 
uniform mat4 ViewMatrix; 
uniform mat4 InverseView; 

uniform int Divisor; 

uniform bool UseHalfRes; 

float LinearlizeDepth(float z) {
	return 2.0 * zNear * zFar / (zFar + zNear - (z * 2.0 - 1.0) * (zFar - zNear)); 
}

float GetWeightAt(ivec2 LowResPixel, ivec2 HighResPixel, vec4 NormalInput, float DepthInput, float Facing) {
	
	vec4 LowResNormalFetch = texelFetch(Normal, LowResPixel, 0); 
	float LowResDepthFetch = texelFetch(Depth, LowResPixel, 0).x; 

	float WeightNormalComponent = pow(max(dot(LowResNormalFetch.xyz, NormalInput.xyz), 0.0),32.0); 
	float WeightDepthComponent = pow(1.0 / (1.0 + abs(LowResDepthFetch-DepthInput)),2.0); 
	float PixelDistanceComponent = 1.0 / (1.0 + distance(vec2(LowResPixel*Divisor), vec2(HighResPixel))*0.5); 
	float RoughnessComponent = pow(1.0 - abs(NormalInput.w - LowResNormalFetch.w), 2.0);  

	return WeightNormalComponent * PixelDistanceComponent * RoughnessComponent; 

}


vec3 CalculateComponent(vec4 Input, vec3 Direction, vec3 Origin, vec3 SkySample, inout float TotalWeight) {
	
	//first check the screen space data 

	vec3 TotalLighting = vec3(0.0); 

	if(Input.x > 0.01 && Input.y > (Input.y+Input.w)*0.1) {
		
		vec3 ViewSpaceDirection = vec3(vec4(Direction.xyz, 0.0) * InverseView);
		vec3 ViewSpacePosition = (ViewMatrix * vec4(Origin, 1.0)).xyz;

		vec4 Fetch = ProjectionMatrix * vec4(ViewSpacePosition + ViewSpaceDirection * Input.x,1.0); 
		Fetch.xyz /= Fetch.w; 

		if(abs(Fetch.x) < 1.0 || abs(Fetch.y) < 1.0) {
			Fetch.xy = Fetch.xy * 0.5 + 0.5; 
			
			
			float L = length(texture(Normal, Fetch.xy).xyz); 

			if(L > 0.75 && L < 1.25) {

				TotalWeight += Input.y; 

				TotalLighting += texture(Albedo, Fetch.xy).xyz * (texture(Direct, Fetch.xy).xyz + texture(BakedIndirect, Fetch.xy).xyz) * Input.y; 
			}
		}
	}

	if(Input.z > 0.01 && Input.w > (Input.y + Input.w) * 0.1) {
		
		vec3 Direction = normalize((Origin + Direction * Input.z) - CameraPosition);  

		TotalLighting += texture(CubeLighting,Direction).xyz * Input.w; 

		vec4 Fetch = texture(CubeLighting,Direction); 
		Fetch.xyz = mix(SkySample,Fetch.xyz, Fetch.w); 


		TotalWeight += Input.w; 

	}


	return TotalLighting /= TotalWeight; 


}



void main() {
	
	ivec2 HighResPixel = ivec2(gl_FragCoord.xy); 
	ivec2 LowResPixel = HighResPixel / Divisor; 
	
	

	vec4 InterpolatedDiffuse = vec4(0.0); 
	float TotalWeightDiffuse = 0.0; 
	vec4 InterpolatedSpecular = vec4(0.0); 


	float TotalWeight = 0.0; 

	vec3 Incident = -normalize(vec3(IncidentMatrix * vec4(TexCoord * 2. - 1., 1., 1.))); 

	vec4 BaseNormal = texture(NormalHighRes, TexCoord); 
	float BaseDepth = LinearlizeDepth(texture(DepthHighRes, TexCoord).x); 
	vec3 BaseWorldPosition = texture(WorldPosition, TexCoord).xyz; 

	float Facing = pow(abs(dot(BaseNormal.xyz, Incident)),4.0); 

	vec3 ActualDiffuseDirection = texture(DiffuseDirection, TexCoord).xyz; 

	if(UseHalfRes) {
		ActualDiffuseDirection = texelFetch(DiffuseDirection, ivec2(gl_FragCoord.xy/2)*2,0).xyz; 
	}

	vec3 SpecularDirection = texture(SpecularDirection, TexCoord).xyz; 

	vec3 SkySampleDiffuse = texture(Sky, ActualDiffuseDirection).xyz; 

	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			
			ivec2 Pixel = LowResPixel + ivec2(x,y); 

			float Weight = GetWeightAt(Pixel, HighResPixel, BaseNormal, BaseDepth, Facing); 

			//IndirectDiffuse.xyz += texelFetch(RawDiffuse, Pixel, 0).xxx * Weight; 

			float IndirectDiffuseSample = texelFetch(RawDiffuse, Pixel, 0).x; 
			
			if(abs(IndirectDiffuseSample) < 0.1) {
				//do nothing, for now 
			}
			else {
				if(IndirectDiffuseSample > 0.0) { 
					InterpolatedDiffuse.x += IndirectDiffuseSample * Weight; 
					InterpolatedDiffuse.y += Weight; 
					TotalWeightDiffuse += Weight; 
				}
				else {
					InterpolatedDiffuse.z += -IndirectDiffuseSample * Weight; 
					InterpolatedDiffuse.w += Weight; 
					TotalWeightDiffuse += Weight; 
				}
			}

			

		} 
	}

	InterpolatedDiffuse.x /= InterpolatedDiffuse.y; 
	InterpolatedDiffuse.z /= InterpolatedDiffuse.w; 

	float SecondaryWeight = 0.0; 

	IndirectDiffuse.xyz = CalculateComponent(InterpolatedDiffuse, ActualDiffuseDirection, BaseWorldPosition,SkySampleDiffuse,SecondaryWeight); 

	if(TotalWeightDiffuse < 0.1 || SecondaryWeight < 0.1) {

		vec4 RawGrab = texture(CubeLighting, ActualDiffuseDirection.xyz); 
	
		IndirectDiffuse.xyz = mix(SkySampleDiffuse,RawGrab.xyz, RawGrab.w); 

		IndirectDiffuse.xyz = texture(BakedIndirect, TexCoord).xyz; 

	}
	
	IndirectDiffuse.xyz += texture(Direct, TexCoord).xyz; 

	//IndirectDiffuse.xyz /= max(TotalWeightDiffuse,0.01); 
	IndirectDiffuse.w = 0.0; 

} 