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
#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

#include "unichar.h"

#ifdef _WIN32
#include "win.h"
#else

// WWUI uses the Win32 dialog style bits, control notifications and virtual key
// codes as its own constants. Dialog resources are generated from .rc files,
// so these values must match winuser.h / commctrl.h exactly.

// Window styles
#define WS_VISIBLE              0x10000000L
#define WS_DISABLED             0x08000000L
#define WS_BORDER               0x00800000L
#define WS_GROUP                0x00020000L
#define WS_TABSTOP              0x00010000L

// Edit control styles
#define ES_LEFT                 0x0000L
#define ES_CENTER               0x0001L
#define ES_RIGHT                0x0002L
#define ES_MULTILINE            0x0004L
#define ES_UPPERCASE            0x0008L
#define ES_LOWERCASE            0x0010L
#define ES_PASSWORD             0x0020L
#define ES_AUTOVSCROLL          0x0040L
#define ES_AUTOHSCROLL          0x0080L
#define ES_NOHIDESEL            0x0100L
#define ES_OEMCONVERT           0x0400L
#define ES_READONLY             0x0800L
#define ES_WANTRETURN           0x1000L
#define ES_NUMBER               0x2000L

// Button styles
#define BS_PUSHBUTTON           0x00000000L
#define BS_DEFPUSHBUTTON        0x00000001L
#define BS_CHECKBOX             0x00000002L
#define BS_AUTOCHECKBOX         0x00000003L
#define BS_RADIOBUTTON          0x00000004L
#define BS_3STATE               0x00000005L
#define BS_AUTO3STATE           0x00000006L
#define BS_GROUPBOX             0x00000007L
#define BS_AUTORADIOBUTTON      0x00000009L
#define BS_OWNERDRAW            0x0000000BL
#define BS_TYPEMASK             0x0000000FL
#define BS_LEFTTEXT             0x00000020L
#define BS_TEXT                 0x00000000L
#define BS_ICON                 0x00000040L
#define BS_BITMAP               0x00000080L
#define BS_LEFT                 0x00000100L
#define BS_RIGHT                0x00000200L
#define BS_CENTER               0x00000300L
#define BS_TOP                  0x00000400L
#define BS_BOTTOM               0x00000800L
#define BS_VCENTER              0x00000C00L
#define BS_PUSHLIKE             0x00001000L
#define BS_MULTILINE            0x00002000L
#define BS_NOTIFY               0x00004000L
#define BS_FLAT                 0x00008000L

// Static control styles
#define SS_LEFT                 0x00000000L
#define SS_CENTER               0x00000001L
#define SS_RIGHT                0x00000002L
#define SS_ICON                 0x00000003L
#define SS_BLACKRECT            0x00000004L
#define SS_GRAYRECT             0x00000005L
#define SS_WHITERECT            0x00000006L
#define SS_BLACKFRAME           0x00000007L
#define SS_GRAYFRAME            0x00000008L
#define SS_WHITEFRAME           0x00000009L
#define SS_USERITEM             0x0000000AL
#define SS_SIMPLE               0x0000000BL
#define SS_LEFTNOWORDWRAP       0x0000000CL
#define SS_OWNERDRAW            0x0000000DL
#define SS_BITMAP               0x0000000EL
#define SS_ENHMETAFILE          0x0000000FL
#define SS_ETCHEDHORZ           0x00000010L
#define SS_ETCHEDVERT           0x00000011L
#define SS_ETCHEDFRAME          0x00000012L
#define SS_TYPEMASK             0x0000001FL
#define SS_NOPREFIX             0x00000080L
#define SS_NOTIFY               0x00000100L
#define SS_CENTERIMAGE          0x00000200L
#define SS_RIGHTJUST            0x00000400L

// Combo box styles
#define CBS_SIMPLE              0x0001L
#define CBS_DROPDOWN            0x0002L
#define CBS_DROPDOWNLIST        0x0003L
#define CBS_OWNERDRAWFIXED      0x0010L
#define CBS_OWNERDRAWVARIABLE   0x0020L
#define CBS_AUTOHSCROLL         0x0040L
#define CBS_OEMCONVERT          0x0080L
#define CBS_SORT                0x0100L

// List view styles
#define LVS_NOCOLUMNHEADER      0x4000

// Notifications and dialog IDs
#define BN_CLICKED              0
#define IDOK                    1
#define IDCANCEL                2

#define WHEEL_DELTA             120

