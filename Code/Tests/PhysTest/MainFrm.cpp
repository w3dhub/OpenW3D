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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "PhysTest.h"

#include "MainFrm.h"
#include "PhysTestDoc.h"
#include "GraphicView.h"
#include "DataView.h"
#include "PhysicsConstantsDialog.h"
#include "VJoyDialog.h"
#include "InertiaDialog.h"
#include "RbodyPropertiesDialog.h"
#include "MotorVehicleDialog.h"
#include "WheeledVehicleDialog.h"
#include "MotorcycleDialog.h"
#include "RenderDeviceDialog.h"
#include "assetmgr.h"
#include "ww3d.h"
#include "part_ldr.h"
#include "pscene.h"
#include "rbody.h"
#include "motorvehicle.h"
#include "wheelvehicle.h"
#include "motorcycle.h"
#include "chunkio.h"

const float IMPULSE_SCALE = 3.0f;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame CHUNK Id's
enum {
	MAINFRAME_CHUNK_VARIABLES					= 0x00800760,
	MAINFRAME_CHUNK_TREEVIEW,
	MAINFRAME_CHUNK_GRAPHICVIEW,
	MAINFRAME_VARIABLE_CONTROLLERPTR			= 0x00,
};


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_ACTIVATEAPP()
	ON_COMMAND(ID_OPTIONS_RUN_SIMULATION, OnOptionsRunSimulation)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_RUN_SIMULATION, OnUpdateOptionsRunSimulation)
	ON_COMMAND(ID_OPTIONS_DISPLAY_BOXES, OnOptionsDisplayBoxes)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_DISPLAY_BOXES, OnUpdateOptionsDisplayBoxes)
	ON_COMMAND(ID_CREATE_RIGID_BODY, OnCreateRigidBody)
	ON_UPDATE_COMMAND_UI(ID_COUPLE_NEGX, OnUpdateImpulseButton)
	ON_COMMAND(ID_COUPLE_NEGX, OnCoupleNegx)
	ON_COMMAND(ID_COUPLE_NEGY, OnCoupleNegy)
	ON_COMMAND(ID_COUPLE_NEGZ, OnCoupleNegz)
	ON_COMMAND(ID_COUPLE_POSX, OnCouplePosx)
	ON_COMMAND(ID_COUPLE_POSY, OnCouplePosy)
	ON_COMMAND(ID_COUPLE_POSZ, OnCouplePosz)
	ON_COMMAND(ID_IMPULSE_NEGX, OnImpulseNegx)
	ON_COMMAND(ID_IMPULSE_NEGY, OnImpulseNegy)
	ON_COMMAND(ID_IMPULSE_NEGZ, OnImpulseNegz)
	ON_COMMAND(ID_IMPULSE_POSX, OnImpulsePosx)
	ON_COMMAND(ID_IMPULSE_POSY, OnImpulsePosy)
	ON_COMMAND(ID_IMPULSE_POSZ, OnImpulsePosz)
	ON_COMMAND(ID_VIEW_IMPULSETOOLBAR, OnViewImpulsetoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IMPULSETOOLBAR, OnUpdateViewImpulsetoolbar)
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_INERTIA, OnInertia)
	ON_COMMAND(ID_OPTIONS_PHYSICS_CONSTANTS, OnOptionsPhysicsConstants)
	ON_COMMAND(ID_FREEZE_OBJECT, OnFreezeObject)
	ON_COMMAND(ID_DEBUG_OBJECT, OnDebugObject)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_OBJECT, OnUpdateDebugObject)
	ON_COMMAND(ID_VIEW_VIRTUALJOYSTICK, OnViewVirtualjoystick)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIRTUALJOYSTICK, OnUpdateViewVirtualjoystick)
	ON_WM_DESTROY()
	ON_COMMAND(ID_OPTIONS_FILLED, OnOptionsFilled)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_FILLED, OnUpdateOptionsFilled)
	ON_COMMAND(ID_OPTIONS_POINTS, OnOptionsPoints)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_POINTS, OnUpdateOptionsPoints)
	ON_COMMAND(ID_OPTIONS_WIREFRAME, OnOptionsWireframe)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_WIREFRAME, OnUpdateOptionsWireframe)
	ON_COMMAND(ID_MOTOR_PROPERTIES, OnMotorProperties)
	ON_COMMAND(ID_WHEEL_PROPERTIES, OnWheelProperties)
	ON_UPDATE_COMMAND_UI(ID_WHEEL_PROPERTIES, OnUpdateWheelProperties)
	ON_UPDATE_COMMAND_UI(ID_MOTOR_PROPERTIES, OnUpdateMotorProperties)
	ON_COMMAND(ID_CAMERA_FOLLOW, OnCameraFollow)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_FOLLOW, OnUpdateCameraFollow)
	ON_COMMAND(ID_CAMERA_FLY, OnCameraFly)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_FLY, OnUpdateCameraFly)
	ON_COMMAND(ID_CAMERA_TETHER, OnCameraTether)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_TETHER, OnUpdateCameraTether)
	ON_COMMAND(ID_CAMERA_RIGID_TETHER, OnCameraRigidTether)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_RIGID_TETHER, OnUpdateCameraRigidTether)
	ON_COMMAND(ID_CREATE_WHEELED_VEHICLE, OnCreateWheeledVehicle)
	ON_UPDATE_COMMAND_UI(ID_CREATE_RIGID_BODY, OnUpdateCreateRigidBody)
	ON_UPDATE_COMMAND_UI(ID_CREATE_WHEELED_VEHICLE, OnUpdateCreateWheeledVehicle)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES, OnUpdateProperties)
	ON_COMMAND(ID_CREATE_MOTORCYCLE, OnCreateMotorcycle)
	ON_UPDATE_COMMAND_UI(ID_CREATE_MOTORCYCLE, OnUpdateCreateMotorcycle)
	ON_COMMAND(ID_MOTORCYCLE_PROPERTIES, OnMotorcycleProperties)
	ON_UPDATE_COMMAND_UI(ID_MOTORCYCLE_PROPERTIES, OnUpdateMotorcycleProperties)
	ON_COMMAND(ID_OPTIONS_RENDER_DEVICE, OnOptionsRenderDevice)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_RENDER_DEVICE, OnUpdateOptionsRenderDevice)
	ON_COMMAND(ID_FILE_IMPORT_MODEL, OnFileImportModel)
	ON_COMMAND(ID_FILE_IMPORT_LEV, OnFileImportLev)
	ON_UPDATE_COMMAND_UI(ID_COUPLE_NEGY, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_COUPLE_NEGZ, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_COUPLE_POSX, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_COUPLE_POSY, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_COUPLE_POSZ, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_IMPULSE_NEGX, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_IMPULSE_NEGY, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_IMPULSE_NEGZ, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_IMPULSE_POSX, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_IMPULSE_POSY, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_IMPULSE_POSZ, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_INERTIA, OnUpdateImpulseButton)
	ON_UPDATE_COMMAND_UI(ID_FREEZE_OBJECT, OnUpdateImpulseButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
	VirtualJoystick(NULL),
	ControlledObject(NULL)
{
}

CMainFrame::~CMainFrame()
{
	REF_PTR_RELEASE(ControlledObject);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return false;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return true;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINTOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndImpulseToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndImpulseToolBar.LoadToolBar(IDR_IMPULSE_TOOLBAR))
	{
		TRACE0("Failed to create impulse toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Make the toolbar dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndImpulseToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_wndImpulseToolBar);

	// Make sure load on demand is activated
	WW3DAssetManager::Get_Instance()->Set_WW3D_Load_On_Demand (true);

	// Create the virtual joystick dialog
	VirtualJoystick = new CVJoyDialog;
	VirtualJoystick->Create(IDD_VJOY_DIALOG,this);

	return 0;
}

void CMainFrame::OnDestroy() 
{
	CFrameWnd::OnDestroy();
	
	if (VirtualJoystick != NULL) {
		VirtualJoystick->DestroyWindow();
		delete VirtualJoystick;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Save/Load

void CMainFrame::Save(ChunkSaveClass & csave)
{
	PhysControllerClass * controlptr = &(VirtualJoystick->Controller);
	csave.Begin_Chunk(MAINFRAME_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,MAINFRAME_VARIABLE_CONTROLLERPTR,controlptr);
	csave.End_Chunk();

	csave.Begin_Chunk(MAINFRAME_CHUNK_TREEVIEW);
	CDataView * data_view = (CDataView *)Splitter.GetPane(0,0);
	data_view->Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(MAINFRAME_CHUNK_GRAPHICVIEW);
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	graphic_view->Save(csave);
	csave.End_Chunk();
}

void CMainFrame::Load(ChunkLoadClass & cload)
{
	PhysControllerClass * controlptr = NULL;
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	CDataView * data_view = (CDataView *)Splitter.GetPane(0,0);
	
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {
			case MAINFRAME_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK_PTR(cload,MAINFRAME_VARIABLE_CONTROLLERPTR,controlptr);
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case MAINFRAME_CHUNK_TREEVIEW:
				data_view->Load(cload);
				break;

			case MAINFRAME_CHUNK_GRAPHICVIEW:
				graphic_view->Load(cload);
				break;
		}
		cload.Close_Chunk();
	}

	if (controlptr != NULL) {
		SaveLoadSystemClass::Register_Pointer(controlptr,&(VirtualJoystick->Controller));
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// Create the main splitter window for the application
	BOOL ok = Splitter.CreateStatic (this, 1, 2);
	ASSERT(ok);
	if (!ok) return false;

	// Create the two views
	ok &= Splitter.CreateView(0,0,RUNTIME_CLASS(CDataView),CSize(200,10),pContext);
	ok &= Splitter.CreateView(0,1,RUNTIME_CLASS(CGraphicView),CSize(340,10),pContext);

	ASSERT(ok);
	if (!ok) return false;

	CGraphicView * view = (CGraphicView *)Splitter.GetPane(0, 1);
	ASSERT(view != NULL);
	if (view == NULL) return false;

	// Set the current directory to the application's run directory.
	TCHAR szFileName[MAX_PATH];
	::GetModuleFileName (NULL, szFileName, sizeof (szFileName));
	LPTSTR pszPath = ::strrchr (szFileName, '\\');
	if (pszPath)
	{
		pszPath[0] = 0;
		::SetCurrentDirectory (szFileName);
	}

	// Initialize WW3D using the window handle for the graphic view
	ok = (WW3D::Init((HWND)*view) == WW3D_ERROR_OK);
	ASSERT(ok);
	if (!ok) return false;

#if 0
	// Show a dialog to the user asking them which
	// device they would like to use.
	CDeviceSelectionDialog deviceSelDialog;

	if (deviceSelDialog.DoModal () == IDOK) {

		// Record the user's selection from the dialog
		int device = deviceSelDialog.GetDeviceIndex();
		int bits = deviceSelDialog.GetBitsPerPixel();

		// Ask the view to initialize itself with the graphics engine
		bool ok = view->Initialize_WW3D(device,bits);
		ASSERT(ok);

	} else {

		// Stop the application from running
		return false;

	}
#else
	view->Initialize_WW3D(0,16);
#endif

	// Register any needed loaders
	WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_ParticleEmitterLoader);

	return true;
}

void CMainFrame::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	// Get a pointer to the 'graphic' pane's window
	CGraphicView * view = (CGraphicView *)Splitter.GetPane(0,1);

	// Were we successful in view's getting the pointer?
	ASSERT(view);
	if (view) {
		// Let the view know whether or not to actively update
		// its display (animation, etc)
		view->Set_Active(bActive == true);
	}

	// Allow the base class to process this message
	CFrameWnd::OnActivateApp(bActive, hTask);
	return ;	
}

void CMainFrame::OnOptionsRunSimulation() 
{
	CGraphicView * view = (CGraphicView *)Splitter.GetPane(0,1);
	ASSERT(view);
	if (view) {
		view->Enable_Simulation(!view->Is_Simulation_Enabled());
	}
}

void CMainFrame::OnUpdateOptionsRunSimulation(CCmdUI* pCmdUI) 
{
	CGraphicView * view = (CGraphicView *)Splitter.GetPane(0,1);
	ASSERT(view);
	
	if (view) {
		pCmdUI->SetCheck(view->Is_Simulation_Enabled());
	}
}

void CMainFrame::OnOptionsDisplayBoxes() 
{
	CGraphicView * view = (CGraphicView *)Splitter.GetPane(0,1);
	ASSERT(view);
	if (view) {
		view->Enable_Collision_Box_Display(!view->Is_Collision_Box_Display_Enabled());
	}
}

void CMainFrame::OnUpdateOptionsDisplayBoxes(CCmdUI* pCmdUI) 
{
	CGraphicView * view = (CGraphicView *)Splitter.GetPane(0,1);
	ASSERT(view);
	
	if (view) {
		pCmdUI->SetCheck(view->Is_Collision_Box_Display_Enabled());
	}
}

void CMainFrame::Add_Object(PhysClass * new_obj)
{
	CPhysTestDoc * doc = Get_Document();

	new_obj->Set_Transform(Matrix3D(1));
	new_obj->Set_Collision_Group(0);
	new_obj->Enable_Debug_Display(true);

	CString name;
	name.Format("%s_%2d",new_obj->Peek_Model()->Get_Name(),doc->Get_Physics_Object_Count());
	new_obj->Set_Name(name);

	if (doc != NULL) {
		doc->Add_Physics_Object(new_obj);
	}
}

CPhysTestDoc * CMainFrame::Get_Document(void)
{
	CDataView * data_view = (CDataView *)Splitter.GetPane(0,0);
	CPhysTestDoc * doc = (CPhysTestDoc *)data_view->GetDocument();
	return doc;
}

void CMainFrame::OnCreateRigidBody() 
{
	const char * model_name = Peek_Selected_Model();
	if (model_name == NULL) return;

	RenderObjClass * model = WW3DAssetManager::Get_Instance()->Create_Render_Obj(model_name);
	if (model == NULL) return;

	RigidBodyClass * obj = NEW_REF(RigidBodyClass,());
	obj->Set_Model(model);
	obj->Enable_Shadow_Generation(true);

	Add_Object(obj);
	obj->Release_Ref();
	model->Release_Ref();
}

void CMainFrame::OnUpdateCreateRigidBody(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(Peek_Selected_Model() != NULL);
}

void CMainFrame::OnCreateWheeledVehicle() 
{
	const char * model_name = Peek_Selected_Model();
	if (model_name == NULL) return;

	RenderObjClass * model = WW3DAssetManager::Get_Instance()->Create_Render_Obj(model_name);
	if (model == NULL) return;

	WheeledVehicleClass * obj = NEW_REF(WheeledVehicleClass,());
	obj->Set_Model(model);
	obj->Enable_Shadow_Generation(true);

	Add_Object(obj);
	obj->Release_Ref();
	model->Release_Ref();
}

void CMainFrame::OnUpdateCreateWheeledVehicle(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(Peek_Selected_Model() != NULL);
}

void CMainFrame::OnCreateMotorcycle() 
{
	const char * model_name = Peek_Selected_Model();
	if (model_name == NULL) return;

	RenderObjClass * model = WW3DAssetManager::Get_Instance()->Create_Render_Obj(model_name);
	if (model == NULL) return;

	MotorcycleClass * obj = NEW_REF(MotorcycleClass,());
	obj->Set_Model(model);
	obj->Enable_Shadow_Generation(true);

	Add_Object(obj);
	obj->Release_Ref();
	model->Release_Ref();
}

void CMainFrame::OnUpdateCreateMotorcycle(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(Peek_Selected_Model() != NULL);
}

const char * CMainFrame::Peek_Selected_Model(void)
{
	CDataView * data_view = (CDataView *)Splitter.GetPane(0,0);
	CPhysTestDoc * doc = (CPhysTestDoc *)data_view->GetDocument();
	if ((data_view == NULL) || (doc == NULL)) return NULL;
	
	// get the currently selected instance
	ItemInfoClass * item = data_view->Get_Selected_Item();
	if (item == NULL) return NULL;
	if (item->Type != ItemInfoClass::MODEL) return NULL;

	return item->Name;
}


PhysClass * CMainFrame::Peek_Selected_Object(void)
{
	CDataView * data_view = (CDataView *)Splitter.GetPane(0,0);
	CPhysTestDoc * doc = (CPhysTestDoc *)data_view->GetDocument();
	if ((data_view == NULL) || (doc == NULL)) return NULL;
	
	// get the currently selected instance
	ItemInfoClass * item = data_view->Get_Selected_Item();
	if (item == NULL) return NULL;
	if (item->Type != ItemInfoClass::INSTANCE) return NULL;

	PhysClass * obj = (PhysClass *)item->Instance;
	return obj;
}

MoveablePhysClass * CMainFrame::Peek_Selected_MoveablePhysClass(void)
{
	PhysClass * obj = Peek_Selected_Object();
	if (obj) {
		return obj->As_MoveablePhysClass();
	}
	return NULL;
}

RigidBodyClass * CMainFrame::Peek_Selected_RigidBodyClass(void)
{
	PhysClass * obj = Peek_Selected_Object();
	if (obj) {
		return obj->As_RigidBodyClass();
	}
	return NULL;
}

MotorVehicleClass * CMainFrame::Peek_Selected_MotorVehicleClass(void)
{
	PhysClass * obj = Peek_Selected_Object();
	if (obj) {
		return obj->As_MotorVehicleClass();
	}
	return NULL;
}

WheeledVehicleClass * CMainFrame::Peek_Selected_WheeledVehicleClass(void)
{
	PhysClass * obj = Peek_Selected_Object();
	if (obj) {
		return obj->As_WheeledVehicleClass();
	}
	return NULL;
}

MotorcycleClass * CMainFrame::Peek_Selected_MotorcycleClass(void)
{
	PhysClass * obj = Peek_Selected_Object();
	if (obj) {
		return obj->As_MotorcycleClass();
	}
	return NULL;
}

void CMainFrame::Apply_Impulse(const Vector3 & imp)
{
	RigidBodyClass * rbody = Peek_Selected_RigidBodyClass();
	if (rbody) {
		// impulses were "tuned" to work with a 1kg object, so scale by the mass
		rbody->Apply_Impulse(rbody->Get_Mass() * IMPULSE_SCALE * imp);
	}
}

void CMainFrame::Apply_Couple(const Vector3 & p0,const Vector3 & i0,const Vector3 & p1,const Vector3 & i1)
{
	RigidBodyClass * rbody = Peek_Selected_RigidBodyClass();
	if (rbody) {
		// impulses were "tuned" to work with a 1kg object, so scale by the mass
		Vector3 pos = rbody->Get_Transform().Get_Translation() + p0;
		rbody->Apply_Impulse(rbody->Get_Mass() * IMPULSE_SCALE * i0,pos);

		pos = rbody->Get_Transform().Get_Translation() + p1;
		rbody->Apply_Impulse(rbody->Get_Mass() * IMPULSE_SCALE * i1,pos);
	}
}

void CMainFrame::OnUpdateImpulseButton(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(Peek_Selected_RigidBodyClass() != NULL);
}

void CMainFrame::OnCoupleNegx() 
{
	Apply_Couple(	Vector3( 0, 1, 0),		// p0
						Vector3( 0, 0, 1),		// i0
						Vector3( 0,-1, 0),		// p1
						Vector3( 0, 0,-1)			// i1
					);
}

void CMainFrame::OnCoupleNegy() 
{
	Apply_Couple(	Vector3( 1, 0, 0),		// p0
						Vector3( 0, 0, 1),		// i0
						Vector3(-1, 0, 0),		// p1
						Vector3( 0, 0,-1)			// i1
					);
}

void CMainFrame::OnCoupleNegz() 
{
	Apply_Couple(	Vector3( 0, 1, 0),		// p0
						Vector3( 1, 0, 0),		// i0
						Vector3( 0,-1, 0),		// p1
						Vector3(-1, 0, 0)			// i1
					);
}

void CMainFrame::OnCouplePosx() 
{
	Apply_Couple(	Vector3( 0,-1, 0),		// p0
						Vector3( 0, 0, 1),		// i0
						Vector3( 0, 1, 0),		// p1
						Vector3( 0, 0,-1)			// i1
					);
}

void CMainFrame::OnCouplePosy() 
{
	Apply_Couple(	Vector3( 1, 0, 0),		// p0
						Vector3( 0, 0,-1),		// i0
						Vector3(-1, 0, 0),		// p1
						Vector3( 0, 0, 1)			// i1
					);
}

void CMainFrame::OnCouplePosz() 
{
	Apply_Couple(	Vector3( 0, 1, 0),		// p0
						Vector3(-1, 0, 0),		// i0
						Vector3( 0,-1, 0),		// p1
						Vector3( 1, 0, 0)			// i1
					);
}

void CMainFrame::OnImpulseNegx() 
{
	Apply_Impulse(Vector3(-1,0,0));
}

void CMainFrame::OnImpulseNegy() 
{
	Apply_Impulse(Vector3(0,-1,0));
}

void CMainFrame::OnImpulseNegz() 
{
	Apply_Impulse(Vector3(0,0,-1));
}

void CMainFrame::OnImpulsePosx() 
{
	Apply_Impulse(Vector3(1,0,0));
}

void CMainFrame::OnImpulsePosy() 
{
	Apply_Impulse(Vector3(0,1,0));
}

void CMainFrame::OnImpulsePosz() 
{
	Apply_Impulse(Vector3(0,0,1));
}

void CMainFrame::OnViewImpulsetoolbar() 
{
	ShowControlBar(&m_wndImpulseToolBar,!m_wndImpulseToolBar.IsWindowVisible(),false);
}

void CMainFrame::OnUpdateViewImpulsetoolbar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_wndImpulseToolBar.IsWindowVisible());
}

