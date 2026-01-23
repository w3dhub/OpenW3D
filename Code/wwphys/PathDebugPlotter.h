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
 *                     $Archive:: /Commando/Code/wwphys/PathDebugPlotter.h              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/13/00 3:03p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PATH_DEBUG_PLOTTER_H
#define __PATH_DEBUG_PLOTTER_H

#include "pscene.h"
#include "vector3.h"
#include "vector.h"
#include "rendobj.h"
#include "decophys.h"
#include "widgets.h"

/////////////////////////////////////////////////////////////////////////
//
//	PathDebugPlotterClass
//
/////////////////////////////////////////////////////////////////////////
class PathDebugPlotterClass : public RenderObjClass
{
	public:

		/////////////////////////////////////////////////////////////////////////
		// Public constructors/destructors
		/////////////////////////////////////////////////////////////////////////
		PathDebugPlotterClass (void)
			:	m_ShouldDisplay (false),
				m_PhysObj (NULL)				{ _ThePathDebugPlotter = this; WidgetSystem::Init_Debug_Widgets (); m_BoundingBox.Center.Set (0, 0, 0); m_BoundingBox.Extent.Set (2000, 2000, 2000); }

		~PathDebugPlotterClass (void)		{ Reset (); _ThePathDebugPlotter = NULL; }

		/////////////////////////////////////////////////////////////////////////
		// RenderObjClass required methods
		/////////////////////////////////////////////////////////////////////////
		RenderObjClass *Clone (void) const override { return NULL; }
		int				Class_ID (void) const override	{ return CLASSID_LAST + 104L; }
		void				Render (RenderInfoClass &rinfo) override;
		const AABoxClass &Get_Bounding_Box(void) const override { return m_BoundingBox; }
		
		/////////////////////////////////////////////////////////////////////////
		// Public methods
		/////////////////////////////////////////////////////////////////////////
		void				Add (const Vector3 &start, const Vector3 &end, const Vector3 &color);		
		void				Reset (void)				{ m_VectorList.Delete_All (); }
		void				Display (bool display);
		bool				Is_Displayed (void) const { return m_ShouldDisplay; }
		void				Render_Vector (RenderInfoClass & rinfo, const Vector3 & pt, const Vector3 & vec, const Vector3 & color);

		/////////////////////////////////////////////////////////////////////////
		// Static methods
		/////////////////////////////////////////////////////////////////////////
		static PathDebugPlotterClass *Get_Instance (void)	{ return _ThePathDebugPlotter; }

	private:

		/////////////////////////////////////////////////////////////////////////
		// Static members
		/////////////////////////////////////////////////////////////////////////
		static PathDebugPlotterClass *_ThePathDebugPlotter;

		/////////////////////////////////////////////////////////////////////////
		// Private data types
		/////////////////////////////////////////////////////////////////////////
		typedef struct _VECTOR_INFO
		{
			Vector3	point;
			Vector3	vector;
			Vector3	color;

			_VECTOR_INFO(void)
				: point (0, 0, 0), vector (0, 0, 0), color (1, 0, 0)	{ }

			_VECTOR_INFO(const Vector3 &_point, const Vector3 &_vector, const Vector3 &_color)
				: point (_point), vector (_vector), color (_color)		{ }

			bool operator== (const _VECTOR_INFO &/* src*/) { return false; }
			bool operator!= (const _VECTOR_INFO &/* src*/) { return true; }

		} VECTOR_INFO;

		/////////////////////////////////////////////////////////////////////////
		// Private member data
		/////////////////////////////////////////////////////////////////////////
		DynamicVectorClass<VECTOR_INFO>	m_VectorList;
		bool										m_ShouldDisplay;
		AABoxClass								m_BoundingBox;
		DecorationPhysClass *				m_PhysObj;
};


/////////////////////////////////////////////////////////////////////////
//
// Inlines
//
/////////////////////////////////////////////////////////////////////////
inline void
PathDebugPlotterClass::Add (const Vector3 &start, const Vector3 &end, const Vector3 &color)
{	
	VECTOR_INFO info;
	info.point	= start;
	info.vector = end - start;
	info.color	= color;

	//
	//	Add the vector to our list
	//
	m_VectorList.Add (info);
	return ;
}


inline void
PathDebugPlotterClass::Render (RenderInfoClass &rinfo)
{
	if (m_ShouldDisplay) {
		
		for (int index = 0; index < m_VectorList.Count (); index ++) {
			VECTOR_INFO &info = m_VectorList[index];

			//
			//Render the vector
			//
			Render_Vector (rinfo, info.point, info.vector, info.color);
		}
	}

	return ;
}


inline void
PathDebugPlotterClass::Display (bool display)
{
	PhysicsSceneClass *scene = PhysicsSceneClass::Get_Instance ();

	if (scene != NULL) {
		
		if ((m_ShouldDisplay == false) && display) {

			if (m_PhysObj == NULL) {
				m_PhysObj = new DecorationPhysClass;
				m_PhysObj->Set_Model (this);
				m_PhysObj->Set_Cull_Box (m_BoundingBox);
			}

			scene->Add_Dynamic_Object (m_PhysObj);
			m_ShouldDisplay = true;
		} else if (m_ShouldDisplay && (display == false)) {
			scene->Remove_Object (m_PhysObj);
			m_ShouldDisplay = false;
		}
	}

	return ;
}

#endif //__PATH_DEBUG_PLOTTER_H

