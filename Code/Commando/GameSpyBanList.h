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
// Filename:     GameSpyBanList.h
// Author:       Brian Hayes
// Date:         Mar 2002
// Description:  Maintains a list of banned nicknames/hashes/ipaddresses for GameSpy Servers
//

#ifndef __GAMESPYBANLIST_H__
#define __GAMESPYBANLIST_H__

#include "bittype.h"

enum GAMESPY_KICK_STATE_ENUM
{
	GAMESPY_KICK_STATE_INITIAL = 0,		// Just connected
	GAMESPY_KICK_STATE_APPROVED,		// This user is not in the ban list
	GAMESPY_KICK_STATE_BEGIN,		   	// Give them a message before we boot him
	GAMESPY_KICK_STATE_KICKED,		   	// outta here.
	GAMESPY_KICK_STATE_COUNT
};

class BanEntry : public Node<BanEntry *>  {
public:
	BanEntry(const char *name = NULL, const char *ip = NULL, const char *hash_id = NULL,
		const char *ip_mask = NULL, bool rtype = false);

protected:
	char	hashid[33];
	char	nickname[40];
	ULONG	ipaddress;
	ULONG	ipmask;
	bool	ruletype;

public:
	const char *Get_Nick_Name(void) {return (const char *)nickname;}
	const char *Get_Hash_ID(void) {return (const char *)hashid;}
	ULONG Get_Ip_Address(void) {return ipaddress;}
	ULONG Get_Ip_Netmask(void) {return ipmask;}
	bool Get_Rule_Type(void) {return ruletype;}
};

class cGameSpyBanList
{

public:
	cGameSpyBanList();
	~cGameSpyBanList();

protected:
	List<BanEntry *> * BanList;
	bool Final_Player_Kick(int id);
	bool Begin_Player_Kick(int id);
	void Strip_Escapes(char *var);

public:
	void Think(void);
	bool Kick_Player(int id) { return Begin_Player_Kick(id);}
	void Ban_User(const char *nickname, const char *challenge_response = NULL,
		ULONG ipaddress = 0xffffffff);
	bool Is_User_Banned(const char *nickname, const char *challenge_response, ULONG ipaddress);
	void LoadBans(void);
};

extern cGameSpyBanList GameSpyBanList;

#endif // __GAMESPYBANLIST_H__
