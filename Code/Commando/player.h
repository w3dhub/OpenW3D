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
 *                     $Archive:: /Commando/Code/Commando/player.h                        $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/08/02 6:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 87                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSV_VER)
#pragma once
#endif

#ifndef PLAYER_H
#define PLAYER_H

#include "always.h"
#include "playerdata.h"
#include "bittype.h"
#include "boolean.h"
#include "netinterface.h"
#include "networkobject.h"
#include "netclassids.h"
#include "wwstring.h"
#include "gamespyauthmgr.h"
#include "GameSpyBanList.h"

class Vector3;
class ChunkSaveClass;
class ChunkLoadClass;

//------------------------------------------------------------------------------------
//
// Holds data about a player
//
class cPlayer : public PlayerDataClass, public NetworkObjectClass {

   public:
      friend class cPlayerManager; // so that only cPlayerManager can call ~cPlayer

		virtual uint32		Get_Network_Class_ID(void) const override		{return NETCLASSID_PLAYER;}
		virtual void		Delete(void) override;

		void Init(void);
		void Mark_As_Modified(void);

      //
      // Client or server
      //
	   cPlayer(void);

		bool Save(ChunkSaveClass & csave);
		bool Load(ChunkLoadClass & cload);

		int Get_Id(void) const {return Id;}
		void Set_Id(int id);

		void Set_Name(const WideStringClass & name);
      const WideStringClass & Get_Name(void) const {return Name;}

		virtual void Set_Score(float score) override;
		virtual void Increment_Score(float add) override;

		virtual void Set_Money(float money) override;
		virtual void Increment_Money(float add) override;

		int Get_Ladder_Points(void) const {return (int)LadderPoints;}
      void Set_Ladder_Points(int points);

      int Get_Kills(void) const {return (int)Kills;}
      void Increment_Kills(void);
      void Set_Kills(int new_kills);

      int Get_Deaths(void) const {return (int)Deaths;}
      void Increment_Deaths(void);
      void Set_Deaths(int new_deaths);

      int Get_Player_Type(void) const {return (int)PlayerType;}
      void Set_Player_Type(int type);
		bool Is_Team_Player(void);

      Vector3 Get_Color(void) const;
      float Get_Kill_To_Death_Ratio(void) const;
      void Get_Player_String(int rank, WideStringClass & string, bool force_verbose = false);

			unsigned int Get_WOL_ClanID(void) const {return ClanID;}
			void Set_WOL_ClanID(unsigned int id) {ClanID = id;}

      int Get_Wol_Rank(void) const {return (int)WolRank;}
      void Set_Wol_Rank(int wol_rank);

			unsigned short Get_WOL_Points(void) const
				{return WOLPoints;}

      void Set_WOL_Points(unsigned short points)
				{WOLPoints = points;}

      int Get_Rung(void) const {return (int)Rung;}
      void Set_Rung(int rung);

      int Get_Damage_Scale_Factor(void) const {return DamageScaleFactor;}
      void Set_Damage_Scale_Factor(int factor);

      void Set_Ping(int ping)							{Ping = ping;}
      //int Get_Ping(void)								{return Ping;}
      int Get_Ping(void);

      DWORD Get_Join_Time(void) const {return JoinTimeMs;}
      void Reset_Join_Time(void);

      DWORD Get_Total_Time(void) const {return TotalTimeMs;}
      void Increment_Total_Time(void);
      void Reset_Total_Time(void);

		ULONG Get_Ip_Address(void) const {return IpAddress;}
      void Set_Ip_Address(ULONG ip_address);

		int Get_Fps(void) const {return Fps;}
      void Set_Fps(int fps);

      //int Get_Avg_Ping(void) const;

		DWORD Get_Last_Update_Time_Ms(void) const {return LastUpdateTimeMs;}
      void Set_Last_Update_Time_Ms(DWORD time_ms);

		bool Is_Human(void) const {return Id >= 0;}

      virtual	void Reset_Player(void) override;

		void			Set_Is_In_Game(bool state);
		cBoolean &	Get_Is_In_Game(void) {return IsInGame;}

		void			Set_Is_Waiting_For_Intermission(bool state);
		cBoolean &	Get_Is_Waiting_For_Intermission(void) {return IsWaitingForIntermission;}

