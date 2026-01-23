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

// MainDialogBar.cpp : implementation file
//

#include "StdAfx.h"

#include "LevelEdit.h"
#include "maindialogbar.h"
#include "presetslibform.h"
#include "Utils.h"
#include "instancespage.h"
#include "overlappage.h"
#include "conversationpage.h"
#include "mainfrm.h"
#include "heightfieldpage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// MainDialogBarClass dialog
//
/////////////////////////////////////////////////////////////////////////////
MainDialogBarClass::MainDialogBarClass (void)
	:	m_iCurrentTab (0)
{
	//{{AFX_DATA_INIT(MainDialogBarClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	::memset (m_pTabs, 0, sizeof (m_pTabs));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~MainDialogBarClass
//
/////////////////////////////////////////////////////////////////////////////
MainDialogBarClass::~MainDialogBarClass (void)
{
	for (int index = 0; index < TAB_COUNT; index ++) {
		SAFE_DELETE (m_pTabs[index]);
	}

	return ;
}


void
MainDialogBarClass::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MainDialogBarClass)
	DDX_Control(pDX, IDC_LIBRARY_TAB, m_TabCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MainDialogBarClass, CDialogBar)
	//{{AFX_MSG_MAP(MainDialogBarClass)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_LIBRARY_TAB, OnSelchangeLibraryTab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//
// MainDialogBarClass message handlers
//
/////////////////////////////////////////////////////////////////////////////
BOOL
MainDialogBarClass::OnInitDialog (void) 
{
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
MainDialogBarClass::OnSize
(
	UINT	nType,
	int	cx,
	int	cy
)
{
	CDialogBar::OnSize (nType, cx, cy);

	if (::IsWindow (m_TabCtrl) && (cx > 0) && (cy > 0)) {
		
		//CRect client_rect;
		//GetClientRect (&client_rect);
		
		// Resize the tab control to fill the entire contents of the client area
		m_TabCtrl.SetWindowPos (NULL, 0, 3, cx, cy - 6, SWP_NOZORDER);

		// Get the display rectangle of the tab control
		CRect rect;
		m_TabCtrl.GetWindowRect (&rect);
		m_TabCtrl.AdjustRect (false, &rect);

		// Convert the display rectangle from screen to client coords
		ScreenToClient (&rect);

		//
		// Loop through all the tabs in the library
		//
		for (int tab = 0; tab < TAB_COUNT; tab ++) {
			if (m_pTabs[tab] != NULL) {
				
				//
				// Resize this tab
				//
				m_pTabs[tab]->SetWindowPos (NULL, rect.left, rect.top,
										rect.Width (), rect.Height (), SWP_NOZORDER);
			}
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelchangeLibraryTab
//
/////////////////////////////////////////////////////////////////////////////
void
MainDialogBarClass::OnSelchangeLibraryTab
(
	NMHDR* /* pNMHDR */,
	LRESULT* pResult
) 
{
	// Which tab is selected?
	int newtab = m_TabCtrl.GetCurSel ();

	// Is this a new tab?
	if (m_iCurrentTab != newtab) {
		
		// Is the old tab valid?
		if (m_pTabs[m_iCurrentTab] != NULL) {
			
			// Hide the contents of the old tab
			m_pTabs[m_iCurrentTab]->ShowWindow (SW_HIDE);
		}

		// Is the new tab valid?
		if (m_pTabs[newtab] != NULL) {
			
			// Show the contents of the new tab
			m_pTabs[newtab]->ShowWindow (SW_SHOW);
		}

		// Remember what our new current tab is
		m_iCurrentTab = newtab;		
	}

	
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Initialize
//
/////////////////////////////////////////////////////////////////////////////
void
MainDialogBarClass::Initialize (void)
{
	UpdateData (false);
	
	char presets_string[] = "Presets";
	TC_ITEM tab_info = { 0 };
	tab_info.mask = TCIF_TEXT;
	tab_info.pszText = presets_string;
	m_TabCtrl.InsertItem (TAB_GLOBAL, &tab_info);

	char instances_string[] = "Instances";
	tab_info.mask = TCIF_TEXT;
	tab_info.pszText = instances_string;
	m_TabCtrl.InsertItem (TAB_INSTANCES, &tab_info);

	char conversations_string[] = "Conversations";
	tab_info.mask = TCIF_TEXT;
	tab_info.pszText = conversations_string;
	m_TabCtrl.InsertItem (TAB_CONVERSATION, &tab_info);

	char overlap_string[] = "Overlap";
	tab_info.mask = TCIF_TEXT;
	tab_info.pszText = overlap_string;
	m_TabCtrl.InsertItem (TAB_OVERLAP, &tab_info);

	//
	// Create the form's to add to the different tabs
	//
	m_pTabs[TAB_GLOBAL]			= new PresetsFormClass (this);
	m_pTabs[TAB_INSTANCES]		= new InstancesPageClass (this);
	m_pTabs[TAB_CONVERSATION]	= new ConversationPageClass (this);
	m_pTabs[TAB_OVERLAP]			= new OverlapPageClass (this);
	
//#ifndef PUBLIC_EDITOR_VER  (gth) go ahead and give the heightfield away!

	//
	//	Install the heightfield tab
	//
	char heightfield_string[] = "Heightfield";
	tab_info.mask		= TCIF_TEXT;
	tab_info.pszText	= heightfield_string;
	m_TabCtrl.InsertItem (TAB_HEIGHTFIELD, &tab_info);	

	m_pTabs[TAB_HEIGHTFIELD] = new HeightfieldPageClass (this);

//#endif //PUBLIC_EDITOR_VER

	//
	// Loop through and create the tabs
	//
	for (int index = 0; index < TAB_COUNT; index ++) {		
		if (m_pTabs[index] != NULL) {
			m_pTabs[index]->ShowWindow ((index == 0) ? SW_SHOW : SW_HIDE);
		}
	}

	return ;
}

BOOL MainDialogBarClass::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	/*CMainFrame *main_wnd = (CMainFrame *)::AfxGetMainWnd ();
	if (main_wnd !=  NULL) {
		HACCEL accel_table = main_wnd->GetDefaultAccelerator ();
		if (::TranslateAccelerator (main_wnd->m_hWnd, accel_table, pMsg)) {
			return 1;
		}		
	}*/
	
	return CDialogBar::PreTranslateMessage(pMsg);
}
