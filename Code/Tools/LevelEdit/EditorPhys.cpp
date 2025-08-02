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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditorPhys.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/11/99 5:47p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "EditorPhys.H"
#include "persist.h"
#include "persistfactory.h"
#include "editorchunkids.h"

//
// Persist factory for EditorPhysClass
//
SimplePersistFactoryClass<EditorPhysClass, CHUNKID_EDITOR_PHYS>	_EditorPhysFactory;


///////////////////////////////////////////////////////////////////////////////////
//
//	Get_Bounding_Box
//
const AABoxClass &
EditorPhysClass::Get_Bounding_Box (void) const
{
	assert(Model);
	return Model->Get_Bounding_Box ();
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Get_Transform
//
const Matrix3D &
EditorPhysClass::Get_Transform (void) const
{
	assert(Model);
	return Model->Get_Transform();
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Get_Transform
//
void
EditorPhysClass::Set_Transform (const Matrix3D &transform)
{
	// Note: this kind of object never causes collisions so we
	// can just warp it to the users desired position.  However,
	// we do need to tell the base class that we moved so that
	// it can tell the scene to update us in the culling system
	assert(Model);
	Model->Set_Transform (transform);
	Update_Cull_Box();
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
EditorPhysClass::Get_Factory (void) const
{
	//
	//	This should NEVER be called...
	//
	ASSERT (0);
	return _EditorPhysFactory;
}
