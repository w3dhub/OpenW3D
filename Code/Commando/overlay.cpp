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
 *                     $Archive:: /Commando/Code/commando/overlay.cpp                         $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/02/01 5:50p                                               $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "overlay.h"
#include "scene.h"
#include "camera.h"
#include "wwprofile.h"
#include "ww3d.h"
#include "dx8renderer.h"
#include "textwindow.h"
#include "debug.h"
#include "assets.h"
#include "timemgr.h"
#include "hanim.h"
#include "input.h"
#include "gameinitmgr.h"
#include "renegadedialogmgr.h"

/*
**
*/
SimpleSceneClass  * OverlayGameModeClass::Scene;
CameraClass			* OverlayGameModeClass::Camera;

void	OverlayGameModeClass::Init()
{
	Scene = new SimpleSceneClass;
#ifdef WW3D_COMPILE_WITH_DX8__
	Scene->Get_DX8_Renderer_Container()->Enable_Lighting(false);
#endif
	Scene->Set_Ambient_Light(Vector3(1,1,1));

	Camera = new CameraClass();
	Camera->Set_Position( Vector3( 0, 0, 640 ) );
	Camera->Set_View_Plane( Vector2( -0.5f, -0.375f ), Vector2( 0.5f, 0.375f ) );
	Camera->Set_Clip_Planes(620,620*1000);

	TextWindowClass::Initialize(Scene);
}

void 	OverlayGameModeClass::Shutdown()
{
	TextWindowClass::Shutdown();

	if ( Scene != NULL ) {
		Scene->Release_Ref();
		Scene = NULL;
	}

	if ( Camera != NULL ) {
		Camera->Release_Ref();
		Camera = NULL;
	}
}

void 	OverlayGameModeClass::Render()
{
	WWPROFILE( "Overlay Render" );
	WW3D::Render( Scene, Camera );
}



/*
**
*/
SimpleSceneClass  * Overlay3DGameModeClass::Scene;
CameraClass			* Overlay3DGameModeClass::Camera;
RenderObjClass		* Overlay3DGameModeClass::Model;
float					  Overlay3DGameModeClass::ModelTimer;
bool					  Overlay3DGameModeClass::EndScreen;

void	Overlay3DGameModeClass::Init()
{
	Scene = new SimpleSceneClass;
	Scene->Set_Ambient_Light(Vector3(1,1,1));

	Camera = new CameraClass();
	Camera->Set_Position (Vector3 (0, 0, 800));
	Camera->Set_View_Plane (Vector2 (-0.5f, -0.375f), Vector2 (0.5f, 0.375f));
	Camera->Set_Clip_Planes (620, 620 * 1000);

	Model = NULL;
}

void 	Overlay3DGameModeClass::Shutdown()
{
	Release_Model();

	if ( Scene != NULL ) {
		Scene->Release_Ref();
		Scene = NULL;
	}

	if ( Camera != NULL ) {
		Camera->Release_Ref();
		Camera = NULL;
	}
}

void 	Overlay3DGameModeClass::Render()
{
	if ( Model != NULL ) {
		WW3D::Render( Scene, Camera );
	}
}

void 	Overlay3DGameModeClass::Release_Model( void )
{
	if (Model != NULL) {
		Model->Remove ();
	}
	REF_PTR_RELEASE (Model);
}

void 	Overlay3DGameModeClass::Set_Model( const char * name, const char * anim_name )
{
	Release_Model();

	if ( name != NULL ) {
		Model = WW3DAssetManager::Get_Instance ()->Create_Render_Obj (name);

		if (Model != NULL) {
			Scene->Add_Render_Object (Model);

			if ( anim_name != NULL ) {
				HAnimClass *anim = WW3DAssetManager::Get_Instance ()->Get_HAnim (anim_name);
				if (anim != NULL) {
					Model->Set_Animation (anim, 0, RenderObjClass::ANIM_MODE_ONCE);
					REF_PTR_RELEASE (anim);
				}
			}
		}
	}
}

void 	Overlay3DGameModeClass::Start_Intro( void )
{
	Set_Model( "e32001_fade-in", NULL );
	ModelTimer = 15;
	EndScreen = false;
}

void 	Overlay3DGameModeClass::Start_End_Screen( void )
{
	Set_Model( "endscreen", "endscreen.endscreen" );
	ModelTimer = 50.0f/15.0f + 3;
	EndScreen = true;
}

void 	Overlay3DGameModeClass::Think()
{
	if ( ModelTimer > 0 ) {
		ModelTimer -= TimeManager::Get_Frame_Seconds();
		if ( ModelTimer <= 0 ) {
			ModelTimer = 0;
			Release_Model();

			if ( EndScreen ) {
				GameInitMgrClass::End_Game();
				RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
			}
		}
	}
}

