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
 *                 Project Name : Installer                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Installer/Translator.cpp $*		
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 12/13/01 5:31p                $* 
 *                                                                                             * 
 *                    $Revision:: 5                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "Translator.h"
#include "resource.h"
#include "Win.h"
#include <stdio.h>


// Defines.
#define RESOURCE_NOT_FOUND_STRING		 "[Resource %d not found]"
#define WIDE_RESOURCE_NOT_FOUND_STRING L"[Resource %d not found]"
	

/***********************************************************************************************
 * RxStringClass::RxStringClass --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
RxStringClass::RxStringClass (int resourceid)
	: StringClass()
{
	int	charactercount;
	TCHAR stringbuffer [1024];

	charactercount = LoadString (ProgramInstance, resourceid, stringbuffer, sizeof (stringbuffer) / sizeof (TCHAR));
	if (charactercount == 0) {
		Format (RESOURCE_NOT_FOUND_STRING, resourceid);
	} else {
		*((StringClass*) this) = stringbuffer;
	}
}


/***********************************************************************************************
 * RxWideStringClass::RxWideStringClass --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
RxWideStringClass::RxWideStringClass (int resourceid)
	: WideStringClass()
{
	int   charactercount;
	TCHAR stringbuffer [1024];

	charactercount = LoadString (ProgramInstance, resourceid, stringbuffer, sizeof (stringbuffer) / sizeof (TCHAR));
	if (charactercount == 0) {
		Format (WIDE_RESOURCE_NOT_FOUND_STRING, resourceid);
	} else {
		*((WideStringClass*) this) = stringbuffer;
	}
}


/***********************************************************************************************
 * TxWideStringClass::TxWideStringClass --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
TxWideStringClass::TxWideStringClass (int databaseid, int resourceid)
	: WideStringClass()
{
	// In the event that the translation database has not been loaded see if there is a substitute in the resource.
	if (!TranslateDBClass::Is_Loaded()) {

		switch (databaseid) {

			case IDS_APPLICATION_ERROR:
				*((WideStringClass*) this) = RxStringClass (IDS_RESOURCE_APPLICATION_ERROR);
				break;

			default:
				*((WideStringClass*) this) = TRANSLATE (databaseid);
				break;
		}
	
	} else {
	
		if (resourceid != -1) {

			StringClass multibytestring;

			// If the translation cannot be converted to multi-byte format then substitute the resource.
			*((WideStringClass*) this) = TRANSLATE (databaseid);
			if (!multibytestring.Copy_Wide (*this)) {
				*((WideStringClass*) this) = RxStringClass (resourceid);
			}

		} else {
			*((WideStringClass*) this) = TRANSLATE (databaseid);
		}
	}
}
