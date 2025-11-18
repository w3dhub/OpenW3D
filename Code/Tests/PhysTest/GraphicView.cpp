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

// GraphicView.cpp : implementation of the CGraphicView class
//

#include "stdafx.h"
#include "PhysTest.h"
#include "PhysTestDoc.h"
#include "GraphicView.h"
#include "MainFrm.h"
#include "mmsystem.h"
#include "pscene.h"
#include "ww3d.h"
#include "camera.h"
#include "quat.h"
#include "rcfile.h"
#include "assetmgr.h"
#include "rbody.h"
#include "chunkio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define ORTHO_CAMERA	0
#define ORTHO_WIDTH	20


////////////////////////////////////////////////////////////////////////////
//
//  CHUNK ID's used by GraphicView
//
enum 
{
	GRAPHICVIEW_CHUNK_VARIABLES			= 0x00789931,

	GRAPHICVIEW_VARIABLE_CAMERAMODE		= 0x00,
	GRAPHICVIEW_VARIABLE_DISPLAYBOXES,
	GRAPHICVIEW_VARIABLE_RUNSIMULATION,
	GRAPHICVIEW_VARIABLE_CAMERATM,
};


const float PIP_VIEW = 0.2f;		// size of the pip viewport

////////////////////////////////////////////////////////////////////////////
//
//  TimerCallback
//
void CALLBACK TimerCallback
(
	UINT uID,
	UINT uMsg,
	DWORD dwUser,
	DWORD dw1,
	DWORD dw2
)
{
	HWND hwnd = (HWND)dwUser;
	if (IsWindow(hwnd)) {

		// Send this event off to the view to process (hackish, but fine for now)
		if ((GetProp (hwnd, "WaitingToProcess") == NULL) &&
			 (GetProp (hwnd, "Inactive") == NULL)) {

			SetProp (hwnd, "WaitingToProcess", (HANDLE)1);

			// Send the message to the view
			::PostMessage (hwnd, WM_USER + 101, 0, 0L);
		}
	}
}


