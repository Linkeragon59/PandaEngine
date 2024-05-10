#pragma once

#include "Render_Resource.h"

namespace Render
{
	struct Buffer : public RenderResource
	{
		Buffer() {}
		Buffer(VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties);
		~Buffer();

		void Create(VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties);
		void Destroy();

		VkBuffer myBuffer = VK_NULL_HANDLE;
		VmaAllocation myAllocation = VK_NULL_HANDLE;

		void Map();
		void Unmap();
		void Flush();
		void* myMappedData = nullptr;

		void SetupDescriptor(VkDeviceSize aSize = VK_WHOLE_SIZE, VkDeviceSize anOffset = 0);
		VkDescriptorBufferInfo myDescriptor{};

		VmaAllocator myAllocator = VK_NULL_HANDLE;
	};

	typedef SharedPtr<Buffer> BufferPtr;
}
