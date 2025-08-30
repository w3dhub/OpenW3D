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
 *                     $Archive:: /Commando/Code/Installer/RAMFileFactory.h  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 10/31/01 2:52p                $* 
 *                                                                                             * 
 *                    $Revision:: 2                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _RAM_FILE_FACTORY_H
#define _RAM_FILE_FACTORY_H


// Includes.
#include "FFactory.h"


// Classes 
class	RAMFileFactoryClass : public FileFactoryClass {

	public:
		 RAMFileFactoryClass();
		~RAMFileFactoryClass();

		FileClass *Get_File (const char *filename) override;
		void		  Return_File (FileClass *file) override;

	protected:
		unsigned char *FileBuffer;
		unsigned			FileBufferSize;
		StringClass		FileName;
};


#endif // RAM_FILE_FACTORY_H
