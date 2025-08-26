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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/AttenuationSphere.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/04/00 8:02p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ATTENUATIONSPHERE_H
#define __ATTENUATIONSPHERE_H


#include "Utils.h"
#include "EditorPhys.h"
#include "vector3.h"


// Forward declarations
class NodeClass;


///////////////////////////////////////////////////////////////////////////////
//
//	AttenuationSphereClass
//
///////////////////////////////////////////////////////////////////////////////
class AttenuationSphereClass : public EditorPhysClass
{
public:

	////////////////////////////////////////////////////////
	// Public contsructors/destructors
	////////////////////////////////////////////////////////
	AttenuationSphereClass (void);
	virtual ~AttenuationSphereClass (void);


	////////////////////////////////////////////////////////
	// Public methods
	////////////////////////////////////////////////////////
	void		Display_Around_Node (const NodeClass &node);
	void		Display_Around_Node (const RenderObjClass &render_obj);
	void		Remove_From_Scene (void);
	void		Set_Color (const Vector3 &color);
	void		Set_Radius (float radius);
	void		Set_Opacity (float opacity);

private:

	////////////////////////////////////////////////////////
	// Private member data
	////////////////////////////////////////////////////////
	bool		m_IsInScene;
	Vector3	m_Color;
	float		m_Radius;
	float		m_Opacity;
};


#endif //__ATTENUATIONSPHERE_H

