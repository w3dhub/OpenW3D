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


#ifndef _GAMESPY_QNR_H_
#define _GAMESPY_QNR_H_

/********
INCLUDES
********/
#include <GameSpy/qr/gqueryreporting.h>
#include <WWLib\WideString.h>
#include "trim.h"

/********
DEFINES
********/

class CGameSpyQnR
{
	friend class CCDKeyAuth;
protected:
	char secret_key[9];
	BOOL m_GSInit;
	BOOL m_GSEnabled;
	qr_t query_reporting_rec;
	void DoGameStuff(void);
	BOOL Append_InfoKey_Pair(char *outbuf, int maxlen, const char *key, const char *value);
	BOOL Append_InfoKey_Pair(char *outbuf, int maxlen, const char *key, const StringClass &value);
	BOOL Append_InfoKey_Pair(char *outbuf, int maxlen, const char *key, const WideStringClass &value);
	static const char *gamename;
	static const char *bname;
	static const int prodid;
	static const int cdkey_id;
	static const char *default_heartbeat_list;
	int StartTime;

public:
	void Init(void);
	void LaunchArcade(void);
	void Shutdown(void);
	BOOL Parse_HeartBeat_List(const char *list);
	const char *Get_GameSpy_GameName(void) { return gamename; } 
	const char *Get_Default_HeartBeat_List(void) { return default_heartbeat_list; } 
	void Enable_Reporting(BOOL enable) { m_GSEnabled = enable; }
	BOOL IsEnabled(void) { return m_GSEnabled; }
	void Think();
	void basic_callback(char *outbuf, int maxlen); 
	void info_callback(char *outbuf, int maxlen);
	void rules_callback(char *outbuf, int maxlen);
	void players_callback(char *outbuf, int maxlen);
	CGameSpyQnR();
	virtual ~CGameSpyQnR();

};

extern CGameSpyQnR GameSpyQnR;

#endif