#version 420

in vec2 TexCoord; 
in vec3 WorldPos; 


layout(location = 0) out vec4 OutAlbedo;
layout(location = 1) out vec4 OutNormal;
layout(location = 2) out vec4 OutWorldPosition; 
layout(location = 3) out vec3 OutLighting; 


uniform sampler3D CurrentRayData; 
uniform sampler2D Wind; 

uniform sampler2D EntityDepth; 
uniform sampler2D TerrainDepth; 


uniform sampler2D EntityAlbedo; 
uniform sampler2D EntityNormal; 
uniform sampler2D EntityWorldPosition; 
uniform sampler2D EntityLighting; 

uniform sampler2D TerrainAlbedo; 
uniform sampler2D TerrainNormal; 
uniform sampler2D TerrainWorldPosition; 
uniform sampler2D TerrainLighting; 

uniform vec3 CameraPosition; 
uniform vec3 LightDirection; 
uniform float MaxLength; 
uniform float Time; 
uniform bool RandTexCoord; 
uniform mat4 IdentityMatrix; 


const float pi = 3.141592;
const vec2 hexRatio = vec2(1.0, sqrt(3.0));

//credits for hex tiling goes to Shane (https://www.shadertoy.com/view/Xljczw)
//center, index
vec4 GetHexGridInfo(vec2 uv)
{
  vec4 hexIndex = round(vec4(uv, uv - vec2(0.5, 1.0)) / hexRatio.xyxy);
  vec4 hexCenter = vec4(hexIndex.xy * hexRatio, (hexIndex.zw + 0.5) * hexRatio);
  vec4 offset = uv.xyxy - hexCenter;
  return dot(offset.xy, offset.xy) < dot(offset.zw, offset.zw) ? 
    vec4(hexCenter.xy, hexIndex.xy) : 
    vec4(hexCenter.zw, hexIndex.zw);
}

float GetHexSDF(in vec2 p)
{
  p = abs(p);
  return 0.5 - max(dot(p, hexRatio * 0.5), p.x);
}

//xy: node pos, z: weight
vec3 GetTriangleInterpNode(in vec2 pos, in float freq, in int nodeIndex)
{
  vec2 nodeOffsets[] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0,-1.0));

  vec2 uv = pos * freq + nodeOffsets[nodeIndex] / hexRatio.xy * 0.5;
  vec4 hexInfo = GetHexGridInfo(uv);
  float dist = GetHexSDF(uv - hexInfo.xy) * 2.0;
  return vec3(hexInfo.xy / freq, dist);
}



vec3 GetGrassSample(vec3 Incident,vec2 IncidentXZ,float Angle, vec2 TextureCoordinate, vec3 WorldPos, float Time) {


	vec4 Sample1 = texture(CurrentRayData, vec3(TextureCoordinate.x, Angle/6.28318531, TextureCoordinate.y)); 
	
	float Traversal1 = ((int(Sample1.z * 255) + int(Sample1.w * 255) * 255) / 65536.f) * MaxLength; 

	return vec3(0.0,0.0,Traversal1); 
	
}

vec3 hash33( vec3 p )
{
	p = vec3( dot(p,vec3(127.1,311.7, 74.7)),
			  dot(p,vec3(269.5,183.3,246.1)),
			  dot(p,vec3(113.5,271.9,124.6)));

	return fract(sin(p)*43758.5453123);
}


