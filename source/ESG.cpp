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

#endif