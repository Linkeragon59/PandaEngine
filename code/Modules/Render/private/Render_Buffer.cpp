#include "Render_Buffer.h"

namespace Render
{
	Buffer::Buffer(VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties)
	{
		Create(aSize, aUsage, someProperties);
	}

	Buffer::~Buffer()
	{
		Destroy();
	}

	void Buffer::Create(VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties)
	{
		myAllocator = RenderCore::GetInstance()->GetAllocator();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = aSize;
		bufferInfo.usage = aUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = nullptr;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.flags = 0;
		if (someProperties == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else if (someProperties == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			if (aUsage == VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			{
				allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			}
			else
			{
				allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			}
		}
		else
		{
			Assert(false, "Buffer allocation usage not supported yet.");
		}
		allocInfo.requiredFlags = someProperties;

		VK_CHECK_RESULT(vmaCreateBuffer(myAllocator, &bufferInfo, &allocInfo, &myBuffer, &myAllocation, nullptr), "Failed to create a buffer!");
	}

	void Buffer::Destroy()
	{
		myDescriptor = {};

		if (myMappedData)
		{
			Unmap();
		}

		if (myBuffer)
		{
			vmaDestroyBuffer(myAllocator, myBuffer, myAllocation);
			myBuffer = VK_NULL_HANDLE;
			myAllocation = VK_NULL_HANDLE;
		}
	}

	void Buffer::Map()
	{
		VK_CHECK_RESULT(vmaMapMemory(myAllocator, myAllocation, &myMappedData), "Failed to map a buffer allocation!");
	}

	void Buffer::Unmap()
	{
		vmaUnmapMemory(myAllocator, myAllocation);
		myMappedData = nullptr;
	}

	void Buffer::Flush()
	{
		VK_CHECK_RESULT(vmaFlushAllocation(myAllocator, myAllocation, 0, VK_WHOLE_SIZE), "Failed to flush a buffer allocation!");
	}

	void Buffer::SetupDescriptor(VkDeviceSize aSize, VkDeviceSize anOffset)
	{
		myDescriptor.buffer = myBuffer;
		myDescriptor.offset = anOffset;
		myDescriptor.range = aSize;
	}
}
