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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditorINI.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/04/99 3:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "EditorINI.H"
#include "b64pipe.h"
#include "b64straw.h"
#include "xstraw.h"
#include "xpipe.h"
#include "xstraw.h"
#include "INISup.h"

///////////////////////////////////////////////////////////////////
//
//	Get_Vector3
//
Vector3
EditorINIClass::Get_Vector3
(
	LPCTSTR section,
	LPCTSTR entry,
	const Vector3 &defvalue
)  const
{
	if (section != NULL && entry != NULL) {
		INIEntry * entryptr = Find_Entry(section, entry);
		if (entryptr && entryptr->Value != NULL) {
			Vector3	ret;
			if ( sscanf( entryptr->Value, "%f,%f,%f", &ret[0], &ret[1], &ret[2] ) == 3 ) {
				return ret;
			}
		}
	}

	return defvalue;
}


///////////////////////////////////////////////////////////////////
//
//	Get_Vector4
//
Vector4
EditorINIClass::Get_Vector4
(
	LPCTSTR section,
	LPCTSTR entry,
	const Vector4 &defvalue
) const
{
	if (section != NULL && entry != NULL) {
		INIEntry * entryptr = Find_Entry(section, entry);
		if (entryptr && entryptr->Value != NULL) {
			Vector4	ret;
			if ( sscanf( entryptr->Value, "%f,%f,%f,%f", &ret[0], &ret[1], &ret[2], &ret[3] ) == 4 ) {
				return ret;
			}
		}
	}

	return defvalue;
}


///////////////////////////////////////////////////////////////////
//
//	Get_CString
//
CString
EditorINIClass::Get_CString
(
	LPCTSTR section,
	LPCTSTR entry,
	LPCTSTR defvalue
) const
{
	// Read the string from the INI file
	TCHAR temp_string[128];
	Get_String (section, entry, defvalue, temp_string, sizeof (temp_string));

	// Convert the string to a CString object and return it to the caller
	return CString (temp_string);
}


///////////////////////////////////////////////////////////////////
//
//	Get_Matrix3D
//
Matrix3D
EditorINIClass::Get_Matrix3D
(
	LPCTSTR section,
	LPCTSTR entry,
	const Matrix3D &defvalue
) const
{
	// check input parameters
	if ( section == NULL || entry == NULL ) {
		return defvalue;
	}

	// read UU string
	char buffer[128];
	int length = Get_String(section, entry, "", buffer, sizeof(buffer));

	Matrix3D	tm(1);
	if (buffer[0] != 0) {
		ASSERT( strlen( buffer ) == 4*4*4 );

		// convert UU string to Matrix3D
		
		ASSERT( sizeof( Matrix3D ) == 3*4*4 );
		Base64Pipe b64pipe(Base64Pipe::DECODE);
		BufferPipe bpipe((void*)&tm, sizeof( Matrix3D ) );
		b64pipe.Put_To(&bpipe);
		int outcount = b64pipe.Put(buffer, length);
		outcount += b64pipe.End();
		ASSERT( outcount == sizeof( Matrix3D ) );
	}

	// Pass the matrix back to the caller
	return tm;
}


///////////////////////////////////////////////////////////////////
//
//	Put_Matrix3D
//
bool
EditorINIClass::Put_Matrix3D
(
	LPCTSTR section,
	LPCTSTR entry,
	const Matrix3D &value
) 
{
	// check input parameters
	if ( section == NULL || entry == NULL ) {
		return false;
	}

	// convert Matrix3D to UU string
	ASSERT( sizeof( Matrix3D ) == 3*4*4 );
	BufferStraw straw( (void*)&value, sizeof( Matrix3D ) );
	Base64Straw bstraw(Base64Straw::ENCODE);
	bstraw.Get_From(straw);
	static char	mat_str[4*4*4 + 1];
	int length = bstraw.Get(mat_str, sizeof(mat_str)-1);
	ASSERT( length == 4*4*4 );
	mat_str[length] = '\0';

	// save the UU string
	Put_String(section, entry, mat_str);

	// Return the true result code
	return true;
}


///////////////////////////////////////////////////////////////////
//
//	Put_Vector3
//
bool
EditorINIClass::Put_Vector3
(
	LPCTSTR section,
	LPCTSTR entry,
	const Vector3 &value
) 
{
	// Assume failure
	bool retval = false;

	// Convert the vector to a string and write the string to the INI
	CString string_value;
	string_value.Format ("%f, %f, %f", value.X, value.Y, value.Z);
	retval = Put_String (section, entry, (LPCTSTR)string_value);
	
	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Put_Vector4
//
bool
EditorINIClass::Put_Vector4
(
	LPCTSTR section,
	LPCTSTR entry,
	const Vector4 &value
) 
{
	// Assume failure
	bool retval = false;

	// Convert the vector to a string and write the string to the INI
	CString string_value;
	string_value.Format ("%f, %f, %f, %f", value.X, value.Y, value.Z, value.W);
	retval = Put_String (section, entry, (LPCTSTR)string_value);
	
	// Return the true/false result code
	return retval;
}


