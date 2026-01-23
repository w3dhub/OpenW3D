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
 *                     $Archive:: /Commando/Code/Combat/input.cpp                             $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/15/02 3:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 120                                                        $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "input.h"
#include "slist.h"
#include "assets.h"
#include "ini.h"
#include "directinput.h"
#include "debug.h"
#include "timemgr.h"
#include "registry.h"
#include "ffactory.h"
#include "win.h"
#include "translatedb.h"
#include "string_ids.h"
#include "vehicle.h"
#include "combat.h"
#include "ccamera.h"

#include <dinput.h>

#include <stdio.h>
#include <algorithm>

////////////////////////////////////////////////////////////////
//	Special virtual keys
////////////////////////////////////////////////////////////////
#define VK_MOUSEWHEEL_UP      0x100
#define VK_MOUSEWHEEL_DOWN    0x101


/*
**
*/
float	MouseSensitivity	= 0.5f;
float	MouseScale			= 1;
bool	MouseInvert			= true;
bool	Mouse2DInvert		= false;

const char *DEFAULT_INPUT_FILENAME = "DEFAULT_INPUT.CFG";

#define		BUTTON_BIT_HELD		DirectInput::DI_BUTTON_HELD
#define		BUTTON_BIT_HIT			DirectInput::DI_BUTTON_HIT
#define		BUTTON_BIT_RELEASED	DirectInput::DI_BUTTON_RELEASED
#define		BUTTON_BIT_DOUBLE		8
#define		SLIDER_DEAD_ZONE		0.01f

/*
**	Button states
*/
#define		BUTTON_UP				0x0000
#define		BUTTON_HIT				0x0001
#define		BUTTON_RELEASE			0x0002
#define		BUTTON_HELD				0x0003
#define		BUTTON_DOUBLE			0x0004
#define		BUTTON_SHIFT			0x1000
#define		BUTTON_CTRL			0x2000
#define		BUTTON_ALT			0x4000
#define		BUTTON_DEBUG			0x8000

/*
** Database Strings
*/
#define	ENTRY_ACTIVATE_BUTTON		"ActivateButton"
#define	ENTRY_FUNCTION					"Function"
#define	ENTRY_MIN						"Min"
#define	ENTRY_MAX						"Max"
#define	ENTRY_ACCELERATION			"Acceleration"

#define	SECTION_MISC_SETTINGS		"Misc Settings"
#define	ENTRY_DAMAGE_INDICATORS		"DamageIndicatorsEnabled"
#define	ENTRY_MOUSE_SENSITIVITY		"MouseSensitivity"
#define	ENTRY_MOUSE_SCALE				"MouseScale"
#define	ENTRY_MOUSE_INVERT			"MouseInvert"
#define	ENTRY_MOUSE_2D_INVERT		"Mouse2DInvert"
#define	ENTRY_TARGET_STEERING		"TargetSteering"


typedef struct {
	short	ID;
	const char	*Name;
} StringID;

StringID	ButtonNames[] = {
	{	DIK_F1,					"F1_Key"						},
	{	DIK_F2,					"F2_Key"						},
	{	DIK_F3,					"F3_Key"						},
	{	DIK_F4,					"F4_Key"						},
	{	DIK_F5,					"F5_Key"						},
	{	DIK_F6,					"F6_Key"						},
	{	DIK_F7,					"F7_Key"						},
	{	DIK_F8,					"F8_Key"						},
	{	DIK_F9,					"F9_Key"						},
	{	DIK_F10,					"F10_Key"					},
	{	DIK_F11,					"F11_Key"					},
	{	DIK_F12,					"F12_Key"					},
	{	DIK_0,					"0_Key"						},
	{	DIK_1,					"1_Key"						},
	{	DIK_2,					"2_Key"						},
	{	DIK_3,					"3_Key"						},
	{	DIK_4,					"4_Key"						},
	{	DIK_5,					"5_Key"						},
	{	DIK_6,					"6_Key"						},
	{	DIK_7,					"7_Key"						},
	{	DIK_8,					"8_Key"						},
	{	DIK_9,					"9_Key"						},
	{	DIK_A,					"A_Key"						},
	{	DIK_B,					"B_Key"						},
	{	DIK_C,					"C_Key"						},
	{	DIK_D,					"D_Key"						},
	{	DIK_E,					"E_Key"						},
	{	DIK_F,					"F_Key"						},
	{	DIK_G,					"G_Key"						},
	{	DIK_H,					"H_Key"						},
	{	DIK_I,					"I_Key"						},
	{	DIK_J,					"J_Key"						},
	{	DIK_K,					"K_Key"						},
	{	DIK_L,					"L_Key"						},
	{	DIK_M,					"M_Key"						},
	{	DIK_N,					"N_Key"						},
	{	DIK_O,					"O_Key"						},
	{	DIK_P,					"P_Key"						},
	{	DIK_Q,					"Q_Key"						},
	{	DIK_R,					"R_Key"						},
	{	DIK_S,					"S_Key"						},
	{	DIK_T,					"T_Key"						},
	{	DIK_U,					"U_Key"						},
	{	DIK_V,					"V_Key"						},
	{	DIK_W,					"W_Key"						},
	{	DIK_X,					"X_Key"						},
	{	DIK_Y,					"Y_Key"						},
	{	DIK_Z,					"Z_Key"						},
	{	DIK_MINUS,				"Minus_Key"					},
	{	DIK_EQUALS,				"Equals_Key"				},
	{	DIK_BACK,				"Backspace_Key"			},
	{	DIK_TAB,					"Tab_Key"					},
	{	DIK_LBRACKET,			"Left_Bracket_Key" 		},
	{	DIK_RBRACKET,			"Right_Bracket_Key" 		},
	{	DIK_RETURN,				"Enter_Key"					},
	{	DIK_SEMICOLON,			"Semicolon_Key"			},
	{	DIK_APOSTROPHE,		"Apostrophe_Key"			},
	{	DIK_GRAVE,				"Grave_Key"					},
	{	DIK_BACKSLASH,			"Backslash_Key" 			},
	{	DIK_COMMA,				"Comma_Key" 				},
	{	DIK_PERIOD,				"Period_Key"				},
	{	DIK_SLASH,				"Slash_Key" 				},
	{	DIK_SPACE,				"Space_Bar_Key" 			},
	{	DIK_CAPITAL,			"Caps_Lock_Key" 			},
	{	DIK_NUMLOCK,			"Num_Lock_Key" 			},
	{	DIK_SCROLL,				"Scroll_Lock_Key" 		},
	{	DIK_ESCAPE,				"Escape_Key"				},
	{	DIK_NUMPAD0,			"Keypad_0_Key"				},
	{	DIK_NUMPAD1,			"Keypad_1_Key"				},
	{	DIK_NUMPAD2,			"Keypad_2_Key"				},
	{	DIK_NUMPAD3,			"Keypad_3_Key"				},
	{	DIK_NUMPAD4,			"Keypad_4_Key"				},
	{	DIK_NUMPAD5,			"Keypad_5_Key"				},
	{	DIK_NUMPAD6,			"Keypad_6_Key"				},
	{	DIK_NUMPAD7,			"Keypad_7_Key"				},
	{	DIK_NUMPAD8,			"Keypad_8_Key"				},
	{	DIK_NUMPAD9,			"Keypad_9_Key"				},
	{	DIK_SUBTRACT,			"Keypad_Minus_Key" 		},
	{	DIK_MULTIPLY,			"Keypad_Star_Key"	 		},
	{	DIK_ADD,					"Keypad_Plus_Key"	 		},
	{	DIK_DECIMAL,			"Keypad_Period_Key" 		},
	{	DIK_NUMPADENTER,		"Keypad_Enter_Key"  		},
	{	DIK_DIVIDE,				"Keypad_Slash_Key"  		},
	{	DIK_HOME,				"Home_Key"  				},
	{	DIK_PRIOR,				"Page_Up_Key"  			},
	{	DIK_END,					"End_Key"  					},
	{	DIK_NEXT,				"Page_Down_Key"  			},
	{	DIK_INSERT,				"Insert_Key"  				},
	{	DIK_DELETE,				"Delete_Key"  				},
	{	DIK_UP,					"Up_Key"  					},
	{	DIK_DOWN,				"Down_Key"  				},
	{	DIK_LEFT,				"Left_Key"  				},
	{	DIK_RIGHT,				"Right_Key"  				},
	{	DIK_SYSRQ,				"Sys_Req_Key"  			},
	{	DIK_CONTROL,			"Control_Key"				},
	{	DIK_LCONTROL,			"Left_Control_Key"		},
	{	DIK_RCONTROL,			"Right_Control_Key"		},
	{	DIK_SHIFT,				"Shift_Key" 				},
	{	DIK_LSHIFT,				"Left_Shift_Key" 			},
	{	DIK_RSHIFT,				"Right_Shift_Key" 		},
	{	DIK_ALT,					"Alt_Key"		 			},
	{	DIK_LALT,				"Left_Alt_Key" 			},
	{	DIK_RALT,				"Right_Alt_Key" 			},
	{	DIK_WIN,					"Windows_Key"  			},
	{	DIK_LWIN,				"Left_Windows_Key"  		},
	{	DIK_RWIN,				"Right_Windows_Key"  	},
	{	DIK_APPS,				"App_Menu_Key"  			},

	{	DirectInput::BUTTON_MOUSE_LEFT,	"Left_Mouse_Button"		},
	{	DirectInput::BUTTON_MOUSE_RIGHT,	"Right_Mouse_Button"		},
	{	DirectInput::BUTTON_MOUSE_CENTER,	"Center_Mouse_Button"	},

	{	DirectInput::BUTTON_JOYSTICK_A,	"Joystick_Button_A"		},
	{	DirectInput::BUTTON_JOYSTICK_B,	"Joystick_Button_B"		},

};

