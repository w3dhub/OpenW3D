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

// NodeIDStartDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "NodeIDStartDialog.h"
#include "regkeys.h"
#include "utils.h"
#include "nodemgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// NodeIDStartDialogClass
//
/////////////////////////////////////////////////////////////////////////////
NodeIDStartDialogClass::NodeIDStartDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(NodeIDStartDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(NodeIDStartDialogClass)
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
NodeIDStartDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NodeIDStartDialogClass)
	DDX_Control(pDX, IDC_ID_SPIN, m_IDSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(NodeIDStartDialogClass, CDialog)
	//{{AFX_MSG_MAP(NodeIDStartDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
NodeIDStartDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();

	//
	//	Read the current value from the registry
	//
	int start_id = theApp.GetProfileInt (CONFIG_KEY, NODE_ID_START_VALUE, 100000);
	
	m_IDSpin.SetRange32 (100000, 1000000000);
	m_IDSpin.SetPos (start_id);
	SetDlgItemInt (IDC_ID, start_id);
		
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
NodeIDStartDialogClass::OnOK (void)
{
	//
	//	Store the new value in the registry
	//
	UINT start_id = GetDlgItemInt (IDC_ID);
	theApp.WriteProfileInt (CONFIG_KEY, NODE_ID_START_VALUE, start_id);

	//
	//	Update the node manager
	//
	NodeMgrClass::Reset_New_ID ();

	CDialog::OnOK ();
	return ;
}