////////////////////////////////////////////////////////////////////////////
//
//  WindowProc
//
LRESULT CGraphicView::WindowProc
(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
	// Is this the repaint message we are expecting?
	if (message == WM_USER+101) {
		Timestep();
		Repaint_View();
		RemoveProp(m_hWnd,"WaitingToProcess");
	} 

	// Allow the base class to process this message
	return CView::WindowProc(message, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
// CGraphicView

IMPLEMENT_DYNCREATE(CGraphicView, CView)

BEGIN_MESSAGE_MAP(CGraphicView, CView)
	//{{AFX_MSG_MAP(CGraphicView)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphicView construction/destruction

CGraphicView::CGraphicView() :
	Initialized(false),
	Active(true),
	RunSimulation(false),
	DisplayBoxes(false),
	CameraMode(CAMERA_FLY),
	Camera(NULL),
	TimerID(0),
	LMouseDown(false),
	RMouseDown(false),
	LastPoint(0,0),
	PipCamera(NULL),
	PipScene(NULL),
	Axes(NULL)
{
}

CGraphicView::~CGraphicView()
{
	REF_PTR_RELEASE(Camera);
	REF_PTR_RELEASE(PipCamera);
	REF_PTR_RELEASE(PipScene);
	REF_PTR_RELEASE(Axes);
}

BOOL CGraphicView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

BOOL CGraphicView::Initialize_WW3D(int device,int bits)
{
	// Assume failure
	BOOL ok = false;
	if (device < 0) {
		return false;
	}
	
	// Initialize the rendering engine with the information from
	// this window.
	RECT rect;
	GetClientRect (&rect);

	int cx = rect.right-rect.left;
	int cy = rect.bottom-rect.top;

	WW3DErrorType err = WW3D_ERROR_GENERIC;
	while (err != WW3D_ERROR_OK) {
		err = WW3D::Set_Render_Device(device,cx,cy,bits,true);
		if (err != WW3D_ERROR_OK) {
			device = (device + 1) % WW3D::Get_Render_Device_Count();
		}
	}

	// Load some models that we're going to need into the asset manager
	ResourceFileClass point_file(NULL, "Point.w3d");
	WW3DAssetManager::Get_Instance()->Load_3D_Assets(point_file);
	ResourceFileClass axes_file(NULL, "Axes.w3d");
	WW3DAssetManager::Get_Instance()->Load_3D_Assets(axes_file);

	if (Camera == NULL) {
		Camera = NEW_REF(CameraClass,());
		ASSERT(Camera);

		// Init the camera
		Matrix3D transform;
		transform.Look_At(Vector3(0.0f,-15.0f,5.0f),Vector3(0,0,0),0);
		Camera->Set_Transform(transform);

		// update the camera FOV settings
		// take the larger of the two dimensions, give it the
		// full desired FOV, then give the other dimension an
		// FOV proportional to its relative size
		double hfov,vfov;
		if (cy > cx) {						
			vfov = (float)DEG_TO_RAD(45.0f);
			hfov = (double)cx / (double)cy * vfov;
		} else {		
			hfov = (float)DEG_TO_RAD(45.0f);
			vfov = (double)cy / (double)cx * hfov;
		}

		// Reset the field of view
		Camera->Set_View_Plane(hfov,vfov);

#if ORTHO_CAMERA
		Camera->Set_Projection_Type(CameraClass::ORTHO);
		Camera->Set_View_Plane(Vector2(-ORTHO_WIDTH,-ORTHO_WIDTH),Vector2(ORTHO_WIDTH,ORTHO_WIDTH));
#endif
	}

	if (PipCamera == NULL) {
		PipCamera = NEW_REF(CameraClass,());
		ASSERT(PipCamera);
		PipCamera->Set_Viewport(Vector2(0.0f,1.0f - PIP_VIEW),Vector2(PIP_VIEW,1.0f));
		PipCamera->Set_View_Plane(Camera->Get_Horizontal_FOV(),Camera->Get_Vertical_FOV());
		Update_Pip_Camera();
	}

	if (PipScene == NULL) {
		PipScene = NEW_REF(SimpleSceneClass,());
		PipScene->Set_Ambient_Light(Vector3(1,1,1));
		ASSERT(PipScene);
	}

	if (Axes == NULL) {
		Axes = WW3DAssetManager::Get_Instance()->Create_Render_Obj("Axes");
		ASSERT(Axes);
		Axes->Set_Transform(Matrix3D(1));
		PipScene->Add_Render_Object(Axes);
	}

	// Kick off a timer that we can use to update
	// the display (kinda like a game loop iterator)
	if (TimerID == 0) {
#if 0
		TimerID = (UINT)::timeSetEvent(	50,
													50,
													TimerCallback,
													(DWORD)m_hWnd,
													TIME_PERIODIC);
#else
		TimerID = (UINT)::timeSetEvent(	66,						// only update 15 times a second
													50,
													TimerCallback,
													(DWORD)m_hWnd,
													TIME_PERIODIC);
#endif
	}

	Initialized = true;

	// Return the true/false result code
	return true;
}

void CGraphicView::Save(ChunkSaveClass & csave)
{
	Matrix3D cameratm = Camera->Get_Transform();
	csave.Begin_Chunk(GRAPHICVIEW_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,GRAPHICVIEW_VARIABLE_CAMERAMODE,CameraMode);
	WRITE_MICRO_CHUNK(csave,GRAPHICVIEW_VARIABLE_DISPLAYBOXES,DisplayBoxes);
	WRITE_MICRO_CHUNK(csave,GRAPHICVIEW_VARIABLE_RUNSIMULATION,RunSimulation);
	WRITE_MICRO_CHUNK(csave,GRAPHICVIEW_VARIABLE_CAMERATM,cameratm);
	csave.End_Chunk();
}

void CGraphicView::Load(ChunkLoadClass & cload)
{
	Matrix3D cameratm(1);

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {
		case GRAPHICVIEW_CHUNK_VARIABLES:
			while(cload.Open_Micro_Chunk()) {
				switch(cload.Cur_Micro_Chunk_ID()) {
					READ_MICRO_CHUNK(cload,GRAPHICVIEW_VARIABLE_CAMERAMODE,CameraMode);
					READ_MICRO_CHUNK(cload,GRAPHICVIEW_VARIABLE_DISPLAYBOXES,DisplayBoxes);
					READ_MICRO_CHUNK(cload,GRAPHICVIEW_VARIABLE_RUNSIMULATION,RunSimulation);
					READ_MICRO_CHUNK(cload,GRAPHICVIEW_VARIABLE_CAMERATM,cameratm);
				}
				cload.Close_Micro_Chunk();
			}
			break;
		default:
			WWDEBUG_SAY(("Unhandled chunk: %d File: %s Line: %d\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
		}
		cload.Close_Chunk();
	}

	Camera->Set_Transform(cameratm);
}


/////////////////////////////////////////////////////////////////////////////
// CGraphicView drawing

void CGraphicView::OnDraw(CDC* pDC)
{
	Repaint_View();
}

/////////////////////////////////////////////////////////////////////////////
// CGraphicView diagnostics

#ifdef _DEBUG
void CGraphicView::AssertValid() const
{
	CView::AssertValid();
}

void CGraphicView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPhysTestDoc* CGraphicView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPhysTestDoc)));
	return (CPhysTestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGraphicView message handlers

void CGraphicView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if (Initialized)
	{
		// Change the resolution of the rendering device to 
		// match that of the view's current dimensions
		WW3D::Set_Resolution (cx, cy, -1, true);
 
		// update the camera FOV settings
		// take the larger of the two dimensions, give it the
		// full desired FOV, then give the other dimension an
		// FOV proportional to its relative size
		double hfov,vfov;
		if (cy > cx) {						
			vfov = (float)DEG_TO_RAD(45.0f);
			hfov = (double)cx / (double)cy * vfov;
		} else {		
			hfov = (float)DEG_TO_RAD(45.0f);
			vfov = (double)cy / (double)cx * hfov;
		}

		// Reset the field of view
		ASSERT(Camera);
		Camera->Set_View_Plane(hfov, vfov);
#if ORTHO_CAMERA
		Camera->Set_View_Plane(Vector2(-ORTHO_WIDTH,-ORTHO_WIDTH),Vector2(ORTHO_WIDTH,ORTHO_WIDTH));
#endif
		PipCamera->Set_View_Plane(hfov,vfov);

		// Force a repaint of the screen
		Repaint_View();
	}
}

