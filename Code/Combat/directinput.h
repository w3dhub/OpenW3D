/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
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

/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/directinput.h                         $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/15/02 5:31p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	DIRECTINPUT_H
#define	DIRECTINPUT_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#include "vector3.h"
#include "vector2.h"

typedef enum {
	DIK_SHIFT = 0xF0,
	DIK_CONTROL,
	DIK_ALT,
	DIK_WIN,
} DupeKeys;

// DIK_* constants — available on all platforms for keyboard mapping tables.
// On Win32 these match dinput.h; on Linux they allow the same key table to compile.
#ifdef _WIN32
#include <dinput.h>
#else
#define DIK_F1 0x3B
#define DIK_F2 0x3C
#define DIK_F3 0x3D
#define DIK_F4 0x3E
#define DIK_F5 0x3F
#define DIK_F6 0x40
#define DIK_F7 0x41
#define DIK_F8 0x42
#define DIK_F9 0x43
#define DIK_F10 0x44
#define DIK_F11 0x57
#define DIK_F12 0x58
#define DIK_0 0x0B
#define DIK_1 0x02
#define DIK_2 0x03
#define DIK_3 0x04
#define DIK_4 0x05
#define DIK_5 0x06
#define DIK_6 0x07
#define DIK_7 0x08
#define DIK_8 0x09
#define DIK_9 0x0A
#define DIK_A 0x1E
#define DIK_B 0x30
#define DIK_C 0x2E
#define DIK_D 0x20
#define DIK_E 0x12
#define DIK_F 0x21
#define DIK_G 0x22
#define DIK_H 0x23
#define DIK_I 0x17
#define DIK_J 0x24
#define DIK_K 0x25
#define DIK_L 0x26
#define DIK_M 0x32
#define DIK_N 0x31
#define DIK_O 0x18
#define DIK_P 0x19
#define DIK_Q 0x10
#define DIK_R 0x13
#define DIK_S 0x1F
#define DIK_T 0x14
#define DIK_U 0x16
#define DIK_V 0x2F
#define DIK_W 0x11
#define DIK_X 0x2D
#define DIK_Y 0x2C
#define DIK_Z 0x15
#define DIK_MINUS 0x0C
#define DIK_EQUALS 0x0D
#define DIK_LBRACKET 0x1A
#define DIK_RBRACKET 0x1B
#define DIK_SEMICOLON 0x27
#define DIK_APOSTROPHE 0x28
#define DIK_GRAVE 0x29
#define DIK_BACKSLASH 0x2B
#define DIK_COMMA 0x33
#define DIK_PERIOD 0x34
#define DIK_SLASH 0x35
#define DIK_BACK 0x0E
#define DIK_TAB 0x0F
#define DIK_RETURN 0x1C
#define DIK_SPACE 0x39
#define DIK_LSHIFT 0x2A
#define DIK_RSHIFT 0x36
#define DIK_LCONTROL 0x1D
#define DIK_RCONTROL 0x9D
#define DIK_ESCAPE 0x01
#define DIK_UP 0xC8
#define DIK_DOWN 0xD0
#define DIK_LEFT 0xCB
#define DIK_RIGHT 0xCD
#define DIK_NUMPAD0 0x52
#define DIK_NUMPAD1 0x4F
#define DIK_NUMPAD2 0x50
#define DIK_NUMPAD3 0x51
#define DIK_NUMPAD4 0x4B
#define DIK_NUMPAD5 0x4C
#define DIK_NUMPAD6 0x4D
#define DIK_NUMPAD7 0x47
#define DIK_NUMPAD8 0x48
#define DIK_NUMPAD9 0x49
#define DIK_NUMLOCK 0x45
#define DIK_NUMPADENTER 0x9C
#define DIK_ADD 0x4E
#define DIK_SUBTRACT 0x4A
#define DIK_DECIMAL 0x53
#define DIK_DIVIDE 0xB5
#define DIK_MULTIPLY 0x37
#define DIK_HOME 0xC7
#define DIK_END 0xCF
#define DIK_PRIOR 0xC9
#define DIK_NEXT 0xD1
#define DIK_INSERT 0xD2
#define DIK_DELETE 0xD3
#define DIK_PAUSE 0xC5
#define DIK_SCROLL 0x46
#define DIK_LWIN 0xDB
#define DIK_RWIN 0xDC
#define DIK_CAPITAL 0x3A
#define DIK_SYSRQ 0xB7
#define DIK_LALT 0x38
#define DIK_RALT 0xB8
#define DIK_APPS 0xDD
#endif

