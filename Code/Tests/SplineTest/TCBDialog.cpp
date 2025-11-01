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

// TCBDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SplineTest.h"
#include "TCBDialog.h"
#include "curve.h"
#include "hermitespline.h"
#include "catmullromspline.h"
#include "cardinalspline.h"
#include "tcbspline.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTCBDialog dialog


CTCBDialog::CTCBDialog(CWnd* pParent,TCBSpline3DClass * curve,int key)
	: CDialog(CTCBDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTCBDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Curve = curve;
	Key = key;
}


void CTCBDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTCBDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTCBDialog, CDialog)
	//{{AFX_MSG_MAP(CTCBDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTCBDialog message handlers

BOOL CTCBDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	float t,c,b;
	Curve->Get_TCB_Params(Key,&t,&c,&b);
	SetDlgItemFloat(IDC_TENSION_EDIT,t);
	SetDlgItemFloat(IDC_CONTINUITY_EDIT,c);
	SetDlgItemFloat(IDC_BIAS_EDIT,b);

	return true;  
}

void CTCBDialog::OnOK() 
{
	float t,c,b;
	t = GetDlgItemFloat(IDC_TENSION_EDIT);
	c = GetDlgItemFloat(IDC_CONTINUITY_EDIT);
	b = GetDlgItemFloat(IDC_BIAS_EDIT);
	Curve->Set_TCB_Params(Key,t,c,b);

	CDialog::OnOK();
}

float CTCBDialog::GetDlgItemFloat(int controlid)
{
	CString string;
	GetDlgItemText(controlid,string);
	return atof(string);
}

void CTCBDialog::SetDlgItemFloat(int controlid,float val)
{
	CString string;
	string.Format("%.2f",val);
	SetDlgItemText(controlid,string);
}
