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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/UndoMgr.h                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/19/01 11:34a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __UNDO_MGR_H
#define __UNDO_MGR_H

#include "matrix3d.h"
#include "vector.h"
#include "listtypes.h"


// Forward declarations
class NodeClass;


////////////////////////////////////////////////////////////////////////////////////
//
//	Data types
//
////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	OPERATION_MOVE,
	OPERATION_ROTATE,
	OPERATION_DELETE,
	OPERATION_RESIZE,
	OPERATION_COUNT
} OPERATION_TYPE;


////////////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
////////////////////////////////////////////////////////////////////////////////////
const int UNDO_LEVELS		= 5;


////////////////////////////////////////////////////////////////////////////////////
//
//	UndoBufferClass
//
////////////////////////////////////////////////////////////////////////////////////
class UndoBufferClass
{
public:

	//////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////
	UndoBufferClass (void)
		: m_dwOperationID (0) {}

	virtual ~UndoBufferClass (void) {}

	//////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////

	//
	//	Inline Accessors
	//
	OPERATION_TYPE			Get_Operation_Type (void) const				{ return m_Type; }
	void						Set_Operation_Type (OPERATION_TYPE type)	{ m_Type = type; }
	DWORD						Get_ID (void) const								{ return m_dwOperationID; }
	void						Set_ID (DWORD id)									{ m_dwOperationID = id; }

	//
	//	Operation methods
	//
	void						Capture_Position (const NODE_LIST &node_list);
	void						Capture_Dimensions (const NODE_LIST &node_list);
	void						Capture_Existence (const NODE_LIST &node_list);
	void						Restore_State (void);

	//
	//	Information methods
	//
	LPCTSTR					Get_Operation_Name (void);

protected:

	//////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////
	//void						Restore_Node_State (NodeClass &node);

private:

	//////////////////////////////////////////////////////////
	//	Private data types
	//////////////////////////////////////////////////////////
	class NodeStateClass
	{
		public:
			NodeStateClass (void)
				: node_ptr (0),
				  transform (1),
				  dimensions (0, 0, 0)	{}

			virtual ~NodeStateClass (void);

			bool operator== (const NodeStateClass &src) { return (node_ptr == src.node_ptr); }
			bool operator!= (const NodeStateClass &src) { return !((*this) == src); }
			const NodeStateClass &operator= (const NodeStateClass &src);

			//DWORD			node_id;
			Matrix3D		transform;
			Vector3		dimensions;
			//NodeClass *	deleted_node;
			NodeClass *	node_ptr;
	};

	//////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////
	DWORD											m_dwOperationID;
	OPERATION_TYPE								m_Type;
	DynamicVectorClass<NodeStateClass>	m_StateList;
};


////////////////////////////////////////////////////////////////////////////////////
//
//	UndoMgrClass
//
////////////////////////////////////////////////////////////////////////////////////
class UndoMgrClass
{
public:

	//////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////
	UndoMgrClass (void)
		: m_dwNextOperationID (0),
		  m_iCurrentBuffer (0) {}

	virtual ~UndoMgrClass (void) { Free_Buffer_List (); }

	//////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////

	//
	//	Operation management
	//
	void					Begin_Operation (OPERATION_TYPE type, const NODE_LIST &nodes_affected);
	void					End_Operation (void) { }
	void					Perform_Undo (int ilevels = 1);
	void					Purge_Buffers (void) { Free_Buffer_List (); }

	//
	//	Information methods
	//
	int					Get_Possible_Undo_Count (void) const { return m_iCurrentBuffer; }
	LPCTSTR				Get_Next_Undo_Name (void) const;

protected:

	//////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////
	void					Push_Buffer (UndoBufferClass *pbuffer);
	void					Free_Buffer_List (void);

private:

	//////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////
	DWORD					m_dwNextOperationID;
	int					m_iCurrentBuffer;
	UndoBufferClass * m_pBuffers[UNDO_LEVELS];
};


#endif //__UNDO_MGR_H