#define	NUM_BUTTON_NAMES	( sizeof(ButtonNames) / sizeof(ButtonNames[0]) )


StringID	SliderNames[] = {
	{	Input::SLIDER_MOUSE_LEFT,	"Mouse_Left"	},
	{	Input::SLIDER_MOUSE_RIGHT,	"Mouse_Right"	},
	{	Input::SLIDER_MOUSE_UP,		"Mouse_Up"		},
	{	Input::SLIDER_MOUSE_DOWN,	"Mouse_Down"	},
	{	Input::SLIDER_MOUSE_WHEEL_FORWARD,	"Mouse_Wheel_Forward"	},
	{	Input::SLIDER_MOUSE_WHEEL_BACKWARD,	"Mouse_Wheel_Backward"	},
	{	Input::SLIDER_JOYSTICK_LEFT,	"Joystick_Left"	},
	{	Input::SLIDER_JOYSTICK_RIGHT,	"Joystick_Right"	},
	{	Input::SLIDER_JOYSTICK_UP,		"Joystick_Up"		},
	{	Input::SLIDER_JOYSTICK_DOWN,	"Joystick_Down"	},
};

#define	NUM_SLIDER_NAMES	( sizeof( SliderNames ) / sizeof( SliderNames[0] ) )

#define	NUM_FUNCTIONS	INPUT_FUNCTION_COUNT

StringID	Functions[ NUM_FUNCTIONS ] = {

	{	INPUT_FUNCTION_MOVE_FORWARD,				"MoveForward"			},
	{	INPUT_FUNCTION_MOVE_BACKWARD,				"MoveBackward"			},
	{	INPUT_FUNCTION_MOVE_LEFT,					"MoveLeft"				},
	{	INPUT_FUNCTION_MOVE_RIGHT,					"MoveRight"				},
	{	INPUT_FUNCTION_MOVE_UP,						"MoveUp"					},
	{	INPUT_FUNCTION_MOVE_DOWN,					"MoveDown"				},

	{	INPUT_FUNCTION_WALK_MODE,					"WalkMode"				},

	{	INPUT_FUNCTION_TURN_LEFT,					"TurnLeft"				},
	{	INPUT_FUNCTION_TURN_RIGHT,					"TurnRight"				},
	{	INPUT_FUNCTION_VEHICLE_TURN_LEFT,		"VehicleTurnLeft"		},
	{	INPUT_FUNCTION_VEHICLE_TURN_RIGHT,		"VehicleTurnRight"	},
	{	INPUT_FUNCTION_VEHICLE_TOGGLE_GUNNER,	"VehicleToggleGunner"},

	{	INPUT_FUNCTION_WEAPON_UP,					"WeaponUp"				},
	{	INPUT_FUNCTION_WEAPON_DOWN, 				"WeaponDown"			},
	{	INPUT_FUNCTION_WEAPON_LEFT, 				"WeaponLeft"			},
	{	INPUT_FUNCTION_WEAPON_RIGHT,				"WeaponRight"			},
	{	INPUT_FUNCTION_WEAPON_RESET,				"WeaponReset"			},

	{	INPUT_FUNCTION_ZOOM_IN,						"ZoomIn"					},
	{	INPUT_FUNCTION_ZOOM_OUT,					"ZoomOut"				},

	{	INPUT_FUNCTION_ACTION,						"Action"					},	// Was Ladder
	{	INPUT_FUNCTION_JUMP,							"Jump"					},
	{	INPUT_FUNCTION_CROUCH,						"Crouch"					},

	{	INPUT_FUNCTION_DIVE_FORWARD,				"DiveForward" 			},
	{	INPUT_FUNCTION_DIVE_BACKWARD,				"DiveBackward"			},
	{	INPUT_FUNCTION_DIVE_LEFT,					"DiveLeft"				},
	{	INPUT_FUNCTION_DIVE_RIGHT,					"DiveRight"				},

	{	INPUT_FUNCTION_TURN_AROUND,				"TurnAround"			},
	{	INPUT_FUNCTION_DROP_FLAG,					"DropFlag"				},

	{	INPUT_FUNCTION_NEXT_WEAPON,				"NextWeapon"			},
	{	INPUT_FUNCTION_PREV_WEAPON,				"PrevWeapon"			},
	{	INPUT_FUNCTION_FIRE_WEAPON_PRIMARY,		"FireWeaponPrimary"	},
	{	INPUT_FUNCTION_FIRE_WEAPON_SECONDARY, 	"FireWeaponSecondary"},
	{	INPUT_FUNCTION_USE_WEAPON,					"UseWeapon"				},
	{	INPUT_FUNCTION_RELOAD_WEAPON,				"ReloadWeapon"			},

	{	INPUT_FUNCTION_SELECT_NO_WEAPON,			"SelectNoWeapon"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_0,			"SelectWeapon0"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_1,			"SelectWeapon1"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_2,			"SelectWeapon2"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_3,			"SelectWeapon3"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_4,			"SelectWeapon4"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_5,			"SelectWeapon5"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_6,			"SelectWeapon6"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_7,			"SelectWeapon7"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_8,			"SelectWeapon8"		},
	{	INPUT_FUNCTION_SELECT_WEAPON_9,			"SelectWeapon9"		},

	{	INPUT_FUNCTION_CYCLE_POG,					"CyclePog"				},

//	{	INPUT_FUNCTION_HUD_ZOOM_RADAR_IN,		"HudZoomRadarIn"		},
//	{	INPUT_FUNCTION_HUD_ZOOM_RADAR_OUT,		"HudZoomRadarOut"		},

	{	INPUT_FUNCTION_PANIC,						"Panic"					},
	{	INPUT_FUNCTION_CURSOR_TARGETING,			"CursorTargeting"		},
	{	INPUT_FUNCTION_FIRST_PERSON_TOGGLE,		"FirstPersonToggle"	},
	{	INPUT_FUNCTION_SUICIDE,						"Suicide"				},

	{	INPUT_FUNCTION_VERBOSE_HELP,				"VerboseHelp"			},

	{	INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE,	"BeginPublicMessage" },
	{	INPUT_FUNCTION_BEGIN_TEAM_MESSAGE,	   "BeginTeamMessage"   },
	{	INPUT_FUNCTION_BEGIN_PRIVATE_MESSAGE,	"BeginPrivateMessage"},
	{	INPUT_FUNCTION_BEGIN_CONSOLE,				"BeginConsole"			},

	{	INPUT_FUNCTION_HELP_SCREEN,				"HelpScreen"			},

	{	INPUT_FUNCTION_EVA_OBJECTIVES_SCREEN,	"ObjectivesScreen"	},
	{	INPUT_FUNCTION_EVA_MAP_SCREEN,			"MapScreen"				},

	// DEBUGGING
	{	INPUT_FUNCTION_CAMERA_HEADING_LEFT,		"CameraHeadingLeft"	},
	{	INPUT_FUNCTION_CAMERA_HEADING_RIGHT,	"CameraHeadingRight"	},

	{	INPUT_FUNCTION_CAMERA_TRANSTILT_INC,	"CameraTransTiltInc" },
	{	INPUT_FUNCTION_CAMERA_TRANSTILT_DEC,	"CameraTransTiltDec" },
	{	INPUT_FUNCTION_CAMERA_VIEWTILT_INC,		"CameraViewTiltInc"	},
	{	INPUT_FUNCTION_CAMERA_VIEWTILT_DEC,		"CameraViewTiltDec"	},
	{	INPUT_FUNCTION_CAMERA_DIST_INC,			"CameraDistInc"		},
	{	INPUT_FUNCTION_CAMERA_DIST_DEC,			"CameraDistDec"		},
	{	INPUT_FUNCTION_CAMERA_FOV_INC,			"CameraFOVInc"			},
	{	INPUT_FUNCTION_CAMERA_FOV_DEC,			"CameraFOVDec"			},
	{	INPUT_FUNCTION_CAMERA_HEIGHT_INC,		"CameraHeightInc"		},
	{	INPUT_FUNCTION_CAMERA_HEIGHT_DEC,		"CameraHeightDec"		},

	{	INPUT_FUNCTION_MENU_SERVERQUICKSTART,	"ServerQuickStart"	},
	{	INPUT_FUNCTION_MENU_CLIENTQUICKSTART,	"ClientQuickStart"	},
	{	INPUT_FUNCTION_MAKE_SCREEN_SHOT,			"MakeScreenShot"		},
	{	INPUT_FUNCTION_TOGGLE_MOVIE_CAPTURE,	"ToggleMovieCapture"	},
	//{	INPUT_FUNCTION_INCREMENT_ACTIVE_MESSAGE,	"IncrementActiveMessage"	},
	//{	INPUT_FUNCTION_DECREMENT_ACTIVE_MESSAGE,	"DecrementActiveMessage"	},
	//{	INPUT_FUNCTION_INCREMENT_DEBUG_SWITCH_1,	"IncrementDebugSwitch1"	},
	//{	INPUT_FUNCTION_INCREMENT_DEBUG_SWITCH_2,	"IncrementDebugSwitch2"	},
	{	INPUT_FUNCTION_DEBUG_SINGLE_STEP,		"DebugSingleStep"		},
	{	INPUT_FUNCTION_DEBUG_SINGLE_STEP_STEP,	"DebugSingleStepStep"},
	{	INPUT_FUNCTION_DEBUG_RAPID_MOVE,			"DebugRapidMove"		},
	{	INPUT_FUNCTION_DEBUG_GENERIC0,			"GenericDebug0"		},
	{	INPUT_FUNCTION_DEBUG_GENERIC1,			"GenericDebug1"		},
	{	INPUT_FUNCTION_DEBUG_FAR_CLIP_IN,		"DebugFarClipIn"		},
	{	INPUT_FUNCTION_DEBUG_FAR_CLIP_OUT,		"DebugFarClipOut"		},
	{	INPUT_FUNCTION_QUICK_FULL_EXIT,			"QuickFullExit"	   },
	{	INPUT_FUNCTION_VIS_UPDATE,					"VisUpdate"				},
//	{	INPUT_FUNCTION_TOGGLE_PERFORMANCE_SAMPLING,	"TogglePerformanceSampling"	},
	{	INPUT_FUNCTION_TOGGLE_SNAP_SHOT_MODE,	"ToggleSnapShotMode"	},
	{	INPUT_FUNCTION_SNAP_SHOT_ADVANCE,		"SnapShotAdvance"	},
	//{	INPUT_FUNCTION_DEBUG_OPTIONS_DIALOG,   "DebugOptionsDialog"	},
	{	INPUT_FUNCTION_CNC,							"CNC"	},
	{	INPUT_FUNCTION_QUICKSAVE,					"Quicksave"	},

	// Menu
	{	INPUT_FUNCTION_MENU_TOGGLE,				"MenuToggle"			},

	// In game EVA
	{	INPUT_FUNCTION_EVA_MISSION_OBJECTIVES_TOGGLE,	"EvaMissionObjectives"			},

	{	INPUT_FUNCTION_RADIO_CMD_01, "RadioCommand01"},
	{	INPUT_FUNCTION_RADIO_CMD_02, "RadioCommand02"},
	{	INPUT_FUNCTION_RADIO_CMD_03, "RadioCommand03"},
	{	INPUT_FUNCTION_RADIO_CMD_04, "RadioCommand04"},
	{	INPUT_FUNCTION_RADIO_CMD_05, "RadioCommand05"},
	{	INPUT_FUNCTION_RADIO_CMD_06, "RadioCommand06"},
	{	INPUT_FUNCTION_RADIO_CMD_07, "RadioCommand07"},
	{	INPUT_FUNCTION_RADIO_CMD_08, "RadioCommand08"},
	{	INPUT_FUNCTION_RADIO_CMD_09, "RadioCommand09"},
	{	INPUT_FUNCTION_RADIO_CMD_10, "RadioCommand10"},

	{	INPUT_FUNCTION_RADIO_CMD_11, "RadioCommand11"},
	{	INPUT_FUNCTION_RADIO_CMD_12, "RadioCommand12"},
	{	INPUT_FUNCTION_RADIO_CMD_13, "RadioCommand13"},
	{	INPUT_FUNCTION_RADIO_CMD_14, "RadioCommand14"},
	{	INPUT_FUNCTION_RADIO_CMD_15, "RadioCommand15"},
	{	INPUT_FUNCTION_RADIO_CMD_16, "RadioCommand16"},
	{	INPUT_FUNCTION_RADIO_CMD_17, "RadioCommand17"},
	{	INPUT_FUNCTION_RADIO_CMD_18, "RadioCommand18"},
	{	INPUT_FUNCTION_RADIO_CMD_19, "RadioCommand19"},
	{	INPUT_FUNCTION_RADIO_CMD_20, "RadioCommand20"},

	{	INPUT_FUNCTION_RADIO_CMD_21, "RadioCommand21"},
	{	INPUT_FUNCTION_RADIO_CMD_22, "RadioCommand22"},
	{	INPUT_FUNCTION_RADIO_CMD_23, "RadioCommand23"},
	{	INPUT_FUNCTION_RADIO_CMD_24, "RadioCommand24"},
	{	INPUT_FUNCTION_RADIO_CMD_25, "RadioCommand25"},
	{	INPUT_FUNCTION_RADIO_CMD_26, "RadioCommand26"},
	{	INPUT_FUNCTION_RADIO_CMD_27, "RadioCommand27"},
	{	INPUT_FUNCTION_RADIO_CMD_28, "RadioCommand28"},
	{	INPUT_FUNCTION_RADIO_CMD_29, "RadioCommand29"},
	{	INPUT_FUNCTION_RADIO_CMD_30, "RadioCommand30"},

	{	INPUT_FUNCTION_TEAM_INFO_TOGGLE,		"TeamInfoToggle"	},
	{	INPUT_FUNCTION_BATTLE_INFO_TOGGLE,	"BattleInfoToggle"	},
	{	INPUT_FUNCTION_SERVER_INFO_TOGGLE,	"ServerInfoToggle"	},

	// Profiler Controls
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD0, "ProfileEnterChild0" },
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD1, "ProfileEnterChild1" },
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD2, "ProfileEnterChild2" },
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD3, "ProfileEnterChild3" },
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD4, "ProfileEnterChild4" },
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD5, "ProfileEnterChild5" },
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD6, "ProfileEnterChild6" },
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD7, "ProfileEnterChild7" },
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD8, "ProfileEnterChild8" },
	{ INPUT_FUNCTION_PROFILE_ENTER_CHILD9, "ProfileEnterChild9" },
	{ INPUT_FUNCTION_PROFILE_ENTER_PARENT, "ProfileEnterParent" },
	{ INPUT_FUNCTION_PROFILE_RESET, "ProfileReset" },

};


