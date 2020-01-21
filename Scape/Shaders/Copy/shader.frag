#version 420

in vec2 TexCoord; 
layout(location = 0) out vec4 Lighting;

uniform sampler2D InputTAA; 

void main() {

	Lighting = texture(InputTAA, TexCoord); 

}