#include "Render_Device.h"

namespace Render
{
	VulkanDevice::VulkanDevice(VkPhysicalDevice aPhysicalDevice)
		: myPhysicalDevice(aPhysicalDevice)
	{
		Assert(myPhysicalDevice);

		vkGetPhysicalDeviceProperties(myPhysicalDevice, &myProperties);
		vkGetPhysicalDeviceFeatures(myPhysicalDevice, &myFeatures);
		vkGetPhysicalDeviceMemoryProperties(myPhysicalDevice, &myMemoryProperties);

		uint queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(myPhysicalDevice, &queueFamilyCount, nullptr);
		Assert(queueFamilyCount > 0);
		myQueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(myPhysicalDevice, &queueFamilyCount, myQueueFamilyProperties.data());

		uint extensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(myPhysicalDevice, nullptr, &extensionsCount, nullptr);
		if (extensionsCount > 0)
		{
			mySupportedExtensions.resize(extensionsCount);
			vkEnumerateDeviceExtensionProperties(myPhysicalDevice, nullptr, &extensionsCount, mySupportedExtensions.data());
		}
	}

	VulkanDevice::~VulkanDevice()
	{
		if (myLogicalDevice)
		{
			vkDeviceWaitIdle(myLogicalDevice);

			if (myVmaAllocator)
				vmaDestroyAllocator(myVmaAllocator);

			vkDestroyCommandPool(myLogicalDevice, myGraphicsCommandPool, nullptr);
			vkDestroyDevice(myLogicalDevice, nullptr);
		}
	}

	bool VulkanDevice::SupportsExtension(const char* anExtension)
	{
		for (VkExtensionProperties extension : mySupportedExtensions)
			if (strcmp(extension.extensionName, anExtension) == 0)
				return true;
		return false;
	}

	void VulkanDevice::SetupLogicalDevice(
		const VkPhysicalDeviceFeatures& someEnabledFeatures,
		const std::vector<const char*>& someEnabledLayers,
		const std::vector<const char*>& someEnabledExtensions,
		VkQueueFlags someRequestedQueueTypes)
	{
		myEnabledFeatures = someEnabledFeatures;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		const float defaultQueuePriority(0.0f);

		// Graphics queue
		if (someRequestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			myQueueFamilyIndices.myGraphicsFamily = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = myQueueFamilyIndices.myGraphicsFamily.value();
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}

		// Compute queue
		if (someRequestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			myQueueFamilyIndices.myComputeFamily = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);

			if (myQueueFamilyIndices.myComputeFamily != myQueueFamilyIndices.myGraphicsFamily)
			{
				// We need an additional queue for dedicated compute
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = myQueueFamilyIndices.myComputeFamily.value();
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}

		// Transfer queue
		if (someRequestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			myQueueFamilyIndices.myTransferFamily = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);

			if ((myQueueFamilyIndices.myTransferFamily != myQueueFamilyIndices.myGraphicsFamily)
			&& (myQueueFamilyIndices.myTransferFamily != myQueueFamilyIndices.myComputeFamily))
			{
				// We need an additional queue for dedicated transfer
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = myQueueFamilyIndices.myTransferFamily.value();
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}

		// Logical Device creation
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.enabledLayerCount = static_cast<uint>(someEnabledLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = someEnabledLayers.data();
		deviceCreateInfo.enabledExtensionCount = static_cast<uint>(someEnabledExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = someEnabledExtensions.data();
		deviceCreateInfo.pEnabledFeatures = &myEnabledFeatures;

		VK_CHECK_RESULT(vkCreateDevice(myPhysicalDevice, &deviceCreateInfo, nullptr, &myLogicalDevice), "Could not create the logical device");

		if (myQueueFamilyIndices.myGraphicsFamily.has_value())
		{
			vkGetDeviceQueue(myLogicalDevice, myQueueFamilyIndices.myGraphicsFamily.value(), 0, &myGraphicsQueue);

			VkCommandPoolCreateInfo commandPoolInfo{};
			commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolInfo.queueFamilyIndex = myQueueFamilyIndices.myGraphicsFamily.value();
			commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			VK_CHECK_RESULT(vkCreateCommandPool(myLogicalDevice, &commandPoolInfo, nullptr, &myGraphicsCommandPool), "Failed to create the graphics command pool");
		}
	}

	void VulkanDevice::SetupVmaAllocator(VkInstance anInstance, uint aVulkanApiVersion)
	{
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.instance = anInstance;
		allocatorInfo.physicalDevice = myPhysicalDevice;
		allocatorInfo.device = myLogicalDevice;
		allocatorInfo.vulkanApiVersion = aVulkanApiVersion;

		VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &myVmaAllocator), "Could not create the vma allocator");
	}

	uint VulkanDevice::GetQueueFamilyIndex(VkQueueFlags someQueueTypes) const
	{
		if ((someQueueTypes & VK_QUEUE_COMPUTE_BIT) == someQueueTypes)
		{
			// Dedicated queue for compute
			// Try to find a queue family index that supports compute but not graphics
			for (uint i = 0; i < static_cast<uint>(myQueueFamilyProperties.size()); ++i)
			{
				VkQueueFlags flags = myQueueFamilyProperties[i].queueFlags;
				if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT))
					return i;
			}
		}

		if ((someQueueTypes & VK_QUEUE_TRANSFER_BIT) == someQueueTypes)
		{
			// Dedicated queue for transfer
			// Try to find a queue family index that supports transfer but not graphics and compute
			for (uint i = 0; i < static_cast<uint>(myQueueFamilyProperties.size()); ++i)
			{
				VkQueueFlags flags = myQueueFamilyProperties[i].queueFlags;
				if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT))
					return i;
			}
		}

		// For other queue types, return the first one to support the requested flags
		for (uint i = 0; i < static_cast<uint>(myQueueFamilyProperties.size()); ++i)
		{
			if ((myQueueFamilyProperties[i].queueFlags & someQueueTypes) == someQueueTypes)
				return i;
		}

		Assert(false, "Couldn't find a matching queue family index");
		return 0;
	}

	VkFormat VulkanDevice::FindSupportedFormat(const std::vector<VkFormat>& someCandidateFormats, VkImageTiling aTiling, VkFormatFeatureFlags someFeatures)
	{
		for (VkFormat format : someCandidateFormats)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(myPhysicalDevice, format, &props);

			if (aTiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & someFeatures) == someFeatures)
				return format;
			else if (aTiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & someFeatures) == someFeatures)
				return format;
		}

		Assert(false, "Couldn't find an available format!");
		return VK_FORMAT_UNDEFINED;
	}

	VkFormat VulkanDevice::FindBestDepthFormat()
	{
		// Start with the highest precision packed format
		std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		return FindSupportedFormat(depthFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}
