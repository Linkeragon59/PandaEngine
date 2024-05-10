#pragma once

#include "Render_Resource.h"

namespace Render
{
	struct Image : public RenderResource
	{
		static bool DepthFormatHasStencilAspect(VkFormat aDepthFormat)
		{
			return aDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT;
		}

		Image() {}
		Image(uint aWidth, uint aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties);
		~Image();

		void Create(uint aWidth, uint aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties);
		void Destroy();

		VkImage myImage = VK_NULL_HANDLE;
		VmaAllocation myAllocation = VK_NULL_HANDLE;
		VkFormat myFormat = VK_FORMAT_UNDEFINED;
		VkExtent3D myExtent = { 0, 0, 0 };

		void CreateImageView(VkImageAspectFlags someAspects);
		VkImageView myImageView = VK_NULL_HANDLE;

		void CreateImageSampler(VkSamplerAddressMode aMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
		VkSampler myImageSampler = VK_NULL_HANDLE;

		void SetupDescriptor(VkImageLayout aLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkDescriptorImageInfo myDescriptor = {};

		void TransitionLayout(VkImageLayout anOldLayout, VkImageLayout aNewLayout, VkQueue aQueue, VkCommandPool aCommandPool = VK_NULL_HANDLE);

		VkDevice myDevice = VK_NULL_HANDLE;
		VmaAllocator myAllocator = VK_NULL_HANDLE;
	};

	typedef SharedPtr<Image> ImagePtr;
}
