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
 *                     $Archive:: /Commando/Code/Combat/savegame.cpp                          $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 3/07/02 12:05p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 45                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "savegame.h"
#include "definitionmgr.h"
#include "debug.h"
#include "chunkio.h"
#include "ffactory.h"
#include "combatsaveload.h"
#include "physstaticsavesystem.h"
#include "physdynamicsavesystem.h"
#include "AudioSaveLoad.h"
#include "matrix3d.h"
#include "scriptman.h"
#include "combat.h"
#include "backgroundmgr.h"
#include "conversationmgr.h"
#include "WeatherMgr.h"
#include "wwmemlog.h"
#include "translatedb.h"
#include "mapmgr.h"
#include "encyclopediamgr.h"
#include "ffactorylist.h"
#include "mixfile.h"
#include "texturethumbnail.h"
#include "systeminfolog.h"
#include "wwprofile.h"
#include <stdlib.h>
#include "specialbuilds.h"

/*
**
*/
StringClass		SaveGameManager::MapFilename;
StringClass		SaveGameManager::CurrentGameFilename;
WideStringClass	SaveGameManager::Description;
int				SaveGameManager::MissionDescriptionID = 0;
const char *	SaveGameManager::DefaultDefinitionFilename = "Objects.DDB";

/*
**
*/
enum	{

#ifdef BETACLIENT
	//
	// This CHUNKID_LEVEL_INFO tweaking is temporary, to disallow the aircraft maps 
	// to be used outside of the beta. The maps distributed with the Beta client must have 
	// 1011991648 replaced with 1011991650 and 1011991649 replaced with 1011991651.
	//
	CHUNKID_LEVEL_INFO					=	1011991650,
#else
	CHUNKID_LEVEL_INFO					=	1011991648,
#endif

	CHUNKID_LEVEL_DATA,

	MICROCHUNKID_MAP_FILENAME			=	1,
	MICROCHUNKID_MISSION_DESCRIPTION,
	MICROCHUNKID_DESCRIPTION,
};

/*
**
*/
void __cdecl SaveGameManager::Save_Game( const char * filename, ... )
{
	Debug_Say(( "Save Game %s\n", filename ));
	CurrentGameFilename = filename;

	FileClass * file = _TheWritingFileFactory->Get_File( filename );
	WWASSERT(file);
	file->Open(FileClass::WRITE);

	ChunkSaveClass csave(file);

    SaveLoadSystemClass::ResetPointerRemapper();

	csave.Begin_Chunk( CHUNKID_LEVEL_INFO );
		WRITE_MICRO_CHUNK_WWSTRING( csave,		MICROCHUNKID_MAP_FILENAME,			MapFilename );
		WRITE_MICRO_CHUNK_WIDESTRING( csave,	MICROCHUNKID_DESCRIPTION,			Description );
		WRITE_MICRO_CHUNK( csave,				MICROCHUNKID_MISSION_DESCRIPTION,	MissionDescriptionID );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_LEVEL_DATA );

		_ConversationMgrSaveLoad.Set_Category_To_Save (ConversationMgrClass::CATEGORY_LEVEL);

		SaveLoadSystemClass::Save( csave, _CombatSaveLoad );
		SaveLoadSystemClass::Save( csave, _ConversationMgrSaveLoad );
		SaveLoadSystemClass::Save( csave, _PhysDynamicSaveSystem );
		SaveLoadSystemClass::Save( csave, _TheEncyclopediaMgrSaveLoadSubsystem );
		SaveLoadSystemClass::Save( csave, _DynamicAudioSaveLoadSubsystem );
		SaveLoadSystemClass::Save( csave, _TheMapMgrSaveLoadSubsystem );

		va_list arg_list;
		va_start( arg_list, filename );

		bool done = false;
		while ( !done ) {
			SaveLoadSubSystemClass * sub_system = va_arg( arg_list, SaveLoadSubSystemClass * );
			if ( sub_system != NULL ) {
				SaveLoadSystemClass::Save( csave, *sub_system );
			} else {
				done = true;
			}
		}
		va_end (arg_list);

	csave.End_Chunk();

	file->Close();

	_TheWritingFileFactory->Return_File(file);

}


