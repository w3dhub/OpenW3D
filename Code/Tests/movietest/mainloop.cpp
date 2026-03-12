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

/* $Header: /Commando/Code/Tests/movietest/mainloop.cpp 4     5/06/98 3:10p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tests/movietest/mainloop.cpp                 $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/24/98 10:44a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "mainloop.h"
#include "mono.h"
#include "msgloop.h"
#include "wwfile.h"
#include "rawfile.h"
#include "_globals.h"
#include "_viewpt.h"
#include <sr.hpp>
#include <assert.h>
#include "assetmgr.h"
#include "sr_util.h"
#include "dynamesh.h"
#include "material.h"
#include "world.h"
#include "_world.h"
#include "rendobj.h"
#include "r2dobj.h"
#include "r3dobj.h"
#include "mesh.h"
#include "hmodel.h"
#include "light.h"
#include "wwdebug.h"
#include "ww3d.h"


/*
** Globals
*/

WW3DAssetManager The3DAssetManager;

CameraClass *			Camera;
MeshClass *				Mesh;
LightClass *			Light;
LightClass *			Light2;

HModelClass *			TestModel = NULL;
HAnimClass *			TestAnim = NULL;
HAnimClass *			CameraAnim = NULL;

Quaternion				ViewOrientation(1);
Quaternion				ViewMotion(1);
Vector3					ViewCenter(0,0,0);
float						ViewDist = 55.0f;

/*
** Local functions
*/

void  Render(void);
void	Create_Scene(void);
void	Create_Objects(void);
void	Destroy_Scene(void);
void	Destroy_Objects(void);
void	Load_Data(void);
void	Render_Scene(void);
void	Time_Step(void);
void	Init_Debug(void);
void	Shutdown_Debug(void);
void	wwdebug_message_handler(const char * message);
void	wwdebug_assert_handler(const char * message);
bool	wwdebug_trigger_handler(int trigger_num);
void	Debug_Refs(void);


/*
** delay for time milliseconds
*/
void	Wait( int time )
{
	int start = SystemTimer();
	while ( 1000 * ( SystemTimer() - start ) < ( time * SYSTEM_TIMER_RATE ) ) ;
}


/*
** MAIN GAME LOOP
*/
void Main_Loop(void)
{
	Init_Debug();
	Create_Scene();
	Load_Data();
	Create_Objects();

	while (!Keyboard->Down(VK_ESCAPE)) {

		Time_Step();
		Render();
		Windows_Message_Handler();

//		if (Keyboard->Down(VK_F1)) {
//			while(Keyboard->Down(VK_F1));
//			TheWorld.Next_Render_Device();
//		}

	}

	Destroy_Objects();
	WW3DAssetManager::Get_Instance()->Free_Assets();
	Destroy_Scene();
	Shutdown_Debug();
	Debug_Refs();
}

void Render(void)
{
	WW3D::Begin_Render(true,Vector3(0,0,0.5));
	WW3D::Render(TheScene,Camera);
	WW3D::End_Render();
}

void Create_Scene(void)
{
//	TheWorld.Init_Scene(ScreenResolution,8,1);
	TheScene = new SimpleSceneClass;
}

/*
** Load initial game data
*/
void Load_Data(void)
{
	WW3DAssetManager::Get_Instance()->Load_3D_Assets(RawFileClass("CAMERA.W3D"));
	WW3DAssetManager::Get_Instance()->Load_3D_Assets(RawFileClass("MOVIE.W3D"));
}

/*
**
*/
void	Create_Objects(void)
{
	Camera = new CameraClass("CAMERA");
//	TheWorld.Set_Camera(Camera);
//	Camera->Set_Depth(5000.0f);
//	Camera->Set_Clip_Planes(1.0f,5000.0f);
	Camera->Set_Focal_Length(0.060f);
	Camera->Set_Viewport(Vector2(0.3,0.3),Vector2(0.5,0.5));

	CameraAnim = WW3DAssetManager::Get_Instance()->Get_HAnim("Camera.Camera");
	TestModel = WW3DAssetManager::Get_Instance()->Create_HModel("Movie");
	assert(TestModel);
	TestAnim = WW3DAssetManager::Get_Instance()->Get_HAnim("Movie.Movie");
//	TestModel->Add(&TheWorld);
	TestModel->Add(TheScene);

	Light = new LightClass();
	Light->Set_Transform(Matrix3D(1));
//	Light->Add(&TheWorld);
	Light->Add(TheScene);
}


