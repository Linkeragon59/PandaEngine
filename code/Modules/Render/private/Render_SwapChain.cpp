#include "Render_SwapChain.h"

#include "Render_Device.h"
#include "Render_Renderer.h"

#include "Core_WindowModule.h"

#include "GLFW/glfw3.h"

namespace Render
{
	SwapChain::SwapChain(GLFWwindow* aWindow)
		: RenderTarget()
		, myWindow(aWindow)
	{
		myFramebufferResizedCallbackId = Core::WindowModule::GetInstance()->AddFramebufferSizeCallback([this](int aWidth, int aHeight) {
			(void)aWidth;
			(void)aHeight;
			myFramebufferResized = true;
		}, myWindow);

		VK_CHECK_RESULT(glfwCreateWindowSurface(RenderCore::GetInstance()->GetVkInstance(), myWindow, nullptr, &mySurface), "Failed to create the surface!");

		myFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		TrySetup();
	}

	SwapChain::~SwapChain()
	{
		Cleanup();

		vkDestroySurfaceKHR(RenderCore::GetInstance()->GetVkInstance(), mySurface, nullptr);

		Core::WindowModule::GetInstance()->RemoveFramebufferSizeCallback(myFramebufferResizedCallbackId);
	}

	void SwapChain::AttachRenderer(Renderer* aRenderer)
	{
		Assert(!myRenderer);
		myRenderer = aRenderer;
		
		if (myIsSetup)
			myRenderer->Setup();
	}

	bool SwapChain::TrySetup()
	{
		if (!IsReadyToRender())
			return false;

		SetupVkSwapChain();

		if (myRenderer)
		{
			Assert(!myRenderer->IsSetup());
			myRenderer->Setup();
		}

		myIsSetup = true;
		return true;
	}

	void SwapChain::Cleanup()
	{
		if (!myIsSetup)
			return;

		myIsSetup = false;

		if (myRenderer && myRenderer->IsSetup())
			myRenderer->Cleanup();

		CleanupVkSwapChain();
	}

	void SwapChain::OnResize()
	{
		// TODO : Handle this better
		vkDeviceWaitIdle(myDevice);

		Cleanup();
	}

	void SwapChain::AcquireNext(VkSemaphore aSignalSemaphore)
	{
		VkResult result = vkAcquireNextImageKHR(myDevice, myVkSwapChain, UINT64_MAX, aSignalSemaphore, VK_NULL_HANDLE, &myCurrentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			OnResize();
			return;
		}
		
		Assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire a swapchain image!");
	}

	void SwapChain::Present(VkSemaphore aWaitSemaphore)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &aWaitSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &myVkSwapChain;
		presentInfo.pImageIndices = &myCurrentImageIndex;

		VkResult result = vkQueuePresentKHR(RenderCore::GetInstance()->GetGraphicsQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || myFramebufferResized)
		{
			myFramebufferResized = false;
			OnResize();
			return;
		}
		
		// TODO : can get VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT if retrieving a window that was behind a fullscreen window
		Assert(result == VK_SUCCESS, "Failed to present a swapchain image!");
	}

	bool SwapChain::IsReadyToRender() const
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(myWindow, &width, &height);
		return (width > 0 && height > 0);
	}

	void SwapChain::SetupVkSwapChain()
	{
		VkPhysicalDevice physicalDevice = RenderCore::GetInstance()->GetPhysicalDevice();

		// TODO: Support separate queues for graphics and present. See Vulkan Tutorial
		Assert(RenderCore::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.has_value());
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			physicalDevice,
			RenderCore::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.value(),
			mySurface,
			&presentSupport);
		Assert(presentSupport, "The device doesn't support presenting on the graphics queue!");

		VkSurfaceCapabilitiesKHR capabilities = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mySurface, &capabilities);
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			myExtent = capabilities.currentExtent;
		}
		else
		{
			int width = 0, height = 0;
			glfwGetFramebufferSize(myWindow, &width, &height);
			myExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint)width));
			myExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint)height));
		}

		uint formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mySurface, &formatCount, nullptr);
		Assert(formatCount > 0);
		std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mySurface, &formatCount, availableFormats.data());
		VkSurfaceFormatKHR surfaceFormat = availableFormats[0];
		for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				surfaceFormat = availableFormat;
		}
		myColorFormat = surfaceFormat.format;

		uint presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mySurface, &presentModeCount, nullptr);
		Assert(presentModeCount > 0);
		std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mySurface, &presentModeCount, availablePresentModes.data());
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				presentMode = availablePresentMode;
		}

		uint imageCount = std::clamp(RenderCore::GetInstance()->GetInFlightFramesCount(),
			capabilities.minImageCount,
			capabilities.maxImageCount);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mySurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = myColorFormat;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = myExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK_RESULT(vkCreateSwapchainKHR(myDevice, &createInfo, nullptr, &myVkSwapChain), "Failed to create a swap chain!");

		vkGetSwapchainImagesKHR(myDevice, myVkSwapChain, &imageCount, nullptr);
		std::vector<VkImage> images(imageCount);
		vkGetSwapchainImagesKHR(myDevice, myVkSwapChain, &imageCount, images.data());

		myImages.resize(imageCount);
		for (uint i = 0; i < imageCount; ++i)
		{
			myImages[i] = new Image();
			myImages[i]->myImage = images[i];
			myImages[i]->myFormat = myColorFormat;
			myImages[i]->myExtent = { myExtent.width, myExtent.height, 1 };

			VkImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.image = myImages[i]->myImage;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = myImages[i]->myFormat;
			viewCreateInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;

			VK_CHECK_RESULT(vkCreateImageView(myDevice, &viewCreateInfo, nullptr, &myImages[i]->myImageView), "Failed to create an image view for the swap chain!");
		}
		myCurrentImageIndex = 0;
	}

	void SwapChain::CleanupVkSwapChain()
	{
		for (uint i = 0, e = GetImagesCount(); i < e; ++i)
		{
			vkDestroyImageView(myDevice, myImages[i]->myImageView, nullptr);
			myImages[i]->myImage = VK_NULL_HANDLE;
			myImages[i]->myImageView = VK_NULL_HANDLE;
		}
		myImages.clear();

		vkDestroySwapchainKHR(myDevice, myVkSwapChain, nullptr);
		myVkSwapChain = VK_NULL_HANDLE;

		myExtent = {};
	}
}
