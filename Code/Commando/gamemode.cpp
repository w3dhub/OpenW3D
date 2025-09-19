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
 *                     $Archive:: /Commando/Code/Commando/gamemode.cpp                        $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/02/02 2:27a                                               $*
 *                                                                                             *
 *                    $Revision:: 61                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gamemode.h"
#include "wwprofile.h"
#include "ww3d.h"
#include "wwdebug.h"
#include "miscutil.h"
#include "slist.h"
//#include "menu.h"
#include "textdisplay.h"
#include "thread.h"
#include <stdio.h>
#include "backgroundmgr.h"
#include "render2d.h"
#include "dazzle.h"
#include "combat.h"
#include "meshmdl.h"
#include "rinfo.h"
#include "objectives.h"
#include "messagewindow.h"
#include "dialogmgr.h"
#include "pscene.h"
#include "phys.h"
#include "ccamera.h"
#include "diagnostics.h"
#include "dx8wrapper.h"
#include "sortingrenderer.h"
#include "textureloader.h"
#include "BINKMovie.h"
//#include "helptext.h"
#include "cnetwork.h"
#include "dx8rendererdebugger.h"
#include "ConsoleMode.h"


/*
** The gamemode list
*/
SList<GameModeClass>	  	GameModeList;
Vector3						GameModeManager::BackgroundColor(0,0,0);

int GameMajorModeClass::NumActiveMajorModes = 0;
static unsigned _HiddenFrameCount=0;

/*
** make sure we only have 1 active majormode
*/
void GameMajorModeClass::Activate()
{
	if ( State == GAME_MODE_INACTIVE ) {
		NumActiveMajorModes++;
		assert ( NumActiveMajorModes == 1 );
	}
	GameModeClass::Activate();
}

void GameMajorModeClass::Deactivate()
{
	if ( !Is_Inactive() ) {
		NumActiveMajorModes--;
		assert ( NumActiveMajorModes == 0 );
	}
	GameModeClass::Deactivate();
}

/*
**
*/
void	GameModeManager::Destroy( GameModeClass *mode )
{
	assert( mode != NULL );

	mode->Deactivate();
	mode->Safely_Deactivate();
	Remove( mode );
	delete mode;
}


/*
**
*/
GameModeClass *GameModeManager::Add( GameModeClass *mode )
{
	GameModeList.Add_Tail( mode );
	return mode;
}

void	GameModeManager::Remove( GameModeClass *mode )
{
	GameModeList.Remove( mode );
}

int	GameModeManager::Count()
{
	return GameModeList.Get_Count();
}

/*
**
*/
void	GameModeManager::Destroy_All( void )
{
	while ( Count() ) {
		Destroy( GameModeList.Remove_Tail() );
	}
}

void GameModeManager::List_Active_Game_Modes(void)
{
	if (!Get_Text_Display()) {
		return;
	}
	WWASSERT(Get_Text_Display() != NULL);

	Get_Text_Display()->Print_System("Active game modes:");

	for (SLNode<GameModeClass> * game_mode_node = GameModeList.Head();
		game_mode_node != NULL; game_mode_node = game_mode_node->Next()) {
		GameModeClass * p_mode = game_mode_node->Data();
		WWASSERT(p_mode != NULL);

		if (p_mode->Is_Active()) {
			Get_Text_Display()->Print_System( "  %s", p_mode->Name() );
			//Debug_Say((mode_str));
		}
	}
}



/*
** let all non inactive game modes think
*/
void	GameModeManager::Think( void )
{
	for (	SLNode<GameModeClass> *game_mode_node = GameModeList.Head();
			game_mode_node != NULL;
			game_mode_node = game_mode_node->Next()) {
		GameModeClass *mode = game_mode_node->Data();

		//if ( mode->Get_State() != GAME_MODE_INACTIVE &&
		//	  mode->Get_State() != GAME_MODE_INACTIVE_PENDING ) {
		//if ( mode->Get_State() != GAME_MODE_INACTIVE &&
		if ( !mode->Is_Inactive() ) {

//			char name[80];
//			sprintf( name, "Think - %s", mode->Name() );
			mode->Think();
		}

		mode->Safely_Deactivate(); // if required
	}
	BINKMovie::Update();
}

void GameModeManager::Safely_Deactivate(void)
{
	WWPROFILE( "Deactivate" );
	//
	// This method safely deactivates any inactive pending mode without
	// attempting a think
	//

	for (SLNode<GameModeClass> *game_mode_node = GameModeList.Head();
		game_mode_node != NULL; game_mode_node = game_mode_node->Next()) {

		GameModeClass * p_mode = game_mode_node->Data();
		WWASSERT(p_mode != NULL);
		p_mode->Safely_Deactivate(); // if required
	}
}

