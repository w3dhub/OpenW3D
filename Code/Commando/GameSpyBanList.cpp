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
 *                 Project Name : Renegade                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/GameSpyBanList.cpp                  $*
 *                                                                                             *
 *              Original Author:: Brian Hayes                                                  *
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/15/02 2:55p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


//
// Filename:     GameSpyBanList.cpp
// Author:       Brian Hayes
// Date:         Mar 2002
// Description:  Maintains a list of banned nicknames/hashes/ipaddresses for GameSpy Servers
//

#include "cnetwork.h"
#include "listnode.h"
#include "GameSpyBanList.h"
#include "ini.h"
#include "registry.h"
#include "rawfile.h"
#include "gamespyauthmgr.h"
#include "sctextobj.h"
#include "ConsoleMode.h"
#include "gamesideservercontrol.h"
#include "wwstring.h"
#include <cstdio>


cGameSpyBanList GameSpyBanList;

BanEntry::BanEntry(const char *name, const char *ip, const char *hash_id, const char *ip_mask, bool rtype) {

	memset(nickname, 0, sizeof(nickname));
	ipaddress = 0;
	ipmask = 0xffffffff;
	memset(hashid, 0, sizeof(hashid));
	ruletype = rtype;

	if (name) {
		strncpy(nickname, name, sizeof(nickname) - 1);
	}
	if (ip) ipaddress = inet_addr(ip);
	if (hash_id) {
		strncpy(hashid, hash_id, sizeof(hashid) - 1);
	}
	if (ip_mask) {
		ipmask = inet_addr(ip_mask);
		if (!ipmask) ipmask = 0xffffffff;
	}
}

cGameSpyBanList::cGameSpyBanList() : BanList(NULL) {

	BanList = new List<BanEntry *> ();
}

cGameSpyBanList::~cGameSpyBanList() {

	BanList->Delete();
	delete BanList;
}

void cGameSpyBanList::Ban_User(const char *nickname, const char *challenge_response, DWORD ipaddr) {

	char *buff = NULL;
	char *q = NULL;
	FILE *outf = NULL;
	BanEntry *t = NULL;

	if (nickname && !challenge_response) {
		q = buff = new char[(strlen(nickname)*2)+1];
		const char *n = nickname;
		while (*n) {
			if (*n == '%') *q++ = '%';
			*q++ = *n++;
		}
		*q = 0;
	 	t = new BanEntry(buff);
		delete [] buff;
	} else if (challenge_response) {
	 	t = new BanEntry(NULL, NULL, challenge_response);
	} else if (ipaddr) {
	 	t = new BanEntry(NULL, cNetUtil::Address_To_String(ipaddr));
	} else {
		return;
	}
 	BanList->Add_Tail(t);

	outf = fopen("banlist.txt", "at");

	if (outf) {
		fprintf(outf, "\"%s\" \"%s\" \"%s\" \"%s\" \"%s\"; \"%s\" console BAN\n", t->Get_Rule_Type() ? "Allow" : "Deny",
			t->Get_Nick_Name(), t->Get_Hash_ID(), 
			t->Get_Ip_Address() ? cNetUtil::Address_To_String(t->Get_Ip_Address()) : "", 
			t->Get_Ip_Address() ? "255.255.255.255" : "", nickname ? nickname : "");
		fclose(outf);
	}
}

void cGameSpyBanList::Think(void) {

	cPlayer *player = NULL;
	for (SLNode<cPlayer> *player_node = cPlayerManager::Get_Player_Object_List ()->Head ()
		; player_node != NULL; player_node = player_node->Next ()) {
		
		player = player_node->Data ();
		WWASSERT (player != NULL);
		
		if (player->Get_Is_Active().Is_False() || !player->Is_Human()) {
			continue;
		}
		if (player->Get_GameSpy_Kick_State() == GAMESPY_KICK_STATE_BEGIN &&
				TIMEGETTIME() - player->Get_GameSpy_Kick_State_Entry_Time_Ms() > 3000) {
			Final_Player_Kick(player->Get_Id());
		} else if (player->Get_GameSpy_Kick_State() == GAMESPY_KICK_STATE_INITIAL &&
				player->Get_GameSpy_Auth_State() == GAMESPY_AUTH_STATE_ACCEPTED) {
			if (Is_User_Banned(StringClass(player->Get_Name()), player->Get_GameSpy_Hash_Id(), 
					player->Get_Ip_Address())) {
				Begin_Player_Kick(player->Get_Id());
			} else {
				player->Set_GameSpy_Kick_State(GAMESPY_KICK_STATE_APPROVED);
			}
		}
	}
}


bool cGameSpyBanList::Begin_Player_Kick(int id) {

	cPlayer *player = cPlayerManager::Find_Player(id);
	if (player && player->Get_Is_Active().Is_True() && player->Is_Human()) {

		cScTextObj * p_message = new cScTextObj;
		p_message->Init(U_CHAR("You've been kicked from the server. Please quit."), TEXT_MESSAGE_PRIVATE, true, HOST_TEXT_SENDER, player->Get_Id());
		player->Set_GameSpy_Kick_State(GAMESPY_KICK_STATE_BEGIN);
		return true;
	}
	return false;
}

