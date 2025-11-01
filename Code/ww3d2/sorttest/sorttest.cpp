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

// 
// skeleton.cpp : Defines the entry point for the application.
//
// Skeleton WW3D code, Hector Yee, 8/31/00

#include "resource.h"
#include "wwmath.h"
#include "ww3d.h"
#include "scene.h"
#include "rendobj.h"
#include "camera.h"
#include "assetmgr.h"
#include "msgloop.h"
#include "part_ldr.h"
#include "rendobj.h"
#include "hanim.h"
#include "dx8wrapper.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8fvf.h"
#include "vertmaterial.h"
#include "font3d.h"
#include "render2d.h"
#include "textdraw.h"
#include "rect.h"
#include "mesh.h"
#include "meshmdl.h"
#include "vector2i.h"
#include "bmp2d.h"
#include "decalsys.h"
#include "shattersystem.h"
#include "light.h"
#include "keyboard.h"
#include "wwmouse.h"
#include "predlod.h"
#include "sphere.h"
#include <stdio.h>
#include "dx8renderer.h"
#include "ini.h"

#define MAX_LOADSTRING 100

static void Log_Statistics();
static void Init_3D_Scene();

// Global Variables:
HINSTANCE				hInst;									// current instance
char						szTitle[MAX_LOADSTRING];			// The title bar text
char						szWindowClass[MAX_LOADSTRING];	// The title bar text
WW3DAssetManager *	AssetManager=NULL;
SimpleSceneClass *	my_scene = NULL;
CameraClass *			my_camera = NULL;
Render2DTextClass *	mytext = NULL;
RenderObjClass *		orig_object = NULL;
MaterialPassClass *	mat_pass = NULL;
HAnimClass *			my_anim = NULL; 
Font3DInstanceClass *my_font_a=NULL;
Font3DInstanceClass *my_font_b=NULL;

DecalSystemClass		TheDecalSystem;
bool						running=true;
bool						rotate=false;
bool						sort=true;
float						sep;

// Foward declarations of functions included in this code module:
ATOM						MyRegisterClassA(HINSTANCE hInstance);
LRESULT CALLBACK		WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK		About(HWND, UINT, WPARAM, LPARAM);
void						Render();
void						WWDebug_Message_Callback(DebugType type, const char * message);
void						WWAssert_Callback(const char * message);
void						Debug_Refs(void);
void						LoadAssets();

// Scene class to use mat_pass:
class MatPassSceneClass : public SimpleSceneClass {
protected:
	virtual void Customized_Render(RenderInfoClass & rinfo);
};
void MatPassSceneClass::Customized_Render(RenderInfoClass &rinfo)
{
	if (mat_pass) {
		rinfo.Push_Material_Pass(mat_pass);
		bool hide_mesh = false;
		int hide_flag = hide_mesh ? ((int)RenderInfoClass::RINFO_OVERRIDE_FORCE_SORTING |
			(int)RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY) : 0;
		rinfo.Push_Override_Flags((RenderInfoClass::RINFO_OVERRIDE_FLAGS)	((int)rinfo.Current_Override_Flags() | hide_flag));
	}

	SimpleSceneClass::Customized_Render(rinfo);

	if (mat_pass) {
		rinfo.Pop_Override_Flags();
		rinfo.Pop_Material_Pass();
	}
}


// ----------------------------------------------------------------------------
//
// FPS counter class counts how many times Update() has been called during the
// last second. The fps counter caps at MAX_FPS which is the maximum fps count
// that can be measured.
//
// ----------------------------------------------------------------------------

const unsigned MAX_FPS=2000;
class FPSCounterClass
{
	unsigned frame_times[MAX_FPS];
	unsigned frame_time_count;
public:
	FPSCounterClass();

	void Update();
	unsigned Get_FPS();
};

static FPSCounterClass fps_counter;

// ----------------------------------------------------------------------------

FPSCounterClass::FPSCounterClass()
	:
	frame_time_count(0)
{
}

// ----------------------------------------------------------------------------

void FPSCounterClass::Update()
{
	unsigned frame_time=timeGetTime();
	unsigned limit=frame_time-1000;

	for (unsigned i=0;i<frame_time_count;++i) {
		if (frame_times[i]<limit) {
			frame_times[i]=frame_times[frame_time_count-1];
			frame_time_count--;
		}
	}
	if (frame_time_count<MAX_FPS) {
		frame_times[frame_time_count++]=frame_time;
	}
}

// ----------------------------------------------------------------------------

unsigned FPSCounterClass::Get_FPS()
{
	return frame_time_count;
}


// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, (const char*)IDC_SKELETON);
	
	// install debug callbacks
	WWDebug_Install_Message_Handler(WWDebug_Message_Callback);
	WWDebug_Install_Assert_Handler(WWAssert_Callback);

	// Initialize global strings
	LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringA(hInstance, IDC_SKELETON, szWindowClass, MAX_LOADSTRING);
	MyRegisterClassA(hInstance);

	// Perform application initialization:

	hInst = hInstance; // Store instance handle in our global variable
	HWND hWnd = CreateWindow(
		szWindowClass, 
		szTitle, 
		WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 
		0, 
		CW_USEDEFAULT, 
		0, 
		NULL, 
		NULL, 
		hInstance, 
		NULL);

	ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // WW Inits 
	WWMath::Init ();
	AssetManager=new WW3DAssetManager;	
	AssetManager->Register_Prototype_Loader(&_ParticleEmitterLoader);
	WW3D::Init(hWnd);	
	WW3D::Enable_Munge_Sort_On_Load(true);
	WW3D::Set_Texture_Thumbnail_Mode(WW3D::TEXTURE_THUMBNAIL_MODE_ON);

//	WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_VERTEX);
	WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
//	WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
	WW3D::Set_Collision_Box_Display_Mask(0xFF);
	
	if (WW3D::Set_Render_Device(0,800,600,32,1,true)!=WW3D_ERROR_OK) {
		WW3D::Shutdown();
		WWMath::Shutdown ();
		Debug_Refs();
		return 0;
	}
	
	Init_3D_Scene();	
	WW3D::Enable_Sorting(sort);

	// main loop	
	int time=timeGetTime();
	float theta = 0.0f;

	while (running)
	{
		if (rotate) {
			theta += DEG_TO_RADF(0.5f);
			Matrix3D tm(1);
			tm.Rotate_Z(theta);			
			if (orig_object) orig_object->Set_Transform(tm);			
		}		
		
		Render();
		Windows_Message_Handler();		
		WW3D::Sync(timeGetTime()-time);

		Log_Statistics();
	}

	REF_PTR_RELEASE(my_scene);
	REF_PTR_RELEASE(my_camera);	
	delete mytext;
	REF_PTR_RELEASE(my_font_a);
	REF_PTR_RELEASE(my_font_b);
	REF_PTR_RELEASE(orig_object);	
	REF_PTR_RELEASE(my_anim);
	PredictiveLODOptimizerClass::Free();

	// shutdown
	AssetManager->Free_Assets ();
	delete AssetManager;
	WW3D::Shutdown ();
	WWMath::Shutdown ();

	Debug_Refs();
	return 0;
}

// ----------------------------------------------------------------------------
//
//	Render everything. Rendering stars with Begin_Scene() and ends to End_Scene().
//
// ----------------------------------------------------------------------------

void Render()
{
	// Predictive LOD optimizer optimizes the mesh LOD levels to match the given polygon budget
	
	my_scene->Visibility_Check(my_camera);
	PredictiveLODOptimizerClass::Optimize_LODs(150000);

	WW3D::Begin_Render(true,true,Vector3(0.5f,0.5f,0.5f));		

	// Render 3D scene
	WW3D::Render(my_scene,my_camera);	

	if (mytext) mytext->Render();

	WW3D::End_Render();
	fps_counter.Update();
}

// ----------------------------------------------------------------------------
//
//  FUNCTION: MyRegisterClassA()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
// ----------------------------------------------------------------------------

ATOM MyRegisterClassA(HINSTANCE hInstance)
{
	WNDCLASSEXA wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (const char*)IDI_SKELETON);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_SKELETON;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (const char*)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

// ----------------------------------------------------------------------------
//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
// ----------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (const char*)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				case IDM_RELOAD:
					LoadAssets();
					break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_ACTIVATEAPP:
			if (wParam) WW3D::On_Activate_App();
			else WW3D::On_Deactivate_App();
			return DefWindowProc(hWnd, message, wParam, lParam);
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...			
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			running=false;
			PostQuitMessage(0);
			break;
		case WM_KEYUP:
			if ((wParam&0xff)==VK_ESCAPE) {
			   DestroyWindow(hWnd);
				return 0;
			}
			return DefWindowProc(hWnd, message, wParam, lParam);
		case WM_CHAR:
			{
				char key=LOWORD(wParam);
				switch (key)
				{
				case '-':
					WW3D::Make_Screen_Shot("screen");
					break;
				case '+':
					WW3D::Toggle_Movie_Capture();
					break;
				case ' ':
					rotate=!rotate;
					break;				
				case 's':
					sort=!sort;
					WW3D::Enable_Sorting(sort);
					WW3D::Enable_Static_Sort_Lists(!sort);
					TheDX8MeshRenderer.Invalidate();
					break;
				case 'm':
					{
						SceneClass::PolyRenderType type = my_scene->Get_Polygon_Mode();
						type = (type == SceneClass::POINT) ? SceneClass::LINE : ((type == SceneClass::LINE) ? SceneClass::FILL : SceneClass::POINT);
						my_scene->Set_Polygon_Mode(type);
					}
					break;
				}
			}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return true;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return true;
			}
			break;		
	}
    return false;
}

