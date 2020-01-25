#version 330
layout (location = 0) in vec3 Vert; 
layout (location = 1) in vec2 Texc;
out vec2 TexCoord;  

uniform mat4 IdentityMatrix; 

out vec3 WorldPos; 

void main(void) {
	TexCoord = Texc * 1000.; 
	WorldPos = vec3(Vert.x, 0.0, Vert.y) * 1000.; 
	gl_Position = IdentityMatrix * vec4(WorldPos,1.0); 
}