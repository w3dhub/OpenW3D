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
*     Toolkit_Powerup.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Powerup Manager
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Rich_d $
*     $Revision: 28 $
*     $Modtime: 2/18/02 4:41p $
*     $Archive: /Commando/Code/Scripts/Toolkit_Powerup.cpp $
*
******************************************************************************/

#include <cstdint>
#include "Toolkit.h"

// The Soldier Powerup Tables contains three columns.
// The first chars of the preset name, the weapon powerup, and the twiddler powerup.
// The first preset name match is used.

#define	POWERUP_TABLE_SIZE  ( sizeof(Soldier_Powerup_Table) / sizeof (Soldier_Powerup_Table[0]) )
const char *	Soldier_Powerup_Table[][3] = {
	{ "Mutant_0_Mutant",						"tw_POW00_Armor",				"tw_POW00_Armor"			},
	{ "Mutant_1Off_Acolyte",					"POW_TiberiumAutoRifle_Player",		"tw_POW02_LaserRifle"		},
	{ "Mutant_2SF_Templar",						"POW_TiberiumAutoRifle_Player",	"tw_POW01_TiberiumAutoRifle"},
	{ "Nod_FlameThrower_2SF",					"POW_LaserRifle_Player",		"tw_POW02_LaserRifle"		},
	{ "Nod_FlameThrower_1Off",					"POW_ChemSprayer_Player",		"tw_POW01_Chemsprayer"		},
	{ "Nod_FlameThrower_0",						"POW_Flamethrower_Player",		"tw_POW00_Flamethrower"		},
	{ "Nod_Minigunner_0",						"POW_AutoRifle_Player",			"tw_POW00_AutoRifle"		},
	{ "Nod_Minigunner_1Off_LaserChaingun",		"POW_LaserChaingun_Player",		"tw_POW01_LaserChaingun"	},
	{ "Nod_Minigunner_1Off_Shotgun",			"tw_POW00_Health",				"tw_POW00_Health"			},
	{ "Nod_Minigunner_1Off",					"POW_Chaingun_Player",			"tw_POW01_Chaingun"			},
	{ "Nod_Minigunner_2SF_AutoRifle",			"POW_AutoRifle_Player",			"tw_POW02_AutoRifle"		},
	{ "Nod_Minigunner_2SF_Chaingun",			"POW_Chaingun_Player",			"tw_POW02_Chaingun"			},
	{ "Nod_Minigunner_2SF_Ramjet",				"POW_RamjetRifle_Player",		"tw_POW02_RamjetRifle"		},
	{ "Nod_Minigunner_2SF_Stationary",			"POW_SniperRifle_Player",		"tw_POW02_SniperRifle"		},
	{ "Nod_Minigunner_2SF_Shotgun",				"tw_POW00_Health",				"tw_POW02_SniperRifle"		},
	{ "Nod_Minigunner_2SF_LaserRifle",			"tw_POW00_Health",				"tw_POW00_Health"			},
	{ "Nod_Minigunner_2SF",						"POW_SniperRifle_Player",		"tw_POW00_Health"			},
	{ "Nod_RocketSoldier_2SF_AutoRifle",		"POW_AutoRifle_Player",			"tw_POW02_AutoRifle"		},
	{ "Nod_RocketSoldier_2SF_Chaingun",			"POW_Chaingun_Player",			"tw_POW02_Chaingun"			},
	{ "Nod_RocketSoldier_2SF_VoltAutoRifle",	"POW_VoltAutoRifle_Player",		"tw_POW02_VoltAutoRifle"	},
	{ "Nod_RocketSoldier_2SF_LaserRifle",		"tw_POW00_Health",				"tw_POW00_Health"			},
	{ "Nod_RocketSoldier",						"POW_RocketLauncher_Player",	"tw_POW00_RocketLauncher"	},
	{ "",										"tw_POW00_Health",				"tw_POW00_Health"			},	// Last entry is the catch-all
};

