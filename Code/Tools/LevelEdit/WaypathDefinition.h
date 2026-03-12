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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/WaypathDefinition.h    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/29/00 11:51a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __WAYPATH_DEFINITION_H
#define __WAYPATH_DEFINITION_H

#include "definition.h"
#include "editorchunkids.h"


//////////////////////////////////////////////////////////////////////////////////
//
//	WaypathDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
class WaypathDefinitionClass : public DefinitionClass
{

public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(WaypathDefinitionClass, DefinitionClass);

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	WaypathDefinitionClass (void);
	virtual ~WaypathDefinitionClass (void);

	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const { return CLASSID_WAYPATH; }

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	virtual bool								Save (ChunkSaveClass &csave);
	virtual bool								Load (ChunkLoadClass &cload);
	virtual PersistClass *					Create (void) const;

protected:

	/////////////////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////////////////
	bool											Load_Variables (ChunkLoadClass &cload);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////
	int			m_PassableObjects;
};



#endif //__WAYPATH_DEFINITION_H

