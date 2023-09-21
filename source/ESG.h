#ifndef ES_GRAPHICS
#define ES_GRAPHICS

#include <cstdint>



class ESG
{
public:
	static bool HasAdaptiveVSyncSupport();

	static void RenderSetup();

	static void AddBuffer(uint32_t *target, int width, int height, int depth, const void * data);
};



#ifdef ES_VULKAN
#include <vulkan/vulkan.h>

#else
#include "opengl.h"

#define ESG_BindShader glUseProgram
#define ESG_Uniform1i glUniform1i
#define ESG_Uniform1f glUniform1f
inline void ESG_Uniform2fv(int location, const float *value) { glUniform2fv(location, 1, value); } 

#define ESG_BindVertexArray glBindVertexArray
#define ESG_BindTexture glBindTexture

#endif

#endif