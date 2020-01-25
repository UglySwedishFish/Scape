#version 420

in vec2 TexCoord; 
in vec3 WorldPos; 
layout(location = 0) out vec4 Lighting;

uniform sampler3D RayData; 
uniform vec3 CameraPosition; 
uniform float MaxLength; 
uniform float Time; 
uniform bool RandTexCoord; 

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

vec3 GetGrassSample(vec3 Incident,vec2 IncidentXZ,float Angle, vec2 TextureCoordinate, vec3 WorldPos) {
	vec4 Sample = texture(RayData, vec3(TextureCoordinate.x, Angle/6.28318531, TextureCoordinate.y)); 

	//construct 2D traversal 

	float Traversal = ((int(Sample.z * 255) + int(Sample.w * 255) * 255) / 65536.f) * MaxLength; 

	return vec3(0.0,0.0,Traversal); 
	
}

vec3 hash33( vec3 p )
{
	p = vec3( dot(p,vec3(127.1,311.7, 74.7)),
			  dot(p,vec3(269.5,183.3,246.1)),
			  dot(p,vec3(113.5,271.9,124.6)));

	return fract(sin(p)*43758.5453123);
}



void main() {
	vec3 Incident = normalize(WorldPos - CameraPosition); 

	//would require a TBN matrix actually 
	vec2 IncidentXZ = normalize(Incident.xz); 
	
	float Angle = atan(IncidentXZ.x, IncidentXZ.y); 

	Lighting.xyz = vec3(0.0); 

	vec3 Data = vec3(100000.0); 

	for(int i = 0; i < 3; i++) {
		vec3 interpNode = GetTriangleInterpNode(TexCoord, 10.0, i);

		//THATS WHEN ITS BACK TO THE LAB AGAIN 

		vec3 hash = hash33(vec3(interpNode.xy, 0));

		float ang = hash.x * 2.0 * pi; 

		mat2 rotation = mat2(cos(ang), sin(ang), -sin(ang), cos(ang));

		vec2 NewCoord = TexCoord * rotation; 
		float NewAngle = fract((Angle + ang) / 6.28318531) * 6.28318531; 


		vec3 Sample = GetGrassSample(Incident, IncidentXZ, NewAngle, 15.0 * NewCoord, WorldPos); 

		Data.z = min(Data.z,Sample.z); 
	}

	float Traversal3D = Data.z / (1.0-abs(Incident.y)); 
	
	float MaxTraversal = 1.0f / abs(Incident.y);
	
	Lighting.xyz = (vec3(WorldPos.x, WorldPos.y + 1.2, WorldPos.z) + vec3(Incident.x,Incident.y,Incident.z) * min(Traversal3D,MaxTraversal)) / 1.0f; 


}