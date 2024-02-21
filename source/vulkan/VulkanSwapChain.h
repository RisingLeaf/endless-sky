#pragma once

#include "VulkanDevice.h"

#include <memory>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>


class VulkanSwapChain {
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

	VulkanSwapChain(VulkanDevice &deviceRef, VkExtent2D windowExtent);
	VulkanSwapChain(VulkanDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<VulkanSwapChain> previous);
	~VulkanSwapChain();

	VulkanSwapChain(const VulkanSwapChain &) = delete;
	VulkanSwapChain operator=(const VulkanSwapChain &) = delete;

	VkFramebuffer GetFrameBuffer(int index) { return swapChainFramebuffers[index]; }
	VkRenderPass GetRenderPass() { return renderPass; }
	VkImageView GetImageView(int index) { return swapChainImageViews[index]; }
	size_t ImageCount() { return swapChainImages.size(); }
	VkFormat GetSwapChainImageFormat() { return swapChainImageFormat; }
	VkExtent2D GetSwapChainExtent() { return swapChainExtent; }
	uint32_t Width() { return swapChainExtent.width; }
	uint32_t Height() { return swapChainExtent.height; }

	float ExtentAspectRatio() { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
	VkFormat FindDepthFormat();

	VkResult AcquireNextImage(uint32_t *imageIndex);
	VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

private:
	void Init();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateDepthResources();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateSyncObjects();

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkRenderPass renderPass;

	std::vector<VkImage> depthImages;
	std::vector<VkDeviceMemory> depthImageMemorys;
	std::vector<VkImageView> depthImageViews;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	VulkanDevice &device;
	VkExtent2D windowExtent;

	VkSwapchainKHR swapChain;
	std::shared_ptr<VulkanSwapChain> oldSwapChain;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
};
