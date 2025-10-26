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
#include "AssetMgr.H"
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
#include "line3d.h"
#include "dynamesh.h"
#include "sphereobj.h"
#include "ringobj.h"
#include "surfaceclass.h"
#include "vector2i.h"
#include "bmp2d.h"
#include "decalsys.h"
#include "shattersystem.h"
#include "light.h"
#include "texproject.h"
#include "keyboard.h"
#include "wwmouse.h"
#include "predlod.h"
#include "segline.h"
#include <stdio.h>
#include "dx8renderer.h"
#include "textureloader.h"

#define MAX_LOADSTRING 100

const int RESOLUTION_X = 640;
const int RESOLUTION_Y = 480;
const int BIT_DEPTH = 16;

const Vector3 CAMERA_POSITION(3.0f,0.0f,1.0f);
const Vector3 CAMERA_TARGET(0.0f,0.0f,0.90f);
const Vector3 OBJECT_POSITION(0.0f,0.0f,0.0f);

const char * OBJECT_NAME = "area1-LM";
const char * ANIM_NAME = NULL;

static void Log_Statistics();
static void Init_Assets();
static void Init_3D_Scene();
static void Init_2D_Scene();

class SkeletonSceneClass;

// Global Variables:
HINSTANCE				hInst;									// current instance
TCHAR						szTitle[MAX_LOADSTRING];			// The title bar text
TCHAR						szWindowClass[MAX_LOADSTRING];	// The title bar text
WW3DAssetManager *	AssetManager=NULL;
SkeletonSceneClass *	my_scene = NULL;
SimpleSceneClass *	my_2d_scene = NULL;
CameraClass *			my_camera = NULL;
CameraClass *			my_2d_camera = NULL;
Render2DTextClass *	mytext = NULL;
RenderObjClass *		my_object = NULL;
HAnimClass *			my_anim = NULL; 
TexProjectClass *		my_texture_projector = NULL;
MaterialPassClass *	my_material_pass = NULL;
LightClass *			my_lights[3];
Font3DInstanceClass *my_font_a=NULL;
Font3DInstanceClass *my_font_b=NULL;

DecalSystemClass		TheDecalSystem;
bool						running=true;
bool						rotate=true;
bool						randmat=false;
bool						randtex=false;
bool						staticsort=true;
bool						using_mat_pass=false;
int						shatter=0;

// Foward declarations of functions included in this code module:
ATOM						MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK		WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK		About(HWND, UINT, WPARAM, LPARAM);
void						Enable_Alternate_Materials(RenderObjClass * model,bool onoff);
void						Render();
void						Render2();
RenderObjClass *		Create_Dynamesh(void);
RenderObjClass *		Create_Sphere(void);
RenderObjClass *		Create_Ring(void);
RenderObjClass *		Create_bitmap2d(void);
RenderObjClass *		Create_segline(void);
void						WWDebug_Message_Callback(DebugType type, const char * message);
void						WWAssert_Callback(const char * message);
void						Debug_Refs(void);
void						Create_Decal(RenderObjClass * obj,Matrix3D & tm,float radius,char * texture);
MaterialPassClass *	Create_Material_Pass(void);
TexProjectClass *		Create_Texture_Projector(void);

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
// Cusomized scene for testing material passes.
//
// ----------------------------------------------------------------------------

class SkeletonSceneClass : public SimpleSceneClass
{
public:
	SkeletonSceneClass(void) : TestPass(NULL) { }
	~SkeletonSceneClass(void) { REF_PTR_RELEASE(TestPass); }

	virtual void	Customized_Render(RenderInfoClass & rinfo);
	void				Set_Material_Pass(MaterialPassClass * pass)	{ REF_PTR_SET(TestPass,pass); }	

protected:
	MaterialPassClass *	TestPass;
};

// ----------------------------------------------------------------------------

void	SkeletonSceneClass::Customized_Render(RenderInfoClass & rinfo)	
{ 
	if (TestPass) {
		rinfo.Push_Material_Pass(TestPass);
	}

	SimpleSceneClass::Customized_Render(rinfo);

	if (TestPass) {
		rinfo.Pop_Material_Pass();
	}
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
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SKELETON);
	
	// install debug callbacks
	WWDebug_Install_Message_Handler(WWDebug_Message_Callback);
	WWDebug_Install_Assert_Handler(WWAssert_Callback);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SKELETON, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

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
	AssetManager->Load_3D_Assets("ShatterPlanes0.w3d");		// Shatter planes
	WW3D::Init(hWnd);	

