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
 *                     $Archive:: /Commando/Code/wwui/popupdialog.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/02/01 2:36p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __POPUP_DIALOG_H
#define __POPUP_DIALOG_H


#include "dialogbase.h"
#include "dialogspec.h"
#include "rendobj.h"
#include "wwstring.h"
#include "render2dsentence.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class SimpleSceneClass;
class CameraClass;


////////////////////////////////////////////////////////////////
//
//	PopupDialogClass
//
////////////////////////////////////////////////////////////////
class PopupDialogClass : public DialogBaseClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	PopupDialogClass (const DialogResource *dialog_resource);
	virtual ~PopupDialogClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//
	virtual PopupDialogClass *	As_PopupDialogClass (void) override	{ return this; }

	//
	//	Display methods
	//
	virtual void				Render (void) override;

	virtual void Set_Title(const unichar_t* title);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				Build_Background_Renderers (void);
	
	//
	// Set whether or not background is darkened
	//
	void				Set_Background_Darkened(bool flag)		{ IsBackgroundDarkened = flag; }

	//
	//	From DialogBaseClass
	//
	void				On_Init_Dialog (void) override;


	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////	
	Render2DClass				BlackoutRenderer;
	Render2DClass				BackgroundRenderer;
	Render2DSentenceClass	TextRenderer;
	bool							IsBackgroundDarkened;

};


#endif //__POPUP_DIALOG_H

