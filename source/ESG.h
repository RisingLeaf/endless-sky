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

#endif

#endif
