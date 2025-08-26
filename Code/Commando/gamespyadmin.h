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
// Filename:     gamespyadmin.h
// Author:       Tom Spencer-Smith
// Date:         Jan 2002
// Description:  Gamespy support
//

#ifndef __GAMESPY_H__
#define __GAMESPY_H__

#include "bittype.h"
#include "DlgWOLWait.h"
#include <wwlib/Notify.h>
#include <WWOnline/RefPtr.h>
// #include <wwlib/refcount.h>


class WideStringClass;

//-----------------------------------------------------------------------------
class cGameSpyAdmin :
public Observer<DlgWOLWaitEvent>
//, public RefCountClass

{
public:
	static void					Think(void);
	static void					Reset(void);

	static void					Set_Is_Under_Gamespy_Menuing(bool flag)				{IsUnderGamespyMenuing = flag;}
	static bool					Get_Is_Under_Gamespy_Menuing(void)						{return IsUnderGamespyMenuing;}
	static void					Set_Is_Launch_From_Gamespy_Requested(bool flag)		{IsLaunchFromGamespyRequested = flag;}
	static bool					Get_Is_Launch_From_Gamespy_Requested(void)			{return IsLaunchFromGamespyRequested;}
	static void					Set_Is_Launched_From_Gamespy(bool flag)				{IsLaunchedFromGamespy = flag;}
	static bool					Get_Is_Launched_From_Gamespy(void)						{return IsLaunchedFromGamespy;}
	static void					Set_Is_Server_Gamespy_Listed(bool flag)				{IsServerGamespyListed = flag;}
	static bool					Get_Is_Server_Gamespy_Listed(void)						{return IsServerGamespyListed;}
	static void					Set_Game_Host_Ip(ULONG ip);
	static void					Set_Game_Host_Port(USHORT port);
	static bool					Is_Gamespy_Game(void);
	static bool					Is_Nickname_Collision(WideStringClass & nickname);
	static void					Set_Password_Attempt(WideStringClass & password)	{PasswordAttempt = password;}
	static WideStringClass & Get_Password_Attempt(void)								{return PasswordAttempt;}

private:
	void						HandleNotification(DlgWOLWaitEvent& event);
	static void					Join_Server(void);
	static void					Connect_To_Game_Server(void);

	static bool					DetectingBandwidth;
	static bool					IsUnderGamespyMenuing;
	static bool					IsLaunchFromGamespyRequested;
	static bool					IsLaunchedFromGamespy;
	static bool					IsServerGamespyListed;
	static ULONG				GameHostIp;
	static USHORT				GameHostPort;
	static WideStringClass	PasswordAttempt;
};

//-----------------------------------------------------------------------------

#endif // __GAMESPY_H__




	//static WideStringClass	PlayerNickname;
	//static void					Set_Player_Nickname(WideStringClass & nickname);
