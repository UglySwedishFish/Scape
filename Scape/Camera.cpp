#include "Camera.h"
#include <iostream>

bool Scape::Camera::HandleInput(Window& Window, float MovementSpeed, float MouseSpeed, bool Position, bool Rotation)
{

	bool Movement = false;

	if (Position) {

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			Core::Move_2DXZ(this->Position, MovementSpeed, static_cast<float>(this->Rotation.y - 180.0), static_cast<float>(Window.GetFrameTime()));
			Movement = true;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			Core::Move_2DXZ(this->Position, MovementSpeed, static_cast<float>(this->Rotation.y), static_cast<float>(Window.GetFrameTime()));
			Movement = true;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			Core::Move_3D(this->Position, MovementSpeed, static_cast<float>(this->Rotation.x), static_cast<float>(this->Rotation.y - 90.0), static_cast<float>(Window.GetFrameTime()));
			Movement = true;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			Core::Move_3D(this->Position, MovementSpeed, static_cast<float>(this->Rotation.x + 180.0), static_cast<float>(this->Rotation.y + 90.0), static_cast<float>(Window.GetFrameTime()), true);
			Movement = true;
		}
	}
	if (Rotation) {

		

		float RelativeMouseX = sf::Mouse::getPosition(*Window.GetRawWindow()).x - Window.GetResolution().x / 2;
		float RelativeMouseY = sf::Mouse::getPosition(*Window.GetRawWindow()).y - Window.GetResolution().y / 2;

		if (abs(RelativeMouseX) > 0.1 || abs(RelativeMouseY) > 0.1)
			Movement = true;

		sf::Mouse::setPosition(sf::Vector2i(Window.GetResolution().x / 2, Window.GetResolution().y / 2), *Window.GetRawWindow());

		Vector2f PreviousCameraRotation = Vector2f(this->Rotation.x, this->Rotation.y);

		Vector3f TempRotation = this->Rotation;

		this->Rotation.y += RelativeMouseX * MouseSpeed;
		this->Rotation.x += static_cast<float>(RelativeMouseY * MouseSpeed);

		this->Rotation.y = this->Rotation.y > 360.0 ? this->Rotation.y - 360.0 : this->Rotation.y < 0.0 ? this->Rotation.y + 360.0 : this->Rotation.y;




	}
	return Movement;
}

void Scape::CameraController::PollCamera(Camera& Camera, Window& Window)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {

		//grab some data :) 

		Window.GetRawWindow()->setMouseCursorVisible(false);

		if (!First) {
			float RelativeMouseX = sf::Mouse::getPosition(*Window.GetRawWindow()).x - Window.GetResolution().x / 2;
			float RelativeMouseY = sf::Mouse::getPosition(*Window.GetRawWindow()).y - Window.GetResolution().y / 2;
			
			Vector4f ClipSpace = Vector4f(-RelativeMouseX * 0.25, RelativeMouseY * 0.25, 0.0, 1.0);
			Vector4f View = glm::inverse(Camera.Project) * ClipSpace;
			View /= View.w;
			Vector3f World = Matrix3f(glm::inverse(Camera.View)) * Vector3f(View);

			ActualCameraPosition += World * 0.25f;
			Core::Move_3D(ActualCameraPosition, -0.05f, ActualCameraRotation.x - 180.f, ActualCameraRotation.y + 90.f, 1.0f, true);

		}
		else {
			Previous = Vector2i(sf::Mouse::getPosition(*Window.GetRawWindow()).x, sf::Mouse::getPosition(*Window.GetRawWindow()).y);
		}

		First = false;
		sf::Mouse::setPosition(sf::Vector2i(Window.GetResolution().x / 2, Window.GetResolution().y / 2), *Window.GetRawWindow());


	}
	else if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {

		Window.GetRawWindow()->setMouseCursorVisible(false);


		if (!First) {
			float RelativeMouseX = sf::Mouse::getPosition(*Window.GetRawWindow()).x - Window.GetResolution().x / 2;
			float RelativeMouseY = sf::Mouse::getPosition(*Window.GetRawWindow()).y - Window.GetResolution().y / 2;

			ActualCameraRotation.x += RelativeMouseY;
			ActualCameraRotation.y += RelativeMouseX;
		}
		else {
			Previous = Vector2i(sf::Mouse::getPosition(*Window.GetRawWindow()).x, sf::Mouse::getPosition(*Window.GetRawWindow()).y);
		}
		First = false;
		sf::Mouse::setPosition(sf::Vector2i(Window.GetResolution().x / 2, Window.GetResolution().y / 2), *Window.GetRawWindow());


	}
	else {

		if (!First) {
			sf::Mouse::setPosition(sf::Vector2i(Previous.x, Previous.y), *Window.GetRawWindow());
		}

		First = true;
	}

	Vector3f BasePosition = ActualCameraPosition;

	Core::Move_3D(BasePosition, ZoomLevel, ActualCameraRotation.x - 180.f, ActualCameraRotation.y + 90.f, 1.0f, true);

	Camera.Position = BasePosition;
	Camera.Rotation = Vector3f(ActualCameraRotation, 0.f);
}
