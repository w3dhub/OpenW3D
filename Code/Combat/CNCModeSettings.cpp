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

#include "CNCModeSettings.h"
#include "combatchunkid.h"
#include "persistfactory.h"
#include "definitionfactory.h"
#include "simpledefinitionfactory.h"
#include "playertype.h"
#include "wwhack.h"
#include "debug.h"

DECLARE_FORCE_LINK(CNCModeSettings)

CNCModeSettingsDef* CNCModeSettingsDef::_mInstance = NULL;

// Factories
SimplePersistFactoryClass<CNCModeSettingsDef, CHUNKID_GLOBAL_SETTINGS_DEF_CNCMODE> _CNCModeSettingsDefPersistFactory;
DECLARE_DEFINITION_FACTORY(CNCModeSettingsDef, CLASSID_GLOBAL_SETTINGS_DEF_CNCMODE, "C&C Mode Settings") _CNCModeSettingsDefDefFactory;


CNCModeSettingsDef::CNCModeSettingsDef(void) :
		AnnouncementInterval(30)
	{
	//WWASSERT(_mInstance == NULL);
	_mInstance = this;

	for (int team = 0; team < NUM_TEAMS; ++team)
		{
		mPowerOfflineID[team] = 0;
		mPurchaseCanceledID[team] = 0;
		mInsufficientFundsID[team] = 0;
		mConstructingID[team] = 0;
		mUnitReadyID[team] = 0;
		mIonBeaconDeployedID[team] = 0;
		mIonBeaconDisarmedID[team] = 0;
		mIonBeaconWarningID[team] = 0;
		mNukeBeaconDeployedID[team] = 0;
		mNukeBeaconDisarmedID[team] = 0;
		mNukeBeaconWarningID[team] = 0;
		}

	memset(mRadioCmds, 0, sizeof(mRadioCmds));

	EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_INT, AnnouncementInterval);
	
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mPowerOfflineID[0], "Nod power offline");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mPowerOfflineID[1], "GDI power offline");
	
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mPurchaseCanceledID[0], "Nod purchase canceled");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mPurchaseCanceledID[1], "GDI purchase canceled");
	
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mInsufficientFundsID[0], "Nod inssufficient funds");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mInsufficientFundsID[1], "GDI inssufficient funds");

	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mUnitReadyID[0], "Nod unit ready");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mUnitReadyID[1], "GDI unit ready");

	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mIonBeaconDeployedID[0], "Nod Ion Cannon Deployed");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mIonBeaconDeployedID[1], "GDI Ion Cannon Deployed");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mIonBeaconDisarmedID[0], "Nod Ion Cannon Disarmed");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mIonBeaconDisarmedID[1], "GDI Ion Cannon Disarmed");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mIonBeaconWarningID[0], "Nod Ion Cannon Warning");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mIonBeaconWarningID[1], "GDI Ion Cannon Disarmed");

	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mNukeBeaconDeployedID[0], "Nod Nuke Deployed");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mNukeBeaconDeployedID[1], "GDI Nuke Deployed");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mNukeBeaconDisarmedID[0], "Nod Nuke Disarmed");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mNukeBeaconDisarmedID[1], "GDI Nuke Disarmed");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mNukeBeaconWarningID[0], "Nod Nuke Warning");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mNukeBeaconWarningID[1], "GDI Nuke Warning");

	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[0], "Radio Command 1  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[1], "Radio Command 2  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[2], "Radio Command 3  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[3], "Radio Command 4  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[4], "Radio Command 5  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[5], "Radio Command 6  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[6], "Radio Command 7  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[7], "Radio Command 8  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[8], "Radio Command 9  (CTRL");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[9], "Radio Command 10 (CTRL)");

	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[10], "Radio Command 11 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[11], "Radio Command 12 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[12], "Radio Command 13 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[13], "Radio Command 14 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[14], "Radio Command 15 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[15], "Radio Command 16 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[16], "Radio Command 17 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[17], "Radio Command 18 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[18], "Radio Command 19 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[19], "Radio Command 20 (ALT)");

	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[20], "Radio Command 21 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[21], "Radio Command 22 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[22], "Radio Command 23 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[23], "Radio Command 24 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[24], "Radio Command 25 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[25], "Radio Command 26 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[26], "Radio Command 27 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[27], "Radio Command 28 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[28], "Radio Command 29 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_STRINGSDB_ID, mRadioCmds[29], "Radio Command 30 (CTRL + ALT)");

	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[0], "Radio EmotIcon 1  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[1], "Radio EmotIcon 2  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[2], "Radio EmotIcon 3  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[3], "Radio EmotIcon 4  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[4], "Radio EmotIcon 5  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[5], "Radio EmotIcon 6  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[6], "Radio EmotIcon 7  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[7], "Radio EmotIcon 8  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[8], "Radio EmotIcon 9  (CTRL)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[9], "Radio EmotIcon 10 (CTRL)");

	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[10], "Radio EmotIcon 11 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[11], "Radio EmotIcon 12 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[12], "Radio EmotIcon 13 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[13], "Radio EmotIcon 14 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[14], "Radio EmotIcon 15 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[15], "Radio EmotIcon 16 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[16], "Radio EmotIcon 17 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[17], "Radio EmotIcon 18 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[18], "Radio EmotIcon 19 (ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[19], "Radio EmotIcon 20 (ALT)");

	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[20], "Radio EmotIcon 21 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[21], "Radio EmotIcon 22 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[22], "Radio EmotIcon 23 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[23], "Radio EmotIcon 24 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[24], "Radio EmotIcon 25 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[25], "Radio EmotIcon 26 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[26], "Radio EmotIcon 27 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[27], "Radio EmotIcon 28 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[28], "Radio EmotIcon 29 (CTRL + ALT)");
	NAMED_EDITABLE_PARAM(CNCModeSettingsDef, ParameterClass::TYPE_FILENAME, mRadioCmdIcons[29], "Radio EmotIcon 30 (CTRL + ALT)");

	}


