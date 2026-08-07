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
 *                 Project Name : combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/cheatmgr.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/23/01 3:38p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "cheatmgr.h"
#include "always.h"


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
CheatMgrClass *	CheatMgrClass::_TheInstance	 = nullptr;



//////////////////////////////////////////////////////////////////////
//
//	CheatMgrClass
//
//////////////////////////////////////////////////////////////////////
CheatMgrClass::CheatMgrClass (void)	:
	Flags (0),
	HistoryFlags (0)
{
	_TheInstance = this;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~CheatMgrClass
//
//////////////////////////////////////////////////////////////////////
CheatMgrClass::~CheatMgrClass (void)
{
	_TheInstance = nullptr;
	return ;
}


