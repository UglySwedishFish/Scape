#include "Core.h"

namespace Scape {
	namespace Core {

		//Credit to playdeadgames for the TAA offset and their respective calculations! 
		//This would not have been possible without them 

		Vector2f HaltonSequence[32]; 

		float GetHaltonSequence(int Prime, int Index = 1) {

			float r = 0.0f;
			float f = 1.0f;
			int i = Index;
			while (i > 0)
			{
				f /= Prime;
				r += f * (i % Prime);
				i = (int)std::floor(i / float(Prime));
			}
			return r;

		}

		void PrepareHaltonSequence() {
			for (int i = 0; i < 32; i++) {
				HaltonSequence[i].x = GetHaltonSequence(2, i + 1); 
				HaltonSequence[i].y = GetHaltonSequence(3, i + 1); 
			}
		}



		Matrix4f ViewMatrix(Vector3f Position, Vector3f Rotation)
		{
			Matrix4f Temp = Matrix4f(1.0f);
			Temp = glm::rotate(Temp, glm::radians(Rotation.x), { 1, 0, 0 });
			Temp = glm::rotate(Temp, glm::radians(Rotation.y), { 0, 1, 0 });
			Temp = glm::rotate(Temp, glm::radians(Rotation.z), { 0, 0, 1 });

			Temp = glm::translate(Temp, Vector3f(-Position.x, -Position.y, -Position.z));

			return Temp;
		}

		Matrix4f ModelMatrix(Vector3f Position, Vector3f Rotation, Vector3f Scale) {
			Matrix4f Temp = glm::translate(Matrix4f(1.0f), Vector3f(-Position.x, -Position.y, -Position.z));
			Temp = glm::rotate(Temp, glm::radians(Rotation.x), { 1, 0, 0 });
			Temp = glm::rotate(Temp, glm::radians(Rotation.y), { 0, 1, 0 });
			Temp = glm::rotate(Temp, glm::radians(Rotation.z), { 0, 0, 1 });
			Temp = glm::scale(Temp, Scale);
			return Temp;
		}

		Matrix4f ShadowOrthoMatrix(float edge, float znear, float zfar)
		{
			return glm::ortho(-edge, edge, -edge, edge, znear, zfar);
		}

		Vector2f Upscaler[4] = {
			Vector2f(0.0,0.0), //step 1: the base pixel
			Vector2f(0.5,0.0), //next pixel in the x direction
			Vector2f(0.5,0.5), //we move up one in the y direction
			Vector2f(0.0,0.5) //and we step back one in the x direction. now weve sampled all the sub pixels!
		}; 

		Matrix4f UpscalingMatrix(Vector2f TexelSize, int Frame)
		{

			Vector2f Jitter = Upscaler[Frame % 4]; 

			return glm::translate(Matrix4f(), glm::vec3(Jitter.x * TexelSize.x, Jitter.y * TexelSize.y, 0.0f));
		}

		Matrix4f TAAJitterMatrix(int Sample, Vector2i Resolution)
		{

			Vector2f Jitter = HaltonSequence[Sample % 32];
			Vector2f TexelSize = 1.0f / Vector2f(Resolution);
			return glm::translate(Matrix4f(), glm::vec3(2.0 * Jitter.x * TexelSize.x, 2.0 * Jitter.y * TexelSize.y, 0.0f));
		}

		Vector3f SphericalCoordinate(float Pitch, float Yaw, bool Degrees)
		{
	
			if (Degrees) {
				Pitch = (Pitch / 180.) * PI; 
				Yaw = (Yaw / 180.) * PI; 
			}
			

			Vector3f Coordinate; 

			float CosPitch = cos(Pitch);

			Coordinate.x = -cos(Yaw) * CosPitch;
			Coordinate.y = sin(Pitch); 
			Coordinate.z = -sin(Yaw) * CosPitch;

			return Coordinate; 
		}

	}
}