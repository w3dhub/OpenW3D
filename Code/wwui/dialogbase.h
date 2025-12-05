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
 *                     $Archive:: /Commando/Code/wwui/dialogbase.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/03/01 5:09p                                              $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DIALOG_BASE_H
#define __DIALOG_BASE_H


#include "dialogspec.h"
#include "vector.h"
#include "vector3.h"
#include "rect.h"
#include "refcount.h"
#include "bittype.h"
#include "widestring.h"
#include "controladvisesink.h"
#include "win.h"
#include "Notify.h"

////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class DialogControlClass;
class MenuDialogClass;
class PopupDialogClass;
class ChildDialogClass;
class DialogBaseClass;
class DialogTransitionClass;


////////////////////////////////////////////////////////////////
//	Usefull Macros
////////////////////////////////////////////////////////////////
#define	START_DIALOG(class_name)				\
	{	class_name *dialog = new class_name;	\
		dialog->Start_Dialog ();					\
		REF_PTR_RELEASE (dialog);	}


////////////////////////////////////////////////////////////////
//	Typedefs
////////////////////////////////////////////////////////////////
typedef bool (CALLBACK *DEFAULT_DLG_CMD_HANDLER) (DialogBaseClass *dialog, int ctrl_id, int mesage_id, unsigned int param);


class DialogEvent :
		public TypedEventPtr<DialogEvent, DialogBaseClass>
{
public:
	typedef enum
		{
		STARTED = 1,
		DESTROY,
		ACTIVATED,
		DEACTIVATED,
		} EventID;

	//! Retrieve event
	inline EventID GetEventID(void) const
		{return mEvent;}

	DialogEvent(EventID event, DialogBaseClass* dialog) :
			TypedEventPtr<DialogEvent, DialogBaseClass>(dialog),
			mEvent(event)
		{}

private:
	EventID mEvent;
};


