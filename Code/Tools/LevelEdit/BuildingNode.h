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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/BuildingNode.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/11/01 9:04a                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __BUILDING_NODE_H
#define __BUILDING_NODE_H

#include "objectnode.h"
#include "vector.h"
#include "icons.h"
#include "decophys.h"
#include "building.h"
#include "editorline.h"


////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class PresetClass;
class BuildingChildNodeClass;

////////////////////////////////////////////////////////////////////////////
//
//	BuildingNodeClass
//
////////////////////////////////////////////////////////////////////////////
class BuildingNodeClass : public ObjectNodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	BuildingNodeClass (PresetClass *preset = NULL);
	BuildingNodeClass (const BuildingNodeClass &src);
	~BuildingNodeClass (void);


	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const BuildingNodeClass &operator= (const BuildingNodeClass &src);


	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	// From NodeClass
	void			Initialize (void);
	NodeClass *	Clone (void)								{ return new BuildingNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const					{ return NODE_TYPE_BUILDING; }
	int			Get_Icon_Index (void) const			{ return BUILDING_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const			{ return m_PhysObj; }
	bool			Is_Static (void) const					{ return false; }
	bool			Can_Be_Rotated_Freely (void) const	{ return true; }
	void			On_Rotate (void);
	void			On_Translate (void);
	void			On_Transform (void);
	void			Add_To_Scene (void);
	void			Remove_From_Scene (void);
	void			Hide (bool hide);
	NodeClass *	Add_Child_Node (const Matrix3D &tm);
	bool			Can_Add_Child_Nodes (void)	const;

	int			Get_Sub_Node_Count (void) const		{ return m_ChildNodes.Count (); }
	NodeClass *	Get_Sub_Node (int index);

	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);

	//
	//	From PersistClass
	//
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

	//
	//	Building specific
	//
	void			Enable_Power (bool onoff);
	bool			Is_Power_Enabled (void) const;
	void			Set_Normalized_Health (float health);
	void			Remove_Child_Node (NodeClass *child_node);
	void			Update_Lines (void);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	bool					Load_Variables (ChunkLoadClass &cload);
	void					Free_Child_Nodes (void);
	BuildingGameObj *	Get_Building (void) const		{ return reinterpret_cast<BuildingGameObj *> (m_GameObj); }

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DecorationPhysClass *								m_PhysObj;
	DynamicVectorClass<BuildingChildNodeClass *>	m_ChildNodes;
	DynamicVectorClass<EditorLineClass *>			m_ChildLines;
};



//////////////////////////////////////////////////////////////////
//	On_Rotate
//////////////////////////////////////////////////////////////////
inline void
BuildingNodeClass::On_Rotate (void)
{
	if (m_GameObj != NULL) {
		((BuildingGameObj *)m_GameObj)->Set_Position (m_Transform.Get_Translation ());
	}

	Update_Lines ();
	NodeClass::On_Rotate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Translate
//////////////////////////////////////////////////////////////////
inline void
BuildingNodeClass::On_Translate (void)
{
	if (m_GameObj != NULL) {
		((BuildingGameObj *)m_GameObj)->Set_Position (m_Transform.Get_Translation ());
	}

	Update_Lines ();
	NodeClass::On_Translate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Transform
//////////////////////////////////////////////////////////////////
inline void
BuildingNodeClass::On_Transform (void)
{
	if (m_GameObj != NULL) {
		((BuildingGameObj *)m_GameObj)->Set_Position (m_Transform.Get_Translation ());
	}

	Update_Lines ();
	NodeClass::On_Transform ();
	return ;
}

//////////////////////////////////////////////////////////////////
//	Enable_Power
//////////////////////////////////////////////////////////////////
inline void
BuildingNodeClass::Enable_Power (bool onoff)
{
	if (m_GameObj != NULL) {
		((BuildingGameObj *)m_GameObj)->Enable_Power (onoff);
	}

	return ;
}

//////////////////////////////////////////////////////////////////
//	Is_Power_Enabled
//////////////////////////////////////////////////////////////////
inline bool
BuildingNodeClass::Is_Power_Enabled (void) const
{
	bool retval = false;

	if (m_GameObj != NULL) {
		retval = ((BuildingGameObj *)m_GameObj)->Is_Power_Enabled ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////
//	Set_Normalized_Health
//////////////////////////////////////////////////////////////////
inline void
BuildingNodeClass::Set_Normalized_Health (float health)
{
	if (m_GameObj != NULL) {
		((BuildingGameObj *)m_GameObj)->Set_Normalized_Health (health);
	}

	return ;
}



#endif //__BUILDING_NODE_H