void CGraphicView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	CPhysTestDoc * doc = (CPhysTestDoc *)GetDocument();
	if (doc) {
		// tell the document to create the scene
		doc->Init_Scene();
	}
}

void CGraphicView::Timestep(void)
{
	const float FLY_VELOCITY = 10.0f;

	// Compute the amount of time elapsed for this frame.
	CPhysTestDoc * doc = (CPhysTestDoc *)GetDocument();
	DWORD curtime = ::TIMEGETTIME();
	DWORD elapsedtime = curtime - doc->LastTime;
	if (elapsedtime > 100) {
		elapsedtime = 100;
	}
	float dt = (float)elapsedtime / 1000.0f;
	doc->LastTime = curtime;	

	// fly forward when control is held
	if (::GetAsyncKeyState('A') & 0x8000) {
		Matrix3D transform = Camera->Get_Transform();
		transform.Translate(Vector3(0,0,-FLY_VELOCITY * dt));
		Camera->Set_Transform(transform);
	}

	if (::GetAsyncKeyState('Z') & 0x8000) {
		Matrix3D transform = Camera->Get_Transform();
		transform.Translate(Vector3(0,0,FLY_VELOCITY * dt));
		Camera->Set_Transform(transform);
	}

	// if mode == follow and we have an object selected, look at it
	if (CameraMode == CAMERA_FOLLOW) {
		CMainFrame * wnd = (CMainFrame *)::AfxGetMainWnd();
		PhysClass * obj = wnd->Peek_Selected_Object();
		if (obj) {
			Vector3 target;
			obj->Get_Transform().Get_Translation(&target);
			Vector3 pos;
			Camera->Get_Transform().Get_Translation(&pos);
			Matrix3D tm;
			tm.Look_At(pos,target,0.0f);
			Camera->Set_Transform(tm);
		}
	}

	// if mode == tether and we have an object selected, tie the camera to it
	if ((CameraMode == CAMERA_TETHER) || (CameraMode == CAMERA_RIGID_TETHER)) {
		CMainFrame * wnd = (CMainFrame *)::AfxGetMainWnd();
		PhysClass * obj = wnd->Peek_Selected_Object();
		if (obj) {
			const float TETHER_DIST = 15.0f;
			const float TETHER_ANGLE = DEG_TO_RADF(-20.0f);
			Vector3 pos;
			float zrot = obj->Get_Transform().Get_Z_Rotation();
			obj->Get_Transform().Get_Translation(&pos);	
			
			Matrix3D tm(1);
			
			if (CameraMode == CAMERA_RIGID_TETHER) {
				tm = obj->Get_Transform();
			} else {
				tm.Translate(pos);
				tm.Rotate_Z(zrot);
			}
			
			tm.Rotate_Y(DEG_TO_RADF(-90.0f));
			tm.Rotate_Z(DEG_TO_RADF(-90.0f));
			tm.Rotate_X(TETHER_ANGLE);
			tm.Translate(Vector3(0,0,TETHER_DIST));
			Camera->Set_Transform(tm);
		}
	}
	
	// update the PIP camera transform
	Update_Pip_Camera();

	// Allow the world to simulate (if enabled)
	if (RunSimulation) {
		doc->Scene->Update(1.0f/15.0f , 0);  // 0.05f,0); //dt,0);
	}

	// DEBUG, print the contact point coordinates
#if 0
	Vector3 pt = _LastContactPoint;
	CString string;
	string.Format("Contact Point: %10.5f %10.5f %10.5f   StartBad: %d",pt.X,pt.Y,pt.Z,_StartBad);
	((CMainFrame *)::AfxGetMainWnd())->Set_Status_Bar_Text(string);
#endif
}

