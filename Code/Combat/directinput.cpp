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
 *                     $Archive:: /Commando/Code/Combat/directinput.cpp                       $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/15/02 5:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "directinput.h"
#include "win.h"
#include "debug.h"
#include "timemgr.h"

#include <dinput.h>

/*
**
*/
LPDIRECTINPUT			DIObject				= NULL;
LPDIRECTINPUTDEVICE	DIKeyboardDevice		= NULL;
LPDIRECTINPUTDEVICE	DIMouseDevice			= NULL;
LPDIRECTINPUTDEVICE2	DIJoystickDevice		= NULL;

DIJOYSTATE				DIJoystickState;

// This warning is a result of the old dinput header we use so just disable it.
#pragma warning(disable: 4644)

int PASCAL	InitJoystick(LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef);

char						DirectInput::DIKeyboardButtons[NUM_KEYBOARD_BUTTONS];
char						DirectInput::DIMouseButtons[NUM_MOUSE_BUTTONS];
long						DirectInput::DIMouseAxis[NUM_MOUSE_AXIS];
char						DirectInput::DIJoystickButtons[NUM_MOUSE_BUTTONS];
float						DirectInput::ButtonLastHitTime[NUM_KEYBOARD_BUTTONS];
Vector3					DirectInput::CursorPos (0, 0, 0);
bool						DirectInput::EatMouseHeld = false;
bool						DirectInput::Captured = false;
void *					DirectInput::DirectInputLibrary = NULL;
int						DirectInput::LastKeyPressed = 0;

// Temp State Table (only for joystick currently)
char	Button_State_Table[4] = {	0,
								DirectInput::DI_BUTTON_HIT | DirectInput::DI_BUTTON_HELD,
								DirectInput::DI_BUTTON_RELEASED,
								DirectInput::DI_BUTTON_HELD };


// Buffered input
#define					DI_KEYBOARD_BUFFER_SIZE			20
#define					DI_MOUSE_BUFFER_SIZE				20

int PASCAL DirectInputInitJoystick(LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef);

#define	MINIMUM_DIRECTINPUT_VERSION		0x300

// Button bits
#define		BUTTON_BIT_DOUBLE				8
#define		BUTTON_DOUBLE_THRESHHOLD	0.25f