void CMainFrame::OnViewVirtualjoystick() 
{
	if (VirtualJoystick) {
		if (VirtualJoystick->IsWindowVisible()) {
			VirtualJoystick->ShowWindow(SW_HIDE);
		} else {
			VirtualJoystick->ShowWindow(SW_SHOW);
		}
	}
}

void CMainFrame::OnUpdateViewVirtualjoystick(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(VirtualJoystick && VirtualJoystick->IsWindowVisible());
}

void CMainFrame::OnInertia() 
{
	RigidBodyClass * rbody = Peek_Selected_RigidBodyClass();
	if (rbody == NULL) return;
	CInertiaDialog dialog(this,rbody);
	dialog.DoModal();
}

void CMainFrame::OnProperties() 
{
	MoveablePhysClass * obj = Peek_Selected_MoveablePhysClass();
	if (obj == NULL) return;
	CRbodyPropertiesDialog dialog(this,obj);
	dialog.DoModal();
}

void CMainFrame::OnUpdateProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(Peek_Selected_MoveablePhysClass() != NULL);
}

void CMainFrame::OnMotorProperties() 
{
	MotorVehicleClass * vehicle = Peek_Selected_MotorVehicleClass();
	if (vehicle == NULL) return;
	CMotorVehicleDialog dialog(this,vehicle);
	dialog.DoModal();
}

