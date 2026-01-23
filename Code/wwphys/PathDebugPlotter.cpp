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
 *                     $Archive:: /Commando/Code/wwphys/PathDebugPlotter.cpp             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/15/01 5:02p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "PathDebugPlotter.h"
#include "widgets.h"
#include "matinfo.h"

////////////////////////////////////////////////////////////////////////////////////////////
//
//	Static member initialization
//
////////////////////////////////////////////////////////////////////////////////////////////
PathDebugPlotterClass *PathDebugPlotterClass::_ThePathDebugPlotter = NULL;


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Render_Vector
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathDebugPlotterClass::Render_Vector
(
	RenderInfoClass &	/* rinfo */,
	const Vector3 &	pt,
	const Vector3 &	vec,
	const Vector3 &	color
)
{
	if (vec.Length2() < WWMATH_EPSILON) return;

	PhysicsSceneClass::Get_Instance ()->Add_Debug_Vector (pt, vec, color);
	
	return ;
}