void	SaveGameManager::Pre_Load_Game
(
	 const char *	filename,
	 StringClass & filename_to_load,
	 StringClass &	lsd_filename 
)
{
	//
	//	Get the root name and extension from the filename
	//
	char root_name[_MAX_FNAME] = { 0 };
	char extension[_MAX_EXT] = { 0 };
	::_splitpath (filename, NULL, NULL, root_name, extension);

	SystemInfoLog::Set_Current_Level(root_name);
	filename_to_load = filename;

	//
	//	Reset the search order
	//
	if (FileFactoryListClass::Get_Instance () != NULL)
	{
		FileFactoryListClass::Get_Instance ()->Reset_Search_Start();
	}

	//
	//	Is this a mix file?
	//
	if (::strcmpi (extension, ".mix") == 0) {
		
		StringClass thumb_filename(root_name,true);
		thumb_filename+=".thu";
		ThumbnailManagerClass::Add_Thumbnail_Manager(thumb_filename,filename);

		//
		//	Build the dynamic data filename from mix file's root name
		//
		filename_to_load.Format ("%s.ldd", root_name);
		lsd_filename .Format ("%s.lsd", root_name);

		//
		//	HACK HACK - Put the level 9 mix file first...
		//
		if (	::stricmp (filename, "M09.mix") == 0 &&
				FileFactoryListClass::Get_Instance () != NULL)
		{
			FileFactoryListClass::Get_Instance ()->Set_Search_Start(filename);
		}

	} else if (::strcmpi (extension, ".lsd") == 0) {		
		lsd_filename = filename;
		filename_to_load.Format ("%s.ldd", root_name);
	} else {
		
		//
		//	Dig out the name of the map we'll use with this file
		//
		StringClass map_name(0,true);
		if (Peek_Map_Name (filename, map_name)) {

			char mix_root_name[_MAX_FNAME] = { 0 };
			::_splitpath ((const char *)map_name, NULL, NULL, mix_root_name, NULL);

			//
			//	Build the mix filename from the map name...
			//
			StringClass mix_filename(0, true);
			lsd_filename.Format ("%s.lsd", mix_root_name);
			mix_filename.Format ("%s.mix", mix_root_name);

			//
			//	HACK HACK - Put the level 9 mix file first...
			//
			if (	::stricmp (mix_filename, "M09.mix") == 0 &&
					FileFactoryListClass::Get_Instance () != NULL)
			{
				FileFactoryListClass::Get_Instance ()->Set_Search_Start(mix_filename);
			}

			StringClass thumb_filename(mix_root_name,true);
			thumb_filename+=".thu";
			ThumbnailManagerClass::Add_Thumbnail_Manager(thumb_filename,mix_filename);
		}
	}

	return ;
}

