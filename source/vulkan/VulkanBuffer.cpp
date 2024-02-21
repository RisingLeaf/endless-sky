#include "VulkanBuffer.h"

#include <cassert>
#include <cstring>



VkDeviceSize VulkanBuffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
{
	if(minOffsetAlignment > 0)
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	return instanceSize;
}



VulkanBuffer::VulkanBuffer(VulkanDevice &device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
	VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment)
: device{device}, instanceSize{instanceSize}, instanceCount{instanceCount}, usageFlags{usageFlags},
	memoryPropertyFlags{memoryPropertyFlags}
{
	alignmentSize = GetAlignment(instanceSize, minOffsetAlignment);
	bufferSize = alignmentSize * instanceCount;
	device.CreateBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
}



VulkanBuffer::~VulkanBuffer()
{
	Unmap();
	vkDestroyBuffer(device.Device(), buffer, nullptr);
	vkFreeMemory(device.Device(), memory, nullptr);
}



VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
	assert(buffer && memory && "Called map on buffer before create");
	return vkMapMemory(device.Device(), memory, offset, size, 0, &mapped);
}



void VulkanBuffer::Unmap()
{
	if(mapped)
	{
		vkUnmapMemory(device.Device(), memory);
		mapped = nullptr;
	}
}



void VulkanBuffer::WriteToBuffer(const void *data, VkDeviceSize size, VkDeviceSize offset)
{
	assert(mapped && "Cannot copy to unmapped buffer");

	if(size == VK_WHOLE_SIZE)
		memcpy(mapped, data, bufferSize);
	else
	{
		char *memOffset = (char *)mapped;
		memOffset += offset;
		memcpy(memOffset, data, size);
	}
}



VkResult VulkanBuffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkFlushMappedMemoryRanges(device.Device(), 1, &mappedRange);
}



VkResult VulkanBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkInvalidateMappedMemoryRanges(device.Device(), 1, &mappedRange);
}



VkDescriptorBufferInfo VulkanBuffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
	return VkDescriptorBufferInfo{buffer, offset, size};
}


void VulkanBuffer::WriteToIndex(void *data, int index)
{
	WriteToBuffer(data, instanceSize, index * alignmentSize);
}



VkResult VulkanBuffer::FlushIndex(int index)
{
	return Flush(alignmentSize, index * alignmentSize);
}



VkDescriptorBufferInfo VulkanBuffer::DescriptorInfoForIndex(int index)
{
  return DescriptorInfo(alignmentSize, index * alignmentSize);
}



VkResult VulkanBuffer::InvalidateIndex(int index)
{
	return Invalidate(alignmentSize, index * alignmentSize);
}
