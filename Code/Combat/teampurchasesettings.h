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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/teampurchasesettings.h                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/23/01 3:15p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TEAMPURCHASESETTINGS_H
#define __TEAMPURCHASESETTINGS_H

#include "translatedb.h"
#include "wwstring.h"
#include "definition.h"

//////////////////////////////////////////////////////////////////////
//
//	TeamPurchaseSettingsDefClass
//
//////////////////////////////////////////////////////////////////////
class TeamPurchaseSettingsDefClass : public DefinitionClass
{
public:

	//////////////////////////////////////////////////////////////////////
	//	Public constants
	//////////////////////////////////////////////////////////////////////
	typedef enum
	{
		TEAM_GDI				= 0,
		TEAM_NOD,
		TEAM_COUNT
	} TEAM;
	
	//////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////
	TeamPurchaseSettingsDefClass (void);
	~TeamPurchaseSettingsDefClass (void);

	//////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////

	//
	//	From DefinitionClass
	//
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual const PersistFactoryClass &	Get_Factory (void) const override;

	//
	//	Accessors
	//
	TEAM								Get_Team (void)							{ return Team; }

	const unichar_t *					Get_Enlisted_Name (int index);
	int								Get_Enlisted_Definition (int index)	{ return DefinitionList[index]; }
	const StringClass &			Get_Enlisted_Texture (int index)		{ return TextureList[index]; }

	const unichar_t *					Get_Beacon_Name (void)			{ return TRANSLATE (BeaconNameID); }
	int								Get_Beacon_Cost (void)			{ return BeaconCost; }
	int								Get_Beacon_Definition (void)	{ return BeaconDefinitionID; }
	const StringClass &			Get_Beacon_Texture (void)		{ return BeaconTextureName; }

	const unichar_t *					Get_Supply_Name (void)			{ return TRANSLATE (SupplyNameID); }
	const StringClass &			Get_Supply_Texture (void)		{ return SupplyTextureName; }

	//
	//	Singleton access
	//
	static TeamPurchaseSettingsDefClass *	Get_Definition (TEAM team);
	
	//
	//	Editable support
	//
	DECLARE_EDITABLE (TeamPurchaseSettingsDefClass, DefinitionClass);

protected:

	//////////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////////
	void				Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////////////
	//	Protected constants
	//////////////////////////////////////////////////////////////////////
	enum
	{
		MAX_ENTRIES = 4
	};
	
	//////////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////////
	TEAM					Team;
	
	//
	//	Enlisted character settings
	//
	int					DefinitionList[MAX_ENTRIES];
	int					NameList[MAX_ENTRIES];
	StringClass			TextureList[MAX_ENTRIES];

	//
	//	Beacon settings
	//
	int					BeaconCost;
	int					BeaconDefinitionID;
	int					BeaconNameID;
	StringClass			BeaconTextureName;

	//
	//	Supply settings
	//
	int					SupplyNameID;
	StringClass			SupplyTextureName;
	
	static TeamPurchaseSettingsDefClass *	DefinitionArray[TEAM_COUNT];
};


#endif //__TEAMPURCHASESETTINGS_H