// Virtual key codes
#define VK_LBUTTON              0x01
#define VK_RBUTTON              0x02
#define VK_CANCEL               0x03
#define VK_MBUTTON              0x04
#define VK_XBUTTON1             0x05
#define VK_XBUTTON2             0x06
#define VK_BACK                 0x08
#define VK_TAB                  0x09
#define VK_CLEAR                0x0C
#define VK_RETURN               0x0D
#define VK_SHIFT                0x10
#define VK_CONTROL              0x11
#define VK_MENU                 0x12
#define VK_PAUSE                0x13
#define VK_CAPITAL              0x14
#define VK_ESCAPE               0x1B
#define VK_SPACE                0x20
#define VK_PRIOR                0x21
#define VK_NEXT                 0x22
#define VK_END                  0x23
#define VK_HOME                 0x24
#define VK_LEFT                 0x25
#define VK_UP                   0x26
#define VK_RIGHT                0x27
#define VK_DOWN                 0x28
#define VK_SELECT               0x29
#define VK_PRINT                0x2A
#define VK_EXECUTE              0x2B
#define VK_SNAPSHOT             0x2C
#define VK_INSERT               0x2D
#define VK_DELETE               0x2E
#define VK_HELP                 0x2F
// VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
// VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
#define VK_LWIN                 0x5B
#define VK_RWIN                 0x5C
#define VK_APPS                 0x5D
#define VK_SLEEP                0x5F
#define VK_NUMPAD0              0x60
#define VK_NUMPAD1              0x61
#define VK_NUMPAD2              0x62
#define VK_NUMPAD3              0x63
#define VK_NUMPAD4              0x64
#define VK_NUMPAD5              0x65
#define VK_NUMPAD6              0x66
#define VK_NUMPAD7              0x67
#define VK_NUMPAD8              0x68
#define VK_NUMPAD9              0x69
#define VK_MULTIPLY             0x6A
#define VK_ADD                  0x6B
#define VK_SEPARATOR            0x6C
#define VK_SUBTRACT             0x6D
#define VK_DECIMAL              0x6E
#define VK_DIVIDE               0x6F
#define VK_F1                   0x70
#define VK_F2                   0x71
#define VK_F3                   0x72
#define VK_F4                   0x73
#define VK_F5                   0x74
#define VK_F6                   0x75
#define VK_F7                   0x76
#define VK_F8                   0x77
#define VK_F9                   0x78
#define VK_F10                  0x79
#define VK_F11                  0x7A
#define VK_F12                  0x7B
#define VK_F13                  0x7C
#define VK_F14                  0x7D
#define VK_F15                  0x7E
#define VK_F16                  0x7F
#define VK_F17                  0x80
#define VK_F18                  0x81
#define VK_F19                  0x82
#define VK_F20                  0x83
#define VK_F21                  0x84
#define VK_F22                  0x85
#define VK_F23                  0x86
#define VK_F24                  0x87
#define VK_NUMLOCK              0x90
#define VK_SCROLL               0x91
#define VK_LSHIFT               0xA0
#define VK_RSHIFT               0xA1
#define VK_LCONTROL             0xA2
#define VK_RCONTROL             0xA3
#define VK_LMENU                0xA4
#define VK_RMENU                0xA5
#define VK_OEM_1                0xBA	// ';:'
#define VK_OEM_PLUS             0xBB	// '+'
#define VK_OEM_COMMA            0xBC	// ','
#define VK_OEM_MINUS            0xBD	// '-'
#define VK_OEM_PERIOD           0xBE	// '.'
#define VK_OEM_2                0xBF	// '/?'
#define VK_OEM_3                0xC0	// '`~'
#define VK_OEM_4                0xDB	// '[{'
#define VK_OEM_5                0xDC	// '\|'
#define VK_OEM_6                0xDD	// ']}'
#define VK_OEM_7                0xDE	// ''"'
#define VK_OEM_102              0xE2	// '<>' or '\|' on RT 102-key keyboards

#endif // _WIN32

// Native window handle passed to WWUI by the application.
#ifdef _WIN32
typedef HWND WWUIWindowHandle;
#else
typedef void *WWUIWindowHandle;
#endif

////////////////////////////////////////////////////////////////
//
//	WWUIPlatform
//
//		Keyboard / system queries that WWUI needs from the platform.
// OPENW3D_WIN32 builds use the Win32 API, OPENW3D_SDL3 builds use SDL.
//
////////////////////////////////////////////////////////////////
namespace WWUIPlatform
{
	// Fill a 256 entry array indexed by virtual key code. The high bit (0x80)
	// is set if the key is down, the low bit (0x01) if the key is toggled.
	void	Get_Keyboard_State (uint8_t state[256]);

	// Is the virtual key currently held down?
	bool	Is_Key_Down (int vk);

	// Number of lines to scroll for one notch of the mouse wheel.
	int	Get_Wheel_Scroll_Lines (void);

#ifdef OPENW3D_SDL3
	// Translate an SDL keycode into a virtual key code (0 if there is none).
	int	SDL_Keycode_To_VK (uint32_t sdl_keycode);

	// Convert a UTF-8 string to a unichar_t string.
	std::basic_string<unichar_t>	UTF8_To_Unichar (const char *utf8);

	// Translate a code point index into a UTF-8 string into the matching
	// index in the converted unichar_t string.
	int	UTF8_Index_To_Unichar_Index (const char *utf8, int codepoint_index);
#endif
}
