#include "ESG.h"

namespace {
	std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
	{
		size_t start_pos = 0;
		while((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
		return str;
	}
}


#ifdef ES_VULKAN
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanDescriptors.h"
#include "vulkan/VulkanSwapChain.h"
#include "vulkan/VulkanPipeline.h"
#include "vulkan/VulkanTexture.h"

#include <memory>


namespace ESG {
	struct VulkanPipelineDescription {
		ShaderInfo shaderInfo;
		std::unique_ptr<VulkanPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		VulkanShaderInfo pipelineShaderInfo;
	};

	VulkanDevice &GetDevice()
	{
		static VulkanDevice device;
		return device;
	}
	std::unique_ptr<VulkanSwapChain> swapChain;
	std::vector<VulkanPipelineDescription> pipelineDescriptions;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<std::unique_ptr<VulkanTexture>> textures[2];
	std::unique_ptr<VulkanDescriptorPool> desriptorPool;
	std::unique_ptr<VulkanDescriptorSetLayout> desriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;


	void CreateTextureDescriptors()
	{
		desriptorPool = VulkanDescriptorPool::Builder(GetDevice())
			.SetMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
			.Build();
		desriptorSetLayout = VulkanDescriptorSetLayout::Builder(GetDevice())
			.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build();
		for(int j = 0; j < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; j++)
			descriptorSets.emplace_back();
	}



	void Init()
	{
		CreateTextureDescriptors();

		pipelineDescriptions.emplace_back();
		pipelineDescriptions[0].shaderInfo.attributeLayout = {
			AttributeSize::VECTOR_TWO,
			AttributeSize::VECTOR_TWO,
		};
		pipelineDescriptions[0].shaderInfo.uniformLayout = {
			AttributeSize::VECTOR_TWO,
			AttributeSize::VECTOR_THREE,
		};
		pipelineDescriptions[0].shaderInfo.vertexShaderFilename = "../../resources/shaders/shader.vert.spv";
		pipelineDescriptions[0].shaderInfo.fragmentShaderFilename = "../../resources/shaders/shader.frag.spv";
		pipelineDescriptions[0].pipelineShaderInfo = VulkanPipeline::PrepareShaderInfo(GetDevice(), pipelineDescriptions[0].shaderInfo,
			VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
	}



	void RenderSetup() {}



	void AddBuffer(uint32_t *target, int width, int height, int depth, const void *data)
	{
		textures[0].emplace_back(std::make_unique<VulkanTexture>(GetDevice(), data, width, height, depth, 4));
		for(int j = 0; j < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; j++)
		{
			auto writer = VulkanDescriptorWriter(*desriptorSetLayout, *desriptorPool);
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = textures[0].back()->GetImageLayout();
			imageInfo.imageView = textures[0].back()->GetImageView();
			imageInfo.sampler = textures[0].back()->GetSampler();
			writer.WriteImage(1, &imageInfo);
			writer.Build(descriptorSets.back());
		}
	}

    void ParseShader(std::string &toParse)
    {
		toParse = ReplaceAll(toParse, "//?vulkan ", "");
    }

    bool HasAdaptiveVSyncSupport()	{ return false; }
}


#else

#if !defined(__APPLE__) && !defined(ES_GLES)
#ifdef _WIN32
#include <GL/wglew.h>
#else
#include <GL/glxew.h>
#endif
#endif

namespace ESG {
	void Init() {}

	bool HasAdaptiveVSyncSupport()
	{
	#ifdef __APPLE__
		// macOS doesn't support Adaptive VSync for OpenGL.
		return false;
	#elif defined(ES_GLES)
		return HasOpenGLExtension("_swap_control_tear");
	#elif defined(_WIN32)
		return WGL_EXT_swap_control_tear || HasOpenGLExtension("_swap_control_tear");
	#else
		return GLX_EXT_swap_control_tear;
	#endif
	}



	void RenderSetup()
	{
		glClearColor(0.f, 0.f, 0.0f, 1.f);
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}



	void AddBuffer(uint32_t *target, int width, int height, int depth, const void *data)
	{
		// Upload the images as a single array texture.
		glGenTextures(1, target);
		glBindTexture(GL_TEXTURE_2D_ARRAY, *target);

		// Use linear interpolation and no wrapping.
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Upload the image data.
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, // target, mipmap level, internal format,
			width, height, depth, // width, height, depth,
			0, GL_RGBA, GL_UNSIGNED_BYTE, data); // border, input format, data type, data.

		// Unbind the texture.
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}


    void ParseShader(std::string &toParse)
    {
		toParse = ReplaceAll(toParse, "//?opengl ", "");
    }
}

#endif