//gives [normal,traversal] (traversal is -1 if there was no hit!) 
vec4 Trace(vec3 Direction, vec3 WorldPosition, float MaxTraversal) { //todo : TBN matrix 
	
	vec2 DirectionXZ = normalize(Direction.xz); 
	
	float Angle = atan(DirectionXZ.x, DirectionXZ.y); 

	vec2 ATexCoord = WorldPosition.xz; 

	vec3 Data = vec3(1000000.); 

	for(int i = 0; i < 3; i++) {
		vec3 interpNode = GetTriangleInterpNode(ATexCoord, 15.0, i);

		//THATS WHEN ITS BACK TO THE LAB AGAIN 

		vec3 hash = hash33(vec3(interpNode.xy, 0));

		float ang = hash.x * 2.0 * pi; 

		mat2 rotation = mat2(cos(ang), sin(ang), -sin(ang), cos(ang));

		vec2 NewCoord = ATexCoord * rotation; 

		float NewAngle = fract((Angle + ang) / 6.28318531) * 6.28318531; 


		vec3 Sample = GetGrassSample(Direction, DirectionXZ, NewAngle, 16.0 * NewCoord, WorldPos,Time); 

		Data.z = min(Data.z,Sample.z); 
	}

	Data.z /= 16.0; 

	float Traversal3D = Data.z / sqrt(1.0-clamp(abs(Direction.y)*abs(Direction.y),0.0,1.0)); 

	if(Traversal3D >= MaxTraversal) {
		return vec4(0.0,0.0,0.0,-1.0); 
	}

	return vec4(0.,0.0, 0.0,Traversal3D); 

}




void main() {

	
	float TerrainRawDepthSample = texture(TerrainDepth, TexCoord).x; 
	float EntityDepthSample = texture(EntityDepth, TexCoord).x; 

	if(EntityDepthSample < TerrainRawDepthSample) {
		
		OutAlbedo = texture(EntityAlbedo, TexCoord); 
		OutNormal = texture(EntityNormal, TexCoord); 
		OutWorldPosition = texture(EntityWorldPosition, TexCoord); 
		OutLighting = texture(EntityLighting, TexCoord).xyz; 
		//early exit! 
		return; 
	}

	vec4 RawTerrainWorldPosition = texture(TerrainWorldPosition, TexCoord); 


	OutAlbedo = texture(TerrainAlbedo, TexCoord); 
	OutNormal = texture(TerrainNormal, TexCoord); 
	OutWorldPosition = texture(TerrainWorldPosition, TexCoord); 
	OutLighting = texture(TerrainLighting, TexCoord).xyz; 


	vec3 Incident = normalize(RawTerrainWorldPosition.xyz - CameraPosition); 

	//would require a TBN matrix actually 
	vec2 IncidentXZ = normalize(Incident.xz); 
	
	float Angle = atan(IncidentXZ.x, IncidentXZ.y); 

	vec3 Data = vec3(100000.0); 

	float MaxTraversal = .15 / abs(Incident.y);


	Data.z = Trace(Incident, RawTerrainWorldPosition.xyz, MaxTraversal).w; 
	
	if(Data.z < 0.0 || Data.z > MaxTraversal) {
		
		return; 

	}

	float MaxAbs = max(abs(Incident.x), abs(Incident.z)); 

	float Traversal3D = Data.z; 
	

	vec3 Origin = RawTerrainWorldPosition.xyz; 

	vec3 newWorldPos = Origin + Incident * Data.z; 


	float TraversalPlane = abs(newWorldPos.y); 

	float NewMaxTraversal = (TraversalPlane) / abs(LightDirection.y); 


	//vec4 ShadowTrace = Trace(LightDirection, newWorldPos, NewMaxTraversal); 

	OutAlbedo.xyz = pow(clamp(MaxTraversal-abs(Incident.y) * Data.z,0.0,MaxTraversal)/MaxTraversal,2.0) * vec3(0.1,1.0,0.0); 
	OutWorldPosition.xyz = newWorldPos; 

	vec4 NewClipSpace = IdentityMatrix * vec4(OutWorldPosition.xyz, 1.0); 
	NewClipSpace.xyz /= NewClipSpace.w; 
	NewClipSpace.z = NewClipSpace.z * 0.5 + 0.5; 

	if(EntityDepthSample < NewClipSpace.z) {
		
		OutAlbedo = texture(EntityAlbedo, TexCoord); 
		OutNormal = texture(EntityNormal, TexCoord); 
		OutWorldPosition = texture(EntityWorldPosition, TexCoord); 
		OutLighting = texture(EntityLighting, TexCoord).xyz; 
		//early exit! 
		return; 
	}



}