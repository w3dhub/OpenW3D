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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PathfindStartDefinition.h    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/14/99 4:25p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __PATHFIND_START_DEFINITION_H
#define __PATHFIND_START_DEFINITION_H

#include "definition.h"
#include "editorchunkids.h"


//////////////////////////////////////////////////////////////////////////////////
//
//	PathfindStartDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
class PathfindStartDefinitionClass : public DefinitionClass
{

public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(PathfindStartDefinitionClass, DefinitionClass);

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	PathfindStartDefinitionClass (void);
	virtual ~PathfindStartDefinitionClass (void);

	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const { return CLASSID_PATHFIND_START_DEF; }

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	virtual bool								Save (ChunkSaveClass &csave);
	virtual bool								Load (ChunkLoadClass &cload);
	virtual PersistClass *					Create (void) const;

	// PathfindStartDefinitionClass specific
	int											Get_Object_Type (void) const { return m_GameObjectID; }

protected:

	/////////////////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////////////////
	bool											Load_Variables (ChunkLoadClass &cload);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////
	int m_GameObjectID;
};



#endif //__PATHFIND_START_DEFINITION_H