void CMainFrame::OnUpdateMotorProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(Peek_Selected_MotorVehicleClass() != NULL);
}

void CMainFrame::OnMotorcycleProperties() 
{
	MotorcycleClass * cycle = Peek_Selected_MotorcycleClass();
	if (cycle == NULL) return;
	CMotorcycleDialog dialog(this,cycle);
	dialog.DoModal();
}

void CMainFrame::OnUpdateMotorcycleProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(Peek_Selected_MotorVehicleClass() != NULL);
}

void CMainFrame::OnWheelProperties() 
{
	WheeledVehicleClass * vehicle = Peek_Selected_WheeledVehicleClass();
	if (vehicle == NULL) return;
	CWheeledVehicleDialog dialog(this,vehicle);
	dialog.DoModal();
}

void CMainFrame::OnUpdateWheelProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(Peek_Selected_WheeledVehicleClass() != NULL);
}

void CMainFrame::OnOptionsPhysicsConstants() 
{
	CPhysicsConstantsDialog dialog;
	dialog.DoModal();
}

void CMainFrame::OnFreezeObject() 
{
	CDataView * data_view = (CDataView *)Splitter.GetPane(0,0);
	CPhysTestDoc * doc = (CPhysTestDoc *)data_view->GetDocument();

	ItemInfoClass * item = data_view->Get_Selected_Item();
	if (item == NULL) return;
	if (item->Type != ItemInfoClass::INSTANCE) return;
	
	RigidBodyClass * rbody = (RigidBodyClass *)item->Instance;
	if (rbody == NULL) return;

	rbody->Set_Velocity(Vector3(0,0,0));
	rbody->Set_Angular_Velocity(Vector3(0,0,0));
}

