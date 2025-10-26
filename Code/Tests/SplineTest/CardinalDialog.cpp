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

// CardinalDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SplineTest.h"
#include "CardinalDialog.h"
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
// CCardinalDialog dialog


CCardinalDialog::CCardinalDialog(CWnd* pParent /*=NULL*/,CardinalSpline3DClass * curve,int key)
	: CDialog(CCardinalDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCardinalDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Curve = curve;
	Key = key;
}


void CCardinalDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCardinalDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCardinalDialog, CDialog)
	//{{AFX_MSG_MAP(CCardinalDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCardinalDialog message handlers

BOOL CCardinalDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetDlgItemFloat(IDC_TIGHTNESS_EDIT,Curve->Get_Tightness(Key));
	
	return true;
}

void CCardinalDialog::OnOK() 
{
	Curve->Set_Tightness(Key,GetDlgItemFloat(IDC_TIGHTNESS_EDIT));
	CDialog::OnOK();
}

float CCardinalDialog::GetDlgItemFloat(int controlid)
{
	CString string;
	GetDlgItemText(controlid,string);
	return atof(string);
}

void CCardinalDialog::SetDlgItemFloat(int controlid,float val)
{
	CString string;
	string.Format("%.2f",val);
	SetDlgItemText(controlid,string);
}
