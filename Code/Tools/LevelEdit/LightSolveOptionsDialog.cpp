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

// LightSolveOptionsDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "SceneEditor.h"
#include "Utils.h"
#include "lightsolveoptionsdialog.h"
#include "lightsolveprogressdialog.h"
#include "phys.h"
#include "rendobj.h"
#include "lightsolvecontext.h"
#include "pscene.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// LightSolveOptionsDialogClass dialog


LightSolveOptionsDialogClass::LightSolveOptionsDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(LightSolveOptionsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(LightSolveOptionsDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void LightSolveOptionsDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LightSolveOptionsDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(LightSolveOptionsDialogClass, CDialog)
	//{{AFX_MSG_MAP(LightSolveOptionsDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LightSolveOptionsDialogClass message handlers

BOOL LightSolveOptionsDialogClass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return true;
}

void LightSolveOptionsDialogClass::OnOK() 
{
	LightSolveContextClass context;

	//
	// Setup the solve options
	//
	context.Enable_Occlusion(SendDlgItemMessage(IDC_LIGHT_SOLVE_OCCLUSION,BM_GETCHECK));
	context.Enable_Filtering(SendDlgItemMessage(IDC_LIGHT_SOLVE_FILTERING,BM_GETCHECK));

	//
	// Run the solve
	//
	LightSolveProgressDialog::Solve(context,this);
	
	CDialog::OnOK ();	
	return ;
}

