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
 *                 Project Name : Installer                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Installer/RAMFileFactory.cpp  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 10/31/01 2:57p                $* 
 *                                                                                             * 
 *                    $Revision:: 2                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "RAMFileFactory.h"
#include "BuffFile.h"
#include "RamFile.h"


/***********************************************************************************************
 * RAMFileFactoryClass::RAMFileFactoryClass --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
RAMFileFactoryClass::RAMFileFactoryClass()
	: FileFactoryClass(),
	  FileBuffer (NULL)	
{
}


/***********************************************************************************************
 * RAMFileFactoryClass::RAMFileFactoryClass --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
RAMFileFactoryClass::~RAMFileFactoryClass()
{
	if (FileBuffer != NULL) delete [] FileBuffer;
}


/***********************************************************************************************
 * RAMFileFactoryClass::Get_File --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
FileClass *RAMFileFactoryClass::Get_File (const char *filename)
{
	RAMFileClass *ramfile;

	// If file buffer has not been read then do it now.
	if ((FileBuffer == NULL) || (stricmp (filename, FileName) != 0)) {
	
		BufferedFileClass *bufferfile;

		if (FileBuffer != NULL) delete [] FileBuffer;

		bufferfile = new BufferedFileClass (filename);
		if ((bufferfile != NULL) && bufferfile->Is_Available()) {
			FileBuffer = new unsigned char [bufferfile->Size()];
			bufferfile->Open();
			bufferfile->Read (FileBuffer, bufferfile->Size());
		}

		FileBufferSize = bufferfile->Size();
		FileName			= filename;
		
		// Clean-up.
		delete bufferfile;
	}
		
	ramfile = new RAMFileClass (FileBuffer, FileBufferSize);
	return (ramfile);
}


/***********************************************************************************************
 * RAMFileFactoryClass::Return_File --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void RAMFileFactoryClass::Return_File (FileClass *file)
{
	delete file;
}
