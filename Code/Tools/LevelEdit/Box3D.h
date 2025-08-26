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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Box3D.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/26/01 2:05p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __BOX3D_H
#define __BOX3D_H


#include "Dynamesh.H"
#include "EditorPhys.H"
#include "Utils.h"


/////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////
class RayCollisionTestClass;


/////////////////////////////////////////////////////////////////////////
//
//	Box3DClass
//
/////////////////////////////////////////////////////////////////////////
class Box3DClass : public DynamicMeshClass
{
	public:

		//////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////
		Box3DClass (void)
			: m_Dimensions (1, 1, 1),
			  m_bDirty (false),
			  DynamicMeshClass (12, 24) { Create_Model (); }

		Box3DClass (float width, float height, float depth)
			: m_Dimensions (depth, width, height),
			  m_bDirty (false),
			  DynamicMeshClass (12, 24) { Create_Model (); }

		Box3DClass (const Vector3 &dimensions)
			: m_Dimensions (dimensions),
			  m_bDirty (false),
			  DynamicMeshClass (12, 24) { Create_Model (); }

		virtual ~Box3DClass (void) {}

		//////////////////////////////////////////////////////////
		//	Public methods
		////////////////////////////////////////////////////////////

		//
		//	Base class overrides
		//
		bool							Cast_Ray (RayCollisionTestClass &raytest);
		void							Get_Obj_Space_Bounding_Box(AABoxClass & box) const;
		void							Render (RenderInfoClass &rinfo);

			
		//
		//	Dimension methods
		//
		void							Set_Dimensions (const Vector3 &dimensions);
		void							Set_Width (float width);
		void							Set_Height (float height);
		void							Set_Depth (float depth);

		void							Get_Dimensions (Vector3 &dimensions) const		{ dimensions = m_Dimensions; }
		const Vector3 &			Get_Dimensions (void) const							{ return m_Dimensions; }
		float							Get_Width (void) const									{ return m_Dimensions.Y; }
		float							Get_Height (void) const									{ return m_Dimensions.Z; }
		float							Get_Depth (void) const									{ return m_Dimensions.X; }

		//
		//	Vertex methods
		//
		void							Position_Vertex (int vertex, const Vector3 &new_position);
		void							Translate_Vertex (int vertex, const Vector3 &translation);
		Vector3						Get_Vertex_Position (int vertex);
		void							Get_Vertex_Position (int vertex, Vector3 &position);
		Vector3						Get_Vertex_Lock_Position (int vertex);

		//
		//	Creation routines
		//
		void							Make_Box (const Vector3 &point1, const Vector3 &point2);
		void							Set_Color (const Vector3 &color);

		//
		//	Dynamic modification
		//
		void							Drag_VertexXY (int vertex, POINT new_point, const Vector3 &locked_vertex);
		void							Drag_VertexZ (int vertex, POINT new_point, const Vector3 &locked_vertex);


	protected:

		//////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////
		void							Create_Model (void);
		void							Easy_Vertex (const Vector3 &vertex) { DynamicMeshClass::Vertex (vertex.X, vertex.Y, vertex.Z, 0, 0); }
		void							Easy_Move_Vertex (int vertex_index, const Vector3 &vertex) { DynamicMeshClass::Move_Vertex (vertex_index, vertex.X, vertex.Y, vertex.Z); }
		void							Update_Verticies (void);

	private:

		//////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////
		Vector3						m_Dimensions;
		Vector3						m_Verticies[8];		
		bool							m_bDirty;
};


/////////////////////////////////////////////////////////////////////////
//
//	Box3DPhysClass
//
/////////////////////////////////////////////////////////////////////////
class Box3DPhysClass : public EditorPhysClass
{
	public:

		//////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////
		Box3DPhysClass (void)
			: m_pBox (NULL)								{ Initialize (1, 1, 1); }

		Box3DPhysClass (const Vector3 &dimensions)
			: m_pBox (NULL)								{ Initialize (dimensions.X, dimensions.Y, dimensions.Z); }	

		virtual ~Box3DPhysClass (void)				{ MEMBER_RELEASE (m_pBox); }

		//////////////////////////////////////////////////////////
		//	Public operators/methods
		//////////////////////////////////////////////////////////
		operator Box3DClass * (void) const			{ return m_pBox; }
		Box3DClass *			Get_Box (void) const { return m_pBox; }

	protected:

		//////////////////////////////////////////////////////////
		//	Protected member data
		//////////////////////////////////////////////////////////
		void						Initialize (float x, float y, float z)	{ m_pBox = new Box3DClass (x, y, z); Set_Model (m_pBox); }

	private:

		//////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////
		Box3DClass *			m_pBox;
};


#endif //__BOX3D_H

