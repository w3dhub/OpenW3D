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

/* $Header: /Commando/Code/ww3d2/pivot.cpp 1     1/22/01 3:36p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/pivot.cpp                              $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PivotClass::PivotClass -- Constructor for PivotClass                                      *
 *   PivotClass::Compute_Transform -- Update the pivot's transformation matrix                 *
 *   PivotClass::Compute_Transform -- Update the pivot's transformation matrix                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "pivot.h"
#include "wwmath.h"



/***********************************************************************************************
 * PivotClass::PivotClass -- Constructor for PivotClass                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
PivotClass::PivotClass(void) :
	Index(0),
	Parent(NULL),
	BaseTransform(1),
	Transform(1),
	IsVisible(true),
	IsCaptured(false),
	CapTransform(1),
	WorldSpaceTranslation(false)
{
}


void PivotClass::Capture_Update(void)
{
	if ( WorldSpaceTranslation ) {
		// The Translation of CapTransform is meant to be in world space,
		// so remove before applying orientation
		Matrix3D CapOrientation = CapTransform;
		CapOrientation.Set_Translation( Vector3( 0,0,0 ) );
		Matrix3D::Multiply(Transform,CapOrientation,&(Transform));
		// Now apply translation in world space
		Transform.Adjust_Translation( CapTransform.Get_Translation() );
	} else {
		Matrix3D::Multiply(Transform,CapTransform,&(Transform));
	}
}

