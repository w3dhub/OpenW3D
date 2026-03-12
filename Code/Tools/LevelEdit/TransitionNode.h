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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TransitionNode.h     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/17/01 10:44a                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TRANSITION_NODE_H
#define __TRANSITION_NODE_H

#include "node.h"
#include "icons.h"
#include "transitiongameobj.h"


// Forward declarations
class PresetClass;
class PhysClass;


////////////////////////////////////////////////////////////////////////////
//
//	TransitionNodeClass
//
////////////////////////////////////////////////////////////////////////////
class TransitionNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	TransitionNodeClass (PresetClass *preset = NULL);
	TransitionNodeClass (const TransitionNodeClass &src);
	~TransitionNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const TransitionNodeClass &operator= (const TransitionNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	//
	// From NodeClass
	//
	NodeClass *	Clone (void)							{ return new TransitionNodeClass (*this); }
	void			Initialize (void);
	NODE_TYPE	Get_Type (void) const				{ return NODE_TYPE_TRANSITION; }
	int			Get_Icon_Index (void) const		{ return TRANSITION_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const;
	bool			Is_Static (void) const				{ return false; }
	void			Add_To_Scene (void);
	void			Remove_From_Scene (void);


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

	//
	//	Transition Specific
	//
	int								Get_Transition_Count (void) const;
	TransitionInstanceClass *	Get_Transition (int index);
	TransitionGameObj *			Get_Transition_Game_Obj (void)	{ return m_TransitionObj; }
	int								Find_Transition (TransitionDataClass::StyleType type);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////

private:

	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	PhysClass *				m_PhysObj;
	TransitionGameObj *	m_TransitionObj;
};


//////////////////////////////////////////////////////////////////
//	Peek_Physics_Obj
//////////////////////////////////////////////////////////////////
inline PhysClass *
TransitionNodeClass::Peek_Physics_Obj (void) const
{
	return m_PhysObj;
}


//////////////////////////////////////////////////////////////////
//	On_Rotate
//////////////////////////////////////////////////////////////////
inline void
TransitionNodeClass::On_Rotate (void)
{
	if (m_TransitionObj != NULL) {
		m_TransitionObj->Set_Transform (m_Transform);
		m_TransitionObj->Update_Transitions ();
	}
	NodeClass::On_Rotate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Translate
//////////////////////////////////////////////////////////////////
inline void
TransitionNodeClass::On_Translate (void)
{
	if (m_TransitionObj != NULL) {
		m_TransitionObj->Set_Transform (m_Transform);
		m_TransitionObj->Update_Transitions ();
	}
	NodeClass::On_Translate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Transform
//////////////////////////////////////////////////////////////////
inline void
TransitionNodeClass::On_Transform (void)
{
	if (m_TransitionObj != NULL) {
		m_TransitionObj->Set_Transform (m_Transform);
		m_TransitionObj->Update_Transitions ();
	}
	NodeClass::On_Transform ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	Get_Transition_Count
//////////////////////////////////////////////////////////////////
inline int
TransitionNodeClass::Get_Transition_Count (void) const
{
	int count = 0;
	if (m_TransitionObj != NULL) {
		count = m_TransitionObj->Get_Transition_Count ();
	}

	return count;
}

//////////////////////////////////////////////////////////////////
//	Get_Transition
//////////////////////////////////////////////////////////////////
inline TransitionInstanceClass *
TransitionNodeClass::Get_Transition (int index)
{
	TransitionInstanceClass *transition = NULL;

	if (	m_TransitionObj != NULL &&
			index >= 0 &&
			index < Get_Transition_Count ())
	{
		transition = m_TransitionObj->Get_Transition (index);
	}

	return transition;
}


#endif //__TRANSITION_NODE_H

