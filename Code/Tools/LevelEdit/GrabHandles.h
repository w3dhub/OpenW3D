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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/GrabHandles.h                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/30/99 4:53p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __GRABHANDLES_H
#define __GRABHANDLES_H

#include "box3d.h"
#include "hittestinfo.h"
#include "nodefunction.h"


// Forward declarations
class NodeClass;
class GrabHandlesClass;


/////////////////////////////////////////////////////////////////////////
//
//	GrabHandlePhysClass
//
/////////////////////////////////////////////////////////////////////////
class GrabHandlePhysClass : public NodeFunctionClass, public Box3DPhysClass
{
	public:

		//////////////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//
		GrabHandlePhysClass (void)
			: m_iVertex (0),
			  m_pParent (NULL),
			  m_HitTestInfo (this),
			  NodeFunctionClass (),
			  Box3DPhysClass () { Initialize (); }

		GrabHandlePhysClass (int ivertex, GrabHandlesClass *pparent)
			: m_iVertex (ivertex),
			  m_pParent (pparent),
			  m_HitTestInfo (this),
			  NodeFunctionClass (),
			  Box3DPhysClass () { Initialize (); }

		virtual ~GrabHandlePhysClass (void) {}


		//////////////////////////////////////////////////////////
		//
		//	Public operators/methods
		//

		//
		//	Inline accessors
		//
		int						Get_Vertex (void) const { return m_iVertex; }
		void						Set_Vertex (int ivertex) { m_iVertex = ivertex; }
		GrabHandlesClass *	Get_Parent (void) const { return m_pParent; }
		void						Get_Parent (GrabHandlesClass *pparent) { m_pParent = pparent; }


		//
		//	Overrides from NodeFunctionClass
		//
		void						On_Click (void);

	protected:

		//////////////////////////////////////////////////////////
		//
		//	Protected methdos
		//
		void						Initialize ();

		//////////////////////////////////////////////////////////
		//
		//	Protected member data
		//
		int						m_iVertex;
		GrabHandlesClass *	m_pParent;

	private:

		//////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		HITTESTINFO				m_HitTestInfo;
};


/////////////////////////////////////////////////////////////////////////
//
//	GrabHandlesClass
//
class GrabHandlesClass
{
	public:

		//////////////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//
		GrabHandlesClass (void);
		GrabHandlesClass (const GrabHandlesClass &ref);
		virtual ~GrabHandlesClass (void);


		//////////////////////////////////////////////////////////
		//
		//	Public operators/methods
		//
		const GrabHandlesClass &operator= (const GrabHandlesClass &ref);

		void						Position_Around_Node (NodeClass *pnode);
		void						Remove_From_Scene (void);
		void						Hide (bool bhide);

		//
		//	Inline accessors
		//
		NodeClass *	Get_Node (void) const { return m_pNode; }


	protected:

		//////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		void						Initialize (void);

	private:

		//////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		GrabHandlePhysClass *m_pHandles[8];
		bool						m_bIsAddedToScene;
		NodeClass *				m_pNode;
};


#endif //__GRABHANDLES_H


