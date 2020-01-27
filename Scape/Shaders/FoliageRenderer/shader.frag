#version 420

in vec2 TexCoord; 
in vec3 WorldPos; 


layout(location = 0) out vec4 OutAlbedo;
layout(location = 1) out vec4 OutNormal;
layout(location = 2) out vec4 OutWorldPosition; 
layout(location = 3) out vec3 OutLighting; 
layout(location = 4) out vec4 GrassDirectData; 

uniform sampler3D CurrentRayData; 
uniform sampler2D Wind; 
uniform sampler2D GrassTexture; 
uniform sampler2D GrassSurfaceTexture; 

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

	return vec3(Sample1.xy,Traversal1); 
	
}

vec3 hash33( vec3 p )
{
	p = vec3( dot(p,vec3(127.1,311.7, 74.7)),
			  dot(p,vec3(269.5,183.3,246.1)),
			  dot(p,vec3(113.5,271.9,124.6)));

	return fract(sin(p)*43758.5453123);
}

float seed; 

vec2 hash2() {
    return fract(sin(vec2(seed+=0.1,seed+=0.1))*vec2(43758.5453123,22578.1459123));
}

//gives [normal,traversal] (traversal is -1 if there was no hit!) 
vec4 Trace(vec3 Direction, vec3 WorldPosition, float MaxTraversal) { //todo : TBN matrix 
	
	vec2 DirectionXZ = normalize(Direction.xz); 
	
	float Angle = atan(DirectionXZ.x, DirectionXZ.y); 

	vec2 ATexCoord = WorldPosition.xz; 

	vec3 Data = vec3(1000000.); 

	vec2 NormalData = vec2(0.0); 

	for(int i = 0; i < 3; i++) {
		vec3 interpNode = GetTriangleInterpNode(ATexCoord, 20.0, i);

		//THATS WHEN ITS BACK TO THE LAB AGAIN 

		vec3 hash = hash33(vec3(interpNode.xy, 0));

		float ang = hash.x * 2.0 * pi; 

		mat2 rotation = mat2(cos(ang), sin(ang), -sin(ang), cos(ang));

		vec2 NewCoord = ATexCoord * rotation; 

		float NewAngle = fract((Angle + ang) / 6.28318531) * 6.28318531; 


		vec3 Sample = GetGrassSample(Direction, DirectionXZ, NewAngle, 24.0 * NewCoord, WorldPos,Time); 

		if(Data.z > Sample.z) {
			Data.z = Sample.z; 
			NormalData = (Sample.xy * 2. - 1.) * rotation; 
		}
	}

	Data.z /= 24.0; 

	float Traversal3D = Data.z / sqrt(1.0-clamp(Direction.y*Direction.y,0.0,1.0)); 

	if(Traversal3D >= MaxTraversal) {
		return vec4(0.0,0.0,0.0,-1.0); 
	}

	return vec4(NormalData, Data.z,Traversal3D); 

}


vec4 DoShadowTrace(float TraversalPlane, vec3 Direction, vec3 HitLocation) {
	
	float TraversalToTop = -TraversalPlane / Direction.y; 

	vec3 ShadowOrigin = HitLocation + Direction * TraversalToTop; 

	return Trace(-Direction, ShadowOrigin, TraversalToTop); 

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


void main() {

	seed = ((TexCoord.x * TexCoord.y)*1000.0); 


	GrassDirectData.w = 1.0;
	
	float TerrainRawDepthSample = texture(TerrainDepth, TexCoord).x; 
	float EntityDepthSample = texture(EntityDepth, TexCoord).x; 

	OutNormal = texture(TerrainNormal, TexCoord); 
	float L = length(OutNormal); 

	if(EntityDepthSample < TerrainRawDepthSample || L < 0.5 || L > 1.5) {
		
		OutAlbedo = texture(EntityAlbedo, TexCoord); 
		OutNormal = texture(EntityNormal, TexCoord); 
		OutWorldPosition = texture(EntityWorldPosition, TexCoord); 
		OutLighting = texture(EntityLighting, TexCoord).xyz; 
		//early exit! 
		return; 
	}

	vec4 RawTerrainWorldPosition = texture(TerrainWorldPosition, TexCoord); 


	OutAlbedo = texture(TerrainAlbedo, TexCoord); 
	
	OutWorldPosition = texture(TerrainWorldPosition, TexCoord); 
	OutLighting = texture(TerrainLighting, TexCoord).xyz; 


	vec3 Incident = normalize(RawTerrainWorldPosition.xyz - CameraPosition); 

	//would require a TBN matrix actually 
	vec2 IncidentXZ = normalize(Incident.xz); 
	
	float Angle = atan(IncidentXZ.x, IncidentXZ.y); 

	vec3 Data = vec3(100000.0); 

	float MaxTraversal = .03 / abs(Incident.y);

	vec4 RawTraceData = Trace(Incident, RawTerrainWorldPosition.xyz, MaxTraversal); 

	Data.z = RawTraceData.w; 
	
	bool Hit = Data.z >= 0.0; 

	if(Data.z < 0.0 || Data.z > MaxTraversal) {
		
		Data.z = MaxTraversal; 

	}
	//Data.z = MaxTraversal; 

	float MaxAbs = max(abs(Incident.x), abs(Incident.z)); 

	float Traversal3D = Data.z; 
	

	vec3 Origin = RawTerrainWorldPosition.xyz; 

	vec3 newWorldPos = Origin + Incident * Data.z; 


	float TraversalPlane = abs(Incident.y * Data.z); 

	
	if(Hit) {
		float CoordinateHeight = clamp(MaxTraversal-abs(Incident.y) * Data.z,0.0,MaxTraversal)/MaxTraversal; 
		
		//figure out what the y component of the normal should be 

		//we know that NormalX^2 + NormalY^2 + NormalZ^2 = 1
		//so if do some basic math -> NormalY^2 = 1 - NormalX^2 - NormalY^2 
		//NormalY = sqrt(1.0 - NormalX*NormalX - NormalY*NormalY) 

		vec2 NormalXZ = RawTraceData.xy; 

		OutNormal.xyz = vec3(NormalXZ.y, sqrt(1.0 - NormalXZ.x * NormalXZ.x - NormalXZ.y * NormalXZ.y), NormalXZ.x); 

		NormalXZ = normalize(NormalXZ); 

		float CoordinateAngle = atan(NormalXZ.x, NormalXZ.y) / 6.283;
		
		vec3 GrassAlbedo = pow(texture(GrassTexture, vec2(CoordinateAngle, CoordinateHeight)).xyz,vec3(2.2)) * 2.0 * pow(texture(GrassSurfaceTexture, vec2(newWorldPos.x*.5, newWorldPos.z*.5)).xyz,vec3(2.2)); 
		GrassAlbedo = min(GrassAlbedo, vec3(1.0)); 
		OutAlbedo.xyz = mix(GrassAlbedo,OutAlbedo.xyz, pow(1.0-CoordinateHeight,3.0)); 
		

	}
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

	//shadow trace!!! OOOO

	//project ourselves back to our terrain top via RT (would want to use a a shadowmap lookup for this in the future though...) 
	

	float TraversalY = (Incident.y) * Data.z; 


	vec4 ShadowTrace = DoShadowTrace(TraversalY, LightDirection, newWorldPos); 

	if(ShadowTrace.w >= 0.0) {
		GrassDirectData.w = 0.0; 
	}



}