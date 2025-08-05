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
 *                     $Archive:: /Commando/Code/Commando/init.cpp                            $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/16/02 11:10a                                             $*
 *                                                                                             *
 *                    $Revision:: 258                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Commando_Assert_Handler -- Commando callback function for WWASSERT's                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "init.h"
#include "debug.h"
#include "wwmath.h"
#include "ww3d.h"
#include "wwphys.h"
#include "WWAudio.H"
#include "wwsaveload.h"
#include "input.h"
#include "inputconfigmgr.h"
//#include "gamesettings.h"
#include "ffactory.h"
#include "assets.h"
#include "_globals.h"
#include "combatgmode.h"	// gamemode
#include "langmode.h"		// gamemode
#include "wolgmode.h"		// gamemode
#include "movie.h"			// gamemode
#include "gamemenu.h"		// gamemode
#include "overlay.h"			// gamemode
#include "multihud.h"		// gamemode
#include "console.h"			// gamemode
#include "textdisplay.h"	// gamemode
#include "scorescreen.h"	// gamemode
#include "msgloop.h"
#include "Resource.H"
#include "miscutil.h"
#include "cnetwork.h"
#include "mathutil.h"
#include "win.h"
#include "Part_Ldr.H"
#include "savegame.H"
#include	"systemsettings.h"
#include "gamedata.h"
#include "sphereobj.h"
#include "ringobj.h"
#include "diagnostics.h"
#include "devoptions.h"
#include "translatedb.h"
#include "useroptions.h"
#include "pathmgr.h"
#include "string_ids.h"
#include "soundrobj.h"
#include "crandom.h"
#include "systimer.h"
#include "renegadedialogmgr.h"
#include "campaign.h"
#include "diaglog.h"
#include "mpsettingsmgr.h"
#include "mixfile.h"
#include "ffactorylist.h"
#include "gameinitmgr.h"
#include "serverfps.h"
#include "nicenum.h"
#include "encyclopediamgr.h"
#include "texturethumbnail.h"
#include "playermanager.h"
#include "teammanager.h"
#include "stackdump.h"
#include "registry.h"
#include "bandwidthgraph.h"
#include "buildnum.h"
#include "dx8wrapper.h"
#include "autostart.h"
#include "except.h"
#include "wwmemlog.h"
#include "except.h"
#include "consolemode.h"
#include "serversettings.h"
#include "pscene.h"
#include "dazzle.h"
#include "skinpackagemgr.h"
#include "modpackagemgr.h"
#include "GameSpy_QnR.h"
#include "gamespyadmin.h"
#include "shutdown.h"
#include "specialbuilds.h"

extern const char *VALUE_NAME_TEXTURE_FILTER_MODE;

/*
** Define IMMEDIATE_LOAD if you want to have the game start right
** up in a given level (for profiling for example)
*/

// HACK: ATI demo loads the demo level bypassing the mainmenu
#ifdef ATI_DEMO_HACK
#define IMMEDIATE_LOAD						1
#define IMMEDIATE_LOAD_LEVELNAME			"MDD_0807.mix"
#else
#define IMMEDIATE_LOAD						0
#define IMMEDIATE_LOAD_LEVELNAME			"MDD_0803.mix"
#endif




/*
** This defines the subdirectory where the game will load all data from
*/
const char *	DATA_SUBDIRECTORY			= "DATA\\";
const char *	SAVE_SUBDIRECTORY			= "DATA\\SAVE\\";
const char *	CONFIG_SUBDIRECTORY		= "DATA\\CONFIG\\";
const char *	MOVIES_SUBDIRECTORY		= "DATA\\MOVIES\\";


#define	STRINGS_FILENAME					"STRINGS.TDB"
#define	CONV_DB_FILENAME					"CONV10.CDB"

/*
** DebugDisplayHandler
*/
TextDebugDisplayHandlerClass				TextDisplayHandler;

/*
** Used to modify where game entries are kept in the registry.
*/
extern char DefaultRegistryModifier[1024];

/*
** Static global lod settings for particles
*/
static const float _ParticleLODScreenSizes[17] =
{
	0.0025f,
	0.0050f,
	0.0075f,
	0.0100f,
	0.0300f,
	0.0500f,
	0.0700f,
	0.0900f,
	0.1000f,
	0.3000f,
	0.5000f,
	1.0000f,
	1.2500f,
	1.5000f,
	1.7500f,
	2.0000f,
	NO_MAX_SCREEN_SIZE
};