DECLARE_SCRIPT(M00_Soldier_Powerup_Grant, "")
{
	bool Disabled;		// Has this script been disabled (usually for a custom override)

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( Disabled, 1 );
	}

	void Created(GameObject* /*obj*/) override
	{
		Disabled = false;		// Default to enabled
	}

	// Find the index of the first matching sub name
	int Find_Powerup_Table_Index( const char * name ) 
	{
		for ( int i = 0; i < POWERUP_TABLE_SIZE; i++ ) {
			const char * test_name = Soldier_Powerup_Table[i][0];
			if ( ::strncmp( test_name, name, ::strlen( test_name ) ) == 0 ) {
				return i;
			}
		}
		Commands->Debug_Message( "Soldier_Powerup_Grant failed to find name match %s\n", name );
		return 0;
	}

	// When killed, drop a powerup
	void Killed (GameObject * obj, GameObject * killer) override
	{
		// We only drop a powerup if our killer is a star
		// 100% for Easy
		// 67% for Medium
		// 33% for Hard
		if( !Disabled && killer && Commands->Is_A_Star(killer) && (Commands->Get_Random_Int(0, 3) >= Commands->Get_Difficulty_Level())) 
		{
			// Find the destoyed object's preset name
			const char * preset_name = Commands->Get_Preset_Name( obj );
			Commands->Debug_Message( "Soldier_Powerup_Grant for %s\n", preset_name );

			// Find the index of this object
			int index = Find_Powerup_Table_Index( preset_name );
			Commands->Debug_Message( "Soldier_Powerup_Grant: index %d\n", index );

			// Calculate the star's health percentage
			float star_health = Commands->Get_Health( killer );
			float star_health_max = Commands->Get_Max_Health( killer );
			float star_health_percent = 0;
			if ( star_health_max ) {
				star_health_percent = star_health / star_health_max;
			}

			// Calculate the star's shield percentage
			float star_shield = Commands->Get_Shield_Strength( killer );
			float star_shield_max = Commands->Get_Max_Shield_Strength( killer );
			float star_shield_percent = 0;
			if ( star_shield_max ) {
				star_shield_percent = star_shield / star_shield_max;
			}

			// Calcutate a powerup spawn point
			Vector3 spawn_spot = Commands->Get_Position ( obj );
			spawn_spot.Z += 0.75f;	// Bump it up a bit

			GameObject * powerup = NULL;

			// If star's health < 25%, drop health
			if ( star_health_percent < 0.25f ) {
				Commands->Debug_Message( "Soldier_Powerup_Grant: Star's Health < 25%%.  Dropping Health\n" );
				powerup = Commands->Create_Object ( "tw_POW00_Health", spawn_spot );
			}

			// If star's shield < 25%, drop shield
			if ( powerup == NULL && star_shield_percent < 0.25f ) {
				Commands->Debug_Message( "Soldier_Powerup_Grant: Star's Shield < 25%%.  Dropping Sheild\n" );
				powerup = Commands->Create_Object ( "tw_POW00_Armor", spawn_spot );
			}

			// If star's health > 75%, drop weapon
			if ( powerup == NULL && star_health_percent > 0.75f ) {
				const char * weapon_powerup = Soldier_Powerup_Table[index][1];
				if ( weapon_powerup != NULL && weapon_powerup[0] != 0 ) {
					Commands->Debug_Message( "Soldier_Powerup_Grant: Star's Health > 75%%.  Dropping soldier's weapon %s\n", weapon_powerup );
					powerup = Commands->Create_Object ( weapon_powerup, spawn_spot );
				}
			}

			// Drop soldier's twiddler
			if ( powerup == NULL ) {
				const char * twiddler = Soldier_Powerup_Table[index][2];
				if ( twiddler != NULL && twiddler[0] != 0 ) {
					Commands->Debug_Message( "Soldier_Powerup_Grant: Dropping twiddler %s\n", twiddler );
					powerup = Commands->Create_Object ( twiddler, spawn_spot );
				}
			}

			// It we created a powerup, give it the powerup script
			if ( powerup != NULL ) {
				Commands->Attach_Script( powerup, "M00_Powerup_Destroy", "");
			}
		}
	}

	// Custom to disable this script
	void	Custom( GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/ ) override
	{
		if ( type == M00_CUSTOM_POWERUP_GRANT_DISABLE ) {
			Disabled = true;
		}
	}

};


// DAY - Attach to disable powerup manager on an instance or preset
DECLARE_SCRIPT(M00_Soldier_Powerup_Disable, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 1.0f, 1);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			Commands->Send_Custom_Event(obj, obj, M00_CUSTOM_POWERUP_GRANT_DISABLE, 0, 0.0f);
		}
	}
};


// DAY - Used to destroy a powerup dropped by the powerup manager over time.  Based on Difficulty.
DECLARE_SCRIPT(M00_Powerup_Destroy, "")
{
		
	void Created (GameObject * obj) override
	{
		float lifespan = (30.0f - (Commands->Get_Difficulty_Level() * 10));
		Commands->Start_Timer (obj, this, lifespan, 10);
		
	}
	
	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == 10)
		{
			Commands->Expire_Powerup(obj);
		}
	}
};


/*********************************************************************************************** 
**
** Reveal Encyclopedia Powerup Scripts.  Reveal ID's of encyclopedia ini entries based on type
**
************************************************************************************************/

// DAY - Reveals a Building entry in the Encyclopedia.  Param is the Encyclopedia ID listed in the Buildings.ini file.
DECLARE_SCRIPT (M00_Reveal_Enc_Building_DAY, "BuildingEncyclopediaID:int")
{
	void Custom(GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED )
		{
			bool reveal = Commands->Reveal_Encyclopedia_Building( Get_Int_Parameter("BuildingEncyclopediaID") );
			if ( reveal == 1 )
			{
				Commands->Display_Encyclopedia_Event_UI();
			}
		}
	}

};