CNCModeSettingsDef::~CNCModeSettingsDef(void)
	{
	_mInstance = NULL;
	}


uint32 CNCModeSettingsDef::Get_Class_ID(void) const
	{ 
	return CLASSID_GLOBAL_SETTINGS_DEF_CNCMODE; 
	}


const PersistFactoryClass& CNCModeSettingsDef::Get_Factory(void) const
	{ 
	return _CNCModeSettingsDefPersistFactory; 
	}


PersistClass* CNCModeSettingsDef::Create(void) const
	{
	WWASSERT (0);
	return NULL;
	}


// Save/load constants
enum
	{
	CHUNKID_PARENT =	803001812,
	CHUNKID_VARIABLES,

	VARID_DEF_ANNOUNCEMENT_INTERVAL = 1,
	VARID_DEF_NOD_POWER_OFFLINE_ID,
	VARID_DEF_GDI_POWER_OFFLINE_ID,
	VARID_DEF_NOD_PURCHASE_CANCELED_ID,
	VARID_DEF_GDI_PURCHASE_CANCELED_ID,
	VARID_DEF_NOD_INSUFFICIENT_FUNDS_ID,
	VARID_DEF_GDI_INSUFFICIENT_FUNDS_ID,
	VARID_DEF_NOD_UNIT_READY_ID,
	VARID_DEF_GDI_UNIT_READY_ID,

	VARID_DEF_RADIO_CMD_01,
	VARID_DEF_RADIO_CMD_02,
	VARID_DEF_RADIO_CMD_03,
	VARID_DEF_RADIO_CMD_04,
	VARID_DEF_RADIO_CMD_05,
	VARID_DEF_RADIO_CMD_06,
	VARID_DEF_RADIO_CMD_07,
	VARID_DEF_RADIO_CMD_08,
	VARID_DEF_RADIO_CMD_09,
	VARID_DEF_RADIO_CMD_10,
	VARID_DEF_RADIO_CMD_11,
	VARID_DEF_RADIO_CMD_12,
	VARID_DEF_RADIO_CMD_13,
	VARID_DEF_RADIO_CMD_14,
	VARID_DEF_RADIO_CMD_15,
	VARID_DEF_RADIO_CMD_16,
	VARID_DEF_RADIO_CMD_17,
	VARID_DEF_RADIO_CMD_18,
	VARID_DEF_RADIO_CMD_19,
	VARID_DEF_RADIO_CMD_20,
	VARID_DEF_RADIO_CMD_21,
	VARID_DEF_RADIO_CMD_22,
	VARID_DEF_RADIO_CMD_23,
	VARID_DEF_RADIO_CMD_24,
	VARID_DEF_RADIO_CMD_25,
	VARID_DEF_RADIO_CMD_26,
	VARID_DEF_RADIO_CMD_27,
	VARID_DEF_RADIO_CMD_28,
	VARID_DEF_RADIO_CMD_29,
	VARID_DEF_RADIO_CMD_30,

	VARID_DEF_NOD_ION_BEACON_DEPLOYED_ID,
	VARID_DEF_GDI_ION_BEACON_DEPLOYED_ID,
	VARID_DEF_NOD_ION_BEACON_DISARMED_ID,
	VARID_DEF_GDI_ION_BEACON_DISARMED_ID,
	VARID_DEF_NOD_ION_BEACON_WARNING_ID,
	VARID_DEF_GDI_ION_BEACON_WARNING_ID,

	VARID_DEF_NOD_NUKE_BEACON_DEPLOYED_ID,
	VARID_DEF_GDI_NUKE_BEACON_DEPLOYED_ID,
	VARID_DEF_NOD_NUKE_BEACON_DISARMED_ID,
	VARID_DEF_GDI_NUKE_BEACON_DISARMED_ID,
	VARID_DEF_NOD_NUKE_BEACON_WARNING_ID,
	VARID_DEF_GDI_NUKE_BEACON_WARNING_ID,

	VARID_DEF_RADIO_ICON_01,
	VARID_DEF_RADIO_ICON_02,
	VARID_DEF_RADIO_ICON_03,
	VARID_DEF_RADIO_ICON_04,
	VARID_DEF_RADIO_ICON_05,
	VARID_DEF_RADIO_ICON_06,
	VARID_DEF_RADIO_ICON_07,
	VARID_DEF_RADIO_ICON_08,
	VARID_DEF_RADIO_ICON_09,
	VARID_DEF_RADIO_ICON_10,
	VARID_DEF_RADIO_ICON_11,
	VARID_DEF_RADIO_ICON_12,
	VARID_DEF_RADIO_ICON_13,
	VARID_DEF_RADIO_ICON_14,
	VARID_DEF_RADIO_ICON_15,
	VARID_DEF_RADIO_ICON_16,
	VARID_DEF_RADIO_ICON_17,
	VARID_DEF_RADIO_ICON_18,
	VARID_DEF_RADIO_ICON_19,
	VARID_DEF_RADIO_ICON_20,
	VARID_DEF_RADIO_ICON_21,
	VARID_DEF_RADIO_ICON_22,
	VARID_DEF_RADIO_ICON_23,
	VARID_DEF_RADIO_ICON_24,
	VARID_DEF_RADIO_ICON_25,
	VARID_DEF_RADIO_ICON_26,
	VARID_DEF_RADIO_ICON_27,
	VARID_DEF_RADIO_ICON_28,
	VARID_DEF_RADIO_ICON_29,
	VARID_DEF_RADIO_ICON_30,

	};


