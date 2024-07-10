#ifndef ES_GRAPHICS
#define ES_GRAPHICS

#include <cstdint>
#include <string>



namespace ESG
{
	void Init();
	bool HasAdaptiveVSyncSupport();
	void RenderSetup();
	void AddBuffer(uint32_t *target, int width, int height, int depth, const void * data);
	void ParseShader(std::string &toParse);
};



#ifdef ES_VULKAN
#include <vulkan/vulkan.h>

namespace ESG
{
	class Shader {

	};
};

#else
#include "opengl.h"

namespace ESG
{
	class Shader {
	public:
		Shader() noexcept = default;
		Shader(const char *vertex, const char *fragment);

		void Bind() const noexcept;
		int32_t Attrib(const char *name) const;
		int32_t Uniform(const char *name) const;


	private:
		uint32_t Compile(const char *str, GLenum type);


	private:
		uint32_t program;
	};
};

#endif //ES_VULKAN
#endif //ES_GRAPHICS
