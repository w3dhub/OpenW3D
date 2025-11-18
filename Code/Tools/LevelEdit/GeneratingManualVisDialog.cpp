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

// GeneratingManualVisDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "generatingmanualvisdialog.h"
#include "Utils.h"
#include "LevelEditView.h"
#include "vismgr.h"
#include "sceneeditor.h"
#include "nodemgr.h"
#include "VisPointNode.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
//
// GeneratingManualVisDialogClass
//
/////////////////////////////////////////////////////////////////////////////
GeneratingManualVisDialogClass::GeneratingManualVisDialogClass(CWnd* pParent /*=NULL*/)
	:	m_FarmMode (false),
		m_Stop (false),
		m_TotalPoints (0),
		m_TotalTime (0),
		m_CurrentPoint (0),
		m_StartTime (0),
		m_TotalProcessors (0),
		m_ProcessorIndex (0),
		CDialog(GeneratingManualVisDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(GeneratingManualVisDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//
	//	Determine the section of the status file
	//
	DWORD process_id = ::GetCurrentProcessId ();
	m_StatusSection.Format ("%d", process_id);

	//
	//	Get the installation directory of this application
	//		
	TCHAR exe_path[MAX_PATH] = { 0 };
	::GetModuleFileName (::AfxGetInstanceHandle (), exe_path, sizeof (exe_path));
	CString path = ::Strip_Filename_From_Path (exe_path);
	if (path[::lstrlen(path)-1] != '\\') {
		path += "\\";
	}
	m_StatusFilename = path + "status.vis";
	::WritePrivateProfileString ("Status", m_StatusSection, "Manual Vis Render", m_StatusFilename);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingManualVisDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GeneratingManualVisDialogClass)
	DDX_Control(pDX, IDC_PROGRESS_CTRL, m_ProgressCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(GeneratingManualVisDialogClass, CDialog)
	//{{AFX_MSG_MAP(GeneratingManualVisDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
GeneratingManualVisDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();
	
	m_ProgressCtrl.SetRange (0, 100);	
	m_ProgressCtrl.SetPos (0);

	ShowWindow (SW_SHOW);
	PostMessage (WM_USER + 101);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
GeneratingManualVisDialogClass::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
) 
{
	if (message == WM_USER + 101) {
		::Get_Main_View ()->Allow_Repaint (false);
		
		//
		//	Set-up the render statistics variables
		//
		m_TotalPoints	= Get_Manual_Point_Count ();
		m_CurrentPoint	= 0;
		m_StartTime		= ::TIMEGETTIME();

		//
		//	Render the manual vis points
		//
		VisMgrClass::Render_Manual_Vis_Points (m_FarmMode, m_ProcessorIndex, m_TotalProcessors, ManualVisPointCallback, (DWORD_PTR)this);

		::Get_Main_View ()->Allow_Repaint (true);
		EndDialog (IDOK);	
	}

	return CDialog::WindowProc (message, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////
//
//	On_Manual_Vis_Point_Render
//
//////////////////////////////////////////////////////////////////////////////
bool
GeneratingManualVisDialogClass::On_Manual_Vis_Point_Render (DWORD milliseconds)
{
	//
	//	Update our stats
	//
	m_CurrentPoint ++;
	m_TotalTime += milliseconds;
	Update_Time ();

	//
	//	Process window's messages once a second
	//
	static DWORD last_message_pump = 0;	
	if (::TIMEGETTIME () - last_message_pump > 1000) {
		General_Pump_Messages ();
		last_message_pump = ::TIMEGETTIME ();

		//
		//	Output our current status to the status file
		//
		if (m_FarmMode) {
			CString status;
			status.Format ("Manual Points: %d%%", int(((m_CurrentPoint + 1) * 100) / m_TotalPoints));
			::WritePrivateProfileString ("Status", m_StatusSection, status, m_StatusFilename);
		}
	}

	return !m_Stop;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Manual_Point_Count
//
//////////////////////////////////////////////////////////////////////////////
int
GeneratingManualVisDialogClass::Get_Manual_Point_Count (void)
{
	int total = 0;

	//
	//	Count up the total number of manual points in the level
	//
	VisPointNodeClass *vis_point = NULL;
	for (	vis_point = (VisPointNodeClass *)NodeMgrClass::Get_First (NODE_TYPE_VIS_POINT);
			vis_point != NULL && !m_Stop;
			vis_point = (VisPointNodeClass *)NodeMgrClass::Get_Next (vis_point, NODE_TYPE_VIS_POINT))
	{
		total ++;
	}

	return total;
}


//////////////////////////////////////////////////////////////////////////////
//
//	ManualVisPointCallback
//
//////////////////////////////////////////////////////////////////////////////
bool
GeneratingManualVisDialogClass::ManualVisPointCallback (DWORD milliseconds, DWORD_PTR param)
{
	bool retval = true;

	//
	//	Hand this notification onto the dialog
	//
	if (param != 0) {
		retval = ((GeneratingManualVisDialogClass *)param)->On_Manual_Vis_Point_Render (milliseconds);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Update_Time
//
//////////////////////////////////////////////////////////////////////////////
void
GeneratingManualVisDialogClass::Update_Time (void)
{
	//
	// Compute the elapsed and estimated remaining time
	//
	DWORD cur_ticks = ::TIMEGETTIME();
	DWORD elapsed_ticks;
	if (cur_ticks > m_StartTime) {
		elapsed_ticks = cur_ticks - m_StartTime;
	} else {
		elapsed_ticks = 0xFFFFFFFF - m_StartTime + cur_ticks;
	}

	DWORD avg_ticks = elapsed_ticks / m_CurrentPoint;
	DWORD remaining_ticks = (m_TotalPoints - m_CurrentPoint) * avg_ticks;

	float elapsed_minutes = (float)elapsed_ticks / 60000.0f;
	float remaining_minutes = (float)remaining_ticks / 60000.0f;

	//
	// Update the UI
	//
	CString status_text;
	status_text.Format ("Rendering %d of %d points.", m_CurrentPoint + 1, m_TotalPoints);
	SetDlgItemText (IDC_STATUS_TEXT, status_text);

	CString elapsed_time_text;
	elapsed_time_text.Format ("Elapsed: %.1f min.  Remaining: %.1f min.",elapsed_minutes,remaining_minutes);
	SetDlgItemText (IDC_ELAPSED_TIME_TEXT, elapsed_time_text);
	
	m_ProgressCtrl.SetPos (((m_CurrentPoint + 1) * 100) / m_TotalPoints);
	General_Pump_Messages ();	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	OnCancel
//
//////////////////////////////////////////////////////////////////////////////
void
GeneratingManualVisDialogClass::OnCancel (void)
{
	m_Stop = true;
	::EnableWindow (::GetDlgItem (m_hWnd, IDCANCEL), false);

	CDialog::OnCancel ();
	return ;
}
