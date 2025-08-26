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
 *                     $Archive:: /Commando/Code/Installer/Translator.h $*		
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 12/13/01 5:30p                $* 
 *                                                                                             * 
 *                    $Revision:: 6                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _TRANSLATOR_H
#define _TRANSLATOR_H


// Includes
#include "string_ids.h"
#include "Translatedb.h"
#include "wwstring.h"
#include "widestring.h"


class RxStringClass : public StringClass
{
	public:
		// Get a string from the resource in ANSI format. 
		RxStringClass (int resourceid);	
};


class RxWideStringClass : public WideStringClass
{
	public:
		// Get a string from the resource in wide (Unicode) format.
		RxWideStringClass (int resourceid);
};


class TxWideStringClass : public WideStringClass
{
	public:
		// Get a string from the translation database in wide format.
		TxWideStringClass (int databaseid, int resourceid = -1);
};


#endif // TRANSLATOR_H



