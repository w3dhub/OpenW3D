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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : LevelEdit                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ProfileSection.cpp                                                                                                                                       $Author:: Patrick_s                                                    * 
 *                                                                                             * 
 *                     $Modtime:: 2/23/99 10:26a                                                                                                                                                                            $Revision:: 1                                                            * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "ProfileSection.H"


/////////////////////////////////////////////////////////////////////////////
//
// Get_Next_Key
//
bool
ProfileSectionClass::Get_Next_Key
(
	CString *key_name,
	CString *key_value
)
{
	// Assume failure
	bool ret_val = false;

	// State OK?
	ASSERT (m_pszCurrentKey);
	if (m_pszCurrentKey &&
	    (m_pszCurrentKey[0] != 0)) {

		// Make a temporary local copy of the key data
		TCHAR local_copy[256];
		::lstrcpyn (local_copy, m_pszCurrentKey, sizeof (local_copy));

		// Get a pointer to the divider
		LPTSTR temp_string = ::strchr (local_copy, '=');
		temp_string[0] = 0;
		temp_string += 1;
		
		//
		//	Strip off the comments (if any)
		//
		LPTSTR comment = ::strchr (local_copy, ';');
		if (comment != NULL) {
			comment[0] = 0;
		}

		comment = ::strchr (temp_string, ';');
		if (comment != NULL) {
			comment[0] = 0;
		}

		// Did the caller want the key name back?
		if (key_name != NULL) {
			// Everything on the left side of the divider is the key name
			*key_name = local_copy;
			key_name->TrimLeft ();
			key_name->TrimRight ();
		}

		// Did the caller want the key value back?
		if (key_value != NULL) {
			// Everything on the right side of the divider is the key value
			*key_value = temp_string;
			key_value->TrimLeft ();
			key_value->TrimRight ();
		}

		// Advance the pointer to point to the next key in the section
		m_pszCurrentKey += ::lstrlen (m_pszCurrentKey) + 1;

		// Success!
		ret_val = true;
	}

	// Return the true/false result code
	return ret_val;
}

