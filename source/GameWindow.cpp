/* GameWindow.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "GameWindow.h"

#include "Files.h"
#include "ImageBuffer.h"
#include "Logger.h"
#include "Screen.h"

#include "ESG.h"
#include <GLFW/glfw3.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#define ES_VULKAN

using namespace std;

namespace {
	GLFWwindow *glfwMainWindow;
	GLFWmonitor *primaryMonitor;

	int width = 0;
	int height = 0;
	bool supportsAdaptiveVSync = false;

	vector<GameWindow::InputEvent> events;
	double lastX = 0.;
	double lastY = 0.;
	uint64_t currentMods = 0;

	const std::map<int, uint64_t> GLFW_KEY_TO_MOD = {
		{GLFW_KEY_LEFT_SHIFT, GameWindow::MOD_SHIFT},
		{GLFW_KEY_RIGHT_SHIFT, GameWindow::MOD_SHIFT},
		{GLFW_KEY_LEFT_ALT, GameWindow::MOD_ALT},
		{GLFW_KEY_LEFT_SHIFT, GameWindow::MOD_ALT},
		{GLFW_KEY_LEFT_CONTROL, GameWindow::MOD_CONTROL},
		{GLFW_KEY_RIGHT_CONTROL, GameWindow::MOD_CONTROL},
		{GLFW_KEY_CAPS_LOCK, GameWindow::MOD_CAPS},
		{GLFW_KEY_RIGHT_SUPER, GameWindow::MOD_GUI},
		{GLFW_KEY_LEFT_SUPER, GameWindow::MOD_GUI},
	};

	void GlfwErrorCallback(int error, const char* description)
	{
		Logger::LogError("Error: " + string(description) + "\n");
	}

	void GlfwKeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if(GLFW_KEY_TO_MOD.count(key))
		{
			if(action == GLFW_RELEASE)
				currentMods &= ~GLFW_KEY_TO_MOD.at(key);
			else
				currentMods |= GLFW_KEY_TO_MOD.at(key);
		}
		GameWindow::InputEvent event;
		event.type = action == GLFW_PRESS ? GameWindow::InputEventType::KEY_DOWN
			: GameWindow::InputEventType::KEY_UP;
		event.key = key;
		event.mods = currentMods;
		events.emplace_back(event);
	}

	static void GlfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
	{
		GameWindow::InputEvent event;
		event.type = GameWindow::InputEventType::MOUSE_MOTION;
		event.x = xpos - lastX;
		event.y = ypos - lastY;
		lastX = xpos;
		lastY = ypos;
		events.emplace_back(event);
	}

	static void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		GameWindow::InputEvent event;
		event.type = action == GLFW_PRESS ? GameWindow::InputEventType::MOUSEBUTTON_DOWN
			: GameWindow::InputEventType::MOUSEBUTTON_UP;
		event.key = button;
		glfwGetCursorPos(window, &event.x, &event.y);
		events.emplace_back(event);
	}

	static void GlfwMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		GameWindow::InputEvent event;
		event.type = GameWindow::InputEventType::MOUSEWHEEL;
		event.x = xoffset;
		event.y = yoffset;
		events.emplace_back(event);
	}

	void GlfwWindowCloseCallback(GLFWwindow* window)
	{
		GameWindow::InputEvent event;
		event.type = GameWindow::InputEventType::QUIT;
		events.emplace_back(event);
	}

	void GlfwWindowResizeCallback(GLFWwindow* window, int width, int height)
	{
		GameWindow::AdjustViewport();
	}

	const std::map<int, const char *> EXTRA_KEY_NAMES = {
		{GLFW_KEY_UP, "Up"},
		{GLFW_KEY_DOWN, "Down"},
		{GLFW_KEY_LEFT, "Left"},
		{GLFW_KEY_RIGHT, "Right"},

		{GLFW_KEY_TAB, "Tab"},
		{GLFW_KEY_LEFT_SHIFT, "L Shift"},
		{GLFW_KEY_RIGHT_SHIFT, "R Shift"},
		{GLFW_KEY_LEFT_ALT, "L Alt"},
		{GLFW_KEY_RIGHT_ALT, "R Alt"},
		{GLFW_KEY_SPACE, "Space"},
		{GLFW_KEY_ENTER, "Enter"},
		{GLFW_KEY_BACKSPACE, "Backspace"},
		{GLFW_KEY_ESCAPE, "ESC"},
		{GLFW_KEY_CAPS_LOCK, "Caps Lock"},

		{GLFW_KEY_F1, "F1"},
		{GLFW_KEY_F2, "F2"},
		{GLFW_KEY_F3, "F3"},
		{GLFW_KEY_F4, "F4"},
		{GLFW_KEY_F5, "F5"},
		{GLFW_KEY_F6, "F6"},
		{GLFW_KEY_F7, "F6"},
		{GLFW_KEY_F8, "F8"},
		{GLFW_KEY_F9, "F9"},
		{GLFW_KEY_F10, "F10"},
		{GLFW_KEY_F11, "F11"},
		{GLFW_KEY_F12, "F12"},
	};
}



bool GameWindow::Init(bool headless)
{
	// This needs to be called before any other glfw commands.
	if(!glfwInit())
	{
		Logger::LogError("Failed to init glfw!\n");
		return false;
	}
	glfwSetErrorCallback(GlfwErrorCallback);

	// Get details about the current display.
	primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *videoMode = glfwGetVideoMode(primaryMonitor);
	if(!videoMode)
	{
		ExitWithError("Unable to query monitor resolution!");
		return false;
	}
	if(videoMode->refreshRate && videoMode->refreshRate < 60)
		Logger::LogError("Warning: low monitor frame rate detected (" + to_string(videoMode->refreshRate) + ")."
			" The game will run more slowly.");

	// Make the window just slightly smaller than the monitor resolution.
	const static int minWidth = 640;
	const static int minHeight = 480;
	const int maxWidth = videoMode->width;
	const int maxHeight = videoMode->height;
	if(maxWidth < minWidth || maxHeight < minHeight)
	{
		ExitWithError("Monitor resolution is too small!");
		return false;
	}

	int windowWidth = maxWidth - 100;
	int windowHeight = maxHeight - 100;

	// Decide how big the window should be.
	if(Screen::RawWidth() && Screen::RawHeight())
	{
		// Load the previously saved window dimensions.
		windowWidth = min(windowWidth, Screen::RawWidth());
		windowHeight = min(windowHeight, Screen::RawHeight());
	}


#ifdef ES_VULKAN
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#else
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
#ifdef _WIN32
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
#ifdef ES_GLES
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
#endif
	//SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
#endif

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	if(Preferences::Has("maximized"))
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	if(Preferences::ScreenModeSetting() == "fullscreen")
		glfwMainWindow = glfwCreateWindow(windowWidth, windowHeight, "Endless Sky [GLFW]", primaryMonitor, nullptr);
	else
		glfwMainWindow = glfwCreateWindow(windowWidth, windowHeight, "Endless Sky [GLFW]", nullptr, nullptr);

	if(!glfwMainWindow)
	{
		ExitWithError("Unable to create window!");
		return false;
	}

#ifndef ES_VULKAN
	glfwMakeContextCurrent(glfwMainWindow);
#endif

	// Input event handling:
	glfwSetInputMode(glfwMainWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetKeyCallback(glfwMainWindow, GlfwKeyboardCallback);
	glfwSetCursorPosCallback(glfwMainWindow, GlfwMouseMoveCallback);
	glfwSetMouseButtonCallback(glfwMainWindow, GlfwMouseButtonCallback);
	glfwSetScrollCallback(glfwMainWindow, GlfwMouseWheelCallback);
	glfwSetWindowCloseCallback(glfwMainWindow, GlfwWindowCloseCallback);
	glfwSetWindowSizeCallback(glfwMainWindow, GlfwWindowResizeCallback);


#ifndef ES_VULKAN
	// Initialize GLEW.
#if !defined(__APPLE__) && !defined(ES_GLES)
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
#ifdef GLEW_ERROR_NO_GLX_DISPLAY
	if(err != GLEW_OK && err != GLEW_ERROR_NO_GLX_DISPLAY)
#else
	if(err != GLEW_OK)
#endif
	{
		ExitWithError("Unable to initialize GLEW!");
		return false;
	}
#endif

	// Check that the OpenGL version is high enough.
	const char *glVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	if(!glVersion || !*glVersion)
	{
		ExitWithError("Unable to query the OpenGL version!");
		return false;
	}

	const char *glslVersion = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	if(!glslVersion || !*glslVersion)
	{
		ostringstream out;
		out << "Unable to query the GLSL version. OpenGL version is " << glVersion << ".";
		ExitWithError(out.str());
		return false;
	}

	if(*glVersion < '3')
	{
		ostringstream out;
		out << "Endless Sky requires OpenGL version 3.0 or higher." << endl;
		out << "Your OpenGL version is " << glVersion << ", GLSL version " << glslVersion << "." << endl;
		out << "Please update your graphics drivers.";
		ExitWithError(out.str());
		return false;
	}
#endif
	// Render settings
	ESG::RenderSetup();

	// Check for support of various graphical features.
	supportsAdaptiveVSync = ESG::HasAdaptiveVSyncSupport();

	// Enable the user's preferred VSync state, otherwise update to an available
	// value (e.g. if an external program is forcing a particular VSync state).
	if(!SetVSync(Preferences::VSyncState()))
		Preferences::ToggleVSync();

	// Make sure the screen size and view-port are set correctly.
	AdjustViewport();

#ifndef __APPLE__
	// On OS X, setting the window icon will cause that same icon to be used
	// in the dock and the application switcher. That's not something we
	// want, because the ".icns" icon that is used automatically is prettier.
	SetIcon();
#endif

	return true;
}



// Clean up the SDL context, window, and shut down SDL.
void GameWindow::Quit()
{
	glfwDestroyWindow(glfwMainWindow);
	glfwTerminate();
}



void GameWindow::Step()
{
	glfwSwapBuffers(glfwMainWindow);
	glfwPollEvents();
}



void GameWindow::SetIcon()
{
	if(!glfwMainWindow)
		return;

	// Load the icon file.
	ImageBuffer buffer;
	if(!buffer.Read(Files::Resources() + "icon.png"))
		return;
	if(!buffer.Pixels() || !buffer.Width() || !buffer.Height())
		return;

	GLFWimage images[1];
	images[0].pixels = reinterpret_cast<unsigned char*>(buffer.Pixels());
	images[0].width = buffer.Width();
	images[0].height = buffer.Height();
	glfwSetWindowIcon(glfwMainWindow, 1, images);

}



void GameWindow::AdjustViewport()
{
	if(!glfwMainWindow)
		return;

	// Get the window's size in screen coordinates.
	int windowWidth, windowHeight;
	glfwGetWindowSize(glfwMainWindow, &windowWidth, &windowHeight);

	// Only save the window size when not in fullscreen mode.
	if(!GameWindow::IsFullscreen())
	{
		width = windowWidth;
		height = windowHeight;
	}

	// Round the window size up to a multiple of 2, even if this
	// means one pixel of the display will be clipped.
	int roundWidth = (windowWidth + 1) & ~1;
	int roundHeight = (windowHeight + 1) & ~1;
	Screen::SetRaw(roundWidth, roundHeight);

	// Find out the drawable dimensions. If this is a high- DPI display, this
	// may be larger than the window.
	int drawWidth, drawHeight;
	glfwGetFramebufferSize(glfwMainWindow, &drawWidth, &drawHeight);
	Screen::SetHighDPI(drawWidth > windowWidth || drawHeight > windowHeight);

	// Set the viewport to go off the edge of the window, if necessary, to get
	// everything pixel-aligned.
	drawWidth = (drawWidth * roundWidth) / windowWidth;
	drawHeight = (drawHeight * roundHeight) / windowHeight;
	glViewport(0, 0, drawWidth, drawHeight);
}



// Attempts to set the requested SDL Window VSync to the given state. Returns false
// if the operation could not be completed successfully.
bool GameWindow::SetVSync(Preferences::VSync state)
{
#ifndef ES_VULKAN
	int interval = 1;
	switch(state)
	{
		case Preferences::VSync::adaptive:
			interval = -1;
			break;
		case Preferences::VSync::off:
			interval = 0;
			break;
		case Preferences::VSync::on:
			interval = 1;
			break;
		default:
			return false;
	}
	// Do not attempt to enable adaptive VSync when unsupported,
	// as this can crash older video drivers.
	if(interval == -1 && !supportsAdaptiveVSync)
		return false;

	glfwSwapInterval(interval);
#endif
	return true;
}



// Last window width, in windowed mode.
int GameWindow::Width()
{
	return width;
}



// Last window height, in windowed mode.
int GameWindow::Height()
{
	return height;
}



bool GameWindow::IsMaximized()
{
	return glfwGetWindowAttrib(glfwMainWindow, GLFW_MAXIMIZED) == GLFW_TRUE;
}



bool GameWindow::IsFullscreen()
{
	return glfwGetWindowMonitor(glfwMainWindow);
}



void GameWindow::ToggleFullscreen()
{
	// This will generate a window size change event,
	// no need to adjust the viewport here.
	if(IsFullscreen())
		glfwSetWindowMonitor(glfwMainWindow, nullptr, 0, 0, width, height, 0);
	else
	{
		const GLFWvidmode *videoMode = glfwGetVideoMode(primaryMonitor);
		glfwSetWindowMonitor(glfwMainWindow, primaryMonitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
	}
}



void GameWindow::ShowCursor(bool show)
{
	if(show)
		glfwSetInputMode(glfwMainWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	else
		glfwSetInputMode(glfwMainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}



void GameWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
{
	if(glfwCreateWindowSurface(instance, glfwMainWindow, nullptr, surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface");
}



bool GameWindow::KeyDown(int key)
{
	return glfwGetKey(glfwMainWindow, key) != GLFW_RELEASE;
}



bool GameWindow::ModActive(uint64_t mod)
{
	return currentMods & mod;
}



const char *GameWindow::KeyName(int key)
{
	const char *name = glfwGetKeyName(key, 0);
	if(name)
		return name;
	if(EXTRA_KEY_NAMES.count(key))
		return EXTRA_KEY_NAMES.at(key);
	return "(?)";
}



bool GameWindow::MouseState(double *x, double *y, int button)
{
	glfwGetCursorPos(glfwMainWindow, x, y);
	return glfwGetMouseButton(glfwMainWindow, button);
}



void GameWindow::SetMousePos(double x, double y)
{
	glfwSetCursorPos(glfwMainWindow, x, y);
}



std::vector<GameWindow::InputEvent> &GameWindow::FetchEvents()
{
	static std::vector<GameWindow::InputEvent> copy;
    copy.clear();
    events.swap(copy);

    return copy;
}



void GameWindow::ExitWithError(const string &message, bool doPopUp)
{
	// Print the error message in the terminal and the error file.
	Logger::LogError(message);

	GameWindow::Quit();
}