bool CNCModeSettingsDef::Save(ChunkSaveClass& csave)
	{
	csave.Begin_Chunk(CHUNKID_PARENT);
		DefinitionClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_ANNOUNCEMENT_INTERVAL, AnnouncementInterval);
		
		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_POWER_OFFLINE_ID, mPowerOfflineID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_POWER_OFFLINE_ID, mPowerOfflineID[1]);
		
		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_PURCHASE_CANCELED_ID, mPurchaseCanceledID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_PURCHASE_CANCELED_ID, mPurchaseCanceledID[1]);
		
		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_INSUFFICIENT_FUNDS_ID, mInsufficientFundsID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_INSUFFICIENT_FUNDS_ID, mInsufficientFundsID[1]);

		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_UNIT_READY_ID, mUnitReadyID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_UNIT_READY_ID, mUnitReadyID[1]);

		for (int radioIndex = 0; radioIndex < 30; ++radioIndex)
			{
			WRITE_MICRO_CHUNK(csave, VARID_DEF_RADIO_CMD_01 + radioIndex, mRadioCmds[radioIndex]);
			WRITE_MICRO_CHUNK_WWSTRING(csave, VARID_DEF_RADIO_ICON_01 + radioIndex, mRadioCmdIcons[radioIndex]);			
			}

		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_ION_BEACON_DEPLOYED_ID, mIonBeaconDeployedID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_ION_BEACON_DEPLOYED_ID, mIonBeaconDeployedID[1]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_ION_BEACON_DISARMED_ID, mIonBeaconDisarmedID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_ION_BEACON_DISARMED_ID, mIonBeaconDisarmedID[1]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_ION_BEACON_WARNING_ID, mIonBeaconWarningID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_ION_BEACON_WARNING_ID, mIonBeaconWarningID[1]);

		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_NUKE_BEACON_DEPLOYED_ID, mNukeBeaconDeployedID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_NUKE_BEACON_DEPLOYED_ID, mNukeBeaconDeployedID[1]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_NUKE_BEACON_DISARMED_ID, mNukeBeaconDisarmedID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_NUKE_BEACON_DISARMED_ID, mNukeBeaconDisarmedID[1]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_NOD_NUKE_BEACON_WARNING_ID, mNukeBeaconWarningID[0]);
		WRITE_MICRO_CHUNK(csave, VARID_DEF_GDI_NUKE_BEACON_WARNING_ID, mNukeBeaconWarningID[1]);

	csave.End_Chunk();

	return true;
	}


