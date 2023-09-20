#ifndef ES_GRAPHICS
#define ES_GRAPHICS

class ESG
{
public:
	static bool HasAdaptiveVSyncSupport();

    static void RenderSetup();
};

#ifdef ES_VULKAN
#include <vulkan/vulkan.h>

#else
#include "opengl.h"

#define ESG_BindShader glUseProgram
#define ESG_Uniform1i glUniform1i
#define ESG_BindVertexArray glBindVertexArray

#endif

#endif