#include "Core_EntityCameraComponent.h"

#include "Core_InputModule.h"
#include "Core_WindowModule.h"

namespace Core
{
	// TODO : Implementation is quite broken

	namespace
	{
		float locSpeed = 0.01f;
		float locSensitivity = 0.1f;
	}

	EntityCameraComponent::EntityCameraComponent(GLFWwindow* aWindow)
		: myWindow(aWindow)
	{
		InputModule* inputModule = InputModule::GetInstance();
		myScrollCallbackId = inputModule->AddScrollCallback([this](double aX, double aY) {
			(void)aX;
			myFov -= (float)aY;
			if (myFov < 1.0f)
				myFov = 1.0f;
			if (myFov > 90.0f)
				myFov = 90.0f;
		}, myWindow);

		inputModule->PollCursorPosition(myPrevPosX, myPrevPosY, myWindow);
	}

	EntityCameraComponent::~EntityCameraComponent()
	{
		InputModule* inputModule = InputModule::GetInstance();
		inputModule->RemoveScrollCallback(myScrollCallbackId);
	}

	void EntityCameraComponent::Update()
	{
		myLeft = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), myDirection);
		myUp = glm::cross(myDirection, myLeft);

		InputModule* inputModule = InputModule::GetInstance();

		double mouseX, mouseY;
		inputModule->PollCursorPosition(mouseX, mouseY, myWindow);

		double deltaMouseX = locSensitivity * (myPrevPosX - mouseX);
		double deltaMouseY = locSensitivity * (myPrevPosY - mouseY);

		myPrevPosX = mouseX;
		myPrevPosY = mouseY;

		if (inputModule->PollMouseInput(Input::MouseLeft, myWindow) == Input::Status::Pressed)
		{
			myPitch += (float)deltaMouseY;
			if (myPitch > 89.0f)
				myPitch = 89.0f;
			if (myPitch < -89.0f)
				myPitch = -89.0f;
			myYaw -= (float)deltaMouseX;

			myDirection.x = cos(glm::radians(myYaw)) * cos(glm::radians(myPitch));
			myDirection.y = sin(glm::radians(myPitch));
			myDirection.z = sin(glm::radians(myYaw)) * cos(glm::radians(myPitch));
			myDirection = glm::normalize(myDirection);
		}
		if (inputModule->PollMouseInput(Input::MouseMiddle, myWindow) == Input::Status::Pressed)
		{
			myPosition += -0.1f * ((float)deltaMouseX * myLeft + (float)deltaMouseY * myUp);
		}
		if (inputModule->PollKeyInput(Input::KeyW, myWindow) == Input::Status::Pressed)
		{
			myPosition += locSpeed * myDirection;
		}
		if (inputModule->PollKeyInput(Input::KeyS, myWindow) == Input::Status::Pressed)
		{
			myPosition -= locSpeed * myDirection;
		}
		if (inputModule->PollKeyInput(Input::KeyA, myWindow) == Input::Status::Pressed)
		{
			myPosition += locSpeed * myLeft;
		}
		if (inputModule->PollKeyInput(Input::KeyD, myWindow) == Input::Status::Pressed)
		{
			myPosition -= locSpeed * myLeft;
		}
	}
}