void CMainFrame::Set_Status_Bar_Text(const char * text)
{
	m_wndStatusBar.SetWindowText(text);
}

void CMainFrame::OnDebugObject() 
{
	PhysClass * obj = Peek_Selected_Object();
	obj->Enable_Debug_Display(!obj->Is_Debug_Display_Enabled());
}

void CMainFrame::OnUpdateDebugObject(CCmdUI* pCmdUI) 
{
	PhysClass * obj = Peek_Selected_Object();
	if (obj == NULL) {
		pCmdUI->Enable(false);
	} else {
		pCmdUI->Enable(true);
		pCmdUI->SetCheck(obj->Is_Debug_Display_Enabled());
	}
}

void CMainFrame::OnOptionsFilled() 
{
	PhysicsSceneClass::Get_Instance()->Set_Polygon_Mode(SceneClass::FILL);	
}

void CMainFrame::OnUpdateOptionsFilled(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(PhysicsSceneClass::Get_Instance()->Get_Polygon_Mode() == SceneClass::FILL);
}

void CMainFrame::OnOptionsPoints() 
{
	PhysicsSceneClass::Get_Instance()->Set_Polygon_Mode(SceneClass::POINT);	
}

void CMainFrame::OnUpdateOptionsPoints(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(PhysicsSceneClass::Get_Instance()->Get_Polygon_Mode() == SceneClass::POINT);	
}

