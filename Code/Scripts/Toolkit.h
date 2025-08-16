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

/******************************************************************************
*
* FILE
*     Toolkit.h
*
* DESCRIPTION
*     Designer Common Toolkit for Mission Construction
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Dan_e $
*     $Revision: 79 $
*     $Modtime: 12/07/01 1:18p $
*     $Archive: /Commando/Code/Scripts/Toolkit.h $
*
******************************************************************************/

#ifndef _TOOLKIT_H_
#define _TOOLKIT_H_

// Defines and Includes

#define SCRIPT_DEBUG_MESSAGE( X )

/*
#ifdef WWDEBUG
#define SCRIPT_DEBUG_MESSAGE( X )	if (debug_mode)	{ Commands->Debug_Message X ; }
#else
#define SCRIPT_DEBUG_MESSAGE( X )
#endif
*/


#ifdef WWDEBUG
#define NULL_POINTER_CHECK( X )		(if (X == NULL) { Commands->Debug_Message("***NULL pointer found in script: Line %d of file %s.\n", __LINE__, __FILE__); })
#else
#define NULL_POINTER_CHECK( X )
#endif

#ifdef WWDEBUG
#define DISPLAY_VECTOR3( V )	Commands->Debug_Message("Value of '%s' is (%3.2f, %3.2f, %3.2f) at line %d of file %s.\n", #V, V.X, V.Y, V.Z, __LINE__, __FILE__)
#else
#define DISPLAY_VECTOR3( V )
#endif

#define STAR (Commands->Get_A_Star(Commands->Get_Position(Owner())))

#define DIFFICULTY (Commands->Get_Difficulty_Level())

#define MAX3( a, b, c ) (WWMath::Max(WWMath::Max(a, b), WWMath::Max(b, c)))

#include "CustomEvents.h"
#include "scripts.h"
#include "DPrint.h"
#include "vector3.h"
#include "wwmath.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

#define M00_BROADCASTER_TERMINAL_SIZE_RAD			100
#define M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD	10
#define M00_SIEGE_GROUP_MAXIMUM						10
#define M00_SIEGE_ZONES_MAXIMUM						10


// Innate Priority Levels
#define INNATE_PRIORITY_IDLE						10
#define INNATE_PRIORITY_FOOTSTEPS_HEARD				50 //30
#define INNATE_PRIORITY_CONVERSATION				40
#define INNATE_PRIORITY_BULLET_HEARD				50
#define INNATE_PRIORITY_GUNSHOT_HEARD				50 //70
#define INNATE_PRIORITY_ENEMY_SEEN					90

// Movement Speeds
#define RUN											.8f
#define WALK										.3f
#define CROUCH										.1f

// Secondary Pog Delay
#define	SECONDARY_POG_DELAY							15.0f

//Designer Sound Types

#define M00_SOUND_SIEGE_ZONE						1000
#define M03_SOUND_VOLCANO							1001
#define M00_SOUND_ENGINEER_WANDER					1002
#define M03_SOUND_BUNKER_OCCUPY						1003
#define M00_SOUND_ENGINEER_WANDER_EMERGENCY			1004
#define M00_SOUND_BUILDING_DESTROYED				100000
#define M00_SOUND_ALWAYS_RUN						1005
#define M00_SOUND_ALWAYS_RUN_OFF					1006
#define M00_CUSTOM_CAMERA_ALARM						1007
#define M00_CUSTOM_SAM_SITE_IGNORE					100008

// Toolkit Timer Enumerations

enum
{
	TOOLKIT_TIMER_START = STIMER_TOOLKIT,				// 9900
	M00_TIMER_SOUND_PLAY_2D_RAD,						// 9901
	M00_TIMER_SOUND_PLAY_3D_RAD,						// 9902
	M00_TIMER_SOUND_PLAY_3D_ON_OBJECT_RAD,				// 9903
	M00_TIMER_DROP_OBJECT_RMV,							// 9904
	M00_TIMER_SIEGE_ACTOR_CHECK,						// 9905
	M00_TIMER_TRIGGER_EXPIRED,							// 9906
	M00_TIMER_SIEGE_ZONE								// 9907
};


// Toolkit Custom Enumerations

