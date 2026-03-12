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
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/editoronlyobjectnode.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/19/01 3:08p                                                $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EDITORONLYOBJECTNODE_H
#define __EDITORONLYOBJECTNODE_H


#include "node.h"
#include "icons.h"
#include "decophys.h"


//////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////
class PresetClass;


//////////////////////////////////////////////////////////////////////
//
//	EditorOnlyObjectNodeClass
//
//////////////////////////////////////////////////////////////////////
class EditorOnlyObjectNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	EditorOnlyObjectNodeClass (PresetClass *preset = NULL);
	EditorOnlyObjectNodeClass (const EditorOnlyObjectNodeClass &src);
	~EditorOnlyObjectNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const EditorOnlyObjectNodeClass &operator= (const EditorOnlyObjectNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// From PersistClass
	//
	const PersistFactoryClass &	Get_Factory (void) const;

	//
	// Inherited
	//
	NodeClass *	Clone (void)							{ return new EditorOnlyObjectNodeClass (*this); }
	void			Initialize (void);
	NODE_TYPE	Get_Type (void) const				{ return NODE_TYPE_EDITOR_ONLY_OBJ; }
	int			Get_Icon_Index (void) const		{ return OBJECT_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const		{ return DisplayObj; }
	bool			Is_Static (void) const				{ return false; }
	void			Add_To_Scene (void);
	void			Remove_From_Scene (void);

	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////

private:

	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	DecorationPhysClass *		DisplayObj;
};


#endif //__EDITORONLYOBJECTNODE_H