/*
** FunctionKeyStates
*/
int	FunctionKeyStates[ NUM_FUNCTIONS ] =
{
	BUTTON_HELD, //INPUT_FUNCTION_MOVE_FORWARD,
	BUTTON_HELD, //INPUT_FUNCTION_MOVE_BACKWARD,
	BUTTON_HELD, //INPUT_FUNCTION_MOVE_LEFT,
	BUTTON_HELD, //INPUT_FUNCTION_MOVE_RIGHT,
	BUTTON_HELD, //INPUT_FUNCTION_MOVE_UP,
	BUTTON_HELD, //INPUT_FUNCTION_MOVE_DOWN,

	BUTTON_HELD, //INPUT_FUNCTION_WALK_MODE,

	BUTTON_HELD, //INPUT_FUNCTION_TURN_LEFT,
	BUTTON_HELD, //INPUT_FUNCTION_TURN_RIGHT,

	BUTTON_HELD, //INPUT_FUNCTION_VEHICLE_TURN_LEFT,
	BUTTON_HELD, //INPUT_FUNCTION_VEHICLE_TURN_RIGHT,
	BUTTON_HIT, //INPUT_FUNCTION_VEHICLE_TOGGLE_GUNNER,

	BUTTON_HELD, //INPUT_FUNCTION_WEAPON_UP,
	BUTTON_HELD, //INPUT_FUNCTION_WEAPON_DOWN,
	BUTTON_HELD, //INPUT_FUNCTION_WEAPON_LEFT,
	BUTTON_HELD, //INPUT_FUNCTION_WEAPON_RIGHT,
	BUTTON_HELD, //INPUT_FUNCTION_WEAPON_RESET,

	BUTTON_HELD, //INPUT_FUNCTION_ZOOM_IN,
	BUTTON_HELD, //INPUT_FUNCTION_ZOOM_OUT,

	BUTTON_HIT, //INPUT_FUNCTION_ACTION,
	BUTTON_HIT, //INPUT_FUNCTION_JUMP,
//	BUTTON_HIT, //INPUT_FUNCTION_CROUCH,
	BUTTON_HELD, //INPUT_FUNCTION_CROUCH,

	BUTTON_DOUBLE, //INPUT_FUNCTION_DIVE_FORWARD,
	BUTTON_DOUBLE, //INPUT_FUNCTION_DIVE_BACKWARD,
	BUTTON_DOUBLE, //INPUT_FUNCTION_DIVE_LEFT,
	BUTTON_DOUBLE, //INPUT_FUNCTION_DIVE_RIGHT,

	BUTTON_HIT, //INPUT_FUNCTION_TURN_AROUND,
	BUTTON_HIT, //INPUT_FUNCTION_DROP_FLAG,

	BUTTON_HIT, //INPUT_FUNCTION_NEXT_WEAPON,
	BUTTON_HIT, //INPUT_FUNCTION_PREV_WEAPON,
	BUTTON_HELD, //INPUT_FUNCTION_FIRE_WEAPON_PRIMARY,
	BUTTON_HELD, //INPUT_FUNCTION_FIRE_WEAPON_SECONDARY,
	BUTTON_HIT, //INPUT_FUNCTION_USE_WEAPON,
	BUTTON_HIT, //INPUT_FUNCTION_RELOAD_WEAPON,

	BUTTON_HIT, //INPUT_FUNCTION_SELECT_NO_WEAPON,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_0,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_1,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_2,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_3,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_4,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_5,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_6,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_7,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_8,
	BUTTON_HIT, //INPUT_FUNCTION_SELECT_WEAPON_9,

	BUTTON_HIT, //INPUT_FUNCTION_CYCLE_POG

//	BUTTON_HIT, //INPUT_FUNCTION_HUD_ZOOM_RADAR_IN,
//	BUTTON_HIT, //INPUT_FUNCTION_HUD_ZOOM_RADAR_OUT,

	BUTTON_HIT, //INPUT_FUNCTION_PANIC,
	BUTTON_HELD, //INPUT_FUNCTION_CURSOR_TARGETING,
	BUTTON_HIT, //INPUT_FUNCTION_FIRST_PERSON_TOGGLE,
	BUTTON_HIT, //INPUT_FUNCTION_SUICIDE,

	BUTTON_HIT, //INPUT_FUNCTION_VERBOSE_HELP,

	BUTTON_HIT, //INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE,
	BUTTON_HIT, //INPUT_FUNCTION_BEGIN_TEAM_MESSAGE,
	BUTTON_HIT, //INPUT_FUNCTION_BEGIN_PRIVATE_MESSAGE,
	BUTTON_HIT, //INPUT_FUNCTION_BEGIN_CONSOLE,

	BUTTON_HIT, //INPUT_FUNCTION_HELP_SCREEN,

	BUTTON_HIT,	//INPUT_FUNCTION_EVA_OBJECTIVES_SCREEN
	BUTTON_HIT, //INPUT_FUNCTION_EVA_MAP_SCREEN

	// DEBUGGING
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_HEADING_LEFT,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_HEADING_RIGHT,

	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_TRANSTILT_INC,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_TRANSTILT_DEC,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_VIEWTILT_INC,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_VIEWTILT_DEC,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_DIST_INC,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_DIST_DEC,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_FOV_INC,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_FOV_DEC,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_HEIGHT_INC,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_CAMERA_HEIGHT_DEC,

	BUTTON_HIT | BUTTON_DEBUG, //INPUT_FUNCTION_MENU_SERVERQUICKSTART,
   BUTTON_HIT | BUTTON_DEBUG, //INPUT_FUNCTION_MENU_CLIENTQUICKSTART,
	BUTTON_HIT,						//INPUT_FUNCTION_MAKE_SCREEN_SHOT,
	BUTTON_HIT | BUTTON_DEBUG, //INPUT_FUNCTION_TOGGLE_MOVIE_CAPTURE,

	BUTTON_HIT | BUTTON_DEBUG, //INPUT_FUNCTION_DEBUG_SINGLE_STEP,
	BUTTON_HIT | BUTTON_DEBUG, //INPUT_FUNCTION_DEBUG_SINGLE_STEP_STEP,
	BUTTON_HELD,					//INPUT_FUNCTION_DEBUG_RAPID_MOVE,
	BUTTON_HIT | BUTTON_DEBUG, //INPUT_FUNCTION_DEBUG_GENERIC0,
	BUTTON_HIT | BUTTON_DEBUG, //INPUT_FUNCTION_DEBUG_GENERIC1,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_DEBUG_FAR_CLIP_IN,
	BUTTON_HELD | BUTTON_DEBUG, //INPUT_FUNCTION_DEBUG_FAR_CLIP_OUT,
   BUTTON_HIT | BUTTON_DEBUG, //INPUT_FUNCTION_QUICK_FULL_EXIT,
	BUTTON_HIT | BUTTON_DEBUG, //INPUT_FUNCTION_VIS_UPDATE,
	BUTTON_HIT,						//INPUT_FUNCTION_TOGGLE_SNAP_SHOT_MODE,
	BUTTON_HIT,						//INPUT_FUNCTION_SNAP_SHOT_ADVANCE,
	BUTTON_HIT,						//INPUT_FUNCTION_CNC,
	BUTTON_HIT,						//INPUT_FUNCTION_QUICKSAVE,

	// Menu
	BUTTON_HIT, //INPUT_FUNCTION_MENU_TOGGLE,

	// In game EVA
	BUTTON_HIT, //INPUT_FUNCTION_EVA_MISSION_OBJECTIVES_TOGGLE,

	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_01
	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_02
	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_03
	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_04
	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_05
	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_06
	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_07
	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_08
	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_09
	BUTTON_HIT | BUTTON_CTRL, // INPUT_FUNCTION_CNC_RADIO_CMD_10

	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_11,
	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_12,
	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_13,
	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_14,
	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_15,
	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_16,
	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_17,
	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_18,
	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_19,
	BUTTON_HIT | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_20,

	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_21,
	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_22,
	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_23,
	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_24,
	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_25,
	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_26,
	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_27,
	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_28,
	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_29,
	BUTTON_HIT | BUTTON_CTRL | BUTTON_ALT, // INPUT_FUNCTION_RADIO_CMD_30,

	BUTTON_HIT, //INPUT_FUNCTION_TEAM_INFO_TOGGLE,
	BUTTON_HIT, //INPUT_FUNCTION_BATTLE_INFO_TOGGLE,
	BUTTON_HIT, //INPUT_FUNCTION_SERVER_INFO_TOGGLE

	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD0,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD1,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD2,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD3,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD4,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD5,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD6,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD7,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD8,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_CHILD9,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_ENTER_PARENT,
	BUTTON_HIT, //INPUT_FUNCTION_PROFILE_RESET,
};


