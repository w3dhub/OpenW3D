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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VisPointGenerator.h                                                                                                                                                                                                                                                                                                                               $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __VIS_POINT_GENERATOR_H
#define __VIS_POINT_GENERATOR_H

#include "vector.h"
#include "vector3.h"
#include "grid3d.h"
#include "matrix3d.h"

// Forward declarations
class RenderObjClass;
class MeshClass;
class NodeClass;


//////////////////////////////////////////////////////////////////////////
//
//	Typdefs
//
//////////////////////////////////////////////////////////////////////////
class VisPointListClass : public DynamicVectorClass<Matrix3D>
{
	public:

		VisPointListClass (void)
			: DynamicVectorClass<Matrix3D> () {}
		virtual ~VisPointListClass (void) {}

		Matrix3D transform;
		Vector3	sample_point;
};

class VisPointInfo
{
	public:

		VisPointInfo (void)
			:	m_DoCameraRing (true),
				m_Transform (1)			{ }
		virtual ~VisPointInfo (void)	{ }

		Matrix3D m_Transform;
		bool		m_DoCameraRing;
};


typedef DynamicVectorClass<VisPointListClass *>	VIS_POINT_LIST;


//////////////////////////////////////////////////////////////////////////
//
//	VisPointGeneratorClass
//
//////////////////////////////////////////////////////////////////////////
class VisPointGeneratorClass
{
	public:

		////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		////////////////////////////////////////////////////////////////////
		VisPointGeneratorClass (float granularity = 16.0F);
		~VisPointGeneratorClass (void);

		////////////////////////////////////////////////////////////////////
		//	Public methods
		////////////////////////////////////////////////////////////////////
		void					Submit_Mesh (MeshClass &mesh);
		VIS_POINT_LIST &	Peek_Point_List (void)					{ return m_PointList; }

		////////////////////////////////////////////////////////////////////
		//	Public methods
		////////////////////////////////////////////////////////////////////
		int					Get_Polys_Processed (void) { return m_PolygonsProcessed; }
		int					Get_Total_Points (void) { return m_TotalPoints; }
		void					Set_Stats_Window (HWND hwnd) { m_StatWindow = hwnd; }

		// Node methods
		NodeClass *			Peek_Current_Node (void) const { return m_CurrentNode; }
		void					Set_Current_Node (NodeClass *node);
		void					Post_Process_Nodes (void);
		void					Add_Manual_Nodes (void);

		// Bias methods
		void					Set_Ignore_Bias (bool onoff)	{ m_IgnoreBias = onoff; }

		// Sampling height control
		void					Set_Vis_Sample_Height (float height) { m_VisSampleHeight = height; }
		float					Get_Vis_Sample_Height (void) { return m_VisSampleHeight; }

	private:

		////////////////////////////////////////////////////////////////////
		//	Private methods
		////////////////////////////////////////////////////////////////////
		void		Subdivide_And_Submit_Centers(const Vector3 & point1,const Vector3 & point2,const Vector3 & point3);
		bool		Find_Valid_Points (const Vector3 &start, const Vector3 &end, const Matrix3 &orientation, bool do_camera_ring);
		bool		Submit_Point (const Matrix3D &vis_transform, bool do_camera_ring);
		void		Generate_Camera_Locations (const Matrix3D &camera_tm);
		void		Initialize_Camera_Sim (void);

		NodeClass *Find_Floor_Node (const Vector3 &start_point);
		bool		Test_Camera_Sim_Point (const Vector3 &start_point, const Vector3 &end_point, Matrix3D *transform_result, NodeClass **node_result);

		// Point validation methods
		bool		Do_View_Planes_Pass (const Matrix3D &vis_transform);
		bool		Check_Ceiling (const Vector3 &position, float *ceiling_dist);
		bool		Is_Object_Invalid_Roof (RenderObjClass *render_obj);

		// Grid methods
		bool		Is_Grid_Cell_Empty (const Vector3 &position);

		// Misc
		void		Determine_Granularity (MeshClass &mesh);

		////////////////////////////////////////////////////////////////////
		//	Private member data
		////////////////////////////////////////////////////////////////////
		bool								m_IgnoreBias;
		float								m_Granularity;
		float								m_BaseGranularity;
		VIS_POINT_LIST					m_PointList;
		Grid3DClass<VisPointInfo *>	m_Grid;
		Vector3							m_ViewPlaneExtent;

		// Current state information
		VisPointListClass *			m_CurrentPointList;
		NodeClass *						m_CurrentNode;
		float								m_CurrentCeilingHeight;

		// Camera simulation variables
		int								m_CameraSimPointCount;
		Vector3 *						m_pCameraSimOffsets;

		// For conversion from world-space to grid-space
		Matrix3D							m_WorldToGridTM;

		// Statistical information
		int								m_PolygonsProcessed;
		int								m_TotalPoints;
		HWND								m_StatWindow;

		// How high above the sector to sample
		float								m_VisSampleHeight;
};


#endif //__VIS_POINT_GENERATOR_H
