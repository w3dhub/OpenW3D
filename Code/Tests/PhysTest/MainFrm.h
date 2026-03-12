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

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__616293E8_E4F0_11D2_802E_0040056350C8__INCLUDED_)
#define AFX_MAINFRM_H__616293E8_E4F0_11D2_802E_0040056350C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Vector3;
class CVJoyDialog;
class PhysClass;
class MoveablePhysClass;
class RigidBodyClass;
class MotorVehicleClass;
class WheeledVehicleClass;
class MotorcycleClass;
class CPhysTestDoc;
class ChunkLoadClass;
class ChunkSaveClass;



class CMainFrame : public CFrameWnd
{

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:

	CSplitterWnd					Splitter;
	CVJoyDialog *					VirtualJoystick;
	MoveablePhysClass *			ControlledObject;

public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
	CView * Get_Pane(int iRow, int iCol) const { return (CView *)Splitter.GetPane(iRow, iCol); }
	void Apply_Impulse(const Vector3 & imp);
	void Apply_Couple(const Vector3 & p0,const Vector3 & i0,const Vector3 & p1,const Vector3 & i1);
	void Set_Status_Bar_Text(const char * text);
	void Notify_Selection_Changed(void);

	const char *					Peek_Selected_Model(void);
	PhysClass *						Peek_Selected_Object(void);
	MoveablePhysClass *			Peek_Selected_MoveablePhysClass(void);
	RigidBodyClass *				Peek_Selected_RigidBodyClass(void);
	MotorVehicleClass *			Peek_Selected_MotorVehicleClass(void);
	WheeledVehicleClass *		Peek_Selected_WheeledVehicleClass(void);
	MotorcycleClass *				Peek_Selected_MotorcycleClass(void);
	void								Add_Object(PhysClass * new_obj);
	CPhysTestDoc *					Get_Document(void);
	void								Save(ChunkSaveClass & csave);
	void								Load(ChunkLoadClass & cload);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CStatusBar						m_wndStatusBar;
	CToolBar							m_wndToolBar;
	CToolBar							m_wndImpulseToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnOptionsRunSimulation();
	afx_msg void OnUpdateOptionsRunSimulation(CCmdUI* pCmdUI);
	afx_msg void OnOptionsDisplayBoxes();
	afx_msg void OnUpdateOptionsDisplayBoxes(CCmdUI* pCmdUI);
	afx_msg void OnCreateRigidBody();
	afx_msg void OnUpdateImpulseButton(CCmdUI* pCmdUI);
	afx_msg void OnCoupleNegx();
	afx_msg void OnCoupleNegy();
	afx_msg void OnCoupleNegz();
	afx_msg void OnCouplePosx();
	afx_msg void OnCouplePosy();
	afx_msg void OnCouplePosz();
	afx_msg void OnImpulseNegx();
	afx_msg void OnImpulseNegy();
	afx_msg void OnImpulseNegz();
	afx_msg void OnImpulsePosx();
	afx_msg void OnImpulsePosy();
	afx_msg void OnImpulsePosz();
	afx_msg void OnViewImpulsetoolbar();
	afx_msg void OnUpdateViewImpulsetoolbar(CCmdUI* pCmdUI);
	afx_msg void OnProperties();
	afx_msg void OnInertia();
	afx_msg void OnOptionsPhysicsConstants();
	afx_msg void OnFreezeObject();
	afx_msg void OnDebugObject();
	afx_msg void OnUpdateDebugObject(CCmdUI* pCmdUI);
	afx_msg void OnViewVirtualjoystick();
	afx_msg void OnUpdateViewVirtualjoystick(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnOptionsFilled();
	afx_msg void OnUpdateOptionsFilled(CCmdUI* pCmdUI);
	afx_msg void OnOptionsPoints();
	afx_msg void OnUpdateOptionsPoints(CCmdUI* pCmdUI);
	afx_msg void OnOptionsWireframe();
	afx_msg void OnUpdateOptionsWireframe(CCmdUI* pCmdUI);
	afx_msg void OnMotorProperties();
	afx_msg void OnWheelProperties();
	afx_msg void OnUpdateWheelProperties(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMotorProperties(CCmdUI* pCmdUI);
	afx_msg void OnCameraFollow();
	afx_msg void OnUpdateCameraFollow(CCmdUI* pCmdUI);
	afx_msg void OnCameraFly();
	afx_msg void OnUpdateCameraFly(CCmdUI* pCmdUI);
	afx_msg void OnCameraTether();
	afx_msg void OnUpdateCameraTether(CCmdUI* pCmdUI);
	afx_msg void OnCameraRigidTether();
	afx_msg void OnUpdateCameraRigidTether(CCmdUI* pCmdUI);
	afx_msg void OnCreateWheeledVehicle();
	afx_msg void OnUpdateCreateRigidBody(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCreateWheeledVehicle(CCmdUI* pCmdUI);
	afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
	afx_msg void OnCreateMotorcycle();
	afx_msg void OnUpdateCreateMotorcycle(CCmdUI* pCmdUI);
	afx_msg void OnMotorcycleProperties();
	afx_msg void OnUpdateMotorcycleProperties(CCmdUI* pCmdUI);
	afx_msg void OnOptionsRenderDevice();
	afx_msg void OnUpdateOptionsRenderDevice(CCmdUI* pCmdUI);
	afx_msg void OnFileImportModel();
	afx_msg void OnFileImportLev();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__616293E8_E4F0_11D2_802E_0040056350C8__INCLUDED_)