// ----------------------------------------------------------------------------
//
// WWDebug message callback defines the behavior of WWDEBUG_SAY().
//
// ----------------------------------------------------------------------------

void WWDebug_Message_Callback(DebugType type, const char * message)
{
	OutputDebugStringA(message);
}

// ----------------------------------------------------------------------------
//
// WWAssert callback defines the behavior of WWASSERT().
//
// ----------------------------------------------------------------------------

void WWAssert_Callback(const char * message)
{
	OutputDebugStringA(message);
	_asm int 0x03
}

// ----------------------------------------------------------------------------

void Debug_Refs(void)
{
#ifdef _DEBUG
	WWDEBUG_SAY(("Dumping Un-Released Ref-Counted objects...\r\n"));
	RefBaseNodeClass * first = RefBaseClass::ActiveRefList.First();
	RefBaseNodeClass * node = first;
	while (node->Is_Valid())
	{
		RefBaseClass * obj = node->Get();
		ActiveRefStruct * ref = &(obj->ActiveRefInfo);

		bool display = true;
		int	count = 0;
		RefBaseNodeClass * search = first;
		while (search->Is_Valid()) {

			if (search == node) {	// if this is not the first one
				if (count != 0) {
					display = false;
					break;
				}
			}

			RefBaseClass * search_obj = search->Get();
			ActiveRefStruct * search_ref = &(search_obj->ActiveRefInfo);

			if ( ref->File && search_ref->File &&
				  !strcmp(search_ref->File, ref->File) &&
				  (search_ref->Line == ref->Line) ) {
				count++;
			} else if ( (ref->File == NULL) &&  (search_ref->File == NULL) ) {
				count++;
			}

			search = search->Next();
		}

		if ( display ) {
			WWDEBUG_SAY(( "%d Active Ref: %s %d %p\n", count, ref->File,ref->Line,obj));

			static int num_printed = 0;
			if (++num_printed > 20) {
				WWDEBUG_SAY(( "And Many More......\n"));
				break;
			}
		}

		node = node->Next();
	}
	WWDEBUG_SAY(("Done.\r\n"));
#endif
}

// ----------------------------------------------------------------------
//
// Ugly statistics block!!!
//
// ----------------------------------------------------------------------

