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
 *                     $Archive:: /Commando/Code/commando/dlgmpingamechat.cpp       $*
 *                                                                                             *
 *                       Author:: Tom SS                                                *
 *                                                                                             *
 *                     $Modtime:: 2/19/02 2:40p                                               $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpingamechat.h"

#include "cnetwork.h"
#include "dialogcontrol.h"
#include "dialogmgr.h"
#include "renegadedialog.h"
#include "gamedata.h"
#include "cstextobj.h"
#include "sctextobj.h"
#include "comboboxctrl.h"
#include "playermanager.h"
#include "player.h"
#include "listctrl.h"
#include "messagewindow.h"
#include "wolgmode.h"
#include "translatedb.h"
#include "string_ids.h"


static const unichar_t* Get_Parameter_From_String(const unichar_t* command, WideStringClass& param);

////////////////////////////////////////////////////////////////
//
//	MPChatChildDialogClass
//
////////////////////////////////////////////////////////////////
MPChatChildDialogClass::MPChatChildDialogClass (void)	:
	MessageType (TEXT_MESSAGE_PUBLIC),
	TestForAutoCompletion (false),
	EndDialogOnSend (true),
	ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CHAT_MODULE))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPChatChildDialogClass::On_Init_Dialog (void)
{
	WWASSERT (The_Game () != NULL);

	//
	//	Allow the base class to process the message
	//
	ChildDialogClass::On_Init_Dialog ();

	//
	//	Set the text of the type control accordingly
	//
	if (MessageType == TEXT_MESSAGE_TEAM) {
		Set_Dlg_Item_Text (IDC_TYPE_STATIC, TRANSLATE (IDS_MENU_TEAM_MESSAGE));
	} else if (MessageType == TEXT_MESSAGE_PUBLIC) {
		Set_Dlg_Item_Text (IDC_TYPE_STATIC, TRANSLATE (IDS_MENU_PUBLIC_MESSAGE));
	}

	//
	//	Put the focus into the text control
	//
	((EditCtrlClass *)Get_Dlg_Item (IDC_MESSAGE_EDIT))->Set_Text_Limit (100);
	Get_Dlg_Item (IDC_MESSAGE_EDIT)->Set_Focus ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Send_Message
//
////////////////////////////////////////////////////////////////
void
MPChatChildDialogClass::Process_Message (void)
{
	//	Get the text to send...
	WideStringClass message(0, true);
	message = Get_Dlg_Item_Text(IDC_MESSAGE_EDIT);
	message.Trim();

	if (message.Is_Empty() == false) {
		if (Process_Commands(message) == false) {
			TextMessageEnum message_type	= MessageType;
			int recipient_id					= -1;
			bool is_ok_to_send				= true;

			//
			//	Determine who to send the message to...
			//
			if (RecipientName.Is_Empty() == false) {
			
				//
				//	Lookup the player we're sending the message to
				//
				cPlayer *recipient = cPlayerManager::Find_Player (RecipientName);

				if (recipient != NULL) {
					message_type = TEXT_MESSAGE_PRIVATE;
					recipient_id = recipient->Get_Id ();
				} else {
					is_ok_to_send = false;
				}
			}

			//
			// Transmit the message
			//
			if (is_ok_to_send) {
				Send_Message(message, message_type, recipient_id);
			}
		}

		//
		//	Clear the edit control
		//
		Set_Dlg_Item_Text (IDC_MESSAGE_EDIT, U_CHAR(""));
	}

	return ;
}


void MPChatChildDialogClass::Send_Message(WideStringClass& message, TextMessageEnum type, int recipientID)
{
	//
	//	Send the message
	//
	if (cNetwork::I_Am_Client ()) {
		cCsTextObj *event_obj = new cCsTextObj;
		event_obj->Init(message, type, cNetwork::Get_My_Id(), recipientID);
	} else {
		cScTextObj *event_obj = new cScTextObj;
		event_obj->Init(message, type, false, HOST_TEXT_SENDER, recipientID);
	}
}


////////////////////////////////////////////////////////////////
//
// Process Commands
//
////////////////////////////////////////////////////////////////
bool MPChatChildDialogClass::Process_Commands(const unichar_t* message)
{
	// Does this look like a command?
	if (message && message[0] == U_CHAR('/')) {
		// Separate the parameters into individual strings
		WideStringClass command(255, true);
		const unichar_t* curr_pos = Get_Parameter_From_String(&message[1], command);

		if (command.Get_Length() > 0 && curr_pos[0] != 0) {

			// Kick a player from the game
			if (command.Compare_No_Case(U_CHAR("kick")) == 0) {
				GameModeClass* gameMode = GameModeManager::Find("WOL");

				if (gameMode && gameMode->Is_Active()) {
					// Get the name parameter from the string
					WideStringClass user_name(64, true);
					curr_pos = Get_Parameter_From_String(curr_pos, user_name);

					if (user_name.Get_Length() > 0) {
						WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);
						wolGame->Kick_Player(user_name);
					}
				}
			
				return true;
			}

			// Page users outside of this game.
			if (command.Compare_No_Case(U_CHAR("page")) == 0) {
				GameModeClass* gameMode = GameModeManager::Find("WOL");

				if (gameMode && gameMode->Is_Active()) {
					// Get the name parameter from the string
					WideStringClass user_name(64, true);
					curr_pos = Get_Parameter_From_String(curr_pos, user_name);

					if (user_name.Get_Length() > 0) {
						WideStringClass message_str(0, true);
						message_str = curr_pos;
						message_str.Trim();

						// If the user is in this game then just send them a private message.
						cPlayer* recipient = cPlayerManager::Find_Player(user_name);

						if (recipient) {
							int recipient_id = recipient->Get_Id();
							Send_Message(message_str, TEXT_MESSAGE_PRIVATE, recipient_id);
						} else {
							// Page external users.
							WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);
							wolGame->Page_WOL_User(user_name, message_str);
						}
					}
				}
			
				return true;
			}

			// Reply to the last page
			if (command.Compare_No_Case(U_CHAR("r")) == 0) {
				GameModeClass* gameMode = GameModeManager::Find("WOL");

				if (gameMode && gameMode->Is_Active()) {
					WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);

					WideStringClass message_str(0, true);
					message_str = curr_pos;
					message_str.Trim();

					wolGame->Reply_Last_Page(message_str);
				}
			
				return true;
			}

			// Locate a WOL user
			if (command.Compare_No_Case(U_CHAR("locate")) == 0) {
				GameModeClass* gameMode = GameModeManager::Find("WOL");

				if (gameMode && gameMode->Is_Active()) {
					// Get the name parameter from the string
					WideStringClass user_name(64, true);
					curr_pos = Get_Parameter_From_String(curr_pos, user_name);

					if (user_name.Get_Length() > 0) {
						WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);
						wolGame->Locate_WOL_User(user_name);
					}
				}
			
				return true;
			}

			// Invite another WOL user to this game
			if (command.Compare_No_Case(U_CHAR("invite")) == 0) {
				GameModeClass* gameMode = GameModeManager::Find("WOL");

				if (gameMode && gameMode->Is_Active()) {
					// Get the name parameter from the string
					WideStringClass user_name(64, true);
					curr_pos = Get_Parameter_From_String(curr_pos, user_name);

					if (user_name.Get_Length() > 0) {
						WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);

						WideStringClass message_str(0, true);
						message_str = curr_pos;
						message_str.Trim();

						wolGame->Invite_WOL_User(user_name, message_str);
					}
				}
			
				return true;
			}

			// Join another WOL user at their location
			if (command.Compare_No_Case(U_CHAR("join")) == 0) {
				GameModeClass* gameMode = GameModeManager::Find("WOL");

				if (gameMode && gameMode->Is_Active()) {
					// Get the name parameter from the string
					WideStringClass user_name(64, true);
					curr_pos = Get_Parameter_From_String(curr_pos, user_name);

					if (user_name.Get_Length() > 0) {
						WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);
						wolGame->Join_WOL_User(user_name);
					}
				}
			
				return true;
			}
		}
	}

	return false;
}