typedef HRESULT (WINAPI *DirectInput8CreateType) (HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
DirectInput8CreateType DirectInput8CreatePtr = NULL;


/*
**
*/
void DirectInput::Init( void )
{
	WWDEBUG_SAY(("DirectInput: Init\n"));

	HRESULT        hr;

	WWASSERT(DirectInputLibrary == NULL);
	DirectInputLibrary = LoadLibrary("DINPUT8.DLL");

	if (DirectInputLibrary != NULL) {
		DirectInput8CreatePtr = (DirectInput8CreateType) GetProcAddress((HINSTANCE)DirectInputLibrary, "DirectInput8Create");

		if (DirectInput8CreatePtr) {

			// Create the DirectInput Object
			hr = DirectInput8CreatePtr( ProgramInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DIObject, NULL);
			if FAILED(hr)
			{
				Debug_Say(( "DirectInput %x not available, trying version %x\n", DIRECTINPUT_VERSION/0x100, MINIMUM_DIRECTINPUT_VERSION/0x100 ));
				hr = DirectInput8CreatePtr( ProgramInstance, MINIMUM_DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DIObject, NULL);
				if FAILED(hr)
				{
					Debug_Say(( "DirectInput %x not available\n", MINIMUM_DIRECTINPUT_VERSION ));
					FreeLibrary((HINSTANCE)DirectInputLibrary);
					DirectInputLibrary = NULL;
					return;
				}
			}
		}
	}
//	Debug_Say(( "DirectInput object Created\n" ));

	// Create the Keyboard Object
	hr = DIObject->CreateDevice( GUID_SysKeyboard , &DIKeyboardDevice, NULL);
	WWASSERT( !FAILED(hr) );

	if ( DIKeyboardDevice != NULL ) {

		// Set the keyboard's data format
		hr = DIKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
		WWASSERT( !FAILED(hr) );

		// Set the keyboard's cooperative level
		// First we try for "exclusive" access (mainly so debugging works well) if that fails
		// then we'll take non-exclusive access.
#if 0
		hr = DIKeyboardDevice->SetCooperativeLevel( MainWindow,DISCL_FOREGROUND | DISCL_EXCLUSIVE);
		if (FAILED(hr)) {
			DIKeyboardDevice->SetCooperativeLevel( MainWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		}
#else
			DIKeyboardDevice->SetCooperativeLevel( MainWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
#endif

		// Set Keyboard Buffer Size
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(dipdw);
		dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = DI_KEYBOARD_BUFFER_SIZE;
		hr = DIKeyboardDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
		WWASSERT( !FAILED(hr) );

		// Aquire the keyboard
		hr = DIKeyboardDevice->Acquire();
		if ( FAILED(hr) ) {
			Debug_Say(( "DirectInput Keyboard Failed to Aquire\n" ));
#ifdef WWDEBUG
			if (hr == DIERR_INVALIDPARAM) WWDEBUG_SAY(("DIERR_INVALIDPARAM\n"));
			if (hr == DIERR_NOTINITIALIZED) WWDEBUG_SAY(("DIERR_NOTINITIALIZED\n"));
			if (hr == DIERR_OTHERAPPHASPRIO) WWDEBUG_SAY(("DIERR_OTHERAPPHASPRIO\n"));
#endif
		}

//		Debug_Say(( "DirectInput Keyboard Ready\n" ));
	}

	// Create the Mouse Object
	hr = DIObject->CreateDevice( GUID_SysMouse, &DIMouseDevice, NULL );
	WWASSERT( !FAILED(hr) );

	if ( DIMouseDevice != NULL ) {

		// Set the mouse's data format
		hr = DIMouseDevice->SetDataFormat(&c_dfDIMouse);
		WWASSERT( !FAILED(hr) );

		/**/
		// Set the mouse's cooperative level
		hr = DIMouseDevice->SetCooperativeLevel( MainWindow,
						DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		WWASSERT( !FAILED(hr) );
		/**/

		// Set Mouse Buffer Size
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(dipdw);
		dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = DI_MOUSE_BUFFER_SIZE;
		hr = DIMouseDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
		WWASSERT( !FAILED(hr) );

		// Aquire the mouse
		hr = DIMouseDevice->Acquire();
		if ( FAILED(hr) ) {
			Debug_Say(( "DirectInput Mouse Failed to Aquire\n" ));
			if (hr == DIERR_INVALIDPARAM) WWDEBUG_SAY(("DIERR_INVALIDPARAM\n"));
			if (hr == DIERR_NOTINITIALIZED) WWDEBUG_SAY(("DIERR_NOTINITIALIZED\n"));
			if (hr == DIERR_OTHERAPPHASPRIO) WWDEBUG_SAY(("DIERR_OTHERAPPHASPRIO\n"));
		}

//		Debug_Say(( "DirectInput Mouse Ready\n" ));
	}

	// Enumerate the Joysticks
	DIObject->EnumDevices(DI8DEVCLASS_GAMECTRL, InitJoystick, DIObject, DIEDFL_ATTACHEDONLY );

	if ( DIJoystickDevice != NULL ) {

		// Set the joystick's data format
		hr = DIJoystickDevice->SetDataFormat( &c_dfDIJoystick );
		WWASSERT( !FAILED(hr) );

		// Set the joystick's cooperative level
		hr = DIJoystickDevice->SetCooperativeLevel( MainWindow, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		WWASSERT( !FAILED(hr) );

		DIPROPRANGE diprg;
		diprg.diph.dwSize       = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);

		// Set X Range
		diprg.diph.dwObj        = DIJOFS_X;
		diprg.diph.dwHow        = DIPH_BYOFFSET;
		diprg.lMin              = -1000;
 		diprg.lMax              = +1000;
		hr = DIJoystickDevice->SetProperty(DIPROP_RANGE, &diprg.diph);
		WWASSERT( !FAILED(hr) );

		// Set Y Range
		diprg.diph.dwObj        = DIJOFS_Y;
		hr = DIJoystickDevice->SetProperty(DIPROP_RANGE, &diprg.diph);
		WWASSERT( !FAILED(hr) );

		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(dipdw);
		dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);

	   // set X axis dead zone to 20% (to avoid accidental turning)
		dipdw.diph.dwObj        = DIJOFS_X;
		dipdw.diph.dwHow        = DIPH_BYOFFSET;
		dipdw.dwData            = 200;
		hr = DIJoystickDevice->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
		WWASSERT( !FAILED(hr) );

	   // set Y axis dead zone to 20% (to avoid accidental turning)
		dipdw.diph.dwObj        = DIJOFS_Y;
		hr = DIJoystickDevice->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
		WWASSERT( !FAILED(hr) );

		// Aquire the mouse
		hr = DIJoystickDevice->Acquire();
		if ( FAILED(hr) ) {
			Debug_Say(( "DirectInput Joystick Failed to Aquire\n" ));
		}
	}

	Captured = true;

	Flush();

	//
	//	Reset the double-click array entries
	//
	for ( int index = 0; index < NUM_KEYBOARD_BUTTONS; index++ ) {
		ButtonLastHitTime[index] = 1000;
	}

	return ;
}

/*
**
*/
void DirectInput::Shutdown( void )
{
	WWDEBUG_SAY(("DirectInput: Shutdown\n"));

	if ( DIKeyboardDevice ) {
		DIKeyboardDevice->Unacquire();
		DIKeyboardDevice->Release();
		DIKeyboardDevice = NULL;
	}

	if ( DIMouseDevice ) {
		DIMouseDevice->Unacquire();
		DIMouseDevice->Release();
		DIMouseDevice = NULL;
	}

	if ( DIJoystickDevice ) {
		DIJoystickDevice->Unacquire();
		DIJoystickDevice->Release();
		DIJoystickDevice = NULL;
	}

	if ( DIObject ) {
		DIObject->Release();
		DIObject = NULL;
		if (DirectInputLibrary) {
			FreeLibrary((HINSTANCE)DirectInputLibrary);
		}
	}
}

/*
**
*/
void DirectInput::Flush( void )
{
	memset( DIKeyboardButtons, 0, sizeof(DIKeyboardButtons) );
	memset( DIMouseButtons, 0, sizeof(DIMouseButtons) );
	memset( DIMouseAxis, 0, sizeof(DIMouseAxis) );
	memset( DIJoystickButtons, 0, sizeof(DIJoystickButtons) );
}


/*
** Acquire access to input devices
*/
void DirectInput::Acquire(void)
{
//	WWDEBUG_SAY(("DirectInput: Acquire\n"));

	if (Captured == false) {
		Flush();

		if (DIKeyboardDevice) {
			DIKeyboardDevice->Acquire();
		}

		POINT cursorPos;
		GetCursorPos(&cursorPos);
		ScreenToClient(MainWindow, &cursorPos);

		CursorPos.X = (float)cursorPos.x;
		CursorPos.Y = (float)cursorPos.y;

		if (DIMouseDevice) {
			DIMouseDevice->Acquire();
		}

		Captured = true;
	}
}


/*
** Release accesss to input devices.
*/
void DirectInput::Unacquire(void)
{
//	WWDEBUG_SAY(("DirectInput: Unacquire\n"));

	if (Captured) {
		if (DIMouseDevice) {
			DIMouseDevice->Unacquire();
		}

		if (DIKeyboardDevice) {
			DIKeyboardDevice->Unacquire();
		}

		POINT cursorPos;
		cursorPos.x = (LONG)CursorPos.X;
		cursorPos.y = (LONG)CursorPos.Y;
		ClientToScreen(MainWindow, &cursorPos);
		SetCursorPos(cursorPos.x, cursorPos.y);

		Captured = false;
	}
}


/*
**
*/
int PASCAL InitJoystick(LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef)
{
   LPDIRECTINPUT pdi = (LPDIRECTINPUT)pvRef;

	if ( DIJoystickDevice == NULL ) {

		LPDIRECTINPUTDEVICE temp;
		HRESULT hr;
		hr = pdi->CreateDevice(pdinst->guidInstance, &temp, NULL);

		if ( !FAILED(hr)) {
			hr = temp->QueryInterface(IID_IDirectInputDevice2,
                                    (LPVOID *)&DIJoystickDevice);
			IDirectInputDevice_Release(temp);
		}
		if ( FAILED(hr) ) {
			Debug_Say(( "Failed to Create Joystick\n" ));
		} else {
			return DIENUM_STOP;		// we got one
		}
	}

	return DIENUM_CONTINUE;
}


/*
**
*/
void DirectInput::ReadKeyboard( void )
{
	if ( DIKeyboardDevice == NULL ) return;

	for (int i = 0; i < sizeof( DIKeyboardButtons ); i++ ) {
		DIKeyboardButtons[i] &= DI_BUTTON_HELD;	// make off all but the STATE
	}

	DWORD buffer_size = 1;
	DIDEVICEOBJECTDATA	input_buffer;

	bool done = false;
	while( !done ) {
		// Jani: Try to acquire first (Acquire doesn't increase ref count).
		HRESULT hr = DIKeyboardDevice->Acquire();
		if ( FAILED( hr) ) {
			return;
		}

		hr = DIKeyboardDevice->GetDeviceData(
			sizeof(DIDEVICEOBJECTDATA), &input_buffer, &buffer_size, 0 );

		if FAILED(hr) {

			if ( (hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED) ) {

				if (hr == DIERR_INPUTLOST) {
					Debug_Say(( "DirectInput keyboard lost\n" ));
				}

				// Try to re-aquire
				hr = DIKeyboardDevice->Acquire();
				if ( FAILED( hr) ) {
	//				Debug_Say(( "DirectInput keyboard not re-aquired\n" ));
					return;
				}
				Debug_Say(( "DirectInput keyboard re-aquired\n" ));
				continue;

			} else {
				Debug_Say(( "DirectInput GetDeviceState FAILED %x\n", hr ));
				return;
			}
		}

		if ( buffer_size == 0 ) {
			done = true;
		} else {
			WWASSERT( !(input_buffer.dwOfs & 0x0FF00) );

			if ( input_buffer.dwData & 0x80 ) {
				DIKeyboardButtons[ input_buffer.dwOfs ] |= DI_BUTTON_HIT;
				DIKeyboardButtons[ input_buffer.dwOfs ] |= DI_BUTTON_HELD;
				LastKeyPressed = input_buffer.dwOfs;
			} else {
				DIKeyboardButtons[ input_buffer.dwOfs ] |= DI_BUTTON_RELEASED;
				DIKeyboardButtons[ input_buffer.dwOfs ] &= ~DI_BUTTON_HELD;
			}
		}
	}

	// Set Dupe Keys
	DIKeyboardButtons[ DIK_CONTROL ]	= DIKeyboardButtons[ DIK_LCONTROL ]	| DIKeyboardButtons[ DIK_RCONTROL ] ;
	DIKeyboardButtons[ DIK_SHIFT ]	= DIKeyboardButtons[ DIK_LSHIFT ]	| DIKeyboardButtons[ DIK_RSHIFT ] ;
	DIKeyboardButtons[ DIK_ALT ]		= DIKeyboardButtons[ DIK_LALT ]		| DIKeyboardButtons[ DIK_RALT ];
	DIKeyboardButtons[ DIK_WIN ]		= DIKeyboardButtons[ DIK_LWIN ]		| DIKeyboardButtons[ DIK_RWIN ];

#if 0
retry_keyboard:
	HRESULT  hr = DIKeyboardDevice->GetDeviceState(sizeof(DIKeyboardState),(LPVOID)&DIKeyboardState);
	if FAILED(hr) {

		if ( (hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED) ) {

			if (hr == DIERR_INPUTLOST) {
				Debug_Say(( "DirectInput keyboard lost\n" ));
			}

			// Try to re-aquire
			hr = DIKeyboardDevice->Acquire();
			if ( FAILED( hr) ) {
//				Debug_Say(( "DirectInput keyboard not re-aquired\n" ));
				return;
			}
			Debug_Say(( "DirectInput keyboard re-aquired\n" ));
			goto	retry_keyboard;
		} else {
			Debug_Say(( "DirectInput GetDeviceState FAILED %x\n", hr ));
			return;
		}
	}

	// Set Dupe Keys
	DIKeyboardState[ DIK_CONTROL ]	= DIKeyboardState[ DIK_LCONTROL ]	| DIKeyboardState[ DIK_RCONTROL ] ;
	DIKeyboardState[ DIK_SHIFT ]		= DIKeyboardState[ DIK_LSHIFT ]		| DIKeyboardState[ DIK_RSHIFT ] ;
	DIKeyboardState[ DIK_ALT ]			= DIKeyboardState[ DIK_LALT ]			| DIKeyboardState[ DIK_RALT ];
	DIKeyboardState[ DIK_WIN ]			= DIKeyboardState[ DIK_LWIN ]			| DIKeyboardState[ DIK_RWIN ];
#endif
}


/*
**
*/
void DirectInput::ReadMouse( void )
{
	if ( DIMouseDevice == NULL ) return;

	int i;
	for (i = 0; i < sizeof( DIMouseButtons ); i++ ) {
		DIMouseButtons[i] &= DI_BUTTON_HELD;	// make off all but the STATE
	}

	for (i = 0; i < (sizeof( DIMouseAxis )/sizeof( DIMouseAxis[0] ) ); i++ ) {
		DIMouseAxis[i] = 0;
	}

	DWORD						buffer_size = 1;
	DIDEVICEOBJECTDATA	input_buffer;

	bool done = false;
	while( !done ) {
		// Try to aquire first
		HRESULT hr = DIMouseDevice->Acquire();
		if ( FAILED( hr) ) {
			return;
		}
		hr = DIMouseDevice->GetDeviceData(
			sizeof(DIDEVICEOBJECTDATA), &input_buffer, &buffer_size, 0 );

		if FAILED(hr) {

			if ( (hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED) ) {

				if (hr == DIERR_INPUTLOST) {
					Debug_Say(( "DirectInput mouse lost\n" ));
				}

				// Try to re-aquire
				hr = DIMouseDevice->Acquire();
				if ( FAILED( hr) ) {
	//				Debug_Say(( "DirectInput mouse not re-aquired\n" ));
					return;
				}
				Debug_Say(( "DirectInput mouse re-aquired\n" ));
				continue;

			} else {
				Debug_Say(( "DirectInput mouse GetDeviceState FAILED %x\n", hr ));
				return;
			}
		}

		if ( buffer_size == 0 ) {
			done = true;
		} else {
			int	index = 0;

			switch( input_buffer.dwOfs ) {

				case	DIMOFS_Z:	index++;
				case	DIMOFS_Y:	index++;
				case	DIMOFS_X:
							DIMouseAxis[index]	+= input_buffer.dwData;
							CursorPos[index]		+= ((int)input_buffer.dwData) * 2;
			   			break;

				case	DIMOFS_BUTTON2:	index++;
				case	DIMOFS_BUTTON1:	index++;
				case	DIMOFS_BUTTON0:
							if ( input_buffer.dwData & 0x80 ) {
								DIMouseButtons[ index ] |= DI_BUTTON_HIT;
								DIMouseButtons[ index ] |= DI_BUTTON_HELD;
							} else {
								DIMouseButtons[ index ] |= DI_BUTTON_RELEASED;
								DIMouseButtons[ index ] &= ~DI_BUTTON_HELD;
								EatMouseHeld = false;
							}
							break;
			}
		}
	}

	//
	//	"Eat" the left mouse button as necessary
	//
	if ( EatMouseHeld ) {
		DIMouseButtons[ BUTTON_MOUSE_LEFT & 0xFF ] &= ~DI_BUTTON_HELD;
		DIMouseButtons[ BUTTON_MOUSE_LEFT & 0xFF ] &= ~DI_BUTTON_HIT;
		DIMouseButtons[ BUTTON_MOUSE_LEFT & 0xFF ] |= DI_BUTTON_RELEASED;
	}


#if 0
retry_mouse:
	HRESULT  hr = DIMouseDevice->GetDeviceState( sizeof(DIMouseState), (LPVOID)&DIMouseState );
	if FAILED(hr) {

		if ( (hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED) ) {

			if (hr == DIERR_INPUTLOST) {
				Debug_Say(( "DirectInput mouse lost\n" ));
			}

			// Try to re-aquire
			hr = DIMouseDevice->Acquire();
			if ( FAILED( hr) ) {
//				Debug_Say(( "DirectInput mouse not re-aquired\n" ));
				return;
			}
			Debug_Say(( "DirectInput mouse re-aquired\n" ));
			goto	retry_mouse;
		} else {
			Debug_Say(( "DirectInput GetDeviceState FAILED %x\n", hr ));
			return;
		}
	}

	DIMouseButtons[ 0 ]	= Button_State_Table[ ((DIMouseButtons[ 0 ]&1) << 1) + ((DIMouseState.rgbButtons[ 0 ] & 0x80 )?1:0) ];
	DIMouseButtons[ 1 ]	= Button_State_Table[ ((DIMouseButtons[ 1 ]&1) << 1) + ((DIMouseState.rgbButtons[ 1 ] & 0x80 )?1:0) ];
	DIMouseButtons[ 2 ]	= Button_State_Table[ ((DIMouseButtons[ 2 ]&1) << 1) + ((DIMouseState.rgbButtons[ 2 ] & 0x80 )?1:0) ];
#endif
}


/*
**
*/
void DirectInput::ReadJoystick( void )
{
	if ( DIJoystickDevice == NULL ) return;

    // poll the joystick to read the current state
retry_joystick:

	HRESULT  hr = DIJoystickDevice->Poll();

	if (FAILED(hr)) {

		if ( (hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED) ) {

			if (hr == DIERR_INPUTLOST) {
				Debug_Say(( "DirectInput Joystick lost\n" ));
			}

			// Try to re-aquire
			hr = DIJoystickDevice->Acquire();
			if ( FAILED( hr) ) {
//				Debug_Say(( "DirectInput joystick not re-aquired\n" ));
				return;
			}
			Debug_Say(( "DirectInput Joystick re-aquired\n" ));
			goto	retry_joystick;
		}
	}

	hr = DIJoystickDevice->GetDeviceState( sizeof(DIJoystickState), (LPVOID)&DIJoystickState );
	if FAILED(hr) {
		Debug_Say(( "DirectInput GetDeviceState FAILED %x\n", hr ));
		return;
	}

	DIJoystickButtons[ 0 ]	= Button_State_Table[ ((DIJoystickButtons[ 0 ]&1) << 1) + ((DIJoystickState.rgbButtons[ 0 ] & 0x80 )?1:0) ];
	DIJoystickButtons[ 1 ]	= Button_State_Table[ ((DIJoystickButtons[ 1 ]&1) << 1) + ((DIJoystickState.rgbButtons[ 1 ] & 0x80 )?1:0) ];

}


/*
**
*/
void DirectInput::Read( void )
{
	if (Captured) {
		ReadKeyboard();
		ReadMouse();
		ReadJoystick();

		Update_Double_Clicks();
	}

	return ;
}


/*
**
*/
void DirectInput::Eat_Mouse_Held_States (void)
{
	if (	(DIMouseButtons[BUTTON_MOUSE_LEFT & 0xFF] & DI_BUTTON_HELD) ||
			(DIMouseButtons[BUTTON_MOUSE_LEFT & 0xFF] & DI_BUTTON_HIT))
	{
		EatMouseHeld = true;
	}

	return ;
}


/*
**
*/
long	DirectInput::Get_Joystick_Axis_State( JoystickAxis axis )
{
	return ((long*)&DIJoystickState.lX)[axis];
}


/*
**
*/
void	DirectInput::Update_Double_Clicks (void)
{
	float time_delta = TimeManager::Get_Frame_Real_Seconds();
	for ( int index = 0; index < NUM_KEYBOARD_BUTTONS; index++ ) {

		//
		// Bump time since last
		//
		ButtonLastHitTime[index] += time_delta;

		//
		// If the button is hit, check for double and reset time
		//
		if ( DIKeyboardButtons[index] & DI_BUTTON_HIT ) {
			if ( ButtonLastHitTime[index] <= BUTTON_DOUBLE_THRESHHOLD ) {
				DIKeyboardButtons[index] |= BUTTON_BIT_DOUBLE;
			}
			ButtonLastHitTime[index] = 0;
		}
	}

	return ;
}