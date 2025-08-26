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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Combat/CNCModeSettings.h $
*
* DESCRIPTION
*     Settings for CNC Multiplayer mode.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Patrick $
*
* VERSION INFO
*     $Revision: 5 $
*     $Modtime: 11/28/01 2:22p $
*
******************************************************************************/

#ifndef	__CNCMODESETTINGS_H__
#define	__CNCMODESETTINGS_H__

#include "always.h"
#include "definition.h"

class CNCModeSettingsDef :
		public DefinitionClass
	{
	public:
		enum {NUM_TEAMS = 2};

		CNCModeSettingsDef(void);
		virtual ~CNCModeSettingsDef(void);

		virtual uint32 Get_Class_ID(void) const;

		virtual PersistClass* Create(void) const;
		
		virtual bool Save(ChunkSaveClass& csave);
		virtual bool Load(ChunkLoadClass& cload);

		virtual const PersistFactoryClass& Get_Factory(void) const;	

		static CNCModeSettingsDef* Get_Instance(void)
			{return _mInstance;}	

		int Get_Announcement_Interval(void) const
			{return AnnouncementInterval;}

		int Get_Power_Offline_Report(int team) const;
		int Get_Purchase_Canceled_Report(int team) const;
		int Get_Insufficient_Funds_Report(int team) const;
		int Get_Constructing_Report(int team) const;
		int Get_Unit_Ready_Report(int team) const;

		int Get_Ion_Beacon_Deployed_Report(int team) const;
		int Get_Ion_Beacon_Disarmed_Report(int team) const;
		int Get_Ion_Beacon_Warning_Report(int team) const;

		int Get_Nuke_Beacon_Deployed_Report(int team) const;
		int Get_Nuke_Beacon_Disarmed_Report(int team) const;
		int Get_Nuke_Beacon_Warning_Report(int team) const;

		int Get_Radio_Command(int num) const;
		const char *Get_Radio_Command_Emot_Icon(int num) const;

		// Editable support
		DECLARE_EDITABLE(CNCModeSettingsDef, DefinitionClass);

	protected:
		static CNCModeSettingsDef* _mInstance;

		int AnnouncementInterval;

		int mPowerOfflineID[NUM_TEAMS];
		int mPurchaseCanceledID[NUM_TEAMS];
		int mInsufficientFundsID[NUM_TEAMS];
		int mConstructingID[NUM_TEAMS];
		int mUnitReadyID[NUM_TEAMS];

		int mIonBeaconDeployedID[NUM_TEAMS];
		int mIonBeaconDisarmedID[NUM_TEAMS];
		int mIonBeaconWarningID[NUM_TEAMS];

		int mNukeBeaconDeployedID[NUM_TEAMS];
		int mNukeBeaconDisarmedID[NUM_TEAMS];
		int mNukeBeaconWarningID[NUM_TEAMS];

		int			mRadioCmds[30];
		StringClass	mRadioCmdIcons[30];
		};

#endif // __CNCMODESETTINGS_H__
