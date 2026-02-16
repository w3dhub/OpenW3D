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
 *                     $Archive:: /Commando/Code/wwui/dialogmgr.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/05/02 1:04p                                               $*
 *                                                                                             *
 *                    $Revision:: 40                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dialogmgr.h"
#include "dialogbase.h"
#include "childdialog.h"
#include "dx8wrapper.h"
#include "assetmgr.h"
#include "rinfo.h"
#include "camera.h"
#include "mousemgr.h"
#include "tooltipmgr.h"
#include "stylemgr.h"
#include "dialogcontrol.h"
#include "menudialog.h"
#include "wwuiinput.h"
#include "wwmemlog.h"
#include "dialogtransition.h"
#include "systimer.h"
#include "tooltip.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
DynamicVectorClass<DialogBaseClass *>	DialogMgrClass::DialogList;
DialogBaseClass **							DialogMgrClass::TestArray;
int												DialogMgrClass::TestArrayCount;
int												DialogMgrClass::TestArrayMaxCount;
bool												DialogMgrClass::IsFirstRender	= false;
bool												DialogMgrClass::IsInMenuMode	= false;
DialogBaseClass *								DialogMgrClass::ActiveDialog	= NULL;
BYTE												DialogMgrClass::KeyboardState[256];
DialogControlClass *							DialogMgrClass::InputCapture = NULL;
DialogControlClass *							DialogMgrClass::FocusControl = NULL;
WWUIInputClass *								DialogMgrClass::Input = NULL;
DialogTransitionClass *						DialogMgrClass::Transition = NULL;
DialogBaseClass *								DialogMgrClass::PendingActiveDialog	= NULL;
DialogBaseClass *								DialogMgrClass::TransitionDialog = NULL;
uint32											DialogMgrClass::CurrTime		= 0;
uint32											DialogMgrClass::LastFrameTime	= 0;
Vector3											DialogMgrClass::LastMousePos (0, 0, 0);
bool												DialogMgrClass::LastMouseButtonState[MB_COUNT] = { 0 };
bool												DialogMgrClass::IsFlushing = false;

ToolTipClass* DialogMgrClass::mIMEMessage = NULL;
uint32 DialogMgrClass::mIMEMessageTime = 0;


static bool	GameWasInFocus;