void CMainFrame::OnOptionsWireframe() 
{
	PhysicsSceneClass::Get_Instance()->Set_Polygon_Mode(SceneClass::LINE);
}

void CMainFrame::OnUpdateOptionsWireframe(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(PhysicsSceneClass::Get_Instance()->Get_Polygon_Mode() == SceneClass::LINE);
}

void CMainFrame::Notify_Selection_Changed(void)
{
	if (ControlledObject != NULL) {
		ControlledObject->Set_Controller(NULL);
		ControlledObject->Release_Ref();
		ControlledObject = NULL;
	}
	ControlledObject = Peek_Selected_MoveablePhysClass();
	if (ControlledObject) {
		ControlledObject->Add_Ref();
		if (VirtualJoystick) {
			ControlledObject->Set_Controller(&VirtualJoystick->Controller);
		}
	}
}

void CMainFrame::OnCameraFly() 
{
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	if (graphic_view) {
		graphic_view->Set_Camera_Mode(CGraphicView::CAMERA_FLY);
	}
}

void CMainFrame::OnUpdateCameraFly(CCmdUI* pCmdUI) 
{
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	if (graphic_view && graphic_view->Get_Camera_Mode() == CGraphicView::CAMERA_FLY) {
		pCmdUI->SetCheck(true);
	} else {
		pCmdUI->SetCheck(false);
	}
}