////////////////////////////////////////////////////////////////
//
// Get_Parameter_From_String
//
////////////////////////////////////////////////////////////////

const unichar_t* Get_Parameter_From_String(const unichar_t* command, WideStringClass& param)
{
	#define LOCAL_STRIP_WHITESPACE(str)	\
		while (str[0] != 0 && str[0] == U_CHAR(' ')) {++str;}

	//	Strip off whitespace
	LOCAL_STRIP_WHITESPACE(command);

	const unichar_t* curr_pos = command;

	//	Look for the first whitespace break
	while (curr_pos[0] != 0 && curr_pos[0] != U_CHAR(' ')) {
		++curr_pos;
	}

	// Return the string contents to the caller
	int length = ((curr_pos + 1) - command);

	if (length > 0) {
		unichar_t* buffer = param.Get_Buffer(length + 1);
		u_strncpy(buffer, command, length);
		buffer[length - 1] = 0;
	}

	//	Strip off whitespace
	LOCAL_STRIP_WHITESPACE(curr_pos);

	return curr_pos;
}


//////////////////////////////////////////////////////////////////////
//
//	Auto_Complete_Name
//
//////////////////////////////////////////////////////////////////////
void	
MPChatChildDialogClass::Auto_Complete_Name (void)
{
	EditCtrlClass *edit_ctrl = (EditCtrlClass *)Get_Dlg_Item (IDC_MESSAGE_EDIT);
	if (edit_ctrl == NULL) {
		return ;
	}

	//
	//	Examine everything left of the caret
	//
	int caret_pos = edit_ctrl->Get_Caret_Pos ();
	if (caret_pos > 0) {

		//
		//	Get the message
		//
		WideStringClass message = Get_Dlg_Item_Text (IDC_MESSAGE_EDIT);
		int message_len = message.Get_Length ();
		if (message_len > 0) {

			//
			//	Check to see if we are typing in a command
			//
			int cmd_start_index	= 0;
			int cmd_end_index		= 0;
			if (Find_Current_Command (message, cmd_start_index, cmd_end_index)) {
				cmd_start_index ++;
			
				//
				//	Try to find the start of the name
				//
				const unichar_t *name_start = message.Peek_Buffer () + cmd_start_index;					

				//
				//	Make a copy of the first part of the message before the command
				//
				WideStringClass first_part (cmd_start_index + 1, true);
				::u_strncpy (first_part.Peek_Buffer (), message, cmd_start_index);
				first_part.Peek_Buffer ()[cmd_start_index] = 0;

				//
				//	Make a copy of the remainder of the message after the command
				//
				WideStringClass last_part (message_len - cmd_end_index, true);
				::u_strcpy (last_part.Peek_Buffer (), message.Peek_Buffer () + cmd_end_index);					
				
				//
				//	Copy the typed characters into their own buffer
				//
				int typed_len = caret_pos - cmd_start_index;
				WideStringClass typed_name (typed_len + 1, true);
				::u_strncpy (typed_name.Peek_Buffer (), name_start, typed_len);
				typed_name.Peek_Buffer ()[typed_len] = 0;

				//
				//	Try to complete the name that the user typed in
				//
				WideStringClass completed_name (0, true);
				Complete_Player_Name (typed_name, completed_name);

				//
				//	Did we find a valid completed name?
				//
				int completed_name_len = completed_name.Get_Length ();
				if (completed_name_len >= typed_len) {
					CurrRecipientName = completed_name;
					
					//
					//	Rebuild the message...
					//
					WideStringClass new_message (first_part);
					new_message += completed_name;
					new_message += last_part;
					
					//
					//	Put the new text into the edit control and reset the caret
					//
					edit_ctrl->Set_Text (new_message);
					edit_ctrl->Set_Caret_Pos (caret_pos);
					
					//
					//	Hilight the auto-completed characters
					//
					int hilight_end = caret_pos + (completed_name_len - typed_len);
					edit_ctrl->Set_Sel (caret_pos, hilight_end);
				} else {
					CurrRecipientName = U_CHAR("");
				}
			}
		}	
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Complete_Player_Name
//
//////////////////////////////////////////////////////////////////////
void	
MPChatChildDialogClass::Complete_Player_Name (const unichar_t *typed_name, WideStringClass &completed_name)
{
	const size_t typed_len = ::u_strlen (typed_name);
	
	//
	//	Require more then one character for any name starting with "R'.  This is
	// so that Denzil's "reply to last page" code will work...
	//
	if (typed_len == 1 && (typed_name[0] == U_CHAR('r') || typed_name[0] == U_CHAR('R'))) {
		return ;
	}

	//
	//	Find the player's name that most closely matches the typed name
	//
	for (	SLNode<cPlayer> *player_node = cPlayerManager::Get_Player_Object_List ()->Head ();
			player_node != NULL; 
			player_node = player_node->Next ())
	{
		cPlayer *player = player_node->Data ();
		WWASSERT (player != NULL);

		if (player->Get_Is_Active().Is_False()) {
			continue;
		}

		const unichar_t *player_name = player->Get_Name ();

		//
		//	Is this the best match so far?
		//
		if (::u_strncasecmp (player_name, typed_name, typed_len, U_COMPARE_CODE_POINT_ORDER) == 0) {
			if (	completed_name.Get_Length () == 0 ||
					::u_strcasecmp (player_name, completed_name, U_COMPARE_CODE_POINT_ORDER) < 0)
			{
				completed_name = player_name;
			}
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Enter_Pressed
//
//////////////////////////////////////////////////////////////////////
void
MPChatChildDialogClass::On_EditCtrl_Enter_Pressed (EditCtrlClass * /* edit_ctrl */, int /* ctrl_id */)
{
	//
	//	Send the message
	//
	Process_Message ();
	
	//
	//	Close the dialog
	//
	if (EndDialogOnSend) {
		Get_Parent_Dialog ()->End_Dialog ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Change
//
//////////////////////////////////////////////////////////////////////
void
MPChatChildDialogClass::On_EditCtrl_Change (EditCtrlClass *edit_ctrl, int /* ctrl_id */)
{
	if (TestForAutoCompletion == false) {
		return ;
	}

	//
	//	Examine everything left of the caret
	//
	int caret_pos = edit_ctrl->Get_Caret_Pos ();
	if (caret_pos > 0) {

		//
		//	Get the message
		//
		WideStringClass message = Get_Dlg_Item_Text (IDC_MESSAGE_EDIT);
		if (message.Get_Length () > 0) {

			//
			//	Check to see if we are typing in a command
			//
			int cmd_start_index	= 0;
			int cmd_end_index		= 0;
			if (Find_Current_Command (message, cmd_start_index, cmd_end_index)) {

				//
				//	Fill in the rest of the name for the user
				//
				Auto_Complete_Name ();
			}
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Find_Current_Command
//
//////////////////////////////////////////////////////////////////////
bool
MPChatChildDialogClass::Find_Current_Command(const unichar_t* message, int& start_index, int& end_index)
{
	EditCtrlClass *edit_ctrl = (EditCtrlClass *)Get_Dlg_Item (IDC_MESSAGE_EDIT);
	if (edit_ctrl == NULL) {
		return false;
	}

	bool retval = false;

	//
	//	Get the caret position so we can determine where to
	// start looking for the command
	//
	int caret_pos = edit_ctrl->Get_Caret_Pos ();
	if (caret_pos > 0) {

		//
		//	Look to see if there is a command designator preceding the caret.
		//
		const unichar_t *command_start = ::u_strrchr (message, U_CHAR('/'));
		if (command_start != NULL) {
			start_index = command_start - message;
			command_start ++;
			
			//
			//	Check to ensure there isn't a space between the designator
			// and the caret
			//
			const unichar_t *first_space = ::u_strchr (command_start, U_CHAR(' '));
			if (first_space == NULL) {
				end_index = static_cast<int>(::u_strlen (message));
				retval = true;
			} else if (caret_pos <= (first_space - message)) {
				end_index = (first_space - message);
				retval = true;
			}
		}
	}	

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Key_Down
//
//////////////////////////////////////////////////////////////////////
bool
MPChatChildDialogClass::On_EditCtrl_Key_Down
(
	EditCtrlClass *	edit_ctrl,
	uint32				key_id,
	uint32				/* key_data */
)
{
	bool retval					= false;
	TestForAutoCompletion	= false;

	//
	//	Examine everything left of the caret
	//
	int caret_pos = edit_ctrl->Get_Caret_Pos ();
	if (caret_pos > 0) {

		//
		//	Get the message
		//
		WideStringClass message = Get_Dlg_Item_Text (IDC_MESSAGE_EDIT);
		if (message.Get_Length () > 0) {

			//
			//	Check to see if we are typing in a command
			//
			int cmd_start_index	= 0;
			int cmd_end_index		= 0;
			if (Find_Current_Command (message, cmd_start_index, cmd_end_index)) {
				
				//
				//	Special-case the space key
				//
				if (key_id == VK_SPACE && CurrRecipientName.Get_Length () > 0) {
					RecipientName = CurrRecipientName;

					//
					//	Erase the user's name from the string
					//
					message.Erase (cmd_start_index, cmd_end_index - cmd_start_index);
					edit_ctrl->Set_Text (message);
					edit_ctrl->Set_Caret_Pos (cmd_start_index);					

					//
					//	Update the dialog with the user's name...
					//
					WideStringClass heading_text(0, true);
					heading_text.Format (U_CHAR("%s:"), CurrRecipientName.Peek_Buffer ());
					Set_Dlg_Item_Text (IDC_TYPE_STATIC, heading_text);

					//
					//	Eat the spacebar...
					//
					retval = true;

				} else if (key_id != VK_BACK && key_id != VK_DELETE) {
					TestForAutoCompletion = true;
				}
			}
		}
	}
		
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPChatChildDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_SEND:
			Process_Message ();

			if (EndDialogOnSend) {
				Get_Parent_Dialog ()->End_Dialog ();
			}
			break;
	}

	ChildDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}




////////////////////////////////////////////////////////////////
//
//	MPIngameChatPopupClass
//
////////////////////////////////////////////////////////////////
MPIngameChatPopupClass::MPIngameChatPopupClass (void)	:
	DefaultType (TEXT_MESSAGE_PUBLIC),
	ChatModule (NULL),
	PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_INGAME_CHAT))
{
	//
	//	Configure the background renderer
	//
	StyleMgrClass::Configure_Renderer (&WindowBackgroundRenderer);
	WindowBackgroundRenderer.Get_Shader ()->Set_Depth_Compare (ShaderClass::PASS_ALWAYS);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~MPIngameChatPopupClass
//
////////////////////////////////////////////////////////////////
MPIngameChatPopupClass::~MPIngameChatPopupClass (void)
{
	REF_PTR_RELEASE (ChatModule);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPIngameChatPopupClass::On_Init_Dialog (void)
{
	WWASSERT (The_Game () != NULL);

	//
	//	Align the window with the bottom of the screen
	//
	const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution ();
	RectClass window_rect	= Get_Rect ();
	float height				= window_rect.Height ();
	window_rect.Bottom		= int(screen_rect.Bottom - 40.0F);
	window_rect.Top			= int(window_rect.Bottom - height);
	Set_Rect (window_rect);

	//
	//	Insert the chat module dialog into our window area
	//
	ChatModule = new MPChatChildDialogClass;	
	ChatModule->Set_Default_Type (DefaultType);
	ChatModule->Start_Dialog ();
	Add_Child_Dialog (ChatModule);
	ChatModule->Set_Rect (window_rect);

	//
	//	Darken the background behind this window
	//
	WindowBackgroundRenderer.Add_Quad (window_rect, RGBA_TO_INT32 (0, 0, 0, 200));
	WindowBackgroundRenderer.Enable_Texturing (false);

	//
	//	Let the base class process...
	//
	Set_Background_Darkened (false);
	PopupDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
MPIngameChatPopupClass::Render (void)
{
	//
	//	Render the black backdrop
	//
	WindowBackgroundRenderer.Render ();

	//
	//	Allow the base class to render
	//
	PopupDialogClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPIngameChatPopupClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	/*switch (ctrl_id)
	{
		case IDOK:
			ChatModule->Send_Message ();
			End_Dialog ();
			break;
	}*/

	PopupDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}