////////////////////////////////////////////////////////////////
//
//	DialogBaseClass
//
//		These dialogs are analagous to Windows dialogs.  They are
//	initialized from an RC file, however the dialog template is
//	only parsed - the window isn't actually created.
//
////////////////////////////////////////////////////////////////
class DialogBaseClass : public RefCountClass, public ControlAdviseSinkClass,
		public Notifier<DialogEvent>
{
public:

	////////////////////////////////////////////////////////////////
	//	Public friends
	////////////////////////////////////////////////////////////////
	friend class DialogMgrClass;

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	DialogBaseClass (const DialogResource *dialog_resource);
	virtual ~DialogBaseClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//
	virtual MenuDialogClass *	As_MenuDialogClass (void)	{ return NULL; }
	virtual PopupDialogClass *	As_PopupDialogClass (void)	{ return NULL; }
	virtual ChildDialogClass *	As_ChildDialogClass (void)	{ return NULL; }

	const DialogResource * Get_Dlg_Resource(void) const {return DialogResource_;}

	//
	//	Display methods
	//
	virtual void			Render (void);
	void						Show (bool onoff)			{ IsVisible = onoff; }
	bool						Is_Visible (void) const	{ return IsVisible; }
	void						Set_Dirty(bool onoff=true);

	//
	//	Position control
	//
	const RectClass &		Get_Rect (void) const					{ return Rect; }
	void						Set_Rect (const RectClass &rect);

	//
	//	Flow control
	//
	virtual void			Start_Dialog (void);
	virtual void			End_Dialog (void);
	virtual bool			Is_Running (void)		{ return IsRunning; }

	//
	//	Control access
	//
	DialogControlClass *	Get_Dlg_Item (int id) const;
	DialogControlClass *	Find_Control (const Vector2 &mouse_pos);
	void						Add_Control (DialogControlClass *control);
	void						Remove_Control (DialogControlClass *control);

	DialogControlClass *	Find_Next_Control (DialogControlClass *control, int direction = 1);
	DialogControlClass *	Find_Next_Group_Control (DialogControlClass *control, int direction = 1);

	int						Get_Control_Count (void) const	{ return ControlList.Count (); }
	DialogControlClass *	Get_Control (int index) const		{ return ControlList[index]; }


	//
	//	Control enable state access
	//
	void						Enable_Dlg_Item (int id, bool onoff);
	bool						Is_Dlg_Item_Enabled (int id);

	//
	//	Control text access
	//
	const wchar_t *			Get_Dlg_Item_Text (int id) const;
	void						Set_Dlg_Item_Text (int id, const wchar_t *text);

	int						Get_Dlg_Item_Int (int id) const;
	void						Set_Dlg_Item_Int (int id, int value);

	float						Get_Dlg_Item_Float (int id) const;
	void						Set_Dlg_Item_Float (int id, float value);

	//
	//	Control "check" access
	//
	void						Check_Dlg_Button (int id, bool onoff);
	bool						Is_Dlg_Button_Checked (int id) const;

	//
	//	Child dialog access
	//
	void						Add_Child_Dialog (ChildDialogClass *child);
	void						Remove_Child_Dialog (ChildDialogClass *child);

	//
	//	Title access
	//
	void						Get_Title (WideStringClass *title)	{ *title = Title; }
	void						Set_Title (const wchar_t *title)		{ Title = title; }

	//
	//	Activation access
	//
	virtual bool			Is_Active (void);
	virtual bool			Wants_Activation (void)	{ return true; }

	//
	//	Transition control
	//
	virtual DialogTransitionClass *	Get_Transition_In (DialogBaseClass *prev_dlg)	{ return NULL; }
	virtual DialogTransitionClass *	Get_Transition_Out (DialogBaseClass *next_dlg)	{ return NULL; }
	
	virtual void			Set_Controls_Hidden (bool onoff)			{ AreControlsHidden = onoff; }
	virtual bool			Are_Controls_Hidden (void) const			{ return AreControlsHidden; }

	//
	//	Notifications
	//
	virtual void			On_Command (int ctrl_id, int mesage_id, unsigned int param);

	//
	//	Default processing support
	//
	static void				Set_Default_Command_Handler (DEFAULT_DLG_CMD_HANDLER ptr)	{ DefaultCmdHandler = ptr; }
	static DEFAULT_DLG_CMD_HANDLER	Get_Default_Command_Handler (void)					{ return DefaultCmdHandler; }

	DECLARE_NOTIFIER(DialogEvent)

protected:

	////////////////////////////////////////////////////////////////
	//	Protected typedefs
	////////////////////////////////////////////////////////////////	
	typedef DynamicVectorClass<DialogControlClass *> CONTROL_LIST;
	typedef DynamicVectorClass<ChildDialogClass *> DIALOG_LIST;

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	virtual void			On_Init_Dialog (void);
	virtual void			On_Destroy (void)					{}
	virtual void			On_Activate (bool onoff);
	virtual bool			On_Key_Down (uint32 key_id, uint32 key_data);
	virtual void			On_Unicode_Char(uint16 unicode);
	virtual void			On_Mouse_Wheel (int direction);
	virtual bool			On_Key_Up (uint32 key_id);
	virtual void			On_Frame_Update (void);
	virtual void			On_Periodic (void)				{}

	void						Free_Controls (void);
	void						Update_Mouse_State (void);
	int						Find_Focus_Control (void);
	int						Find_Control_Index (DialogControlClass *control);
	void						Send_Mouse_Input (DialogControlClass *control, const Vector2 &mouse_pos);

	void						Build_Control_List (CONTROL_LIST &list);
	void						Set_Default_Focus (void);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////	
	WideStringClass				Title;
	RectClass						Rect;
	const DialogResource *				DialogResource_;
	CONTROL_LIST					ControlList;
	DIALOG_LIST						ChildDialogList;
	DialogControlClass *			LastFocusControl;
	bool								IsVisible;
	bool								AreControlsHidden;
	bool								IsRunning;
	int								LastMouseClickTime;

	static DEFAULT_DLG_CMD_HANDLER	DefaultCmdHandler;
};

#endif //__DIALOG_BASE_H