		void			Set_Is_Active(bool flag);
		cBoolean &	Get_Is_Active(void) {return IsActive;}
		bool	Is_Active(void) {return IsActive.Is_True();}

		bool			Is_Alive_And_Kicking(void) const;

		//
		// Fast, in-game sorting support
		//
		void					Set_Fast_Sort_Key(int key)					{ FastSortKey = key; }
		int					Get_Fast_Sort_Key(void)						{ return FastSortKey; }

		//
		//	Server-to-client data importing/exporting
		//
		virtual void		Import_Creation(BitStreamClass &packet) override;
		virtual void		Import_Rare(BitStreamClass &packet) override;
		virtual void		Import_Occasional(BitStreamClass &packet) override;

		virtual void		Export_Creation(BitStreamClass &packet) override;
		virtual void		Export_Rare(BitStreamClass &packet) override;
		virtual void		Export_Occasional(BitStreamClass &packet) override;

		enum					{NUM_NEWBIE_GAMES = 5};
		void					Set_Num_Wol_Games(int num_games);
		int					Get_Num_Wol_Games(void)						{ return NumWolGames; }

		//
		// GameSpy support
		//
		void								Set_GameSpy_Auth_State(GAMESPY_AUTH_STATE_ENUM state);
		GAMESPY_AUTH_STATE_ENUM		Get_GameSpy_Auth_State(void)	const	{return GameSpyAuthState;}
		void								Set_GameSpy_Auth_State_Entry_Time_Ms(DWORD time_ms);
		DWORD								Get_GameSpy_Auth_State_Entry_Time_Ms(void) const {return GameSpyAuthStateEntryTimeMs;}
		void								Set_GameSpy_Challenge_String(StringClass & challenge_string);
		void								Set_GameSpy_Hash_Id(StringClass & hash_id);
		StringClass &					Get_GameSpy_Challenge_String(void) {return GameSpyChallengeString;}
		StringClass &					Get_GameSpy_Hash_Id(void) {return GameSpyHashId;}
		void								Set_GameSpy_Kick_State(GAMESPY_KICK_STATE_ENUM state);
		GAMESPY_KICK_STATE_ENUM				Get_GameSpy_Kick_State(void)	const	{return GameSpyKickState;}
		void								Set_GameSpy_Kick_State_Entry_Time_Ms(DWORD time_ms);
		DWORD								Get_GameSpy_Kick_State_Entry_Time_Ms(void) const {return GameSpyKickStateEntryTimeMs;}

		//
		// N.B. If you change the state of any of these on the server then you
		//      need to call Mark_As_Modified.
		//
		cBoolean				Invulnerable;

      static const int	INVALID_ID;

   private:
	   ~cPlayer(void); // only the cPlayerManager can destroy...
		void On_Create(void);
		void On_Destroy(void);

      WideStringClass	Name;
      int					Id;
		safe_int				LadderPoints;
      safe_int				Kills;
      safe_int				Deaths;
      safe_int				PlayerType;
		safe_int				Rung;
		safe_int				WolRank;

		unsigned short WOLPoints;

		int					DamageScaleFactor;
		int					Ping;
		DWORD					JoinTimeMs;
		DWORD					TotalTimeMs;
		ULONG					IpAddress;
		int					Fps;
		DWORD					LastUpdateTimeMs;
		int					FastSortKey;
		int					NumWolGames;
		cBoolean				IsWaitingForIntermission;

		//
		// GameSpy support
		//
		GAMESPY_AUTH_STATE_ENUM		GameSpyAuthState;
		DWORD								GameSpyAuthStateEntryTimeMs;
		StringClass						GameSpyChallengeString;
		StringClass						GameSpyHashId;
		GAMESPY_KICK_STATE_ENUM		GameSpyKickState;
		DWORD						GameSpyKickStateEntryTimeMs;

		//
		// N.B. If you change the state of any of these on the server then you
		//      need to call Mark_As_Modified.
		//
		cBoolean				IsInGame;
		cBoolean				IsActive;

		unsigned int ClanID;
};

//-----------------------------------------------------------------------------

#endif	//	PLAYER_H













		//virtual void	On_Kill(int victim_id);
		//virtual void On_Kill(int victim_id, int victim_team);
