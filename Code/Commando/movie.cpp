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
 *                     $Archive:: /Commando/Code/Commando/movie.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 2/25/02 11:46a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 31                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "movie.h"
#include "BINKMovie.h"
#include "campaign.h"
#include "input.h"
#include "_globals.h"
#include "registry.h"
#include "renegadedialogmgr.h"
#include "wwmemlog.h"
#include "gameinitmgr.h"
#include "WWAudio.h"
#include "specialbuilds.h"
#include "stylemgr.h"
#include "render2dsentence.h"
#include "ffactory.h"
#include "wwfile.h"

enum {
	STARTUP_MOVIE_OFF,
	STARTUP_MOVIE_EA,
	STARTUP_MOVIE_INTRO,
};

int MovieStartupMode = STARTUP_MOVIE_OFF;
bool	IntroMovieSkipAllowed = false;
bool	SkipAllIntroMovies = false;

void	MovieGameModeClass::Init()
{
	WWMEMLOG(MEM_BINK);
	BINKMovie::Init();
	IsPending = false;
	IsPlaying = false;

	RegistryClass registry( APPLICATION_SUB_KEY_NAME_OPTIONS );
	if ( registry.Is_Valid() ) {
		IntroMovieSkipAllowed = registry.Get_Bool( "IntroMovieSkipAllowed", false );
		SkipAllIntroMovies = registry.Get_Bool( "SkipAllIntroMovies", false );
		registry.Set_Bool( "SkipAllIntroMovies", SkipAllIntroMovies );
	}

}

void 	MovieGameModeClass::Shutdown()
{
	BINKMovie::Shutdown();
	IsPending = false;
	IsPlaying = false;
}

/*
** called each time through the main loop
*/
void 	MovieGameModeClass::Think()
{
	WWMEMLOG(MEM_BINK);
	if ( Is_Active() ) {
		BINKMovie::Update();

		if ( IsPending == false && BINKMovie::Is_Complete() ) {
			Movie_Done();
			return;
		}

		if (( MovieStartupMode == STARTUP_MOVIE_INTRO ) && 
			( IntroMovieSkipAllowed == false )) {
			return;
		}

		bool	leave = Input::Get_State(INPUT_FUNCTION_MENU_TOGGLE);
		static bool was_leave = true;
		if ( leave && !was_leave ) {
			was_leave = leave;		// Weird.  Double looping calls
			Movie_Done();
		}
		was_leave = leave;
	}
}

void 	MovieGameModeClass::Render()
{
	WWMEMLOG(MEM_BINK);
	if ( Is_Active() ) {

		if (IsPlaying) {
			if (BINKMovie::Is_Complete () == false) {
				BINKMovie::Render ();
			} else {
				WWAudioClass::Get_Instance ()->Temp_Disable_Audio (false);
				BINKMovie::Stop ();
				IsPlaying = false;
			}
		}
	}
}

void	MovieGameModeClass::Start_Movie( const char * filename )
{
	WWMEMLOG(MEM_BINK);
	
	//
	//	Check to see if we should enforce the CD or not...
	//
	bool force_cd = true;

#ifdef WWDEBUG
	RegistryClass registry( APPLICATION_SUB_KEY_NAME_DEBUG );
	if ( registry.Is_Valid() ) {
		force_cd = (registry.Get_Int( "DisableCDCheck", 0 ) == 0);
	}
#endif //WWDEBUG

#if defined(BETACLIENT) || defined(FREEDEDICATEDSERVER) || defined(MULTIPLAYERDEMO)
	force_cd = false;
#endif //BETACLIENT

	//
	//	Play the movie (if it exists locally)
	//
	FileClass *file = _TheFileFactory->Get_File(filename);
	if (file->Is_Available()) {
		Play_Movie ( filename );
	} else {

		//
		//	Strip any path information off the filename
		//
		StringClass filename_only( filename, true );
		const char *delimiter = ::strrchr( filename, '\\' );
		if ( delimiter != NULL ) {
			filename_only = delimiter + 1;
		}

		//
		//	Try to find the CD...
		//
		StringClass cd_path;
		if ( CDVerifier.Get_CD_Path( cd_path ) ) {
			
			//
			//	Build a full-path to the movie on the CD
			//
			StringClass full_path = cd_path;
			if ( cd_path[cd_path.Get_Length () - 1] != '\\' ) {
				full_path += "\\";
			}
			full_path += filename_only;
			Play_Movie( full_path );

		} else if ( force_cd ) {
			PendingMovieFilename	= filename_only;
			IsPending				= true;
			CDVerifier.Display_UI( this );
		}
	}
}

