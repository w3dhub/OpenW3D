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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/GeneratingEdgeSampledVisDialog.cpp $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/02/00 8:14p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "LevelEdit.h"
#include "generatingedgesampledvisdialog.h"
#include "visgenprogress.h"
#include "Utils.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////
static const float MILLISECONDS			= 1000.0F;
static const float SECONDS_PER_MINUTE	= 60.0F;
static const float TICKS_PER_MINUTE		= SECONDS_PER_MINUTE * MILLISECONDS;
static const float MINUTES_PER_TICK		= 1.0f / TICKS_PER_MINUTE;

//////////////////////////////////////////////////////////////////////////
//	Local Prototypes
//////////////////////////////////////////////////////////////////////////
static UINT fnGeneratingVisDialogThread (DWORD_PTR dwparam1, DWORD_PTR dwparam2, DWORD_PTR, HRESULT *, HWND *);


/////////////////////////////////////////////////////////////////////////////
//
// GeneratingEdgeSampledVisDialogClass
//
/////////////////////////////////////////////////////////////////////////////
GeneratingEdgeSampledVisDialogClass::GeneratingEdgeSampledVisDialogClass(CWnd* pParent /*=NULL*/)
	:	m_IsCancelled (false),
		m_IsFinished (false),
		m_ProgressObj (NULL),
		m_StartTime (0),
		m_FarmMode (false),
		CDialog(GeneratingEdgeSampledVisDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(GeneratingEdgeSampledVisDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	Create (GeneratingEdgeSampledVisDialogClass::IDD, NULL);
	
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
	::WritePrivateProfileString ("Status", m_StatusSection, "", m_StatusFilename);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingEdgeSampledVisDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GeneratingEdgeSampledVisDialogClass)
	DDX_Control(pDX, IDC_PROGRESS_CTRL, m_ProgressCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(GeneratingEdgeSampledVisDialogClass, CDialog)
	//{{AFX_MSG_MAP(GeneratingEdgeSampledVisDialogClass)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
BOOL
GeneratingEdgeSampledVisDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();
	
	//
	//	Configure the progress control
	//
	m_ProgressCtrl.SetRange (0, 100);
	m_ProgressCtrl.SetPos (0);

	//
	//	Start the timer
	//
	SetTimer (777, 1000, NULL);

	//
	//	Record the time at operation start
	//
	m_StartTime = ::GetTickCount ();
	Update_Stats ();
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnTimer
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingEdgeSampledVisDialogClass::OnTimer (UINT_PTR nIDEvent)
{
	//
	//	Refresh the dialog
	//
	if (nIDEvent == 777) {
		Update_Stats ();
	}

	CDialog::OnTimer (nIDEvent);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCancel
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingEdgeSampledVisDialogClass::OnCancel (void) 
{
	m_IsCancelled = true;

	//
	//	Stop the vis generation process
	//
	if (m_ProgressObj != NULL) {
		m_ProgressObj->Request_Cancel ();
		Update_Stats ();
	}

	//
	//	Gray out the cancel button for some feedback to the user
	//
	::EnableWindow (::GetDlgItem (m_hWnd, IDCANCEL), false);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Stats
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingEdgeSampledVisDialogClass::Update_Stats (void)
{
	if (m_ProgressObj != NULL) {
		
		//
		//	Update the simple text controls
		//
		SetDlgItemInt (IDC_TOTAL_EDGE_COUNT,		m_ProgressObj->Get_Total_Edge_Count ());
		SetDlgItemInt (IDC_TOTAL_SAMPLE_COUNT,		m_ProgressObj->Get_Total_Sample_Count ());
		SetDlgItemInt (IDC_AVG_SAMPLES_PER_NODE,	m_ProgressObj->Get_Average_Samples_Per_Node ());
		SetDlgItemInt (IDC_EDGE_COUNT,				m_ProgressObj->Get_Current_Node_Edge_Count ());
		SetDlgItemInt (IDC_SAMPLE_COUNT,				m_ProgressObj->Get_Current_Node_Sample_Count ());

		//
		//	Update the progress bar
		//
		int processed		= m_ProgressObj->Get_Processed_Node_Count ();
		int total			= m_ProgressObj->Get_Node_Count ();
		float percent		= static_cast<float>(processed) / static_cast<float>(total);
		m_ProgressCtrl.SetPos (percent * 100);

		//
		//	Update the status text
		//
		if (m_IsCancelled == false) {

			//
			//	Make a rough estimate of the time we have remaining
			//
			DWORD elapsed_ticks = ::GetTickCount () - m_StartTime;
			DWORD avg_ticks = elapsed_ticks / std::max(processed,1);
			DWORD remaining_ticks = avg_ticks * (total - processed);

			float elapsed_minutes	= MINUTES_PER_TICK * (float)elapsed_ticks;
			float remaining_minutes	= MINUTES_PER_TICK * (float)remaining_ticks;

			CString status_text;
			status_text.Format ("Nodes processed %d of %d.",processed,total);
			SetDlgItemText (IDC_STATUS_TEXT, status_text);
			
			CString elapsed_text;			
			elapsed_text.Format ("Elapsed: %.1f min Remaining: %.1f min.",elapsed_minutes, remaining_minutes);
			SetDlgItemText (IDC_ELAPSED_TIME_TEXT, elapsed_text);

		} else {
			
			CString status_text;
			status_text.Format ("Nodes processed %d of %d.",processed,total);
			SetDlgItemText (IDC_STATUS_TEXT, status_text);
			SetDlgItemText (IDC_ELAPSED_TIME_TEXT,"Cancelled!");

		}


		//
		//	Output our current status to the status file
		//
		if (m_FarmMode) {
			CString status;
			status.Format ("Sector Vis: %d%%", int(percent * 100));
			::WritePrivateProfileString ("Status", m_StatusSection, status, m_StatusFilename);
		}

	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Progress_Obj
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingEdgeSampledVisDialogClass::Set_Progress_Obj (VisGenProgressClass *obj)
{
	m_ProgressObj = obj;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Display
//
/////////////////////////////////////////////////////////////////////////////
GeneratingEdgeSampledVisDialogClass *
GeneratingEdgeSampledVisDialogClass::Display (void)
{	
	//
	//	Create the dialog on a separate thread
	//
	GeneratingEdgeSampledVisDialogClass *dialog = NULL;
	::Create_UI_Thread (fnGeneratingVisDialogThread, 0, (DWORD_PTR)&dialog, 0, NULL, NULL);
	return dialog;
}


/////////////////////////////////////////////////////////////////////////////
//
// End_Dialog
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingEdgeSampledVisDialogClass::End_Dialog (void)
{	
	::DestroyWindow (m_hWnd);
	::PostQuitMessage (0);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Finished
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingEdgeSampledVisDialogClass::Set_Finished (bool is_finished)
{	
	m_IsFinished = is_finished;
	End_Dialog();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// fnGeneratingVisDialogThread
//
////////////////////////////////////////////////////////////////////////////
UINT
fnGeneratingVisDialogThread
(
	DWORD_PTR /* dwparam1 */,
	DWORD_PTR dwparam2,
	DWORD_PTR /*dwparam3*/,
	HRESULT* /*presult*/,
	HWND* /*phmain_wnd*/
)
{
	GeneratingEdgeSampledVisDialogClass *dialog = new GeneratingEdgeSampledVisDialogClass ();
	dialog->ShowWindow (SW_SHOW);

	//
	//	Return the dialog object to the caller
	//
	GeneratingEdgeSampledVisDialogClass **return_val = (GeneratingEdgeSampledVisDialogClass **)dwparam2;
	if (return_val != NULL) {
		(*return_val) = dialog;
	}

	return 1;
}