/***********************************************************************************************
 * Append_To_Assert_History --  keeps a timestamped history of asserts.                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/26/2001  tss : Created.                                                                 *
 *=============================================================================================*/
void Append_To_Assert_History(const char * message)
{
	if (message == NULL) {
		//
		// bail
		//
		return;
	}

	FILE * file = ::fopen("_asserts.txt", "at");
   if (file == NULL) {
		return;
	}

	char line[2000] = "";

	//
	// Blank lines plus timestamp
	//
	::sprintf(line, "\n\nWhen:       %s\n", cMiscUtil::Get_Text_Time());
	::fwrite(line, 1, ::strlen(line), file);

	//
	// version #
	//
	::sprintf(line, "Version:    %d\n", DebugManager::Get_Version_Number());
	::fwrite(line, 1, ::strlen(line), file);

	//
	// Full filename
	//
	char full_filename[MAX_PATH];
	::GetModuleFileName(NULL, full_filename, sizeof(full_filename));
	::sprintf(line, "Filename:   %s\n", full_filename);
	::fwrite(line, 1, ::strlen(line), file);

	//
	// File size
	//
	HANDLE hfile = ::CreateFile(full_filename, 0, 0, NULL, OPEN_EXISTING, 0L, NULL);
	if (hfile != INVALID_HANDLE_VALUE)
	{
		DWORD file_size = ::GetFileSize(hfile, NULL);
		::CloseHandle(hfile);
		::sprintf(line, "Filesize:   %d\n", file_size);
		::fwrite(line, 1, ::strlen(line), file);
	}

	//
	// The assert message itself
	//
	::sprintf(line, "Assert:     %s\n", message);
	::fwrite(line, 1, ::strlen(line), file);

	::fclose(file);
}


/***********************************************************************************************
 * Commando_Assert_Handler -- Commando callback function for WWASSERT's                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/28/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void Commando_Assert_Handler(const char * message)
{
	if (message == NULL) {
		//
		// bail
		//
		return;
	}

	//
	// TSS092601
	//
	Append_To_Assert_History(message);

	//
	// If the exception handler is try to quit the game then don't show an assert.
	//
	if (Is_Trying_To_Exit()) {
		ExitProcess(0);
	}

	//
	// Hand the message off to the scrolling debug screen
	//
	DebugManager::Display( message );

	//
	//
	//    IF YOU WANT TO DISABLE CERTAIN ACTIONS HERE, USE
	//    THE REGISTRY SWITCHES!!!!!!!!!!!!!
	//
	//
#ifdef WWDEBUG
	Copy_Logs(DebugManager::Get_Version_Number());
#endif // WWDEBUG
	RegistryClass registry( APPLICATION_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		registry.Set_Int( VALUE_NAME_APPLICATION_CRASH_VERSION, 0 );

	}

#ifdef WWDEBUG

   if (cDevOptions::SoundEffectOnAssert.Is_True() &&
		WWAudioClass::Get_Instance() != NULL) {
		//
		// Sound effect
		//
		WWAudioClass::Get_Instance()->Create_Instant_Sound("Debug_Assertion", Matrix3D(1));
   }

   if (cDevOptions::DisplayLogfileOnAssert.Is_True()) {
		//
		// Open up the logfile so that the user can see the assert
		// at the bottom.
		//
		cNetwork::Shell_Command(DebugManager::Logfile_Name());
	}

	//LOG_CALL_STACK;

	if (cDevOptions::BreakToDebuggerOnAssert.Is_True()) {

		if (cDevOptions::ShutdownInputOnAssert.Is_True()) {
			//
			// This input shutdown is to help those of us who still have the
			// DirectInput / debugger problem. For some of us the registry fix
			// doesn't help.
			//
			Input::Shutdown();
		}
																																			/*

									  m
			$m                mm            m
			 "$mmmmm        m$"    mmmmmmm$"
					 """$m   m$    m$""""""
			mmmmmmm$$$$$$$$$"mmmm
		  mmm$$$$$$$$$$$$$$$$$$ m$$$$m  "    m  "
		$$$$$$$$$$$$$$$$$$$$$$  $$$$$$"$$$
		 mmmmmmmmmmmmmmmmmmmmm  $$$$$$$$$$
		 $$$$$$$$$$$$$$$$$$$$$  $$$$$$$"""  m
		 "$$$$$$$$$$$$$$$$$$$$$ $$$$$$  "      "
			  """""""$$$$$$$$$$$m """"
				 mmmmmmmm"  m$   "$mmmmm
			  $$""""""      "$     """"""$$
			m$"               "m           "
                       																												*/

		_asm int 0x03;
	}

	if (cDevOptions::ExitThreadOnAssert.Is_True()) {
      ExitThread(1);
   }

