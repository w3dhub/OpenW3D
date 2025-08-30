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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/encyclopediamgr.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/03/01 11:19a                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ENCYCLOPEDIAMGR_H
#define __ENCYCLOPEDIAMGR_H


#include "saveloadsubsystem.h"
#include "vector.h"
#include "combatchunkid.h"


///////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////
class DamageableGameObj;


///////////////////////////////////////////////////////////////////////
// Global singleton instance
///////////////////////////////////////////////////////////////////////
extern class EncyclopediaMgrClass _TheEncyclopediaMgrSaveLoadSubsystem;


//////////////////////////////////////////////////////////////////////
//
//	EncyclopediaMgrClass
//
//////////////////////////////////////////////////////////////////////
class EncyclopediaMgrClass : public SaveLoadSubSystemClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		TYPE_UNKNOWN		= -1,
		TYPE_CHARACTER		= 0,
		TYPE_WEAPON,
		TYPE_VEHICLE,
		TYPE_BUILDING,
		TYPE_COUNT

	} TYPE;

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Initialization methods
	//
	static void			Initialize (void);
	static void			Shutdown (void);

	//
	//	Content control
	//
	static bool			Reveal_Object (DamageableGameObj *game_obj);
	static bool			Reveal_Object (TYPE type, int object_id);
	static bool			Is_Object_Revealed (TYPE type, int object_id);

	static void			Reveal_Objects (TYPE type);
	static void			Reveal_All_Objects (void);
	static void			Hide_Objects (TYPE type);
	static void			Hide_All_Objects (void);

	//
	//	Memory persistence
	//
	static void			Store_Data (void);
	static void			Restore_Data (void);

	//
	//	UI
	//
	static void			Display_Event_UI (void);

	//
	//	Inherited
	//
	uint32				Chunk_ID (void) const override			{ return CHUNKID_ENCYCLOPEDIAMGR; }
	
private:
	
	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	bool					Save (ChunkSaveClass &csave) override;
	bool					Load (ChunkLoadClass &cload) override;
	const char *		Name (void) const override					{ return "EncyclopediaMgrClass"; }
								
	//							
	//	Save load support	
	//							
	void					Load_Variables (ChunkLoadClass &cload);

	//
	//	Initialization support
	//
	static void			Build_Bit_Vector (TYPE type);

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////	
	static BooleanVectorClass	KnownObjectVector[TYPE_COUNT];
	static BooleanVectorClass	CopyOfKnownObjectVector[TYPE_COUNT];
};


#endif //__ENCYCLOPEDIAMGR_H
