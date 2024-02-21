#pragma once

#include "VulkanBuffer.h"
#include "VulkanDescriptors.h"

#include <map>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

enum class AttributeSize : uint32_t {
	SIMPLE_FLOAT = 1,
	VECTOR_TWO   = 2,
	VECTOR_THREE = 3,
	VECTOR_FOUR  = 4,
};

namespace EsToVulkan {
    const std::map<AttributeSize, VkFormat> FORMAT_MAP_VULKAN = {
		{AttributeSize::SIMPLE_FLOAT, VK_FORMAT_R32_SFLOAT},
		{AttributeSize::VECTOR_TWO, VK_FORMAT_R32G32_SFLOAT},
		{AttributeSize::VECTOR_THREE, VK_FORMAT_R32G32B32A32_SFLOAT},
		{AttributeSize::VECTOR_FOUR, VK_FORMAT_R32G32B32A32_SFLOAT},
	};
	const std::map<AttributeSize, unsigned long> FORMAT_MAP_SIZE = {
		{AttributeSize::SIMPLE_FLOAT, 1},
		{AttributeSize::VECTOR_TWO,   2},
		{AttributeSize::VECTOR_THREE, 4},
		{AttributeSize::VECTOR_FOUR,  4},
	};
	const std::map<AttributeSize, unsigned long> FORMAT_MAP_TYPE_SIZE = {
		{AttributeSize::SIMPLE_FLOAT, sizeof(float)},
		{AttributeSize::VECTOR_TWO,   sizeof(float) * 4},
		{AttributeSize::VECTOR_THREE, sizeof(float) * 4},
		{AttributeSize::VECTOR_FOUR,  sizeof(float) * 4},
	};
}

struct ShaderInfo {
	std::vector<AttributeSize> attributeLayout;
	std::vector<AttributeSize> uniformLayout;

	std::string vertexShaderFilename;
	std::string fragmentShaderFilename;
	uint numTextures = 1;
};
