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
 *                     $Archive:: /Commando/Code/wwui/childdialog.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/17/01 5:37p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __CHILD_DIALOG_H
#define __CHILD_DIALOG_H


#include "dialogbase.h"
#include "dialogspec.h"
#include "wwstring.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
//
//	ChildDialogClass
//
////////////////////////////////////////////////////////////////
class ChildDialogClass : public DialogBaseClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ChildDialogClass (const DialogResource *dialog_resource);
	virtual ~ChildDialogClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//
	virtual ChildDialogClass *	As_ChildDialogClass (void) override	{ return this; }

	//
	//	From DialogBaseClass
	//
	virtual void			Render (void) override;
	virtual bool			Is_Active (void) override;
	virtual bool			Wants_Activation (void) override	{ return false; }

	//
	//	Parent dialog access
	//
	void						Set_Parent_Dialog (DialogBaseClass *dialog)	{ ParentDialog = dialog; }
	DialogBaseClass *		Get_Parent_Dialog (void) const					{ return ParentDialog; }

	//
	//	Data management
	//
	//		Note: Returning false from either method will keep
	// the dialog from advancing.
	//
	virtual bool			On_Apply (void)	{ return true; }
	virtual bool			On_Discard (void)	{ return true; }

	virtual void			On_Reload (void)	{ }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogBaseClass
	//
	virtual void			On_Init_Dialog (void) override;

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////	
	DialogBaseClass *		ParentDialog;
};


#endif //__CHILD_DIALOG_H

