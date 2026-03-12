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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/lightsolvesavesystem.h       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/04/02 11:11a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LIGHTSOLVESAVESYSTEM_H
#define LIGHTSOLVESAVESYSTEM_H

#include "saveloadsubsystem.h"

class ChunkLoadClass;
class ChunkSaveClass;
class StaticPhysClass;



/**
** LightSolveSaveSystemClass
** This class is a save-load subsystem which can save and restore a lighting solve in the editor.
** Since the editor uses a very abstracted description of the level we cannot use the game's
** mechanism of saving the lighting solve with the model.  This system should be called last
** (or at least after all of the tiles and terrain nodes) in the save process for saving an LVL file.
*/
class LightSolveSaveSystemClass : public SaveLoadSubSystemClass
{
public:

	virtual uint32				Chunk_ID (void) const;

protected:

	virtual bool				Contains_Data(void) const;
	virtual bool				Save (ChunkSaveClass &csave);
	virtual bool				Load (ChunkLoadClass &cload);
	virtual const char*		Name() const { return "LightSolveSaveSystemClass"; }
	virtual void				On_Post_Load(void);

	bool							Save_Lighting_For_Object(ChunkSaveClass & csave,StaticPhysClass * pobj);
	bool							Load_Lighting_For_Object(ChunkLoadClass & cload);

	/*
	** internal chunk id's
	*/
	enum
	{
		LSS_CHUNKID_OBJECT_LIGHT_SOLVE			= 30102537,
		LSS_CHUNKID_OBJECT_VARIABLES,
		LSS_CHUNKID_OBJECT_LIGHTING,

		LSS_VARIABLE_OBJECT_ID						= 0,
		LSS_VARIABLE_OBJECT_CLASSID,
		LSS_VARIABLE_OBJECT_SUBOBJCOUNT,

	};
};

/*
** _TheLightSolveSaveSystem - global instance of this sub-system
*/
extern LightSolveSaveSystemClass _TheLightSolveSaveSystem;



#endif //LIGHTSOLVESAVESYSTEM_H

