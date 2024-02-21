#include "VulkanPipeline.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <vector>
#include <numeric>

#include "../Logger.h"
#include "es_vulkan.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanModel.h"



std::vector<uint32_t> VulkanShaderInfo::GetDynamicOffsets(uint imageIndex, uint numElements)
{
	uint32_t alignmentSize = uniformBuffer->GetAlignmentSize();
	std::vector<uint32_t> offsets;
	uint realNumElements = std::max(numElements, uniformBuffer->GetInstanceCount() / VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
	for(uint i = 0; i < realNumElements; i++)
		offsets.emplace_back(alignmentSize * (40 * imageIndex + i));

	return offsets;
}



VulkanPipeline::VulkanPipeline(VulkanDevice &device, const std::string &vertFilePath, const std::string &fragFilePath,
	const VulkanPipelineConfigInfo &configInfo, const std::vector<AttributeSize> &attributeDescriptors)
: device(device)
{
	CreateGraphicsPipeline(vertFilePath, fragFilePath, configInfo, attributeDescriptors);
}



VulkanPipeline::~VulkanPipeline()
{
	vkDestroyShaderModule(device.Device(), vertShaderModule, nullptr);
	vkDestroyShaderModule(device.Device(), fragShaderModule, nullptr);
	vkDestroyPipeline(device.Device(), graphicsPipeline, nullptr);
}



void VulkanPipeline::Bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}



void VulkanPipeline::DefaultPipelineConfigInfo(VulkanPipelineConfigInfo &configInfo)
{
	configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  	configInfo.viewportInfo.viewportCount = 1;
  	configInfo.viewportInfo.pViewports = nullptr;
  	configInfo.viewportInfo.scissorCount = 1;
  	configInfo.viewportInfo.pScissors = nullptr;

	// Rasterizer settings, specifies how the primitives are mapped onto pixels.
	configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.rasterizationInfo.lineWidth = 1.0f;
	configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE; // Similar to glCullFaces
	configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
	configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
	configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
	configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

	// Multisampling, not needed in ES as ships are not drawn as objects but as sprites.
	configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	configInfo.colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
	configInfo.colorBlendInfo.attachmentCount = 1;
	configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
	configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
	configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
	configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
	configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

	configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.depthStencilInfo.depthTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.minDepthBounds = 0.0f;
	configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
	configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.front = {};
	configInfo.depthStencilInfo.back = {};

	configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
	configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
	configInfo.dynamicStateInfo.flags = 0;
}



VulkanShaderInfo VulkanPipeline::PrepareShaderInfo(VulkanDevice &device, ShaderInfo &inputInfo, const int maxFrames)
{
	VulkanShaderInfo shaderInfo{};
	shaderInfo.bufferCount = 40;

	uint32_t uniformSize = 0;
	for(const auto &uniformValue : inputInfo.uniformLayout)
		uniformSize += EsToVulkan::FORMAT_MAP_TYPE_SIZE.at(uniformValue);

	auto LCM = [](int a, int b) -> int {
		int max = (a > b) ? a : b;
		do
		{
			if (max % a == 0 && max % b == 0)
				return max;
			else
				++max;
		} while (true);
	};

	auto minOffsetAlignment = LCM(
		device.properties.limits.minUniformBufferOffsetAlignment,
		device.properties.limits.nonCoherentAtomSize);

	shaderInfo.uniformBuffer = std::make_unique<VulkanBuffer>(
		device,
		uniformSize,
		shaderInfo.bufferCount * maxFrames,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		device.properties.limits.minUniformBufferOffsetAlignment);
	shaderInfo.uniformBuffer->Map();


	shaderInfo.desriptorPool = VulkanDescriptorPool::Builder(device)
		.SetMaxSets(shaderInfo.bufferCount * maxFrames)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, shaderInfo.bufferCount * maxFrames)
		.Build();
	shaderInfo.desriptorSetLayout = VulkanDescriptorSetLayout::Builder(device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT)
		.Build();
	for(int i = 0; i < maxFrames; i++)
	{
		for(int j = 0; j < shaderInfo.bufferCount; j++)
		{
			shaderInfo.descriptorSets.emplace_back();
			auto bufferInfo = shaderInfo.uniformBuffer->DescriptorInfoForIndex(i * shaderInfo.bufferCount + j);
			VulkanDescriptorWriter(*shaderInfo.desriptorSetLayout, *shaderInfo.desriptorPool)
				.WriteBuffer(0, &bufferInfo)
				.Build(shaderInfo.descriptorSets.back());
		}
	}

	return shaderInfo;
}



std::vector<char> VulkanPipeline::ReadFile(const std::string &filepath)
{
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);

	if(!file.is_open())
	{
		Logger::LogError("Failed to open file: " + filepath);
		throw std::runtime_error("failed to open file: " + filepath);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}



void VulkanPipeline::CreateGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath,
	const VulkanPipelineConfigInfo &configInfo, const std::vector<AttributeSize> &attributeDescriptors)
{
	assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline, no layout specified.");
	assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create pipeline, no renderpass specified.");

	auto vertCode = ReadFile(vertFilePath);
	auto fragCode = ReadFile(fragFilePath);

	CreateShaderModule(vertCode, &vertShaderModule);
	CreateShaderModule(fragCode, &fragShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fragShaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	uint32_t sizePerVertex = 0;
	for(const auto &attributeDescriptor : attributeDescriptors)
		sizePerVertex += EsToVulkan::FORMAT_MAP_SIZE.at(attributeDescriptor);

	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(float) * sizePerVertex;
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(attributeDescriptors.size());
	uint32_t offset = 0;
	for(int i = 0; i < attributeDescriptors.size(); i++)
	{
		attributeDescriptions[i].binding = 0;
		attributeDescriptions[i].location = i;
		attributeDescriptions[i].format = EsToVulkan::FORMAT_MAP_VULKAN.at(attributeDescriptors[i]);
		attributeDescriptions[i].offset = offset;
		offset += EsToVulkan::FORMAT_MAP_SIZE.at(attributeDescriptors[i]) * sizeof(float);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
	pipelineInfo.pViewportState = &configInfo.viewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
	pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
	pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
	pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

	pipelineInfo.layout = configInfo.pipelineLayout;
	pipelineInfo.renderPass = configInfo.renderPass;
	pipelineInfo.subpass = configInfo.subpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if(vkCreateGraphicsPipelines(device.Device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create graphics pipeline");
}



void VulkanPipeline::CreateShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if(vkCreateShaderModule(device.Device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
		throw std::runtime_error("failed to create shader module");
}
