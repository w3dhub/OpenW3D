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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/gamedata.h                               $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/01/02 4:32p                                              $*
 *                                                                                             *
 *                    $Revision:: 135                                                        $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	GAMEDATA_H
#define	GAMEDATA_H

#if defined(_MSV_VER)
#pragma once
#endif

#include "teammanager.h"
#include "bittype.h"
#include "boolean.h"
#include "netinterface.h"
#include "wwstring.h"
#include "widestring.h"
#include "radar.h"
#include "DlgMPTeamSelect.h"
#include <wwlib/Signaler.h>

const int		MAX_PASSWORD_SIZE			= 16;				//including NULL
const int		MAX_MAPNAME_SIZE			= 28;				//including NULL
const LPCSTR	SERVER_CONFIG_PREFIX		= "svrcfg_";
const int		MAX_MOTD_LENGTH			= 100;
#define MAX_CLAN_SLOTS 2

class SoldierGameObj;
class cPacket;
//class Render2DTextClass;
class Render2DSentenceClass;
class Font3DInstanceClass;
//class cGameDataDeathMatch;
//class cGameDataTeamDeathMatch;
class cGameDataSinglePlayer;
class cGameDataSkirmish;
class cGameDataCnc;
class WOLGameInfo;

//
// This class describes a server configuration. Dynamic instantiations
// of related parameters are stored elsewhere.
//
class	cGameData :
		public Signaler<MPChooseTeamSignal>
{
	public:

		enum GameTypeEnum {
			GAME_TYPE_SINGLE_PLAYER = 0,
			GAME_TYPE_SKIRMISH,
			GAME_TYPE_CNC,
		};

		enum WinTypeEnum {
			WIN_TYPE_FORCED = 0,				// gameover command, or server quits
			WIN_TYPE_COWARDICE,				// opponents abandon game
			WIN_TYPE_TIME,						// time expired
			WIN_TYPE_BASE_DESTRUCTION,		// base was destroyed
			WIN_TYPE_BEACON,					// beacon placed successfully
		};

		cGameData(void);
		virtual ~cGameData(void);
		cGameData& operator=(const cGameData& rhs);

		static void		Onetime_Init(void);
		static void		Onetime_Shutdown(void);

		virtual void	On_Game_Begin(void);
		virtual void	On_Game_End(void);

		virtual void	Think(void);
		virtual void	Render(void);
		virtual void	Reset_Game(bool is_reloaded);

		void				Swap_Team_Sides(void);
		void				Remix_Team_Sides(void);
		void				Rebalance_Team_Sides(void);

		void				Add_Bottom_Text(WideStringClass & text);
		void				Get_Time_Limit_Text(WideStringClass& text);
		void				Get_Dedicated_Server_Label(WideStringClass& text,unsigned& color);
		void				Get_Gameplay_Not_Permitted_Label(WideStringClass& text, unsigned& color);

		static cGameData * Create_Game_Of_Type(GameTypeEnum game_type);

		void				Set_Game_Title(const unichar_t *title)		{ GameTitle = title; }
		void				Set_Motd(const unichar_t * motd);
		void				Set_Password(const unichar_t *password)		{ Password = password;}
		void				Set_Map_Name(const StringClass & map_name);
		void				Set_Mod_Name(const StringClass & mod_name);
		void				Set_Map_Cycle(int map, const StringClass & map_name);
		void				Set_Owner(const WideStringClass & owner);
		bool				Set_Max_Players(int max_players);
		bool				Set_Time_Limit_Minutes(int time_limit_minutes);
		void				Set_Radar_Mode(RadarModeEnum mode);
		void				Set_Intermission_Time_Seconds(int time);
		void				Set_Version_Number(int version_number) {VersionNumber = version_number;}
		bool				Set_Current_Players(int current_players);
		void				Set_Ip_Address(unsigned int ip_address);
		void				Set_Port(int port);

		void				Set_QuickMatch_Server(bool isServer)	{IsQuickMatchServer.Set(isServer);}
		bool				Is_QuickMatch_Server(void) const			{return IsQuickMatchServer.Get();}

		void Set_Clan(int slot, unsigned int clanID);
		unsigned int Get_Clan(int slot) const;

		void Clear_Clans(void);
		int Find_Free_Clan_Slot(void) const;

		bool Is_Clan_Competing(unsigned int clanID) const;
		bool Is_Clan_Game_Open(void) const;

		const unichar_t *	Get_Game_Title(void)					const	{return GameTitle;}
		const unichar_t *	Get_Motd(void)							const	{return Motd;}
		const unichar_t *	Get_Password(void)					const	{return Password;}
		const StringClass& Get_Mod_Name()					const	{return ModName;}
		const StringClass& Get_Map_Name()					const	{return MapName;}
		const StringClass& Get_Map_Cycle(int map);
		bool				Does_Map_Exist (void);
		const WideStringClass & Get_Owner(void)			const {return Owner;}
		int				Get_Max_Players(void)				const	{return MaxPlayers;}
		int				Get_Time_Limit_Minutes(void)		const {return TimeLimitMinutes;}
		int				Get_Intermission_Time_Seconds(void) const {return IntermissionTimeSeconds;}
		int				Get_Version_Number(void)			const {return VersionNumber;}
		int				Get_Current_Players(void)			const	{return CurrentPlayers;}
		unsigned int				Get_Ip_Address(void)					const	{return IpAddress;}
		int				Get_Port(void)							const	{return Port;}
		RadarModeEnum	Get_Radar_Mode(void)					const	{return RadarMode;}

		virtual const unichar_t* Get_Game_Name(void)			const	= 0;
		virtual GameTypeEnum	Get_Game_Type(void)			const	= 0;
		const char *	Get_Game_Type_Name(void) const;
		virtual int		Choose_Player_Type(cPlayer* player, int team_choice, bool is_grunt);

		bool				Is_Time_Limit(void)					const	{return TimeLimitMinutes > 0;}

		void				Rotate_Map(void);
		void				Clear_Map_Cycle(void);

		void				Set_Ip_And_Port(void);
		virtual void	Load_From_Server_Config(void)		{}
		virtual void	Save_To_Server_Config(void)		{}

		virtual bool	Is_Editable_Teaming(void)			const	{return false;}
		virtual bool	Is_Editable_Clan_Game(void)		const	{return false;}
		virtual bool	Is_Editable_Friendly_Fire(void)	const	{return false;}

		virtual void	Soldier_Added(SoldierGameObj * p_soldier);
		virtual void	Show_Game_Settings_Limits(void);

		//
		//	Version information
		//
		bool	Do_Exe_Versions_Match(void)		{ return DoExeVersionsMatch; }
		bool	Do_String_Versions_Match(void)	{ return DoStringVersionsMatch; }

		//
		//	Map cycle looping support
		//
		bool	Is_Map_Cycle_Over (void)				{ return IsMapCycleOver; }
		void	Set_Is_Map_Cycle_Over (bool onoff)	{ IsMapCycleOver = onoff; }
		bool	Do_Maps_Loop (void)						{ return DoMapsLoop; }
		void	Set_Do_Maps_Loop (bool onoff)			{ DoMapsLoop = onoff; }
		bool	Is_Map_Valid(char **filename = NULL);


		//
		// Ideally, none of these would exist, so comment out any unused ones...
		//
		virtual bool	Is_Single_Player(void)			const	{return false;}
		virtual bool	Is_Skirmish(void)					const	{return false;}
		virtual bool	Is_Cnc(void)						const	{return false;}

		virtual cGameDataSinglePlayer *		As_Single_Player(void)		{return NULL;}
		virtual cGameDataSkirmish *			As_Skirmish(void)				{return NULL;}
		virtual cGameDataCnc *					As_Cnc(void)					{return NULL;}

		virtual int		Get_Min_Players(void) const {return 2;}
		int				Choose_Available_Team(int preference);
		int				Choose_Smallest_Team(void);
		virtual bool	Is_Game_Over(void);
		void				Game_Over_Processing(void);
		bool				Set_Generic_Num(int generic_num,
								int lower_bound, int upper_bound, int & set_num);
		virtual bool	Is_Limited(void) const;
		virtual bool	Is_Valid_Settings(WideStringClass& outMsg, bool check_as_server = false);
		virtual void	Export_Tier_1_Data(cPacket & packet);
		virtual void	Import_Tier_1_Data(cPacket & packet);
		virtual void	Import_Tier_1_Data(const WOLGameInfo& gameInfo);
		virtual void	Export_Tier_2_Data(cPacket & packet);
		virtual void	Import_Tier_2_Data(cPacket & packet);

		//
		// Dynamic data
		//
		void				Begin_Intermission(void);
		float				Get_Intermission_Time_Remaining(void) {return IntermissionTimeRemaining;}
		void				Set_Intermission_Time_Remaining(float time) {IntermissionTimeRemaining = time;}
		float				Get_Maximum_World_Distance(void) {return MaximumWorldDistance;}
		void				Set_Maximum_World_Distance(float distance);
		unsigned int	Get_Frame_Count(void) const {return FrameCount;}
		LPSYSTEMTIME	Get_Game_Start_Time(void) {return &GameStartTime;}
		int				Get_Duration_Seconds(void);
		void				Set_Min_Qualifying_Time_Minutes(int minutes);
		int				Get_Min_Qualifying_Time_Minutes(void)			{return MinQualifyingTimeMinutes;}
		WideStringClass	Get_Team_Word(void);
		void				Set_Time_Remaining_Seconds(float time_remaining_seconds);
		void				Reset_Time_Remaining_Seconds(void);
		float				Get_Time_Remaining_Seconds(void);
		bool				Is_Valid_Player_Type(int player_type);
		void				Set_Ini_Filename(const StringClass& name) {IniFilename = name;}
		const StringClass& Get_Ini_Filename() { return IniFilename;}
		virtual bool	Is_Gameplay_Permitted(void);

		virtual	bool	Remember_Inventory( void )	const					{ return false; }

		void				Set_Winner_ID(int id)								{mWinnerID = id;}
		int				Get_Winner_ID(void) const							{return mWinnerID;}

		static void		Set_Hosted_Game_Number(int number) 				{HostedGameNumber = number;}
		static void		Increment_Hosted_Game_Number(void) 				{HostedGameNumber++;}
		static int		Get_Hosted_Game_Number(void) 						{return HostedGameNumber;}

		/*
		void				Set_Server_Is_Gameplay_Permitted(bool flag)	{ServerIsGameplayPermitted = flag;}
		bool				Get_Server_Is_Gameplay_Permitted(void)			{return ServerIsGameplayPermitted;}
		*/

		static	bool	Is_Manual_Restart(void)								{return IsManualRestart;}
		static	void	Set_Manual_Restart(bool is_manual_restart) 	{IsManualRestart = is_manual_restart;}

		static	bool	Is_Manual_Exit(void)									{return IsManualExit;}
		static	void	Set_Manual_Exit(bool is_manual_exit) 			{IsManualExit = is_manual_exit;}


		static	void					Set_Win_Text(WideStringClass & text);
		static	WideStringClass	Get_Win_Text(void)					{return WinText;}

		void				Filter_Spawners(void);

		void				Set_Mvp_Name(WideStringClass name);
		const WideStringClass & Get_Mvp_Name(void) const				{return MvpName;}

		void				Set_Mvp_Count(int count);
		void				Increment_Mvp_Count(void)							{MvpCount++;}
		const int		Get_Mvp_Count(void) const							{return MvpCount;}

		void				Set_Win_Type(WinTypeEnum type);
		WinTypeEnum		Get_Win_Type(void) const							{return WinType;}

		void				Set_Game_Duration_S(unsigned int seconds);
		unsigned int				Get_Game_Duration_S(void) const					{return GameDurationS;}

		virtual	void	Get_Description(WideStringClass & description);


		WideStringClass	Get_Settings_Description(void)						{return(SettingsDescription.Peek_Buffer());}
		void					Set_Settings_Description(WideStringClass desc)	{SettingsDescription = desc;}

		cBoolean			IsIntermission;
		cBoolean			IsDedicated;
		cBoolean			IsAutoRestart;
		cBoolean 		IsFriendlyFirePermitted;
		cBoolean 		IsTeamChangingAllowed;
		cBoolean 		IsPassworded;
		cBoolean 		IsFreeWeapons;
		cBoolean 		IsLaddered;
		cBoolean 		IsClanGame;
		cBoolean 		IsClientTrusted;
		cBoolean 		RemixTeams;
		cBoolean 		CanRepairBuildings;
		cBoolean 		DriverIsAlwaysGunner;
		cBoolean 		SpawnWeapons;


		enum {MAX_MAPS = 100};

		static	int		Get_Mission_Number_From_Map_Name( const char * map_name );
		void					Load_From_Server_Config(LPCSTR config_file);
		void					Save_To_Server_Config(LPCSTR config_file);

	protected:

		void ReceiveSignal(MPChooseTeamSignal&) override;

		static const StringClass	INI_SECTION_NAME;
		static const float			LIMITS_X_POS;

		bool					DoMapsLoop;
		bool					IsMapCycleOver;
		int					MapCycleIndex;

	private:
		cGameData(const cGameData& rhs);					// Disallow copy
		bool Has_Config_File_Changed(void);
		unsigned int Get_Config_File_Mod_Time(void);

		static const int	MAX_TIME_LIMIT;

#if(0)
		static Render2DTextClass	*	PTextRenderer;
		static Font3DInstanceClass *	PFont;
#endif
		static Render2DSentenceClass	*	PTextRenderer;

		static int			HostedGameNumber; //temp hack 09/26/01

		static	bool					IsManualRestart;
		static	bool					IsManualExit;
		static	WideStringClass	WinText;

		WideStringClass	GameTitle;
		WideStringClass	Motd;
		WideStringClass	Password;
		StringClass			MapName;
		StringClass			ModName;
		StringClass			MapCycle[MAX_MAPS];
		WideStringClass	Owner;
		DynamicVectorClass<WideStringClass> BottomText;
		DynamicVectorClass<WideStringClass> OldBottomText;
		StringClass			SettingsDescription;

		int					MaxPlayers;
		int					TimeLimitMinutes;
		int					IntermissionTimeSeconds;
		int					VersionNumber; // for now use exe byte size
		bool					DoExeVersionsMatch;
		bool					DoStringVersionsMatch;
		unsigned int					IpAddress; // hosting address
		int					Port;
		StringClass			IniFilename;
		RadarModeEnum		RadarMode;
		unsigned int		LastServerConfigModTime;

		//
		// Dynamic data about game state
		//
		int					CurrentPlayers;
		float					IntermissionTimeRemaining;
		float					TimeRemainingSeconds;
		float					MaximumWorldDistance;
		int					MinQualifyingTimeMinutes;

		int					mWinnerID;
		WinTypeEnum			WinType;

		SYSTEMTIME			GameStartTime;
		unsigned int					GameStartTimeMs;
		unsigned int		FrameCount;
		WideStringClass	MvpName;
		int					MvpCount;
		unsigned int					GameDurationS;

		cBoolean				IsQuickMatchServer;
		//bool					ServerIsGameplayPermitted;

		unsigned int mClanSlots[MAX_CLAN_SLOTS];
};

