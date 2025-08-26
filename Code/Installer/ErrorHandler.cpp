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
 *                     $Archive:: /Commando/Code/Installer/ErrorHandler.cpp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/25/01 6:56p                $* 
 *                                                                                             * 
 *                    $Revision:: 7                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "ErrorHandler.h"
#include "CodeControl.h"
#include "resource.h"
#include "Translator.h"
#include "Win.h"


// Defines.
#if VERBOSE_ERROR_MESSAGES
#define SYSTEM_ERROR_FORMAT_STRING			L"%s\nFile: %s\nLine: %d\n"
#define APPLICATION_ERROR_FORMAT_STRING	L"%s\n\nFile: %s\nLine: %d\n"
#define CAB_ERROR_FORMAT_STRING				L"%s %d\n\nFile: %s\nLine: %d\n"
#else
#define SYSTEM_ERROR_FORMAT_STRING			L"%s"
#define APPLICATION_ERROR_FORMAT_STRING	L"%s"
#define CAB_ERROR_FORMAT_STRING				L"%s %d"
#endif


/***********************************************************************************************
 * Handle_Fatal_System_Error --																					  *
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
void Handle_Fatal_System_Error (int errorcode, const char *filename, int sourceline)
{
	WideStringClass errormessage, messagebody;
	LPVOID			 messagebuffer;

	FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
 						NULL,
						errorcode,
						MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR) &messagebuffer,
						0,
						NULL);

	messagebody = (TCHAR*) messagebuffer;
	LocalFree (messagebuffer);
	
	#if VERBOSE_ERROR_MESSAGES
	errormessage.Format (SYSTEM_ERROR_FORMAT_STRING, messagebody, WideStringClass (filename), sourceline);
	#else
	errormessage.Format (SYSTEM_ERROR_FORMAT_STRING, messagebody);
	#endif

	throw (errormessage);
}


/***********************************************************************************************
 * Handle_Fatal_Application_Error --																			  *
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
void Handle_Fatal_Application_Error (int errorcode, const char *filename, int sourceline)
{
	WideStringClass errormessage;

	#if VERBOSE_ERROR_MESSAGES
	errormessage.Format (APPLICATION_ERROR_FORMAT_STRING, TxWideStringClass (errorcode), WideStringClass (filename), sourceline);
	#else
	errormessage.Format (APPLICATION_ERROR_FORMAT_STRING, TxWideStringClass (errorcode));
	#endif

	throw (errormessage);
}


/***********************************************************************************************
 * Handle_Fatal_Cab_Error --																						  *
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
void Handle_Fatal_Cab_Error (int errorcode, const char *filename, int sourceline)
{
	WideStringClass errormessage;

	#if VERBOSE_ERROR_MESSAGES
	errormessage.Format (CAB_ERROR_FORMAT_STRING, TxWideStringClass (IDS_CAB_ERROR), errorcode, WideStringClass (filename), sourceline);
	#else
	errormessage.Format (CAB_ERROR_FORMAT_STRING, TxWideStringClass (IDS_CAB_ERROR), errorcode);
	#endif

	throw (errormessage);
}
