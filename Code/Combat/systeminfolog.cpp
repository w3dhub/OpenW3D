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

#include "systeminfolog.h"
#include "registry.h"
#include "timemgr.h"
#include "debug.h"
#include "playerdata.h"
#include "combat.h"
#include "cheatmgr.h"
#include "objectives.h"
#include "ffactory.h"
#include "savegame.h"
#include "systimer.h"
#include "specialbuilds.h"

const unsigned NUM_GAMES_LOGGED=10;

//#define	COMBAT_SUB_KEY_NAME_DEBUG				"Software\\Westwood\\Renegade\\Debug"

#if	defined(FREEDEDICATEDSERVER)
#define	COMBAT_SUB_KEY_NAME_DEBUG				"Software\\Westwood\\RenegadeFDS\\Debug"
#elif defined(MULTIPLAYERDEMO)
#define	COMBAT_SUB_KEY_NAME_DEBUG				"Software\\Westwood\\RenegadeMPDemo\\Debug"
#elif defined(BETACLIENT)
#define	COMBAT_SUB_KEY_NAME_DEBUG				"Software\\Westwood\\RenegadeBeta\\Debug"
#elif defined(BETASERVER)
#define	COMBAT_SUB_KEY_NAME_DEBUG				"Software\\Westwood\\RenegadeBeta\\Debug"
#else
#define	COMBAT_SUB_KEY_NAME_DEBUG				"Software\\Westwood\\Renegade\\Debug"
#endif



#define	SYSTEM_INFO_LOG_CURRENT_STATE			"SystemInfoLog_CurrentState"
#define	SYSTEM_INFO_LOG_LATEST_GAME			"SystemInfoLog_LatestGame"

static int Team0TotalSizes;
static int Team0SizeReported;
static int Team1TotalSizes;
static int Team1SizeReported;

static unsigned AvgFPS;
static unsigned MinFPS;
static unsigned VeryMinFPS;
static unsigned MaxFPS;
static unsigned VeryMaxFPS;
static unsigned CurrentTotalFPS;
static unsigned CurrentFPSCount;
static unsigned CurrentLoadingTime;
static unsigned PlayingStartTime;
static unsigned TotalPlayingTime;
static StringClass CurrentLevel;
static StringClass CurrentString;

static void Get_Latest_Game_String(RegistryClass& reg, int i, StringClass& string)
{
	StringClass keyname(0,true);
	keyname.Format("%s%d",SYSTEM_INFO_LOG_LATEST_GAME,i+1);
	reg.Get_String(keyname, string);
}

static void Set_Latest_Game_String(RegistryClass& reg, int i, const StringClass& string)
{
	StringClass keyname(0,true);
	keyname.Format("%s%d",SYSTEM_INFO_LOG_LATEST_GAME,i+1);
	reg.Set_String(keyname, string);
}

void SystemInfoLog::Set_State_Loading()
{
	VeryMinFPS=1000;
	MinFPS=1000;
	VeryMaxFPS=0;
	MaxFPS=0;
	CurrentTotalFPS=0;
	CurrentFPSCount=0;
	CurrentLoadingTime=0;

	RegistryClass registry( COMBAT_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		registry.Set_Int( SYSTEM_INFO_LOG_CURRENT_STATE, 1 );
		StringClass string(0,true);
		for (int i=NUM_GAMES_LOGGED-1;i>0;--i) {
			Get_Latest_Game_String(registry,i-1,string);
			Set_Latest_Game_String(registry,i,string);
		}

		Get_Final_String(CurrentString);
		CurrentString+="Crashed while loading";

		Set_Latest_Game_String(registry, 0, CurrentString );
	}
}

void SystemInfoLog::Set_Current_Level(const char* level_name)
{
	CurrentLevel=level_name;
	RegistryClass registry( COMBAT_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		Get_Final_String(CurrentString);
		CurrentString+="Crashed while loading";
		Set_Latest_Game_String(registry,0,CurrentString );

	}
}

