/* GameWindow.h
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

#ifndef GAMEWINDOW_H_
#define GAMEWINDOW_H_

#include "Preferences.h"

#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan_core.h>

// This class is a collection of global functions for handling SDL_Windows.
class GameWindow {
public:
	enum class InputEventType : uint {
		NONE = 0,

		MOUSE_MOTION,
		MOUSEBUTTON_DOWN,
		MOUSEBUTTON_UP,
		MOUSEWHEEL,

		KEY_DOWN,
		KEY_UP,

		QUIT,
	};

	// Keyboard Modifiers
	static const uint64_t MOD_SHIFT   = 1;
	static const uint64_t MOD_CONTROL = 1 << 1;
	static const uint64_t MOD_ALT     = 1 << 2;
	static const uint64_t MOD_CAPS    = 1 << 3;
	static const uint64_t MOD_GUI     = 1 << 4;

	struct InputEvent {
		InputEventType type = InputEventType::NONE;

		int key = 0;
		uint64_t mods;

		double x;
		double y;
	};

public:
	static std::string SDLVersions();
	static bool Init(bool headless);
	static void Quit();

	// Paint the next frame in the main window.
	static void Step();

	// Ensure the proper icon is set on the main window.
	static void SetIcon();

	// Handle resize events of the main window.
	static void AdjustViewport();

	// Attempt to set the game's VSync setting.
	static bool SetVSync(Preferences::VSync state);

	// Last known windowed-mode width & height.
	static int Width();
	static int Height();

	static bool IsMaximized();
	static bool IsFullscreen();
	static void ToggleFullscreen();
	static void ShowCursor(bool show);

	static void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

	static bool KeyDown(int key);
	static bool ModActive(uint64_t mod);
	static const char *KeyName(int key);

	static bool MouseState(double *x, double *y, int button = 0);
	static void SetMousePos(double x, double y);

	static std::vector<InputEvent> &FetchEvents();

	// Print the error message in the terminal, error file, and message box.
	// Checks for video system errors and records those as well.
	static void ExitWithError(const std::string& message, bool doPopUp = true);
};



#endif
