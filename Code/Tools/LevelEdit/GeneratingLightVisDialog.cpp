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

// GeneratingLightVisDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "GeneratingLightVisDialog.h"
#include "Utils.h"
#include "SceneEditor.h"
#include "LevelEditView.h"
#include "rendobj.h"
#include "phys.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////////////////////////////////////////////////////////////////////
//
// GeneratingLightVisDialogClass dialog
//
//////////////////////////////////////////////////////////////////////////////
GeneratingLightVisDialogClass::GeneratingLightVisDialogClass(CWnd* pParent /*=NULL*/) : 
	CDialog(GeneratingLightVisDialogClass::IDD, pParent),
	m_IsCancelled(false),
	m_StartTicks(0),
	m_CurrentLight(0),
	m_FirstLight(0),
	m_LastLight(0),
	m_FarmMode(false),
	m_TotalProcessors(1),
	m_ProcessorIndex(0)
{
	//{{AFX_DATA_INIT(GeneratingLightVisDialogClass)
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
	::WritePrivateProfileString ("Status", m_StatusSection, "", m_StatusFilename);
	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
//////////////////////////////////////////////////////////////////////////////
void 
GeneratingLightVisDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GeneratingLightVisDialogClass)
	DDX_Control(pDX, IDC_PROGRESS_CTRL, m_ProgressCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(GeneratingLightVisDialogClass, CDialog)
	//{{AFX_MSG_MAP(GeneratingLightVisDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
//////////////////////////////////////////////////////////////////////////////
BOOL 
GeneratingLightVisDialogClass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ProgressCtrl.SetRange (0, 100);	
	m_ProgressCtrl.SetPos (0);

	ShowWindow (SW_SHOW);
	PostMessage (WM_USER+101);
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
// OnCancel
//
//////////////////////////////////////////////////////////////////////////////
void 
GeneratingLightVisDialogClass::OnCancel() 
{
	m_IsCancelled = true;
}

//////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
//////////////////////////////////////////////////////////////////////////////
LRESULT 
GeneratingLightVisDialogClass::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_USER+101) {

		::Get_Main_View ()->Allow_Repaint (false);
		SceneEditorClass *scene_editor = ::Get_Scene_Editor ();
		m_StartTicks = ::GetTickCount ();

		/*
		** Generate the start and end index for the lights we're going to vis
		*/
		int light_count = scene_editor->Get_Static_Light_Count();
		float lights_per_processor = (float)light_count / (float)m_TotalProcessors;
		
		m_FirstLight	= (int)::floor (lights_per_processor * (float)m_ProcessorIndex);
		m_LastLight		= (int)::ceil (lights_per_processor * (float)(m_ProcessorIndex+1));
		m_LastLight		= min (light_count, m_LastLight);

		/*
		** Prepare to do the actual vis sampling
		*/
		bool restore_vis_point_display = scene_editor->Are_Vis_Points_Displayed ();
		scene_editor->Display_Vis_Points (false);
	
		/*
		** Perform a vis sample for each light, updating the UI after each one
		*/
		for (int i=m_FirstLight; (i<m_LastLight) && (m_IsCancelled == false); i++) {
			scene_editor->Generate_Vis_For_Light(i);
			Update_Status(i);
			General_Pump_Messages();	
		}

		/*
		** Restore the scene settings and close this dialog.
		*/
		if (restore_vis_point_display) {
			scene_editor->Display_Vis_Points (true);
		}
		::Get_Main_View ()->Allow_Repaint (true);
		EndDialog (TRUE);

	}

	return CDialog::WindowProc(message, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////
//
// Update_Status
//
//////////////////////////////////////////////////////////////////////////////
void 
GeneratingLightVisDialogClass::Update_Status (int cur_light)
{
	int light_index = (cur_light - m_FirstLight) + 1;
	int light_count = (m_LastLight - m_FirstLight);

	// 
	// Update the light counters
	//
	CString status_text;
	status_text.Format ("Rendering %d of %d points.",light_index,light_count);
	SetDlgItemText (IDC_STATUS_TEXT, status_text);

	//
	// Update the elapsed and estimated remaining time
	//
	DWORD cur_ticks = ::GetTickCount();
	DWORD elapsed_ticks;
	if (cur_ticks > m_StartTicks) {
		elapsed_ticks = cur_ticks - m_StartTicks;
	} else {
		elapsed_ticks = 0xFFFFFFFF - m_StartTicks + cur_ticks;
	}
	DWORD avg_ticks = elapsed_ticks / light_index;
	DWORD remaining_ticks = (m_LastLight - cur_light) * avg_ticks;
	
	float elapsed_minutes = (float)elapsed_ticks / 60000.0f;
	float remaining_minutes = (float)remaining_ticks / 60000.0f;
	
	CString elapsed_time_text;
	elapsed_time_text.Format ("Elapsed: %.1f min.  Remaining: %.1f min.",elapsed_minutes,remaining_minutes);
	SetDlgItemText (IDC_ELAPSED_TIME_TEXT, elapsed_time_text);

	m_ProgressCtrl.SetPos ((light_index * 100) / light_count);

	//
	//	If in farm mode, output our current status to the status file
	//
	if (m_FarmMode) {
		CString status;
		status.Format ("Lights: %d%%", int((light_index * 100) / light_count));
		::WritePrivateProfileString ("Status", m_StatusSection, status, m_StatusFilename);
	}

	return;
}
