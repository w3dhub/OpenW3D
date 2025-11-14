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
