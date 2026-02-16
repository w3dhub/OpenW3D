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
 *                     $Archive:: /Commando/Code/commando/dlgmpingamechat.h       $*
 *                                                                                             *
 *                       Author:: Tom SS                                                *
 *                                                                                             *
 *                     $Modtime:: 2/19/02 2:28p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGMPINGAMECHAT_H__
#define __DLGMPINGAMECHAT_H__

#include "popupdialog.h"
#include "childdialog.h"
#include "sctextobj.h"


////////////////////////////////////////////////////////////////
//
//	MPChatChildDialogClass
//
////////////////////////////////////////////////////////////////
class MPChatChildDialogClass : public ChildDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MPChatChildDialogClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Configuration
	//
	void	Set_Default_Type (TextMessageEnum message_type)		{ MessageType = message_type; }
	void	Set_End_Dialog_On_Send (bool onoff)						{ EndDialogOnSend = onoff; }

	//
	//	Inherited
	//
	void	On_Init_Dialog (void) override;
	bool	On_EditCtrl_Key_Down (EditCtrlClass *edit_ctrl, uint32 key_id, uint32 key_data) override;
	void	On_EditCtrl_Change(EditCtrlClass *edit_ctrl, int ctrl_id) override;
	void	On_EditCtrl_Enter_Pressed (EditCtrlClass *edit_ctrl, int ctrl_id) override;
	void	On_Command (int ctrl_id, int message_id, unsigned int param) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////	
	void Process_Message(void);
	void Send_Message(WideStringClass& message, TextMessageEnum type, int recipientID);
	bool Process_Commands(const unichar_t* message);

	//
	//	Auto completetion
	//
	void	Auto_Complete_Name (void);
	void	Complete_Player_Name (const unichar_t *typed_name, WideStringClass &completed_name);
	bool	Find_Current_Command (const unichar_t *	message, int &start_index, int &end_index);


	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	TextMessageEnum	MessageType;
	WideStringClass	RecipientName;
	WideStringClass	CurrRecipientName;
	bool					TestForAutoCompletion;
	bool					EndDialogOnSend;
};


////////////////////////////////////////////////////////////////
//
//	MPIngameChatPopupClass
//
////////////////////////////////////////////////////////////////
class MPIngameChatPopupClass : public PopupDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MPIngameChatPopupClass (void);
	~MPIngameChatPopupClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Configuration
	//
	void	Set_Default_Type (TextMessageEnum message_type)		{ DefaultType = message_type; }

	//
	//	Inherited
	//
	void	On_Init_Dialog (void) override;
	void	On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void	Render (void) override;

	//
	//	Callbacks
	//
	//void	On_ComboBoxCtrl_Sel_Change (ComboBoxCtrlClass *combo_ctrl, int ctrl_id, int old_sel, int new_sel);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void	Send_Message (void);
	void	Configure_Recipient_Combobox (void);
	void	Configure_Message_Log_List (void);


	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DClass					WindowBackgroundRenderer;
	TextMessageEnum				DefaultType;
	MPChatChildDialogClass *	ChatModule;
};


#endif //__DLGMPINGAMECHAT_H__