bool CNCModeSettingsDef::Load(ChunkLoadClass& cload)
	{
	while (cload.Open_Chunk())
		{
		switch (cload.Cur_Chunk_ID())
			{
			case CHUNKID_PARENT:
				DefinitionClass::Load(cload);
				break;
								
			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk())
					{
					switch (cload.Cur_Micro_Chunk_ID())
						{
						READ_MICRO_CHUNK(cload, VARID_DEF_ANNOUNCEMENT_INTERVAL, AnnouncementInterval);

						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_POWER_OFFLINE_ID, mPowerOfflineID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_POWER_OFFLINE_ID, mPowerOfflineID[1]);
						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_PURCHASE_CANCELED_ID, mPurchaseCanceledID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_PURCHASE_CANCELED_ID, mPurchaseCanceledID[1]);
						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_INSUFFICIENT_FUNDS_ID, mInsufficientFundsID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_INSUFFICIENT_FUNDS_ID, mInsufficientFundsID[1]);
						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_UNIT_READY_ID, mUnitReadyID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_UNIT_READY_ID, mUnitReadyID[1]);

						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_01, mRadioCmds[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_02, mRadioCmds[1]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_03, mRadioCmds[2]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_04, mRadioCmds[3]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_05, mRadioCmds[4]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_06, mRadioCmds[5]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_07, mRadioCmds[6]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_08, mRadioCmds[7]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_09, mRadioCmds[8]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_10, mRadioCmds[9]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_11, mRadioCmds[10]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_12, mRadioCmds[11]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_13, mRadioCmds[12]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_14, mRadioCmds[13]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_15, mRadioCmds[14]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_16, mRadioCmds[15]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_17, mRadioCmds[16]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_18, mRadioCmds[17]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_19, mRadioCmds[18]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_20, mRadioCmds[19]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_21, mRadioCmds[20]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_22, mRadioCmds[21]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_23, mRadioCmds[22]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_24, mRadioCmds[23]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_25, mRadioCmds[24]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_26, mRadioCmds[25]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_27, mRadioCmds[26]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_28, mRadioCmds[27]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_29, mRadioCmds[28]);
						READ_MICRO_CHUNK(cload, VARID_DEF_RADIO_CMD_30, mRadioCmds[29]);

						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_01, mRadioCmdIcons[0]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_02, mRadioCmdIcons[1]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_03, mRadioCmdIcons[2]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_04, mRadioCmdIcons[3]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_05, mRadioCmdIcons[4]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_06, mRadioCmdIcons[5]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_07, mRadioCmdIcons[6]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_08, mRadioCmdIcons[7]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_09, mRadioCmdIcons[8]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_10, mRadioCmdIcons[9]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_11, mRadioCmdIcons[10]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_12, mRadioCmdIcons[11]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_13, mRadioCmdIcons[12]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_14, mRadioCmdIcons[13]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_15, mRadioCmdIcons[14]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_16, mRadioCmdIcons[15]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_17, mRadioCmdIcons[16]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_18, mRadioCmdIcons[17]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_19, mRadioCmdIcons[18]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_20, mRadioCmdIcons[19]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_21, mRadioCmdIcons[20]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_22, mRadioCmdIcons[21]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_23, mRadioCmdIcons[22]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_24, mRadioCmdIcons[23]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_25, mRadioCmdIcons[24]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_26, mRadioCmdIcons[25]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_27, mRadioCmdIcons[26]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_28, mRadioCmdIcons[27]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_29, mRadioCmdIcons[28]);
						READ_MICRO_CHUNK_WWSTRING(cload, VARID_DEF_RADIO_ICON_30, mRadioCmdIcons[29]);
						
						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_ION_BEACON_DEPLOYED_ID, mIonBeaconDeployedID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_ION_BEACON_DEPLOYED_ID, mIonBeaconDeployedID[1]);
						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_ION_BEACON_DISARMED_ID, mIonBeaconDisarmedID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_ION_BEACON_DISARMED_ID, mIonBeaconDisarmedID[1]);
						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_ION_BEACON_WARNING_ID, mIonBeaconWarningID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_ION_BEACON_WARNING_ID, mIonBeaconWarningID[1]);

						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_NUKE_BEACON_DEPLOYED_ID, mNukeBeaconDeployedID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_NUKE_BEACON_DEPLOYED_ID, mNukeBeaconDeployedID[1]);
						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_NUKE_BEACON_DISARMED_ID, mNukeBeaconDisarmedID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_NUKE_BEACON_DISARMED_ID, mNukeBeaconDisarmedID[1]);
						READ_MICRO_CHUNK(cload, VARID_DEF_NOD_NUKE_BEACON_WARNING_ID, mNukeBeaconWarningID[0]);
						READ_MICRO_CHUNK(cload, VARID_DEF_GDI_NUKE_BEACON_WARNING_ID, mNukeBeaconWarningID[1]);

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID (),__FILE__,__LINE__));
							break;
						}

					cload.Close_Micro_Chunk();
					}
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
			}

		cload.Close_Chunk();
		}

	return true;
	}


int CNCModeSettingsDef::Get_Power_Offline_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mPowerOfflineID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Purchase_Canceled_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mPurchaseCanceledID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Insufficient_Funds_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mInsufficientFundsID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Constructing_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mConstructingID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Unit_Ready_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mUnitReadyID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Ion_Beacon_Deployed_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mIonBeaconDeployedID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Ion_Beacon_Disarmed_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mIonBeaconDisarmedID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Ion_Beacon_Warning_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mIonBeaconWarningID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Nuke_Beacon_Deployed_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mNukeBeaconDeployedID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Nuke_Beacon_Disarmed_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mNukeBeaconDisarmedID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Nuke_Beacon_Warning_Report(int team) const
	{
	if (team >= 0 && team < NUM_TEAMS)
		{
		return mNukeBeaconWarningID[team];
		}

	return 0;
	}


int CNCModeSettingsDef::Get_Radio_Command(int num) const
	{
	return mRadioCmds[num];
	}


const char *CNCModeSettingsDef::Get_Radio_Command_Emot_Icon(int num) const
	{
	const char *retval = NULL;
	if (num >= 0 && num < 30)
		{
		retval = mRadioCmdIcons[num];
		}
	return retval;
	}
