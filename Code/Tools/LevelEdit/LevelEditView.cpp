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

// LevelEditView.cpp : implementation of the CLevelEditView class
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "leveleditdoc.h"
#include "LevelEditView.h"
#include "camera.h"
#include "cameramgr.h"
#include "mainfrm.h"
#include "ww3d.h"
#include "MouseMgr.h"
#include "sceneeditor.h"
#include <process.h>
#include "nodemgr.h"
#include "mmsystem.h"
#include "dynamesh.h"
#include "vertmaterial.h"
#include "win.h"
#include "WWAudio.h"
#include "combat.h"
#include "timemgr.h"
#include "gameobjmanager.h"
#include "gameobjobserver.h"
#include "scriptman.h"
#include "backgroundmgr.h"
#include "dazzle.h"
#include "phys.h"
#include "WeatherMgr.h"
#include "SoundEnvironment.h"
#include "heightfieldeditor.h"


/////////////////////////////////////////////////////////////////////////
//  Local Prototypes
/////////////////////////////////////////////////////////////////////////
void CALLBACK fnTimerCallback (UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);


/////////////////////////////////////////////////////////////////////////
//  Static member initialization
/////////////////////////////////////////////////////////////////////////
int	CLevelEditView::_iPaintingLock	= 0;
bool	CLevelEditView::_bNeedsRepaint	= false;

/////////////////////////////////////////////////////////////////////////
//	Globals
/////////////////////////////////////////////////////////////////////////
CRITICAL_SECTION _CriticalSection;


IMPLEMENT_DYNCREATE(CLevelEditView, CView)

BEGIN_MESSAGE_MAP(CLevelEditView, CView)
	//{{AFX_MSG_MAP(CLevelEditView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////
