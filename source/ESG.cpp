#include "ESG.h"

#ifdef ES_VULKAN

#else

#if !defined(__APPLE__) && !defined(ES_GLES)
#ifdef _WIN32
#include <GL/wglew.h>
#else
#include <GL/glxew.h>
#endif
#endif

bool ESG::HasAdaptiveVSyncSupport()
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



void ESG::RenderSetup()
{
    glClearColor(0.f, 0.f, 0.0f, 1.f);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}



void ESG::AddBuffer(uint32_t *target, int width, int height, int depth, const void *data)
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

#endif