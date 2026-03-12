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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TileNode.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/08/01 10:09a                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TILE_NODE_H
#define __TILE_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "staticphys.h"

// Forward declarations
class PresetClass;


////////////////////////////////////////////////////////////////////////////
//
//	TileNodeClass
//
////////////////////////////////////////////////////////////////////////////
class TileNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	TileNodeClass (PresetClass *preset = NULL);
	TileNodeClass (const TileNodeClass &src);
	~TileNodeClass (void);


	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const TileNodeClass &operator= (const TileNodeClass &src);


	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	//
	//	RTTI
	//
	TileNodeClass *As_TileNodeClass (void)			{ return this; }

	// From NodeClass
	void			Initialize (void);
	NodeClass *	Clone (void)							{ return new TileNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const				{ return NODE_TYPE_TILE; }
	int			Get_Icon_Index (void) const		{ return TILE_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const		{ return m_PhysObj; }
	bool			Is_Static (void) const				{ return true; }
	bool			Can_Be_Rotated_Freely (void) const	{ return true; }
	void			Set_ID (uint32 id);
	void			Update_Cached_Vis_IDs (void);

	void			Pre_Export (void);
	void			Post_Export (void);


	//
	//	From PersistClass
	//
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

	//
	//	Notifications
	//
	void			On_Rotate (void);
	void			On_Translate (void);
	void			On_Transform (void);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	bool			Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	StaticPhysClass *	m_PhysObj;
	uint32				m_VisObjectID;
	uint32				m_VisSectorID;
};


//////////////////////////////////////////////////////////////////
//	Set_ID
//////////////////////////////////////////////////////////////////
inline void
TileNodeClass::Set_ID (uint32 id)
{
	if (m_PhysObj != NULL) {
		m_PhysObj->Set_ID (id);
	}

	NodeClass::Set_ID (id);
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Rotate
//////////////////////////////////////////////////////////////////
inline void
TileNodeClass::On_Rotate (void)
{
	NodeClass::On_Rotate ();

	//
	//	Update the cull-link index
	//
	Update_Cached_Cull_Link ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Translate
//////////////////////////////////////////////////////////////////
inline void
TileNodeClass::On_Translate (void)
{
	NodeClass::On_Translate ();

	//
	//	Update the cull-link index
	//
	Update_Cached_Cull_Link ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Transform
//////////////////////////////////////////////////////////////////
inline void
TileNodeClass::On_Transform (void)
{
	NodeClass::On_Transform ();

	//
	//	Update the cull-link index
	//
	Update_Cached_Cull_Link ();
	return ;
}


#endif //__TILE_NODE_H