void CGraphicView::Repaint_View(void)
{
	static bool _already_painting = false;
	if (_already_painting) return;
	_already_painting = true;

	// Get the document to display
	CPhysTestDoc * doc = (CPhysTestDoc *)GetDocument();

	// Are we in a valid state?
	if (Initialized && doc->Scene)	{

		// Update the W3D frame times according to our elapsed tick count
		//WW3D::Sync(WW3D::Get_Sync_Time() + (ticks_elapsed * m_animationSpeed));

		// Render the scenes
		WW3D::Begin_Render(true, true, Vector3(0,0,0));
		WW3D::Render(doc->Scene,Camera,false,false);
		WW3D::Render(PipScene,PipCamera,false,true);
		WW3D::End_Render();
	}        

	_already_painting = false;
}

void CGraphicView::Set_Active(bool onoff)
{
	Active = onoff;
	if (!Active) { 
		::SetProp(m_hWnd,"Inactive",(HANDLE)1); 
	} else { 
		RemoveProp(m_hWnd,"Inactive"); 
		CPhysTestDoc * doc = (CPhysTestDoc *)GetDocument();
		doc->LastTime = ::TIMEGETTIME();	
	}
}

bool CGraphicView::Is_Collision_Box_Display_Enabled(void)
{
	return DisplayBoxes;
}

void CGraphicView::Enable_Collision_Box_Display(bool onoff)
{
	DisplayBoxes = onoff;
	if (DisplayBoxes) {
		WW3D::Set_Collision_Box_Display_Mask(COLLISION_TYPE_PHYSICAL);
	} else {
		WW3D::Set_Collision_Box_Display_Mask(0);
	}

}

void CGraphicView::OnDestroy() 
{
	// remove our properties
	::RemoveProp(m_hWnd,"Inactive"); 
	::RemoveProp(m_hWnd,"WaitingToProcess");

	CView::OnDestroy();
	
	// Free the camera object
	REF_PTR_RELEASE(Camera);
	REF_PTR_RELEASE(PipScene);
	REF_PTR_RELEASE(PipCamera);
	REF_PTR_RELEASE(Axes);

	// Is there an update thread running?
	if (TimerID == 0) {
		// Stop the timer
		::timeKillEvent((UINT)TimerID);
		TimerID = 0;        
	}

	Initialized = false;    
}


void CGraphicView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Capture all mouse messages
	SetCapture();

	// Left mouse button is down
	LMouseDown = true;
	LastPoint = point;

	// Allow the base class to process this message
	CView::OnLButtonDown(nFlags, point);
}

