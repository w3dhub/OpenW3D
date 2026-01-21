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

// ScriptEditDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "ScriptEditDialog.h"
#include "ScriptMgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// ScriptEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ScriptEditDialogClass::ScriptEditDialogClass(CWnd* pParent /*=NULL*/)
	:	m_CurrentParamType (PARAM_TYPE_STRING),
		m_CurrentParamIndex (0),
		m_ValueVector3Dlg (NULL),
		CDialog(ScriptEditDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ScriptEditDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~ScriptEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ScriptEditDialogClass::~ScriptEditDialogClass (void)
{
	SAFE_DELETE (m_ValueVector3Dlg);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ScriptEditDialogClass)
	DDX_Control(pDX, IDC_PARAM_VALUE_STRING_EDIT, m_ValueStringEdit);
	DDX_Control(pDX, IDC_PARAM_VALUE_NUMBER_SPIN, m_ValueNumberSpin);
	DDX_Control(pDX, IDC_PARAM_VALUE_NUMBER_EDIT, m_ValueNumberEdit);
	DDX_Control(pDX, IDC_PARAM_VALUE_LIST_COMBO, m_ValueListCombo);
	DDX_Control(pDX, IDC_PARAM_VALUE_BOOL_CHECK, m_ValueBoolCheck);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ScriptEditDialogClass, CDialog)
	//{{AFX_MSG_MAP(ScriptEditDialogClass)
	ON_CBN_SELCHANGE(IDC_PARAM_NAME_COMBO, OnSelChangeParamNameCombo)
	ON_CBN_SELCHANGE(IDC_SCRIPT_NAME, OnSelChangeScriptName)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PARAM_VALUE_NUMBER_SPIN, OnDeltaposParamValueNumberSpin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ScriptEditDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();
	
	//
	// Add the total list of scripts to the combobox
	//
	for (int index = 0; index < ScriptMgrClass::Get_Count (); index ++) {
		EditScriptClass *script = ScriptMgrClass::Get_Script (index);		
		if (script != NULL) {

			//
			// Add this script to the combobox
			//
			int item_index = SendDlgItemMessage (IDC_SCRIPT_NAME, CB_ADDSTRING, 0, (LPARAM)script->Get_Name ());
			SendDlgItemMessage (IDC_SCRIPT_NAME, CB_SETITEMDATA, (WPARAM)item_index, (LPARAM)script);

			//
			//	Select this script if it is the default
			//
			if (::lstrcmpi (m_Script.Get_Name (), script->Get_Name ()) == 0) {
				SendDlgItemMessage (IDC_SCRIPT_NAME, CB_SETCURSEL, (WPARAM)item_index);
			}
		}
	}

	//
	// Reposition the hidden controls so they occupy the same area as
	// the default 'value' control.
	//
	CRect rect;
	m_ValueStringEdit.GetWindowRect (&rect);
	ScreenToClient (&rect);
	m_ValueNumberEdit.SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (), SWP_NOZORDER);
	m_ValueListCombo.SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height () * 10, SWP_NOZORDER);
	m_ValueBoolCheck.SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (), SWP_NOZORDER);
	m_ValueNumberSpin.SetRange32 (-1000000, 1000000);
	m_ValueNumberSpin.SetBuddy (&m_ValueNumberEdit);

	//
	//	Create the 'Vector3' dialog control
	//
	m_ValueVector3Dlg = new Vector3DialogClass;
	m_ValueVector3Dlg->Create (this);	
	CRect v3_rect;
	m_ValueVector3Dlg->GetWindowRect (&v3_rect);
	m_ValueVector3Dlg->ShowWindow (SW_HIDE);
	m_ValueVector3Dlg->SetWindowPos (&m_ValueNumberEdit, rect.left, (rect.top + rect.Height () / 2) - (v3_rect.Height () / 2), 0, 0, SWP_NOSIZE);
	
	Fill_Param_Combo ();
	Switch_Value_Control_Type ();
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_Param_Combo
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::Fill_Param_Combo (void)
{
	// Clear the contents of the param combobox
	SendDlgItemMessage (IDC_PARAM_NAME_COMBO, CB_RESETCONTENT);

	// Add the param names to the combobox
	for (int index = 0; index < m_Script.Get_Param_Count (); index ++) {
		CString name = m_Script.Get_Param_Name (index);
		SendDlgItemMessage (IDC_PARAM_NAME_COMBO, CB_ADDSTRING, (WPARAM)0, (LPARAM)(LPCTSTR)name);
	}

	// Select the first parameter in the list by default
	SendDlgItemMessage (IDC_PARAM_NAME_COMBO, CB_SETCURSEL, (WPARAM)0);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Switch_Value_Control_Type
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::Switch_Value_Control_Type (void)
{
	// Get the parameter index and use it to determine the new combo type
	int index = SendDlgItemMessage (IDC_PARAM_NAME_COMBO, CB_GETCURSEL);
	if (index != CB_ERR) {
		Switch_Value_Control_Type (index);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Switch_Value_Control_Type
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::Switch_Value_Control_Type (int param_index)
{
	// Is the new type different than the old one?
	PARAM_TYPES type = m_Script.Get_Param_Type (param_index);
	if (type != m_CurrentParamType) {
		
		//
		// What's the new type?		
		//
		switch (type)
		{
			case PARAM_TYPE_FLOAT:
			case PARAM_TYPE_INT:
			{
				if (type == PARAM_TYPE_FLOAT) {
					::Make_Edit_Float_Ctrl (m_ValueNumberEdit);
				} else {
					::Make_Edit_Int_Ctrl (m_ValueNumberEdit);
				}

				m_ValueNumberEdit.ShowWindow (SW_SHOW);
				m_ValueNumberSpin.ShowWindow (SW_SHOW);
			}
			break;

			case PARAM_TYPE_BOOL:
			{
				m_ValueBoolCheck.ShowWindow (SW_SHOW);
			}
			break;

			case PARAM_TYPE_VECTOR3:
			{
				m_ValueVector3Dlg->ShowWindow (SW_SHOW);
			}
			break;
			
			case PARAM_TYPE_STRING:
			default:
			{
				m_ValueStringEdit.ShowWindow (SW_SHOW);
			}
			break;
		}

		//
		// Hide the old window
		//
		switch (m_CurrentParamType)
		{
			case PARAM_TYPE_FLOAT:
			case PARAM_TYPE_INT:
			{
				if ((type != PARAM_TYPE_FLOAT) &&
					 (type != PARAM_TYPE_INT)) {
					m_ValueNumberEdit.ShowWindow (SW_HIDE);
					m_ValueNumberSpin.ShowWindow (SW_HIDE);
				}
			}
			break;

			case PARAM_TYPE_BOOL:
			{
				m_ValueBoolCheck.ShowWindow (SW_HIDE);
			}
			break;
			
			case PARAM_TYPE_VECTOR3:
			{
				m_ValueVector3Dlg->ShowWindow (SW_HIDE);
			}
			break;			
			
			case PARAM_TYPE_STRING:
			default:
			{
				m_ValueStringEdit.ShowWindow (SW_HIDE);
			}
			break;
		}

		// Remember the new type
		m_CurrentParamType = type;
	}

	//
	// Make sure the value is correct
	//
	Fill_Value_Control (m_Script.Get_Param_Value (param_index));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_Value_Control
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::Fill_Value_Control (LPCTSTR value)
{
	// What's type?		
	switch (m_CurrentParamType)
	{
		case PARAM_TYPE_FLOAT:
		{
			m_ValueNumberEdit.SetWindowText (value);
			m_ValueNumberSpin.SetPos (::atol (value));
			//m_ValueNumberSpin.SetPos (int(float_value * 100.00F));
		}
		break;

		case PARAM_TYPE_INT:
		{
			m_ValueNumberEdit.SetWindowText (value);
			m_ValueNumberSpin.SetPos (::atol (value));
		}
		break;

		case PARAM_TYPE_BOOL:
		{			
			m_ValueBoolCheck.SetWindowText (m_Script.Get_Param_Name (m_CurrentParamIndex));
			m_ValueBoolCheck.SetCheck (::lstrcmpi (value, "true") == 0);
		}
		break;

		case PARAM_TYPE_VECTOR3:
		{						
			//
			//	Determine what the 3 components of the vector3 are
			//
			Vector3 new_value (0, 0, 0);
			::sscanf (value, "%f %f %f", &new_value.X, &new_value.Y, &new_value.Z);
			
			//
			//	Pass the new value onto the dialog
			//
			m_ValueVector3Dlg->Set_Default_Value (new_value);
		}
		break;
		
		case PARAM_TYPE_STRING:
		default:
		{
			m_ValueStringEdit.SetWindowText (value);
		}
		break;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Save_Current_Value
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::Save_Current_Value (void)
{
	// What's type?
	CString text_value;
	switch (m_CurrentParamType)
	{
		case PARAM_TYPE_FLOAT:
		{
			// Validate the float
			float float_value = ::GetDlgItemFloat (m_hWnd, IDC_PARAM_VALUE_NUMBER_EDIT, true);
			text_value.Format ("%.2f", float_value);
		}
		break;

		case PARAM_TYPE_INT:
		{
			// Validate the int
			int num_value = GetDlgItemInt (IDC_PARAM_VALUE_NUMBER_EDIT);
			text_value.Format ("%d", num_value);
		}
		break;

		case PARAM_TYPE_BOOL:
		{
			text_value = m_ValueBoolCheck.GetCheck () ? "true" : "false";
		}
		break;

		case PARAM_TYPE_VECTOR3:
		{
			//
			//	Get the current value from the dialog and convert it to a string
			//
			Vector3 curr_value = m_ValueVector3Dlg->Get_Current_Value ();			
			text_value.Format ("%.2f %.2f %.2f", curr_value.X, curr_value.Y, curr_value.Z);
		}
		break;
		
		case PARAM_TYPE_STRING:
		default:
		{
			m_ValueStringEdit.GetWindowText (text_value);
		}
		break;
	}

	// Pass the new value onto the property
	m_Script.Set_Param_Value (m_CurrentParamIndex, text_value);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelChangeParamNameCombo
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::OnSelChangeParamNameCombo (void) 
{
	// Make sure we save the current value from the value control
	Save_Current_Value ();

	// Get the parameter index
	m_CurrentParamIndex = SendDlgItemMessage (IDC_PARAM_NAME_COMBO, CB_GETCURSEL);
	if (m_CurrentParamIndex != CB_ERR) {
		
		// Reset the dialog controls to reflect this parameter
		Switch_Value_Control_Type ();
		Fill_Value_Control (m_Script.Get_Param_Value (m_CurrentParamIndex));
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelChangeScriptName
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::OnSelChangeScriptName (void) 
{
	// Get the current selection
	int index = SendDlgItemMessage (IDC_SCRIPT_NAME, CB_GETCURSEL);
	if (index != CB_ERR) {
		EditScriptClass *script = (EditScriptClass *)SendDlgItemMessage (IDC_SCRIPT_NAME, CB_GETITEMDATA, index);
		if (script != NULL) {
			m_Script = (*script);

			// Fill the parameter's combobox with param names
			Fill_Param_Combo ();
			Switch_Value_Control_Type (0);
			m_CurrentParamIndex = 0;
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeltaposParamValueNumberSpin
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::OnDeltaposParamValueNumberSpin
(
	NMHDR *pNMHDR,
	LRESULT *pResult
) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int value = GetDlgItemInt (IDC_PARAM_VALUE_NUMBER_EDIT);
	value += pNMUpDown->iDelta;
	SetDlgItemInt (IDC_PARAM_VALUE_NUMBER_EDIT, value);

	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
ScriptEditDialogClass::OnOK (void)
{
	Save_Current_Value ();

	CDialog::OnOK ();
	return ;
}