/*
**
*/
class	AcceleratedKeyDef {
public:
	float	Value;
	short	Function;
	float	Min;
	float	Max;
	float	Acceleration;
};


/*
**
*/
float	Input::Sliders[ NUM_SLIDERS ];
bool	Input::ConsoleMode = false;
bool	Input::MenuMode = false;
int	Input::Queue[ Input::QUEUE_MAX ];
int	Input::QueueHead;
int	Input::QueueTail;
int	Input::QueueSize;
float	Input::FunctionValue[ INPUT_FUNCTION_COUNT ];
float	Input::FunctionClamp[ INPUT_FUNCTION_COUNT ];
int	Input::FunctionPrimaryKeys[ INPUT_FUNCTION_COUNT ];
int	Input::FunctionSecondaryKeys[ INPUT_FUNCTION_COUNT ];
bool	Input::DamageIndicatorsEnabled = true;
bool	Input::UsingDirectInput = true;

SimpleDynVecClass<AcceleratedKeyDef>	AcceleratedKeyList;

int	_StatsHelpScreen;
int	_StatsObjectives;
int	_StatsMap;
int	_StatsMenu;


/*
**
*/
void	Input::Init(bool use_dinput)
{
	ConsoleMode	= false;
	MenuMode		= false;
	if (use_dinput) {
		DirectInput::Init();
	}
	UsingDirectInput = use_dinput;

	_StatsHelpScreen = 0;
	_StatsObjectives = 0;
	_StatsMap = 0;
	_StatsMenu = 0;

	memset( FunctionValue, 0, sizeof( FunctionValue ) );

	return ;
}

void	Input::Shutdown( void )
{
	Free_Mappings();
	if (UsingDirectInput) {
		DirectInput::Shutdown();
	}

	Debug_Say(( "Help Screen Hits: %d\n", _StatsHelpScreen ));
	Debug_Say(( " Objectives Hits: %d\n", _StatsObjectives ));
	Debug_Say(( " Map Screen Hits: %d\n", _StatsMap ));
	Debug_Say(( "Menu Screen Hits: %d\n", _StatsMenu ));

	return ;
}


void	Input::Free_Mappings( void )
{
	//
	//	Simply reset the key arrays
	//
	::memset (FunctionPrimaryKeys, 0, sizeof (FunctionPrimaryKeys));
	::memset (FunctionSecondaryKeys, 0, sizeof (FunctionSecondaryKeys));

	//
	//	Free the accelerated key list
	//
	AcceleratedKeyList.Delete_All ();
	return;
}


void	Input::Load_Registry( const char * /* key */ )
{
	/*RegistryClass * registry = new RegistryClass( key );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {
		MouseSensitivity	= registry->Get_Float( "MouseSensitivity",	MouseSensitivity );
		MouseScale			= registry->Get_Float( "MouseScale",	MouseScale );
		MouseInvert			= registry->Get_Bool( "MouseInvert",	MouseInvert );
		Mouse2DInvert		= registry->Get_Bool( "Mouse2DInvert",	Mouse2DInvert );
	}
	delete registry;*/

	//Input::Set_Mouse_Sensitivity( MouseSensitivity );

}

void	Input::Save_Registry( const char * /* key */ )
{
	/*RegistryClass * registry = new RegistryClass( key );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {
		registry->Set_Float( "MouseSensitivity",	MouseSensitivity );
		registry->Set_Float( "MouseScale",	MouseScale );
		registry->Set_Bool( "MouseInvert",	MouseInvert );
		registry->Set_Bool( "Mouse2DInvert",	Mouse2DInvert );
	}
	delete registry;*/
}



float	Input::Get_Mouse_Sensitivity( void )
{
	return MouseSensitivity;
}

