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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/CoverAttackPointNode.h   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/11/01 9:30a                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __COVER_ATTACK_POINT_NODE_H
#define __COVER_ATTACK_POINT_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "decophys.h"
#include "coverspotnode.h"


////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class PresetClass;
class CoverSpotNode;


////////////////////////////////////////////////////////////////////////////
//
//	CoverAttackPointNodeClass
//
////////////////////////////////////////////////////////////////////////////
class CoverAttackPointNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	CoverAttackPointNodeClass (PresetClass *preset = NULL);
	CoverAttackPointNodeClass (const CoverAttackPointNodeClass &src);
	~CoverAttackPointNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const CoverAttackPointNodeClass &operator= (const CoverAttackPointNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// CoverAttackPointNodeClass specific
	//
	CoverSpotNodeClass *	Peek_Cover_Spot (void) const					{ return m_CoverSpot; }
	void						Set_Cover_Spot (CoverSpotNodeClass *spot) { m_CoverSpot = spot; }

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	//
	// From NodeClass
	//
	void			Initialize (void);
	NodeClass *	Clone (void)								{ return new CoverAttackPointNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const					{ return NODE_TYPE_COVER_ATTACK_POINT; }
	int			Get_Icon_Index (void) const			{ return OBJECT_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const			{ return m_PhysObj; }
	PhysClass *	Peek_Collision_Obj (void) const;
	bool			Is_Static (void) const					{ return false; }
	bool			Show_Settings_Dialog (void)			{ return true; }
	bool			Can_Be_Rotated_Freely (void) const	{ return true; }
	void			On_Delete (void);
	NodeClass *	Get_Parent_Node (void) const			{ return m_CoverSpot; }

	//
	//	Notifications
	//
	void			On_Rotate (void);
	void			On_Translate (void);
	void			On_Transform (void);
	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);


	//	From PersistClass
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	bool			Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DecorationPhysClass *	m_PhysObj;
	CoverSpotNodeClass *		m_CoverSpot;

	static PhysClass *		_TheCollisionObj;
	static int					_InstanceCount;
};


//////////////////////////////////////////////////////////////////
//	On_Rotate
//////////////////////////////////////////////////////////////////
inline void
CoverAttackPointNodeClass::On_Rotate (void)
{
	if (m_CoverSpot != NULL) {
		m_CoverSpot->Update_Lines ();
	}

	NodeClass::On_Rotate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Translate
//////////////////////////////////////////////////////////////////
inline void
CoverAttackPointNodeClass::On_Translate (void)
{
	if (m_CoverSpot != NULL) {
		m_CoverSpot->Update_Lines ();
	}

	NodeClass::On_Translate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Transform
//////////////////////////////////////////////////////////////////
inline void
CoverAttackPointNodeClass::On_Transform (void)
{
	if (m_CoverSpot != NULL) {
		m_CoverSpot->Update_Lines ();
	}

	NodeClass::On_Transform ();
	return ;
}


#endif //__COVER_ATTACK_POINT_NODE_H