void Log_Statistics()
{
	StringClass format(255,true);
	StringClass status_text(500,true);

//	static unsigned last_frame_time=0;
//	unsigned frame_time=timeGetTime();
//	unsigned time=frame_time-last_frame_time;
//	last_frame_time=frame_time;
//	if (time>1000) time=1000;
//	float fps=1000.0f/float(time);
//	static float time_fps;
//	static float current_fps;
//	static unsigned fps_count;
//	time_fps+=fps;
//	fps_count++;
//	if (fps_count==20) {
//		fps_count=0;
//		current_fps=time_fps/20.0f;
//		time_fps=0.0f;
//	}
	unsigned current_fps=fps_counter.Get_FPS();

	static unsigned stats_mode=1;

	static bool tab_pressed;
	if (GetAsyncKeyState(VK_TAB)) {
		if (!tab_pressed) {
			stats_mode++;
			stats_mode%=3;
		}
		tab_pressed=true;
	}
	else {
		tab_pressed=false;
	}

	switch (stats_mode) {
	case 0:		
		Debug_Statistics::Record_Texture_Mode(Debug_Statistics::RECORD_TEXTURE_NONE);
		break;
	case 1:
		Debug_Statistics::Record_Texture_Mode(Debug_Statistics::RECORD_TEXTURE_NONE);
		format.Format("%d FPS\n",current_fps);
		status_text+=format;
		format.Format("%d Polys/frame (%dk pps)\n",Debug_Statistics::Get_DX8_Polygons(),unsigned(Debug_Statistics::Get_DX8_Polygons()*float(current_fps))/1000);
		status_text+=format;
		if (sort)
			format.Format("Sorting On\n");
		else
			format.Format("Sorting Off\n");
		status_text+=format;
		break;
	case 2:
		Debug_Statistics::Record_Texture_Mode(Debug_Statistics::RECORD_TEXTURE_NONE);
		format.Format("%d FPS\n",current_fps);
		status_text+=format;
		format.Format("%d Polys/frame (%dk pps)\n",Debug_Statistics::Get_DX8_Polygons(),unsigned(Debug_Statistics::Get_DX8_Polygons()*float(current_fps))/1000);
		status_text+=format;
		format.Format("%d Verts/frame (%dk vps)\n",Debug_Statistics::Get_DX8_Vertices(),unsigned(Debug_Statistics::Get_DX8_Vertices()*float(current_fps))/1000);
		status_text+=format;
		format.Format("%d DX8 calls\n",DX8Wrapper::Get_Last_Frame_DX8_Calls());
		status_text+=format;
		format.Format("%d VB changes\n",DX8Wrapper::Get_Last_Frame_Vertex_Buffer_Changes());
		status_text+=format;
		format.Format("%d IB changes\n",DX8Wrapper::Get_Last_Frame_Index_Buffer_Changes());
		status_text+=format;
		format.Format("%d texture changes\n",DX8Wrapper::Get_Last_Frame_Texture_Changes());
		status_text+=format;
		format.Format("%d material changes\n",DX8Wrapper::Get_Last_Frame_Material_Changes());
		status_text+=format;
		format.Format("%d light changes\n",DX8Wrapper::Get_Last_Frame_Light_Changes());
		status_text+=format;
		format.Format("%d RS changes\n",DX8Wrapper::Get_Last_Frame_Render_State_Changes());
		status_text+=format;
		format.Format("%d TSS changes\n",DX8Wrapper::Get_Last_Frame_Texture_Stage_State_Changes());
		status_text+=format;
		format.Format("%d Mtx changes\n",DX8Wrapper::Get_Last_Frame_Matrix_Changes());
		status_text+=format;
		if (sort)
			format.Format("Sorting On\n");
		else
			format.Format("Sorting Off\n");
		status_text+=format;		
		break;
	}

	mytext->Reset();
	mytext->Set_Location(Vector2(0.0,0.0));
	mytext->Draw_Text(status_text,0xffff0000);
}

void	LoadAssets()
{
	if (orig_object)
	{
		my_scene->Remove_Render_Object(orig_object);
		REF_PTR_RELEASE(orig_object);
	}

	INIClass ini;
	ini.Load("sorttest.ini");

	StringClass asset=ini.Get_String("GENERAL","ASSET");
	
	AssetManager->Load_3D_Assets(asset+".w3d");
	
	orig_object = AssetManager->Create_Render_Obj(asset);

	float rad=1;
	
	if (orig_object)
	{	
		my_scene->Add_Render_Object(orig_object);
		rad=orig_object->Get_Bounding_Sphere().Radius;
	}	

	Matrix3D camtransform(1);
	camtransform.Look_At(Vector3(4*rad,0,0),Vector3(0,0,0),0);
	my_camera->Set_Transform(camtransform);
	my_camera->Set_Clip_Planes(1.0f,10*rad);

	// Load swatch for material pass
	if (mat_pass)
	{
		REF_PTR_RELEASE(mat_pass);
	}
	StringClass swatch=ini.Get_String("GENERAL","SWATCH");
	AssetManager->Load_3D_Assets(swatch+".w3d");
	RenderObjClass *swatch_robj = AssetManager->Create_Render_Obj(swatch);

	if (swatch_robj) {

		if (swatch_robj->Class_ID() == RenderObjClass::CLASSID_MESH) {

			// Create pass from swatch mesh
			MeshModelClass *meshmdl = ((MeshClass *)swatch_robj)->Get_Model();
			mat_pass = new MaterialPassClass();
			mat_pass->Set_Texture(meshmdl->Peek_Single_Texture());
			mat_pass->Set_Shader(meshmdl->Get_Single_Shader());
			mat_pass->Set_Material(meshmdl->Peek_Single_Material());

		}

		swatch_robj->Release_Ref();
	}

}

void Init_3D_Scene()
{
  	my_font_a = AssetManager->Get_Font3DInstance("font12x16.tga");
  	my_font_b = AssetManager->Get_Font3DInstance("fontnew4.tga");
  
  	mytext=new Render2DTextClass(my_font_a);
  	mytext->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

	// build scene	
	my_scene=NEW_REF(MatPassSceneClass,());
	my_scene->Set_Ambient_Light(Vector3(1.0f,1.0f,1.0f));		

	my_camera=NEW_REF(CameraClass,());	

	LoadAssets();
}