void	Input::Set_Mouse_Sensitivity( float mouse_sensitivity )
{
	// save this in the registry...
	MouseSensitivity = WWMath::Clamp( mouse_sensitivity, 0, 1 );

	/*
	When MouseSensitivity == 0, set MouseScale to 0.00025
	When MouseSensitivity == 1, set MouseScale to 0.025
	When MouseSensitivity == x, set MouseScale to 25 * 10^(2x-5)
	*/

	MouseScale = 25.0f * pow( 10.0f, 2*MouseSensitivity - 5 );
}

bool	Input::Get_Mouse_Invert( void )
{
	return MouseInvert;
}

void	Input::Set_Mouse_Invert( bool invert )
{
	// save this in the registry...
	MouseInvert = invert;
}

bool	Input::Get_Mouse_2D_Invert( void )
{
	return Mouse2DInvert;
}

void	Input::Set_Mouse_2D_Invert( bool invert )
{
	// save this in the registry...
	Mouse2DInvert = invert;
}

void	Input::Flush( void )
{
	if (UsingDirectInput) {
		DirectInput::Flush();
	}
}


void	Input::Update_Sliders( void )
{
	if (!UsingDirectInput) {
		return;
	}
	float wheel_scale = -0.05f;

	//
	//	Update mouse sliders
	//
	float	mouse_x = MouseScale * (float)(DirectInput::Get_Mouse_Axis( DirectInput::MOUSE_X_AXIS ));
	float	mouse_y = MouseScale * (float)(DirectInput::Get_Mouse_Axis( DirectInput::MOUSE_Y_AXIS ));
	float	mouse_z = wheel_scale * (float)(DirectInput::Get_Mouse_Axis( DirectInput::MOUSE_Z_AXIS ));

	if (TimeManager::Get_Frame_Real_Seconds() > 0.0f) {
		mouse_x /= TimeManager::Get_Frame_Real_Seconds();
		mouse_y /= TimeManager::Get_Frame_Real_Seconds();
	} else {
		mouse_x = 0.0f;
		mouse_y = 0.0f;
		mouse_z = 0.0f;
	}

	WWASSERT(WWMath::Is_Valid_Float(mouse_x));
	WWASSERT(WWMath::Is_Valid_Float(mouse_y));

	// it comes in as inverted
	if ( !MouseInvert ) {
		mouse_y = -mouse_y;
	}

//	Debug_Say(( "Mouse %f %f\n", mouse_x, mouse_y ));

	Sliders[ SLIDER_MOUSE_LEFT - FIRST_SLIDER ]				= MAX( -mouse_x , 0.0f );
	Sliders[ SLIDER_MOUSE_RIGHT - FIRST_SLIDER ]				= MAX(  mouse_x , 0.0f );
	Sliders[ SLIDER_MOUSE_UP - FIRST_SLIDER ]					= MAX( -mouse_y , 0.0f );
	Sliders[ SLIDER_MOUSE_DOWN - FIRST_SLIDER ]				= MAX(  mouse_y , 0.0f );
	Sliders[ SLIDER_MOUSE_WHEEL_FORWARD - FIRST_SLIDER ]	= MAX( -mouse_z , 0.0f );
	Sliders[ SLIDER_MOUSE_WHEEL_BACKWARD - FIRST_SLIDER ]	= MAX(  mouse_z , 0.0f );

	//
	// Update joystick sliders
	//
	float	joystick_x = (float)DirectInput::Get_Joystick_Axis_State( DirectInput::JOYSTICK_X_AXIS ) / 1000.0f;
	float	joystick_y = (float)DirectInput::Get_Joystick_Axis_State( DirectInput::JOYSTICK_Y_AXIS ) / 1000.0f;

	const float DEAD_ZONE = 0.15f;
	if (WWMath::Fabs(joystick_x) < DEAD_ZONE) {
		joystick_x = 0.0f;
	} else {
		joystick_x = WWMath::Sign(joystick_x) * (WWMath::Fabs(joystick_x) - DEAD_ZONE) * 1.5f / (1.0f - DEAD_ZONE);
	}

	Sliders[ SLIDER_JOYSTICK_LEFT - FIRST_SLIDER ]	= MAX( -joystick_x , 0.0f );
	Sliders[ SLIDER_JOYSTICK_RIGHT - FIRST_SLIDER ]	= MAX(  joystick_x , 0.0f );
	Sliders[ SLIDER_JOYSTICK_UP - FIRST_SLIDER ]	= MAX( -joystick_y , 0.0f );
	Sliders[ SLIDER_JOYSTICK_DOWN - FIRST_SLIDER ]	= MAX(  joystick_y , 0.0f );
	return ;
}


void	Input::Update( void )
{
	if (!UsingDirectInput) {
		return;
	}

	if (!GameInFocus) {
		DirectInput::Unacquire();
		return;
	}

	// Update Direct Input
	DirectInput::Read();
	Update_Sliders ();

	//
	// zero all values
	//
	memset( FunctionValue, 0, sizeof( FunctionValue ) );

	// No ESC, O, M, F1, etc in cinematics!
	if ( COMBAT_CAMERA && COMBAT_CAMERA->Is_In_Cinematic() && !DebugManager::Allow_Cinematic_Keys() ) {
		return;
	}

	//
	//	Don't process function inputs if we're in the menu
	//
	if ( MenuMode == false || ConsoleMode ) {

		//
		//	Update the value of each function based on what keys are down
		//
		int index;
		for (index = 0; index < INPUT_FUNCTION_COUNT; index ++) {

			float value1 = Get_Value (index, FunctionPrimaryKeys[index], 1.0F);
			float value2 = Get_Value (index, FunctionSecondaryKeys[index], 1.0F);

			//
			//	Special case the enter key.  If the enter key isn't hit, then
			// check for the num-pad enter key
			//
			if (value1 == 0.0F && value2 == 0.0F) {
				if (	FunctionPrimaryKeys[index] == DIK_RETURN ||
						FunctionSecondaryKeys[index] == DIK_RETURN)
				{
					value1 = Get_Value (index, DIK_NUMPADENTER, 1.0F);
				}
			}

			FunctionValue[index] = std::max (value1, value2);
		}

		_StatsHelpScreen += Get_State( INPUT_FUNCTION_HELP_SCREEN ) ? 1 : 0;
		_StatsObjectives += Get_State( INPUT_FUNCTION_EVA_OBJECTIVES_SCREEN ) ? 1 : 0;
		_StatsMap += Get_State( INPUT_FUNCTION_EVA_MAP_SCREEN ) ? 1 : 0;
		_StatsMenu += Get_State( INPUT_FUNCTION_MENU_TOGGLE ) ? 1 : 0;

		//
		//	Special case sniping mode.  In sniping mode we make the primary zoom-in and zoom-out
		// keys exclusive.
		//
		if (COMBAT_CAMERA->Is_Star_Sniping ()) {

			int key1 = FunctionPrimaryKeys[INPUT_FUNCTION_ZOOM_IN];
			int key2 = FunctionPrimaryKeys[INPUT_FUNCTION_ZOOM_OUT];

			//
			//	Loop over and zero out any function that uses either of these primary keys
			//
			for (index = 0; index < INPUT_FUNCTION_COUNT; index ++) {

				//
				//	Does this function use one of the keys that is mapped to the zoom functions?
				//
				if (	FunctionPrimaryKeys[index] == key1 || FunctionSecondaryKeys[index] == key1 ||
						FunctionPrimaryKeys[index] == key2 || FunctionSecondaryKeys[index] == key2)
				{
					//
					//	Zero out this function as necessary
					//
					if (index != INPUT_FUNCTION_ZOOM_IN && index != INPUT_FUNCTION_ZOOM_OUT) {
						FunctionValue[index] = 0;
					}
				}
			}
		}

		//
		//	Special case the vehicle turning so it inherits the values
		// of the strafe command
		//
		FunctionValue[INPUT_FUNCTION_VEHICLE_TURN_LEFT]		= std::max (FunctionValue[INPUT_FUNCTION_MOVE_LEFT], FunctionValue[INPUT_FUNCTION_TURN_LEFT]);
		FunctionValue[INPUT_FUNCTION_VEHICLE_TURN_RIGHT]	= std::max (FunctionValue[INPUT_FUNCTION_MOVE_RIGHT], FunctionValue[INPUT_FUNCTION_TURN_RIGHT]);

		//
		// Apply accelerated keys
		//
		for (index = 0; index < AcceleratedKeyList.Count (); index ++) {
			AcceleratedKeyDef &def = AcceleratedKeyList[index];

			//
			//	Is this fuction activated?
			//
			if (	Get_Value (def.Function, FunctionPrimaryKeys[def.Function], 1.0F) ||
					Get_Value (def.Function, FunctionSecondaryKeys[def.Function], 1.0F))
			{
				//
				//	Apply the accelerated value
				//
				def.Value += def.Acceleration * TimeManager::Get_Frame_Real_Seconds ();
				def.Value = WWMath::Clamp (def.Value, def.Min, def.Max);

				//
				//	Store the function's value (if necessary)
				//
				if (def.Value != 0.0F) {
					FunctionValue[def.Function] = def.Value;
				}

			} else {

				//
				//	Reset the function's accelerated value
				//
				def.Value = 0.0F;
			}
		}
	} else {

		int index = INPUT_FUNCTION_BEGIN_CONSOLE;
		float value1 = Get_Value (index, FunctionPrimaryKeys[index], 1.0F);
		float value2 = Get_Value (index, FunctionSecondaryKeys[index], 1.0F);

		FunctionValue[index] = std::max (value1, value2);
	}

	return ;
}

