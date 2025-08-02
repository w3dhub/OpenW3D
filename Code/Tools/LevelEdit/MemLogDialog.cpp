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

// MemLogDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "memlogdialog.h"
#include "wwmemlog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////
enum
{
	COL_NAME			= 0,
	COL_CURRENT,
	COL_PEAK,
};


/////////////////////////////////////////////////////////////////////////////
//
// MemLogDialogClass
//
/////////////////////////////////////////////////////////////////////////////
MemLogDialogClass::MemLogDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(MemLogDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(MemLogDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
MemLogDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MemLogDialogClass)
	DDX_Control(pDX, IDC_MEM_USAGE_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(MemLogDialogClass, CDialog)
	//{{AFX_MSG_MAP(MemLogDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
MemLogDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Configure the columns
	//
	m_ListCtrl.InsertColumn (COL_NAME,		"Name");
	m_ListCtrl.InsertColumn (COL_CURRENT,	"Current Usage");
	m_ListCtrl.InsertColumn (COL_PEAK,		"Peak Usage");
	m_ListCtrl.SetExtendedStyle (m_ListCtrl.GetExtendedStyle () | LVS_EX_FULLROWSELECT);
	
	//
	//	Choose an appropriate size for the columns
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	rect.right -= ::GetSystemMetrics (SM_CXVSCROLL) + 2;
	m_ListCtrl.SetColumnWidth (COL_NAME,		(rect.Width () * 3) / 5);
	m_ListCtrl.SetColumnWidth (COL_CURRENT,	rect.Width () / 5);
	m_ListCtrl.SetColumnWidth (COL_PEAK,		rect.Width () / 5);
	
	//
	//	Loop over all the categories in the logger
	//
	int count = WWMemoryLogClass::Get_Category_Count ();
	for (int index = 0; index < count; index ++) {
		const char *name	= WWMemoryLogClass::Get_Category_Name (index);
		int current_mem	= WWMemoryLogClass::Get_Current_Allocated_Memory (index);
		int peak_mem		= WWMemoryLogClass::Get_Peak_Allocated_Memory (index);

		//
		//	Insert this category in the list
		//
		int item_index = m_ListCtrl.InsertItem (index, name);
		if (item_index >= 0) {
			CString current_mem_string;
			CString peak_mem_string;
			current_mem_string.Format ("%.2f KB", (float)current_mem / 1024.0F);
			peak_mem_string.Format ("%.2f KB", (float)peak_mem / 1024.0F);
			m_ListCtrl.SetItemText (item_index, COL_CURRENT, current_mem_string);
			m_ListCtrl.SetItemText (item_index, COL_PEAK, peak_mem_string);
		}
	}
	
	return TRUE;
}

