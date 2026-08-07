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

#if !defined(AFX_LODSETTINGSDIALOG_H__633E0B62_49FF_11D3_A050_00104B791122__INCLUDED_)
#define AFX_LODSETTINGSDIALOG_H__633E0B62_49FF_11D3_A050_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LODSettingsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// LODSettingsDialogClass dialog

class LODSettingsDialogClass : public CDialog
{
// Construction
public:
	LODSettingsDialogClass (unsigned int distance = 0, CWnd* pParent = nullptr);

// Dialog Data
	//{{AFX_DATA(LODSettingsDialogClass)
	enum { IDD = IDD_LOD_SETTINGS };
	CSpinButtonCtrl	m_DistanceSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LODSettingsDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(LODSettingsDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		////////////////////////////////////////////////////////
		//	Public methods
		////////////////////////////////////////////////////////
		unsigned int		Get_Distance (void) const { return m_Distance; }

	private:

		////////////////////////////////////////////////////////
		//	Private member data
		////////////////////////////////////////////////////////
		unsigned int		m_Distance;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LODSETTINGSDIALOG_H__633E0B62_49FF_11D3_A050_00104B791122__INCLUDED_)
