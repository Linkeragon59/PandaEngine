#pragma once

namespace Render::Debug
{
	void PopulateValidationLayers(std::vector<const char*>& anOutLayerList);
	void PopulateDebugExtensions(std::vector<const char*>& anOutExtensionList);
	void PopulateDebugDeviceExtensions(std::vector<const char*>& anOutExtensionList);

	VkResult CreateDebugMessenger(
		VkInstance anInstance,
		const VkDebugUtilsMessengerCreateInfoEXT* aCreateInfo,
		const VkAllocationCallbacks* anAllocator,
		VkDebugUtilsMessengerEXT* anOutMessenger);

	void DestroyDebugMessenger(
		VkInstance anInstance,
		VkDebugUtilsMessengerEXT aMessenger,
		const VkAllocationCallbacks* anAllocator);

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT aMessageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT aMessageType,
		const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData,
		void* aUserData);

	void FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& someOutInfo);

	void SetupDebugMarkers(VkDevice aDevice);
	void BeginRegion(VkCommandBuffer aCmdbuffer, const char* aMarkerName, glm::vec4 aColor);
	void EndRegion(VkCommandBuffer aCmdbuffer);
}