/*
**
*/
class	DirectInput {

public:
	typedef enum {
		NUM_KEYBOARD_BUTTONS	= 256,
		NUM_MOUSE_BUTTONS		= 3,
		NUM_JOYSTICK_BUTTONS	= 2
	} ButtonCounts;

	typedef enum {
		MOUSE_X_AXIS,
		MOUSE_Y_AXIS,
		MOUSE_Z_AXIS,
		NUM_MOUSE_AXIS,
	} MouseAxis;

	typedef enum {
		JOYSTICK_X_AXIS,
		JOYSTICK_Y_AXIS,
	} JoystickAxis;

	typedef enum {
		DI_BUTTON_HELD = 1,
		DI_BUTTON_HIT = 2,
		DI_BUTTON_RELEASED = 4
	} ButtonStateBitMask;

	/*
	** Buttons include all keyboard keys, plus the buttons
	** on the mouse and joysticks
	*/
	enum {
		BUTTON_KEYBOARD_FIRST	= 0,
		BUTTON_MOUSE_FIRST		= 256,
		BUTTON_MOUSE_LEFT			= BUTTON_MOUSE_FIRST,
		BUTTON_MOUSE_RIGHT,
		BUTTON_MOUSE_CENTER,
		BUTTON_JOYSTICK_FIRST,
		BUTTON_JOYSTICK_A			= BUTTON_JOYSTICK_FIRST,
		BUTTON_JOYSTICK_B,
		BUTTON_MAX
		//NUM_BUTTONS,
	};

	/*
	**
	*/
	static void Init( void );
	static void Shutdown( void );
	static void Read( void );
	static void Flush( void );

	static void Acquire(void);
	static void Unacquire(void);

	/*
	**
	*/
	static	int	Get_Keyboard_Button(	int button )	{	return DIKeyboardButtons[ button & 0xFF ]; }
	static	int	Get_Mouse_Button(	int button )		{	return DIMouseButtons[ button & 0xFF ]; }
	static	int	Get_Joystick_Button(	int button )	{	return DIJoystickButtons[ button & 0xFF ]; }
	static	int	Get_Mouse_Axis( MouseAxis axis )		{	return DIMouseAxis[axis]; }

	/*
	**
	*/
	static	void	Eat_Mouse_Held_States (void);

	// Still non-buffered
	static	int	Get_Joystick_Axis_State( JoystickAxis axis );

	//
	//	Cursor support
	//
	static void		Reset_Cursor_Pos (const Vector2 &pos)	{ CursorPos.X = pos.X; CursorPos.Y = pos.Y; }
	static void		Get_Cursor_Pos (Vector3 *pos)				{ *pos = CursorPos; }

	//
	//	Button support
	//
	static char		Get_Button_Value (int button_id);

	//
	//	Return the DIK ID of the last keyboard key pressed
	//
	static int		Get_Last_Key_Pressed (void)	{ return LastKeyPressed; }

private:

	//
	//	Internal methods
	//
	static void		Update_Double_Clicks (void);

	//
	//	Private member data
	//
	static	char						DIKeyboardButtons[NUM_KEYBOARD_BUTTONS];
	static	char						DIMouseButtons[NUM_MOUSE_BUTTONS];
	static	int						DIMouseAxis[NUM_MOUSE_AXIS];
	static	char						DIJoystickButtons[NUM_MOUSE_BUTTONS];
	static	float						ButtonLastHitTime[NUM_KEYBOARD_BUTTONS];

	static	Vector3					CursorPos;
	static	bool						EatMouseHeld;

	static	void *					DirectInputLibrary;

	static	int						LastKeyPressed;

	static bool Captured;

	static	void ReadKeyboard( void );
	static	void ReadMouse( void );
	static	void ReadJoystick( void );

};

///////////////////////////////////////////////////////////
//	Get_Button_Value
///////////////////////////////////////////////////////////
WWINLINE char
DirectInput::Get_Button_Value (int button_id)
{
	char retval = 0;

	if (button_id < BUTTON_MOUSE_FIRST) {
		retval = DIKeyboardButtons[button_id];
	} else if (button_id < BUTTON_JOYSTICK_FIRST) {
		retval = DIMouseButtons[button_id - BUTTON_MOUSE_FIRST];
	} else if (button_id < BUTTON_MAX) {
		retval = DIJoystickButtons[button_id - BUTTON_JOYSTICK_FIRST];
	}

	return retval;
}

#endif