enum
{
	M00_CUSTOM_TRIGGER_ACTIVATE_ALL = 9000,				// 0
	M00_CUSTOM_TRIGGER_ACTIVATE_KILLED,					// 1
	M00_CUSTOM_TRIGGER_ACTIVATE_DESTROYED,				// 2
	M00_CUSTOM_TRIGGER_ACTIVATE_ZONE_ENTERED,			// 3
	M00_CUSTOM_TRIGGER_ACTIVATE_ZONE_EXITED,			// 4
	M00_CUSTOM_TRIGGER_ACTIVATE_ZONE_ENTERED_OR_EXITED,	// 5
	M00_CUSTOM_TRIGGER_ACTIVATE_ENEMY_SEEN,				// 6
	M00_CUSTOM_TRIGGER_ACTIVATE_DAMAGED,				// 7
	M00_CUSTOM_TRIGGER_ACTIVATE_ACTION_COMPLETE,		// 8
	M00_CUSTOM_TRIGGER_ACTIVATE_ANIMATION_COMPLETE,		// 9

	M00_CUSTOM_BROADCASTER_REGISTRATION,				// 10
	M00_CUSTOM_BROADCASTER_PROMPTER,					// 11
	M00_CUSTOM_BROADCASTER_REGISTRY_ERROR,				// 12
	
	M00_CUSTOM_ANIMATION_DROP_OBJECT,					// 13

	M00_CUSTOM_ACTION_RECEIVE_TYPE_DEFAULT,				// 14

	M00_CUSTOM_TRIGGER_TIMER_EXPIRED,					// 15

	M00_CUSTOM_SIEGE_ACTOR_GROUP_CHECK,					// 16
	M00_CUSTOM_SIEGE_ZONE_VERIFICATION,					// 17
	M00_CUSTOM_SIEGE_ACTOR_IS_DEAD,						// 18
	M00_CUSTOM_SIEGE_ZONE_CHECKING_ACTOR,				// 19
	M00_CUSTOM_SIEGE_ACTOR_RESPONDING,					// 20
	M00_CUSTOM_SIEGE_ACTOR_UNREGISTER,					// 21
	M00_CUSTOM_SIEGE_OBJECT_GROUP_CHANGE,				// 22
	M00_CUSTOM_CINEMATIC_PRIMARY_KILLED,				// 23
	M00_CUSTOM_POWERUP_GRANT_DISABLE,					// 24
	M00_CUSTOM_OBJECT_ATTACHED_PRIMARY,					// 25

	M00_CUSTOM_SOUND_BUILDING,							// 26
	M00_CUSTOM_BUILDING_EXPLODE,						// 27
	M00_CUSTOM_BUILDING_EXPLODE_NO,						// 28
	M00_CUSTOM_BUILDING_EXPLODE_YES,					// 29
	M00_CUSTOM_PLAY_BUILDING_SOUND,						// 30

	M00_CREATE_RANDOM_EXPLOSION,						// 31
	M00_SELECT_EMPTY_HANDS,								// 32
	M00_OBELISK_WEAPON_ID,								// 33
	M00_LAUNCH_SSM,										// 34
	M00_SEND_OBJECT_ID,									// 35
	M00_LOITER_ENABLE_TOGGLE,							// 36
	M00_ENABLE_DAMAGE_MOD,								// 37

	// This type and the next 20+ are used to set a slot in a cinematic script to an ID
	M00_CUSTOM_CINEMATIC_SET_SLOT = 10000,				// 10000
};

inline int Get_Int_Random(int min, int max)
{
	float random;
	int d_random;
	random = Commands->Get_Random(float(min), float(max)+1.0f-WWMATH_EPSILON);
	d_random = int(random);
	d_random = (d_random > max) ? max : d_random;
	return d_random;
}


// Vector 3 Color definitions for use with Text messages
#define TEXT_COLOR_OBJECTIVE_PRIMARY			Vector3 ( .196f, .882f, .196f)
#define TEXT_COLOR_OBJECTIVE_SECONDARY			Vector3 ( .196f, .588f, .980f)
#define TEXT_COLOR_OBJECTIVE_TERTIARY			Vector3 ( .588f, .196f, .588f)
#define TEXT_COLOR_PLAYERTYPE_GDI				Vector3 ( .882f, .686f, .255f)
#define TEXT_COLOR_PLAYERTYPE_NOD				Vector3 ( .784f,  0.0f,  0.0f)
#define TEXT_COLOR_PLAYERTYPE_NEUTRAL			Vector3 ( .882f, .882f, .941f)
#define TEXT_COLOR_PLAYERTYPE_MUTANT			Vector3 (  0.0f, .392f,  0.0f)
#define TEXT_COLOR_WHITE						Vector3 (  1.0f,  1.0f,  1.0f)
#define TEXT_COLOR_YELLOW						Vector3 (  1.0f,  1.0f,  0.0f)


#endif // _TOOLKIT_H_