int	Input::Console_Get_Key()
{
	if ( QueueSize > 0 ) {
		int key = Queue[ QueueTail ];
		if ( ++QueueTail == QUEUE_MAX ) {
			QueueTail = 0;
		}
		QueueSize--;
		return key;
	}
	return 0;
}

void	Input::Console_Add_Key( int key )
{
	if ( QueueSize < QUEUE_MAX ) {
		Queue[ QueueHead ] = key;
		if ( ++QueueHead == QUEUE_MAX ) {
			QueueHead = 0;
		}
		QueueSize++;
	}
}


/*
**
*/
bool	Input::Is_Button_Down (int button_id)
{
	if (!UsingDirectInput) {
		return(false);
	}

	return ((DirectInput::Get_Button_Value(button_id) & BUTTON_BIT_HELD) != 0);
}


/*
**
*/
float	Input::Get_Value( int function_index, int input, float /* clamp */ )
{
	if (!UsingDirectInput) {
		return(0.0f);
	}

	if ( input >= FIRST_SLIDER ) {			// return slider state

		float slider = Sliders[ input - FIRST_SLIDER ];

//
// ignore the dead zone
#if 0
		if ( slider < SLIDER_DEAD_ZONE ) {
			slider = 0;
		}

// Ignore the clamp
		if ( slider > clamp ) {
			slider = clamp;
		}
#endif

//		if ( input == SLIDER_MOUSE_LEFT ) 	Debug_Say(( "Slider %f\n", slider ));

		return slider;

	} else {
#if(0)
		//
		//	Bail if this is a debug key and the debug button isn't down
		//
		if ((FunctionKeyStates[function_index] & 0xF000) & BUTTON_DEBUG) {
			if ((DirectInput::Get_Button_Value(DIK_F9) & BUTTON_BIT_HELD) == 0) {
				return 0.0F;
			}
		} else if ((DirectInput::Get_Button_Value(DIK_F9) & BUTTON_BIT_HELD)) {
			return 0.0F;
		}
#endif

		int modifier = 0;

		// Special case ctrl and alt for the 0 - 9 keys
		// Assumtion:  DIK_keys go 1,2,3..9,0
		if ( (input&0xFF) >= DIK_1 && (input&0xFF) <= DIK_0 ) {
			// Shift is a key, used for walk!
//			modifier |= ((DirectInput::Get_Button_Value(DIK_SHIFT) & BUTTON_BIT_HELD) ? BUTTON_SHIFT : 0);
			modifier |= ((DirectInput::Get_Button_Value(DIK_CONTROL) & BUTTON_BIT_HELD) ? BUTTON_CTRL : 0);
			modifier |= ((DirectInput::Get_Button_Value(DIK_ALT) & BUTTON_BIT_HELD) ? BUTTON_ALT : 0);
		}

		modifier |= ((DirectInput::Get_Button_Value(DIK_F9) & BUTTON_BIT_HELD) ? BUTTON_DEBUG : 0);

		int funcModifier = (FunctionKeyStates[function_index] & 0xF000);

		if (funcModifier != modifier) {
			return 0.0f;
		}

		switch( FunctionKeyStates[ function_index ] & 0x0F ) {
			case BUTTON_UP:
				return (DirectInput::Get_Button_Value( input ) & BUTTON_BIT_HELD) ? 0.0 : 1.0;

			case BUTTON_HIT:
				return (DirectInput::Get_Button_Value( input ) & BUTTON_BIT_HIT) ? 1.0 : 0.0;

			case BUTTON_RELEASE:
				return (DirectInput::Get_Button_Value( input ) & BUTTON_BIT_RELEASED) ? 1.0 : 0.0;

			case BUTTON_HELD:
				return (DirectInput::Get_Button_Value( input ) & BUTTON_BIT_HELD) ? 1.0 : 0.0;

			case BUTTON_DOUBLE:
				return (DirectInput::Get_Button_Value( input ) & BUTTON_BIT_DOUBLE) ? 1.0 : 0.0;
		}
	}

	return 0.0f;
}


/*
**
*/
short	Input::Get_Function( const char *name )
{
	if ( name && name[0] ) {
		// find function
		for ( int function = 0; function < NUM_FUNCTIONS; function++ ) {
			if ( !stricmp( name, Functions[function].Name ) ) {
				return Functions[function].ID;
			}
		}
		Debug_Say(( "Unknown function name %s\n", name ));
	}
	return 0;
}


/*
**
*/
const char *Input::Get_Key_Name( short key_id )
{
	//
	// Check each button name
	//
	int index;
	for (index = 0; index < NUM_BUTTON_NAMES; index ++) {
		if (ButtonNames[index].ID == key_id) {
			return ButtonNames[index].Name;
		}
	}

	//
	// Check each slider name
	//
	for (index = 0; index < NUM_SLIDER_NAMES; index ++) {
		if (SliderNames[index].ID == key_id) {
			return SliderNames[index].Name;
		}
	}

	Debug_Say(( "Could not find a name for key %d\n", key_id ));
	return NULL;
}

/*
**
*/
short	Input::Get_Key( const char *name )
{
	if (name != NULL && name[0] != 0) {

		//
		// Check each button name
		//
		int index;
		for (index = 0; index < NUM_BUTTON_NAMES; index ++) {
			if (::stricmp (name, ButtonNames[index].Name) == 0) {
				return ButtonNames[index].ID;
			}
		}

		//
		// Check each slider name
		//
		for (index = 0; index < NUM_SLIDER_NAMES; index ++) {
			if (::stricmp (name, SliderNames[index].Name) == 0) {
				return SliderNames[index].ID;
			}
		}

		Debug_Say(( "Could not find key name %s\n", name ));
	}

	return 0;
}


/*
**
*/
int
Input::Get_Primary_Key_For_Function (int function_id)
{
	return FunctionPrimaryKeys[ function_id ];
}


/*
**
*/
int
Input::Get_Secondary_Key_For_Function (int function_id)
{
	return FunctionSecondaryKeys[ function_id ];
}


/*
**
*/
void
Input::Set_Primary_Key_For_Function (int function_id, int key_id)
{
	FunctionPrimaryKeys[ function_id ] = key_id;

	// Use weapon key is the same as secondary fire, just hit rather than held
	if ( function_id == INPUT_FUNCTION_FIRE_WEAPON_SECONDARY ) {
		Set_Primary_Key_For_Function ( INPUT_FUNCTION_USE_WEAPON, key_id );
	}

	// Copy jump and crouch to move up and down
	if ( function_id == INPUT_FUNCTION_CROUCH ) {
		Set_Primary_Key_For_Function ( INPUT_FUNCTION_MOVE_DOWN, key_id );
	}
	if ( function_id == INPUT_FUNCTION_JUMP ) {
		Set_Primary_Key_For_Function ( INPUT_FUNCTION_MOVE_UP, key_id );
	}


	return ;
}


/*
**
*/
void
Input::Set_Secondary_Key_For_Function (int function_id, int key_id)
{
	FunctionSecondaryKeys[ function_id ] = key_id;

	// Use weapon key is the same as secondary fire, just hit rather than held
	if ( function_id == INPUT_FUNCTION_FIRE_WEAPON_SECONDARY ) {
		Set_Secondary_Key_For_Function ( INPUT_FUNCTION_USE_WEAPON, key_id );
	}

	// Copy jump and crouch to move up and down
	if ( function_id == INPUT_FUNCTION_CROUCH ) {
		Set_Secondary_Key_For_Function ( INPUT_FUNCTION_MOVE_DOWN, key_id );
	}
	if ( function_id == INPUT_FUNCTION_JUMP ) {
		Set_Secondary_Key_For_Function ( INPUT_FUNCTION_MOVE_UP, key_id );
	}

	return ;
}


/*
**
*/
void
Input::Save_Configuration (const char *filename)
{
	INIClass	input_ini;

	//
	//	Loop over each function and save its key mappings to the INI file
	//
	for (int index = 0; index < NUM_FUNCTIONS; index ++) {
		StringClass pri_name;
		StringClass sec_name;
		pri_name.Format ("%s_Primary",	Functions[index].Name);
		sec_name.Format ("%s_Secondary",	Functions[index].Name);

		StringClass pri_key;
		StringClass sec_key;

		//
		//	Get the name of the primary key that's mapped to this function
		//
		if (FunctionPrimaryKeys[index] != 0) {
			pri_key = Get_Key_Name (FunctionPrimaryKeys[index]);
		}

		//
		//	Get the name of the secondary  key that's mapped to this function
		//
		if (FunctionSecondaryKeys[index] != 0) {
			sec_key = Get_Key_Name (FunctionSecondaryKeys[index]);
		}

		//
		//	Read the primary and secondary keys for this function
		//
		input_ini.Put_String ("Generic Key Mappings", pri_name, pri_key);
		input_ini.Put_String ("Generic Key Mappings", sec_name, sec_key);
	}

	//
	//	Save the accelerated functions
	//
	Save_Accelerated_Keys (&input_ini);
	Save_Misc_Settings (&input_ini);

	//
	//	Save the data to a file
	//
	StringClass	config_filename;
	config_filename.Format( "config\\%s", filename );
	FileClass *ini_file = _TheWritingFileFactory->Get_File (config_filename);
	if (ini_file != NULL) {
		ini_file->Open (FileClass::WRITE);
		input_ini.Save (*ini_file);
		_TheWritingFileFactory->Return_File (ini_file);
	}

	return ;
}

