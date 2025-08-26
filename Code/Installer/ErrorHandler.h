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
 *                     $Archive:: /Commando/Code/Installer/ErrorHandler.h  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 10/25/01 10:07p               $* 
 *                                                                                             * 
 *                    $Revision:: 4                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _ERROR_HANDLER_H
#define _ERROR_HANDLER_H

// Includes.
#include "resource.h"

#define FATAL_SYSTEM_ERROR				Handle_Fatal_System_Error (GetLastError(), __FILE__, __LINE__)
#define FATAL_APP_ERROR(errorcode)	Handle_Fatal_Application_Error (errorcode, __FILE__, __LINE__)
#define FATAL_CAB_ERROR(errorcode) 	Handle_Fatal_Cab_Error (errorcode, __FILE__, __LINE__)

void Handle_Fatal_System_Error (int errorcode, const char *filename, int sourceline);
void Handle_Fatal_Application_Error (int errorcode, const char *filename, int sourceline);
void Handle_Fatal_Cab_Error (int errorcode, const char *filename, int sourceline);

#endif // _ERROR_HANDLER_H
