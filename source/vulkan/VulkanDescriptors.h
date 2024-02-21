#pragma once

#include "VulkanDevice.h"

#include <memory>
#include <unordered_map>



class VulkanDescriptorSetLayout {
public:
	class Builder {
	public:
		Builder(VulkanDevice &device) : device{device} {}

		Builder &AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
		std::unique_ptr<VulkanDescriptorSetLayout> Build() const;

	private:
		VulkanDevice &device;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
	};

	VulkanDescriptorSetLayout(VulkanDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
	~VulkanDescriptorSetLayout();

	VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &) = delete;
	VulkanDescriptorSetLayout &operator=(const VulkanDescriptorSetLayout &) = delete;

	VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }

private:
	VulkanDevice &device;
	VkDescriptorSetLayout descriptorSetLayout;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

	friend class VulkanDescriptorWriter;
};



class VulkanDescriptorPool {
public:
	class Builder {
	public:
		Builder(VulkanDevice &device) : device{device} {}

		Builder &AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
		Builder &SetPoolFlags(VkDescriptorPoolCreateFlags flags);
		Builder &SetMaxSets(uint32_t count);
		std::unique_ptr<VulkanDescriptorPool> Build() const;

	private:
		VulkanDevice &device;
		std::vector<VkDescriptorPoolSize> poolSizes{};
		uint32_t maxSets = 1000;
		VkDescriptorPoolCreateFlags poolFlags = 0;
	};

	VulkanDescriptorPool(VulkanDevice &device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes);
	~VulkanDescriptorPool();

	VulkanDescriptorPool(const VulkanDescriptorPool &) = delete;
	VulkanDescriptorPool &operator=(const VulkanDescriptorPool &) = delete;

	bool AllocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
	void FreeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
	void ResetPool();

private:
	VulkanDevice &device;
	VkDescriptorPool descriptorPool;

	friend class VulkanDescriptorWriter;
};



class VulkanDescriptorWriter {
public:
	VulkanDescriptorWriter(VulkanDescriptorSetLayout &setLayout, VulkanDescriptorPool &pool);

	VulkanDescriptorWriter &WriteBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
	VulkanDescriptorWriter &WriteImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

	bool Build(VkDescriptorSet &set);
	void Overwrite(VkDescriptorSet &set);

private:
	VulkanDescriptorSetLayout &setLayout;
	VulkanDescriptorPool &pool;
	std::vector<VkWriteDescriptorSet> writes;
};