/*
**
*/
typedef struct
{
	int text_id;
	int dik_id;
}	KEY_NAME_MAPPING;



const KEY_NAME_MAPPING DIK_KEY_NAME_ARRAY[] =
{
	{	IDS_KEYNAME_DIK_F1,			DIK_F1 },
	{	IDS_KEYNAME_DIK_F2,			DIK_F2 },
	{	IDS_KEYNAME_DIK_F3,			DIK_F3 },
	{	IDS_KEYNAME_DIK_F4,			DIK_F4 },
	{	IDS_KEYNAME_DIK_F5,			DIK_F5 },
	{	IDS_KEYNAME_DIK_F6,			DIK_F6 },
	{	IDS_KEYNAME_DIK_F7,			DIK_F7 },
	{	IDS_KEYNAME_DIK_F8,			DIK_F8 },
	{	IDS_KEYNAME_DIK_F9,			DIK_F9 },
	{	IDS_KEYNAME_DIK_F10,			DIK_F10 },
	{	IDS_KEYNAME_DIK_F11,			DIK_F11 },
	{	IDS_KEYNAME_DIK_F12,			DIK_F12 },
	{	IDS_KEYNAME_DIK_0,			DIK_0 },
	{	IDS_KEYNAME_DIK_1,			DIK_1 },
	{	IDS_KEYNAME_DIK_2,			DIK_2 },
	{	IDS_KEYNAME_DIK_3,			DIK_3 },
	{	IDS_KEYNAME_DIK_4,			DIK_4 },
	{	IDS_KEYNAME_DIK_5,			DIK_5 },
	{	IDS_KEYNAME_DIK_6,			DIK_6 },
	{	IDS_KEYNAME_DIK_7,			DIK_7 },
	{	IDS_KEYNAME_DIK_8,			DIK_8 },
	{	IDS_KEYNAME_DIK_9,			DIK_9 },
	{	IDS_KEYNAME_DIK_A,			DIK_A },
	{	IDS_KEYNAME_DIK_B,			DIK_B },
	{	IDS_KEYNAME_DIK_C,			DIK_C },
	{	IDS_KEYNAME_DIK_D,			DIK_D },
	{	IDS_KEYNAME_DIK_E,			DIK_E },
	{	IDS_KEYNAME_DIK_F,			DIK_F },
	{	IDS_KEYNAME_DIK_G,			DIK_G },
	{	IDS_KEYNAME_DIK_H,			DIK_H },
	{	IDS_KEYNAME_DIK_I,			DIK_I },
	{	IDS_KEYNAME_DIK_J,			DIK_J },
	{	IDS_KEYNAME_DIK_K,			DIK_K },
	{	IDS_KEYNAME_DIK_L,			DIK_L },
	{	IDS_KEYNAME_DIK_M,			DIK_M },
	{	IDS_KEYNAME_DIK_N,			DIK_N },
	{	IDS_KEYNAME_DIK_O,			DIK_O },
	{	IDS_KEYNAME_DIK_P,			DIK_P },
	{	IDS_KEYNAME_DIK_Q,			DIK_Q },
	{	IDS_KEYNAME_DIK_R,			DIK_R },
	{	IDS_KEYNAME_DIK_S,			DIK_S },
	{	IDS_KEYNAME_DIK_T,			DIK_T },
	{	IDS_KEYNAME_DIK_U,			DIK_U },
	{	IDS_KEYNAME_DIK_V,			DIK_V },
	{	IDS_KEYNAME_DIK_W,			DIK_W },
	{	IDS_KEYNAME_DIK_X,			DIK_X },
	{	IDS_KEYNAME_DIK_Y,			DIK_Y },
	{	IDS_KEYNAME_DIK_Z,			DIK_Z },
	{	IDS_KEYNAME_DIK_MINUS,		DIK_MINUS },
	{	IDS_KEYNAME_DIK_EQUALS,		DIK_EQUALS },
	{	IDS_KEYNAME_DIK_BACK,		DIK_BACK },
	{	IDS_KEYNAME_DIK_TAB,			DIK_TAB },
	{	IDS_KEYNAME_DIK_LBRACKET,	DIK_LBRACKET },
	{	IDS_KEYNAME_DIK_RBRACKET,	DIK_RBRACKET },
	{	IDS_KEYNAME_DIK_RETURN,		DIK_RETURN },
	{	IDS_KEYNAME_DIK_SEMICOLON,	DIK_SEMICOLON },
	{	IDS_KEYNAME_DIK_APOSTROPHE,DIK_APOSTROPHE },
	{	IDS_KEYNAME_DIK_GRAVE,		DIK_GRAVE },
	{	IDS_KEYNAME_DIK_BACKSLASH,	DIK_BACKSLASH },
	{	IDS_KEYNAME_DIK_COMMA,		DIK_COMMA },
	{	IDS_KEYNAME_DIK_PERIOD,		DIK_PERIOD },
	{	IDS_KEYNAME_DIK_SLASH,		DIK_SLASH },
	{	IDS_KEYNAME_DIK_SPACE,		DIK_SPACE },
	{	IDS_KEYNAME_DIK_CAPITAL,	DIK_CAPITAL },
	{	IDS_KEYNAME_DIK_NUMLOCK,	DIK_NUMLOCK },
	{	IDS_KEYNAME_DIK_SCROLL,		DIK_SCROLL },
	{	IDS_KEYNAME_DIK_ESCAPE,		DIK_ESCAPE },
	{	IDS_KEYNAME_DIK_NUMPAD0,	DIK_NUMPAD0 },
	{	IDS_KEYNAME_DIK_NUMPAD1,	DIK_NUMPAD1 },
	{	IDS_KEYNAME_DIK_NUMPAD2,	DIK_NUMPAD2 },
	{	IDS_KEYNAME_DIK_NUMPAD3,	DIK_NUMPAD3 },
	{	IDS_KEYNAME_DIK_NUMPAD4,	DIK_NUMPAD4 },
	{	IDS_KEYNAME_DIK_NUMPAD5,	DIK_NUMPAD5 },
	{	IDS_KEYNAME_DIK_NUMPAD6,	DIK_NUMPAD6 },
	{	IDS_KEYNAME_DIK_NUMPAD7,	DIK_NUMPAD7 },
	{	IDS_KEYNAME_DIK_NUMPAD8,	DIK_NUMPAD8 },
	{	IDS_KEYNAME_DIK_NUMPAD9,	DIK_NUMPAD9 },
	{	IDS_KEYNAME_DIK_SUBTRACT,	DIK_SUBTRACT },
	{	IDS_KEYNAME_DIK_MULTIPLY,	DIK_MULTIPLY },
	{	IDS_KEYNAME_DIK_ADD,			DIK_ADD },
	{	IDS_KEYNAME_DIK_DECIMAL,		DIK_DECIMAL },
	{	IDS_KEYNAME_DIK_NUMPADENTER,	DIK_NUMPADENTER },
	{	IDS_KEYNAME_DIK_DIVIDE,		DIK_DIVIDE },
	{	IDS_KEYNAME_DIK_HOME,		DIK_HOME },
	{	IDS_KEYNAME_DIK_PRIOR,		DIK_PRIOR },
	{	IDS_KEYNAME_DIK_END,			DIK_END },
	{	IDS_KEYNAME_DIK_NEXT,		DIK_NEXT },
	{	IDS_KEYNAME_DIK_INSERT,		DIK_INSERT },
	{	IDS_KEYNAME_DIK_DELETE,		DIK_DELETE },
	{	IDS_KEYNAME_DIK_UP,			DIK_UP },
	{	IDS_KEYNAME_DIK_DOWN,		DIK_DOWN },
	{	IDS_KEYNAME_DIK_LEFT,		DIK_LEFT },
	{	IDS_KEYNAME_DIK_RIGHT,		DIK_RIGHT },
	{	IDS_KEYNAME_DIK_SYSRQ,		DIK_SYSRQ },
	{	IDS_KEYNAME_DIK_CONTROL,	DIK_CONTROL },
	{	IDS_KEYNAME_DIK_LCONTROL,	DIK_LCONTROL },
	{	IDS_KEYNAME_DIK_RCONTROL,	DIK_RCONTROL },
	{	IDS_KEYNAME_DIK_SHIFT,		DIK_SHIFT },
	{	IDS_KEYNAME_DIK_LSHIFT,		DIK_LSHIFT },
	{	IDS_KEYNAME_DIK_RSHIFT,		DIK_RSHIFT },
	{	IDS_KEYNAME_DIK_ALT,			DIK_ALT },
	{	IDS_KEYNAME_DIK_LALT,		DIK_LALT },
	{	IDS_KEYNAME_DIK_RALT,		DIK_RALT },
	{	IDS_KEYNAME_DIK_WIN,			DIK_WIN },
	{	IDS_KEYNAME_DIK_LWIN,		DIK_LWIN },
	{	IDS_KEYNAME_DIK_RWIN,		DIK_RWIN },
	{	IDS_KEYNAME_DIK_APPS,		DIK_APPS },
	{	IDS_INPUT_LBUTTON,			DirectInput::BUTTON_MOUSE_LEFT },
	{	IDS_INPUT_RBUTTON,			DirectInput::BUTTON_MOUSE_RIGHT },
	{	IDS_INPUT_MBUTTON,			DirectInput::BUTTON_MOUSE_CENTER },
	{	IDS_INPUT_MW_UP,				Input::SLIDER_MOUSE_WHEEL_FORWARD },
	{	IDS_INPUT_MW_DN,				Input::SLIDER_MOUSE_WHEEL_BACKWARD },
	{	0,									DirectInput::BUTTON_JOYSTICK_A },
	{	0,									DirectInput::BUTTON_JOYSTICK_B },
	{	0,									Input::SLIDER_MOUSE_LEFT },
	{	0,									Input::SLIDER_MOUSE_RIGHT },
	{	0,									Input::SLIDER_MOUSE_UP },
	{	0,									Input::SLIDER_MOUSE_DOWN },
	{	0,									Input::SLIDER_JOYSTICK_LEFT },
	{	0,									Input::SLIDER_JOYSTICK_RIGHT },
	{	0,									Input::SLIDER_JOYSTICK_UP },
	{	0,									Input::SLIDER_JOYSTICK_DOWN },
};

