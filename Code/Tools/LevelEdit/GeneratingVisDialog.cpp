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

// GeneratingVisDialog.cpp : implementation file
//

#include "stdafx.h"
#include "leveledit.h"
#include "generatingvisdialog.h"
#include "utils.h"
#include "sceneeditor.h"
#include "nodemgr.h"
#include "vispointgenerator.h"
#include "leveleditview.h"
#include "groupmgr.h"
#include "node.h"
#include "VisPointNode.h"
#include "camera.h"
#include "cameramgr.h"
#include "vismgr.h"

#include "vissectorsampler.h"
#include "visgenprogress.h"
#include "collisiongroups.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
//
// GeneratingVisDialogClass dialog
//
/////////////////////////////////////////////////////////////////////////////
GeneratingVisDialogClass::GeneratingVisDialogClass (float granularity, CWnd *parent)
	:	m_bStop (false),
		m_Granularity (granularity),
		m_SampleHeight (10.0f),
		m_CurrentPoint (0),
		m_TotalPoints (0),
		m_StartTime (0),
		m_FarmMode (false),
		m_IgnoreBias (false),
		m_SelectionOnly (false),
		CDialog(GeneratingVisDialogClass::IDD, parent)
{
	//{{AFX_DATA_INIT(GeneratingVisDialogClass)
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
	::WritePrivateProfileString ("Status", m_StatusSection, "Point Gen", m_StatusFilename);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// GeneratingVisDialogClass
//
/////////////////////////////////////////////////////////////////////////////
void
GeneratingVisDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GeneratingVisDialogClass)
	DDX_Control(pDX, IDC_PROGRESS_CTRL, m_ProgressCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(GeneratingVisDialogClass, CDialog)
	//{{AFX_MSG_MAP(GeneratingVisDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
//////////////////////////////////////////////////////////////////////////////
BOOL
GeneratingVisDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	m_ProgressCtrl.SetRange (0, 100);	
	m_ProgressCtrl.SetPos (0);

	ShowWindow (SW_SHOW);
	PostMessage (WM_USER+101);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
//
//	OnCancel
//
//////////////////////////////////////////////////////////////////////////////
void
GeneratingVisDialogClass::OnCancel (void) 
{
	m_bStop = true;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Reset_Vis_Data_For_Nodes
//
//////////////////////////////////////////////////////////////////////////////
void
GeneratingVisDialogClass::Reset_Vis_Data_For_Nodes (NODE_LIST &node_list)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene == NULL) {
		return ;
	}

	//
	//	Reset vis for the current node list
	//
	for (int index = 0; index < node_list.Count (); index ++) {
		NodeClass *node = node_list[index];
		if (node != NULL) {
			scene->Reset_Vis_For_Node (node);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Build_Node_List
//
//////////////////////////////////////////////////////////////////////////////
void
GeneratingVisDialogClass::Build_Node_List (NODE_LIST &node_list)
{
	//
	//	Build a list of nodes that should be used to calculate
	// vis renders
	//
	VisMgrClass::Build_Node_List (node_list, m_SelectionOnly);

	//
	//	Determine which nodes to process if we
	// are in farm mode.
	//
	if (m_FarmMode) {
		int	nodes = node_list.Count ();
		float	nodes_per_processor = ((float)nodes) / ((float)m_TotalProcessors);

		int starting_node	= (int)::floor (nodes_per_processor * (float)m_ProcessorIndex);
		int ending_node	= (int)::ceil (nodes_per_processor * (float)(m_ProcessorIndex+1));
		ending_node = min (ending_node, nodes);

		//
		//	Copy the nodes from the total list into a temporary list
		//
		NODE_LIST temp_list;
		for (int index = starting_node; index < ending_node; index ++) {
			temp_list.Add (node_list[index]);
		}

		node_list.Delete_All ();
		node_list = temp_list;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	WindowProc
//
//////////////////////////////////////////////////////////////////////////////
LRESULT
GeneratingVisDialogClass::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
)
{
	if (message == WM_USER+101) {

		::Get_Main_View ()->Allow_Repaint (false);
		m_StartTime = ::GetTickCount();

		//
		//	Build a list of nodes we should use during the vis-generation
		//
		NODE_LIST node_list;
		Build_Node_List (node_list);

		//
		//	Reset vis if we are doing a selection set only
		//
		if (m_SelectionOnly) {
			Reset_Vis_Data_For_Nodes (node_list);
		}

		//
		//	Generate a list of points we need to render
		//
		VisPointGeneratorClass generator (m_Granularity);
		generator.Set_Ignore_Bias (m_IgnoreBias);
		generator.Set_Vis_Sample_Height (m_SampleHeight);
		Generate_Points (node_list, generator);

		//
		//	Update the dialog so the user know we are about to start rendering
		// vis points
		//
		::ShowWindow (::GetDlgItem (m_hWnd, IDC_BUILD_LIST_CHECK_ICON), SW_SHOW);
		m_ProgressCtrl.SetPos (0);
		General_Pump_Messages ();

		//
		//	Setup the vis render
		//
		SceneEditorClass *scene_editor = ::Get_Scene_Editor ();
		bool restore_vis_point_display = scene_editor->Are_Vis_Points_Displayed ();
		scene_editor->Display_Vis_Points (false);

		m_TotalPoints	= generator.Get_Total_Points ();
		m_CurrentPoint	= 0;
		scene_editor->Set_Total_Vis_Points (m_TotalPoints);

		//
		//	Render the generated vis points
		//
		VIS_POINT_LIST &point_list = generator.Peek_Point_List ();
		Render_Vis_Points (point_list);
		
		//
		//	Update the dialog so the user knows we are done
		//
		::ShowWindow (::GetDlgItem (m_hWnd, IDC_BUILD_LIST_CHECK_ICON), SW_SHOW);
		General_Pump_Messages ();

		if (restore_vis_point_display) {
			scene_editor->Display_Vis_Points (true);
		}

		// Close the dialog
		::Get_Main_View ()->Allow_Repaint (true);
		EndDialog (TRUE);
	}

	// Allow the base class to process this message
	return CDialog::WindowProc (message, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////
//
//	Update_Time
//
//////////////////////////////////////////////////////////////////////////////
void
GeneratingVisDialogClass::Update_Time (void)
{
	//
	// Compute the elapsed and estimated remaining time
	//
	DWORD cur_ticks = ::GetTickCount();
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
//	Render_Vis_Points
//
//////////////////////////////////////////////////////////////////////////////
void
GeneratingVisDialogClass::Render_Vis_Points (VIS_POINT_LIST &point_list)
{
	SceneEditorClass *scene_editor = ::Get_Scene_Editor ();
	VisLogClass &vis_log = scene_editor->Get_Vis_Log ();

	//
	//	Loop through all the points and vis-render them
	//
	int count = point_list.Count ();
	for (int index = 0; (index < count) && !m_bStop; index ++) {		

		VisPointListClass *sub_point_list = point_list[index];

		//
		//	Render vis for this point
		//
		Vector3 sample_point			= sub_point_list->sample_point;
		VisSampleClass vis_sample	= scene_editor->Update_Vis (sample_point, sub_point_list->transform);
		vis_log.Log_Sample (vis_sample);
		scene_editor->Create_Vis_Point (sub_point_list->transform);
		m_CurrentPoint ++;

		//
		//	Update the estimated time remaining
		//
		Update_Time ();

		//
		//	Render vis for any sub-points
		//		
		for (int sub_point = 0; sub_point < sub_point_list->Count (); sub_point ++) {
			Matrix3D &transform = (*sub_point_list)[sub_point];

			//
			// Render vis in each of the directions available to us
			//
			vis_sample = scene_editor->Update_Vis (sample_point, transform, VisDirBitsType(VIS_FORWARD_BIT | VIS_LEFT_BIT | VIS_RIGHT_BIT | VIS_UP_BIT | VIS_DOWN_BIT));
			vis_log.Log_Sample (vis_sample);
			scene_editor->Create_Vis_Point (transform);

			// Increment our total count of points
			m_CurrentPoint ++;

			//
			//	Update the estimated time remaining
			//
			Update_Time ();
		}

		//
		//	Output our current status to the status file
		//
		if (m_FarmMode) {
			CString status;
			status.Format ("Sectors: %d%%", int(((m_CurrentPoint + 1) * 100) / m_TotalPoints));
			::WritePrivateProfileString ("Status", m_StatusSection, status, m_StatusFilename);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Generate_Points
//
//////////////////////////////////////////////////////////////////////////////
void
GeneratingVisDialogClass::Generate_Points
(
	NODE_LIST &					node_list,
	VisPointGeneratorClass &generator
)
{
	//
	//	Count up the total number of polygongs
	//
	int total_polys	= 0;
	int count			= node_list.Count ();
	int index;
	for (index = 0; (index < count) && !m_bStop; index ++) {			
		NodeClass *node = node_list[index];
		RenderObjClass *rendobj = node->Peek_Render_Obj ();
		if (rendobj != NULL) {
			total_polys += rendobj->Get_Num_Polys ();
		}			
	}

	//
	//	Now, ask all the nodes to generate vis points
	//
	for (index = 0; (index < count) && !m_bStop; index ++) {
		
		//
		// Update the UI
		//
		int poly_count = generator.Get_Polys_Processed ();
		CString status_text;
		status_text.Format ("Processing %d / %d nodes, %d / %d polys.", index + 1, count, poly_count, total_polys);
		SetDlgItemText (IDC_STATUS_TEXT, status_text);

		CString elapsed_text;
		elapsed_text.Format ("Elapsed Time: %.1f minutes.",(::GetTickCount() - m_StartTime) / 60000.0f);
		SetDlgItemText (IDC_ELAPSED_TIME_TEXT, elapsed_text);
		
		m_ProgressCtrl.SetPos ((index * 100) / count);
		General_Pump_Messages ();			
		
		//
		//	Generate the points
		//
		NodeClass *node = node_list[index];
		node->Add_Vis_Points (generator);
	}

	return ;
}


