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

/******************************************************************************
*
* NAME
*     $Archive: /Commando/Code/Commando/DlgMessageBox.h $
*
* DESCRIPTION
*     Popup message dialog.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 8 $
*     $Modtime: 10/13/01 4:50p $
*
******************************************************************************/

#ifndef __DLGMESSAGEBOX_H__
#define __DLGMESSAGEBOX_H__

#include <wwui/popupdialog.h>
#include <wwlib/Notify.h>

class DlgMsgBox;

class DlgMsgBoxEvent :
		public TypedEventPtr<DlgMsgBoxEvent, DlgMsgBox>
	{
	public:
		typedef enum
			{
			None = 0, // NULL event
			Okay,     // Okay button pressed
			Yes,      // Yes button pressed
			No,       // No button pressed
			Quitting, // Dialog quitting.
			} EventID;

		//! Retrieve event
		inline EventID Event(void) const
			{return mEvent;}

		//! User data access
		inline unsigned int Get_User_Data(void) const
			{return mUserData;}

		inline void Set_User_Data(unsigned int data)
			{mUserData = data;}

		DlgMsgBoxEvent(EventID event, DlgMsgBox* object, unsigned int user_data) :
				TypedEventPtr<DlgMsgBoxEvent, DlgMsgBox>(object),
			  mEvent(event), mUserData (user_data)
			{}

	protected:
		// Prevent copy and assignment
		DlgMsgBoxEvent(const DlgMsgBoxEvent&);
		const DlgMsgBoxEvent& operator=(const DlgMsgBoxEvent&);

	private:
		EventID mEvent;
		unsigned int mUserData;
	};


class DlgMsgBox :
		public PopupDialogClass,
		public Notifier<DlgMsgBoxEvent>
	{
	public:
		typedef enum
			{
			Okay = 0, // Message box with okay button (Default)
			YesNo,    // Yes/No message box
			} Type;

		static bool DoDialog(const unichar_t* title, const unichar_t* text,
			DlgMsgBox::Type type = DlgMsgBox::Okay, Observer<DlgMsgBoxEvent>* observer = NULL,
			unsigned int user_data = 0);

		static bool DoDialog(int titleID, int textID, DlgMsgBox::Type type = DlgMsgBox::Okay,
			Observer<DlgMsgBoxEvent>* observer = NULL, unsigned int user_data = 0);

		void Set_User_Data(unsigned int user_data)
			{mUserData = user_data;}

		unsigned int Get_User_Data(void) const
			{return mUserData;}

		static int Get_Current_Count(void)
			{return CurrentCount;}

	protected:
		DlgMsgBox();
		virtual ~DlgMsgBox();

		void SetResourceType(DlgMsgBox::Type type);
		void End_Dialog(void) override;
		void On_Command(int ctrl, int message, unsigned int param) override;

		DECLARE_NOTIFIER(DlgMsgBoxEvent)

	private:
		// Prevent copy and assignment
		DlgMsgBox(const DlgMsgBox&);
		const DlgMsgBox& operator=(const DlgMsgBox&);

		static int CurrentCount;
		unsigned int mUserData;
	};

#endif // __DLGMESSAGEBOX_H__