const int KEYNAME_MAP_COUNT	= sizeof (DIK_KEY_NAME_ARRAY) / sizeof (KEY_NAME_MAPPING);


/*
**
*/
void
Input::Load_Configuration (const char *filename)
{
	Free_Mappings ();

	//
	//	Try to load the INI file
	//
	INIClass	*input_ini = Get_INI (filename);
	if (input_ini == NULL) {
		Debug_Say(("Input::Load_Configuration - Unable to load %s\n", filename));
		return ;
	}

	//
	//	Loop over each function and try to read data about it from the INI
	//
	for (int index = 0; index < NUM_FUNCTIONS; index ++) {
		StringClass pri_name;
		StringClass sec_name;
		pri_name.Format ("%s_Primary",	Functions[index].Name);
		sec_name.Format ("%s_Secondary",	Functions[index].Name);

		//
		//	Read the primary and secondary keys for this function
		//
		StringClass pri_key(0,true);
		input_ini->Get_String (pri_key,"Generic Key Mappings", pri_name);
		StringClass sec_key(0,true);
		input_ini->Get_String (sec_key,"Generic Key Mappings", sec_name);

		//
		//	Set the primary key for this function
		//
		if (pri_key.Is_Empty () == false) {
			FunctionPrimaryKeys[index] = Get_Key (pri_key);
		}

		//
		//	Set the secondary key for this function
		//
		if (pri_key.Is_Empty () == false) {
			FunctionSecondaryKeys[index] = Get_Key (sec_key);
		}
	}

	//
	//	Load the accelerated keys from the ini
	//
	Load_Accelerated_Keys (input_ini);
	Load_Misc_Settings (input_ini);

	//
	//	Free the INI
	//
	Release_INI (input_ini);

	//
	//	Reset the mouse sensitivity to ensure its clamped properly
	//
	Input::Set_Mouse_Sensitivity (MouseSensitivity);
	return ;
}


/*
**
*/
void
Input::Load_Accelerated_Keys (INIClass	*input_ini)
{
	int count = input_ini->Entry_Count ("Accelerated Keys");

	//
	//	Load information about each accelerated key
	//
	for (int index = 0; index < count; index ++) {
		StringClass entry_name(input_ini->Get_Entry ("Accelerated Keys", index),true);
		StringClass section_name(0,true);
		input_ini->Get_String (section_name, "Accelerated Keys", entry_name);
		Load_Accelerated_Key (input_ini, section_name);
	}

	return ;
}


/*
**
*/
void
Input::Load_Accelerated_Key (INIClass *input_ini, const char *section_name)
{
	StringClass function_name(0,true);
	input_ini->Get_String (function_name, section_name, ENTRY_FUNCTION);

	//
	//	Fill in the structure we use to describe the accelerated key from the INI section.
	//
	AcceleratedKeyDef	def;
	def.Function				= Get_Function (function_name);
	def.Min						= input_ini->Get_Float (section_name, ENTRY_MIN, 0);
	def.Max						= input_ini->Get_Float (section_name, ENTRY_MAX, 0);
	def.Acceleration			= input_ini->Get_Float (section_name, ENTRY_ACCELERATION, 1);

	//
	//	Add this definition to the list
	//
	AcceleratedKeyList.Add (def);
	return ;
}


/*
**
*/
void
Input::Save_Accelerated_Keys (INIClass	*input_ini)
{
	for (int index = 0; index < AcceleratedKeyList.Count (); index ++) {
		AcceleratedKeyDef &def = AcceleratedKeyList[index];

		//
		//	Add an entry for this key to the section
		//
		StringClass entry_name;
		StringClass section_name;
		entry_name.Format ("%d", index + 1);
		section_name.Format ("AcceleratedKey%d", index + 1);
		input_ini->Put_String ("Accelerated Keys", entry_name, section_name);

		//
		//	Save this accelerated key's definition  to its own section
		//
		input_ini->Put_String (section_name,	ENTRY_FUNCTION,		Functions[def.Function].Name);
		input_ini->Put_Float (section_name,		ENTRY_MIN,				def.Min);
		input_ini->Put_Float (section_name,		ENTRY_MAX,				def.Max);
		input_ini->Put_Float (section_name,		ENTRY_ACCELERATION,	def.Acceleration);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Translated_Key_Name
//
////////////////////////////////////////////////////////////////
void
Input::Get_Translated_Key_Name (int dik_id, WideStringClass &name)
{	
	for (int index = 0; index < KEYNAME_MAP_COUNT; index ++) {

		//
		//	Is this the entry we're looking for?
		//	
		if (DIK_KEY_NAME_ARRAY[index].dik_id == dik_id) {
			if (DIK_KEY_NAME_ARRAY[index].text_id != 0) {
				name = TRANSLATE (DIK_KEY_NAME_ARRAY[index].text_id);
			}
			break;
		}
	}

	return ;
}


/*
**
*/
void
Input::Load_Misc_Settings (INIClass *input_ini)
{
	DamageIndicatorsEnabled = input_ini->Get_Bool (SECTION_MISC_SETTINGS, ENTRY_DAMAGE_INDICATORS, true);
	MouseSensitivity			= input_ini->Get_Float (SECTION_MISC_SETTINGS, ENTRY_MOUSE_SENSITIVITY, 0.5F);
	MouseScale					= input_ini->Get_Float (SECTION_MISC_SETTINGS, ENTRY_MOUSE_SCALE, 1.0F);
	MouseInvert					= input_ini->Get_Bool (SECTION_MISC_SETTINGS, ENTRY_MOUSE_INVERT, true);
	Mouse2DInvert				= input_ini->Get_Bool (SECTION_MISC_SETTINGS, ENTRY_MOUSE_2D_INVERT, false);

	bool is_target_steering	= input_ini->Get_Bool (SECTION_MISC_SETTINGS, ENTRY_TARGET_STEERING, false);
	VehicleGameObj::Set_Target_Steering (is_target_steering);

	//
	//	Apply the settings
	//
	Input::Set_Mouse_Sensitivity( MouseSensitivity );
	return ;
}


/*
**
*/
void
Input::Save_Misc_Settings (INIClass	*input_ini)
{
	input_ini->Put_Bool (SECTION_MISC_SETTINGS, ENTRY_DAMAGE_INDICATORS, DamageIndicatorsEnabled);
	input_ini->Put_Float (SECTION_MISC_SETTINGS, ENTRY_MOUSE_SENSITIVITY, MouseSensitivity);
	input_ini->Put_Float (SECTION_MISC_SETTINGS, ENTRY_MOUSE_SCALE, MouseScale);
	input_ini->Put_Bool (SECTION_MISC_SETTINGS, ENTRY_MOUSE_INVERT, MouseInvert);
	input_ini->Put_Bool (SECTION_MISC_SETTINGS, ENTRY_MOUSE_2D_INVERT, Mouse2DInvert);
	input_ini->Put_Bool (SECTION_MISC_SETTINGS, ENTRY_TARGET_STEERING, VehicleGameObj::Is_Target_Steering ());
	return ;
}


/*
**
*/
int
Input::Find_First_Function_By_Primary_Key (int key_id)
{
	return Find_Next_Function_By_Primary_Key (-1, key_id);
}


/*
**
*/
int
Input::Find_Next_Function_By_Primary_Key (int function_id, int key_id)
{
	int retval = -1;

	for (int index = (function_id + 1); index < INPUT_FUNCTION_COUNT; index ++) {
		if (FunctionPrimaryKeys[index] == key_id) {
			retval = index;
			break;
		}
	}

	return retval;
}



/*
**
*/
int
Input::Find_First_Function_By_Secondary_Key (int key_id)
{
	return Find_Next_Function_By_Secondary_Key (-1, key_id);
}


/*
**
*/
int
Input::Find_Next_Function_By_Secondary_Key (int function_id, int key_id)
{
	int retval = -1;

	for (int index = (function_id + 1); index < INPUT_FUNCTION_COUNT; index ++) {
		if (FunctionSecondaryKeys[index] == key_id) {
			retval = index;
			break;
		}
	}

	return retval;
}
