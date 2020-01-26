#version 420

in vec2 TexCoord; 

//data is stored as GL_RGBA8. 2 first components is normal x and normal z (normal y is reconstructed), and then the last 2 is an encoded depth value (constant precision for now) 
layout(location = 0) out vec4 Data;

uniform int TextureSize; 
uniform int RayCount; 
uniform float MinStepSize; 
uniform float MaxStepLength; 
uniform float Time; 
uniform sampler1D Primitives; 
uniform sampler2D WindTexture; 
uniform int PrimitiveCount; 



struct Primitive {
	int Type; 
	vec2 Size; 
	float Rotation; 
	vec2 Position; 
	vec2 Shift; 
}; 

Primitive GetPrimitiveData(int Index) {

	Primitive OutPrimitive; 

	vec3 Fetches[3]; 
	for(int i = 0; i < 3; i++) 
		Fetches[i] = texelFetch(Primitives, Index*3+i, 0).xyz; 

	OutPrimitive.Type = int(Fetches[0].x+.1); 
	OutPrimitive.Size = Fetches[0].yz; 
	OutPrimitive.Rotation = Fetches[1].x; 
	OutPrimitive.Position = Fetches[1].yz; 
	OutPrimitive.Shift = Fetches[2].xy; 

	return OutPrimitive; 

}

float SignedDistanceUnitCircle(vec2 p, float r) {
	return length(p) - r; 
}

bool HandleCircle(inout float MinDistance, inout vec3 NormalHit, float TraversalDistance, Primitive Data, vec2 Position, mat2 WindRotation, float WindIntensity) {
	
	vec2 ActualP = (fract(Position + Data.Shift * WindRotation * WindIntensity * cos(TraversalDistance*4.0))-Data.Position) / Data.Size ; 

	float Dist = SignedDistanceUnitCircle(ActualP, clamp(TraversalDistance,0.01,1.0)); 

	if(Dist <= 0.0) {
		NormalHit = vec3(1.0,0.0,0.0);
		return true; 
	}

	MinDistance = min(MinDistance, Dist); 
	MinDistance = max(MinDistance, 0.001f); 
	return false; 
}


void main() {

	//figure out our coordinate on the texture! 

	ivec2 Pixel = ivec2(gl_FragCoord.xy); 
	int Ray = Pixel.x / TextureSize; 

	ivec2 PixelActual = Pixel % TextureSize; 

	vec2 TextureCoordActual = vec2(PixelActual) / vec2(TextureSize); 

	float RayAngle = (float(Ray) / float(RayCount)) * 6.28318531; 

	vec2 RayDirection = vec2(cos(RayAngle), sin(RayAngle)); 


	vec3 NormalHit = vec3(0.0,1.0,0.0); 

	vec2 CurrentLocation = TextureCoordActual; 

	Data = vec4(0.0,0.0,1.0,1.0); 

	



	for(float Traversal = 0.0; Traversal < MaxStepLength;) {
	
		vec3 RawWindSample = texture(WindTexture, CurrentLocation * 0.05 + vec2(1.0,1.0)*Time).xyz; 

		vec2 WindDirectionXZ = normalize(RawWindSample.xz * 2. - 1.); 

		float WindAngle = atan(WindDirectionXZ.x, WindDirectionXZ.y);

		mat2 rotation = mat2(cos(WindAngle), sin(WindAngle), -sin(WindAngle), cos(WindAngle));

		float WindIntensity = RawWindSample.y; 

		float MinDistance = MinStepSize; 

		for(int PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++) {

			Primitive CurrentPrimitive = GetPrimitiveData(PrimitiveIndex); 

			switch(CurrentPrimitive.Type) {
				case 0: 
					if(HandleCircle(MinDistance, NormalHit, Traversal, CurrentPrimitive, CurrentLocation,rotation, WindIntensity)) {
						
						int TraversalDistanceInt = int((Traversal / MaxStepLength)*65536); 

						Data.x = 0.0; 
						Data.y = 0.0; 
						Data.z = (TraversalDistanceInt & 255) / 255.f; 
						Data.w = (TraversalDistanceInt / 256) / 255.f; 

						return; 

					}
				break; 
				
			}
				
		}


		Traversal += MinDistance; 
		CurrentLocation += RayDirection * MinDistance; 

	}


}