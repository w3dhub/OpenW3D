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
*     $Archive: /Commando/Code/WWOnline/WOLLoginInfo.h $
*
* DESCRIPTION
*     LoginInfo encapsulates a user's persona login information.
*     LoginInfo handles the storage/retrival of nicknames/passwords also.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 10 $
*     $Modtime: 1/18/02 11:46p $
*
******************************************************************************/

#ifndef __WOLLOGININFO_H__
#define __WOLLOGININFO_H__

#include "RefCounted.h"
#include "RefPtr.h"
#include <wwlib/widestring.h>

namespace WOL
{
#include <wolapi/WOLAPI.h>
}

#if defined(_MSC_VER)
#pragma warning(push, 3)
#endif

#include <vector>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace WWOnline {

class LoginInfo;
typedef std::vector< RefPtr<LoginInfo> > LoginInfoList;

class LoginInfo :
		public RefCounted
	{
	public:
		static const LoginInfoList& GetList(void);
		static void ClearList(void);
	
		static RefPtr<LoginInfo> Find(const wchar_t* nickname);
		static RefPtr<LoginInfo> Find(const char* nickname);

		static RefPtr<LoginInfo> Create(const wchar_t* nickname, const wchar_t* password, bool isEncrypted);
		static RefPtr<LoginInfo> Create(const char* nickname, const char* password, bool isEncrypted);

		static void StoreLogin(const char* nickname, const char* password, bool isEncrypted,
			WOL::Locale locale);

		const WideStringClass& GetNickname(void) const
			{return mNickname;}

		const WideStringClass& GetPassword(void) const
			{return mPassword;}

		void SetPassword(const wchar_t* password, bool isEncrypted);

		bool IsPasswordEncrypted(void) const
			{return mIsPasswordEncrypted;}

		void SetLocale(WOL::Locale locale);
		WOL::Locale GetLocale(void);

		bool IsStored(void) const
			{return mIsStored;}

		// Remeber the login
		void Remember(bool store);

		// Delete login from local storage
		void Forget(bool purge);

	protected:
		LoginInfo(const wchar_t* nickname, const wchar_t* password, bool isEncrypted);
		virtual ~LoginInfo();

		// Prevent copy and assignment
		LoginInfo(const LoginInfo&);
		const LoginInfo& operator=(const LoginInfo&);

		static int IndexOf(const wchar_t* nickname);
		int IndexOf(void) const;

	private:
		static LoginInfoList _mLoginList;

		WideStringClass mNickname;
		WideStringClass mPassword;
		WOL::Locale mLocale;

		bool mIsPasswordEncrypted;
		bool mIsStored;
	};


class NewLoginInfoEvent
	{
	public:
		NewLoginInfoEvent(const RefPtr<LoginInfo>& login, const WideStringClass& message) :
				mLogin(login), mMessage(message)
			{}

		const RefPtr<LoginInfo>& GetLogin(void) const
			{return mLogin;}

		const WideStringClass& GetMessage(void) const
			{return mMessage;}

	private:
		NewLoginInfoEvent(const NewLoginInfoEvent&);
		const NewLoginInfoEvent& operator=(const NewLoginInfoEvent&);

		RefPtr<LoginInfo> mLogin;
		WideStringClass mMessage;
	};

}

#endif // __WOLLOGININFO_H__
