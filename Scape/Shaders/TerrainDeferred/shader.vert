#version 330
layout(location=0) in vec3 Vertices; 
layout(location=1) in vec2 TexCoords; 
layout(location=2) in vec2 LightMapTexCoords;
layout(location=3) in vec3 Normals;
layout(location=4) in vec3 Tangents;

uniform mat4 IdentityMatrix; 


out vec2 TexCoord;
out vec3 Position;
out vec3 Normal; 
out vec3 Tang; 
out vec2 LightMapTC; 

uniform int LightMapWidth; 


void main(void) { 

	LightMapTC = LightMapTexCoords.xy; 
	LightMapTC.x *= (128.0 / float(LightMapWidth)); 

	vec3 PositionPrevious = vec3(0.0); 
	vec3 PositionCurrent = vec3(0.0); 

	mat3 NormalMatrix; 


	
	
	PositionPrevious = Vertices; 
	PositionCurrent = Vertices; 
	
	
	Position = (vec4(PositionCurrent, 1.0)).xyz; 
	Normal = Normals; 
	TexCoord = TexCoords.xy;
	Tang = Tangents; 


	gl_Position = IdentityMatrix *  vec4(PositionCurrent,1.0); 
}
