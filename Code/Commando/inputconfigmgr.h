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
 *                     $Archive:: /Commando/Code/commando/inputconfigmgr.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/26/01 9:45a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __INPUT_CONFIG_MGR_H
#define __INPUT_CONFIG_MGR_H


#include "inputconfig.h"
#include "vector.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;


////////////////////////////////////////////////////////////////
//
//	InputConfigMgrClass
//
////////////////////////////////////////////////////////////////
class InputConfigMgrClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void		Initialize (void);
	static void		Shutdown (void);

	//
	//	Save/load support
	//
	static void		Save (void);
	static void		Load (void);

	//
	//	Current configuration support
	//
	static bool		Get_Current_Configuration (InputConfigClass &config);
	static int		Get_Current_Configuration_Index (void)							{ return CurrentConfigIndex; }
	static void		Load_Current_Configuration (void);
	static void		Load_Default_Configuration (void);
	static void		Load_Configuration (const InputConfigClass &config);
	static void		Save_Current_Configuration (void);	
	static void		Save_Configuration (const InputConfigClass &config);


	//
	//	Configuration list support
	//
	static int		Add_Configuration (const unichar_t *display_name);
	static void		Delete_Configuration (const char *filename);
	static void		Delete_Configuration (int index);
	

	//
	//	Configuration list access
	//
	static int		Get_Configuration_Count (void)									{ return ConfigList.Count (); }
	static void		Get_Configuration (int index, InputConfigClass &config)	{ config = ConfigList[index]; }	

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	static int		Find_Configuration (const char *filename);
	static void		Get_Unique_Config_Filename (StringClass &filename);
	static void		Get_Config_Path (StringClass &full_path);

	//
	//	Save/load support
	//
	static void		Save_Variables (ChunkSaveClass &csave);
	static void		Load_Variables (ChunkLoadClass &cload);

	static void		Save_Config_List (ChunkSaveClass &csave);
	static void		Load_Config_List (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////	
	static DynamicVectorClass<InputConfigClass>	ConfigList;
	static int												CurrentConfigIndex;
};


#endif //__INPUT_CONFIG_MGR_H

