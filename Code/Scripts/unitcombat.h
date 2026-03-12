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
 *                     $Archive:: /Commando/Code/Scripts/unitcombat.h                         $*
 *                                                                                             *
 *                      $Author:: David_y                                                     $*
 *                                                                                             *
 *                     $Modtime:: 4/27/00 10:54a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*/

/***********************************************************************************************
*
* Custom Event Type and Param Definitions
*
************************************************************************************************/

// SCRIPT OVERRIDE CUSTOM DEFINITIONS
// OV_CREATED		= Sent by unit to control to let control know unit has been created.
// OV_VERIFIED		= Sent by unit to control to verify that control is currently handling the unit.
// OV_ARRIVED		= Sent by unit to control when the unit arrives at a destination.
// OV_ANIMCOMPLETE	= Sent by unit to control when an animation has been completed.
// OV_OVERRIDE		= Sent by control to unit when control wants to override combat code on the unit.
// OV_REQUEST		= Sent by control to unit when control wants to verify that it is currently handling the unit.
#define OV_CREATED		1000
#define OV_CONFIRM		1001
#define OV_ARRIVED		1002
#define OV_ANIMCOMPLETE	1003
#define OV_OVERRIDE		1004
#define OV_VERIFY		1005
#define OV_DAMAGED		1006
#define OV_ENEMYSEEN	1007
#define OV_SOUNDHEARD	1008
#define OV_KILLED		1009
// OV_AWARE param is a bool.  0 turns awareness off, 1 sets it on.
#define OV_AWARE		1010

// Zone Entering Confirmation to Controller Custom Definition
#define ZONE_CONFIRM	1011

// Conversation queue custom send from conversation controller to object.
#define CONVERSATION	1012

// Scoreboard Customs and Params
#define HITDIRECTION	9910
	#define HD_FRONTHIT				10
	#define HD_BACKHIT				20
#define BODYLOCATION	9920
	#define BL_HEAD					10
	#define BL_CHEST				20
	#define BL_GROIN				30
	#define BL_RIGHTLEG				40
	#define BL_LEFTLEG				50
	#define BL_RIGHTARM				60
	#define BL_LEFTARM				70
#define TALLY			9950
	#define TALLY_KILL				10
	#define TALLY_STRUCTURE			20
	#define TALLY_CIVILIAN			30

// Secret_Level Tracks Completed Secrets in the Level
// If Param is Under 100, it is used to register the total number for
// a specific level.
#define SECRET_LEVEL	9960
	#define SL_COMPLETE				100

// Statistic Collection Custom Definitions
#define STAT_CALL		9990

// MISSION CONTROL CUSTOM DEFINITONS:
// MC_COMPLETE		= Custom event register for reporting a completed objective.
// MC_RESET			= Custom event register for reporting a need to reset an objective.
// MC_IPRIMARY		= Custom event for sending objective primary/secondary.
// MC_IPRIORITY		= Custom event for sending objective priority.
// MC_IINTROTEXT	= Custom event for sending intro text record number.
// MC_ISOLVETEXT	= Custom event for sending immediate solve text record number.
// MC_IOUTTEXT		= Custom event for sending final solve text record number.
// MC_IRESETTEXT	= Custom event for sending objective reset text record number.
#define MC_COMPLETE		1
#define MC_RESET		2
#define MC_IPRIMARY		3
#define	MC_IPRIORITY	4
#define MC_IINTROTEXT	5
#define MC_ISOLVETEXT	6
#define	MC_IOUTTEXT		7
#define MC_IRESETTEXT	8
#define MC_STARTGAME	9

// Movie Control ID's to and from Mission and Movie Controllers
// Params between Intro (10) and Outro (99) are cutscenes
#define MOVIE_PLAY		10
	#define MP_INTRO		10
	#define MP_OUTRO		99


// Mission 3 CONTROL CUSTOM DEFINITIONS:
// M3_SAMOBJECTIVE	= Secondary mission to destroy SAM sites in village completed.
// M3_SAMDESTROYED	= Secondary mission SAM destroyed, send in reinforcements.
#define M3_SAMOBJECTIVE	11
#define M3_SAMDESTROYED	12

// Mission 1 Custom Definitions:
// M1_SAMDESTROYED	= Primary mission to destroy SAM sites completed.
// M1_ACT1CONTROL	= Custom sent from mission start to act 1 controller, animates nod soldier.
// M1_BASEGATE		= Custom Type sent to open and close Base Gate for Harvester
#define M1_SAMDESTROYED	13
#define M1_ACT1CONTROL	14
#define M1_BASEGATE		15
	#define BASEGATE_OPEN			10
	#define BASEGATE_CLOSE			20

// Timer ID's (Unique.  No Collision with other ScriptEvents)
#define	TIMER_IDLE_ANIM			6000
#define TIMER_ARRIVED_FLAG		6010
#define TIMER_SEARCH_LOCATION	6020
#define TIMER_CROUCH_ATTACK		6030
#define TIMER_COMBAT_SPEECH		6040
#define TIMER_TURRET_ATTACK		6050
#define TIMER_HARVESTER_REGEN	6060

//Scoreboard Timers
#define	TIMER_COMBO				6070
#define TIMER_COMBOEVA			6080
#define TIMER_STAT_CALL			6090
#define	TIMER_MISSION			6100

//Mission Objective Timers
#define	TIMER_OBJECTIVE			6110
#define	TIMER_EVASPEECH			6120
#define TIMER_MISSION_FAILURE	6130
#define TIMER_MISSION_SUCCESS	6140

//Movie Control Timers
#define TIMER_REGISTER			6150

//Mission 1 Timers
#define TIMER_M1_A1		102
#define TIMER_M1_A2		103
#define TIMER_TUTORIAL	104
#define TIMER_M1_A3		105

//Mission 1 Radar Events
#define RADAR_M1_P1		150

//Mission 3 Timers
#define TIMER_M3_A1T1	301
#define	TIMER_M3_A1T2	302
#define TIMER_M3_A1T3	303
#define TIMER_M3_B1		304
#define TIMER_M3_B2		305
#define TIMER_M3_B3		306
#define TIMER_M3_A3T1	307
#define TIMER_M3_A3T2	308
#define TIMER_M3_A4T1	309
#define TIMER_M3_A2T1	310
#define TIMER_M3_A4T3	311
#define TIMER_M3_A5T1	312
#define TIMER_M3_A5T2	313
#define TIMER_M3_A4T2	314
#define TIMER_M3_A1T4	315

//Mission 2 Radar Events
#define RADAR_M2_P1		250
#define RADAR_M2_P2		251

//Mission 3 Radar Events
#define RADAR_M3_P1		350
#define RADAR_M3_P2		351
#define RADAR_M3_S1		352
#define RADAR_M3_S2		353
#define RADAR_M3_S3		354

//Custom Event Sound Responses
#define CUSTOM_ALLY_INFO		7000
#define CUSTOM_TARGET_INFO		7010

//Designer Sound Types
#define SOUND_ENEMY_SEEN	SOUND_TYPE_DESIGNER + 5000
#define SOUND_DEATH			SOUND_TYPE_DESIGNER + 5010

