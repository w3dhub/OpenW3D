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

//
// Filename:     netinterface.cpp
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//

#include "netinterface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "miscutil.h"
#include "wwdebug.h"
#include <windows.h>
#include "gamespyadmin.h"
#include "useroptions.h"

//
// class statics
//
WideStringClass	cNetInterface::Nickname;
int cNetInterface::mSidePreference = -1;

//-----------------------------------------------------------------------------
cNetInterface::cNetInterface(void)
{
}

//-----------------------------------------------------------------------------
cNetInterface::~cNetInterface(void)
{
}

//-----------------------------------------------------------------------------
WideStringClass cNetInterface::Get_Nickname(void)
{
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		
		//
		// If the gamespy nickname is blank, set it to "Unnamed"
		//
		if (!::strcmp(cUserOptions::GameSpyNickname.Get(), "")) {
			char hostname[MAX_COMPUTERNAME_LENGTH + 1] = {0};
			DWORD size = sizeof(hostname);
			if (!::GetComputerNameA(hostname, &size) || hostname[0] == '\0') {
				strcpy(hostname, "Unnamed");
			}

			// GameSpy nicknames are limited to 30 characters elsewhere.
			const size_t max_gamespy_length = 30;
			if (strlen(hostname) > max_gamespy_length) {
				hostname[max_gamespy_length] = '\0';
			}

			cUserOptions::GameSpyNickname.Set(hostname);
		}

		WideStringClass wide_name;
		wide_name.Convert_From(cUserOptions::GameSpyNickname.Get());
		return wide_name;
	} else {
		return Nickname;
	}
}

//-----------------------------------------------------------------------------
void cNetInterface::Set_Nickname(WideStringClass & name)
{
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		WideStringClass wide_name = name;
		if (wide_name.Get_Length() > 30) {
			wide_name[30] = 0;
		}
		StringClass nickname;
		wide_name.Convert_To(nickname);
		cUserOptions::GameSpyNickname.Set(nickname.Peek_Buffer());
	} else {
		Nickname = name;

		//
		// Abbreviate to 9 chars
		//
		if (Nickname.Get_Length() > 9) {
			Nickname[9] = 0;
		}
	}
}

//-----------------------------------------------------------------------------
void cNetInterface::Set_Random_Nickname(void)
{      
	char name[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = sizeof(name);
	::GetComputerNameA(name, &size);

	int length_test = MAX_COMPUTERNAME_LENGTH + 1 - MAX_NICKNAME_LENGTH;
	if (length_test > 0) {
		name[MAX_NICKNAME_LENGTH - 1] = 0;
	}

	WideStringClass widename;
	widename.Convert_From(name);

   Set_Nickname(widename);
}


void cNetInterface::Set_Side_Preference(int side)
{
	mSidePreference = side;
}


int cNetInterface::Get_Side_Preference(void)
{
	return mSidePreference;
}
























//WideStringClass & cNetInterface::Get_Nickname(void)
	/*
	Nickname = name;

  //
	// Abbreviate to 9 chars
	//
	if (Nickname.Get_Length() > 9) {
		Nickname[9] = 0;
	}
	*/

	/*
	Nickname = name;
	
	int max_len = 0;
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		max_len = 34;
	} else {
		max_len = 9;
	}

	if (Nickname.Get_Length() > max_len) {
		Nickname[max_len] = 0;
	}
	*/

   //return Nickname;

