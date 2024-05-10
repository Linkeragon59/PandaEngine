#include "Render_Image.h"

#include "Render_Helpers.h"

namespace Render
{

	Image::Image(uint aWidth, uint aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties)
	{
		Create(aWidth, aHeight, aFormat, aTiling, aUsage, someProperties);
	}

	Image::~Image()
	{
		Destroy();
	}

	void Image::Create(uint aWidth, uint aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties)
	{
		myDevice = RenderCore::GetInstance()->GetDevice();
		myAllocator = RenderCore::GetInstance()->GetAllocator();

		myFormat = aFormat;
		myExtent = { aWidth, aHeight, 1 };

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = myFormat;
		imageInfo.extent = myExtent;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = aTiling;
		imageInfo.usage = aUsage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 0; // Ignored when using VK_SHARING_MODE_EXCLUSIVE
		imageInfo.pQueueFamilyIndices = nullptr;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocInfo{};
		if (someProperties == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else
		{
			Assert(false, "Image allocation usage not supported yet.");
		}
		allocInfo.requiredFlags = someProperties;

		VK_CHECK_RESULT(vmaCreateImage(myAllocator, &imageInfo, &allocInfo, &myImage, &myAllocation, nullptr), "Failed to create an image!");
	}

	void Image::Destroy()
	{
		myDescriptor = {};

		if (myImageSampler)
		{
			vkDestroySampler(myDevice, myImageSampler, nullptr);
			myImageSampler = VK_NULL_HANDLE;
		}

		if (myImageView)
		{
			vkDestroyImageView(myDevice, myImageView, nullptr);
			myImageView = VK_NULL_HANDLE;
		}

		if (myImage)
		{
			vmaDestroyImage(myAllocator, myImage, myAllocation);
			myImage = VK_NULL_HANDLE;
			myAllocation = VK_NULL_HANDLE;
			myFormat = VK_FORMAT_UNDEFINED;
		}
	}

	void Image::CreateImageView(VkImageAspectFlags someAspects)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = myImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = myFormat;
		viewInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
		};
		viewInfo.subresourceRange.aspectMask = someAspects;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VK_CHECK_RESULT(vkCreateImageView(myDevice, &viewInfo, nullptr, &myImageView), "Failed to create an image view!");
	}

	void Image::CreateImageSampler(VkSamplerAddressMode aMode)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = aMode;
		samplerInfo.addressModeV = aMode;
		samplerInfo.addressModeW = aMode;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		VK_CHECK_RESULT(vkCreateSampler(myDevice, &samplerInfo, nullptr, &myImageSampler), "Failed to create a sampler!");
	}

	void Image::SetupDescriptor(VkImageLayout aLayout)
	{
		myDescriptor.sampler = myImageSampler;
		myDescriptor.imageView = myImageView;
		myDescriptor.imageLayout = aLayout;
	}

	void Image::TransitionLayout(VkImageLayout anOldLayout, VkImageLayout aNewLayout, VkQueue aQueue, VkCommandPool aCommandPool)
	{
		VkCommandBuffer commandBuffer = Helpers::BeginOneTimeCommand(aCommandPool);

		VkPipelineStageFlags sourceStage = 0;
		VkPipelineStageFlags destinationStage = 0;

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = anOldLayout;
		barrier.newLayout = aNewLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = myImage;

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (anOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && aNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (anOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && aNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (anOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && aNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			Assert(false, "Image transition not supported yet!");
		}
		
		if (aNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (DepthFormatHasStencilAspect(myFormat))
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		Helpers::EndOneTimeCommand(commandBuffer, aQueue, aCommandPool);
	}
}
