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
 *                     $Archive:: /Commando/Code/Commando/multihud.h                          $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 1/24/02 8:38p                                               $*
 *                                                                                             *
 *                    $Revision:: 50                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	MULTIHUD_H
#define	MULTIHUD_H

#if defined(_MSV_VER)
#pragma once
#endif

#include "always.h"
#include "gamemode.h"
#include "bittype.h"

class SmartGameObj;
class Render2DTextClass;
class Render2DSentenceClass;
class Font3DInstanceClass;
class NetworkObjectClass;
class WideStringClass;

enum PlayerlistFormatEnum
{
	PLAYERLIST_FORMAT_TINY,
	PLAYERLIST_FORMAT_MEDIUM,
	PLAYERLIST_FORMAT_FULL,
};

//-----------------------------------------------------------------------------
class	MultiHUDClass {
public:
	static void		Init(void);
	static void		Shutdown(void);
	static void		Think(void);
	static void		Render(void);

	static void		Toggle();
	static bool		Is_On();
	static void		Render_Text(WideStringClass & text, float x, float y, ULONG color = 0xFFFFFFFF);
   static void		Show_Player_Names(void);

#ifdef WWDEBUG
	static void		Render_Debug_Text(LPCSTR text, float x, float y, ULONG color = 0xFFFFFFFF);
	static void		Show_Import_State_Counts(NetworkObjectClass *object);
	static void		Show_Distance_And_Priority(NetworkObjectClass *object);
	static void		Show_Player_Rhost_Data(SmartGameObj * smart_obj);
   static void		Show_Description(NetworkObjectClass * p_object, float height_offset, float max_distance);
   static void		Show_Spawner_Data(void);
	static void		Show_Client_Rhost_Data(void);
#endif

	static void		Set_Bottom_Text_Y_Pos(float y)				{BottomTextYPos = y;}
	static float	Get_Bottom_Text_Y_Pos(void)					{return BottomTextYPos;}

	/*
	static void		Set_Verbose_Lists(bool flag)					{VerboseLists = flag;}
	static bool		Get_Verbose_Lists(void)							{return VerboseLists;}
	static bool		Toggle_Verbose_Lists(void)						{VerboseLists = !VerboseLists; return VerboseLists;}
	*/
	static void							Set_Playerlist_Format(PlayerlistFormatEnum playerlist_format)				{PlayerlistFormat = playerlist_format;}
	static PlayerlistFormatEnum	Get_Playerlist_Format(void)															{return PlayerlistFormat;}
	//static void							Next_Playerlist_Format(void)															{PlayerlistFormat++;}
	static void							Next_Playerlist_Format(void);

private:
	static const float				MAX_OVERLAY_DISTANCE_M;
   static const float				Y_INCREMENT_FACTOR;
	static float						BottomTextYPos;
#ifdef WWDEBUG
	static Render2DTextClass	*	PTextRenderer;
	static Font3DInstanceClass *	PFont;
#endif // WWDEBUG
	static Render2DSentenceClass*	NameRenderer;
	//static bool							VerboseLists;
	static bool							IsOn;
	static PlayerlistFormatEnum	PlayerlistFormat;
};

//-----------------------------------------------------------------------------

#endif // MULTIHUD_H











