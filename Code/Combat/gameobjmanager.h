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
 *                     $Archive:: /Commando/Code/Combat/gameobjmanager.h                      $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 12/09/01 2:02p                                              $*
 *                                                                                             *
 *                    $Revision:: 38                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	GAMEOBJMANAGER_H
#define	GAMEOBJMANAGER_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef SLIST_H
	#include "slist.h"
#endif

#include "networkobjectmgr.h"

/*
**
*/
class BaseGameObj;
class SmartGameObj;
class SoldierGameObj;
class ScriptableGameObj;
class VehicleGameObj;
class BuildingGameObj;
class PhysicalGameObj;
class ChunkSaveClass;
class ChunkLoadClass;
class StaticPhysClass;
class BuildingAggregateClass;
class LightPhysClass;
class Vector3;

/*
**	Static IDs
*/
enum
{
	NETID_GDI_BASE_CONTROLLER	= NETID_STATIC_OBJECT_MIN + 1,
	NETID_NOD_BASE_CONTROLLER,
	NETID_NOD_TEAM,
	NETID_GDI_TEAM,
	NETID_SERVER_FPS,
	NETID_SERVER_WEATHER,
	NETID_SERVER_BACKGROUND
};


/*
** GameObjManager
**
** A collection of routines to maintain lists of game objects
**
** In the editor, building objects will be created and added to this manager.  In the game, they will
** be loaded.  After a level has been loaded, the 'Init_Level_Buildings' function should be called.
** This manager will be the entry point for any operations that need to happen on all existing buildings
** such as save/load, per-frame processing, etc.
*/

class GameObjManager {

public:
	static	void			Init(void);
	static	void			Shutdown(void);

	static	bool			Save( ChunkSaveClass &csave );
	static	bool			Load( ChunkLoadClass &cload );

	static	int			Generate_Control();
	static	int			Think();
	static	int			Post_Think();

	static	void			Init_All(void);
	static	void			Destroy_All( void );
	//static	void			Destroy_Pending( void );

	// BaseGameObjs
	static	void			Add( BaseGameObj *obj );
	static	void			Remove( BaseGameObj *obj ) { GameObjList.Remove( obj ); }
	static	SList<BaseGameObj>	  	*Get_Game_Obj_List( void )			{ return &GameObjList; }

	// SmartGameObjs
	static	void			Add_Smart( SmartGameObj *obj ) { SmartGameObjList.Add_Tail( obj ); }
	static	void			Remove_Smart( SmartGameObj *obj ) { SmartGameObjList.Remove( obj ); }
	static	SList<SmartGameObj>	  	*Get_Smart_Game_Obj_List( void )	{ return &SmartGameObjList; }

	// Star GameObjs
	static	void			Add_Star( SoldierGameObj *obj ) { StarGameObjList.Add_Tail( obj ); }
	static	void			Remove_Star( SoldierGameObj *obj ) { StarGameObjList.Remove( obj ); }
	static	SList<SoldierGameObj>	*Get_Star_Game_Obj_List( void )	{ return &StarGameObjList; }

	// BuildingGameObjs
	static	void			Init_Buildings(void);
	static	void			Update_Building_Collection_Spheres (void);
	static	void			Debug_Set_All_Building_States(float health_percentage,bool power_on);
	static	void			Add_Building( BuildingGameObj *obj ) { BuildingGameObjList.Add_Tail( obj ); }
	static	void			Remove_Building( BuildingGameObj *obj ) { BuildingGameObjList.Remove( obj ); }
	static	SList<BuildingGameObj>	*Get_Building_Game_Obj_List( void )	{ return &BuildingGameObjList; }

	// Environment Zone
	static	bool			Is_In_Environment_Zone( Vector3 & pos );

	// Find Game Objs
   static   SoldierGameObj    *Find_Soldier_Of_Client_ID(int client_id);
   static   SoldierGameObj    *Find_Different_Player_Soldier(int my_id);
	static	SoldierGameObj		*Find_Soldier_Of_Player_Type(int player_type);
	static	PhysicalGameObj	*Find_PhysicalGameObj( int id );
	static	SmartGameObj		*Find_SmartGameObj( int id );
	static	ScriptableGameObj	*Find_ScriptableGameObj( int id );
	static	VehicleGameObj		*Find_Vehicle_Occupied_By( SoldierGameObj * p_soldier );

	// Cinematic Freeze
	static	bool					Is_Cinematic_Freeze_Active( void )				{ return CinematicFreezeActive; }
	static	void					Activate_Cinematic_Freeze( bool activate )	{ CinematicFreezeActive = activate; }
	static	void					Toggle_Cinematic_Freeze( void )					{ CinematicFreezeActive = !CinematicFreezeActive; }

private:
	static	SList<BaseGameObj>	  	GameObjList;			// list of all game objs
	static	SList<SmartGameObj>	  	SmartGameObjList;		// list of all smart game objs
	static	SList<SoldierGameObj>	StarGameObjList;		// list of all star game objs
	static	SList<BuildingGameObj>	BuildingGameObjList;	// list of all builiding game objs

	static	bool							CinematicFreezeActive;
};

#endif		//	GAMEOBJMANAGER_H
