/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D.
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

#include <iostream>

#include "directinput.h"
#include "debug.h"
#include "systimer.h"
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_events.h>

char						DirectInput::DIKeyboardButtons[NUM_KEYBOARD_BUTTONS];
char						DirectInput::DIMouseButtons[NUM_MOUSE_BUTTONS];
int						DirectInput::DIMouseAxis[NUM_MOUSE_AXIS];
char						DirectInput::DIJoystickButtons[NUM_MOUSE_BUTTONS];
float						DirectInput::ButtonLastHitTime[NUM_KEYBOARD_BUTTONS];
Vector3					DirectInput::CursorPos (0, 0, 0);
bool						DirectInput::EatMouseHeld = false;
bool						DirectInput::Captured = false;
void *					DirectInput::DirectInputLibrary = nullptr;
int						DirectInput::LastKeyPressed = 0;
static uint32_t last_mouse_time = 0;

// Temp State Table (only for joystick currently)
char	Button_State_Table[4] = {	0,
								DirectInput::DI_BUTTON_HIT | DirectInput::DI_BUTTON_HELD,
								DirectInput::DI_BUTTON_RELEASED,
								DirectInput::DI_BUTTON_HELD };

void DirectInput::Init( void )
{
	WWDEBUG_SAY(("DirectInput[SDL3]: Init\n"));
}

void DirectInput::Shutdown( void )
{
	WWDEBUG_SAY(("DirectInput[SDL3]: Shutdown\n"));
}

void DirectInput::Flush( void )
{
	memset( DIKeyboardButtons, 0, sizeof(DIKeyboardButtons) );
	memset( DIMouseButtons, 0, sizeof(DIMouseButtons) );
	memset( DIMouseAxis, 0, sizeof(DIMouseAxis) );
	memset( DIJoystickButtons, 0, sizeof(DIJoystickButtons) );
}

void DirectInput::Acquire(void)
{
	WWDEBUG_SAY(("DirectInput[SDL3]: Acquire\n"));
}

void DirectInput::Unacquire(void)
{
	WWDEBUG_SAY(("DirectInput[SDL3]: Unacquire\n"));
}

void DirectInput::ReadKeyboard( void )
{
    const bool *key_states = SDL_GetKeyboardState(NULL);

    for (int i = 0; i < sizeof( DIKeyboardButtons ); i++ ) {
		DIKeyboardButtons[i] &= DI_BUTTON_HELD;	// make off all but the STATE
	}

    for(int idx = 0; idx < NUM_KEYBOARD_BUTTONS; idx++){
        if ( key_states[idx] ) {
            DIKeyboardButtons[ idx ] |= DI_BUTTON_HIT;
            DIKeyboardButtons[ idx ] |= DI_BUTTON_HELD;
            LastKeyPressed = idx;
        } else {
            DIKeyboardButtons[ idx ] |= DI_BUTTON_RELEASED;
            DIKeyboardButtons[ idx ] &= ~DI_BUTTON_HELD;
        }
    }


}

void DirectInput::ReadMouse( void )
{
	uint32_t now_ms = timeGetTime();

	if(now_ms - last_mouse_time < 16){
		// don't clear out the data if it's too new, otherwise we don't get any mouse events
		return;
	}

	for (int i = 0; i < sizeof( DIMouseButtons ); i++ ) {
		DIMouseButtons[i] &= DI_BUTTON_HELD;	// make off all but the STATE
	}

	for (int i = 0; i < (sizeof( DIMouseAxis )/sizeof( DIMouseAxis[0] ) ); i++ ) {
		DIMouseAxis[i] = 0;
	}
}

void DirectInput::Read( void )
{
    ReadKeyboard();
	ReadMouse();
}

void DirectInput::Eat_Mouse_Held_States (void)
{
	if (	(DIMouseButtons[BUTTON_MOUSE_LEFT & 0xFF] & DI_BUTTON_HELD) ||
			(DIMouseButtons[BUTTON_MOUSE_LEFT & 0xFF] & DI_BUTTON_HIT))
	{
		EatMouseHeld = true;
	}

	return ;
}

int	DirectInput::Get_Joystick_Axis_State( JoystickAxis axis )
{
	return 0;
}

void DirectInput::SDL3_Key_Pressed(SDL_KeyboardEvent* evt){
	if ( evt->down ) {
		DIKeyboardButtons[ evt->scancode ] |= DI_BUTTON_HIT;
		DIKeyboardButtons[ evt->scancode ] |= DI_BUTTON_HELD;
		LastKeyPressed = evt->scancode;
	} else {
		DIKeyboardButtons[ evt->scancode ] |= DI_BUTTON_RELEASED;
		DIKeyboardButtons[ evt->scancode ] &= ~DI_BUTTON_HELD;
	}
}

void DirectInput::SDL3_Mouse_Motion_Event(SDL_MouseMotionEvent* evt){
	CursorPos[1] = evt->y;
	CursorPos[0] = evt->x;
	DIMouseAxis[1] = (int)evt->yrel;
	DIMouseAxis[0] = (int)evt->xrel;

	last_mouse_time = timeGetTime();
}

void DirectInput::SDL3_Mouse_Button_Event(SDL_MouseButtonEvent* evt){
	int index = 0;
	if(evt->button = 2){
		index = 0;
	}else if(evt->button == 1){
		index = 1;
	}else if(evt->button == 0){
		index = 2;
	}else{
		// drop it like it's hot, we don't use this button
		return;
	}

	if(evt->down){
		DIMouseButtons[ index ] |= DI_BUTTON_HIT;
		DIMouseButtons[ index ] |= DI_BUTTON_HELD;
	} else {
		DIMouseButtons[ index ] |= DI_BUTTON_RELEASED;
		DIMouseButtons[ index ] &= ~DI_BUTTON_HELD;
		EatMouseHeld = false;
	}
}