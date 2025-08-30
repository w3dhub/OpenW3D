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
 *                     $Archive:: /Commando/Code/commando/mainmenutransition.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/27/01 2:59p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MAIN_MENU_TRANSITION_H
#define __MAIN_MENU_TRANSITION_H


#include "dialogtransition.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class RenderObjClass;
class HAnimClass;
class CameraClass;
class DialogBaseClass;


////////////////////////////////////////////////////////////////
//
//	MainMenuDialogClass
//
////////////////////////////////////////////////////////////////

class MainMenuTransitionClass : public DialogTransitionClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MainMenuTransitionClass (void);
	~MainMenuTransitionClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	From DialogTransitionClass
	//
	void	On_Frame_Update (void) override;
	void	Set_Type (TYPE type) override;
	bool	Is_Complete (void) override					{ return (CurrentFrame >= TargetFrame); }

	//
	//	Configuration
	//
	void	Set_Camera (CameraClass *camera)	{ Camera = camera; }
	void	Set_Model (RenderObjClass *model);	
	void	Set_Dialogs (DialogBaseClass *dialog, DialogBaseClass *other_dlg);

	//
	//	Validity check
	//
	bool	Is_Valid (void) const;
	
private:	

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void	Update_Controls (void);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	DialogBaseClass *	Dialog;
	DialogBaseClass *	OtherDlg;
	RenderObjClass *	Model;
	HAnimClass *		TransitionAnim;
	CameraClass *		Camera;
	float					CurrentFrame;
	float					TargetFrame;

};


#endif //__MAIN_MENU_TRANSITION_H

