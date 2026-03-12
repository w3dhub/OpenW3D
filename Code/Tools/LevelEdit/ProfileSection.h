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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ProfileSection.h                                                                                                                                        $Author:: Patrick_s                                                    *
 *                                                                                             *
 *                     $Modtime:: 4/15/99 6:30p                                                                                                                                                                             $Revision:: 1                                                            *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PROFILE_SECTION_CLASS_H
#define __PROFILE_SECTION_CLASS_H


/////////////////////////////////////////////////////////////////////////////
//
// ProfileSectionClass
//
class ProfileSectionClass
{
	public:

		///////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//
		ProfileSectionClass (LPCTSTR section_data)
			: m_pszSectionData (section_data) { Before_First (); }

		virtual ~ProfileSectionClass (void) {}


		///////////////////////////////////////////////////
		//
		//	Public methods
		//
		void				Before_First (void) { m_pszCurrentKey = m_pszSectionData; }
		bool				Get_Next_Key (CString *key_name, CString *key_value);

	private:

		///////////////////////////////////////////////////
		//
		//	Private constructors/destructors
		//
		ProfileSectionClass (void) {}


		///////////////////////////////////////////////////
		//
		//	Private member data
		//
		LPCTSTR m_pszSectionData;
		LPCTSTR m_pszCurrentKey;
};


#endif //__PROFILE_SECTION_CLASS_H
