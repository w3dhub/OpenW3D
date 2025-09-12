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

#if !defined(AFX_GENERATINGMANUALVISDIALOG_H__E2778314_7A17_49ED_98B2_2DDACB7F167F__INCLUDED_)
#define AFX_GENERATINGMANUALVISDIALOG_H__E2778314_7A17_49ED_98B2_2DDACB7F167F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
//
// GeneratingManualVisDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class GeneratingManualVisDialogClass : public CDialog
{
// Construction
public:
	GeneratingManualVisDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(GeneratingManualVisDialogClass)
	enum { IDD = IDD_GENERATING_MANUAL_VIS };
	CProgressCtrl	m_ProgressCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GeneratingManualVisDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GeneratingManualVisDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////////////////
	void			Set_Farm_Mode (int index, int total) { m_ProcessorIndex = index; m_TotalProcessors = total; m_FarmMode = true; }


private:

	///////////////////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////////////////
	int			Get_Manual_Point_Count (void);
	bool			On_Manual_Vis_Point_Render (DWORD milliseconds);
	void			Update_Time (void);

	///////////////////////////////////////////////////////////////////////////////
	//	Static methods
	///////////////////////////////////////////////////////////////////////////////
	static bool	ManualVisPointCallback (DWORD milliseconds, DWORD_PTR param);


	///////////////////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////////////////
	bool			m_Stop;
	bool			m_FarmMode;
	int			m_ProcessorIndex;
	int			m_TotalProcessors;
	int			m_TotalPoints;
	int			m_TotalTime;
	int			m_CurrentPoint;
	DWORD			m_StartTime;

	CString		m_StatusSection;
	CString		m_StatusFilename;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERATINGMANUALVISDIALOG_H__E2778314_7A17_49ED_98B2_2DDACB7F167F__INCLUDED_)
