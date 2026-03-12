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
 *                     $Archive:: /Commando/Code/Commando/commandochunkids.h                  $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 4/13/00 1:19p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	COMMANDOCHUNKID_H
#define	COMMANDOCHUNKID_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	SAVELOADIDS_H
	#include "saveloadids.h"
#endif

//#ifndef	DEFINITIONCLASSIDS_H
//	#include "definitionclassids.h"
//#endif


/*
** CHUNKIDs
*/
enum {
	CHUNKID_COMMANDO								= CHUNKID_COMMANDO_BEGIN,
	CHUNKID_COMMANDO_SOLDIER_OBSERVER,
};


/*
** CLASSIDs
*/
enum {
//	CLASSID_GAME_OBJECT_DEF_SOLDIER			= 	CLASSID_GAME_OBJECTS + 1,
};


#endif	//	COMMANDOCHUNKID_H
