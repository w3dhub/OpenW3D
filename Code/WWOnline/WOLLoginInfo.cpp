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
* FILE
*     $Archive: /Commando/Code/WWOnline/WOLLoginInfo.cpp $
*
* DESCRIPTION
*     LoginInfo encapsulates a user's persona login information.
*     LoginInfo handles the storage/retrival of nicknames/passwords also.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 15 $
*     $Modtime: 1/18/02 11:50p $
*
******************************************************************************/

#include <atlbase.h>
#include "WOLLoginInfo.h"
#include "WOLSession.h"
#include <WWLib\WWString.h>
#include <WWDebug\WWDebug.h>

namespace WOL
{
#include <WOLAPI\wolapi.h>
}

namespace WWOnline {

#define MAX_NICKNAMES 32

LoginInfoList LoginInfo::_mLoginList;

/******************************************************************************
*
* NAME
*     LoginInfo::GetList
*
* DESCRIPTION
*     Get a list of all the cached WWOnline logins.
*
* INPUTS
*     NONE
*
* RESULT
*     LoginList - List to populate with WWOnline logins.
*
******************************************************************************/

const LoginInfoList& LoginInfo::GetList(void)
	{
	if (_mLoginList.empty())
		{
		RefPtr<Session> session = Session::GetInstance(false);
	
		if (session.IsValid() && session->IsStoreLoginAllowed())
			{
			// Four logins allowed per user
			_mLoginList.reserve(4);

			const CComPtr<WOL::IChat>& chat = session->GetChatObject();

			for (int index = 1; index <= MAX_NICKNAMES; ++index)
				{
				const char* nickname = NULL;
				const char* password = NULL;
				chat->GetNick(index, &nickname, &password);
		
				if (nickname && (strlen(nickname) > 0))
					{
					RefPtr<LoginInfo> login = Create(nickname, password, true);

					if (login.IsValid())
						{
						WOL::Locale locale = WOL::LOC_UNKNOWN;
						chat->GetNickLocale(index, &locale);
						login->SetLocale(locale);

						login->mIsStored = true;

						_mLoginList.push_back(login);
						}
					}
				}
			}
		}
	
	return _mLoginList;
	}


/******************************************************************************
*
* NAME
*     LoginInfo::ClearList
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginInfo::ClearList(void)
	{
	_mLoginList.clear();
	}


/******************************************************************************
*
* NAME
*     LoginInfo::Find
*
* DESCRIPTION
*     Search for a login with a specified nickname.
*
* INPUTS
*     Nickname - Nickname of login to search for.
*
* RESULT
*     Login - Login matching specified name; if available.
*
******************************************************************************/

RefPtr<LoginInfo> LoginInfo::Find(const wchar_t* name)
	{
	if (name)
		{
		const LoginInfoList& list = GetList();
		const unsigned int count = list.size();

		for (unsigned int index = 0; index < count; ++index)
			{
			RefPtr<LoginInfo> login = list[index];
			WWASSERT(login.IsValid());

			const WideStringClass& nickname = login->GetNickname();

			if (nickname.Compare_No_Case(name) == 0)
				{
				return login;
				}
			}
		}

	return NULL;
	}


RefPtr<LoginInfo> LoginInfo::Find(const char* nickname)
	{
	WideStringClass wideNickname(0, true);
	wideNickname = nickname;
	return Find(wideNickname);
	}


/******************************************************************************
*
* NAME
*     LoginInfo::Create
*
* DESCRIPTION
*     Create a login info instance
*
* INPUTS
*     Nickname - Login username
*     Password - Login password
*     IsEncrypted - True if the password is encrypted.
*
* RESULT
*     LoginInfo - Instance of newly created login information.
*
******************************************************************************/

RefPtr<LoginInfo> LoginInfo::Create(const wchar_t* nickname, const wchar_t* password, bool isEncrypted)
	{
	if (nickname && (wcslen(nickname) > 0))
		{
		return new LoginInfo(nickname, password, isEncrypted);
		}

	return NULL;
	}


RefPtr<LoginInfo> LoginInfo::Create(const char* nickname, const char* password, bool isEncrypted)
	{
	if (nickname && (strlen(nickname) > 0))
		{
		WideStringClass name(0, true);
		name = nickname;

		WideStringClass pass(0, true);
		pass = password;

		return new LoginInfo(name, pass, isEncrypted);
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     LoginInfo::LoginInfo
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Nickname - Login username
*     Password - Login password
*     IsEncrypted - True if the password is encrypted.
*
* RESULT
*     NONE
*
******************************************************************************/

LoginInfo::LoginInfo(const wchar_t* nickname, const wchar_t* password, bool isEncrypted) :
		mNickname(nickname),
		mPassword(password),
		mLocale(WOL::LOC_UNKNOWN),
		mIsPasswordEncrypted(isEncrypted),
		mIsStored(false)
	{
	WWDEBUG_SAY(("WOL: Instantiating LoginInfo %S\n", (const WCHAR*)mNickname));

	if (mNickname.Get_Length() > 9)
		{
		mNickname[9] = 0;
		}
	}


/******************************************************************************
*
* NAME
*     LoginInfo::~LoginInfo
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*
* RESULT
*
******************************************************************************/

LoginInfo::~LoginInfo()
	{
	WWDEBUG_SAY(("WOL: Destructing LoginInfo %S\n", (const WCHAR*)mNickname));
	}


/******************************************************************************
*
* NAME
*     LoginInfo::SetPassword
*
* DESCRIPTION
*     Set the password for this login.
*
* INPUTS
*     Password  - New password
*     Encrypted - True if password is encrypted.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginInfo::SetPassword(const wchar_t* password, bool isEncrypted)
	{
	if (password != NULL)
		{
		mPassword = password;
		mIsPasswordEncrypted = isEncrypted;
		}
	else
		{
		mPassword = L"";
		mIsPasswordEncrypted = false;
		}
	}


/******************************************************************************
*
* NAME
*     LoginInfo::SetLocale
*
* DESCRIPTION
*     Set the locale for this login.
*
* INPUTS
*     Locale - New locale.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginInfo::SetLocale(WOL::Locale locale)
	{
	mLocale = locale;

	int index = IndexOf();

	if (index != 0)
		{
		RefPtr<Session> session = Session::GetInstance(false);

		if (session.IsValid() && session->IsStoreLoginAllowed())
			{
			const CComPtr<WOL::IChat>& chat = session->GetChatObject();
			WWASSERT(chat);
			chat->SetNickLocale(index, locale);
			}
		}
	}


/******************************************************************************
*
* NAME
*     LoginInfo::GetLocale
*
* DESCRIPTION
*     Returns the current locale for this login.
*
* INPUTS
*     NONE
*
* RESULT
*     Locale - Locale for this login.
*
******************************************************************************/

WOL::Locale LoginInfo::GetLocale(void)
	{
	if (mLocale == WOL::LOC_UNKNOWN)
		{
		int index = IndexOf();

		if (index != 0)
			{
			RefPtr<Session> session = Session::GetInstance(false);

			if (session.IsValid())
				{
				const CComPtr<WOL::IChat>& chat = session->GetChatObject();
				WWASSERT(chat);
				chat->GetNickLocale(index, &mLocale);
				}
			}
		}

	return mLocale;
	}


/******************************************************************************
*
* NAME
*     LoginInfo::Remember
*
* DESCRIPTION
*     Remember this login. If the store flag is true then the login is written
*     to disk for retrieval in later sessions.
*
* INPUTS
*     Store - True to store this login to the disk.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginInfo::Remember(bool store)
	{
	if (store)
		{
		mIsStored = true;

		int index = IndexOf();

		if (index == 0)
			{
			StringClass name(64, true);
			mNickname.Convert_To(name);

			StringClass pass(64, true);
			mPassword.Convert_To(pass);

			// Store login information into the registry
			StoreLogin(name.Peek_Buffer(), pass.Peek_Buffer(), mIsPasswordEncrypted, mLocale);
			}
		}

	// If the login is not in the list then add it now.
	bool found = false;
	LoginInfoList::iterator iter = _mLoginList.begin();

	while (iter != _mLoginList.end())
		{
		if (iter->ReferencedObject() == this)
			{
			found = true;
			break;
			}

		iter++;
		}

	if (!found)
		{
		_mLoginList.push_back(this);
		}
	}


/******************************************************************************
*
* NAME
*     LoginInfo::Forget
*
* DESCRIPTION
*     Forget about this login. If the purge flag is true then the login
*     is deleted from disk.
*
* INPUTS
*     Purge - True to purge this login from the disk.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginInfo::Forget(bool purge)
	{
	if (purge)
		{
		mIsStored = false;

		// Remove login for the persist cache
		RefPtr<Session> session = Session::GetInstance(false);

		if (session.IsValid() && session->IsStoreLoginAllowed())
			{
			int index = IndexOf();

			if (index != 0)
				{
				const CComPtr<WOL::IChat>& chat = session->GetChatObject();
				chat->SetNick(index, "", "", false);
				chat->SetNickLocale(index, WOL::LOC_UNKNOWN);
				}
			}
		}

	// Remove login from the list
	LoginInfoList::iterator iter = _mLoginList.begin();

	while (iter != _mLoginList.end())
		{
		if (iter->ReferencedObject() == this)
			{
			_mLoginList.erase(iter);
			break;
			}

		iter++;
		}
	}


/******************************************************************************
*
* NAME
*     LoginInfo::IndexOf
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

int LoginInfo::IndexOf(const wchar_t* nick)
	{
	RefPtr<Session> session = Session::GetInstance(false);

	if (session.IsValid())
		{
		char username[64];
		wcstombs(username, nick, sizeof(username));

		const CComPtr<WOL::IChat>& chat = session->GetChatObject();

		for (int index  = 1; index <= MAX_NICKNAMES; ++index)
			{
			const char* nickname = NULL;
			const char* password = NULL;
			HRESULT result = chat->GetNick(index, &nickname, &password);

			if (SUCCEEDED(result))
				{
				if (nickname && (strlen(nickname) > 0) && (stricmp(username, nickname) == 0))
					{
					return index;
					}
				}
			}
		}

	return 0;
	}


/******************************************************************************
*
* NAME
*     LoginInfo::IndexOf
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     Index - 
*
******************************************************************************/

int LoginInfo::IndexOf(void) const
	{
	return IndexOf(mNickname);
	}


/******************************************************************************
*
* NAME
*     LoginInfo::StoreLogin
*
* DESCRIPTION
*     Store login information.
*
* INPUTS
*     Nickname        - Login nickname
*     Password        - Login password
*     IsPassEncrytped - True if the password is already encrypted.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginInfo::StoreLogin(const char* nickname, const char* password,
		bool isPasswordEncrypted, WOL::Locale locale)
	{
	RefPtr<Session> session = Session::GetInstance(false);

	if (session.IsValid() && session->IsStoreLoginAllowed())
		{
		const CComPtr<WOL::IChat>& chat = session->GetChatObject();

		// Find the next empty slot
		int index;
		for (index = 1; index <= MAX_NICKNAMES; ++index)
			{
			const char* slotNick = NULL;
			const char* slotPass = NULL;
			chat->GetNick(index, &slotNick, &slotPass);

			if ((slotNick == NULL) || strlen(slotNick) == 0)
				{
				break;
				}
			}

		if (index > MAX_NICKNAMES)
			{
			// All slots taken!  Kill the last one
			index = MAX_NICKNAMES;
			}

		chat->SetNick(index, nickname, password, !isPasswordEncrypted);
		chat->SetNickLocale(index, locale);
		}
	}

} // namespace WWOnline