void CGraphicView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// if both buttons are now up, release the mouse
	if (!RMouseDown) {
		ReleaseCapture();
	}

	// Left mouse button is now up
	LMouseDown = false;

	CView::OnLButtonUp(nFlags, point);
}

void CGraphicView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// Capture all mouse messages
	SetCapture();

	// Right mouse button is down
	RMouseDown = true;
	LastPoint = point;
	
	CView::OnRButtonDown(nFlags, point);
}

void CGraphicView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// if both buttons are now up, release the mouse
	if (!LMouseDown) {
		ReleaseCapture();
	}

	// Right mouse button is now up
	RMouseDown = false;
	
	CView::OnRButtonUp(nFlags, point);
}


void CGraphicView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Get the document to display
	CPhysTestDoc * doc = (CPhysTestDoc *)GetDocument();
	WWASSERT(doc);
#if 0
	PhysicsSceneClass * scene = doc->Scene;
	WWASSERT(scene);
#endif

	// When Only the Left mouse button is down, we "look-around" with
	// vertical locking (rotate about Z until Y points up)
	if ((LMouseDown) && (!RMouseDown)) {

		// Are we in a valid state?
		if (Initialized && doc->Scene) {
			
			RECT rect;
			GetClientRect (&rect);

			float mid_point_x = float(rect.right >> 1);
			float mid_point_y = float(rect.bottom >> 1);

			float last_point_x = ((float)LastPoint.x - mid_point_x) / mid_point_x;
			float last_point_y = (mid_point_y - (float)LastPoint.y) / mid_point_y;

			float point_x = ((float)point.x - mid_point_x) / mid_point_x;
			float point_y = (mid_point_y - (float)point.y) / mid_point_y;

			// invert the axes (being lazy here...)
			point_x = -point_x;
			point_y = -point_y;
			last_point_x = -last_point_x;
			last_point_y = -last_point_y;

			// Rotate around the object (orbit) using a 0.00F - 1.00F percentage of
			// the mouse coordinates
			Quaternion rotation = ::Trackball (last_point_x,last_point_y,point_x,point_y, 0.8F);

			// If we're not in CAMERA_FOLLOW mode, just rotate our view
			if (CameraMode != CAMERA_FOLLOW) {
				
				// Apply the rotation to the camera
				Matrix3D transform = Camera->Get_Transform ();
				Matrix3D::Multiply(transform,Build_Matrix3D(rotation),&transform);

				// Straighten out the Y axis
				Vector3 pos = transform.Get_Translation();
				Vector3 target = transform * Vector3(0,0,-1);
				transform.Look_At(pos,target,0.0f);

				// Rotate and translate the camera
				Camera->Set_Transform (transform);
			
			} else {

				// We're in camera follow mode, this means the camera is always going
				// to point right at the object.  We want to orbit the camera's position
				// around the object.
				Matrix3D camera_tm = Camera->Get_Transform();
				Vector3 relative_obj_pos(0,0,-10);

				CMainFrame * wnd = (CMainFrame *)::AfxGetMainWnd();
				PhysClass * obj = wnd->Peek_Selected_Object();
				if (obj) {

					Vector3 obj_pos;
					obj->Get_Transform().Get_Translation(&obj_pos);
					Matrix3D::Inverse_Transform_Vector(camera_tm,obj_pos,&relative_obj_pos);
	
				} 

				camera_tm.Translate(relative_obj_pos);
				Matrix3D rotation_tm;
				Build_Matrix3D(rotation).Get_Orthogonal_Inverse(rotation_tm);
				Matrix3D::Multiply(camera_tm,rotation_tm,&camera_tm);
				camera_tm.Translate(-relative_obj_pos);

				Camera->Set_Transform(camera_tm);
			}
		}        
	}

	LastPoint = point;
	Update_Pip_Camera();

	CView::OnMouseMove(nFlags, point);
}

void CGraphicView::Update_Pip_Camera(void)
{
	if (PipCamera && Camera) {
		Matrix3D transform = Camera->Get_Transform();
		transform.Set_Translation(Vector3(0,0,0));
		transform.Translate(Vector3(0,0,4));
		PipCamera->Set_Transform(transform);
	}
}
