#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "VulkanDevice.h"
#include "es_vulkan.h"



struct VulkanPipelineConfigInfo {
	VulkanPipelineConfigInfo() = default;
	VulkanPipelineConfigInfo(const VulkanPipelineConfigInfo&) = delete;
	VulkanPipelineConfigInfo& operator=(const VulkanPipelineConfigInfo&) = delete;

	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

struct VulkanShaderInfo {
	std::unique_ptr<VulkanBuffer> uniformBuffer;
	uint32_t bufferCount;

	std::unique_ptr<VulkanDescriptorPool> desriptorPool;
	std::unique_ptr<VulkanDescriptorSetLayout> desriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<uint32_t> GetDynamicOffsets(uint imageIndex, uint numElements);
};

class VulkanPipeline {
public:
	VulkanPipeline(VulkanDevice &device, const std::string &vertFilePath, const std::string &fragFilePath,
		const VulkanPipelineConfigInfo &configInfo, const std::vector<AttributeSize> &attributeDescriptors);
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline &) = delete;
	VulkanPipeline &operator=(const VulkanPipeline &) = delete;
	VulkanPipeline(VulkanPipeline &&) = delete;
	VulkanPipeline &operator=(VulkanPipeline &&) = delete;

	void Bind(VkCommandBuffer commandBuffer);
	static void DefaultPipelineConfigInfo(VulkanPipelineConfigInfo &configInfo);

	static VulkanShaderInfo PrepareShaderInfo(VulkanDevice &device, ShaderInfo &inputInfo, const int maxFrames);

private:
	static std::vector<char> ReadFile(const std::string &filepath);

	void CreateGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath,
		const VulkanPipelineConfigInfo &configInfo, const std::vector<AttributeSize> &attributeDescriptors);

	void CreateShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);


	VulkanDevice &device;

	VkPipeline graphicsPipeline;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
};
