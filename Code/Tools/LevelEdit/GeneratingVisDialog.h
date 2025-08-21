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

#if !defined(AFX_GENERATINGVISDIALOG_H__447D6F69_04A5_11D3_A025_00104B791122__INCLUDED_)
#define AFX_GENERATINGVISDIALOG_H__447D6F69_04A5_11D3_A025_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GeneratingVisDialog.h : header file
//

#include "listtypes.h"
#include "vispointgenerator.h"


/////////////////////////////////////////////////////////////////////////////
//
// GeneratingVisDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class GeneratingVisDialogClass : public CDialog
{
// Construction
public:
	GeneratingVisDialogClass(float granularity, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(GeneratingVisDialogClass)
	enum { IDD = IDD_GENERATING_VIS };
	CProgressCtrl	m_ProgressCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GeneratingVisDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GeneratingVisDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:
		
		/////////////////////////////////////////////////////////////////////////////////
		//	Public methods
		/////////////////////////////////////////////////////////////////////////////////
		void			Set_Ignore_Bias (bool onoff)	{ m_IgnoreBias = onoff; }

		void			Set_Farm_Mode (int index, int total) { m_ProcessorIndex = index; m_TotalProcessors = total; m_FarmMode = true; }

		void			Do_Selection_Only (bool sel_only)	{ m_SelectionOnly = sel_only; }

		void			Set_Sample_Height (float sample_height) { m_SampleHeight = sample_height; }

	protected:
		
		/////////////////////////////////////////////////////////////////////////////////
		//	Protected methods
		/////////////////////////////////////////////////////////////////////////////////
		void			Update_Time (void);
		void			Build_Node_List (NODE_LIST &list);
		void			Render_Vis_Points (VIS_POINT_LIST &point_list);
		bool			On_Manual_Vis_Point_Render (DWORD milliseconds);
		int			Get_Manual_Point_Count (void);
		void			Generate_Points (NODE_LIST &node_list, VisPointGeneratorClass &generator);
		void			Reset_Vis_Data_For_Nodes (NODE_LIST &node_list);

		/////////////////////////////////////////////////////////////////////////////////
		//	Protected static methods
		/////////////////////////////////////////////////////////////////////////////////
		static bool ManualVisPointCallback (DWORD milliseconds, DWORD_PTR param);

	private:
		
		/////////////////////////////////////////////////////////////////////////////////
		//	Private member data
		/////////////////////////////////////////////////////////////////////////////////
		int			m_CurrentPoint;
		int			m_TotalPoints;
		DWORD			m_StartTime;

		bool			m_IgnoreBias;
		bool			m_bStop;
		float			m_Granularity;
		float			m_SampleHeight;
		bool			m_SelectionOnly;

		int			m_TotalProcessors;
		int			m_ProcessorIndex;
		
		bool			m_FarmMode;
		CString		m_StatusFilename;
		CString		m_StatusSection;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERATINGVISDIALOG_H__447D6F69_04A5_11D3_A025_00104B791122__INCLUDED_)