void CMainFrame::OnCameraFollow() 
{
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	if (graphic_view) {
		graphic_view->Set_Camera_Mode(CGraphicView::CAMERA_FOLLOW);
	}
}

void CMainFrame::OnUpdateCameraFollow(CCmdUI* pCmdUI) 
{
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	if (graphic_view && graphic_view->Get_Camera_Mode() == CGraphicView::CAMERA_FOLLOW) {
		pCmdUI->SetCheck(true);
	} else {
		pCmdUI->SetCheck(false);
	}
}

void CMainFrame::OnCameraTether() 
{
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	if (graphic_view) {
		graphic_view->Set_Camera_Mode(CGraphicView::CAMERA_TETHER);
	}
}

void CMainFrame::OnUpdateCameraTether(CCmdUI* pCmdUI) 
{
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	if (graphic_view && graphic_view->Get_Camera_Mode() == CGraphicView::CAMERA_TETHER) {
		pCmdUI->SetCheck(true);
	} else {
		pCmdUI->SetCheck(false);
	}
}

void CMainFrame::OnCameraRigidTether() 
{
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	if (graphic_view) {
		graphic_view->Set_Camera_Mode(CGraphicView::CAMERA_RIGID_TETHER);
	}
}

void CMainFrame::OnUpdateCameraRigidTether(CCmdUI* pCmdUI) 
{
	CGraphicView * graphic_view = (CGraphicView *)Splitter.GetPane(0,1);
	if (graphic_view && graphic_view->Get_Camera_Mode() == CGraphicView::CAMERA_RIGID_TETHER) {
		pCmdUI->SetCheck(true);
	} else {
		pCmdUI->SetCheck(false);
	}
}



void CMainFrame::OnOptionsRenderDevice() 
{
	CRenderDeviceDialog dialog;
	dialog.DoModal();
}

void CMainFrame::OnUpdateOptionsRenderDevice(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(WW3D::Is_Initted());
}


void CMainFrame::OnFileImportModel() 
{
 	// show the file open dialong
	CFileDialog file_dialog(	true,    // bOpenFileDialog, 
										".W3D",	// LPCTSTR lpszDefExt = NULL, 
										NULL,    // LPCTSTR lpszFileName = NULL, 
										OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
										"Westwood 3D Files|*.w3d|"
									);
	
	// read in the file
	if (file_dialog.DoModal()) {
		Get_Document()->Load_W3D_File(file_dialog.GetPathName()); 
	}	
}

void CMainFrame::OnFileImportLev() 
{
 	// show the file open dialong
	CFileDialog file_dialog(	true,    // bOpenFileDialog, 
										".LEV",	// LPCTSTR lpszDefExt = NULL, 
										NULL,    // LPCTSTR lpszFileName = NULL, 
										OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
										"LEV Files|*.lev|"
									);
	
	// read in the file
	if (file_dialog.DoModal()) {
		Get_Document()->Load_LEV_File(file_dialog.GetPathName()); 
	}	
}

