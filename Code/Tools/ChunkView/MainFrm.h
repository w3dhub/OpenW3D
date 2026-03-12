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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : ChunkView                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/ChunkView/MainFrm.h                    $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/28/99 9:48a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__53C69A68_72AB_11D3_BB4D_00902742EA14__INCLUDED_)
#define AFX_MAINFRM_H__53C69A68_72AB_11D3_BB4D_00902742EA14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CChunkDataView;
class CChunkTreeView;


class CMainFrame : public CFrameWnd
{

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
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
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CChunkDataView *	Get_Data_View(void);
	CChunkTreeView *	Get_Tree_View(void);

protected:  // control bar embedded members
	CStatusBar			m_wndStatusBar;
	CToolBar				m_wndToolBar;
	CReBar				m_wndReBar;
	CSplitterWnd		m_wndSplitter;


// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDisplayHex();
	afx_msg void OnUpdateDisplayHex(CCmdUI* pCmdUI);
	afx_msg void OnDisplayMicrochunks();
	afx_msg void OnUpdateDisplayMicrochunks(CCmdUI* pCmdUI);
	afx_msg void OnWordmodeByte();
	afx_msg void OnUpdateWordmodeByte(CCmdUI* pCmdUI);
	afx_msg void OnWordmodeShort();
	afx_msg void OnUpdateWordmodeShort(CCmdUI* pCmdUI);
	afx_msg void OnWordmodeLong();
	afx_msg void OnUpdateWordmodeLong(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__53C69A68_72AB_11D3_BB4D_00902742EA14__INCLUDED_)