////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Initialize (const char *stylemgr_ini)
{
	StyleMgrClass::Initialize_From_INI (stylemgr_ini);
	MouseMgrClass::Initialize ();
	ToolTipMgrClass::Initialize ();
	MenuDialogClass::Initialize ();

	::memset (KeyboardState, 0, sizeof (KeyboardState));
	TestArrayMaxCount = 0;

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Shutdown (void)
{
	//
	//	Remove all the dialogs from our list
	//
	Flush_Dialogs ();

	MenuDialogClass::Shutdown ();
	StyleMgrClass::Shutdown ();
	MouseMgrClass::Shutdown ();
	ToolTipMgrClass::Shutdown ();

	Set_Active_Dialog (NULL);

	delete[] TestArray;
	TestArray=NULL;

	if (mIMEMessage) {
		delete mIMEMessage;
		mIMEMessage = NULL;
		mIMEMessageTime = 0;
	}

	//
	//	Release our hold on the input mechanism
	//
	REF_PTR_RELEASE (Input);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Inputs
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Reset_Inputs (void)
{
	//
	//	Update the cached mouse button states
	//
	LastMouseButtonState[MB_LBUTTON] = Input->Is_Button_Down (VK_LBUTTON);
	LastMouseButtonState[MB_MBUTTON] = Input->Is_Button_Down (VK_MBUTTON);
	LastMouseButtonState[MB_RBUTTON] = Input->Is_Button_Down (VK_RBUTTON);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Register_Dialog
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Register_Dialog (DialogBaseClass *dialog)
{
	//
	//	Check to see if this dialog is already in our list
	//
	if (DialogList.ID (dialog) == -1) {
		Reset_Inputs ();

		//
		//	Keep a lock on the dialog
		//
		dialog->Add_Ref ();

		//
		//	Add the dialog to our list.  Note:  We keep popup dialogs
		// as the topmost windows in our z order.  Z order is determined
		// by the order in the list.
		//
		bool is_top_level = true;
		if (dialog->As_PopupDialogClass () == NULL) {
			int new_index = DialogList.Count () - 1;
			for (; new_index >= 0; new_index --) {
				if (DialogList[new_index]->As_PopupDialogClass () == NULL) {
					break;
				}
				is_top_level = false;
			}
			DialogList.Insert (new_index + 1, dialog);
		} else {
			DialogList.Add (dialog);
		}

		//
		//	Make this the active dialog
		//
		if (is_top_level && dialog->Wants_Activation ()) {
			Set_Active_Dialog (dialog);
		}

		//
		//	Update the framework
		//
		On_Dialog_Added ();

		//
		//	Handle the first dialog...
		//
		if (DialogList.Count () == 1) {
			MouseMgrClass::Show_Cursor (true);
			IsFirstRender = true;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	UnRegister_Dialog
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::UnRegister_Dialog (DialogBaseClass *dialog)
{
	//
	//	Is this dialog in our list?
	//
	int index = DialogList.ID (dialog);
	if (index != -1) {
		Reset_Inputs ();

		//
		//	Remove the dialog from our list
		//
		DialogList.Delete (index);
		REF_PTR_RELEASE (dialog);

		//
		//	Update the framework
		//
		On_Dialog_Removed ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Dialog_Added
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::On_Dialog_Added (void)
{
	//
	//	Center the mouse in the window
	//
#if (0)
	HWND wnd = (HWND)WW3D::Get_Window ();
	RECT rect = { 0 };
	::GetClientRect (wnd, &rect);
#endif //(0)
	/*Vector3 mouse_pos = Get_Mouse_Pos ();
	mouse_pos.X = rect.left + (rect.right - rect.left) / 2;
	mouse_pos.Y = rect.top + (rect.bottom - rect.top) / 2;
	Set_Mouse_Pos (mouse_pos);*/
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Dialog_Removed
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::On_Dialog_Removed (void)
{
	//
	//	Are there any dialogs left?
	//
	if (DialogList.Count () == 0) {

		//
		//	Revert from "dialog" mode.
		//
		MouseMgrClass::Show_Cursor (false);
		Set_Active_Dialog (NULL);

	} else if (IsFlushing == false) {

		//
		//	Force "focus" to go to the most recent dialog
		//
		int index = DialogList.Count ();
		while (index --) {
			if (DialogList[index]->Wants_Activation ()) {
				Set_Active_Dialog (DialogList[index]);
				break;
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Transition
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Update_Transition (void)
{
	if (Transition != NULL) {

		//
		//	Allow the transition to think
		//
		Transition->On_Frame_Update ();
		Transition->Render ();

		//
		//	Did the transition finish?
		//
		if (Transition->Is_Complete ()) {

			//
			//	Let the dialog controls be displayed (as necessary)
			//
			if (TransitionDialog != NULL) {
				TransitionDialog->Set_Controls_Hidden (false);
			}

			if (PendingActiveDialog != NULL) {
				PendingActiveDialog->Set_Controls_Hidden (false);
			}

			//
			//	Now make the pending dialog active
			//
			Internal_Set_Active_Dialog (PendingActiveDialog);
			PendingActiveDialog	= NULL;
			TransitionDialog		= NULL;

			//
			//	Release our hold on the transition
			//
			REF_PTR_RELEASE (Transition);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::On_Frame_Update (void)
{
	WWMEMLOG(MEM_GAMEDATA);

	//
	//	Update the timing
	//
	uint32 old_time	= CurrTime;
	CurrTime				= TIMEGETTIME ();
	LastFrameTime		= CurrTime - old_time;


//	DynamicVectorClass<DialogBaseClass *> test_list = DialogList;
	if (DialogList.Count()>TestArrayMaxCount) {
		delete[] TestArray;
		TestArrayMaxCount=DialogList.Count();
		TestArray=new DialogBaseClass*[TestArrayMaxCount];
	}
	TestArrayCount=DialogList.Count();
	for (int i=0;i<TestArrayCount;++i) {
		TestArray[i]=DialogList[i];
	}

	//
	//	Let each dialog think
	//
	for (int index = 0; index < DialogList.Count (); index ++) {
		//
		//	Simple check to ensure that the DialogList hasn't changed
		//	due to this On_Frame_Update () call
		//
		if (index >= TestArrayCount || DialogList[index] != TestArray[index]) {
			break;
		}

		DialogBaseClass* dialog = DialogList[index];
		WWASSERT(dialog != NULL);

		dialog->Add_Ref();

		if (dialog->Is_Active () && dialog->As_ChildDialogClass () == NULL) {
			dialog->On_Frame_Update ();
		}

		//
		//	Force an "On_Periodic" for dialogs that aren't in focus...
		//
		dialog->On_Periodic ();
		dialog->Release_Ref();
	}

	//
	//	Return from "dialog" mode if the ESC key has been let up...
	//
	if (IsInMenuMode && DialogList.Count () == 0 && ((KeyboardState[VK_ESCAPE] & 0x80) == 0)) {
		IsInMenuMode = false;
		Input->Exit_Menu_Mode ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Render (void)
{
	WWMEMLOG(MEM_GAMEDATA);

	if (!GameInFocus) {
		GameWasInFocus=false;
		return;
	}
	if (!GameWasInFocus) {
		DialogMgrClass::Reset();
		GameWasInFocus=true;
	}

	//
	//	Enable static sorting levels
	//
	bool enable_static_sort = WW3D::Are_Static_Sort_Lists_Enabled ();
	WW3D::Enable_Static_Sort_Lists (true);

	//
	//	Enable the "console" mode if this is the first dialog
	// we're rendering
	//
	if (IsFirstRender) {
		IsFirstRender	= false;
		IsInMenuMode	= true;
		Input->Enter_Menu_Mode ();
	}

	//
	//	Update any transitions
	//
	Update_Transition ();

	//
	//	Render each dialog
	//
	for (int index = 0; index < DialogList.Count (); index ++) {
		bool render = false;

		//
		//	Should this dialog be rendered?
		//
		DialogBaseClass *dialog = DialogList[index];
		if (	dialog->Is_Visible () /*&&
				dialog != PendingActiveDialog &&
				dialog != TransitionOutDialog*/ )
		{
			//
			//	Don't render here if its a child-dialog
			//
			render = (dialog->As_ChildDialogClass () == NULL);
		}

		//
		//	Render the dialog
		//
		if (render) {
			dialog->Render ();
		}
	}

	//
	//	Render the tooltip
	//
	Vector3 cursor_pos = Get_Mouse_Pos ();
	ToolTipMgrClass::Update (Vector2 (cursor_pos.X, cursor_pos.Y));
	ToolTipMgrClass::Render ();

	if ((mIMEMessageTime > CurrTime) && mIMEMessage) {
		mIMEMessage->Render();
	}

	//
	//	Render the mouse cursor
	//
	MouseMgrClass::Render ();

	//
	//	Update the cached mouse button states
	//
	Reset_Inputs ();

	//
	//	Put the static sort flag back the way we found it
	//
	WW3D::Enable_Static_Sort_Lists (enable_static_sort);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Active_Dialog
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Set_Active_Dialog (DialogBaseClass *dialog)
{
	if (ActiveDialog == dialog) {
		return ;
	}

	//REF_PTR_RELEASE (Transition);
	if (Transition != NULL) {
		PendingActiveDialog = dialog;
		return ;
	}

	//
	//	Check to see if we should play an outro-transition for
	// the currently active dialog
	//
	if (ActiveDialog != NULL) {
		Transition = ActiveDialog->Get_Transition_Out (dialog);
		if (Transition != NULL) {
			PendingActiveDialog	= dialog;
			TransitionDialog		= ActiveDialog;

			//
			//	Hide the controls on the pending dialog (if necessary)
			//
			if (PendingActiveDialog != NULL) {
				//PendingActiveDialog->Set_Controls_Hidden (true);
			}
		}
	}

	//
	//	Check to see if we should play an intro-transition for
	// the new active dialog
	//
	if (dialog != NULL && Transition == NULL) {
		Transition = dialog->Get_Transition_In (ActiveDialog);
		if (Transition != NULL) {
			PendingActiveDialog	= dialog;
			TransitionDialog		= dialog;

			//
			//	Hide the controls on the transitioning dialog
			//
			//TransitionDialog->Set_Controls_Hidden (true);
		}
	}

	//
	//	If we don't have any transition to play, then simply
	// set the active dialog
	//
	if (Transition == NULL) {
		Internal_Set_Active_Dialog (dialog);
	}

	return ;
}

void
DialogMgrClass::Reset (void)
{
//	REF_PTR_RELEASE (Transition);
//	if (PendingActiveDialog) {
//		REF_PTR_SET(ActiveDialog,PendingActiveDialog);
//		REF_PTR_RELEASE(PendingActiveDialog);
//	}

//	Internal_Set_Active_Dialog (ActiveDialog);

	if (ActiveDialog) {
		ActiveDialog->Set_Dirty();
	}
	if (PendingActiveDialog) {
		PendingActiveDialog->Set_Dirty();
	}
}


////////////////////////////////////////////////////////////////
//
//	Internal_Set_Active_Dialog
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Internal_Set_Active_Dialog (DialogBaseClass *dialog)
{
	//
	//	Get rid of the input capture and focus
	//
	Release_Capture ();

	//
	//	Switch the active dialog
	//
	DialogBaseClass *old_dialog = ActiveDialog;
	ActiveDialog = dialog;

	//
	//	Notify the old dialog
	//
	if (old_dialog != NULL) {
		old_dialog->On_Activate (false);
		REF_PTR_RELEASE (old_dialog);
	}

	Set_Focus (NULL);

	//
	//	Notify the new dialog (if necessary)
	//
	if (ActiveDialog != NULL) {
		ActiveDialog->Add_Ref ();
		ActiveDialog->On_Activate (true);
	}

	//
	//	Remove any tooltips
	//
	ToolTipMgrClass::Reset ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
DialogMgrClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	if (Transition != NULL) {
		return false;
	}

	//
	//	Update the keyboard state
	//
	::GetKeyboardState (KeyboardState);

	//
	//	Notify the active dialog (if any)
	//
	if (ActiveDialog != NULL) {
		return ActiveDialog->On_Key_Down (key_id, key_data);
	}

	return false;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Up
//
////////////////////////////////////////////////////////////////
bool
DialogMgrClass::On_Key_Up (uint32 key_id)
{
	if (Transition != NULL) {
		return false;
	}

	//
	//	Update the keyboard state
	//
	::GetKeyboardState (KeyboardState);

	//
	//	Notify the active dialog (if any)
	//
	if (ActiveDialog != NULL) {
		return ActiveDialog->On_Key_Up (key_id);
	}

	return false;
}


////////////////////////////////////////////////////////////////
//
//	On_Unicode_Char
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::On_Unicode_Char (uint16 unicode)
{
	if (Transition != NULL) {
		return ;
	}

	//
	//	Update the keyboard state
	//
	::GetKeyboardState (KeyboardState);

	//
	//	Notify the active dialog (if any)
	//
	if (ActiveDialog != NULL) {
		ActiveDialog->On_Unicode_Char (unicode);
	}

	return ;
}

////////////////////////////////////////////////////////////////
//
//	Find_Control
//
////////////////////////////////////////////////////////////////
DialogControlClass *
DialogMgrClass::Find_Control (const Vector2 &mouse_pos)
{
	DialogControlClass *retval = NULL;

	//
	//	Get this information from the active dialog
	//
	if (ActiveDialog != NULL) {
		retval = ActiveDialog->Find_Control (mouse_pos);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Capture
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Set_Capture (DialogControlClass *control)
{
	InputCapture = control;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Release_Capture
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Release_Capture (void)
{
	InputCapture = NULL;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Flush_Dialogs
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Flush_Dialogs (void)
{
	WWDEBUG_SAY(("DialogMgrClass: Flushing dialogs\n"));

	IsFlushing = true;

	Set_Active_Dialog (NULL);

	//
	//	Remove all the dialogs from our list
	//
	while (DialogList.Count () > 0) {
		DialogList[DialogList.Count () - 1]->End_Dialog ();
	}

	Set_Focus (NULL);

	//
	//	Reset our transition variables
	//
	TransitionDialog		= NULL;
	PendingActiveDialog	= NULL;
	REF_PTR_RELEASE (Transition);

	//
	//	Release our hold on the transitioning dialogs
	//
	//REF_PTR_RELEASE (TransitionInDialog);
	//REF_PTR_RELEASE (TransitionOutDialog);

	//
	//	Reset the list
	//
	DialogList.Delete_All ();

	IsFlushing = false;

	WWDEBUG_SAY(("DialogMgrClass: Flush complete\n"));
	return ;
}


bool DialogMgrClass::Is_Flushing_Dialogs(void)
{
	return IsFlushing;
}


////////////////////////////////////////////////////////////////
//
//	Was_Button_Down
//
////////////////////////////////////////////////////////////////
bool
DialogMgrClass::Was_Button_Down (int vk_mouse_button_id)
{
	bool retval = false;

	switch (vk_mouse_button_id)
	{
		case VK_LBUTTON:
			retval = LastMouseButtonState[MB_LBUTTON];
			break;

		case VK_MBUTTON:
			retval = LastMouseButtonState[MB_MBUTTON];
			break;

		case VK_RBUTTON:
			retval = LastMouseButtonState[MB_RBUTTON];
			break;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Focus
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Set_Focus (DialogControlClass *control)
{
	if (FocusControl == control) {
		return ;
	}

	//
	//	Change the focus
	//
	DialogControlClass *old_focus_ctrl = FocusControl;
	FocusControl = control;

	//
	//	Let go of the currently focus'd control
	//
	if (old_focus_ctrl != NULL) {
		old_focus_ctrl->On_Kill_Focus (control);
	}

	//
	//	Reset the focus
	//
	if (FocusControl != NULL) {
		FocusControl->On_Set_Focus ();
	}

	//
	//	Remove the input capture
	//
	Release_Capture ();
	return ;
}


DialogBaseClass* DialogMgrClass::Find_Dialog(int dialogID)
{
	for (int index = 0; index < DialogList.Count(); ++index) {
		if (DialogList[index]->Get_Dlg_Resource()->id == dialogID) {
			return DialogList[index];
		}
	}

	return NULL;
}


////////////////////////////////////////////////////////////////
//
//	Rollback
//
////////////////////////////////////////////////////////////////
void
DialogMgrClass::Rollback (DialogBaseClass *dialog)
{
	for (int index = DialogList.Count () - 1; index >= 0; index --) {

		//
		//	Stop once we've come across the dialog we're rolling back to
		//
		if (DialogList[index] == dialog) {
			break;
		}

		//
		//	Close this dialog (if necessary)
		//
		if (DialogList[index]->As_ChildDialogClass () == NULL) {
			DialogList[index]->End_Dialog ();
		}
	}

	return ;
}


void DialogMgrClass::Show_IME_Message(const unichar_t* message, uint32 duration)
{
	if (mIMEMessage == NULL) {
		mIMEMessage = new ToolTipClass;
	}

	if (mIMEMessage) {
		mIMEMessage->Set_Position(Vector2(10,10));
		mIMEMessage->Set_Text(message);
		mIMEMessageTime = (CurrTime + duration);
	}
}