void SystemInfoLog::Set_State_Playing()
{
	RegistryClass registry( COMBAT_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		registry.Set_Int( SYSTEM_INFO_LOG_CURRENT_STATE, 2 );
		Get_Final_String(CurrentString);
		CurrentString+="Crashed while playing";
		Set_Latest_Game_String(registry,0,CurrentString );
	}
	PlayingStartTime=TIMEGETTIME();
}

void SystemInfoLog::Get_Final_String(StringClass& string)
{
	StringClass vnum(0,true);
	StringClass ptime(0,true);
	StringClass avgfps(0,true);
	vnum.Format("%d.%2.2d",DebugManager::Get_Version_Number()>>16,DebugManager::Get_Version_Number()&0xffff);
	ptime.Format("%2.2d:%2.2d",TotalPlayingTime/60,TotalPlayingTime%60);
	avgfps.Format("%d.%1.1d",AvgFPS/10,AvgFPS%10);
	string.Format(
		"%5s %16s %6s %6d %6d %6s %8d ",
		vnum,
		CurrentLevel,
		ptime,
		MinFPS!=1000 ? MinFPS : 0,
		MaxFPS,
		avgfps,
		CurrentLoadingTime);
}

void SystemInfoLog::Set_State_Exiting()
{
	TotalPlayingTime=(TIMEGETTIME()-PlayingStartTime)/1000;

	AvgFPS=0;
	if (CurrentFPSCount) {
		AvgFPS=CurrentTotalFPS*10/CurrentFPSCount;
	}

	RegistryClass registry( COMBAT_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		registry.Set_Int( SYSTEM_INFO_LOG_CURRENT_STATE, 3 );

		Get_Final_String(CurrentString);
		CurrentString+="Crashed while exiting.";
		Set_Latest_Game_String(registry,0,CurrentString);
	}
}

void SystemInfoLog::Reset_State()
{
	RegistryClass registry( COMBAT_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		int old_state=registry.Get_Int( SYSTEM_INFO_LOG_CURRENT_STATE);
		registry.Set_Int( SYSTEM_INFO_LOG_CURRENT_STATE, 0 );
		// If was exiting...
		if (old_state==3) {
			Get_Final_String(CurrentString);
			CurrentString+="OK";
			Set_Latest_Game_String(registry,0,CurrentString);
		}
	}
	CurrentLevel="";
}

void SystemInfoLog::Record_Loading_Time(unsigned loading_time)
{
	CurrentLoadingTime=loading_time;
}

void SystemInfoLog::Record_Frame()
{
	unsigned fps=TimeManager::Get_Average_Frame_Rate();
	CurrentTotalFPS+=fps;
	CurrentFPSCount++;
	if (fps<VeryMinFPS) {
		VeryMinFPS=fps;
	}
	else if (fps<MinFPS) {
		MinFPS=fps;
	}

	if (fps>VeryMaxFPS) {
		VeryMaxFPS=fps;
	}
	else if (fps>MaxFPS) {
		MaxFPS=fps;
	}
}

void SystemInfoLog::Get_Log(StringClass& string)
{
	RegistryClass registry( COMBAT_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		string.Format(
			"Ten latest levels played:\r\n"
			"%3s %5s %16s %6s %6s %6s %6s %8s %s\r\n",
			"#",
			"Build",
			"Level",
			"Length",
			"MinFPS",
			"MaxFPS",
			"AvgFPS",
			"LoadTime",
			"Status");

		StringClass tmp_string(0,true);

		for (int i=0;i<NUM_GAMES_LOGGED;++i) {
			Get_Latest_Game_String(registry,i,tmp_string);
			StringClass tmp_string2(0,true);
			tmp_string2.Format("%2d. ",i+1);
			string+=tmp_string2;
			string+=tmp_string;
			string+="\r\n";
		}
	}
}

void SystemInfoLog::Get_Compact_Log(StringClass& string)
{
	int fps=0;
	if (CurrentFPSCount) {
		fps=CurrentTotalFPS*10/CurrentFPSCount;
	}

	string.Format("%d.%d\t",fps/10,fps%10);
}


