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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditorINI.h                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/20/99 11:22a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EDITORINI_H
#define __EDITORINI_H

#include "INI.H"
#include "Vector3.H"
#include "Vector4.H"
#include "Matrix3D.H"


////////////////////////////////////////////////////////////////////////////////
//
//	EditorINIClass
//
class EditorINIClass : public INIClass
{
	public:

		EditorINIClass (void)
			: INIClass () {}

		EditorINIClass (FileClass &file)
			: INIClass (file) {}

		//
		//	Read methods
		//
		Vector3	Get_Vector3 (LPCTSTR section, LPCTSTR entry, const Vector3 &defvalue = Vector3 (0,0,0)) const;
		Vector4	Get_Vector4 (LPCTSTR section, LPCTSTR entry, const Vector4 &defvalue = Vector4 (0,0,0,0)) const;
		CString	Get_CString (LPCTSTR section, LPCTSTR entry, LPCTSTR defvalue = "") const;
		Matrix3D	Get_Matrix3D (LPCTSTR section, LPCTSTR entry_base, const Matrix3D &defvalue = Matrix3D(1)) const;

		//
		//	Write methods
		//
		bool		Put_Vector3 (LPCTSTR section, LPCTSTR entry, const Vector3 &value);
		bool		Put_Vector4 (LPCTSTR section, LPCTSTR entry, const Vector4 &value);
		bool		Put_Matrix3D (LPCTSTR section, LPCTSTR entry_base, const Matrix3D &value);
};

#endif //__EDITORINI_H

