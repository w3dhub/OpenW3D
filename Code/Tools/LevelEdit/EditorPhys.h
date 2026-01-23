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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditorPhys.h                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/24/00 1:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EDITORPHYS_H
#define __EDITORPHYS_H


#include "dynamicphys.h"
#include "pscene.h"
#include "wwdebug.h"
#include "physcoltest.h"
#include "rendobj.h"


///////////////////////////////////////////////////////////////////////////////////
//
//	EditorPhysClass
//
//	Editor's representation of a physics object.
//
///////////////////////////////////////////////////////////////////////////////////
class EditorPhysClass : public DynamicPhysClass
{
	public:

		virtual bool					Cast_Ray(PhysRayCollisionTestClass & raytest);
		virtual bool					Cast_AABox(PhysAABoxCollisionTestClass & /* boxtest */)	{ return false; }
		virtual bool					Cast_OBBox(PhysOBBoxCollisionTestClass & /* boxtest */)	{ return false; }
		virtual void					Timestep (float /* dt */)											{};

		virtual const AABoxClass & Get_Bounding_Box (void) const;
		virtual const Matrix3D &	Get_Transform (void) const;
		virtual void					Set_Transform (const Matrix3D &transform);

		virtual const PersistFactoryClass &	Get_Factory (void) const;
};


///////////////////////////////////////////////////////////////////////////////////
//
//	Cast_Ray
//
inline bool
EditorPhysClass::Cast_Ray (PhysRayCollisionTestClass &raytest)
{
	// State OK?
	WWASSERT (Model != NULL);

	// Cast the ray and return the result
	bool retval = false;	
	if ((Model != NULL) &&
	    (Model->Cast_Ray (raytest))) {
			
		// Success!
		raytest.CollidedPhysObj = this;
		retval = true;
	}

	// Return the true/false result code
	return retval;
}

#endif //__EDITORPHYS_H

