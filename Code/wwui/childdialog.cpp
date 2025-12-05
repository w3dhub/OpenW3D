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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/childdialog.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/16/01 8:20a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "childdialog.h"
#include "assetmgr.h"
#include "texture.h"
#include "stylemgr.h"


////////////////////////////////////////////////////////////////
//
//	ChildDialogClass
//
////////////////////////////////////////////////////////////////
ChildDialogClass::ChildDialogClass (const DialogResource *dialog_resource)	:
	ParentDialog (NULL),
	DialogBaseClass (dialog_resource)
{	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ChildDialogClass
//
////////////////////////////////////////////////////////////////
ChildDialogClass::~ChildDialogClass (void)
{	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
ChildDialogClass::On_Init_Dialog (void)
{
	DialogBaseClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
ChildDialogClass::Render (void)
{
	//
	//	Only render the controls... don't render any
	// backdrop or border.
	//
	DialogBaseClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Active
//
////////////////////////////////////////////////////////////////
bool
ChildDialogClass::Is_Active (void)
{
	bool retval = false;

	//
	//	We are automatically active if our parent is active
	//
	if (IsVisible && ParentDialog != NULL && ParentDialog->Is_Active ()) {
		//retval = true;
	}

	return retval;
}
