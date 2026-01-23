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
 *                     $Archive:: /Commando/Code/Commando/CDKeyAuth.cpp                       $*
 *                                                                                             *
 *              Original Author:: Brian Hayes                                                  *
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/15/02 2:56p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "unichar.h"
#include <gamespy/gcdkey/gcdkeys.h>
#include <gamespy/gcdkey/gcdkeyc.h>
#include <gamespy/nonport.h>
#include <gamespy/md5.h>
#include <stdlib.h>
#include "wwdebug.h"
#include "CDKeyAuth.h"
#include "gamespyauthmgr.h"
#include "registry.h"
#include "playermanager.h"
#include "_globals.h"
#include "GameSpy_QnR.h"
#include "ServerSettings.h"

// static void c_auth_callback(int localid, int authenticated, char *errmsg, void *instance)
// {
//	((CCDKeyAuth *)instance)->auth_callback(localid, authenticated, errmsg);
// }


//generate a rand nchar challenge
char * CCDKeyAuth::GenChallenge(int nchars)
{
	static char s[33];
	if (nchars > 32) nchars = 32;
	s[nchars] = 0;
	while (nchars--)
	{
		s[nchars] = (char)('a' + rand() % 26);
	}
	return s;
}

/* Callback function to indicate whether a client has been authorized or not.
If the client has been, then we send them a "welcome" string, representative of
allowing them to "enter" the game. If they have not been authenticated, we dump
them after sending an error message */
void CCDKeyAuth::auth_callback(int /* gameid */, int localid, int authenticated, [[maybe_unused]] char *errmsg, void * /* instance */)
{
//	client_t *clients = (client_t *)instance;
//	char outbuf[512];

	WWDEBUG_SAY(("CDKeyAuth -- %d:%d:%s\n", localid, authenticated, errmsg));

	cPlayer * p_player = cPlayerManager::Find_Player(localid);
	if (p_player != NULL) {

		if (!authenticated) //doh.. bad!
		{
			// This client failed.
			//p_player->Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_REJECTING);
			//cGameSpyAuthMgr::Evict_Player(localid);
			cGameSpyAuthMgr::Initiate_Auth_Rejection(localid);

	//		printf("Client %d was NOT authenticated (%s)\n",localid, errmsg);
	//		sprintf(outbuf,"E:%s\n",errmsg);
	//		send(clients[localid].sock, outbuf, strlen(outbuf),0);
	//		shutdown(clients[localid].sock, 2);
	//		closesocket(clients[localid].sock);
	//		clients[localid].sock = INVALID_SOCKET;
		} else
		{
			// This client passed.
			p_player->Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_ACCEPTED);
	//		printf("Client %d was authenticated (%s)\n",localid, errmsg);
	//		sprintf(outbuf,"M:Welcome to the game, have fun! (%s)\n",errmsg);
	//		send(clients[localid].sock, outbuf, strlen(outbuf),0);
		}
	}
}

void CCDKeyAuth::DisconnectUser(int localid) {
	gcd_disconnect_user(CGameSpyQnR::cdkey_id, localid);

}

void CCDKeyAuth::AuthenticateUser(int localid, ULONG ip, char *challenge, char *authstring) {

	// Customize this with our playerdata struct
	// Take the response from our challenge that we sent to the client
	// and send it off to the Authserver along with the original challenge

	gcd_authenticate_user(CGameSpyQnR::cdkey_id, localid, ip, challenge, authstring, CCDKeyAuth::auth_callback, NULL, NULL);

}


void CCDKeyAuth::AuthSerial(const char *challenge, StringClass &resp) {

	char response[RESPONSE_SIZE];
	StringClass sserial;

	WWASSERT(challenge);

	GetSerialNum(sserial);

	const char *serial = sserial;

	char *cdkey = new char [strlen(serial)+1];
	char *outb = cdkey;
	const char *linep = serial;
	char md5hash[33];

	while (*linep) {
		if (*linep >= '0' && *linep <= '9') {
			*outb++ = *linep;
		}
		linep++;
	}
	*outb = 0;

	// MD5 Hash Here.
	MD5Digest((BYTE *)cdkey, strlen(cdkey), md5hash);

	// hashserial, challenge, outbuf
	gcd_compute_response(cdkey, (char *)challenge, response, CDResponseMethod_NEWAUTH);

	delete [] cdkey;
	resp = response;
}

void CCDKeyAuth::GetSerialNum(StringClass &serial) {

	RegistryClass main_reg(APPLICATION_SUB_KEY_NAME);
	StringClass stringval;
	StringClass serial_out;
	main_reg.Get_String("Serial", stringval);
	ServerSettingsClass::Encrypt_Serial(stringval, serial, false);
}
