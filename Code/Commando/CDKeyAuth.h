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

#ifndef _CDKEYAUTH_H_
#define _CDKEYAUTH_H_

/********
INCLUDES
********/
#include <WWLib\wwstring.h>

/********
DEFINES
********/

class CCDKeyAuth
{

protected:

	static void auth_callback(int gameid, int localid, int authenticated, char *errmsg, void *instance);

public:
	static void GetSerialNum(StringClass &serial);
	static void DisconnectUser(int localid);
	static void AuthenticateUser(int localid, ULONG ip, char *challenge, char *authstring);
	static char *GenChallenge(int nchars);
	static void AuthSerial(const char *challenge, StringClass &resp);

};


#endif // CDKEYAUTH
