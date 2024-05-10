#include "Render_Helpers.h"

namespace Render::Helpers
{
	void CheckInstanceLayersSupport(std::vector<const char*>& someInOutLayers)
	{
		uint layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		someInOutLayers.erase(std::remove_if(someInOutLayers.begin(), someInOutLayers.end(), [availableLayers](const char* layer) {
			for (uint i = 0; i < availableLayers.size(); ++i)
			{
				if (strcmp(layer, availableLayers[i].layerName) == 0)
				{
					return false;
				}
			}
			Log("Layer %s is not supported and couldn't be enabled!", layer);
			return true;
		}), someInOutLayers.end());
	}

	void CheckInstanceExtensionsSupport(std::vector<const char*>& someInOutExtensions)
	{
		uint extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		someInOutExtensions.erase(std::remove_if(someInOutExtensions.begin(), someInOutExtensions.end(), [availableExtensions](const char* extension) {
			for (uint i = 0; i < availableExtensions.size(); ++i)
			{
				if (strcmp(extension, availableExtensions[i].extensionName) == 0)
				{
					return false;
				}
			}
			Log("Extension %s is not supported and couldn't be enabled!", extension);
			return true;
		}), someInOutExtensions.end());
	}

	VkCommandBuffer BeginOneTimeCommand(VkCommandPool aCommandPool)
	{
		if (aCommandPool == VK_NULL_HANDLE)
			aCommandPool = RenderCore::GetInstance()->GetGraphicsCommandPool();

		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = aCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(RenderCore::GetInstance()->GetDevice(), &allocInfo, &commandBuffer), "Failed to alloc a one time command buffer");

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Failed to begin a one time command buffer");

		return commandBuffer;
	}

	void EndOneTimeCommand(VkCommandBuffer aCommandBuffer, VkQueue aQueue, VkCommandPool aCommandPool /*= VK_NULL_HANDLE*/)
	{
		if (aCommandPool == VK_NULL_HANDLE)
			aCommandPool = RenderCore::GetInstance()->GetGraphicsCommandPool();

		VK_CHECK_RESULT(vkEndCommandBuffer(aCommandBuffer), "Failed to end a one time command buffer");

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &aCommandBuffer;

		vkQueueSubmit(aQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(aQueue);

		vkFreeCommandBuffers(RenderCore::GetInstance()->GetDevice(), aCommandPool, 1, &aCommandBuffer);
	}
}
