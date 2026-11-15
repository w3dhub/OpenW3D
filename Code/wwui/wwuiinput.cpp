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

#if defined(OPENW3D_SDL3)
#include <SDL3/SDL_events.h>
#endif


WWUIInputClass::WWUIInputClass(void) :
	mIMEManager(nullptr)
{
}


WWUIInputClass::~WWUIInputClass(void)
{
	if (mIMEManager) {
		mIMEManager->Release_Ref();
	}
}


void WWUIInputClass::InitIME(WWUIWindowHandle hwnd)
{
	if (mIMEManager == nullptr) {
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


#if defined(OPENW3D_WIN32)
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
			return DialogMgrClass::On_Key_Down(int(wParam), int(lParam));
			break;

		case WM_KEYUP:
			return DialogMgrClass::On_Key_Up(int(wParam));
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
#elif defined(OPENW3D_SDL3)
bool WWUIInputClass::ProcessEvent(const SDL_Event& event)
{
	WWMEMLOG(MEM_GAMEDATA);

	if (mIMEManager) {
		if (mIMEManager->ProcessEvent(event)) {
			return true;
		}
	}

	switch (event.type) {
		case SDL_EVENT_KEY_DOWN: {
			int vk = WWUIPlatform::SDL_Keycode_To_VK(event.key.key);
			if (vk == 0) {
				return false;
			}

			// Build the key data the same way as the WM_KEYDOWN lParam:
			// repeat count, scan code and previous key state.
			uint32 key_data = 1 | ((uint32(event.key.raw) & 0xFF) << 16);
			if (event.key.repeat) {
				key_data |= (1 << 30);
			}

			return DialogMgrClass::On_Key_Down(vk, key_data);
			}
			break;

		case SDL_EVENT_KEY_UP: {
			int vk = WWUIPlatform::SDL_Keycode_To_VK(event.key.key);
			if (vk == 0) {
				return false;
			}

			return DialogMgrClass::On_Key_Up(vk);
			}
			break;

		case SDL_EVENT_TEXT_INPUT: {
			std::basic_string<unichar_t> text = WWUIPlatform::UTF8_To_Unichar(event.text.text);
			for (unichar_t ch : text) {
				DialogMgrClass::On_Unicode_Char(ch);
			}
			return true;
			}
			break;

		default:
			break;
	}

	return false;
}
#endif


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
