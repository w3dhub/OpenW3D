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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/StringPicker.cpp             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/22/01 3:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "stringpicker.h"
#include "stringpickermaindialog.h"
#include "translatedb.h"
#include "translateobj.h"


/////////////////////////////////////////////////////////////////////////
//
//	StringPickerClass
//
/////////////////////////////////////////////////////////////////////////
StringPickerClass::StringPickerClass (void)
	:	m_EntryID (0)
{	
	m_Icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_LIST), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	~StringPickerClass
//
/////////////////////////////////////////////////////////////////////////
StringPickerClass::~StringPickerClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	On_Pick
//
/////////////////////////////////////////////////////////////////////////
void
StringPickerClass::On_Pick (void)
{
	//
	//	Show the dialog to the user so they can pick a string
	//
	StringPickerMainDialogClass dialog (this);
	dialog.Set_String_ID (m_EntryID);
	if (dialog.DoModal () == IDOK) {
		
		m_EntryID = dialog.Get_String_ID ();

		//
		//	Change the text of the window control
		//
		TDBObjClass *object = TranslateDBClass::Find_Object (m_EntryID);
		if (object != NULL) {
			const StringClass &text = object->Get_ID_Desc ();
			SetWindowText (text);
		} else {
			SetWindowText ("");
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Set_Entry
//
/////////////////////////////////////////////////////////////////////////
void
StringPickerClass::Set_Entry (int entry_id)
{
	m_EntryID = entry_id;

	//
	//	Change the text of the window control
	//
	TDBObjClass *object = TranslateDBClass::Find_Object (entry_id);
	if (object != NULL) {
		const StringClass &text = object->Get_ID_Desc ();
		SetWindowText (text);
	} else {
		SetWindowText ("");
	}

	return ;
}