#endif // WWDEBUG

}

/*
**
*/
void _stdcall AudioTextCallback(AudibleSoundClass *sound_obj, const StringClass &text, uint32 user_param)
{
	Vector3 red = Vector3( 1, 0.5f, 0.5f );
	StringClass str;
	str.Format( "%s\n", (const char *)text );
	DebugManager::Display_Text( str, red );
}

/*
**
*/
class LoggingFileFactoryClass : public SimpleFileFactoryClass {
public:
	virtual FileClass * Get_File( const char * filename ) {
		FileClass * file = BaseFactory->Get_File( filename );
		Debug_Say(( "FILE_LOG : %s %s\n", filename, ((file==NULL||!file->Is_Available())?"MISSING":"") ));
		return file;
	}
	void Set_Base_Factory( FileFactoryClass * factory ) { BaseFactory = factory; }
private:
	FileFactoryClass * BaseFactory;
};

/*
**
*/
class StrippingFileFactoryClass : public SimpleFileFactoryClass {
public:
	virtual FileClass * Get_File( const char * filename ) {
		StringClass stripped(true);
		Strip_Path_From_Filename( stripped, filename );
		return BaseFactory->Get_File( stripped );
	}
	void Set_Base_Factory( FileFactoryClass * factory ) { BaseFactory = factory; }
private:
	FileFactoryClass * BaseFactory;
};

/*
**
*/
SimpleFileFactoryClass	RenegadeWritingFileFactory;

SimpleFileFactoryClass	RenegadeBaseFileFactory;
MixFileFactoryClass	*	AlwaysMixFileFactory;
FileFactoryListClass		_RenegadeFileFactory;
StrippingFileFactoryClass	AudioFileFactory;
LoggingFileFactoryClass		LoggingFileFactory;

/*
**
*/
void	Construct_Directory_Structure(void)
{
	//
	//	Lookup the path of the executable
	//
	char path[MAX_PATH] = { 0 };
	::GetModuleFileName (NULL, path, sizeof (path));

	//
	//	Strip off the filename
	//
	char *filename = ::strrchr (path, '\\');
	if (filename != NULL) {
		filename[1] = 0;
	}

	StringClass data_dir(path,true);
	data_dir += "data";

	StringClass save_dir(data_dir + "\\save",true);
	StringClass config_dir(data_dir + "\\config",true);

	//
	//	Create the data directory if necessary
	//
	if (GetFileAttributes (data_dir) == 0xFFFFFFFF) {
		::CreateDirectory (data_dir, NULL);
	}

	//
	//	Create the save directory if necessary
	//
	if (GetFileAttributes (save_dir) == 0xFFFFFFFF) {
		::CreateDirectory (save_dir, NULL);
	}

	//
	//	Create the config directory if necessary
	//
	if (GetFileAttributes (config_dir) == 0xFFFFFFFF) {
		::CreateDirectory (config_dir, NULL);
	}


	return ;
}

static bool Verify_Log_Directory(const StringClass& folder)
{
	if (GetFileAttributes(folder)!=0xffffffff) return true;
	//HANDLE file;
	//file = CreateFile(folder, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//if (file!=INVALID_HANDLE_VALUE) {
	//	CloseHandle(file);
	//	return true;
	//}

	if (CreateDirectory(folder,NULL)) {
		return true;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		return(true);
	}
	return false;
}

static bool Create_Log_File_Name(const StringClass& folder, StringClass& filename,bool use_numbering)
{
	StringClass original(filename);
	if (!use_numbering) {
		filename.Format("%s\\%s",folder,original);
		return true;
	}
	for (int i=0;i<999;++i) {
		HANDLE file;
		filename.Format("%s\\%3.3d%s",folder,i,original);
		file = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file!=INVALID_HANDLE_VALUE) {
			CloseHandle(file);
			return true;
		}
	}
	return false;
}

