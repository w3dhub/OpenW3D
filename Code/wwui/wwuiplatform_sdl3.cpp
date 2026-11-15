/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D Developers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "wwuiplatform.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_stdinc.h>

#include <cstring>


////////////////////////////////////////////////////////////////
//
//	Keycode_To_VK
//
//		Modifier keys are reported as their left/right specific
// virtual key when 'sided' is true, otherwise as the generic
// VK_SHIFT/VK_CONTROL/VK_MENU (as WM_KEYDOWN does).
//
////////////////////////////////////////////////////////////////
static int
Keycode_To_VK (SDL_Keycode key, bool sided)
{
	//
	//	Letters and digits map to their uppercase ASCII values
	//
	if (key >= SDLK_A && key <= SDLK_Z) {
		return int(key - SDLK_A) + 'A';
	}

	if (key >= SDLK_0 && key <= SDLK_9) {
		return int(key - SDLK_0) + '0';
	}

	if (key >= SDLK_F1 && key <= SDLK_F12) {
		return int(key - SDLK_F1) + VK_F1;
	}

	if (key >= SDLK_F13 && key <= SDLK_F24) {
		return int(key - SDLK_F13) + VK_F13;
	}

	if (key >= SDLK_KP_1 && key <= SDLK_KP_9) {
		return int(key - SDLK_KP_1) + VK_NUMPAD1;
	}

	switch (key) {
		case SDLK_BACKSPACE:		return VK_BACK;
		case SDLK_TAB:				return VK_TAB;
		case SDLK_CLEAR:			return VK_CLEAR;
		case SDLK_RETURN:			return VK_RETURN;
		case SDLK_KP_ENTER:		return VK_RETURN;
		case SDLK_PAUSE:			return VK_PAUSE;
		case SDLK_CAPSLOCK:		return VK_CAPITAL;
		case SDLK_ESCAPE:			return VK_ESCAPE;
		case SDLK_SPACE:			return VK_SPACE;
		case SDLK_PAGEUP:			return VK_PRIOR;
		case SDLK_PAGEDOWN:		return VK_NEXT;
		case SDLK_END:				return VK_END;
		case SDLK_HOME:			return VK_HOME;
		case SDLK_LEFT:			return VK_LEFT;
		case SDLK_UP:				return VK_UP;
		case SDLK_RIGHT:			return VK_RIGHT;
		case SDLK_DOWN:			return VK_DOWN;
		case SDLK_SELECT:			return VK_SELECT;
		case SDLK_EXECUTE:		return VK_EXECUTE;
		case SDLK_PRINTSCREEN:	return VK_SNAPSHOT;
		case SDLK_INSERT:			return VK_INSERT;
		case SDLK_DELETE:			return VK_DELETE;
		case SDLK_HELP:			return VK_HELP;
		case SDLK_LGUI:			return VK_LWIN;
		case SDLK_RGUI:			return VK_RWIN;
		case SDLK_APPLICATION:	return VK_APPS;
		case SDLK_SLEEP:			return VK_SLEEP;
		case SDLK_KP_0:			return VK_NUMPAD0;
		case SDLK_KP_MULTIPLY:	return VK_MULTIPLY;
		case SDLK_KP_PLUS:		return VK_ADD;
		case SDLK_KP_MINUS:		return VK_SUBTRACT;
		case SDLK_KP_PERIOD:		return VK_DECIMAL;
		case SDLK_KP_DIVIDE:		return VK_DIVIDE;
		case SDLK_NUMLOCKCLEAR:	return VK_NUMLOCK;
		case SDLK_SCROLLLOCK:	return VK_SCROLL;
		case SDLK_LSHIFT:			return sided ? VK_LSHIFT : VK_SHIFT;
		case SDLK_RSHIFT:			return sided ? VK_RSHIFT : VK_SHIFT;
		case SDLK_LCTRL:			return sided ? VK_LCONTROL : VK_CONTROL;
		case SDLK_RCTRL:			return sided ? VK_RCONTROL : VK_CONTROL;
		case SDLK_LALT:			return sided ? VK_LMENU : VK_MENU;
		case SDLK_RALT:			return sided ? VK_RMENU : VK_MENU;
		case SDLK_SEMICOLON:		return VK_OEM_1;
		case SDLK_EQUALS:			return VK_OEM_PLUS;
		case SDLK_PLUS:			return VK_OEM_PLUS;
		case SDLK_COMMA:			return VK_OEM_COMMA;
		case SDLK_MINUS:			return VK_OEM_MINUS;
		case SDLK_PERIOD:			return VK_OEM_PERIOD;
		case SDLK_SLASH:			return VK_OEM_2;
		case SDLK_GRAVE:			return VK_OEM_3;
		case SDLK_LEFTBRACKET:	return VK_OEM_4;
		case SDLK_BACKSLASH:		return VK_OEM_5;
		case SDLK_RIGHTBRACKET:	return VK_OEM_6;
		case SDLK_APOSTROPHE:	return VK_OEM_7;
		default:						break;
	}

	return 0;
}


