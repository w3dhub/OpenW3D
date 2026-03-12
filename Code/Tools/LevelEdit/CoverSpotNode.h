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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/CoverSpotNode.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/25/01 4:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __COVERSPOT_NODE_H
#define __COVERSPOT_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "decophys.h"
#include "editorphys.h"
#include "segline.h"
#include "editorline.h"


////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class PresetClass;
class CoverAttackPointNodeClass;
class CoverEntryClass;


////////////////////////////////////////////////////////////////////////////
//
//	CoverSpotNodeClass
//
////////////////////////////////////////////////////////////////////////////
class CoverSpotNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	CoverSpotNodeClass (PresetClass *preset = NULL);
	CoverSpotNodeClass (const CoverSpotNodeClass &src);
	~CoverSpotNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const CoverSpotNodeClass &operator= (const CoverSpotNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// CoverSpotNodeClass specific
	//
	CoverAttackPointNodeClass *Add_Attack_Point (const Matrix3D &tm);
	void			Remove_Attack_Point (CoverAttackPointNodeClass *attack_point);
	bool			Requires_Crouch (void) const		{ return m_RequiresCrouch; }
	void			Set_Requires_Crouch (bool onoff) { m_RequiresCrouch = onoff; }
	void			Update_Lines (void);

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	void											On_Post_Load (void);

	//
	// From NodeClass
	//
	void			Initialize (void);
	NodeClass *	Clone (void)								{ return new CoverSpotNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const					{ return NODE_TYPE_COVER_SPOT; }
	int			Get_Icon_Index (void) const			{ return OBJECT_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const			{ return m_PhysObj; }
	PhysClass *	Peek_Collision_Obj (void) const;
	bool			Is_Static (void) const					{ return false; }
	bool			Show_Settings_Dialog (void);
	bool			Can_Be_Rotated_Freely (void) const	{ return true; }
	void			Add_To_Scene (void);
	void			Remove_From_Scene (void);
	void			Hide (bool hide);
	NodeClass *	Add_Child_Node (const Matrix3D &tm);
	bool			Can_Add_Child_Nodes (void) const		{ return true; }

	int			Get_Sub_Node_Count (void) const		{ return m_AttackPointNodes.Count (); }
	NodeClass *	Get_Sub_Node (int index);

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
	void			Free_Attack_Points (void);

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DecorationPhysClass *									m_PhysObj;
	DynamicVectorClass<CoverAttackPointNodeClass *>	m_AttackPointNodes;
	DynamicVectorClass<EditorLineClass *>				m_AttackPointLines;
	DynamicVectorClass<Matrix3D>							m_AttackPointLoadList;
	bool															m_RequiresCrouch;
	CoverEntryClass *											m_GameCoverSpot;

	static PhysClass *										_TheCollisionObj;
	static int													_InstanceCount;
};


//////////////////////////////////////////////////////////////////
//	On_Rotate
//////////////////////////////////////////////////////////////////
inline void
CoverSpotNodeClass::On_Rotate (void)
{
	Update_Lines ();

	NodeClass::On_Rotate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Translate
//////////////////////////////////////////////////////////////////
inline void
CoverSpotNodeClass::On_Translate (void)
{
	Update_Lines ();

	NodeClass::On_Translate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Transform
//////////////////////////////////////////////////////////////////
inline void
CoverSpotNodeClass::On_Transform (void)
{
	Update_Lines ();

	NodeClass::On_Transform ();
	return ;
}



#endif //__COVERSPOT_NODE_H