static void Copy_Log(const StringClass& folder,const char* filename,bool use_numbering)
{
	RawFileClass raw_log_file(filename);
	if (raw_log_file.Is_Available()) {
		int size=raw_log_file.Size();
		if (size) {
			StringClass log_file_name(filename);
			if (Create_Log_File_Name(folder,log_file_name,use_numbering)) {
				DWORD written;
				HANDLE file;
				file = CreateFile(log_file_name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (INVALID_HANDLE_VALUE != file) {
					raw_log_file.Open();
					unsigned char* memory=new unsigned char[size];
					raw_log_file.Read(memory,size);
					raw_log_file.Close();
					WriteFile(file, memory, size, &written, NULL);
					delete[] memory;
					CloseHandle(file);
				}
			}
		}
	}
}

static class CopyThreadClass : public ThreadClass
{
public:
	unsigned Version;

	CopyThreadClass()
		:
		Version(0),
		ThreadClass("LogCopyThread", &Exception_Handler) {}

	void Thread_Function()
	{
		// Write log to network folder

		char computer_name[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = sizeof(computer_name);
		::GetComputerName(computer_name, &size);

		RegistryClass reg(APPLICATION_SUB_KEY_NAME_DEBUG);
		char path[MAX_PATH];
		reg.Get_String("LogPath", path, sizeof(path), "\\\\tanya\\game\\projects\\renegade\\_error_logs");
		strcat(path, "\\");

		StringClass folder_name(0,true);
		folder_name.Format("%s%d.%d",path,Version>>16,Version&0xffff);
		if (!Verify_Log_Directory(folder_name)) return;
		folder_name+="\\";
		folder_name+=computer_name;
		if (!Verify_Log_Directory(folder_name)) return;

		Copy_Log(folder_name,DebugManager::Logfile_Name(),true);		//"_logfile.txt",true);
		Copy_Log(folder_name,"_asserts.txt",true);
		Copy_Log(folder_name,"_except.txt",true);
		Copy_Log(folder_name,"sysinfo.txt",false);
	}
} CopyThread;

void Copy_Logs(unsigned version)
{
	RegistryClass registry( APPLICATION_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		if (registry.Get_Int( VALUE_NAME_DISABLE_LOG_COPYING,0 )) return;
	}

	if (CopyThread.Is_Running()) return;
	CopyThread.Version=version;
	CopyThread.Execute();

	int time=TIMEGETTIME();
	while (TIMEGETTIME()-time<5000) {
		if (!CopyThread.Is_Running()) {
			break;
		}
		Sleep(100);
	}
}

void Application_Exception_Callback(void)
{
#ifdef WWDEBUG
	Copy_Logs(DebugManager::Get_Version_Number());
#endif // WWDEBUG
	RegistryClass registry( APPLICATION_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		registry.Set_Int( VALUE_NAME_APPLICATION_CRASH_VERSION, 0 );
	}
}

bool RestartNeeded = true;


/***********************************************************************************************
 * Get_Version_Number -- Collate version information.                                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to var to fill with major version number                                      *
 *           Ptr to var to fill with minor version number                                      *
 *                                                                                             *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/3/2001 11:26PM ST : Created                                                            *
 *=============================================================================================*/
void Get_Version_Number(unsigned long *major, unsigned long *minor)
{
	// Version info removed per Legal review requirements. LFeenanEA - 8th February 2025
	
	unsigned long version_major = 0;
	unsigned long version_minor = 0;
}


/*
**
*/
#if 0
#define	FIRST_CHAR	'a'
#define	LAST_CHAR	'z'

#include "realcrc.h"
int	CRC_Next( unsigned char ** p, int length ) 
{
	int ret = 0;
	if ( length == -1 ) {
		return 1;
	}
	if ( **p == LAST_CHAR ) {
		*p = *p-1;
		ret |= CRC_Next( p, length-1 );
		*p = *p+1;
		**p = FIRST_CHAR;
	} else {
		**p = **p+1;
	}
	return ret;

}

void CRC_Check( void ) 
{
	Debug_Say(( "CRC_Check\n" ));
	int count = 0;

#define	MAX_STRING	10
#define	GOAL_CRC		65729409

	int start = timeGetTime();

	unsigned char string[MAX_STRING+1];
	for ( int length = 1; length <= MAX_STRING; length++ ) 
	{
		unsigned char * p = &string[length-1];
		string[length] = 0;
		for ( int i = 0; i < length; i++ ) {
			string[i] = FIRST_CHAR;
		}

		while ( 1 ) {
/*			if ( (count & 0x0FFFF) == 0 ) {
				Debug_Say(( "Checking %d \"%s\"\n", count, string ));
			}*/
			int crc = CRC_String( (char *)string );
			if ( crc == GOAL_CRC ) {
				Debug_Say(( "CRC MATCH \"%s\" (count %d) at %d\n\n", string, count, timeGetTime()-start ));
			}
			count++;
			if ( CRC_Next( &p, length-1 ) != 0 ) {
				break;
			}
		}
Debug_Say(( "End length %d (count %d) at %d\n", length, count, timeGetTime()-start ));
	}

//	strcpy( string, "miketheheadlesschickenisgod" );
//	Debug_Say(( "CRC %d\n", crc ));
	return;
}
#endif
/*
**
*/
bool Game_Init(void)
{
	WWMEMLOG(MEM_GAMEINIT);

	// Set registry key to 1 for the duration of the init. This way we know if the program crashed while the init.
	RegistryClass registry( APPLICATION_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		registry.Set_Int( VALUE_NAME_GAME_INITIALIZATION_IN_PROGRESS, 1 );
		unsigned crash_version=registry.Get_Int( VALUE_NAME_APPLICATION_CRASH_VERSION, 0 );
#ifdef WWDEBUG
		if (crash_version) Copy_Logs(crash_version);
#endif // WWDEBUG
		registry.Set_Int( VALUE_NAME_APPLICATION_CRASH_VERSION, 0 );
	}

	//
	//	Ensure our directory structure exists
	//
	Construct_Directory_Structure ();

	//
	//	Initialize our debugging framework
	//
	DebugManager::Init();
  	DebugManager::Load_Registry_Settings( APPLICATION_SUB_KEY_NAME_DEBUG );
	WWDebug_Install_Assert_Handler(Commando_Assert_Handler);
	BuildInfoClass::Log_Build_Info();

//CRC_Check();

	Get_Version_Number(NULL, NULL);

	// setup Writing Factory
	RenegadeWritingFileFactory.Set_Sub_Directory( DATA_SUBDIRECTORY );
	_TheWritingFileFactory = &RenegadeWritingFileFactory;

	RenegadeBaseFileFactory.Set_Sub_Directory( DATA_SUBDIRECTORY );
	RenegadeBaseFileFactory.Append_Sub_Directory( SAVE_SUBDIRECTORY );
	RenegadeBaseFileFactory.Append_Sub_Directory( CONFIG_SUBDIRECTORY );

	_TheSimpleFileFactory->Set_Sub_Directory( DATA_SUBDIRECTORY );
	_TheSimpleFileFactory->Append_Sub_Directory( SAVE_SUBDIRECTORY );
	_TheSimpleFileFactory->Append_Sub_Directory( CONFIG_SUBDIRECTORY );

	_TheSimpleFileFactory->Set_Strip_Path( true );

	_RenegadeFileFactory.Add_FileFactory( &RenegadeBaseFileFactory, "" );
	_RenegadeFileFactory.Add_FileFactory( new MixFileFactoryClass( "Always2.dat", &RenegadeBaseFileFactory ), "Always2.dat" );
	_RenegadeFileFactory.Add_FileFactory( new MixFileFactoryClass( "Always.dbs", &RenegadeBaseFileFactory ), "Always.dbs" );
	_RenegadeFileFactory.Add_FileFactory( new MixFileFactoryClass( "Always.dat", &RenegadeBaseFileFactory ), "Always.dat" );

	//
	//	Search for all mix files in the data directory
	//
	WIN32_FIND_DATA find_info	= { 0 };
	BOOL keep_going				= TRUE;
	HANDLE file_find				= NULL;
	for (file_find = ::FindFirstFile ("data\\*.mix", &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (file_find, &find_info))
	{
		//
		//	Add this mix file to our mix file factory list
		//
		_RenegadeFileFactory.Add_FileFactory( new MixFileFactoryClass (find_info.cFileName, &RenegadeBaseFileFactory ), find_info.cFileName );
	}

	//
	//	Close the search handle
	//
	if (file_find != INVALID_HANDLE_VALUE) {
		::FindClose (file_find);
	}

	_TheFileFactory = &_RenegadeFileFactory;

	// Logging File Factory
	if ( DebugManager::Is_File_Logging_Enabled() ) {
		LoggingFileFactory.Set_Base_Factory( &_RenegadeFileFactory );
		_TheFileFactory = &LoggingFileFactory;
	}

	AudioFileFactory.Set_Base_Factory( _TheFileFactory );

	//Setup_Mix_File();

	// Lets seed the Random Generator, a little
	int count = TIMEGETTIME() & 0xFF;
	while ( count-- > 0 ) {
		FreeRandom.Get_Int();
	}

	// Thumbnail manager pre init will ensure that thumbnail database
	// is up-to-date.
	bool show_thumbnail_pre_init_dialog = true;
#ifdef WWDEBUG
	show_thumbnail_pre_init_dialog = cDevOptions::ShowThumbnailPreInitDialog.Get();
#endif // WWDEBUG
//	ThumbnailManagerClass::Pre_Init(show_thumbnail_pre_init_dialog);

	//
	// Create an instance of the sound library
	//
	new WWAudioClass(ConsoleBox.Is_Exclusive());
	WWAudioClass::Get_Instance()->Initialize( APPLICATION_SUB_KEY_NAME_SOUND );
	WWAudioClass::Get_Instance()->Set_File_Factory( &AudioFileFactory );
	// Install text callback
	WWAudioClass::Get_Instance()->Register_Text_Callback(AudioTextCallback,0);

	//
	//	Load the multiplayer settings
	//
	MPSettingsMgrClass::Load_Settings ();

	/*
	**
	*/
	WW3DAssetManager * asset_manager = new WW3DAssetManager;
//	asset_manager->Open_Texture_File_Cache("cache_");
	asset_manager->Set_WW3D_Load_On_Demand( true );
	asset_manager->Register_Prototype_Loader(&_ParticleEmitterLoader);
	asset_manager->Register_Prototype_Loader(&_SphereLoader);
	asset_manager->Register_Prototype_Loader(&_RingLoader);
	asset_manager->Register_Prototype_Loader (&_SoundRenderObjLoader);
	asset_manager->Set_Activate_Fog_On_Load (true);

	//GameSettings::Init();

	// Initialize WWMath
	WWMath::Init();

	// Initialize the pathfind system
	PathMgrClass::Initialize ();

	// Initialize WW3D
	switch ( WW3D::Init(MainWindow, NULL, ConsoleBox.Is_Exclusive() ? true : false)) {
	case WW3D_ERROR_OK:	// Success!
		break;
	case WW3D_ERROR_DIRECTX8_INITIALIZATION_FAILED:
	default:
		WWDEBUG_SAY(("WW3D::Init Failed!\r\n"));
		::MessageBox(NULL,
			"DirectX 8.0 or later is required to play C&C:Renegade.",
			"Renegade Graphics Initialization Error.",
			MB_OK);
		return false;
	}

	if (ConsoleBox.Is_Exclusive()) {
		WW3D::Enable_Decals(false);
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
		scene->Set_Max_Simultaneous_Shadows(0);
		DazzleRenderObjClass::Enable_Dazzle_Rendering(false);
	} else {
		if ( WW3D::Registry_Load_Render_Device( APPLICATION_SUB_KEY_NAME_RENDER, true ) != WW3D_ERROR_OK ) {
			WWDEBUG_SAY(("WW3D::Registry_Load_Render_Device Failed!\r\n"));
			return false;
		}

		if ( WW3D::Registry_Save_Render_Device( APPLICATION_SUB_KEY_NAME_RENDER ) != WW3D_ERROR_OK ) {
			WWDEBUG_SAY(("WW3D::Registry_Save_Render_Device Failed!\r\n"));
			return false;
		}
		WW3D::Enable_Static_Sort_Lists (true);
	}
	if (AutoRestart.Get_Restart_Flag() || ServerSettingsClass::Is_Command_Line_Mode() || ConsoleBox.Is_Exclusive()) {
		if (!ConsoleBox.Is_Exclusive()) {
			::ShowWindow( MainWindow, SW_MINIMIZE );	// minimize if we are starting automatically.
		}
		ConsoleBox.Init();
	} else {
		::ShowWindow( MainWindow, SW_SHOW );	// show the (initially hidden) window
	}

	// Clear screen
	for (int frame=0;frame<3;++frame) {
		WW3D::Begin_Render(true,true,Vector3(0.0f,0.0f,0.0f));
		WW3D::End_Render();
	}

	ParticleEmitterClass::Set_Default_Remove_On_Complete(false);	// (gth) 09/17/2000 - by default emitters shouldn't self-destruct

	for (int i=0; i<17; i++) {
		ParticleBufferClass::Set_LOD_Max_Screen_Size(i,_ParticleLODScreenSizes[i]);
	}

	if ( cUserOptions::PermitDiagLogging.Is_True() &&
		  DebugManager::Is_Diag_Logging_Enabled() ) {

		DiagLogClass::Init();
	}

	WWPhys::Init();
	WWSaveLoad::Init();

	//
	//	Load the strings table
	//
	TranslateDBClass::Initialize();
	FileClass *file	= _TheFileFactory->Get_File( STRINGS_FILENAME );
	if (file != NULL) {
		file->Open (FileClass::READ);				//	Open or the file
		if ( file->Is_Available() ) {
			ChunkLoadClass cload (file);				// Load the database
			SaveLoadSystemClass::Load(cload);
		}
		file->Close ();								// Close the file
		_TheFileFactory->Return_File (file);
	}
	//TranslateDBClass::Set_Current_Language (TranslateDBClass::LANGID_CHINESE);

	//
	//	Initialize the input control system
	//
	bool dinput_avail = (ConsoleBox.Is_Exclusive()) ? false : true;

	Input::Init(dinput_avail);
	Input::Load_Registry( APPLICATION_SUB_KEY_NAME_CONTROLS );
	InputConfigMgrClass::Initialize();

	//
	//	Initialize the skin selection framework
	//
	SkinPackageMgrClass::Initialize ();
	ModPackageMgrClass::Initialize ();
	ModPackageMgrClass::Build_List ();

	//
	//	Load the conversation database
	//
	file	= _TheFileFactory->Get_File( CONV_DB_FILENAME );
	if (file != NULL) {
		file->Open (FileClass::READ);				//	Open or the file
		if ( file->Is_Available() ) {
			ChunkLoadClass cload (file);				// Load the database
			SaveLoadSystemClass::Load(cload);
		}
		file->Close ();								// Close the file
		_TheFileFactory->Return_File (file);
	}

	//
	//	Check to make sure the code version matches the strings
	// table version
	//
	if (TranslateDBClass::Get_Version_Number () != STRINGS_VER) {
		MessageBox( 0,
			"This build of Renegade is out of sync with the strings database (strings.tdb).  Strings will be incorrect and may cause the game to crash.",
			"Version Error",
			MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND  );
	}

	//
	//	Note:  Due to interdependencies (yuck!) between these subsystems, here's the
	// order they need to be initialized in:
	//
	//		- Physics Scene Class (via CombatManager::Scene_Init)
	//		- SystemSettings
	//		- RenegadeDialogMgrClass
	//
	//
	CombatManager::Scene_Init();
	if (!ConsoleBox.Is_Exclusive()) {
		SystemSettings::Init();
	}
   RenegadeDialogMgrClass::Initialize();

	cNetwork::Onetime_Init();

	cServerFps::Create_Instance();

	cPlayerManager::Onetime_Init();
	cTeamManager::Onetime_Init();
	cGameData::Onetime_Init();
	cBandwidthGraph::Onetime_Init();

   cNetUtil::Wsa_Init();

	CombatManager::Init(ConsoleBox.Is_Exclusive() ? false : true);

	CampaignManager::Init();

	// This order is also draw and think order
	GameModeManager::Add( new CombatGameModeClass );
	GameModeManager::Add( new LanGameModeClass );
	GameModeManager::Add( new WolGameModeClass );
	GameModeManager::Add( new OverlayGameModeClass );
	GameModeManager::Add( new MenuGameModeClass2 );		// Putting menu after overlay so we see the text
	GameModeManager::Add( new MovieGameModeClass );
	GameModeManager::Add( new ConsoleGameModeClass );
	GameModeManager::Add( new ScoreScreenGameModeClass );
	//GameModeManager::Add( new LobbyGameModeClass );
	GameModeManager::Add( new TextDisplayGameModeClass );
	GameModeManager::Add( new Overlay3DGameModeClass );

	GameModeManager::Find( "Overlay3D" )->Activate();
	GameModeManager::Find( "Overlay" )->Activate();
	GameModeManager::Find( "Console" )->Activate();
	GameModeManager::Find( "Menu" )->Activate();
	GameModeManager::Find( "TextDisplay" )->Activate();

	// After TextDisplay is created, install the Display Handler
	DebugManager::Set_Display_Handler(&TextDisplayHandler);

	//DEADMENU MenuManager::Set_Menu( "Menu_Main" );

	// Load the accelerator table and hand it off to WWLIB.
	// Note:  Accelerator tables that are loaded from resources (like
	// we are doing here) do not need to be manually freed.  Windows
	// will cleanup for us when the process terminates.
	HACCEL haccel = ::LoadAccelerators (::GetModuleHandle (NULL), MAKEINTRESOURCE (IDR_ACCELERATOR));
	if (haccel) {
		::Add_Accelerator (MainWindow, haccel);
	}

	//WW3D::Set_Texture_Reduction( 1 );

	//
	//	Initialize the encyclopedia logic
	//
	EncyclopediaMgrClass::Initialize ();

#if (IMMEDIATE_LOAD)
	// Immediately load up the specified level
	GameInitMgrClass::Initialize_SP ();
	GameInitMgrClass::Start_Game (IMMEDIATE_LOAD_LEVELNAME);
#endif

	cDiagnostics::Init();
	//cHelpText::Init();

	//
	// NIC initialization
	//
	cNicEnum::Init();



	if ( registry.Is_Valid() ) {
		registry.Set_Int( VALUE_NAME_GAME_INITIALIZATION_IN_PROGRESS, 0 );
		registry.Set_Int( VALUE_NAME_APPLICATION_CRASH_VERSION, DebugManager::Get_Version_Number() );
	}


#if (IMMEDIATE_LOAD==0)

#if 0	// Not anymore
	//
	//	Start the main menu
	//
	RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
#else


	//
	// Parse the server settings files if they will be used soon to make sure there are no errors.
	//
	if (ServerSettingsClass::Is_Command_Line_Mode()) {
		if (!ServerSettingsClass::Parse(false)) {
			AutoRestart.Set_Restart_Flag(false);
			Game_Shutdown();
			return (false);
		}
	}

	//
	// If this is a post crash restart (or a FDS starting up) then just go straight into the game.
	//
	if (AutoRestart.Get_Restart_Flag()) {
		AutoRestart.Restart_Game();
	} else if (cGameSpyAdmin::Is_Gamespy_Game()) {
		if (!ConsoleBox.Is_Exclusive()) {
			RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_SPLASH_IN);
		}
	} else {
		MovieGameModeClass * mode = (MovieGameModeClass *)GameModeManager::Find ("Movie");
		if ( mode ) {
			mode->Activate();
			mode->Startup_Movies( );
		}
	}

#endif


#endif


	//
	// Send our Spy Usage Info off to Gamespy
	//
	GameSpyQnR.TrackUsage();

	return true;
}








/***********************************************************************************************
 * Build_Registry_Location_String -- Get a complete path to a registry location                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Game base path                                                                    *
 *           Base modifier path                                                                *
 *           Sub path                                                                          *
 *                                                                                             *
 * OUTPUT:   Ptr to complete path                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2001 3:39PM ST : Created                                                             *
 *=============================================================================================*/
char *Build_Registry_Location_String(const char *base, const char *modifier, const char *sub)
{
	static char _whole_registry_string[1024];

	WWASSERT(base != NULL);
	WWASSERT(sub != NULL);


	if (modifier == NULL) {
		modifier = DefaultRegistryModifier;
	}

	if (base && *base != 0) {
		strcpy(_whole_registry_string, base);
	}
	if (modifier && *modifier != 0) {
		strcat(_whole_registry_string, "\\");
		strcat(_whole_registry_string, modifier);
	}
	if (sub && *sub != 0) {
		strcat(_whole_registry_string, "\\");
		strcat(_whole_registry_string, sub);
	}
	return(_whole_registry_string);
}