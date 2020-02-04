#version 330
in vec2 TexCoord; 

layout(location = 0) out float OutDepth; 
layout(location = 1) out vec4 OutNormal; 





uniform sampler2D Depth; 
uniform sampler2D Normal;

uniform float zNear; 
uniform float zFar; 


float LinearlizeDepth(float z) {
	return 2.0 * zNear * zFar / (zFar + zNear - (z * 2.0 - 1.0) * (zFar - zNear)); 
}


void main() {
	
	OutDepth = LinearlizeDepth(texture(Depth, TexCoord).x); 
	OutNormal = texture(Normal, TexCoord); 

}