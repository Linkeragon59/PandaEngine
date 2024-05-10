#pragma once

#include "Render_RenderTarget.h"

struct GLFWwindow;

namespace Render
{
	class SwapChain : public RenderTarget
	{
	public:
		SwapChain(GLFWwindow* aWindow);
		~SwapChain() override;
		void AttachRenderer(Renderer* aRenderer) override;
		GLFWwindow* GetAssociatedWindow() const override { return myWindow; }
		
		bool TrySetup();
		void Cleanup();
		bool IsSetup() const { return myIsSetup; }

		void OnResize();

		void AcquireNext(VkSemaphore aSignalSemaphore);
		void Present(VkSemaphore aWaitSemaphore);

		GLFWwindow* GetWindowHandle() const { return myWindow; }

	private:
		bool IsReadyToRender() const;
		void SetupVkSwapChain();
		void CleanupVkSwapChain();
		bool myIsSetup = false;

		GLFWwindow* myWindow = nullptr;
		uint myFramebufferResizedCallbackId = UINT_MAX;
		bool myFramebufferResized = false;
		VkSurfaceKHR mySurface = VK_NULL_HANDLE;

		VkSwapchainKHR myVkSwapChain = VK_NULL_HANDLE;
	};
}
