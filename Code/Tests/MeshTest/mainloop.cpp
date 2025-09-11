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

/* $Header: /Commando/Code/Tests/MeshTest/mainloop.cpp 47    12/10/98 5:53p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tests/MeshTest/mainloop.cpp                  $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 12/07/98 1:30p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 47                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define NOMINMAX

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
#include "_scenes.h"
#include "rendobj.h"
#include "r3dobj.h"
#include "mesh.h"
#include "hmodel.h"
#include "light.h"
#include "wwdebug.h"
#include "ww3d.h"


#define SPIN_MODEL
#define SPIN_LIGHT
//#define CAPTURE_BONE
//#define TEST_DAMAGE
//#define TEST_TEXTURE_ANIMATION

/*
** Globals
*/

WW3DAssetManager The3DAssetManager;

CameraClass *			Camera;
LightClass *			Light;
LightClass *			Light2;

RenderObjClass *		TestModel = NULL;
HAnimClass *			TestAnim = NULL;

float						CameraDist = 10.0f;
float						CameraDir = 0.0f;

Quaternion				ModelOrientation(1);
#ifdef SPIN_MODEL
Quaternion				ModelRotation(Vector3(0,0,1),DEG_TO_RAD(1.5f));
#else
Quaternion				ModelRotation(1);
#endif

Quaternion				LightOrientation(1);
Quaternion				LightRotation(Vector3(.2,1,.1),DEG_TO_RAD(3.0f));

/*
** Local functions
*/

void	Render(void);
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

		if (Keyboard->Down(VK_F1)) {
			while(Keyboard->Down(VK_F1));
			WW3D::Set_Next_Render_Device();
		}
		if (Keyboard->Down(VK_F2)) {
			while(Keyboard->Down(VK_F2));
		}
	}

	Destroy_Objects();
	WW3DAssetManager::Get_Instance()->Free_Assets();
	Destroy_Scene();
	Shutdown_Debug();
	Debug_Refs();
}

void Render(void)
{
	ViewportClass view(Vector2(-1,-1),Vector2(1,1));
	Light->Set_Diffuse(Vector3(1.0f,1.0f,1.0f));
	TheScene->Set_Ambient_Light(Vector3(0.8f,0.8f,0.8f));
	WW3D::Begin_Render(true,true,Vector3(0.2f,0.2f,0.5f));
	WW3D::Gerd_Render(TheScene,Camera);
	WW3D::End_Render();
}


void Create_Scene(void)
{
	int rd_index = 0;
	int width = 640;
	int height = 480;
	int color_depth = 16;
	bool windowed = true;

	WW3D::Set_Render_Device(rd_index,width,height,color_depth,windowed);
	TheScene = new SimpleSceneClass();
}

/*
** Load initial game data
*/
void Load_Data(void)
{
//	WW3DAssetManager::Get_Instance()->Load_3D_Assets(RawFileClass("crap.W3D"));
//	WW3DAssetManager::Get_Instance()->Load_3D_Assets(RawFileClass("HUMAN.W3D"));
//	WW3DAssetManager::Get_Instance()->Load_3D_Assets(RawFileClass("COMMANDO.W3D"));
//	WW3DAssetManager::Get_Instance()->Load_3D_Assets(RawFileClass("Mtankl1.W3D"));
	WW3DAssetManager::Get_Instance()->Load_3D_Assets(RawFileClass("Triangle.W3D"));
//	WW3DAssetManager::Get_Instance()->Load_3D_Assets(RawFileClass("Sphere.W3D"));
//	WW3DAssetManager::Get_Instance()->Load_3D_Assets(RawFileClass("new_mtl_test.W3D"));
}


/*			  
**  
*/			   
void Create_Objects(void)
{
	Camera = NEW_REF(CameraClass,());
	Camera->Set_Viewport(Vector2(0,0),Vector2(640,480));
	Camera->Set_Clip_Planes(1.0f, 200.0f);
	Camera->Set_Environment_Range(1.0f, 200.0f);

//	TestModel = WW3DAssetManager::Get_Instance()->Create_Render_Obj("Mtankl1");
//	TestModel = WW3DAssetManager::Get_Instance()->Create_Render_Obj("Crap");
	TestModel = WW3DAssetManager::Get_Instance()->Create_Render_Obj("triangle");
//	TestModel = WW3DAssetManager::Get_Instance()->Create_Render_Obj("new_mtl_test");
	assert(TestModel);
	TestModel->Add(TheScene);

	Light = NEW_REF(LightClass,());
	Matrix3D lighttm(1);
	lighttm.Set_Translation(Vector3(5,0,0));
	Light->Set_Transform(lighttm);
	Light->Add(TheScene);

}	


/*
**
*/
void Destroy_Scene(void)
{
	TheScene->Release_Ref();
	TheScene = NULL;
}


/*
**
*/
void	Destroy_Objects(void)
{
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

	WW3DAssetManager::Get_Instance()->Free_Assets();

}


