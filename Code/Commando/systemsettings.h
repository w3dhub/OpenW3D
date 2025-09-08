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
 *                     $Archive:: /Commando/Code/Commando/systemsettings.h                    $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 9/07/01 4:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	SYSTEMSETTINGS_H
#define	SYSTEMSETTINGS_H

#include "vector.h"

class	SystemSettingEntry;
class	RegistryClass;
class INIClass;
//DEADMENU class	MenuEntryClass;
class	ConsoleFunctionClass;

//
// System settings, not related to any particular game
// The settings will exist without other underlying subsystems
// ( Static Shadow Settings even when no Physics Scene is allocated )
// but can be applied when the subsystem is available.
// The settings will save and load in the registry.
// The settings will be be made available to the console system.
// The settings can be accessed from the menu system
//
class	SystemSettings {

public:
	static	void		Init( void );
	static	void		Shutdown( void );

	// List maintenance
	static	void		Add_Setting( SystemSettingEntry * entry )			{ SettingList.Add( entry ); }
	static	void		Remove_Setting( SystemSettingEntry * entry )		{ SettingList.Delete( entry ); }

	static	void		Apply_All( void );

	// Registry save and load
	static	void		Registry_Save( const char * sub_key );
	static	void		Registry_Load( const char * sub_key );

	// Menu Entry Access
	//DEADMENU static	MenuEntryClass	*	Create_Menu_Entry( const char * setting_name );

	// Console Access
	static	void		Add_Console_Functions( DynamicVectorClass<ConsoleFunctionClass *>	& list );

private:

	static	DynamicVectorClass<SystemSettingEntry *>	SettingList;
};

//
// System Settings Entries manage each of the different settings.
// Each can apply itself to underlying subsystems
// Each can save and load itself in the registry
// Each can be accessed via the console system
// Each can be accessed from the menu system
//
class	SystemSettingEntry {

public:
	virtual	~SystemSettingEntry(void){};
	virtual	const char *	Get_Name( void )							= 0;
	virtual	const char *	Get_INI_Name( void )					= 0;
	virtual	const char *	Get_Help( void )							= 0;
	virtual	void				Apply( void )								= 0;

	// Registry save and load
	virtual	void	Registry_Save( RegistryClass & registry )		= 0;
	virtual	void	Registry_Load( RegistryClass & registry )		= 0;

	// INI save and load
	virtual	void	INI_Save( INIClass & ini )		= 0;
	virtual	void	INI_Load( INIClass & ini )		= 0;

	// Menu Entry Access
	//DEADMENU virtual	MenuEntryClass	*	Create_Menu_Entry( void )			= 0;

	// Console Access
	virtual	ConsoleFunctionClass *	Create_Console_Function()	= 0;
};


/*
** Bool Entry
*/
class	SystemSettingEntryBool : public SystemSettingEntry {
public:
	SystemSettingEntryBool( void );

	virtual	void					Apply( void ) override;

	virtual	void					Registry_Save( RegistryClass & registry ) override;
	virtual	void					Registry_Load( RegistryClass & registry ) override;

	virtual	void					INI_Save( INIClass & ini ) override;
	virtual	void					INI_Load( INIClass & ini ) override;

	//DEADMENU virtual	MenuEntryClass	*	Create_Menu_Entry( void );

	virtual	ConsoleFunctionClass *	Create_Console_Function() override;

				bool					Get_State( void );
				void					Set_State( bool state );

protected:
				bool					State;

private:
	virtual	bool					Get_Bool( void )							= 0;
	virtual	void					Set_Bool( bool state )					= 0;
};


/*
** Slider Entry
*/
class	SystemSettingEntrySlider : public SystemSettingEntry {
public:
	SystemSettingEntrySlider( void );

	virtual	void					Apply( void ) override;

	virtual	void					Registry_Save( RegistryClass & registry ) override;
	virtual	void					Registry_Load( RegistryClass & registry ) override;

	virtual	void					INI_Save( INIClass & ini ) override;
	virtual	void					INI_Load( INIClass & ini ) override;
	//DEADMENU virtual	MenuEntryClass	*	Create_Menu_Entry( void );

	virtual	ConsoleFunctionClass *	Create_Console_Function() override;

				int					Get_Value( void );
				void					Set_Value( int value );

				int					Get_Step_Size( void )	 		{ return StepSize; }

protected:
				int					Value;
				int					Min;
				int					Max;
				int					StepSize;

				void					Set_Range( int min, int max )	{ Min = min; Max = max; }
				void					Set_Step_Size( int step )		{ StepSize = step; }

private:
	virtual	int					Get_Slider( void ) 							= 0;
	virtual	void					Set_Slider( int value )						= 0;
};


/*
** Enum Entry
*/
class	SystemSettingEntryEnum : public SystemSettingEntry {
public:
	SystemSettingEntryEnum( void );
	virtual ~SystemSettingEntryEnum(void){};

	virtual	void					Apply( void ) override;

	virtual	void					Registry_Save( RegistryClass & registry ) override;
	virtual	void					Registry_Load( RegistryClass & registry ) override;

	virtual	void					INI_Save( INIClass & ini ) override;
	virtual	void					INI_Load( INIClass & ini ) override;
	//DEADMENU virtual	MenuEntryClass	*	Create_Menu_Entry( void );

	virtual	ConsoleFunctionClass *	Create_Console_Function() override;

				int					Get_Selection( void );
				void					Set_Selection( int selection );
				void					Set_Selection( const char * name );

protected:
				int					Selection;

private:
	virtual	int					Get_Enum( void )									= 0;
	virtual	void					Set_Enum( int selection )						= 0;
public:
	virtual	int					Get_Enum_Count( void )							= 0;
	virtual	const char *		Get_Enum_Name( int selection )				= 0;

};

#endif	// SYSTEMSETTINGS_H
