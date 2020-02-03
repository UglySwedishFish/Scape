#version 330
in vec2 TexCoord; 

layout(location = 0) out vec4 IndirectDiffuse; //alpha component contains AO 
layout(location = 1) out vec3 IndirectSpecular; //~ no need for any alpha component 

uniform sampler2D NormalHighRes; 
uniform sampler2D DepthHighRes; 

uniform sampler2D Normal; 
uniform sampler2D Depth; 

uniform float zNear; 
uniform float zFar; 

float LinearlizeDepth(float z) {
	return 2.0 * zNear * zFar / (zFar + zNear - (z * 2.0 - 1.0) * (zFar - zNear)); 
}

//for specular, use frosbite trick to upscale AND increase sample count 
//for diffuse, just do more simplistic upscaling by picking the pixel with the most representative input data 


void main() {

	

	
	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			




		} 
	}






	


} 