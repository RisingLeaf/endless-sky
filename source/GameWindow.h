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

#include <set>
#include <string>



// This class is a collection of global functions for handling SDL_Windows.
class GameWindow {
public:
	enum class Mods {
		SHIFT,
		CAPS,
		ALT,
		CTRL,
		GUI,
		CTRL_GUI,
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

	static const uint8_t *GetKeyboard();
	static uint8_t GetScancode(int32_t key);
	static bool GetMod(Mods mods);
	static const char *GetKeyname(int32_t key);
	static void GetMousePos(int &x, int &y);
	static void SetMousePos(double x, double y);
	static bool GetMouseButton(uint16_t button);
	static void SetCursorVisibillity(bool visible);

	// Print the error message in the terminal, error file, and message box.
	// Checks for video system errors and records those as well.
	static void ExitWithError(const std::string& message, bool doPopUp = true);
};



#endif
