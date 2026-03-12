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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PresetExport.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/12/00 1:01p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PRESET_EXPORT_H
#define __PRESET_EXPORT_H

#include "bittype.h"
#include "vector.h"
#include "wwstring.h"


//////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////
class ParameterClass;
class TextFileClass;
class PresetClass;


//////////////////////////////////////////////////////////////////////////
//
//	PresetExportClass
//
//////////////////////////////////////////////////////////////////////////
class PresetExportClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	PresetExportClass (void)		{}
	~PresetExportClass (void)		{}

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	//
	//	Static methods
	//
	static void			Import (const char *filename);
	static void			Export (uint32 class_id, const char *filename);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	static bool			Can_Export_Parameter (ParameterClass *parameter);
	static void			Write_Column_Headers (uint32 class_id, TextFileClass &file);
	static void			Export_Preset (TextFileClass &file, PresetClass *preset);
	static int			Find_Header (TextFileClass &file);

	static ParameterClass *Find_Parameter (PresetClass *preset, const char *parameter_name);

	static bool			Validate_Columns (int class_id, DynamicVectorClass<StringClass> &column_headers);
	static bool			Read_Column_Headers (TextFileClass &file, DynamicVectorClass<StringClass> &column_headers);

	static bool			Import_Presets (TextFileClass &file, int class_id, DynamicVectorClass<StringClass> &column_headers);
	static bool			Import_Setting (PresetClass *preset, ParameterClass *parameter, const char *value);
};


#endif //__PRESET_EXPORT_H
