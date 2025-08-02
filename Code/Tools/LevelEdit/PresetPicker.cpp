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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PresetPicker.cpp             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/16/00 4:20p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "presetpicker.h"
#include "selectpresetdialog.h"
#include "preset.h"
#include "resource.h"


/////////////////////////////////////////////////////////////////////////
//
//	PresetPickerClass
//
/////////////////////////////////////////////////////////////////////////
PresetPickerClass::PresetPickerClass (void)
	:	m_Preset (NULL),
		m_ClassID (0),
		m_IconIndex (0)
{	
	m_Icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_PRESET_TINY), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	~PresetPickerClass
//
/////////////////////////////////////////////////////////////////////////
PresetPickerClass::~PresetPickerClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	On_Pick
//
/////////////////////////////////////////////////////////////////////////
void
PresetPickerClass::On_Pick (void)
{
	//
	//	Show the dialog to the user so they can pick a preset
	//
	SelectPresetDialogClass dialog (this);
	dialog.Set_Class_ID (m_ClassID);
	dialog.Set_Icon_Index (m_IconIndex);
	dialog.Set_Preset (m_Preset);
	if (dialog.DoModal () == IDOK) {
		
		PresetClass *preset = dialog.Get_Selection ();
		Set_Preset (preset);		
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Set_Preset
//
/////////////////////////////////////////////////////////////////////////
void
PresetPickerClass::Set_Preset (PresetClass *preset)
{
	//
	//	Build a new text string to display
	//
	CString new_text;
	if (preset != NULL) {
		new_text = preset->Get_Name ();
	}

	//
	//	Put the new text into the control
	//
	SetWindowText (new_text);

	m_Preset = preset;	
	return ;
}