void	MovieGameModeClass::Play_Movie( const char * filename )
{
	WWAudioClass::Get_Instance ()->Temp_Disable_Audio (true);
	
	FontCharsClass* font = StyleMgrClass::Get_Font(StyleMgrClass::FONT_INGAME_SUBTITLE_TXT);

	BINKMovie::Play( filename, "data\\subtitle.ini", font );

	if (font) {
		font->Release_Ref();
	}

	IsPlaying = true;
	return ;
}

void	MovieGameModeClass::HandleNotification (CDVerifyEvent &event)
{
	if ( event.Event() == CDVerifyEvent::VERIFIED ) {

		//
		//	Get the path to the CD...
		//
		StringClass cd_path;
		if ( CDVerifier.Get_CD_Path( cd_path ) ) {
			
			//
			//	Build a full-path to the movie on the CD
			//
			StringClass full_path = cd_path;
			if ( cd_path[cd_path.Get_Length () - 1] != '\\' ) {
				full_path += "\\";
			}
			full_path += PendingMovieFilename;
			Play_Movie( full_path );
		}		
	} else if ( event.Event() == CDVerifyEvent::NOT_VERIFIED ) {			

		if ( MovieStartupMode == STARTUP_MOVIE_EA || MovieStartupMode == STARTUP_MOVIE_INTRO) {
			MovieStartupMode = STARTUP_MOVIE_OFF;
			
			// Goto main menu
			RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
			Deactivate();

		} else {
			Movie_Done();
		}
	}

	PendingMovieFilename = "";
	IsPending = false;
	return ;
}


void	MovieGameModeClass::Startup_Movies( void )
{
	if ( SkipAllIntroMovies ) {
		MovieStartupMode = STARTUP_MOVIE_INTRO;
		MovieGameModeClass::Movie_Done();
	} else {
		MovieStartupMode = STARTUP_MOVIE_EA;
		Start_Movie( "MOVIES\\EA_WW.BIK" );		// Play WW/EA movie
	}
}

void	MovieGameModeClass::Movie_Done( void )
{
	if (IsPlaying) {
		WWAudioClass::Get_Instance ()->Temp_Disable_Audio (false);
		BINKMovie::Stop ();
		IsPlaying = false;
	}

	if ( MovieStartupMode == STARTUP_MOVIE_EA ) {
		MovieStartupMode = STARTUP_MOVIE_INTRO;
		Start_Movie( "MOVIES\\R_INTRO.BIK" );		// Play Renegade intro movie
	} else if ( MovieStartupMode == STARTUP_MOVIE_INTRO ) {
		MovieStartupMode = STARTUP_MOVIE_OFF;
		// Goto main menu
		
#ifdef MULTIPLAYERDEMO
		RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_SPLASH_IN);
#else
		RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
#endif //MULTIPLAYERDEMO

		IntroMovieSkipAllowed = true;

		RegistryClass registry( APPLICATION_SUB_KEY_NAME_OPTIONS );
		if ( registry.Is_Valid() ) {
			registry.Set_Bool( "IntroMovieSkipAllowed", true );
		}

		Deactivate();
	} else {
		CampaignManager::Continue();
	}
}