extern cGameData *						PTheGameData;
extern cGameData *						The_Game(void);

extern cGameDataSinglePlayer *		The_Single_Player_Game(void);
extern cGameDataSkirmish *				The_Skirmish_Game(void);
extern cGameDataCnc *					The_Cnc_Game(void);

#endif	// GAMEDATA_H


















//extern cGameDataDeathMatch *			The_Deathmatch_Game(void);
//extern cGameDataTeamDeathMatch *		The_Team_Deathmatch_Game(void);

			//GAME_TYPE_MULTIPLAY_FIRST,
			//GAME_TYPE_DEATHMATCH = GAME_TYPE_MULTIPLAY_FIRST,
			//GAME_TYPE_TEAM_DEATHMATCH,
			//GAME_TYPE_CNC = GAME_TYPE_MULTIPLAY_FIRST,
			//GAME_TYPE_MULTIPLAY_LAST = GAME_TYPE_CNC

		//virtual bool	Is_Deathmatch(void)				const	{return false;}
		//virtual bool	Is_Team_Deathmatch(void)		const	{return false;}
		//virtual cGameDataDeathMatch *			As_Deathmatch(void)			{return NULL;}
		//virtual cGameDataTeamDeathMatch *	As_Team_Deathmatch(void)	{return NULL;}
		//static const char* Get_Game_Type_Name(GameTypeEnum type);
		//virtual bool	Is_Editable_Reload_Map(void)		const	{return false;}
		//virtual bool	Is_Editable_Max_Players(void)		const	{return false;}
		//virtual bool	Is_Team_Game(void) const {return false;}
		//virtual bool	Is_Respawn_On_Demand(void) const					{return true;}
		//void				Set_Min_Game_Time_Required_Mins(int mins);
		//int				Get_Min_Game_Time_Required_Mins(void) {return MinGameTimeRequiredMins;}
		//void				Set_Full_Score_Time_Threshold_Mins(int mins);
		//int				Get_Full_Score_Time_Threshold_Mins(void) {return FullScoreTimeThresholdMins;}
		//int					MinGameTimeRequiredMins;
		//int					FullScoreTimeThresholdMins;
		//cBoolean			IsBloodShed;