//
//	CLevelEditView
//
/////////////////////////////////////////////////////////////////////////
CLevelEditView::CLevelEditView (void)
	: m_iDeviceIndex (-1),
	  m_iBitsPerPixel (16),
	  m_iWindowed (1),
	  m_bRenderDeviceInitialized (false),
	  m_pCameraMgr (NULL),
	  m_pMouseMgr (NULL),
	  m_bActive (false),
	  m_p2DScene (NULL),
	  m_p2DCamera (NULL),
	  m_TimerID (0),
	  m_HorzFOV (0),
	  m_VertFOV (0),
	  m_CursorXRatio (1),
	  m_CursorYRatio (1),
	  m_hMouseHook (NULL)
{
    // Get the windowed mode from the registry
    CString string_windowed = theApp.GetProfileString ("Config", "Windowed", "1");
	 m_iWindowed = ::atoi ((LPCTSTR)string_windowed) || Is_Silent_Mode ();


	::InitializeCriticalSection (&_CriticalSection);
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	~CLevelEditView
//
/////////////////////////////////////////////////////////////////////////
CLevelEditView::~CLevelEditView (void)
{
	// If the camera manager exists, free it.
	SAFE_DELETE (m_pCameraMgr);
	SAFE_DELETE (m_pMouseMgr);	

	m_pCursorMesh->Remove ();
	MEMBER_RELEASE (m_pCursorMesh);
	MEMBER_RELEASE (m_p2DScene);
	MEMBER_RELEASE (m_p2DCamera);

    // Cache this information in the registry
	 TCHAR temp_string[12];
	 ::itoa (m_iWindowed, temp_string, 10);
    theApp.WriteProfileString ("Config", "Windowed", temp_string);

	::DeleteCriticalSection (&_CriticalSection);
	return ;
}

/////////////////////////////////////////////////////////////////////////
//
//	PreCreateWindow
//
/////////////////////////////////////////////////////////////////////////
BOOL
CLevelEditView::PreCreateWindow (CREATESTRUCT& cs)
{
	// Allow the base class to process this message	
	return CView::PreCreateWindow (cs);
}


/////////////////////////////////////////////////////////////////////////
//
//	OnDraw
//
void
CLevelEditView::OnDraw (CDC* pDC)
{
	CLevelEditDoc* doc = GetDocument();
	ASSERT_VALID(doc);

	// If we are in fullscreen mode, then erase the window background
	if (m_iWindowed == 0) {

		// Get the client rectangle of the window
		RECT rect;
		GetClientRect (&rect);

		// Erase the background
		::FillRect (*pDC, &rect, (HBRUSH)(COLOR_WINDOW + 1));
	}

	// Ask the view to repaint itself.
	Repaint_View ();
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	MFC Junk
//
/////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void CLevelEditView::AssertValid() const
{
	CView::AssertValid();
}

void CLevelEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CLevelEditDoc* CLevelEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLevelEditDoc)));
	return (CLevelEditDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////
//
//	Initialize_Render_Device
//
/////////////////////////////////////////////////////////////////////////
bool
CLevelEditView::Initialize_Render_Device (void)
{
	::EnterCriticalSection (&_CriticalSection);

	// Assume failure
	m_bRenderDeviceInitialized = false;
	
	// Initialize the rendering engine with the information from
	// this window.
	RECT rect;
	GetClientRect (&rect);

	int cx = rect.right-rect.left;
	int cy = rect.bottom-rect.top;
	int bpp = m_iBitsPerPixel;
	if (m_iWindowed == 0) {
		cx = 640;
		cy = 480;
		bpp = 16;

		// Determine what our aspect ratio should be to
		// constrain the mouse to the 'window' while in full screen mode.
		m_CursorXRatio = cx / ((float)rect.right);
		m_CursorYRatio = cy / ((float)rect.bottom);
	} else {
		m_CursorXRatio = 1.0F;
		m_CursorYRatio = 1.0F;
	}

	// Ask W3D to use this rendering device
	m_bRenderDeviceInitialized = (WW3D::Set_Render_Device (m_iDeviceIndex,
																		    cx,
																		    cy,
																		    bpp,
																		    m_iWindowed) == WW3D_ERROR_OK);

	ASSERT (m_bRenderDeviceInitialized);

	// Create a new camera manager object
	if (m_pCameraMgr == NULL) {
		m_pCameraMgr = new CameraMgr;
	}

	// Create a new mouse manager object
	if (m_pMouseMgr == NULL) {
		m_pMouseMgr = new MouseMgrClass;
	}

	// Reset the camera's FOV
	m_pCameraMgr->Get_Camera ()->Set_View_Plane (m_HorzFOV, m_VertFOV);

	if (m_p2DCamera == NULL) {
		
		// Create a new instance of the camera class to use
		// when rendering the background BMP
		m_p2DCamera = new CameraClass ();
        
		// Were we successful in creating the new instance?
		ASSERT (m_p2DCamera != NULL);
		if (m_p2DCamera != NULL) {

			// Set the default values for the new camera
			m_p2DCamera->Set_View_Plane (Vector2 (-1.00F, -1.00F), Vector2 (1.00F, 1.00F));
			m_p2DCamera->Set_Position (Vector3 (0.00F, 0.00F, 1.00F));
			m_p2DCamera->Set_Clip_Planes (0.1F, 10.0F);
			//m_p2DCamera->Set_Position (Vector3 (0.00F, 0.00F, 1.00F));
			//m_p2DCamera->Set_Transform (Matrix3D (1));
		}
	}

	if (m_p2DScene == NULL) {

		// Instantiate a new scene
		m_p2DScene = new SimpleSceneClass;

		// Allocate a new mesh we can use to represent the mouse
		//.cursor while in full screen mode
		m_pCursorMesh = new DynamicScreenMeshClass (2, 6);
		//m_pCursorMesh->Set_Force_Visible (true);

		VertexMaterialClass *vmat = new VertexMaterialClass;

		vmat->Set_Emissive (1,1,1);
		vmat->Set_Diffuse (0,0,0);

		m_pCursorMesh->Set_Vertex_Material (vmat);
		MEMBER_RELEASE (vmat);

		m_pCursorMesh->Begin_Tri_Strip();

			m_pCursorMesh->Set_Vertex_Color (Vector3 (0, 0.384F, 0.5F));
			m_pCursorMesh->Vertex (0.0F,	0.0F, 0, 0, 0);			
			m_pCursorMesh->Set_Vertex_Color (Vector3 (0, 0, 0.5F));
			m_pCursorMesh->Vertex (0.026F,0.028F, 0, 0, 0);
			m_pCursorMesh->Set_Vertex_Color (Vector3 (0, 0.5F, 1));
			m_pCursorMesh->Vertex (0.010F,0.024F, 0, 0, 0);

		m_pCursorMesh->End_Tri_Strip();


		m_pCursorMesh->Begin_Tri_Strip();

			m_pCursorMesh->Set_Vertex_Color (Vector3 (0, 0.5F, 1));
			m_pCursorMesh->Vertex (0.010F,0.024F, 0, 0, 0);
			m_pCursorMesh->Set_Vertex_Color (Vector3 (0.53F, 0.76F, 1));
			m_pCursorMesh->Vertex (0.0F,	0.041F, 0, 0, 0);
			m_pCursorMesh->Set_Vertex_Color (Vector3 (0, 0.5F, 1));
			m_pCursorMesh->Vertex (0.0F,	0.0F, 0, 0, 0);

		m_pCursorMesh->End_Tri_Strip();

		// Add the cursor to the 2D scene
		m_pCursorMesh->Add (m_p2DScene);
	}

	LeaveCriticalSection (&_CriticalSection);

	// Return the true/false result code
	return m_bRenderDeviceInitialized;
}

/////////////////////////////////////////////////////////////////////////
//
//	OnSize
//
void
CLevelEditView::OnSize
(
	UINT nType,
	int cx,
	int cy
)
{
	// Allow the base class to process this message
	CView::OnSize (nType, cx, cy);

	int bpp = m_iBitsPerPixel;
	if (m_iWindowed == 0) {

		// Determine what our aspect ratio should be to
		// constrain the mouse to the 'window' while in full screen mode.
		m_CursorXRatio = 640 / ((float)cx);
		m_CursorYRatio = 480 / ((float)cy);

		cx = 640;
		cy = 480;
		bpp = 16;
	}

	// update the camera FOV settings
	// take the larger of the two dimensions, give it the
	// full desired FOV, then give the other dimension an
	// FOV proportional to its relative size
	m_HorzFOV = 0.0F;
	m_VertFOV = 0.0F;
	if (cy > cx) {						
		m_VertFOV = (float)DEG_TO_RAD(45.0f);
		m_HorzFOV = (double)cx / (double)cy * m_VertFOV;
	}
	else {
		m_HorzFOV = (float)DEG_TO_RAD(45.0f);
		m_VertFOV = (double)cy / (double)cx * m_HorzFOV;
	}

	if (m_bRenderDeviceInitialized) {
		
		if (m_iWindowed == 1) {
			// Change the resolution of the rendering device to 
			// match that of the view's current dimensions
			WW3D::Set_Device_Resolution (cx, cy, bpp, m_iWindowed);
		}

		// Reset the field of view
		m_pCameraMgr->Get_Camera ()->Set_View_Plane (m_HorzFOV, m_VertFOV);		

		// Force a repaint of the screen
		Repaint_View ();
	}
	
	return ;	
}


////////////////////////////////////////////////////////////////////////////
//
//  Allow_Repaint
//
////////////////////////////////////////////////////////////////////////////
void
CLevelEditView::Allow_Repaint (bool active)
{
	// Either increment or decrement the lock
	_iPaintingLock += (active ? -1 : 1);
	
	//
	// Refresh the view if necessary
	//
	if ((_iPaintingLock == 0) && _bNeedsRepaint) {
		::Refresh_Main_View ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Repaint_View
//
////////////////////////////////////////////////////////////////////////////
void
CLevelEditView::Repaint_View (void)
{
	CLevelEditDoc *doc = (CLevelEditDoc *)GetDocument ();

	//
	//	Make sure separate threads don't try to render at the same time
	//
	::EnterCriticalSection (&_CriticalSection);
	_bNeedsRepaint = false;

	//
	// Are we in a valid state?
	//
	WWASSERT (_iPaintingLock >= 0);
	if (m_bRenderDeviceInitialized &&
	    (_iPaintingLock == 0) &&
		 (doc != NULL) &&
	    (doc->Get_Scene () != NULL) &&
		 (::Is_Silent_Mode () == false))
	{
		_iPaintingLock ++;

		//
		//	Get access to the scene and current render camera
		//
		SceneEditorClass *scene		= ::Get_Scene_Editor ();
		CameraClass *camera			= scene->Get_Render_Camera ();
		
		//
		//	Let the game free up anything it needs
		//
		NetworkObjectMgrClass::Delete_Pending ();
		GameObjObserverManager::Delete_Pending ();
		ScriptManager::Destroy_Pending ();

		//
		//	Call updates on some of the subsystems
		//
		WWAudioClass::Get_Instance ()->On_Frame_Update ();

		TimeManager::Update ();
		((CMainFrame *)::AfxGetMainWnd ())->Update_Poly_Count ();
		((CMainFrame *)::AfxGetMainWnd ())->Update_Texture_Mem ();

		//
		// Let the camera know to update its animation (if any)
		//
		::Get_Camera_Mgr ()->On_Frame ();
		::Get_Scene_Editor ()->On_Frame ();
		::Get_Scene_Editor ()->Update_Selection_Boxes ();

		// Update some sub-systems.
		BackgroundMgrClass::Update (scene, camera);
		SoundEnvironmentClass *sound_environment = CombatManager::Get_Sound_Environment ();		
		if (sound_environment != NULL) sound_environment->Update (scene, camera);
		WeatherMgrClass::Update (scene, camera);

		//
		//	Collect visible objects and such...
		//		
		PhysicsSceneClass::Get_Instance ()->Pre_Render_Processing (*camera);

		//
		// Set-up the rendering process
		//
		WW3D::Begin_Render (true, true, BackgroundMgrClass::Get_Clear_Color());
		
		//
		//	Render the background scene from the game (this will give
		// us the sky and stuff).
		//		
		WW3D::Render (CombatManager::Get_Background_Scene(), camera);

		//
		// Now render the main scene
		//		
		WW3D::Render (scene, camera);

		//
		//	Render any heightfield editing gizmos
		//
		HeightfieldEditorClass::Render ();

		//
		//	Render the dazzle layer (if necessary)
		//
		DazzleLayerClass *dazzle_layer = CombatManager::Get_Dazzle_Layer ();
		if (dazzle_layer != NULL) {
			dazzle_layer->Render (camera);
		}

		//
		// Finish out the rendering process
		//
		WW3D::End_Render ();

		//
		//	Cleanup the rendering processing
		//
		PhysicsSceneClass::Get_Instance ()->Post_Render_Processing ();

		_iPaintingLock --;
	} else {
		_bNeedsRepaint = true;
	}

	::LeaveCriticalSection (&_CriticalSection);
	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  OnFinalRelease
//
void
CLevelEditView::OnFinalRelease (void)
{	
	// Allow the base class to process this message	
	CView::OnFinalRelease ();
	return;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnInitialUpdate
//
void
CLevelEditView::OnInitialUpdate (void) 
{
	// Allow the base class to process this message	
	CView::OnInitialUpdate();

	// Initialize the render device before we can update the view
	if (m_bRenderDeviceInitialized == false) {
		Initialize_Render_Device ();
	}

	if (m_bRenderDeviceInitialized)
	{
		Start_Update ();
	}

	//MainWindow = m_hWnd;
	return ;	
}


////////////////////////////////////////////////////////////////////////////
//
//  Start_Update
//
void
CLevelEditView::Start_Update (bool bstart) 
{
	if (bstart && (m_TimerID == 0)) {

		// Kick off a timer that we can use to update
		// the display (kinda like a game loop iterator)
		m_TimerID = (UINT)::timeSetEvent (50,
													 50,
													 fnTimerCallback,
													 (DWORD_PTR)m_hWnd,
													 TIME_PERIODIC);

	} else if ((bstart == false) && (m_TimerID != 0)) {

		// Stop the timer
		::timeKillEvent ((UINT)m_TimerID);

		// We don't need this anymore
		m_TimerID = 0;		
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  fnMouseHookProc
//
LRESULT CALLBACK
fnMouseHookProc
(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
)
{
	LRESULT result = 0L;

	CLevelEditView *pview = ::Get_Main_View ();
	if (pview != NULL) {
		MOUSEHOOKSTRUCT *pinfo = (MOUSEHOOKSTRUCT *)lParam;
		if (pinfo != NULL) {

			bool allow_update = pinfo->hwnd == pview->m_hWnd;
			if (!allow_update) {
				allow_update = (::GetProp (pinfo->hwnd, "ALLOW_UPDATE") != NULL);
				if (!allow_update && ::GetParent (pinfo->hwnd)) {
					allow_update = (::GetProp (::GetParent (pinfo->hwnd), "ALLOW_UPDATE") != NULL);
				}

				if (	::Get_Mouse_Mgr () &&
						::Get_Mouse_Mgr ()->Get_Mouse_Mode () == MouseMgrClass::MODE_CAMERA_WALK) {
					allow_update = true;
				}
			}

			// Start or stop updating the view
			pview->Start_Update (allow_update);
		}

		// Call the next hook in our chain
		result = ::CallNextHookEx (::Get_Main_View ()->Get_Hook (), nCode, wParam, lParam);
	}

	return result;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnCreate
//
int
CLevelEditView::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Let the main window know we want to be the active view
	CMainFrame *pwnd = (CMainFrame *)::AfxGetMainWnd ();
	if (pwnd) {
		pwnd->SetActiveView (this, false);
	}

	m_hMouseHook = ::SetWindowsHookEx (WH_MOUSE,
												  fnMouseHookProc,
												  ::AfxGetInstanceHandle (),
												  ::GetCurrentThreadId ());
	return 0;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnLButtonDown
//
void
CLevelEditView::OnLButtonDown
(
	UINT nFlags,
	CPoint point
)
{
	// Pass this onto the mouse manager
	m_pMouseMgr->Handle_LButton_Down (nFlags, point);	

	// Allow the base class to process this message	
	CView::OnLButtonDown(nFlags, point);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnLButtonDown
//
void
CLevelEditView::OnLButtonUp
(
	UINT nFlags,
	CPoint point
)
{
	// Pass this onto the mouse manager
	m_pMouseMgr->Handle_LButton_Up (nFlags, point);	

	// Allow the base class to process this message	
	CView::OnLButtonUp(nFlags, point);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnLButtonDown
//
void
CLevelEditView::OnRButtonDown
(
	UINT nFlags,
	CPoint point
)
{
	// Pass this onto the mouse manager
	m_pMouseMgr->Handle_RButton_Down (nFlags, point);	
	
	// Allow the base class to process this message	
	CView::OnRButtonDown (nFlags, point);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnRButtonUp
//
void
CLevelEditView::OnRButtonUp
(
	UINT nFlags,
	CPoint point
)
{
	// Pass this onto the mouse manager
	m_pMouseMgr->Handle_RButton_Up (nFlags, point);	

	// Allow the base class to process this message	
	CView::OnRButtonUp (nFlags, point);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnMouseMove
//
void
CLevelEditView::OnMouseMove
(
	UINT nFlags,
	CPoint point
)
{
	// Pass this onto the mouse manager
	m_pMouseMgr->Handle_Mouse_Move (nFlags, point);
	SetFocus ();

	// Allow the base class to process this message	
	CView::OnMouseMove (nFlags, point);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  WindowProc
//
LRESULT
CLevelEditView::WindowProc
(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
	// Is this the repaint message we are expecting?
	if (message == WM_USER+101)
	{
		Repaint_View ();
		::RemoveProp (m_hWnd, "WaitingToProcess");
	}

	// Allow the base class to process this message	
	return CView::WindowProc(message, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDestroy
//
void
CLevelEditView::OnDestroy (void)
{
	// Kill the timer
	Start_Update (false);

	if (m_hMouseHook != NULL) {
		::UnhookWindowsHookEx (m_hMouseHook);
		m_hMouseHook = NULL;
	}

	::RemoveProp (m_hWnd, "WaitingToProcess");

	// Allow the base class to process this message	
	CView::OnDestroy ();
	return ;	
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_View
//
void
CLevelEditView::Reset_View (void)
{
	ASSERT (m_pCameraMgr != NULL);

	// Re-initialize the camera
	m_pCameraMgr->Init_Camera ();

	// Reset the mouse mode
	m_pMouseMgr->Set_Mouse_Mode (MouseMgrClass::MODE_OBJECT_MANIPULATE);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  fnTimerCallback
//
void CALLBACK
fnTimerCallback
(
	UINT /* uID */,
	UINT /* uMsg */,
	DWORD_PTR dwUser,
	DWORD_PTR /* dw1 */,
	DWORD_PTR /* dw2 */
)
{
	HWND hwnd = (HWND)dwUser;
	if (hwnd != NULL) {

        // Send this event off to the view to process (hackish, but fine for now)
        if ((GetProp (hwnd, "WaitingToProcess") == NULL) &&
            (CLevelEditView::_iPaintingLock == 0))
        {
            SetProp (hwnd, "WaitingToProcess", (HANDLE)1);

            // Send the message to the view so it will be in the
				// same thread (Surrender doesn't seem to be thread-safe)
				::PostMessage (hwnd, WM_USER+101, 0, 0L);
        }
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnLButtonDblClk
//
void
CLevelEditView::OnLButtonDblClk
(
	UINT nFlags,
	CPoint point
)
{
	// Pass this onto the mouse manager
	m_pMouseMgr->Handle_LButton_Dblclk (nFlags, point);	

	// Allow the base class to process this message	
	CView::OnLButtonDown(nFlags, point);
	return ;
}
