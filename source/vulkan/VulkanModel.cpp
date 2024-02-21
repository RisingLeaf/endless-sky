#include "VulkanModel.h"

#include "es_vulkan.h"
#include "../Logger.h"
#include "VulkanBuffer.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>



VulkanModel::VulkanModel(VulkanDevice &device, const std::vector<float> &vertices, const std::vector<AttributeSize> &attributeDescriptors)
: device(device), attributeDescriptors(attributeDescriptors)
{
	CreateVertexBuffers(vertices);
}



VulkanModel::~VulkanModel() { }



void VulkanModel::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = {vertexBuffer->GetBuffer()};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}



void VulkanModel::Draw(VkCommandBuffer commandBuffer)
{
	vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}



void VulkanModel::CreateVertexBuffers(const std::vector<float> &vertices)
{
	uint32_t sizePerVertex = 0;
	for(const auto &attributeDescriptor : attributeDescriptors)
		sizePerVertex += EsToVulkan::FORMAT_MAP_SIZE.at(attributeDescriptor);

	vertexCount = static_cast<uint32_t>(vertices.size() / sizePerVertex);
	assert(vertexCount >= 3 && "vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(float) * vertexCount * sizePerVertex;
	uint32_t vertexSize = sizePerVertex * sizeof(float);

	VulkanBuffer stagingBuffer(device, vertexSize, vertexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void *)vertices.data());

	vertexBuffer = std::make_unique<VulkanBuffer>(device, vertexSize, vertexCount,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	device.CopyBuffer(stagingBuffer.GetBuffer(), vertexBuffer->GetBuffer(), bufferSize);
}
