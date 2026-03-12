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
 *                     $Archive:: /Commando/Code/Combat/gameobjref.h                          $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/06/01 2:30p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	GAMEOBJREF_H
#define	GAMEOBJREF_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	REFLIST_H
	#include "reflist.h"
#endif

/*
**
*/
class		ScriptableGameObj;
typedef	ScriptableGameObj	*									GameObjRefPtr;
typedef	ReferenceableClass<ScriptableGameObj>			ReferenceableGameObj;
typedef	ReferencerClass										GameObjReference;
typedef	RefCountedReferencerClass							RefCountedGameObjReference;

#endif	//	GAMEOBJREF_H
