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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/WaypointLine.h               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/18/99 11:05a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WAYPOINT_LINE_H
#define __WAYPOINT_LINE_H


#include "Line3D.H"


///////////////////////////////////////////////////////////////////////////////
//
//	WaypointLineClass
//
class WaypointLineClass : public EditorPhysClass
{
	public:

		////////////////////////////////////////////////////////
		//
		// Public contsructors/destructors
		//
		WaypointLineClass (void)
			: m_StartPos (0, 0, 0),
			  m_EndPos (1, 1, 1),
			  EditorPhysClass () { m_pLine = new Line3DClass (Vector3 (0,0,0), Vector3 (1,1,1), 0.08F, 0.0F, 0.8F, 0.0F); Set_Model (m_pLine); }

		virtual ~WaypointLineClass (void) { MEMBER_RELEASE (m_pLine); }


		////////////////////////////////////////////////////////
		//
		// Public methods
		//

		// Used to reset the position of the line
		void				Reset (const Vector3 &start, const Vector3 &end)					{ m_StartPos = start; m_EndPos = end; m_pLine->Reset (start, end, 0.08F); }
		void				Reset_Start (const Vector3 &start)										{ m_StartPos = start; m_pLine->Reset (m_StartPos, m_EndPos, 0.08F); }
		void				Reset_End (const Vector3 &end)											{ m_EndPos = end; m_pLine->Reset (m_StartPos, m_EndPos, 0.08F); }

	private:

		////////////////////////////////////////////////////////
		//
		// Private member data
		//
		Vector3			m_StartPos;
		Vector3			m_EndPos;
		Line3DClass		*m_pLine;
};


#endif //__WAYPOINT_LINE_H