//	WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_VERTEX);
	WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
//	WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
//	WW3D::Set_Collision_Box_Display_Mask(0xFF);
	
	if (WW3D::Set_Render_Device(-1,RESOLUTION_X,RESOLUTION_Y,BIT_DEPTH,1,true)!=WW3D_ERROR_OK) {
		if (WW3D::Set_Any_Render_Device()!=WW3D_ERROR_OK) {
			WW3D::Shutdown();
			WWMath::Shutdown ();
			Debug_Refs();
			return 0;
		}
	}

	Init_Assets();
	Init_2D_Scene();
	Init_3D_Scene();

	// main loop	
	int time=timeGetTime();
	float theta = 0.0f;

	while (running)
	{
		if (my_object && rotate) {
			theta += DEG_TO_RADF(0.1f);
			Matrix3D tm(1);
			tm.Rotate_Z(theta);
			//tm.Rotate_X(theta*1.37f);
			//tm.Rotate_Y(theta*1.09f);
			tm.Set_Translation(OBJECT_POSITION);
			my_object->Set_Transform(tm);			
		}		

		if (my_object && my_texture_projector) {
			my_texture_projector->Set_Transform(my_object->Get_Bone_Transform("ProjectorBone"));
		}

		if (shatter==1)
		{
			if (my_object->Class_ID()==RenderObjClass::CLASSID_MESH)
			{
				my_scene->Remove_Render_Object(my_object);
				ShatterSystem::Shatter_Mesh((MeshClass*)my_object,Vector3(0,0,0),Vector3(1,0,0));				
				REF_PTR_RELEASE(my_object);
				int count=ShatterSystem::Get_Fragment_Count();
				int i;
				for (i=0; i<count; i++)
				{
					my_object=ShatterSystem::Get_Fragment(i);
					my_object->Set_Position(my_object->Get_Position()+Vector3(rand()%15,rand()%15,rand()%15));
					my_scene->Add_Render_Object(my_object);
					REF_PTR_RELEASE(my_object);
				}
			}
			shatter++;
		}

		if (randmat)
		{
			if (my_object->Class_ID()==RenderObjClass::CLASSID_MESH)
			{
				MeshClass *mesh=(MeshClass*) my_object;
				MaterialInfoClass *mi=mesh->Get_Material_Info();
				VertexMaterialClass *vm=mi->Get_Vertex_Material(0);
				VertexMaterialClass *newvm=NEW_REF(VertexMaterialClass,(*vm));
				newvm->Set_Emissive(rand()%256/256.0,rand()%256/256.0,rand()%256/256.0);
				mesh->Replace_VertexMaterial(vm,newvm);
				mi->Replace_Material(0,newvm);
				REF_PTR_RELEASE(vm);
				REF_PTR_RELEASE(newvm);
			}
			randmat=!randmat;
		}

		if (randtex)
		{
			if (my_object->Class_ID()==RenderObjClass::CLASSID_MESH)
			{
				MeshClass *mesh=(MeshClass*) my_object;
				MaterialInfoClass *mi=mesh->Get_Material_Info();
				TextureClass *tex=mi->Peek_Texture(0);
				int ran=rand() % 3;
				char name[100];
				switch (ran)
				{
				case 0: sprintf(name,"catwalk.tga");
					break;
				case 1: sprintf(name,"chickonmap.tga");
					break;
				case 2: sprintf(name,"fullmoon.tga");
					break;
				}
				TextureClass *newtex=AssetManager->Get_Texture(name);				
				mesh->Replace_Texture(tex,newtex);
				mi->Replace_Texture(0,newtex);				
				REF_PTR_RELEASE(newtex);
			}
			randtex=!randtex;
		}

		if (my_texture_projector != NULL) {
			my_texture_projector->Pre_Render_Update(my_camera->Get_Transform());
		}

		WW3D::Enable_Static_Sort_Lists(staticsort);
		Render();
		Windows_Message_Handler();		
		WW3D::Sync(timeGetTime()-time);

		Log_Statistics();
	}

	REF_PTR_RELEASE(my_scene);
	REF_PTR_RELEASE(my_camera);
	REF_PTR_RELEASE(my_2d_scene);
	REF_PTR_RELEASE(my_2d_camera);
	REF_PTR_RELEASE(my_lights[0]);
	REF_PTR_RELEASE(my_lights[1]);
	REF_PTR_RELEASE(my_lights[2]);
	delete mytext;
	REF_PTR_RELEASE(my_font_a);
	REF_PTR_RELEASE(my_font_b);
	REF_PTR_RELEASE(my_object);
	REF_PTR_RELEASE(my_anim);
	REF_PTR_RELEASE(my_material_pass);
	REF_PTR_RELEASE(my_texture_projector);
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
	PredictiveLODOptimizerClass::Optimize_LODs(5000);

	WW3D::Begin_Render(true,true,Vector3(0.0f,0.0f,0.0f));		

	// Render 3D scene
	WW3D::Render(my_scene,my_camera);	

	// Render 2D scene
	WW3D::Render(my_2d_scene,my_2d_camera);	

	if (mytext) mytext->Render();

	WW3D::End_Render();
	fps_counter.Update();
}

