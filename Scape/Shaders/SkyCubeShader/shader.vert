#version 330
layout (location = 0) in vec3 Vert; 
out vec3 Vertex;  

uniform mat4 ViewMatrix; 

void main(void) {
	Vertex = Vert; 
	gl_Position = ViewMatrix * vec4(Vert,1.0); 
}