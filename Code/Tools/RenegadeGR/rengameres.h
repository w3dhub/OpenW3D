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
File Name   : rengameres.h
Author      : Joe Howes (jhowes@westwood.com)
Start Date  : Apr 5, 2000
Last Update :
------------------------------------------------------------------------------

Wraps game results for a game of multiplayer Renegade and provides method
for sending to a WOL game results server.

\****************************************************************************/

#ifndef __RENGAMERES_H_
#define __RENGAMERES_H_


/*----------------------------------------------------------------------.
| MACROS                                                                |
`----------------------------------------------------------------------*/
//#define GRSETTING_USING_WOLAPI				// Uncomment if the class is
												// being used with the game
												// (as opposed to the test applet)
static unsigned int GR_SCORE_SCALE = 10000000;
static unsigned int GR_BASE_SKU = 8704;

// Errors
static const int GR_ERROR_BIND_FAILED		= -100;
static const int GR_ERROR_CONNECT_FAILED	= -101;

// Game Specific
static char GR_GAME_ID[5]			= { "IDNO" };
static char GR_PLAYER_COUNT[5]		= { "PLRS" };	
static char GR_CLAN_GAME[5]			= { "CLGM" };	
static char GR_DURATION[5]			= { "DURA" };	
static char GR_MAP_NAME[5]			= { "MPNM" };	
static char GR_SKU[5]				= { "GSKU" };	
static char GR_STYLE[5]				= { "GSTY" };	
static char GR_NUM_CLANS[5]			= { "NTMS" };	
static char GR_START_TIME[5]		= { "TIME" };	
static char GR_TOURNAMENT[5]		= { "TRNY" };	

// Player Specific
static char GR_LOGINS[5]			= { "LGL?" };
static char GR_SCORES[5]			= { "SCO?" };	
static char GR_CLANIDS[5]			= { "CLN?" };
static char GR_DURATIONS[5]			= { "DUR?" };
static char GR_IPS[5]				= { "IPL?" };
static char GR_DEATHS[5]			= { "DTH?" };
static char GR_KILLS[5]				= { "KIL?" };
static char GR_SELFKILLS[5]			= { "SKL?" };
static char GR_DAMAGEPOINTS[5]		= { "DMP?" };


/*----------------------------------------------------------------------.
| DATATYPES																|
`----------------------------------------------------------------------*/
typedef enum gamestyle
{
	GR_DEATHMATCH = 0, 
	GR_CAPTUREFLAG, 
	GR_FLAGBALL,
	GR_KINGREALM,
	GR_HIGHLANDER,
	GR_GAUNTLET,

	GR_NUM_GAMESTYLES
} GameStyle;

typedef enum language
{
	GR_ENGLISH = 0,
	GR_UK_ENGLISH,
	GR_GERMAN,
	GR_FRENCH,
	GR_DUTCH,
	GR_ITALIAN,
	GR_JAPANESE,
	GR_SPANISH,
	GR_SCANDINAVIAN,
	GR_KOREAN,

	NUM_LANGUAGES
} Language;


class RenegadeGameRes
{
public:
	// METHODS
	RenegadeGameRes(const char* host = NULL, int port = 0) :
					    _host(NULL), _port(0),
						_myplayercount(0), _game_id(0), _player_count(0), _clan_game(0), 
						_duration(0), _map_name(NULL), _sku(0), _style(GR_DEATHMATCH), 
						_num_clans(0), _start_time(0), _tournament(0), _logins(NULL), 
						_scores(NULL), _clan_ids(NULL), _durations(NULL), _ips(NULL), 
						_deaths(NULL), _kills(NULL), _selfkills(NULL), _damagepoints(NULL)
	{
		if( host != NULL )
		{
			setHost(host);
			setPort(port);
		}
	}

	~RenegadeGameRes();


	void setHost(const char* host)				
	{ 
		if( _host != NULL )
			delete[] _host;
		_host = new char[strlen(host)+1];	
		strcpy(_host, host); 
	}
	void setPort(int val)						{ _port = val; }
	void setGameID(int unsigned int val)		{ _game_id = val; }	
	void setPlayerCount(unsigned char val)		{ _player_count = val; }	
	void setClanGame(unsigned char val)			{ _clan_game = val; }	
	void setDuration(int unsigned int val)		{ _duration = val; }	
	void setMapName(const char* val);
	void setSKU(Language lang)					{ _sku = GR_BASE_SKU | lang; }	
	void setStyle(GameStyle val)				{ _style = (unsigned char)val; }	
	void setNumClans(unsigned char val)			{ _num_clans = val; }	
	void setStartTime(int unsigned int val)	{ _start_time = val; }	
	void setTournament(unsigned char val)		{ _tournament = val; }	

	void addPlayer(const char* login = "INVALID", double score = 0.0, int unsigned int clan_id = 0,
				   int unsigned int duration = 0, int unsigned int ip = 0, 
				   int unsigned int deaths = 0, int unsigned int kills = 0, 
				   int unsigned int selfkills = 0, int unsigned int damagepoints = 0);

	int sendResults();



private:	
	// METHODS
	char** _addToArr(char** arr, const char* item);
	int unsigned int* _addToArr(int unsigned int* arr, int unsigned int item);


	// MEMBERS
	char*				_host;
	int					_port;
	int					_myplayercount;

	// Game Specific
	int unsigned int	_game_id;
	unsigned char		_player_count;
	unsigned char		_clan_game;				// Boolean
	int unsigned int	_duration;				// Secs since epoch
	char*				_map_name;				// Must be NULL terminated
	int unsigned int	_sku;
	unsigned char		_style;					// Will be converted to an unsigned char
	unsigned char		_num_clans;
	int unsigned int	_start_time;			// Secs since epoch
	unsigned char		_tournament;			// Boolean

	// Player Specific  (These are all arrays)
	char**				_logins;				// Must be NULL terminated
	int unsigned int*	_scores;
	int unsigned int*	_clan_ids;
	int unsigned int*	_durations;				// Secs since epoch
	int unsigned int*	_ips;					// As integers, not dotted quads
	int unsigned int*	_deaths;
	int unsigned int*	_kills;
	int unsigned int*	_selfkills;
	int unsigned int*	_damagepoints;	
};


#endif