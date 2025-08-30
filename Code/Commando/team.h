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
 *                     $Archive:: /Commando/Code/Commando/team.h                          $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 11/13/01 7:54p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 35                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSV_VER)
#pragma once
#endif

#ifndef TEAM_H
#define TEAM_H

#include "vector3.h"
#include "bittype.h"
#include "soldier.h"
#include "widestring.h"

const int MAX_TEAMNAME_SIZE = 10; //including NULL

class cPacket;

//------------------------------------------------------------------------------------
//
// Holds data about a team
//
class cTeam : public NetworkObjectClass {

   public:
      friend class cTeamManager; // so that only cTeamManager can call ~cTeam

		cTeam(void);

		virtual uint32		Get_Network_Class_ID(void) const override					{return NETCLASSID_TEAM;}
		virtual void		Delete(void) override											{delete this;}

		//
      // Server only
      //
      void					Increment_Kills(void);
      void					Increment_Deaths(void);
      void					Increment_Score(float increment);
      //void					Increment_Money(int increment);

      //
      // Client only
      //
      void					Set_Kills(int new_kills);
      void					Set_Deaths(int new_deaths);
      void					Set_Score(float new_score);
      //void					Set_Money(int new_money);

      //
      // Client or server
      //
	   void					Init(int team_number);
		void					Init_Team_Name(void);
      WideStringClass	Get_Name(void) const									{return Name;}
      int					Get_Id(void) const									{return TeamNumber;}
      void					Reset(void);
      int					Tally_Size(void) const;
      int					Get_Kills(void) const								{return Kills;}
      int					Get_Deaths(void) const								{return Deaths;}
      float					Get_Score(void) const								{return Score;}
      //int					Get_Money(void) const								{return Money;}
      float					Get_Kill_To_Death_Ratio(void) const;
      void					Get_Team_String(int rank, WideStringClass & string) const;
		Vector3				Get_Color(void) const;

      int					Tally_Money(void) const;

		//
		//	Server-to-client data importing/exporting
		//
		virtual void		Import_Creation(BitStreamClass &packet) override;
		virtual void		Import_Rare(BitStreamClass &packet) override;
		virtual void		Import_Occasional(BitStreamClass &packet) override;
		virtual void		Import_Frequent(BitStreamClass &packet) override;

		virtual void		Export_Creation(BitStreamClass &packet) override;
		virtual void		Export_Rare(BitStreamClass &packet) override;
		virtual void		Export_Occasional(BitStreamClass &packet) override;
		virtual void		Export_Frequent(BitStreamClass &packet) override;

   private:
	   ~cTeam(void); // only the cTeamManager can destroy...

      WideStringClass	Name;
      int					Kills;
      int					Deaths;
      float					Score;
      int					Money;
      int					TeamNumber;
};


#endif	//	TEAM_H














