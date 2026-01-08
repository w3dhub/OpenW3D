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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/purchasesettings.h                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/05/02 5:14p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PURCHASESETTINGS_H
#define __PURCHASESETTINGS_H

#include "always.h"
#include "definition.h"
#include "widestring.h"


///////////////////////////////////////////////////////////////////////////////////////////
//
//	PurchaseSettingsDefClass
//
///////////////////////////////////////////////////////////////////////////////////////////
class PurchaseSettingsDefClass : public DefinitionClass
{
public:

	//////////////////////////////////////////////////////////////////////////
	//	Public constants
	//////////////////////////////////////////////////////////////////////////
	typedef enum
	{
		TYPE_CLASSES		= 0,
		TYPE_VEHICLES,
		TYPE_EQUIPMENT,
		TYPE_SECRET_CLASSES,
		TYPE_SECRET_VEHICLES,
		TYPE_COUNT
	} TYPE;

	typedef enum
	{
		TEAM_GDI				= 0,
		TEAM_NOD,
		TEAM_MUTANT_GDI,
		TEAM_MUTANT_NOD,
		TEAM_COUNT
	} TEAM;
	
	//////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////
	PurchaseSettingsDefClass (void);
	~PurchaseSettingsDefClass (void);

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

	//
	//	Accessors
	//
	TYPE								Get_Type (void)							{ return Type; }
	TEAM								Get_Team (void)							{ return Team; }

	const unichar_t *					Get_Name (int index);
	int								Get_Cost (int index)						{ return CostList[index]; }
	int								Get_Definition (int index)				{ return DefinitionList[index]; }
	const StringClass &			Get_Texture (int index)					{ return TextureList[index]; }

	const unichar_t *					Get_Enlisted_Name (int index);
	int								Get_Enlisted_Definition (int index)	{ return DefinitionList[index]; }
	const StringClass &			Get_Enlisted_Texture (int index)		{ return TextureList[index]; }

	//
	//	Alternate support
	//
	int								Get_Alt_Definition (int index, int alt_index)	{ return AlternateDefinitionList[index][alt_index]; }
	const StringClass &			Get_Alt_Texture (int index, int alt_index)		{ return AlternateTextureList[index][alt_index]; }

	//
	//	Singleton access
	//
	static PurchaseSettingsDefClass *	Find_Definition (TYPE type, TEAM team);
	
	//
	//	Editable support
	//
	DECLARE_EDITABLE (PurchaseSettingsDefClass, DefinitionClass);

protected:

	//////////////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////////////
	void				Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////////////////
	//	Protected constants
	//////////////////////////////////////////////////////////////////////////
	enum
	{
		ROWS	= 4,
		COLS	= 5,
		MAX_ALTERNATES = 3,
		MAX_ENTRIES = 10
	};
	
	//////////////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////////////

	TEAM					Team;
	TYPE					Type;
	int					CostList[MAX_ENTRIES];
	int					DefinitionList[MAX_ENTRIES];
	int					NameList[MAX_ENTRIES];
	StringClass			TextureList[MAX_ENTRIES];

	int					AlternateDefinitionList[MAX_ENTRIES][MAX_ALTERNATES];
	StringClass			AlternateTextureList[MAX_ENTRIES][MAX_ALTERNATES];
	
	static PurchaseSettingsDefClass *	DefinitionArray[TYPE_COUNT][TEAM_COUNT];
};


#endif //__PURCHASESETTINGS_H