void Time_Step(void)
{
	if (Keyboard->Down(VK_UP)) {
		CameraDist = CameraDist * 0.9f;
	}
	if (Keyboard->Down(VK_DOWN)) {
		CameraDist = CameraDist * 1.1f;
	}
	if (Keyboard->Down(VK_LEFT)) {
		CameraDir -= (float)DEG_TO_RAD(10.0f);
	}
	if (Keyboard->Down(VK_RIGHT)) {
		CameraDir += (float)DEG_TO_RAD(10.0f);
	}
	
	Matrix3D camtm(1);
	camtm.Rotate_Z(CameraDir);
	camtm.Rotate_X(DEG_TO_RAD(35.0f));
	camtm.Translate(Vector3(0,0,CameraDist));

	Camera->Set_Transform(camtm);

#ifdef SPIN_MODEL
	ModelOrientation = ModelOrientation * ModelRotation;
	ModelOrientation.Normalize();
	if (TestModel)	TestModel->Set_Transform(Build_Matrix3D(ModelOrientation));
#else
	if (TestModel) TestModel->Set_Transform(Matrix3D(1));
#endif

#ifdef SPIN_LIGHT
	LightOrientation = LightOrientation * LightRotation;
	LightOrientation.Normalize();
	Matrix3D ltm = Build_Matrix3D(LightOrientation);
	ltm.Translate(Vector3(5.0f,0.0f,0.0f));
	Light->Set_Transform(ltm);
#endif



#if 0
	MeshClass * mesh = NULL;
	static int _frame = 0;

	if (TestAnim) {
		_frame++;
		if (_frame >= TestAnim->Get_Num_Frames()) {
			_frame = 0;
		}
	  	TestModel->Set_Animation(TestAnim,_frame);
	
	} else {

		TestModel->Set_Animation();
	}

#ifdef CAPTURE_BONE
	int boneid = TestModel->Get_Bone_Index("Head");
	if (boneid != -1) {
		static float r = 10.0f;
		static float theta = 0.0f;
		theta += 0.05f;
		Matrix3D tm(1);
		tm.Rotate_Z(theta);

		TestModel->Capture_Bone(boneid);
		TestModel->Control_Bone(boneid,tm);
	}
#endif

#ifdef SPIN_MODEL	
	ModelOrientation = ModelOrientation * ModelRotation;
	ModelOrientation.Normalize();
	TestModel->Set_Transform(Build_Matrix3D(ModelOrientation));
#else
	TestModel->Set_Transform(Matrix3D(1));
#endif
	
	Matrix3D camtm(1);
	camtm.Rotate_X(DEG_TO_RAD(35.0f));
	camtm.Translate(0,0,20.0f);
	Camera->Set_Transform(camtm);

	camtm.Make_Identity();
	camtm.Translate(Vector3(0,0,20.0f));

	Camera->Set_Transform(camtm);
	Camera->Set_Viewport(Vector2(0,0),Vector2(320,200));
	Camera->Set_View_Plane(60.0f,4.0f/3.0f * 60.0f);
	Camera->Set_Clip_Planes(1.0f, 200.0f);
	Camera->Set_Environment_Range(1.0f, 200.0f);

#ifdef SPIN_LIGHT
	LightOrientation = LightOrientation * LightRotation;
	LightOrientation.Normalize();
	Matrix3D ltm = Build_Matrix3D(LightOrientation);
	ltm.Translate(Vector3(5.0f,0.0f,0.0f));
	Light->Set_Transform(ltm);
#endif

#ifdef TEST_DAMAGE
#define DAMAGE_RATE 0.03f;
	static float _dam_amt = 0.0f;
	static float _dam_chg = DAMAGE_RATE;

	_dam_amt += _dam_chg;
	if (_dam_amt >= 1.0f) {
		_dam_amt = 1.0f;
		_dam_chg = -DAMAGE_RATE;
	}
	if (_dam_amt <= 0.0f) {
		_dam_amt = 0.0f;
		_dam_chg = DAMAGE_RATE;
	}

	mesh = (MeshClass*)TestModel->Get_Sub_Object("Orca01");
	if (mesh) {
		mesh->Apply_Damage(0,_dam_amt);
		mesh->Release_Ref();
	}
#endif

#ifdef TEST_TEXTURE_ANIMATION
	
	mesh = (MeshClass*)TestModel->Get_Sub_Object("Box01");
	if (mesh) {

		MaterialInfoClass * matinfo = mesh->Get_Material_Info();

		if (matinfo) {
			MaterialClass * mtl = matinfo->Get_Material(0);
			
			if (mtl && (openw3d::stricmp(mtl->Get_Name(),"Explosion Material") == 0)) {
			
#if 1
				int curframe = mtl->Get_Channel_Anim_Frame(MaterialClass::DIFFUSE_COLOR);
				curframe = (curframe+1) % mtl->Get_Channel_Anim_Frame_Count(MaterialClass::DIFFUSE_COLOR);
				mtl->Set_Channel_Anim_Frame(MaterialClass::DIFFUSE_COLOR,curframe);
#else
				TextureClass * tex = mtl->Get_Channel_Texture(MaterialClass::DIFFUSE_COLOR);
				assert(tex);

				int curframe = tex->Get_Anim_Frame();
				curframe = (curframe+1) % tex->Get_Num_Frames();
				tex->Set_Anim_Frame(curframe);
				//tex->invalidate();
				tex->Release_Ref();
#endif
			}

			matinfo->Release_Ref();
			matinfo = NULL;

			if (mtl) mtl->Release_Ref();
		}
			
		mesh->Release_Ref();
	}

#endif
#endif

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

	if (RefCountClass::Total_Refs() != 0) {
		sprintf(buf,"Main Loop End %d refs\n", RefCountClass::Total_Refs() );
		MessageBox(NULL,buf,"Ref Debugging",MB_OK);
	}

	RefBaseNodeClass * node = RefBaseClass::ActiveRefList.First();
	
	while (node->Is_Valid())
	{
		RefBaseClass * obj = node->Get();

		ActiveRefStruct * ref = &(obj->ActiveRefInfo);
		
		sprintf(buf,"Active Ref: %s\nLine: %d\nPointer %p\n", ref->File,ref->Line,obj);
		if (MessageBox(NULL,buf,"Ref Debugging",MB_OKCANCEL) == IDCANCEL) {
			break;
		}
		
		node = node->Next();
	}

#endif
}