// DAY - Reveals a Character entry in the Encyclopedia.  Param is the Encyclopedia ID listed in the Characters.ini file.
DECLARE_SCRIPT (M00_Reveal_Enc_Character_DAY, "CharacterEncyclopediaID:int")
{
	void Custom(GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED )
		{
			bool reveal = Commands->Reveal_Encyclopedia_Character( Get_Int_Parameter("CharacterEncyclopediaID") );
			if ( reveal == 1 )
			{
				Commands->Display_Encyclopedia_Event_UI();
			}
		}
	}

};


// DAY - Reveals a Vehicle entry in the Encyclopedia.  Param is the Encyclopedia ID listed in the Vehicles.ini file.
DECLARE_SCRIPT (M00_Reveal_Enc_Vehicle_DAY, "VehicleEncyclopediaID:int")
{
	void Custom(GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED )
		{
			bool reveal = Commands->Reveal_Encyclopedia_Vehicle( Get_Int_Parameter("VehicleEncyclopediaID") );
			if ( reveal == 1 )
			{
				Commands->Display_Encyclopedia_Event_UI();
			}
		}
	}

};


// DAY - Reveals a Weapon entry in the Encyclopedia.  Param is the Encyclopedia ID listed in the Weapons.ini file.
//			Normally not necessary as picking up weapon will reveal entry.
DECLARE_SCRIPT (M00_Reveal_Enc_Weapon_DAY, "WeaponEncyclopediaID:int")
{
	void Custom(GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED )
		{
			bool reveal = Commands->Reveal_Encyclopedia_Weapon( Get_Int_Parameter("WeaponEncyclopediaID") );
			if ( reveal == 1 )
			{
				Commands->Display_Encyclopedia_Event_UI();
			}
		}
	}

};



/*********************************************************************************************** 
**
** Mutliplayer Score/Money Grant Scripts
**		DAY - Two scripts that grant random incremental allotments of points or money for
**		multiplayer.
**
************************************************************************************************/

// DAY - Multiplayer Script to Grant Score
DECLARE_SCRIPT (M00_GrantScore_Powerup, "ScoreAmount:float,Entire_Team=0:int,Randomizer=1:int")
{

	void Custom( GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * sender ) override
	{

		float score;
		bool team_grant;
		int random_multiply;
		int factor_multiply;
		float score_grant;
		
		score = Get_Float_Parameter( "ScoreAmount" );
		team_grant = (Get_Int_Parameter("Entire_Team") == 1) ? true : false;
		random_multiply = Get_Int_Parameter( "Randomizer" );
		if ( random_multiply < 1 )
		{
			random_multiply = 1;
		}
		factor_multiply = Commands->Get_Random_Int( 1, (random_multiply + 1) );
		score_grant = factor_multiply * score;

		if ( type == CUSTOM_EVENT_POWERUP_GRANTED )
		{
			Commands->Give_Points( sender, score_grant, team_grant );
		}
	}

};

// DAY - Multiplayer Script to Grant Money
DECLARE_SCRIPT (M00_GrantMoney_Powerup, "ScoreAmount:float,Entire_Team=0:int,Randomizer=1:int")
{

	void Custom( GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * sender ) override
	{

		float score;
		bool team_grant;
		int random_multiply;
		int factor_multiply;
		float score_grant;
		
		score = Get_Float_Parameter( "ScoreAmount" );
		team_grant = (Get_Int_Parameter("Entire_Team") == 1) ? true : false;
		random_multiply = Get_Int_Parameter( "Randomizer" );
		if ( random_multiply < 1 )
		{
			random_multiply = 1;
		}
		factor_multiply = Commands->Get_Random_Int( 1, (random_multiply + 1) );
		score_grant = factor_multiply * score;

		if ( type == CUSTOM_EVENT_POWERUP_GRANTED )
		{
			Commands->Give_Money( sender, score_grant, team_grant );
		}
	}

};


// Attach this script to the Tiberium Refinery controller. It will issue money to that
// controller's team until the Refinery is destroyed.

DECLARE_SCRIPT (M00_Tiberium_Refinery, "MoneyAmount:int,TimerLength:int")
{
	void Created (GameObject * obj) override
	{
		Commands->Start_Timer (obj, this, Get_Int_Parameter("TimerLength"), 1);
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			float health = Commands->Get_Health (obj);
			if (health)
			{
				Commands->Give_Money (obj, Get_Int_Parameter ("MoneyAmount"), true);
				Commands->Start_Timer (obj, this, Get_Int_Parameter("TimerLength"), 1);
			}
		}
	}
};


DECLARE_SCRIPT (M00_CNC_Crate, "")
{
	void Custom (GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * sender) override
	{
		if ((type == CUSTOM_EVENT_POWERUP_GRANTED) && (sender))
		{
			Commands->Give_Money (sender, 100.0f, false);
		}
	}
};


DECLARE_SCRIPT (M00_Death_Powerup, "")
{
	void Custom (GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * sender) override
	{
		if ((type == CUSTOM_EVENT_POWERUP_GRANTED) && (sender))
		{
			Commands->Apply_Damage (sender, 10000.0f, "Death", NULL);
		}
	}
};
