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

#if !defined(AFX_ZONEEDITDIALOG_H__68FE6085_8F85_11D3_8DE2_00104B6FD9E3__INCLUDED_)
#define AFX_ZONEEDITDIALOG_H__68FE6085_8F85_11D3_8DE2_00104B6FD9E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "dialogtoolbar.h"
#include "obbox.h"
#include "dx8wrapper.h"


/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
class SimpleSceneClass;
class CameraClass;
class RenderObjClass;
class PhysClass;
class Box3DClass;



/////////////////////////////////////////////////////////////////////////////
//
// ZoneEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class ZoneEditDialogClass : public CDialog
{
// Construction
public:
	ZoneEditDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ZoneEditDialogClass)
	enum { IDD = IDD_ZONE_EDIT };
	CButton	m_MoveZoneCheck;
	CButton	m_SizeZoneCheck;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ZoneEditDialogClass)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ZoneEditDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnTop();
	afx_msg void OnFront();
	afx_msg void OnLeft();
	afx_msg void OnRight();
	afx_msg void OnEditZone();
	afx_msg void OnSizeZone();
	afx_msg void OnLastFrame();
	afx_msg void OnFirstFrame();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:
	
	/////////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////////
	void						Set_Phys_Obj (PhysClass *phys_obj);
	
	void						Set_Zone (const OBBoxClass &zone)					{ m_ZoneBox = zone; }
	const OBBoxClass &	Get_Zone (void) const									{ return m_ZoneBox; }

protected:

	/////////////////////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////////////////////
	void						Handle_Keypress (void);
	void						Load_Object (void);
	void						Render_View (void);
	void						Insert_Zone (void);
	static void CALLBACK fnUpdateTimer (UINT, UINT, DWORD, DWORD, DWORD);

	void						Update_Status (void);

	static LRESULT CALLBACK fn3DWindow (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	void						Handle_LBUTTON_DOWN (WPARAM wparam, LPARAM lparam);
	void						Handle_LBUTTON_UP (WPARAM wparam, LPARAM lparam);
	void						Handle_MOUSEMOVE (WPARAM wparam, LPARAM lparam);

private:

	/////////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////////
	UINT						m_TimerID;
	SimpleSceneClass *	m_Scene;
	CameraClass *			m_Camera;
	IDirect3DSwapChain9 *m_SwapChain;

	RenderObjClass *		m_RenderObj;
	PhysClass *				m_PhysObj;
	Box3DClass *			m_Zone;
	OBBoxClass				m_ZoneBox;

	float						m_LookAtDist;
	bool						m_IsEditingZone;
	bool						m_IsSizingZone;
	bool						m_Initialized;
	CPoint					m_LastPoint;

	DialogToolbarClass	m_Toolbar;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZONEEDITDIALOG_H__68FE6085_8F85_11D3_8DE2_00104B6FD9E3__INCLUDED_)
