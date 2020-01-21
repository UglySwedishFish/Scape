#version 330
layout(location=0) in vec3 Vertices; 
layout(location=1) in vec3 TexCoords; 
layout(location=2) in vec3 LightMapTexCoords;
layout(location=3) in vec3 Normals;
layout(location=4) in vec3 Tangents;

uniform mat4 IdentityMatrix; 

uniform sampler2D BoneData; 
uniform sampler2D InstanceData; 

out vec2 TexCoord;
out vec3 Position;
out vec3 Normal; 
out vec3 Tang; 
out vec2 LightMapTC; 



void main(void) {

	
	mat4 ModelMatrix = mat4(0.); 
	for(int i = 0; i < 4; i++)
		ModelMatrix[i] = texelFetch(InstanceData, ivec2(i+gl_InstanceID*5,0), 0);

	vec4 LightMapData = texelFetch(InstanceData, ivec2(gl_InstanceID*5+4, 0), 0); 

	LightMapTC = LightMapTexCoords.xy; 

	LightMapTC *= LightMapData.zw; 
	LightMapTC += LightMapData.xy; 

	ModelMatrix = transpose(ModelMatrix); 
	vec3 PositionPrevious = vec3(0.0); 
	vec3 PositionCurrent = vec3(0.0); 

	mat3 NormalMatrix; 


	
	
	PositionPrevious = Vertices; 
	PositionCurrent = Vertices; 
	NormalMatrix = transpose(inverse(mat3(ModelMatrix))); 
	
	
	Position = (ModelMatrix * vec4(PositionCurrent, 1.0)).xyz; 
	Normal = NormalMatrix * Normals; 
	Normal = Normals; 
	TexCoord = TexCoords.xy;
	Tang = Tangents; 


	gl_Position = IdentityMatrix * ModelMatrix *  vec4(PositionCurrent,1.0); 
}
