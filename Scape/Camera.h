#pragma once

#include "Core.h"
#include "Window.h"


namespace Scape {

		struct Camera {
			Matrix4f Project, ProjectHighFOV, View, PrevView, PrevProject, Jitter, RawProject;
			Vector3f Position, Rotation, PreviousPosition;
			float Height, MaxJumpSpeed, GravityAcceleration, MaxGravitySpeed;
			bool ShadowCamera;
			float znear, zfar; 
			inline Camera(float fov, float znear, float zfar, Vector3f pos, Vector3f rot, Window& screen) :
				Project(glm::perspective(glm::radians(fov), float(screen.GetResolution().x) / float(screen.GetResolution().y), znear, zfar)),
				ProjectHighFOV(glm::perspective(glm::radians(fov+20.f), float(screen.GetResolution().x) / float(screen.GetResolution().y), znear, zfar)),
				Position(pos),
				Rotation(rot),
				View(Scape::Core::ViewMatrix(pos, rot)),
				ShadowCamera(false),
				znear(znear),
				zfar(zfar)
			{
				RawProject = Project;
			}
			inline Camera() :
				Project(Matrix4f(0.)),
				View(Matrix4f(0.)),
				PrevView(Matrix4f(0.)),
				Position(Vector3f(0.)),
				Rotation(Vector3f(0.)),
				ShadowCamera(false)
			{

			}
			inline void SetPosition(Vector3f Position) {
				this->Position = Position;
				PrevView = View;
				View = Scape::Core::ViewMatrix(Position, Rotation);
			}
			inline void SetRotation(Vector3f Rotation) {
				this->Rotation = Rotation;
				PrevView = View;
				View = Scape::Core::ViewMatrix(Position, Rotation);
			}
			inline void Move(Vector3f PositionAddon) {
				PrevView = View;
				View = Scape::Core::ViewMatrix(Position + PositionAddon, Rotation);
			}
			inline void Rotate(Vector3f RotationAddon) {
				PrevView = View;
				View = Scape::Core::ViewMatrix(Position, Rotation + RotationAddon);
			}

			bool HandleInput(Window& Window, float MovementSpeed, float MouseSpeed, bool Position, bool Rotation);

		};
		
		struct CameraController {

			Vector2i Previous;
			float ZoomLevel = 0.0;
			Vector3f ActualCameraPosition = Vector3f(0.);
			Vector2f ActualCameraRotation = Vector3f(0.);

			bool First = true;


			void PollCamera(Camera& Camera, Window& Window);

		};
	
}