// ----------------------------------------------------------------------------
//
//  FUNCTION: MyRegisterClass()
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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_SKELETON);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_SKELETON;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

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
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
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
				case ' ':
					shatter++;
					break;
				case '-':
					WW3D::Make_Screen_Shot("screen");
					break;
				case '+':
					WW3D::Toggle_Movie_Capture();
					break;
				case 'r':
					rotate=!rotate;
					break;
				case '1':
					WW3D::Set_NPatches_Level(1);
					break;
				case '2':
					WW3D::Set_NPatches_Level(2);
					break;
				case '3':
					WW3D::Set_NPatches_Level(3);
					break;
				case '4':
					WW3D::Set_NPatches_Level(4);
					break;
				case '5':
					WW3D::Set_NPatches_Level(5);
					break;
				case '6':
					WW3D::Set_NPatches_Level(6);
					break;
				case '7':
					WW3D::Set_NPatches_Level(7);
					break;
				case '8':
					WW3D::Set_NPatches_Level(8);
					break;
				case 'q':
					randmat=!randmat;
					break;
				case 'w':
					randtex=!randtex;
					break;
				case 's':
					staticsort=!staticsort;
					break;
				case 'b':
					{
						using_mat_pass=!using_mat_pass;
						if (!using_mat_pass) {
							my_scene->Set_Material_Pass(0);
						}
						else {
							my_scene->Set_Material_Pass(my_material_pass);
						}
					}
					break;

				case 'g':
					{
						if (WW3D::Get_NPatches_Gap_Filling_Mode()==WW3D::NPATCHES_GAP_FILLING_DISABLED) {
							WW3D::Set_NPatches_Gap_Filling_Mode(WW3D::NPATCHES_GAP_FILLING_ENABLED);
						}
						else {
							WW3D::Set_NPatches_Gap_Filling_Mode(WW3D::NPATCHES_GAP_FILLING_DISABLED);
						}
					}
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


void Enable_Alternate_Materials(RenderObjClass * model,bool onoff)
{
	if (model == NULL) return;

	for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
		RenderObjClass * subobj = model->Get_Sub_Object(i);
		Enable_Alternate_Materials(subobj,onoff);
		REF_PTR_RELEASE(subobj);
	}

	if (model->Class_ID() == RenderObjClass::CLASSID_MESH) {
		MeshModelClass * mesh_model = ((MeshClass *)model)->Get_Model();
		mesh_model->Enable_Alternate_Material_Description(onoff);
		REF_PTR_RELEASE(mesh_model);
	}
}

// ----------------------------------------------------------------------------
//
// WWDebug message callback defines the behavior of WWDEBUG_SAY().
//
// ----------------------------------------------------------------------------

void WWDebug_Message_Callback(DebugType type, const char * message)
{
	OutputDebugString(message);
}

// ----------------------------------------------------------------------------
//
// WWAssert callback defines the behavior of WWASSERT().
//
// ----------------------------------------------------------------------------

