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
 *                     $Archive:: /Commando/Code/Combat/assetdep.cpp                          $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/29/01 9:48p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 12                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "assetdep.h"
#include "chunkio.h"
#include "pathutil.h"
#include "wwstring.h"
#include "assetmgr.h"
#include "ffactory.h"
#include "saveloadstatus.h"
#include "wwprofile.h"

///////////////////////////////////////////////////////////////////////
//	Local prototypes
///////////////////////////////////////////////////////////////////////
static void Asset_Name_From_Filename (StringClass& new_name, const char *filename);
static void Get_Filename_From_Path (StringClass& new_name, const char *filename);


///////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////
static const char * ALWAYS_FILENAME	= "always.dep";
static const char * DEP_EXTENSION	= ".dep";

enum
{
	CHUNKID_FILE_LIST			= 0x04020527,

	VARID_ASSET_FILENAME		= 0x01,
};


///////////////////////////////////////////////////////////////////////
//
//	Save_Always_Dependencies
//
///////////////////////////////////////////////////////////////////////
void
AssetDependencyManager::Save_Always_Dependencies (const char *path, ASSET_LIST &asset_list)
{
	//
	//	Get a pointer to the file object
	//
	StringClass filename(path + StringClass ("\\") + StringClass (ALWAYS_FILENAME),true);
	FileClass * file		= _TheWritingFileFactory->Get_File (filename);	
	if (file != NULL) {

		//
		//	Open or create the file
		//
		file->Open (FileClass::WRITE);
		
		//
		//	Save the asset list to the file
		//
		ChunkSaveClass csave (file);
		Save_Dependencies (csave, asset_list);

		//
		//	Close the file
		//
		file->Close ();
		_TheWritingFileFactory->Return_File (file);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Save_Level_Dependencies
//
///////////////////////////////////////////////////////////////////////
void
AssetDependencyManager::Save_Level_Dependencies (const char *full_path, ASSET_LIST &asset_list)
{
	//
	//	Get a pointer to the file object
	//
	FileClass * file = _TheWritingFileFactory->Get_File (full_path);
	if (file != NULL) {

		//
		//	Open or create the file
		//
		file->Open (FileClass::WRITE);
		
		//
		//	Save the asset list to the file
		//
		ChunkSaveClass csave (file);
		Save_Dependencies (csave, asset_list);

		//
		//	Close the file
		//
		file->Close ();
		_TheWritingFileFactory->Return_File (file);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Save_Dependencies
//
///////////////////////////////////////////////////////////////////////
void
AssetDependencyManager::Save_Dependencies (ChunkSaveClass &csave, ASSET_LIST &asset_list)
{
	csave.Begin_Chunk (CHUNKID_FILE_LIST);

		//
		//	Write each filename dependency to a chunk
		//
		for (int index = 0; index < asset_list.Count (); index ++) {
			StringClass &filename = asset_list[index];
			WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_ASSET_FILENAME, filename);
		}

	csave.End_Chunk ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Level_Assets
//
///////////////////////////////////////////////////////////////////////
void
AssetDependencyManager::Load_Level_Assets (const char *level_name)
{
	//
	//	Strip the extension (if necessary)
	//
	StringClass base_name(level_name,true);
	const char *extension = ::strrchr (base_name, '.');
	if (extension != NULL && base_name.Get_Length () > 4) {
		base_name.Erase (base_name.Get_Length () - 4, 4);
	}

	//
	//	Build a filename from the level name, and load the assets from it.
	//
	StringClass filename(base_name + StringClass (DEP_EXTENSION),true);
	Load_Assets (filename);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Always_Assets
//
///////////////////////////////////////////////////////////////////////
void
AssetDependencyManager::Load_Always_Assets (void)
{
	//
	//	Load the assets from the always file
	//
	Load_Assets (ALWAYS_FILENAME);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Assets
//
///////////////////////////////////////////////////////////////////////
void
AssetDependencyManager::Load_Assets (const char *filename)
{
	//
	//	Get a pointer to the file object
	//
	FileClass * file	= _TheFileFactory->Get_File (filename);
	if (file != NULL) {

		if ( file->Is_Available() ) {
			//
			//	Open the file
			//
			file->Open (FileClass::READ);

			//
			//	Load the asset dependencies from the file
			//
			ChunkLoadClass cload (file);
			Load_Assets (cload);

			//
			//	Close the file
			//
			file->Close ();
		} else {
			WWDEBUG_SAY(( "Failed to find %s\n", filename ));
		}

		_TheFileFactory->Return_File (file);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Assets
//
///////////////////////////////////////////////////////////////////////
void
AssetDependencyManager::Load_Assets (ChunkLoadClass &cload)
{
	WWLOG_PREPARE_TIME_AND_MEMORY("AssetDependencyManager::Load_Assets (ChunkLoadClass &cload)");
	cload.Open_Chunk ();
	WWASSERT (cload.Cur_Chunk_ID () == CHUNKID_FILE_LIST);
	if (cload.Cur_Chunk_ID () == CHUNKID_FILE_LIST) {

		//
		//	Read the filename of each asset from the chunk and
		// load its assets into the asset manager.
		//
		while (cload.Open_Micro_Chunk ()) {
			switch (cload.Cur_Micro_Chunk_ID ())
			{
				case VARID_ASSET_FILENAME:
				{
					//
					//	Read the filename from the chunk
					//
					StringClass filename(0,true);
					int size = cload.Cur_Micro_Chunk_Length ();
					cload.Read (filename.Get_Buffer (size), size);

					//
					// Determine what the render object name should be from
					// the filename.
					//
					StringClass render_obj_name(0,true);
					::Asset_Name_From_Filename (render_obj_name,filename);
					INIT_SUB_STATUS(filename);

					//
					//	Load the assets from this file into the asset manager
					//
					if (WW3DAssetManager::Get_Instance ()->Render_Obj_Exists (render_obj_name) == false) {
						WW3DAssetManager::Get_Instance ()->Load_3D_Assets (filename);
					}
//	WWLOG_INTERMEDIATE(filename);
				}
				break;

				default:
					WWDEBUG_SAY (("Unexpected chunk id %d found while preloading assets.\r\n", cload.Cur_Micro_Chunk_ID()));
					break;
			}

			cload.Close_Micro_Chunk ();
		}
	}

	cload.Close_Chunk ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Filename_From_Path
//
////////////////////////////////////////////////////////////////////////////
void Get_Filename_From_Path (StringClass& new_filename, const char *path)
{
	new_filename = cPathUtil::ExtractFilename(path);
}


////////////////////////////////////////////////////////////////////////////
//
//  Asset_Name_From_Filename
//
////////////////////////////////////////////////////////////////////////////
void Asset_Name_From_Filename (StringClass& asset_name, const char *filename)
{
	// Get the filename from this path
	::Get_Filename_From_Path (asset_name, filename);

	// Find and strip off the extension (if it exists)
	char *extension = (char *)::strrchr (asset_name, '.');
	if (extension != NULL) {
		extension[0] = 0;
	}
}
