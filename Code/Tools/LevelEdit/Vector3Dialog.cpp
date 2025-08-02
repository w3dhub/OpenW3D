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

// Vector3Dialog.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "Vector3Dialog.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Vector3DialogClass
//
/////////////////////////////////////////////////////////////////////////////
Vector3DialogClass::Vector3DialogClass (CWnd *parent)
	:	m_DefaultValue (0, 0, 0),
		CDialog (Vector3DialogClass::IDD, parent)
{
	//{{AFX_DATA_INIT(Vector3DialogClass)
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
Vector3DialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Vector3DialogClass)
	DDX_Control(pDX, IDC_Z_STATIC, m_ZStatic);
	DDX_Control(pDX, IDC_Y_STATIC, m_YStatic);
	DDX_Control(pDX, IDC_X_STATIC, m_XStatic);
	DDX_Control(pDX, IDC_Z_EDIT, m_ZEdit);
	DDX_Control(pDX, IDC_Y_EDIT, m_YEdit);
	DDX_Control(pDX, IDC_X_EDIT, m_XEdit);
	DDX_Control(pDX, IDC_Z_SPIN, m_ZSpin);
	DDX_Control(pDX, IDC_Y_SPIN, m_YSpin);
	DDX_Control(pDX, IDC_X_SPIN, m_XSpin);
	//}}AFX_DATA_MAP

	return ;
}


BEGIN_MESSAGE_MAP(Vector3DialogClass, CDialog)
	//{{AFX_MSG_MAP(Vector3DialogClass)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//	OnCreate
//
/////////////////////////////////////////////////////////////////////////////
int
Vector3DialogClass::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Set_Default_Value
//
/////////////////////////////////////////////////////////////////////////////
void
Vector3DialogClass::Set_Default_Value (const Vector3 &value)
{
	m_DefaultValue = value;

	//
	//	Put the default values into the controls
	//
	::SetWindowFloat (m_XEdit, m_DefaultValue.X);
	::SetWindowFloat (m_YEdit, m_DefaultValue.Y);
	::SetWindowFloat (m_ZEdit, m_DefaultValue.Z);	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Get_Current_Value
//
/////////////////////////////////////////////////////////////////////////////
Vector3
Vector3DialogClass::Get_Current_Value (void)
{
	Vector3 retval (0, 0, 0);

	//
	//	Get the current value from the controls
	//
	retval.X = ::GetWindowFloat (m_XEdit, true);
	retval.Y = ::GetWindowFloat (m_YEdit, true);
	retval.Z = ::GetWindowFloat (m_ZEdit, true);	
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Create
//
/////////////////////////////////////////////////////////////////////////////
void
Vector3DialogClass::Create (CWnd *parent)
{
	CDialog::Create (Vector3DialogClass::IDD, parent);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
Vector3DialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();
	
	//
	//	Turn all the edit controls into 'float' controls that will
	// allow the user to type only numbers (with decimals and signs)
	//
	::Make_Edit_Float_Ctrl (m_XEdit);
	::Make_Edit_Float_Ctrl (m_YEdit);
	::Make_Edit_Float_Ctrl (m_ZEdit);

	//
	//	Set the float control ranges
	//
	m_XSpin.SetRange32 (-1000000, 1000000);
	m_YSpin.SetRange32 (-1000000, 1000000);
	m_ZSpin.SetRange32 (-1000000, 1000000);

	//
	//	Set the default values for the control
	//
	::SetWindowFloat (m_XEdit, m_DefaultValue.X);
	::SetWindowFloat (m_YEdit, m_DefaultValue.Y);
	::SetWindowFloat (m_ZEdit, m_DefaultValue.Z);
	return TRUE;
}