void WWAssert_Callback(const char * message)
{
	OutputDebugString(message);
	_asm int 0x03
}

// ----------------------------------------------------------------------------

RenderObjClass * Create_Dynamesh(void)
{
	// Create a tetrahedron
	DynamicMeshClass *dynamesh = NEW_REF(DynamicMeshClass, (4, 6));
	dynamesh->Set_Shader(ShaderClass::_PresetOpaqueShader);
	VertexMaterialClass *vm = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_NODIFFUSE);
	dynamesh->Set_Vertex_Material(vm);
	vm->Release_Ref();
	TextureClass *texture = AssetManager->Get_Texture("chickonmap.tga");
	dynamesh->Set_Texture(texture);
	texture->Release_Ref();
	dynamesh->Vertex(0.0f, 100.0f, 0.0f, 1.0f, 0.0f);
	dynamesh->Vertex(100.0f, -100.0f, 0.0f ,1.0f, -1.0f);
	dynamesh->Vertex(-100.0f, -100.0f, 0.0f, -1.0f, -1.0f);
	dynamesh->Vertex(0.0f, -100.0f, -100.0f, 0.0f, 0.0f);
	dynamesh->Vertex(0.0f, 100.0f, 0.0f, 1.0f, 0.0f);
	dynamesh->Vertex(100.0f, -100.0f, 0.0f ,1.0f, -1.0f);

	return dynamesh;
}

RenderObjClass *Create_segline(void)
{
	SegmentedLineClass *segline = NEW_REF(SegmentedLineClass,());
	
	Vector3 locations[4];
	locations[0].Set(0,0,0);
	locations[1].Set(50,50,50);
	locations[2].Set(50,-50,10);
	locations[3].Set(-50,50,10);
	segline->Set_Points(4, locations);
	//segline->Set_Shader(ShaderClass::_PresetOpaqueShader);
	segline->Set_Shader(ShaderClass::_PresetAlphaShader);
	segline->Set_Color(Vector3(1,0,1));
	segline->Set_Opacity(0.5);
	segline->Set_Freeze_Random(1);
	segline->Set_Visible(1);
	segline->Set_Width(10);
	TextureClass *tex=NEW_REF(TextureClass,("cloudlayer.tga"));
	segline->Set_Texture(tex);
	REF_PTR_RELEASE(tex);
//	segline->Set_Texture_Mapping_Mode(SegmentedLineClass::TILED_TEXTURE_MAP);
	segline->Set_Texture_Tile_Factor(5);
	segline->Set_Noise_Amplitude(5);
	segline->Set_Merge_Intersections(1);

	return segline;
}

// ----------------------------------------------------------------------------

RenderObjClass * Create_Sphere(void)
{
	SphereRenderObjClass *sph=NEW_REF(SphereRenderObjClass,());

	sph->Set_Color(Vector3(1,0,1));
	sph->Set_Scale(Vector3(100,100,100));
	return sph;
}

// ----------------------------------------------------------------------------

RenderObjClass * Create_bitmap2d(void)
{
	Bitmap2DObjClass *bmp=NEW_REF(Bitmap2DObjClass,("chickonmap.tga",0.0,0.0,false,false,false));
	return bmp;
}

// ----------------------------------------------------------------------------

RenderObjClass * Create_Ring(void)
{
	RingRenderObjClass *rng=NEW_REF(RingRenderObjClass,());

	rng->Set_Color(Vector3(1,0,1));
	rng->Set_Inner_Scale(Vector2(10,10));
	rng->Set_Outer_Scale(Vector2(50,50));	
	return rng;
}

// ----------------------------------------------------------------------------