bool cGameSpyBanList::Final_Player_Kick(int id) {

	cPlayer *player = cPlayerManager::Find_Player(id);
	if (player && player->Get_Is_Active().Is_True() && player->Is_Human()) {
		StringClass user_name = static_cast<const unichar_t *>(player->Get_Name());
		ConsoleBox.Print("%s was kicked\n", user_name);
		player->Set_GameSpy_Kick_State(GAMESPY_KICK_STATE_KICKED);
		cNetwork::Server_Kill_Connection(id);
		cNetwork::Cleanup_After_Client(id);
		return true;
	}
	return false;
}

bool cGameSpyBanList::Is_User_Banned(const char *nickname, const char *challenge_response, 
									 DWORD ipaddress) {

	BanEntry *t = BanList->First();

	bool ret = false;

	for (;t && t->Is_Valid(); t = t->Next()) {

		if (challenge_response && *challenge_response && 
				strcmp(challenge_response, t->Get_Hash_ID()) == 0) {
			ret = true;
		} else if (t->Get_Hash_ID() && strlen(t->Get_Hash_ID())){
			ret = false;
			continue;
		}

		if (ipaddress && t->Get_Ip_Address() && ((ipaddress & t->Get_Ip_Netmask()) == 
				(t->Get_Ip_Address() & t->Get_Ip_Netmask()))) {
			ret = true;
		} else if (t->Get_Ip_Address()) {
			ret = false;
			continue;
		}

		if (t->Get_Nick_Name() && nickname && strlen(t->Get_Nick_Name()) && strlen(nickname)) {
			char *a = strdup(t->Get_Nick_Name());
			char *b = strdup(nickname);

            openw3d::string_to_upper(a); openw3d::string_to_upper(b);

			//
			// This code means that you can put a BAN name in as
			// "%foo" == match on "foo" at the end of the string
			// "foo%" == match on "foo" at the beginning of the string
			// "%foo%" == match on "foo" anywhere in the string
			// 
			// to match on a literal % in the nickname use %%
			//
			// Note the use of small var names to make it even more confusing
			if ( ((a[0] == '%') && (a[1] != '%')) &&
				((a[strlen(a)-1] == '%') && (a[strlen(a)-2] != '%')) ) {
				a[0] = 0;
				a[strlen(&a[1])] = 0;
				Strip_Escapes(&a[1]);
				ret = (strstr(b, &a[1]) != NULL);
			} else if ((a[0] == '%') && (a[1] != '%')) {
				a[0] = 0;
				Strip_Escapes(&a[1]);
				if (strlen(b) > (strlen(&a[1])-1)) {
					ret = (strcmp(&b[strlen(b)-strlen(&a[1])], &a[1]) == 0);
				}
			} else if (strlen(a) > 1 && (a[strlen(a)-1] == '%') && (a[strlen(a)-2] != '%')) {
				a[strlen(a)-1] = 0;
				Strip_Escapes(a);
				if (strlen(b) > (strlen(a)-1)) {
					b[strlen(a)] = 0;
					ret = (strcmp(b, a) == 0);
				}
			} else {
				Strip_Escapes(a);
				ret = (strcmp(b, a) == 0);
			}

			free(a); free(b);
		}

		if (ret) {
			ret = !t->Get_Rule_Type();
			break;
		}
	}

	if (t && t->Is_Valid()) {
		// matched on a rule
		return ret;
	} else {
		// Everyone is good if they don't match a rule
		return false;
	}

}

void cGameSpyBanList::Strip_Escapes(char *var) {
	char *q = (char *)var;
	while ((q = strstr(q, "%%")) != NULL) {
		memmove(q, q+1, strlen(q)); 
		q++;
	}
}

void cGameSpyBanList::LoadBans(void) {

	char buff[512];
	FILE *outf = NULL;

	if (!BanList->Is_Empty()) BanList->Delete();

	outf = fopen("banlist.txt", "rt");
	if (!outf) return;
	buff[sizeof(buff)-1] = 0;

	while (fgets(buff, sizeof(buff)-1, outf)) {
		// Format of each line "ruletype" "nickname" "hashid" "ip" "netmask"
		char *nickname = NULL;
		char *ruletype = NULL;
		char *ip = NULL;
		char *hashid = NULL;
		char *ipmask = NULL;

		if (strlen(buff) < 3) continue;
		if (buff[0] == ';') continue;

		char *s = strchr(buff, '"');
		if (!s) continue;
		*s++ = 0;
		char *q = strchr(s, '"');
		if (!q) continue;
		*q++ = 0;
		ruletype = s;

		s = strchr(q, '"');
		if (!s) continue;
		*s++ = 0;
		q = strchr(s, '"');
		if (!q) continue;
		*q++ = 0;
		nickname = s;

		while (q) {
			s = strchr(q, '"');
			if (!s) break;
			*s++ = 0;
			q = strchr(s, '"');
			if (!q) break;
			*q++ = 0;
			hashid = s;

			s = strchr(q, '"');
			if (!s) break;
			*s++ = 0;
			q = strchr(s, '"');
			if (!q) break;
			*q++ = 0;
			ip = s;

			s = strchr(q, '"');
			if (!s) break;
			*s++ = 0;
			q = strchr(s, '"');
			if (!q) break;
			*q++ = 0;
			ipmask = s;
			break;
		}

		BanEntry *t = new BanEntry(nickname, ip, hashid, ipmask, stricmp(ruletype, "Allow") == 0);
		BanList->Add_Tail(t);
	}
	fclose(outf);
}
