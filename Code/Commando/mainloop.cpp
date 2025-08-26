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
 *                     $Archive:: /Commando/Code/Commando/mainloop.cpp                        $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                        $*
 *                                                                                             *
 *                     $Modtime:: 2/21/02 3:13p                                               $*
 *                                                                                             *
 *                    $Revision:: 77                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "mainloop.h"
#include "init.h"
#include "shutdown.h"
#include "timemgr.h"
#include "input.h"
#include "gamemode.h"
#include "debug.h"
#include "msgloop.h"
#include "wwprofile.h"
#include "cnetwork.h"
#include "miscutil.h"
//#include "gamesettings.h"
#include "WWAudio.h"
#include "devoptions.h"
#include "multihud.h"
#include "gamedata.h"
#include "diagnostics.h"
#include "wwprofile.h"
#include "crandom.h"
#include "dialogmgr.h"
#include "ccamera.h"
#include "pathmgr.h"
#include "networkobjectmgr.h"
#include "WebBrowser.h"
#include "AutoStart.h"
#include "gameinitmgr.h"
#include "servercontrol.h"
#include "ConsoleMode.h"
#include "gamespyadmin.h"
#include "demosupport.h"
#include "GameSpy_QnR.h"


/*
**
*/
bool	RunMainLoop = true;
int		ExitCode = EXIT_SUCCESS;

void Stop_Main_Loop(int exitCode)
{
	RunMainLoop = false;
	ExitCode = exitCode;
}


void _Game_Main_Loop_Loop(void)
{
	WWPROFILE( "Main Loop" );

	unsigned long time1 = TIMEGETTIME();

   TimeManager::Update();

   Input::Update();


{	WWPROFILE( "Pathfind Evaluate" );
   if (COMBAT_CAMERA != NULL) {
		Vector3 camera_pos = COMBAT_CAMERA->Get_Position();
		PathMgrClass::Resolve_Paths( camera_pos );
	}
}

{	WWPROFILE( "Think" );
   GameModeManager::Think();
	GameInitMgrClass::Think();
}

{	WWPROFILE( "Dialog Mgr Update" );
   DialogMgrClass::On_Frame_Update ();
}

{	WWPROFILE( "Network Object Mgr Think" );
   NetworkObjectMgrClass::Think ();
	ServerControl.Service();
}

{	WWPROFILE("GameSpy_QnR");
	GameSpyQnR.Think();
}

	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		WWPROFILE( "cGameSpyAdmin Think" );
		cGameSpyAdmin::Think();
	}

	//
	// If the following assert hits it may indicate that your
	// working directory pathname got cleared in the project settings.
	//
	WWASSERT(GameModeManager::Find("Combat") != NULL);

	if (!GameModeManager::Find("Combat")->Is_Active()) {
		cNetwork::Update();
	}

	// Denzil - Embedded browser
	if (WebBrowser::IsWebPageDisplayed() == false) {
		GameModeManager::Render();
	}

	if (AutoRestart.Is_Active()) {
		AutoRestart.Think();
	}

{	WWPROFILE("ConsoleBox");
	ConsoleBox.Think();
}

	DEMO_SECURITY_CHECK;

{	WWPROFILE( "Audio" );
	if (!ConsoleBox.Is_Exclusive()) {
		WWAudioClass::Get_Instance ()->On_Frame_Update (0);
	}
}
	// Give the sound manager a chance to think
  // PROFILE(	"Audio", WWAudioClass::Get_Instance ()->On_Frame_Update (0) );

   Windows_Message_Handler();
#ifdef WWDEBUG
   // Sometimes it is useful to be able to artificially lower the frame rate
   Sleep(cDevOptions::DesiredFrameSleepMs.Get());
#endif

#if 0
{	WWPROFILE( "Random" );
	// spin the Random Generator, a little
	int count = FreeRandom.Get_Int( 5 );
	while ( count-- > 0 ) {
		FreeRandom.Get_Int();
	}
}
#endif

   DebugManager::Update();


	/*
	** Sleep for a while if we are hogging the CPU.
	*/
	if (cNetwork::I_Am_Only_Server()) {
		unsigned long time2 = TIMEGETTIME();
		if (time2 >= time1) {

			/*
			** 16 (approx) for 60 fps. (1000/60)
			*/
			unsigned long diff = time2 - time1;
			if (diff < 16) {
				unsigned long sleep_time = 16 - (time2 - time1);
				Sleep(sleep_time);
			}
		}
	}
}

/*
** MAIN GAME LOOP
*/
int Game_Main_Loop(void)
{
	const unsigned long servicetime = 1000; // Time in milliseconds.

	unsigned long time;

	// Only run main loop if the init is succesful!
	if (Game_Init()) {
		while ( RunMainLoop ) {
			_Game_Main_Loop_Loop();
		}

		// IML: Allow a short period to process any outstanding sound effects before shutdown.
		time = TIMEGETTIME();
		while (TIMEGETTIME() - time < servicetime) {
			WWAudioClass::Get_Instance ()->On_Frame_Update (0);
		}

		Game_Shutdown();
	}

	return ExitCode;
}