/*
** let all non inactive game modes draw
*/
void	GameModeManager::Render( void )
{
	WWPROFILE( "Render" );

	if (!ConsoleBox.Is_Exclusive()) {

		bool clear=true;
		bool old_enable_draw=DX8Wrapper::_Is_Triangle_Draw_Enabled();
		bool old_enable_sorting_draw=SortingRendererClass::_Is_Triangle_Draw_Enabled();
		if (_HiddenFrameCount) {
			DX8Wrapper::_Enable_Triangle_Draw(false);
			SortingRendererClass::_Enable_Triangle_Draw(false);
			clear=false;
		}

		// Update the mesh debugger. This doesn't do anything at all unless the debugger is
		// enabled, so it is safe to always call it
		DX8RendererDebugger::Update();

		bool do_pscene = (COMBAT_SCENE != NULL) && !cNetwork::I_Am_Only_Server();
		if (!GameInFocus) do_pscene=false;	// Don't render the game scene if the applicationisn't active
		if (do_pscene) {

			//
			//	Don't pre-process the combat scene (does VIS and stuff) if
			// the game isn't active.  (This gives us a menu performance boost).
			//
			if (Find( "Combat" )->Is_Active()) {
				COMBAT_SCENE->Pre_Render_Processing(*COMBAT_CAMERA);
			}
		}

		{
			WWPROFILE( "Begin_Render" );

			WW3D::Begin_Render (clear, clear, BackgroundMgrClass::Get_Clear_Color());
		}

		if (GameInFocus) {
			WWPROFILE( "Render Game Modes" );
			for (	SLNode<GameModeClass> *game_mode_node = GameModeList.Head();
					game_mode_node != NULL;
					game_mode_node = game_mode_node->Next()) {
				GameModeClass *mode = game_mode_node->Data();

				if ( mode->Get_State() != GAME_MODE_INACTIVE ) {
					mode->Render();
				}
			}
		}

		{
			WWPROFILE( "Message Window" );
			if (CombatManager::Get_Message_Window () != NULL) {
				CombatManager::Get_Message_Window ()->Render();
			}
		}

		{
			WWPROFILE( "ObjectiveViewer" );
			ObjectiveManager::Render_Viewer();
		}

		{
			WWPROFILE( "DialogMgr" );
			DialogMgrClass::Render();
		}

		{
			WWPROFILE( "cDiagnostics" );
			cDiagnostics::Render();
		}

		/*
		{
			WWPROFILE( "cHelpText" );
			cHelpText::Render();
		}
		*/

		// Only update the movie when the application is active
		if (GameInFocus) {
			WWPROFILE( "BINK" );
			BINKMovie::Render();
		}

		{
			WWPROFILE( "End_Render" );
			WW3D::End_Render();
		}


		if (do_pscene) {
			COMBAT_SCENE->Post_Render_Processing();
		}

		{
			WWPROFILE( "Switch_Thread" );
			ThreadClass::Switch_Thread(); // This is important when working with multiple threads!
		}

		if (_HiddenFrameCount) {
			_HiddenFrameCount--;
			DX8Wrapper::_Enable_Triangle_Draw(old_enable_draw);
			SortingRendererClass::_Enable_Triangle_Draw(old_enable_sorting_draw);
			TextureLoader::Flush_Pending_Load_Tasks();
		}
	}
}

void GameModeManager::Hide_Render_Frames(unsigned frame_count)
{
	_HiddenFrameCount=frame_count;
}

/*
** find a registered game mode by name
*/
GameModeClass	*GameModeManager::Find( const char * name )
{
	for (	SLNode<GameModeClass> *game_mode_node = GameModeList.Head();
			game_mode_node != NULL;
			game_mode_node = game_mode_node->Next()) {
		GameModeClass *mode = game_mode_node->Data();

		if ( !openw3d::string_compare( name, mode->Name() ) ) {
			return mode;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
void GameModeClass::Activate()
{
	if (State == GAME_MODE_INACTIVE) {
		Init();
		State = GAME_MODE_ACTIVE;
	}

	if (State == GAME_MODE_INACTIVE_PENDING) {
		State = GAME_MODE_ACTIVE;
	}
}

//-----------------------------------------------------------------------------
void GameModeClass::Deactivate()
{
	if (!Is_Inactive()) {
		State = GAME_MODE_INACTIVE_PENDING;
	}
}

//-----------------------------------------------------------------------------
void GameModeClass::Safely_Deactivate()
{
	if (State == GAME_MODE_INACTIVE_PENDING) {
		Shutdown();
		State = GAME_MODE_INACTIVE;
	}
}

//-----------------------------------------------------------------------------
void GameModeClass::Suspend()
{
	if (State == GAME_MODE_ACTIVE) {
		State = GAME_MODE_SUSPENDED;
	}
}

//-----------------------------------------------------------------------------
void GameModeClass::Resume()
{
	if (State == GAME_MODE_SUSPENDED) {
		State = GAME_MODE_ACTIVE;
	}
}