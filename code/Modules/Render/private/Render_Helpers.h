#pragma once

#define VK_CHECK_RESULT(X, Msg) Verify((X == VK_SUCCESS), Msg)

namespace Render::Helpers
{
	void CheckInstanceLayersSupport(std::vector<const char*>& someInOutLayers);
	void CheckInstanceExtensionsSupport(std::vector<const char*>& someInOutExtensions);

	VkCommandBuffer BeginOneTimeCommand(VkCommandPool aCommandPool = VK_NULL_HANDLE);
	void EndOneTimeCommand(VkCommandBuffer aCommandBuffer, VkQueue aQueue, VkCommandPool aCommandPool = VK_NULL_HANDLE);
}