// This stuff should go to the playerinfolog...

static StringClass CurrentMapName;

void PlayerInfoLog::Set_Current_Map_Name(const char* map_name)
{
	CurrentMapName=map_name;
	StringClass tmp_name(0,true);
	if (SaveGameManager::Peek_Map_Name (CurrentMapName, tmp_name)) {
		CurrentMapName = tmp_name;
	}

}

#define ADD_HISTORY(n) work.Format n; tmp+=work;

void PlayerInfoLog::Append_To_Log(PlayerDataClass* data)
{
	if (data->Get_Game_Time()==0.0f) return;
#ifdef WWDEBUG
	StringClass tmp;
	StringClass work(0,true);
	ADD_HISTORY(("Map name: %s\r\n",CurrentMapName));
	ADD_HISTORY(("Game time: %2.2f\r\n",data->Get_Game_Time()));
	ADD_HISTORY(("Session time: %2.2f\r\n",data->Get_Session_Time()));
	ADD_HISTORY(("Enemies killed: %d\r\n",data->Get_Enemies_Killed()));
	ADD_HISTORY(("Allies killed: %d\r\n",data->Get_Allies_Killed()));
	ADD_HISTORY(("Shots fired: %d\r\n",data->Get_Shots_Fired()));
	ADD_HISTORY(("Powerups collected: %d\r\n",data->Get_Powerups_Collected()));
	ADD_HISTORY(("Vehicles destroyed: %d\r\n",data->Get_Vehiclies_Destroyed()));
	ADD_HISTORY(("Buildings destroyed: %d\r\n",data->Get_Building_Destroyed()));
	ADD_HISTORY(("Time in vehicles: %d\r\n",data->Get_Vehicle_Time()));
	ADD_HISTORY(("Kills from vehicles: %d\r\n",data->Get_Kills_From_Vehicle()));
	ADD_HISTORY(("Squishes: %d\r\n",data->Get_Squishes()));
	ADD_HISTORY(("Credits granted: %d\r\n",data->Get_Credit_Grant()));
	ADD_HISTORY(("Times reloaded: %d\r\n",CombatManager::Get_Reload_Count()));
	ADD_HISTORY(("Cheats used: %s\r\n",CheatMgrClass::Get_Instance()->Was_Cheat_Used( CheatMgrClass::ALL_CHEATS ) ? "Yes" : "No"));
	ADD_HISTORY(("Secondary objectives completed: %d\r\n",ObjectiveManager::Get_Num_Completed_Objectives( ObjectiveManager::TYPE_SECONDARY )));
	ADD_HISTORY(("Tertiary objectives completed: %d\r\n",ObjectiveManager::Get_Num_Completed_Objectives( ObjectiveManager::TYPE_TERTIARY )));

	tmp+="\r\n";

	DWORD written;
	HANDLE file = CreateFileA("history.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != file) {
		SetFilePointer(file, 0, NULL, FILE_END);
		WriteFile(file, tmp, strlen(tmp), &written, NULL);
		CloseHandle(file);
	}
#endif // WWDEBUG
}

void PlayerInfoLog::Get_Compact_Log(StringClass& string)
{
	int team0size=0;
	int team1size=0;
	if (Team0SizeReported) {
		team0size=10*Team0TotalSizes/Team0SizeReported;
	}
	if (Team1SizeReported) {
		team1size=10*Team1TotalSizes/Team1SizeReported;
	}

	string.Format("%s\t%d.%d\t%d.%d\t",CurrentMapName,team0size/10,team0size%10,team1size/10,team1size%10);

	Team0SizeReported=0;
	Team1SizeReported=0;
	Team0TotalSizes=0;
	Team1TotalSizes=0;
}

void PlayerInfoLog::Report_Tally_Size(int type, int size)
{
	if (type==0) {
		Team0TotalSizes+=size;
		Team0SizeReported++;
	}
	else if (type==1) {
		Team1TotalSizes+=size;
		Team1SizeReported++;
	}
}