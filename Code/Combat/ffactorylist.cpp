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
 *                     $Archive:: /Commando/Code/Combat/ffactorylist.cpp                      $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/21/02 3:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "ffactorylist.h"
#include "wwfile.h"


FileFactoryListClass * FileFactoryListClass::Instance = nullptr;

/*
**
*/
FileFactoryListClass::FileFactoryListClass( void ) :
	SearchStartIndex( 0 ),
	TempFactory( nullptr )
{
	WWASSERT( Instance == nullptr );
	Instance = this;
}

FileFactoryListClass::~FileFactoryListClass( void )
{
	WWASSERT( Instance == this );
	Instance = nullptr;
}

/*
**
*/
void	FileFactoryListClass::Add_FileFactory( FileFactoryClass * factory, const char *name )
{
	FactoryList.Add( factory );
	FactoryNameList.Add( name );
	Reset_Search_Start ();
}


/*
**
*/
void	FileFactoryListClass::Remove_FileFactory( FileFactoryClass * factory )
{
	for (int index = 0; index < FactoryList.Count (); index ++) {

		//
		//	If this is the factory we're looking for, then remove
		// it from the list.
		//
		if (FactoryList[index] == factory) {
			FactoryList.Delete (index);
			FactoryNameList.Delete (index);
			Reset_Search_Start ();
			break;
		}
	}

	return ;
}

/***********************************************************************************************
 * FileFactoryListClass::Remove_FileFactory -- Remove any old file factory                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to removed factory. nullptr if none in the list                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/7/2001 4:40PM ST : Created                                                              *
 *=============================================================================================*/
FileFactoryClass *FileFactoryListClass::Remove_FileFactory(void)
{
	FileFactoryClass *factory = nullptr;

	if (FactoryList.Count()) {
		factory = FactoryList[0];
		FactoryList.Delete(0);
		FactoryNameList.Delete(0);
	}

	Reset_Search_Start ();
	return(factory);
}


void	FileFactoryListClass::Add_Temp_FileFactory( FileFactoryClass * factory )
{
	WWASSERT( TempFactory == nullptr );
	TempFactory = factory;
}

FileFactoryClass * FileFactoryListClass::Remove_Temp_FileFactory( void )
{
	FileFactoryClass * factory = TempFactory;
	TempFactory = nullptr;
	return factory;
}


FileClass * FileFactoryListClass::Get_File( char const *filename )
{
	// Very kludgly...

	// Then the temp factory
	if ( TempFactory ) {
		FileClass * file = TempFactory->Get_File( filename );
		if ( file != nullptr ) {
			if ( file->Is_Available() ) {
				return file;
			} else {
				TempFactory->Return_File( file );
			}
		}
	}

	// Try the first in the list...
	if ( SearchStartIndex < FactoryList.Count() ) {
		FileClass * file = FactoryList[SearchStartIndex]->Get_File( filename );
		if ( file != nullptr ) {
			if ( file->Is_Available() ) {
				return file;
			} else {
				FactoryList[SearchStartIndex]->Return_File( file );
			}
		}
	}

	// Then try the rest
	for ( int i = 0; i < FactoryList.Count(); i++ ) {
		if (i != SearchStartIndex) {
			FileClass * file = FactoryList[i]->Get_File( filename );
			if ( file != nullptr ) {
				if ( file->Is_Available() ) {
					return file;
				} else {
					FactoryList[i]->Return_File( file );
				}
			}
		}
	}

	// Failed!

	// Just use the first and don't check for available
	if ( FactoryList.Count() > 0 ) {
		FileClass * file = FactoryList[0]->Get_File( filename );
		if ( file != nullptr ) {
			return file;
		}
	}

	return nullptr;
}

void FileFactoryListClass::Return_File( FileClass *file )
{
	// This is kinda bad. Just return it to the first one.  (Since they all do the same thing)
	FactoryList[0]->Return_File( file );
}


void FileFactoryListClass::Set_Search_Start( const char *name )
{
	SearchStartIndex = 0;

	//
	//	Try to find which mix file we should start searching from...
	//
	for (int index = 0; index < FactoryNameList.Count (); index ++) {
		if (FactoryNameList[index].Compare_No_Case( name ) == 0) {
			SearchStartIndex = index;
			break;
		}
	}
	return ;
}
