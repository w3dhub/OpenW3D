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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ProgressUI.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/07/99 5:41p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "ProgressUI.h"


///////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////
ProgressUIClass *	ProgressUIMgrClass::_CurrentProgressUI = NULL;


///////////////////////////////////////////////////////////////////////
//
//	Set_Status_Text
//
///////////////////////////////////////////////////////////////////////
void
ProgressUIMgrClass::Set_Status_Text (LPCTSTR text)
{
	if (_CurrentProgressUI != NULL) {
		_CurrentProgressUI->Set_Status_Text (text);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Progress_Percent
//
///////////////////////////////////////////////////////////////////////
void
ProgressUIMgrClass::Set_Progress_Percent (float percent)
{
	if (_CurrentProgressUI != NULL) {
		_CurrentProgressUI->Set_Progress_Percent (percent);
	}

	return ;
}


