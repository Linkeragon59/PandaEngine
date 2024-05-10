#pragma once

struct GLFWwindow;

namespace Core
{
	class EntityCameraComponent
	{
	public:
		EntityCameraComponent(GLFWwindow* aWindow);
		~EntityCameraComponent();

		void Update();

		void SetPosition(const glm::vec3& aPosition) { myPosition = aPosition; }
		void SetDirection(const glm::vec3& aDirection) { myDirection = aDirection; }
		void SetAspectRatio(float anAspectRatio) { myAspectRatio = anAspectRatio; }
		void SetFov(float aFov) { myFov = aFov; }
		void SetNearFar(float aZNear, float aZFar) { myZNear = aZNear; myZFar = aZFar; }

		glm::mat4 GetViewMatrix() const { return glm::lookAt(myPosition, myPosition + myDirection, myUp); }
		glm::mat4 GetPerspectiveMatrix() const
		{
			glm::mat4 perspective = glm::perspective(glm::radians(myFov), myAspectRatio, myZNear, myZFar);
			perspective[1][1] *= -1.f;
			return perspective;
		}

		GLFWwindow* GetWindow() const { return myWindow; }

	private:
		float myAspectRatio = 1.0f;
		float myFov = 45.0f;
		float myZNear = 0.1f;
		float myZFar = 256.0f;

		glm::vec3 myPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 myDirection = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 myUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 myLeft = glm::vec3(1.0f, 0.0f, 0.0f);

		GLFWwindow* myWindow = nullptr;
		uint myScrollCallbackId = UINT_MAX;

		double myPrevPosX = 0.0;
		double myPrevPosY = 0.0;
		float myPitch = 0.f;
		float myYaw = -90.f;
	};
}
