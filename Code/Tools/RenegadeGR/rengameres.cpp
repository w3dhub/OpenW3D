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

/****************************************************************************\
*        C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S         *
******************************************************************************
Project Name: Generic Game Results Server
File Name   : rengameres.cpp
Author      : Joe Howes (jhowes@westwood.com)
Start Date  : Apr 5, 2000
Last Update :
------------------------------------------------------------------------------

Wraps game results for a game of multiplayer Renegade and provides method
for sending to a WOL game results server.

\****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "packet.h"
#include "tcpmgr.h"
#include "tcpcon.h"
#include "wencrypt.h"
#include "rengameres.h"


RenegadeGameRes::~RenegadeGameRes()
{
	if( _host != NULL ) delete[] _host;
	if( _map_name != NULL ) delete[] _map_name;
	if( _logins != NULL )
	{
		for(int i = 0; i < _myplayercount; i++)
			if( _logins[i] != NULL ) delete[] _logins[i];
		delete[] _logins;
	}
	if( _scores != NULL ) delete[] _scores;
	if( _clan_ids != NULL ) delete[] _clan_ids;
	if( _durations != NULL ) delete[] _durations;
	if( _ips != NULL ) delete[] _ips;
	if( _deaths != NULL ) delete[] _deaths;
	if( _kills != NULL ) delete[] _kills;
	if( _selfkills != NULL ) delete[] _selfkills;
	if( _damagepoints != NULL ) delete[] _damagepoints;
}


void RenegadeGameRes::setMapName(const char* val)
{
	if( _map_name != NULL ) delete[] _map_name;

	if( val == NULL ) return;
	
	_map_name = new char[strlen(val)+1];
	strcpy(_map_name, val);
}


void RenegadeGameRes::addPlayer(const char* login, double score, long unsigned int clan_id,
								long unsigned int duration, long unsigned int ip, 
								long unsigned int deaths, long unsigned int kills, 
								long unsigned int selfkills, long unsigned int damagepoints)
{	
	char** newstr = _addToArr(_logins, login);
	delete[] _logins;
	_logins = newstr;

	// There's no happy easy way to shoot a floating value over
	// the wire.  The values in jscores are in (0.0 <= x <= 1.0), so
	// we take the maximum value for a 4-byte int and multiply it
	// by the jscore floating value.  We now have a scale of the same
	// accuracy as a 4-byte float.
	
	// Expecting a score X such that -0.5 <= X <= 0.5
	score += 0.5;	// So that it can be stored unsigned
	long unsigned int convscore = (long unsigned int)(score * GR_SCORE_SCALE);
	long unsigned int* newlui = _addToArr(_scores, convscore);
	delete[] _scores;
	_scores = newlui;

	newlui = _addToArr(_clan_ids, clan_id);
	delete[] _clan_ids;
	_clan_ids = newlui;

	newlui = _addToArr(_durations, duration);
	delete[] _durations;
	_durations = newlui;

	newlui = _addToArr(_ips, ip);
	delete[] _ips;
	_ips = newlui;

	newlui = _addToArr(_deaths, deaths);
	delete[] _deaths;
	_deaths = newlui;

	newlui = _addToArr(_kills, kills);
	delete[] _kills;
	_kills = newlui;

	newlui = _addToArr(_selfkills, selfkills);
	delete[] _selfkills;
	_selfkills = newlui;

	newlui = _addToArr(_damagepoints, damagepoints);
	delete[] _damagepoints;
	_damagepoints = newlui;

	_myplayercount++;	// *** MUST BE INCREMENTED __AFTER__ ADDING ALL THIS STUFF!!!! ***
}


int RenegadeGameRes::sendResults()
{
	assert( _host != NULL );
	assert( _port != 0 );
	//GameResPacket grPacket
	// Build the packet
	PacketClass rawPacket;
	rawPacket.Add_Field(GR_GAME_ID, _game_id);
	rawPacket.Add_Field(GR_PLAYER_COUNT, (long)_player_count);
	rawPacket.Add_Field(GR_CLAN_GAME, _clan_game);
	rawPacket.Add_Field(GR_DURATION, _duration);
	rawPacket.Add_Field(GR_MAP_NAME, _map_name);
	rawPacket.Add_Field(GR_SKU, _sku);
	rawPacket.Add_Field(GR_STYLE, _style);
	rawPacket.Add_Field(GR_NUM_CLANS, _num_clans);
	rawPacket.Add_Field(GR_START_TIME, _start_time);
	rawPacket.Add_Field(GR_TOURNAMENT, _tournament);

	
	for (int i = 0 ; i < _player_count; i++) 
  	{
    	GR_LOGINS[3] = (char)('0' + (char)i);
    	rawPacket.Add_Field(GR_LOGINS, (char *)((const char*)_logins[i]));

		GR_SCORES[3] = (char)('0' + (char)i);
		rawPacket.Add_Field(GR_SCORES, _scores[i]);

		GR_CLANIDS[3] = (char)('0' + (char)i);
		rawPacket.Add_Field(GR_CLANIDS, _clan_ids[i]);

		GR_DURATIONS[3] = (char)('0' + (char)i);
		rawPacket.Add_Field(GR_DURATIONS, _durations[i]);

		GR_IPS[3] = (char)('0' + (char)i);
		rawPacket.Add_Field(GR_IPS, _ips[i]);

		GR_DEATHS[3] = (char)('0' + (char)i);
		rawPacket.Add_Field(GR_DEATHS, _deaths[i]);

		GR_KILLS[3] = (char)('0' + (char)i);
		rawPacket.Add_Field(GR_KILLS, _kills[i]);

		GR_SELFKILLS[3] = (char)('0' + (char)i);
		rawPacket.Add_Field(GR_SELFKILLS, _selfkills[i]);

		GR_DAMAGEPOINTS[3] = (char)('0' + (char)i);
		rawPacket.Add_Field(GR_DAMAGEPOINTS, _damagepoints[i]);
	}
	int packetsize = 0;
	void* outPacket = rawPacket.Create_Comms_Packet(packetsize);
	void* encPacket = PrepareEncryptedPacket((unsigned char*)outPacket, &packetsize);
	bit8 result = 0;
	sint32 sendlen = 0;
	

	// If the _host member is not set, then this method is being called in-game
	// and wants to use the WOLAPI methods to send game results.  Otherwise it
	// is some small test application and we will use the tcp classes provided by Neal.
#ifndef GRSETTING_USING_WOLAPI

	TCPMgr	tcpMgr;
	TCPCon*	tcpCon;
	uint32 handle = -1;
	result = tcpMgr.connect(_host, _port, &handle);
	if( result == false )
		sendlen = GR_ERROR_BIND_FAILED;
	else
	{
		result = tcpMgr.getOutgoingConnection(&tcpCon, handle, 5);
		if( result == false )
			sendlen = GR_ERROR_CONNECT_FAILED;
		else
		{
			sendlen = tcpCon->write((uint8*)encPacket, packetsize, 5);
			tcpCon->close();
		}
	}

#else

	result = RequestGameresSend( _host, _port, encPacket, packetsize );

#endif


	// Clean up
	delete[] outPacket;
	delete[] encPacket;


	if( _host != NULL )
		return sendlen;
	else
		return (int)result;


	return 0;
}


/*----------------------------------------------------------------------------------.
| METHOD: _addToArr																	|
| Takes a pointer to an array and a new item, constructs a new array and returns	|
| a pointer to it.																	|
`----------------------------------------------------------------------------------*/
char** RenegadeGameRes::_addToArr(char** arr, const char* item)
{
	char** newarr = NULL;


	if( arr == NULL )
	{
		// Make a new array
		assert( _myplayercount == 0 );
		newarr = new char*[1];
		newarr[0] = new char[strlen(item)+1];	// Space for the new item
		// Add the new item
		strcpy(newarr[_myplayercount], item);
	}
	else
	{
		// Make a new array and copy all the old stuff over
		int i = 0;
		assert( _myplayercount > 0 );
		newarr = new char*[_myplayercount+1];
		for(i = 0; i < _myplayercount; i++)
			newarr[i] = arr[i];
		newarr[_myplayercount] = new char[strlen(item)+1];
		// Add the new item
		strcpy(newarr[_myplayercount], item);
	}
	return newarr;
}


/*----------------------------------------------------------------------------------.
| METHOD: _addToArr																	|
| Takes a pointer to an array and a new item, constructs a new array and returns	|
| a pointer to it.																	|
`----------------------------------------------------------------------------------*/
long unsigned int* RenegadeGameRes::_addToArr(long unsigned int* arr, long unsigned int item)
{
	long unsigned int* newarr = NULL;


	if( arr == NULL )
	{
		// Make a new array
		assert( _myplayercount == 0 );
		newarr = new long unsigned int[1];		
		// Add the new item
		newarr[0] = item;
	}
	else
	{
		// Make a new array and copy all the old stuff over
		assert( _myplayercount > 0 );
		newarr = new long unsigned int[_myplayercount+1];
		for(int i = 0; i < _myplayercount; i++)
			newarr[i] = arr[i];
		// Add the new item
		newarr[_myplayercount] = item;
	}

	return newarr;
}