void	SaveGameManager::Load_Game( const char * filename )
{
	WWLOG_PREPARE_TIME_AND_MEMORY("Load_Game");

	WWMEMLOG(MEM_GAMEDATA);
	Debug_Say(( "Load Game %s\n", filename ));
	CurrentGameFilename = filename;

	FileClass * file = _TheFileFactory->Get_File( filename );
	WWASSERT( file );
	file->Open( FileClass::READ );
	ChunkLoadClass cload(file);

	WWLOG_INTERMEDIATE("Open file");
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_LEVEL_INFO:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						
						READ_MICRO_CHUNK_WWSTRING( cload,	MICROCHUNKID_MAP_FILENAME,			MapFilename );
						READ_MICRO_CHUNK( cload,			MICROCHUNKID_MISSION_DESCRIPTION,	MissionDescriptionID );
						READ_MICRO_CHUNK_WIDESTRING( cload, MICROCHUNKID_DESCRIPTION,			Description );

						default:
							Debug_Say(( "Unrecognized Level Info chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}


				{
				// Load level specific Defs
				StringClass temp_ddb(MapFilename,true);
				WWASSERT( temp_ddb.Get_Length() > 4 );
				temp_ddb.Erase( MapFilename.Get_Length()-4, 4 );
				temp_ddb	+= ".ddb";
				Load_Definitions(temp_ddb);
				}
				WWLOG_INTERMEDIATE("Load_Definitions");

				// Load the static data
				Load_Level();	
				WWLOG_INTERMEDIATE("Load_Level");
				
				break;
								
			case CHUNKID_LEVEL_DATA:
				if (CombatManager::I_Am_Server()) {
					SaveLoadSystemClass::Load( cload, false );
				}
				WWLOG_INTERMEDIATE("Load");
				break;

			default:
				Debug_Say(( "Unrecognized Level chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	file->Close();
	_TheFileFactory->Return_File(file);
	WWLOG_INTERMEDIATE("Rest of the stuff");
}


bool	SaveGameManager::Smart_Peek_Description
(
	const char *		filename,
	WideStringClass &	description,
	WideStringClass &	mission_name
)
{
	//
	//	Get the root name and extension from the filename
	//
	char root_name[_MAX_FNAME] = { 0 };
	char extension[_MAX_EXT] = { 0 };
	::_splitpath (filename, NULL, NULL, root_name, extension);

	StringClass filename_to_load(filename,true);

	//
	//	Is this a mix file?
	//
	FileFactoryClass * mix_factory = NULL;
	if (::strcmpi (extension, ".mix") == 0) {		
		
		//
		// Configure a mix file factory for this mix file
		//
		Debug_Say(( "Adding Temp MIX file factory %s\n", filename ));
		if ( FileFactoryListClass::Get_Instance() != NULL ) {
			mix_factory = new MixFileFactoryClass( filename, _TheFileFactory );
			FileFactoryListClass::Get_Instance()->Add_FileFactory( mix_factory, filename );
		}

		//
		//	Build the dynamic data filename from mix file's root name
		//
		filename_to_load.Format ("%s.ldd", root_name);
	}

	//
	//	Peek at the information inside this mix file...
	//
	bool retval = Peek_Description (filename_to_load, description, mission_name);

	//
	//	Remove the temporary mix file factory we added
	//
	if (mix_factory != NULL) {
		FileFactoryListClass::Get_Instance()->Remove_FileFactory(mix_factory);
		delete mix_factory;
		mix_factory = NULL;
	}

	return retval;
}


bool SaveGameManager::Peek_Description
(
	const char *		filename,
	WideStringClass &	description,
	WideStringClass &	mission_name
)
{
	//
	//	Open the file as a chunk
	//
	FileClass * file = _TheFileFactory->Get_File(filename);
	WWASSERT(file != NULL);
	file->Open(FileClass::READ);
	ChunkLoadClass cload(file);

	bool retval			= false;
	int mission_name_id	= 0;
	StringClass map_filename(0,true);
	
	//
	//	Loop until we've found the header chunk
	//
	while (retval == false && cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_LEVEL_INFO:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						
						//
						//	Read the header chunks
						//
						READ_MICRO_CHUNK_WWSTRING( cload,	MICROCHUNKID_MAP_FILENAME,			map_filename );
						READ_MICRO_CHUNK( cload,			MICROCHUNKID_MISSION_DESCRIPTION,	mission_name_id );
						READ_MICRO_CHUNK_WIDESTRING( cload,	MICROCHUNKID_DESCRIPTION,			description );

					}
					cload.Close_Micro_Chunk();
				}
				retval = true;
				break;
		}
		cload.Close_Chunk();
	}

	//
	//	Either load the mission name from the translation database
	//	or simply return the map filename
	//
	if (mission_name_id == 0) {
		mission_name.Convert_From ( map_filename );
		unichar_t *extension = (unichar_t *)::u_strrchr (mission_name, U_CHAR('.'));
		if (extension != NULL) {
			extension[0] = 0;
		}
	} else {
		mission_name = TRANSLATE(mission_name_id);
	}

	//
	//	Close the file
	//
	file->Close();
	_TheFileFactory->Return_File(file);

	return retval;
}

bool SaveGameManager::Peek_Map_Name( const char * filename, StringClass &map_name )
{
	//
	//	Open the file as a chunk
	//
	FileClass * file = _TheFileFactory->Get_File(filename);
	WWASSERT(file != NULL);
	file->Open(FileClass::READ);
	ChunkLoadClass cload(file);

	bool retval = false;
	
	//
	//	Loop until we've found the header chunk
	//
	while (retval == false && cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_LEVEL_INFO:
				while (retval == false && cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						
					//
					//	Read the map name string from chunk	
					//
					case MICROCHUNKID_MAP_FILENAME:
						LOAD_MICRO_CHUNK_WWSTRING( cload, map_name );
						retval = true;
						break;
					}
					cload.Close_Micro_Chunk();
				}
				break;
		}
		cload.Close_Chunk();
	}

	//
	//	Close the file
	//
	file->Close();
	_TheFileFactory->Return_File(file);

	return retval;
}