void Create_Decal(RenderObjClass * robj,Matrix3D & tm,float radius,char * texture_name)
{
	/*
	** Allocate the decal generator
	*/
	DecalGeneratorClass * gen = TheDecalSystem.Lock_Decal_Generator();
	WWASSERT(gen != NULL);

	/*
	** Set up the transform, projection, and bounding volume parameters
	*/
	const float DECAL_HALF_SLAB_THICKNESS = 50.0f;
	const float DECAL_BACKFACE_THRESHHOLD = 0.0f;
	float backup_dist = DECAL_HALF_SLAB_THICKNESS + 0.01f;

	Matrix3D transform = tm;
	transform.Translate_Z(backup_dist);
	gen->Set_Transform(transform);
	gen->Set_Ortho_Projection(-radius,radius,-radius,radius,0.01f,backup_dist + 2.0f * DECAL_HALF_SLAB_THICKNESS);
	gen->Set_Backface_Threshhold(DECAL_BACKFACE_THRESHHOLD);

	/*
	** Set up the material settings.  Just plug in the standard alpha shader and the
	** vertex material which all decals use.  Then grab the texture which the user
	** specified...
	*/
	MaterialPassClass	* material = gen->Get_Material();

	VertexMaterialClass * decal_mtl = NEW_REF(VertexMaterialClass,());
	decal_mtl->Set_Ambient(0,0,0);
	decal_mtl->Set_Diffuse(0,0,0);
	decal_mtl->Set_Specular(0,0,0);
	decal_mtl->Set_Emissive(1,1,1);
	decal_mtl->Set_Opacity(1.0f);
	decal_mtl->Set_Shininess(0.0f);

	TextureClass * tex = WW3DAssetManager::Get_Instance()->Get_Texture(texture_name);
	tex->Set_U_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
	tex->Set_V_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);

	material->Set_Shader(ShaderClass::_PresetAlphaShader);
	material->Set_Material(decal_mtl);
	material->Set_Texture(tex);

	REF_PTR_RELEASE(decal_mtl);
	REF_PTR_RELEASE(tex);
	REF_PTR_RELEASE(material);

	/*
	** Apply!
	*/
	robj->Create_Decal(gen);
	TheDecalSystem.Unlock_Decal_Generator(gen);
}

// ----------------------------------------------------------------------------

MaterialPassClass *	Create_Material_Pass(void)
{
	TextureClass * texture = WW3DAssetManager::Get_Instance()->Get_Texture("decal.tga");	
	VertexMaterialClass * mtl = NEW_REF(VertexMaterialClass,());
	mtl->Set_Diffuse(0.5f,0.5f,0.5f);
	mtl->Set_Specular(0,0,0);
	mtl->Set_Ambient(0,0,0);
	mtl->Set_Emissive(0.5f,0.5f,0.5f);
	mtl->Set_Lighting(false);

	EnvironmentMapperClass * mapper = NEW_REF(EnvironmentMapperClass,(0));
	mtl->Set_Mapper(mapper,0);

	MaterialPassClass * test_pass = NEW_REF(MaterialPassClass,());
	test_pass->Set_Shader(ShaderClass::_PresetAdditiveShader);
	test_pass->Set_Material(mtl);
	test_pass->Set_Texture(texture);

	REF_PTR_RELEASE(mtl);
	REF_PTR_RELEASE(texture);
	REF_PTR_RELEASE(mapper);

	return test_pass;
}

// ----------------------------------------------------------------------------

TexProjectClass *	Create_Texture_Projector(void)
{
	const float SIZE = 10.0f;
	const float ZNEAR = 0.1f;
	const float ZFAR = 300.0f;

	TexProjectClass * new_projector = NEW_REF(TexProjectClass,());
	new_projector->Init_Additive();
	new_projector->Set_Intensity(1.0,true);
	new_projector->Set_Ortho_Projection(-SIZE,SIZE,-SIZE,SIZE,ZNEAR,ZFAR);
	
	Matrix3D tm;
	tm.Look_At(Vector3(200,200,200),Vector3(0,0,0),0.0f);
	new_projector->Set_Transform(tm);

	TextureClass * texture = WW3DAssetManager::Get_Instance()->Get_Texture("GreenSpot.tga");	

	new_projector->Set_Texture(texture);
	REF_PTR_RELEASE(texture);

	return new_projector;
}

// ----------------------------------------------------------------------------