/*
**
*/
void Destroy_Scene(void)
{
//	TheWorld.Set_Camera(NULL);
//	TheWorld.Shutdown_Scene();
	delete TheScene;
	TheScene = NULL;
}


/*
**
*/
void	Destroy_Objects(void)
{
	if (Mesh) {
		Mesh->Release_Ref();
		Mesh = NULL;
	}

	if (TestModel) {
		TestModel->Remove();
		TestModel->Release_Ref();
		TestModel = NULL;
	}

	if (Camera) {
		Camera->Release_Ref();
		Camera = NULL;
	}

	if (Light) {
		Light->Remove();
		Light->Release_Ref();
		Light = NULL;
	}

	if (Light2) {
		Light2->Remove();
		Light2->Release_Ref();
		Light2 = NULL;
	}
}


void Time_Step(void)
{
	static int _frame = 0;
	static int _camframe = 0;
	static float _t = 0.0f;
	static float _dt = 0.01f;

	_t += _dt;
	if (_t > 1.0f) {
		_t = 1.0f;
		_dt = -0.01f;
	}
	if (_t < 0.0f) {
		_t = 0.0f;
		_dt = 0.01f;
	}

	if (Mesh) {
		Mesh->Apply_Damage(0,_t);
	}

	if (TestAnim) {
		_frame++;
		if (_frame >= TestAnim->Get_Num_Frames()) {
			_frame = 0;
		}

		TestModel->Set_Transform(Matrix3D(1));
	  	TestModel->Set_Animation(TestAnim,_frame);
	} else {
		TestModel->Set_Transform(Matrix3D(1));
		TestModel->Set_Animation();
	}

	if (CameraAnim) {
		_camframe++;
		if (_camframe >= CameraAnim->Get_Num_Frames()) {
			_camframe = 0;
		}

		Camera->Set_Transform(Matrix3D(1));
		Camera->Set_Animation(CameraAnim,_camframe);
	} else {
		Matrix3D cam(1);
		cam.Translate(ViewCenter);
		cam = cam * Build_Matrix3D(ViewOrientation);
		cam.Translate(Vector3(0,0,ViewDist));
	 	Camera->Set_Transform(cam);
	}
}


void	Init_Debug(void)
{
	/*
	** Install message handler functions for the WWDebug messages
	** and assertion failures.
	*/
	WWDebug_Install_Message_Handler(wwdebug_message_handler);
	WWDebug_Install_Assert_Handler(wwdebug_assert_handler);
	WWDebug_Install_Trigger_Handler(wwdebug_trigger_handler);

}

void	Shutdown_Debug(void)
{
	/*
	** Remove message handler functions for the WWDebug messages
	** and assertion failures.
	*/
	WWDebug_Install_Message_Handler(NULL);
	WWDebug_Install_Assert_Handler(NULL);
	WWDebug_Install_Trigger_Handler(NULL);
}

void wwdebug_message_handler(const char * message)
{
	/*
	** Hand the message off to the scrolling debug screen
	*/
//	Debug_Say((message));
}

void wwdebug_assert_handler(const char * message)
{
	/*
	** Hand the message off to the scrolling debug screen
	*/
//	Debug_Say((message));

	/*
	** break into the debugger
	*/
	_asm int 0x03;
}

bool wwdebug_trigger_handler(int trigger_num)
{
	return Keyboard->Down(trigger_num);
}

void Debug_Refs(void)
{
#ifndef NDEBUG
	char buf[1024];

	if (RefCountClass::Total_Refs() > 0) {
		sprintf(buf,"Main Looop End %d refs\n", RefCountClass::Total_Refs());
		MessageBox(NULL,buf,"Ref Debugging",MB_OK);
	}

	SList<ActiveRefStruct> * reflist = RefCountClass::Get_Active_Ref_List();

	SLNode<ActiveRefStruct> * objnode;

	for (	objnode = reflist->Head(); objnode; objnode = objnode->Next()) {

		ActiveRefStruct * ref = objnode->Data();

		sprintf(buf,"Active Ref: %s Line: %d Pointer: %p\n", ref->File,ref->Line,ref->Object);
		if (MessageBox(NULL,buf,"Ref Debugging",MB_OKCANCEL) == IDCANCEL) {
			break;
		}
	}
#endif
}