/*
**
*/
void	SaveGameManager::Save_Level( void )
{
	Debug_Say(( "Save Level %s\n", MapFilename ));
	Save_Save_Load_System(	MapFilename,	
									&_PhysStaticDataSaveSystem, 
									&_PhysStaticObjectsSaveSystem,
									&_StaticAudioSaveLoadSubsystem,
									&_TheBackgroundMgr,
									&_TheWeatherMgr,
									&_TheMapMgrSaveLoadSubsystem,
									NULL );
}

void	SaveGameManager::Load_Level( void )
{
	Debug_Say(( "Load Level %s\n", MapFilename ));
	Load_Save_Load_System( MapFilename, false );	// false = no automatic post load processing (needs to be called explicitly)
}

/*
**
*/
void	SaveGameManager::Save_Definitions( const char * filename )
{
	Debug_Say(( "Save Definitions %s\n", filename ));
	Save_Save_Load_System( filename, &_TheDefinitionMgr, NULL );
}

void	SaveGameManager::Load_Definitions( const char * filename )
{
	WWMEMLOG(MEM_GAMEDATA);
	Debug_Say(( "Load Definitions %s\n", filename ));
	Load_Save_Load_System( filename, true );	// true = automatic post load processing
}

/*
**
*/
void __cdecl SaveGameManager::Save_Save_Load_System( const char * filename, ... )
{
	FileClass * file = _TheWritingFileFactory->Get_File( filename );
	WWASSERT(file);
	file->Open(FileClass::WRITE);
	ChunkSaveClass csave(file);

	va_list arg_list;
	va_start( arg_list, filename );
	bool done = false;
	while ( !done ) {
		SaveLoadSubSystemClass * sub_system = va_arg( arg_list, SaveLoadSubSystemClass * );
		if ( sub_system != NULL ) {
			SaveLoadSystemClass::Save( csave, *sub_system );
		} else {
			done = true;
		}
	}
	va_end (arg_list);

	file->Close();
	_TheWritingFileFactory->Return_File(file);
}

void	SaveGameManager::Load_Save_Load_System( const char * filename, bool auto_post_load )
{
	FileClass * file = _TheFileFactory->Get_File( filename );
	if ( file != NULL ) {
		file->Open( FileClass::READ );
		ChunkLoadClass cload(file);
		SaveLoadSystemClass::Load( cload, auto_post_load );
		file->Close();
		_TheFileFactory->Return_File(file);
	} else {
		Debug_Say(( "Failed to load file %s\n", filename ));
//		WWASSERT( file );
	}
}

