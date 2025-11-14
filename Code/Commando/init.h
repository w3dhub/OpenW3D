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
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/init.h                              $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/16/02 11:10a                                             $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef INIT_H
#define INIT_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#include "ffactory.h"


void	Get_Version_Number(unsigned int *major, unsigned int *minor);
bool	Game_Init(void);
void	Install_Fonts (bool onoff);
void	Application_Exception_Callback(void);
void	Copy_Logs(unsigned version);

#define VALUE_NAME_DISABLE_SERVER_SYSINFO_COLLECTING "DisableServerSysInfoCollecting"
#define VALUE_NAME_GAME_INITIALIZATION_IN_PROGRESS "GameInitInProgress"
#define VALUE_NAME_APPLICATION_CRASH_VERSION "ApplicationCrashVersion"
#define VALUE_NAME_DISABLE_LOG_COPYING "DisableLogCopying"

extern SimpleFileFactoryClass	RenegadeBaseFileFactory;

#endif