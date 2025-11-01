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
 *                     $Archive:: /Commando/Code/Tools/ChunkView/MainFrm.cpp                  $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/28/99 9:46a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ChunkView.h"

#include "MainFrm.h"
#include "ChunkDataView.h"
#include "ChunkTreeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_DISPLAY_HEX, OnDisplayHex)
	ON_UPDATE_COMMAND_UI(ID_DISPLAY_HEX, OnUpdateDisplayHex)
	ON_COMMAND(ID_DISPLAY_MICROCHUNKS, OnDisplayMicrochunks)
	ON_UPDATE_COMMAND_UI(ID_DISPLAY_MICROCHUNKS, OnUpdateDisplayMicrochunks)
	ON_COMMAND(ID_WORDMODE_BYTE, OnWordmodeByte)
	ON_UPDATE_COMMAND_UI(ID_WORDMODE_BYTE, OnUpdateWordmodeByte)
	ON_COMMAND(ID_WORDMODE_SHORT, OnWordmodeShort)
	ON_UPDATE_COMMAND_UI(ID_WORDMODE_SHORT, OnUpdateWordmodeShort)
	ON_COMMAND(ID_WORDMODE_LONG, OnWordmodeLong)
	ON_UPDATE_COMMAND_UI(ID_WORDMODE_LONG, OnUpdateWordmodeLong)
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

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return false;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return true;
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

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// Create the main splitter window for the application
	BOOL ok = m_wndSplitter.CreateStatic (this, 1, 2);

	ASSERT(ok);
	if (!ok) return false;
	
	// Create the two sub views 
	ok &= m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CChunkTreeView),CSize(200,10),pContext);
	ok &= m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CChunkDataView),CSize(340,10),pContext);
	
	ASSERT(ok);
	if (!ok) return false;

	return true;
}

CChunkDataView * CMainFrame::Get_Data_View(void)
{
	return (CChunkDataView *)m_wndSplitter.GetPane(0,1);
}

CChunkTreeView * CMainFrame::Get_Tree_View(void)
{
	return (CChunkTreeView *)m_wndSplitter.GetPane(0,0);
}


void CMainFrame::OnDisplayHex() 
{
	Get_Data_View()->Set_Display_Mode(CChunkDataView::DISPLAY_MODE_HEX);
}

void CMainFrame::OnUpdateDisplayHex(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(Get_Data_View()->Get_Display_Mode() == CChunkDataView::DISPLAY_MODE_HEX);
}

void CMainFrame::OnDisplayMicrochunks() 
{
	Get_Data_View()->Set_Display_Mode(CChunkDataView::DISPLAY_MODE_MICROCHUNKS);	
}

void CMainFrame::OnUpdateDisplayMicrochunks(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(Get_Data_View()->Get_Display_Mode() == CChunkDataView::DISPLAY_MODE_MICROCHUNKS);
}

void CMainFrame::OnWordmodeByte() 
{
	Get_Data_View()->Set_Word_Size(CChunkDataView::WORD_SIZE_BYTE);
}

void CMainFrame::OnUpdateWordmodeByte(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(Get_Data_View()->Get_Word_Size() == CChunkDataView::WORD_SIZE_BYTE);
}

void CMainFrame::OnWordmodeShort() 
{
	Get_Data_View()->Set_Word_Size(CChunkDataView::WORD_SIZE_SHORT);
}

void CMainFrame::OnUpdateWordmodeShort(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(Get_Data_View()->Get_Word_Size() == CChunkDataView::WORD_SIZE_SHORT);
}

void CMainFrame::OnWordmodeLong() 
{
	Get_Data_View()->Set_Word_Size(CChunkDataView::WORD_SIZE_LONG);
}

void CMainFrame::OnUpdateWordmodeLong(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(Get_Data_View()->Get_Word_Size() == CChunkDataView::WORD_SIZE_LONG);
}
