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
 *                     $Archive:: /Commando/Code/wwui/wwuiinput.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/08/02 8:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "wwuiinput.h"
#include "dialogmgr.h"
#include "wwmemlog.h"


WWUIInputClass::WWUIInputClass(void) :
	mIMEManager(NULL)
{
}


WWUIInputClass::~WWUIInputClass(void)
{
	if (mIMEManager) {
		mIMEManager->Release_Ref();
	}
}


void WWUIInputClass::InitIME(HWND hwnd)
{
	if (mIMEManager == NULL) {
		mIMEManager = IME::IMEManager::Create(hwnd);

		if (mIMEManager) {
			Observer<IME::UnicodeChar>::NotifyMe(*mIMEManager);
			Observer<IME::IMEEvent>::NotifyMe(*mIMEManager);
		}
	}
}


IME::IMEManager* WWUIInputClass::GetIME(void) const
{
	if (mIMEManager) {
		mIMEManager->Add_Ref();
	}

	return mIMEManager;
}


bool WWUIInputClass::ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
	WWMEMLOG(MEM_GAMEDATA);

	if (mIMEManager) {
		if (mIMEManager->ProcessMessage(hwnd, msg, wParam, lParam, result)) {
			return true;
		}
	}

	result = 0;

	switch (msg) {
		case WM_KEYDOWN:
			return DialogMgrClass::On_Key_Down(wParam, lParam);
			break;

		case WM_KEYUP:
			return DialogMgrClass::On_Key_Up(wParam);
			break;

		case WM_CHAR:
			DialogMgrClass::On_Unicode_Char((unichar_t)wParam);
			return true;
			break;

		default:
			break;
	}

	return false;
}


void WWUIInputClass::HandleNotification(IME::UnicodeChar& unicode)
{
	DialogMgrClass::On_Unicode_Char(unicode.Subject());
}


void WWUIInputClass::HandleNotification(IME::IMEEvent& event)
	{
	if (IME::IME_LANGUAGECHANGED == event.GetAction())
		{
		const unichar_t* description = event.Subject()->GetDescription();
		DialogMgrClass::Show_IME_Message(description, 2500);
		}
	else if (IME::IME_GUIDELINE == event.GetAction())
		{
		unichar_t desc[255];
		unsigned int level = event.Subject()->GetGuideline(desc, sizeof(desc));

		if (GL_LEVEL_NOGUIDELINE != level)
			{
			DialogMgrClass::Show_IME_Message(desc, 30000);
			}
		}
	}
