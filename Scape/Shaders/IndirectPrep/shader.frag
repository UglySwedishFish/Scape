#version 330
in vec2 TexCoord; 

layout(location = 0) out float ViewSpaceZ; 

uniform sampler2D WorldPosition; 
uniform sampler2D Normal; 
uniform mat4 ViewMatrix; 

void main() {
	ViewSpaceZ = (ViewMatrix * vec4(texture(WorldPosition, TexCoord).xyz,1.0)).z; 

	float L = length(texture(Normal, TexCoord).xyz); 

	if(L < 0.5 || L > 1.5) 
		ViewSpaceZ = 0.0; 

}