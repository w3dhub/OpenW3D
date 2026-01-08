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
 *                     $Archive:: /Commando/Code/Combat/savegame.h                            $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 11/02/01 2:19p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 8                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SAVEGAME_H
#define	SAVEGAME_H

#include "always.h"
#include "wwstring.h"
#include "widestring.h"

/*
**
*/
class	SaveGameManager {

public:
	
	// Map filename (LSD) access
	static void Set_Map_Filename( const char * filename )	{ MapFilename = filename; }
	static const char * Get_Map_Filename( void )			{ return MapFilename; }
	
	// Description access
	static void Set_Description( const unichar_t * text )		{ Description = text; }
	static const unichar_t * Get_Description( void )			{ return Description; }

	// Mission description access
	static int Get_Mission_Description_ID( void )			{ return MissionDescriptionID; }
	static void	Set_Mission_Description_ID( int id )		{ MissionDescriptionID = id; }
	
	// Utility functions
	static bool Smart_Peek_Description( const char *filename, WideStringClass &description, WideStringClass &mission_name );
	static bool Smart_Peek_Map_Name( const char * filename, StringClass &map_name );
	static bool Peek_Description( const char *filename, WideStringClass &description, WideStringClass &mission_name );
	static bool Peek_Map_Name( const char * filename, StringClass &map_name );

	// LDD Access - Editor only calls Save_Game, App calls both
	static void __cdecl Save_Game( const char * filename, ... );
	static void	Load_Game( const char * filename );
	static void	Pre_Load_Game( const char * filename, StringClass &filename_to_load, StringClass &lsd_filename );
	static const char * Get_Current_Game_Filename( void )	{ return CurrentGameFilename; }
	
	// LSD Access - Editor only calls Save_Level, App only calls Load_Level
	static void	Save_Level( void );
	static void	Load_Level( void );

	// DDB Access - Editor only calls Save_Level, App only calls Load_Level
	static void	Save_Definitions( const char * filename = DefaultDefinitionFilename );
	static void	Load_Definitions( const char * filename = DefaultDefinitionFilename );

	// Generic SaveLoadSubSystem Access
	static void	Load_Save_Load_System( const char * filename, bool auto_post_load );
	static void __cdecl Save_Save_Load_System( const char * filename, ... );

protected:
	static StringClass		MapFilename;
	static StringClass		CurrentGameFilename;
	static WideStringClass	Description;
	static int				MissionDescriptionID;

	static const char * DefaultDefinitionFilename;
};

#endif	//	SAVEGAME_H
