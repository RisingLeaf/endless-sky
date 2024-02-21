#pragma once

#include "VulkanDevice.h"
#include "VulkanBuffer.h"


#include <cstdint>
#include <map>
#include <memory>
#include "es_vulkan.h"



class VulkanModel {
public:

	std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() const;
	std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const;


	VulkanModel(VulkanDevice &device, const std::vector<float> &vertices, const std::vector<AttributeSize> &attributeDescriptors);
	~VulkanModel();

	VulkanModel(const VulkanModel &) = delete;
	VulkanModel operator=(const VulkanModel &) = delete;
	VulkanModel(VulkanModel &&) = delete;
	VulkanModel &operator=(VulkanModel &&) = delete;

	void Bind(VkCommandBuffer commandBuffer);
	void Draw(VkCommandBuffer commandBuffer);

private:
	void CreateVertexBuffers(const std::vector<float> &vertices);

	VulkanDevice &device;
	std::unique_ptr<VulkanBuffer> vertexBuffer;
	uint32_t vertexCount;

	const std::vector<AttributeSize> attributeDescriptors;
};
