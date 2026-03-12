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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PresetPropSheet.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/12/00 3:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PRESETPROPSHEET_H
#define __PRESETPROPSHEET_H

#include "editorpropsheet.h"


/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
class PresetClass;


/////////////////////////////////////////////////////////////////////////////
//
// PresetPropSheetClass
//
/////////////////////////////////////////////////////////////////////////////
class PresetPropSheetClass : public EditorPropSheetClass
{
public:

	/////////////////////////////////////////////////////////////////////////////
	//	Public constructor
	/////////////////////////////////////////////////////////////////////////////
	PresetPropSheetClass (void)	:	m_Preset (NULL)	{ }

	/////////////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////////////

	//
	//	From CDialog
	//
	BOOL		OnInitDialog (void);
	void		OnOk (void);
	BOOL		OnCommand(WPARAM wParam, LPARAM lParam);

	//
	//	From EditorPropSheetClass
	//
	void		Reposition_Buttons (int cx, int cy);

	//
	//	PresetPropSheetClass specific
	//
	void		Set_Preset (PresetClass *preset)	{ m_Preset = preset; }

private:

	/////////////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////////////
	PresetClass *	m_Preset;
};


#endif //__PRESETPROPSHEET_H

