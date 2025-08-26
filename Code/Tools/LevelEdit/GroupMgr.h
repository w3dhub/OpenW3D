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

///////////////////////////////////////////////////////////////////////////////////////
//
//	GroupMgrClass.H
//
//	Class declarations for managing different types of level items.
//

#ifndef __GROUPMGR_H
#define __GROUPMGR_H


#include "Utils.h"
#include "vector3.h"
#include "aabox.h"
#include "sphere.h"
#include "uniquelist.h"
#include "listtypes.h"

class NodeClass;
class PresetClass;


///////////////////////////////////////////////////////////////
//
//	GroupMgrClass
//
//	Used to manage all items in a group.
//
class GroupMgrClass
{
	public:

		///////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//
		GroupMgrClass (void)
			: m_bHidden (false),
			  m_bDirty (true) {}

		GroupMgrClass (const GroupMgrClass &src)
			: m_bHidden (false),
			  m_bDirty (true) { *this = src; }

		virtual ~GroupMgrClass (void) { Free_List (); }


		///////////////////////////////////////////////////
		//
		//	Public methods
		//

		//
		//	operators
		//
		NodeClass *				operator[] (int index)			{ return m_GroupList[index]; }
		const NodeClass *		operator[] (int index) const	{ return m_GroupList[index]; }
		const GroupMgrClass &operator= (const GroupMgrClass &src);

		//
		//	Required enumeration methods
		//
		virtual int				Get_Count (void) const		{ return m_GroupList.Count (); }
		virtual NodeClass *	Get_At (int index) const	{ return m_GroupList[index]; }

		//
		//	Item addition/removal methods
		//
		virtual void			Add_Node (NodeClass *node);
		virtual void			Import_Nodes (const NODE_LIST &node_list);
		virtual void			Export_Nodes (NODE_LIST &node_list);
		virtual void			Remove_Node (NodeClass *node);
		virtual void			Reset (void);
		virtual void			Free_List (void);
		virtual bool			Is_Item_In_Group (NodeClass *node);

		//
		//	Member information methods
		//
		virtual bool			Is_Hidden (void) const { return m_bHidden; }
		virtual void			Hide (bool bhide);

		//
		//	Type methods
		//
		virtual bool			Is_Managed (void) const { return true; }

		//
		//	Information methods
		//		
		const Vector3 &		Get_Center (void) 					{ Update (); return m_LowZCenter; }
		const Vector3 &		Get_Abs_Center (void) 				{ Update (); return m_AbsCenter; }
		const SphereClass &	Get_Bounding_Sphere (void)			{ Update (); return m_BoundingSphere; }
		const AABoxClass &	Get_Bounding_Box (void)				{ Update (); return m_BoundingBox; }
		const CString &		Get_Name (void) const				{ return m_Name; }
		void						Set_Name (LPCTSTR name)				{ m_Name = name; }

		//
		//	Update methods
		//
		void						Set_Dirty (void) { m_bDirty = true; }		

		//
		//	Copy methods
		//
		virtual void			Clone_Group (void);

	protected:

		///////////////////////////////////////////////////
		//
		//	Protected member data
		//

		virtual void			Recalc_Stats (void);
		virtual void			Update (void) { if (m_bDirty) { Recalc_Stats (); } }


		///////////////////////////////////////////////////
		//
		//	Protected member data
		//
		NODE_LIST		m_GroupList;
		SphereClass		m_BoundingSphere;
		AABoxClass		m_BoundingBox;
		Vector3			m_LowZCenter;
		Vector3			m_AbsCenter;
		CString			m_Name;
		bool				m_bDirty;
		bool				m_bHidden;
};


///////////////////////////////////////////////////////////////
//
//	SelectionMgrClass
//
//	Used to manage all items in the current selection set.
//
class SelectionMgrClass : public GroupMgrClass
{
	public:

		///////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//
		SelectionMgrClass (void) : GroupMgrClass () {}
		virtual ~SelectionMgrClass (void) {}


		///////////////////////////////////////////////////
		//
		//	Public methods
		//

		//
		//	Item addition/removal methods
		//
		virtual void		Add_Node (NodeClass *node);		
		virtual void		Remove_Node (NodeClass *node);
		virtual void		Reset (void);

		//
		//	Copy methods
		//
		virtual void		Clone_Group (void);
};


///////////////////////////////////////////////////////////////
//
//	UserGroupMgrClass
//
//	Used to manage all nodes in a user-defined group .
//
class UserGroupMgrClass : public GroupMgrClass
{
	public:

		///////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//
		UserGroupMgrClass (void) : GroupMgrClass () {}
		virtual ~UserGroupMgrClass (void);


		///////////////////////////////////////////////////
		//
		//	Public methods
		//

		//
		//	Item addition/removal methods
		//
		virtual void		Add_Node (NodeClass *node);		
		virtual void		Remove_Node (NodeClass *node);

		//
		//	Type methods
		//
		virtual bool		Is_Managed (void) const { return false; }

	protected:

		///////////////////////////////////////////////////
		//
		//	Protected methods
		//
		virtual void		Recalc_Stats (void);
		virtual void		Update_Global_Group_List (void);
};


#endif //__GROUPMGR_H