void Debug_Refs(void)
{
#ifdef _DEBUG
	WWDEBUG_SAY(("Dumping Un-Released Ref-Counted objects...\r\n"));
	RefCountNodeClass * first = RefCountClass::ActiveRefList.First();
	RefCountNodeClass * node = first;
	while (node->Is_Valid())
	{
		RefCountClass * obj = node->Get();
		ActiveRefStruct * ref = &(obj->ActiveRefInfo);

		bool display = true;
		int	count = 0;
		RefCountNodeClass * search = first;
		while (search->Is_Valid()) {

			if (search == node) {	// if this is not the first one
				if (count != 0) {
					display = false;
					break;
				}
			}

			RefCountClass * search_obj = search->Get();
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

	static unsigned stats_mode;

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
		break;
	}

	mytext->Reset();
	mytext->Set_Location(Vector2(0.0,0.0));
	mytext->Draw_Text(status_text,0xffff0000);
}

// ----------------------------------------------------------------------------

void Init_Assets()
{

	TextureClass *tex=AssetManager->Get_Texture("PPVF0102.TGA");
	REF_PTR_RELEASE(tex);

	// Preload test assets. Note that not all of these will actually be used.
/*	AssetManager->Load_3D_Assets("GreenSphere.w3d");		// Tests regular meshes
	AssetManager->Load_3D_Assets("TexSphere.w3d");			// Tests texture mapping
	AssetManager->Load_3D_Assets("TexScrlSphere.w3d");		// Tests Linear Offset mapper
	AssetManager->Load_3D_Assets("SphereAnim.w3d");			// Tests hierarchical animation (if you install its anim)
   AssetManager->Load_3D_Assets("SkinTest.w3d");			// Tests skinning and environment mapping
   AssetManager->Load_3D_Assets("sorting_test.w3d");			// Tests skinning and environment mapping
   AssetManager->Load_3D_Assets("boxtest.w3d");				// Tests BoxRenderObjClass (turn on the box display mask)
  	AssetManager->Load_3D_Assets("Test1.w3d");				// Test yellow particles		
  	AssetManager->Load_3D_Assets("Test2.w3d");				// Tests alpha particles
  	AssetManager->Load_3D_Assets("MPassSphere.w3d");		// Tests more than 2 passes on a mesh
  	AssetManager->Load_3D_Assets("MPassTeapot.w3d");		// Tests more that 2 passes
  	AssetManager->Load_3D_Assets("DupSphere.w3d");			// Tests 3 passes which all share the same uv coords
  	AssetManager->Load_3D_Assets("vcolortest.w3d");			// Tests vertex color
  	AssetManager->Load_3D_Assets("valphatest.w3d");			// Tests vertex alpha
  	AssetManager->Load_3D_Assets("LightmapTest0.w3d");		// Tests pre-lighting, 3 solves: vertex, multi-pass, multi-stage
  	AssetManager->Load_3D_Assets("LightmapTest1.w3d");		// Tests pre-lighting with alternate solves! (not currently runtime switchable)
	AssetManager->Load_3D_Assets("CubeSkin.w3d");			// Tests a simple skin, also can be used to test decals on skin meshes
	AssetManager->Load_3D_Assets("ProjectorTest.w3d");		// This model contains a mesh and an animated "ProjectorBone" for testing texture projections
	AssetManager->Load_3D_Assets("PolyRateTest.w3d");		// This model has lots of polys!
	AssetManager->Load_3D_Assets("PPVF01A.w3d");				// Game asset
	AssetManager->Load_3D_Assets("SPHERE_TEST.w3d");		// Tests multitexturing
	AssetManager->Load_3D_Assets("SCREEN_ALIGN.w3d");		// Tests camera aligned meshes
	AssetManager->Load_3D_Assets("halfsphere.w3d");			// Tests two-sided flag, camera aligned, and camera oriented meshes
	AssetManager->Load_3D_Assets("tsi01a.w3d");			// Tests static sort level
	AssetManager->Load_3D_Assets("tsi08a.w3d");			// Tests static sort level	
*/	
	AssetManager->Load_3D_Assets("area1-LM.w3d");
/*
	AssetManager->Load_3D_Assets("S_A_Human.W3D");
	AssetManager->Load_3D_Assets("c_nod_rsold_L0.W3D");
	AssetManager->Load_3D_Assets("h_a_a0a1.W3D");
*/
}
	
// ----------------------------------------------------------------------------
//
// Create a scene for 2D-display, such as text and rectangles.
//
// ----------------------------------------------------------------------------

void Init_2D_Scene()
{
	// Create a scene for the 2D stuff -----------------------------------------

	my_2d_scene=NEW_REF(SimpleSceneClass,());
	my_2d_scene->Set_Ambient_Light(Vector3(1,1,1));

	// Create fonts to be used for text rendering ------------------------------

	my_font_a = AssetManager->Get_Font3DInstance("font12x16.tga");
	my_font_b = AssetManager->Get_Font3DInstance("fontnew4.tga");

	mytext=new Render2DTextClass(my_font_b);
	mytext->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

	// Scene needs camera to be rendered with ----------------------------------

	my_2d_camera = NEW_REF(CameraClass,());   
#ifdef DRAWBITMAP
	my_2d_camera->Set_Position(Vector3(0,0,1));	
	my_2d_camera->Set_Clip_Planes(0.995,2);
#else
	my_2d_camera->Set_Position(Vector3(0,0,320));
	my_2d_camera->Set_Clip_Planes(300,1000);
#endif
	Vector2 min = Vector2(-1, -0.75f);
	Vector2 max = Vector2(+1, +0.75f);
	my_2d_camera->Set_View_Plane(min, max);			
}

void Init_3D_Scene()
{
	WW3D::Enable_Sorting(false);
	WW3D::Set_NPatches_Level(1);

	// build scene	
	my_scene=NEW_REF(SkeletonSceneClass,());
	my_scene->Set_Ambient_Light(Vector3(0.5f,0.5f,0.5f));

	// set up a light
	my_lights[0]=my_lights[1]=my_lights[2]=NULL;
	my_lights[0]=NEW_REF(LightClass,(LightClass::DIRECTIONAL));	
	my_lights[0]->Set_Ambient(Vector3(0.1f,0.1f,0.1f));
	my_lights[0]->Set_Diffuse(Vector3(1.0f,1.0f,1.0f));
	my_lights[0]->Set_Spot_Direction(Vector3(0,1,0));
	my_scene->Add_Render_Object(my_lights[0]);

	// Scene needs camera to be rendered with ----------------------------------
	my_camera=NEW_REF(CameraClass,());
	Matrix3D camtransform(1);
	camtransform.Look_At(CAMERA_POSITION,CAMERA_TARGET,0);
	my_camera->Set_Transform(camtransform);

	my_object = AssetManager->Create_Render_Obj(OBJECT_NAME);
	if (ANIM_NAME != NULL) {
		my_anim = AssetManager->Get_HAnim(ANIM_NAME);
	}

	if (my_object != NULL) {
		
		my_object->Set_Position(OBJECT_POSITION);

#if 0
		Enable_Alternate_Materials(my_object,true);
#endif

		// Decal System
#if 0
		Matrix3D decal_tm;
		decal_tm.Look_At(Vector3(30,5,10),Vector3(0,0,10),DEG_TO_RADF(65.5f));
		Create_Decal(my_object,decal_tm,13.7f,"decal.tga");
#endif

//		PredictiveLODOptimizerClass::Add_Object(my_object);		
		my_object->Set_Position(Vector3(0,0,0));
		if (my_anim != NULL) {
			my_object->Set_Animation(my_anim,0,RenderObjClass::ANIM_MODE_LOOP);
		}
		my_scene->Add_Render_Object(my_object);

		if (my_object->Class_ID()==RenderObjClass::CLASSID_PARTICLEEMITTER) {
			((ParticleEmitterClass*) my_object)->Start();
		}
	}	

	// 3D line object ----------------------------------------------------------
#if 0
	Line3DClass *line = NULL;
	line = NEW_REF(Line3DClass,(Vector3(0,0,0),Vector3(100,100,0), 10,1,1,1,1));
	my_scene->Add_Render_Object(line);
  	REF_PTR_RELEASE(line);	// Obejct can be freed after adding scene so that it gets killed when scene dies
#endif

	// Create texture projector material pass ----------------------------------
#if 0
	my_texture_projector = Create_Texture_Projector();
	my_scene->Set_Material_Pass(my_texture_projector->Peek_Material_Pass());
//	REF_PTR_RELEASE(my_texture_projector);
#endif

	// Create material pass ----------------------------------------------------
#if 0
	my_material_pass = Create_Material_Pass();
	my_scene->Set_Material_Pass(my_material_pass);
	REF_PTR_RELEASE(my_material_pass);
#endif

}















