#pragma once
#include "DependenciesMath.h"
#include <string>
#include <algorithm>
constexpr double PI = 3.14159265;

namespace Scape {
	namespace Core {
		template<typename T>
		inline T mix(T A, T B, float Mix) {
			return A * static_cast<T>(Mix) + B * static_cast<T>(1.0 - Mix);
		}

		struct Instance {
			
			Matrix4f ModelMatrix, ModelMatrixPrevious;
			Vector3f Position, Rotation, Scale;
		};

		struct KernelInstance {
			Matrix4f ModelMatrix; 
			int Model; 
		};

		struct BoundingBox {
			Vector3f Min = Vector3f(0.), Max = Vector3f(0.);
			inline Vector3f Center() { return (Min + Max) / 2.f; }
			inline Vector3f Size() {
				return Max - Min;
			}
			inline int LongestAxis() {
				Vector3f Length = Size();
				return Length.x > Length.y && Length.x > Length.z ? 0 : Length.y > Length.z ? 1 : 2;
			}
			inline float Area() {
				return (Size().x * Size().y + Size().x * Size().z + Size().y * Size().z) * 2.;
			}
			inline void ResizeToFit(BoundingBox Box) {
				Max = glm::max(Max, Box.Max);
				Min = glm::min(Min, Box.Min);
			}
			inline void ResizeToFit(Vector3f Point) {
				Max = glm::max(Max, Point);
				Min = glm::min(Min, Point);
			}

			BoundingBox(Vector3f Min = Vector3f(100000.), Vector3f Max = Vector3f(-100000.)) : Min(Min), Max(Max) {}

		};

		template<typename T>
		T min3(T a, T b, T c) {
			return std::min(a, std::min(b, c));
		}
		template<typename T>
		T max3(T a, T b, T c) {
			return std::max(a, std::max(b, c));
		}
		template<typename T>
		T min4(T a, T b, T c, T d) {
			return std::min(std::min(a, b), std::min(c, d));
		}
		template<typename T>
		T max4(T a, T b, T c, T d) {
			return std::max(std::max(a, b), std::max(c, d));
		}

		template<typename T, typename V>
		void Move_2DXY(V& Pos, T Speed, T Rotation, T FrameTime)
		{
			Pos.x -= cos(Rotation * T(PI / 180.)) * Speed * FrameTime;
			Pos.y -= sin(Rotation * T(PI / 180.)) * Speed * FrameTime;

		}
		template<typename T, typename V>
		void Move_2DXZ(V& Pos, T Speed, T Rotation, T FrameTime)
		{
			Pos.x -= cos(Rotation * T(PI / 180.)) * Speed * FrameTime;
			Pos.z -= sin(Rotation * T(PI / 180.)) * Speed * FrameTime;
		}
		template<typename T, typename V>
		void Move_3D(V& Pos, T Speed, T RotationX, T RotationY, T FrameTime, bool Forward = false)
		{

			float HMultiplier = Forward ? -cos(RotationX * (PI / 180.)) : cos(RotationX * (PI / 180.));

			Pos.x -= (cos(RotationY * (PI / 180.)) * HMultiplier) * Speed * FrameTime;
			Pos.y += sin(RotationX * (PI / 180.)) * Speed * FrameTime;
			Pos.z -= (sin(RotationY * (PI / 180.)) * HMultiplier) * Speed * FrameTime;
		}

		Matrix4f ViewMatrix(Vector3f Position, Vector3f Rotation);
		Matrix4f ModelMatrix(Vector3f Position, Vector3f Rotation, Vector3f Scale = Vector3f(1.0));
		Matrix4f ShadowOrthoMatrix(float edge, float znear, float zfar);
		Matrix4f UpscalingMatrix(Vector2f TexelSize, int Frame); 
		Matrix4f TAAJitterMatrix(int Sample, Vector2i Resolution); 

		Vector3f SphericalCoordinate(float Pitch, float Yaw, bool Degrees = false); 
		void PrepareHaltonSequence(); 
		

	}

}