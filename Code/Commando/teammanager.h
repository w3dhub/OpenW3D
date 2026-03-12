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
 *                     $Archive:: /Commando/Code/Commando/teammanager.h                   $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 1/03/02 5:49p                                               $*
 *                                                                                             *
 *                    $Revision:: 31                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSV_VER)
#pragma once
#endif

#ifndef	TEAMMANAGER_H
#define	TEAMMANAGER_H

#include "vector3.h"
#include "bittype.h"
#include "slist.h"
#include "team.h"
#include "wwstring.h"
#include "widestring.h"

class Render2DTextClass;
class Font3DInstanceClass;

const int MAX_TEAMS = 2;

//
// cTeamManager
//
// A collection of routines to maintain a list of team objects
//
class cTeamManager {

public:
	static void							Onetime_Init(void);
	static void							Onetime_Shutdown(void);
	static void							Think(void);
	static void							Render(void);

	static void							List_Print(WideStringClass & text, Vector3 color);
	static void							Construct_Heading(WideStringClass & heading);
   static void       				Render_Team_List(void);
   static WideStringClass			Get_Team_Name(int team_number);
   static void       				Sort_Teams(void);
   static int        				Get_Leaders_Id(void);
   static void       				Reset_Teams(void);
	static cTeam	   * 				Find_Team(int team_number);
	static cTeam*							Find_Empty_Team(void);
	static SList<cTeam> * 			Get_Team_Object_List(void)	{return &TeamList;}
   static void	      				Add(cTeam * p_team);
   static void       				Remove_All(void); // N.B. destructive
	static float						Get_Team_List_Height(void) {return TeamListHeight;}
	static int							Count(void)	{return TeamList.Get_Count();};
	static int							Compute_Team_List_Height(void);
   static void       				Log_Team_List(void);

private:
   static void       				Remove(cTeam * p_team); // N.B. destructive
   static void       				Remove(int team_number); // N.B. destructive
   static int        				Get_Non_Empty_Team_Count(void);
   static int        				Sort_Score_Ktd_Kills(cTeam * p_team1, cTeam * p_team2);
   static int							Team_Compare(const void * elem1, const void * elem2);

	static SList<cTeam>				TeamList;					// list of all team objects
   static cTeam *						Team_Array[MAX_TEAMS];	// qsort-ed list of non-empty teams
	static float						TeamListHeight;
	static Render2DTextClass	*	PTextRenderer;
	static Font3DInstanceClass *	PFont;
	static const float				Y_INCREMENT_FACTOR;
	static int							XPos;
	static int							YPos;
};

#endif // TEAMMANAGER_H









