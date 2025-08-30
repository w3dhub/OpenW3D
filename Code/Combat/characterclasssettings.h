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
 *                     $Archive:: /Commando/Code/Combat/characterclasssettings.h      $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 2/12/01 11:20a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 2                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	__CHARACTER_CLASS_SETTINGS_H
#define	__CHARACTER_CLASS_SETTINGS_H

#include "always.h"
#include "definition.h"


///////////////////////////////////////////////////////////////////////////////////////////
//
//	CharacterClassSettingsDefClass
//
///////////////////////////////////////////////////////////////////////////////////////////
class CharacterClassSettingsDefClass : public DefinitionClass
{
public:

	//////////////////////////////////////////////////////////////////////////
	//	Public constants
	//////////////////////////////////////////////////////////////////////////
	typedef enum
	{
		CLASS_MINIGUNNER			= 0,
		CLASS_ROCKET_SOLDIER,
		CLASS_GRENADIER,
		CLASS_ENGINEER,
		CLASS_FLAME_THROWER,
		CLASS_MUTANT,
		CLASS_COUNT
	} CLASS;

	typedef enum
	{
		RANK_ENLISTED				= 0,
		RANK_OFFICER,
		RANK_SPECIAL_FORCES,
		RANK_BOSS,
		RANK_COUNT
	} RANK;

	typedef enum
	{
		TEAM_GDI				= 0,
		TEAM_NOD,
		TEAM_COUNT
	} TEAM;

	
	//////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////
	CharacterClassSettingsDefClass (void);
	~CharacterClassSettingsDefClass (void);

	//////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////

	//
	//	From DefinitionClass
	//
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual const PersistFactoryClass &	Get_Factory (void) const override;

	static CharacterClassSettingsDefClass *			Get_Instance (void)	{ return GlobalInstance; }	

	//
	//	Accessors
	//
	int								Get_Cost (CLASS char_class, RANK char_rank, TEAM char_team);
	int								Get_Definition (CLASS char_class, RANK char_rank, TEAM char_team);

	
	//
	//	Editable support
	//
	DECLARE_EDITABLE (CharacterClassSettingsDefClass, DefinitionClass);

protected:
	
	//////////////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////////////

	int					CostTable[CLASS_COUNT][RANK_COUNT][TEAM_COUNT];
	int					DefinitionTable[CLASS_COUNT][RANK_COUNT][TEAM_COUNT];

	static CharacterClassSettingsDefClass *			GlobalInstance;
};



#endif	//	__CHARACTER_CLASS_SETTINGS_H
