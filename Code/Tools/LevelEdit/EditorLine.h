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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditorLine.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/26/01 2:19p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EDITOR_LINE_H
#define __EDITOR_LINE_H

#include "editorphys.h"
#include "segline.h"
#include "vector3.h"

///////////////////////////////////////////////////////////////////////////////
//
//	EditorLineClass
//
///////////////////////////////////////////////////////////////////////////////
class EditorLineClass : public EditorPhysClass
{
public:

	////////////////////////////////////////////////////////
	// Public contsructors/destructors
	////////////////////////////////////////////////////////
	EditorLineClass (void);
	virtual ~EditorLineClass (void) { MEMBER_RELEASE (Line); }


	////////////////////////////////////////////////////////
	// Public methods
	////////////////////////////////////////////////////////

	//
	//	Position methods
	//
	void				Reset (const Vector3 &start, const Vector3 &end);
	void				Reset_Start (const Vector3 &start);
	void				Reset_End (const Vector3 &end);

	void				Hide (bool onoff);

	void				Set_Color (const Vector3 &color)		{ Line->Set_Color ((Vector3 &)color); }
	void				Set_Width (float width)					{ Line->Set_Width (width); Update_Cull_Box (); }

private:

	////////////////////////////////////////////////////////
	// Private member data
	////////////////////////////////////////////////////////
	Vector3					StartPos;
	Vector3					EndPos;
	SegmentedLineClass *	Line;
};


///////////////////////////////////////////////////////////////////////////////////
// EditorLineClass
///////////////////////////////////////////////////////////////////////////////////
inline
EditorLineClass::EditorLineClass (void)	:
	StartPos (0, 0, 0),
	EndPos (1, 1, 1),
	EditorPhysClass ()
{
	Line = new SegmentedLineClass;

	Vector3 locs[2] = { Vector3 (0,0,0), Vector3 (1,1,1) };
	Line->Set_Points (2, locs);

	Line->Set_Width (0.075F);
	Line->Set_Color (Vector3 (0, 0, 0.75F));
	Line->Set_Opacity (1.0F);
	//Line->Set_Shader (ShaderClass::_PresetOpaqueSolidShader);

	Set_Model (Line);
	return ;
}

///////////////////////////////////////////////////////////////////////////////////
// Reset
///////////////////////////////////////////////////////////////////////////////////
inline void
EditorLineClass::Reset (const Vector3 &start, const Vector3 &end)
{
	Reset_Start (start);
	Reset_End (end);
	return ;
}

///////////////////////////////////////////////////////////////////////////////////
// Reset_Start
///////////////////////////////////////////////////////////////////////////////////
inline void
EditorLineClass::Reset_Start (const Vector3 &start)
{
	StartPos = start;
	Set_Position (StartPos);
	Update_Cull_Box ();
	return ;
}

///////////////////////////////////////////////////////////////////////////////////
// Reset_End
///////////////////////////////////////////////////////////////////////////////////
inline void
EditorLineClass::Reset_End (const Vector3 &end)
{
	Matrix3D::Inverse_Transform_Vector (Get_Transform (), end, &EndPos);
	Line->Set_Point_Location (1, EndPos);
	Update_Cull_Box ();
	return ;
}

///////////////////////////////////////////////////////////////////////////////////
// Hide
///////////////////////////////////////////////////////////////////////////////////
inline void
EditorLineClass::Hide (bool onoff)
{
	if (Line != NULL) {
		Line->Set_Hidden (onoff);
	}
	return ;
}


#endif //__EDITOR_LINE_H