////////////////////////////////////////////////////////////////
//
//	SDL_Keycode_To_VK
//
////////////////////////////////////////////////////////////////
int
WWUIPlatform::SDL_Keycode_To_VK (uint32_t sdl_keycode)
{
	return Keycode_To_VK (SDL_Keycode (sdl_keycode), false);
}


////////////////////////////////////////////////////////////////
//
//	Get_Keyboard_State
//
////////////////////////////////////////////////////////////////
void
WWUIPlatform::Get_Keyboard_State (uint8_t state[256])
{
	::memset (state, 0, 256);

	//
	//	Keys
	//
	int num_keys = 0;
	const bool *keys = SDL_GetKeyboardState (&num_keys);
	SDL_Keymod mod_state = SDL_GetModState ();

	for (int scancode = 0; scancode < num_keys; scancode ++) {
		if (keys[scancode]) {
			SDL_Keycode key = SDL_GetKeyFromScancode (SDL_Scancode (scancode), SDL_KMOD_NONE, false);

			//
			//	Win32 reports both the generic and the left/right specific
			// virtual keys as down for modifiers.
			//
			int vk = Keycode_To_VK (key, false);
			if (vk != 0) {
				state[vk] |= 0x80;
			}

			int sided_vk = Keycode_To_VK (key, true);
			if (sided_vk != 0) {
				state[sided_vk] |= 0x80;
			}
		}
	}

	//
	//	Toggle keys
	//
	if (mod_state & SDL_KMOD_CAPS) {
		state[VK_CAPITAL] |= 0x01;
	}

	if (mod_state & SDL_KMOD_NUM) {
		state[VK_NUMLOCK] |= 0x01;
	}

	if (mod_state & SDL_KMOD_SCROLL) {
		state[VK_SCROLL] |= 0x01;
	}

	//
	//	Mouse buttons
	//
	SDL_MouseButtonFlags buttons = SDL_GetMouseState (nullptr, nullptr);

	if (buttons & SDL_BUTTON_LMASK) {
		state[VK_LBUTTON] |= 0x80;
	}

	if (buttons & SDL_BUTTON_RMASK) {
		state[VK_RBUTTON] |= 0x80;
	}

	if (buttons & SDL_BUTTON_MMASK) {
		state[VK_MBUTTON] |= 0x80;
	}

	if (buttons & SDL_BUTTON_X1MASK) {
		state[VK_XBUTTON1] |= 0x80;
	}

	if (buttons & SDL_BUTTON_X2MASK) {
		state[VK_XBUTTON2] |= 0x80;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Key_Down
//
////////////////////////////////////////////////////////////////
bool
WWUIPlatform::Is_Key_Down (int vk)
{
	if (vk < 0 || vk > 255) {
		return false;
	}

	uint8_t state[256];
	Get_Keyboard_State (state);
	return (state[vk] & 0x80) != 0;
}


////////////////////////////////////////////////////////////////
//
//	Get_Wheel_Scroll_Lines
//
////////////////////////////////////////////////////////////////
int
WWUIPlatform::Get_Wheel_Scroll_Lines (void)
{
	//
	//	SDL has no equivalent of SPI_GETWHEELSCROLLLINES, use the
	// Windows default.
	//
	return 3;
}


////////////////////////////////////////////////////////////////
//
//	Append_Codepoint
//
////////////////////////////////////////////////////////////////
static void
Append_Codepoint (std::basic_string<unichar_t> &string, Uint32 codepoint)
{
	if (sizeof (unichar_t) == 2 && codepoint > 0xFFFF) {

		//
		//	Encode as a UTF-16 surrogate pair
		//
		codepoint -= 0x10000;
		string.push_back (unichar_t (0xD800 + (codepoint >> 10)));
		string.push_back (unichar_t (0xDC00 + (codepoint & 0x3FF)));

	} else {
		string.push_back (unichar_t (codepoint));
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	UTF8_To_Unichar
//
////////////////////////////////////////////////////////////////
std::basic_string<unichar_t>
WWUIPlatform::UTF8_To_Unichar (const char *utf8)
{
	std::basic_string<unichar_t> retval;

	if (utf8 != nullptr) {
		Uint32 codepoint = 0;
		while ((codepoint = SDL_StepUTF8 (&utf8, nullptr)) != 0) {
			Append_Codepoint (retval, codepoint);
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	UTF8_Index_To_Unichar_Index
//
////////////////////////////////////////////////////////////////
int
WWUIPlatform::UTF8_Index_To_Unichar_Index (const char *utf8, int codepoint_index)
{
	if (utf8 == nullptr || codepoint_index <= 0) {
		return 0;
	}

	std::basic_string<unichar_t> converted;

	Uint32 codepoint = 0;
	for (int index = 0; index < codepoint_index; index ++) {
		if ((codepoint = SDL_StepUTF8 (&utf8, nullptr)) == 0) {
			break;
		}

		Append_Codepoint (converted, codepoint);
	}

	return int(converted.length ());
}
