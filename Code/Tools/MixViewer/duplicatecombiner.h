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
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/MixViewer/duplicatecombiner.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/12/01 7:17p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DUPLICATE_REMOVER_H
#define __DUPLICATE_REMOVER_H

#include "wwstring.h"
#include "vector.h"


//////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////
class MixFileFactoryClass;
class MixCombiningDialogClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	DuplicateRemoverClass
//
//////////////////////////////////////////////////////////////////////////////////
class DuplicateRemoverClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructor
	//////////////////////////////////////////////////////////////////
	DuplicateRemoverClass (void);
	~DuplicateRemoverClass (void);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	//	Configuration
	//
	void			Add_Mix_File (const char *filename)				{ MixFileList.Add (filename); }
	void			Set_Destination_File (const char *filename)	{ DestinationMixFilename = filename; }

	//
	//	Processing
	//
	void			Process (void);

private:

	//////////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////////
	void			Open_Mix_Files (DynamicVectorClass<MixFileFactoryClass *> &list);
	void			Close_Mix_Files (DynamicVectorClass<MixFileFactoryClass *> &list);
	void			Internal_Process (void);

	static UINT	fnThreadProc (LPVOID pParam);


	//
	//	Utilities
	//
	void			Copy_File (MixFileFactoryClass *src_mix, MixFileFactoryClass *dest_mix, const char *filename);
	bool			Clean_Directory (LPCTSTR local_dir);
	void			Get_Temp_Filename (StringClass &full_path);
	void			Make_Temp_Directory (void);
	void			Delete_Temp_Directory (void);
	bool			Delete_File (LPCTSTR filename);
	StringClass	Make_Path (LPCTSTR path, LPCTSTR filename);
	bool			Is_File_In_Factory (const StringClass &filename, MixFileFactoryClass *factory);


	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	DynamicVectorClass<StringClass>	MixFileList;
	StringClass								DestinationMixFilename;
	StringClass								TempDirectory;
	int										TempFilenameStart;
	MixCombiningDialogClass	*			Dialog;
};


#endif //__DUPLICATE_REMOVER_H

