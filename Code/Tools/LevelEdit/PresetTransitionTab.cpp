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

// PresetGeneralTab.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "PresetTransitionTab.h"
#include "definition.h"
#include "preset.h"
#include "Utils.h"
#include "TransitionEditDialog.h"
#include "objectnode.h"
#include "rendobj.h"
#include "SceneEditor.h"
#include "decophys.h"
#include "CollisionGroups.h"
#include "staticphys.h"
#include "transition.h"
#include "wheelvehicle.h"
#include "cameramgr.h"
#include "hlod.h"
#include "rinfo.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Constants
//
/////////////////////////////////////////////////////////////////////////////
const float DROP_POS			= 1000.0F;
const float DROP_HEIGHT		= 5.0F;

const int COL_NAME			= 0;
const int COL_ANIMATION		= 1;


/////////////////////////////////////////////////////////////////////////////
//
// PresetTransitionTabClass
//
/////////////////////////////////////////////////////////////////////////////
PresetTransitionTabClass::PresetTransitionTabClass (PresetClass *preset)
	:	m_Preset (preset),
		m_Height (0),
		m_RenderObj (NULL),
		m_TransitionList (NULL),
		DockableFormClass(PresetTransitionTabClass::IDD)
{
	//{{AFX_DATA_INIT(PresetTransitionTabClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~PresetTransitionTabClass
//
/////////////////////////////////////////////////////////////////////////////
PresetTransitionTabClass::~PresetTransitionTabClass (void)
{
	MEMBER_RELEASE (m_RenderObj);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::DoDataExchange (CDataExchange *pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PresetTransitionTabClass)
	DDX_Control(pDX, IDC_TRANSITION_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PresetTransitionTabClass, DockableFormClass)
	//{{AFX_MSG_MAP(PresetTransitionTabClass)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TRANSITION_LIST, OnItemChangedTransitionList)
	ON_BN_CLICKED(IDC_ADD_BUTTON, OnAddButton)
	ON_BN_CLICKED(IDC_DELETE_BUTTON, OnDeleteButton)
	ON_BN_CLICKED(IDC_MODIFY_BUTTON, OnModifyButton)
	ON_NOTIFY(LVN_DELETEITEM, IDC_TRANSITION_LIST, OnDeleteitemTransitionList)
	ON_NOTIFY(NM_DBLCLK, IDC_TRANSITION_LIST, OnDblclkTransitionList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#ifdef _DEBUG
void PresetTransitionTabClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void PresetTransitionTabClass::Dump(CDumpContext& dc) const
{
	DockableFormClass::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::HandleInitDialog (void)
{
	ASSERT (m_Preset != NULL);

	m_ListCtrl.InsertColumn (COL_NAME, "Name");
	m_ListCtrl.InsertColumn (COL_ANIMATION, "Animation");

	//
	//	Add the transitions to the list ctrl
	//
	int count = m_TransitionList->Count ();
	for (int index = 0; index < count; index ++) {
				
		//
		//	Insert this transition into the control
		//
		TransitionDataClass *transition = (*m_TransitionList)[index];
		if (transition != NULL) {
			TransitionDataClass *our_copy = new TransitionDataClass;
			(*our_copy) = (*transition);
			Insert_Transition (our_copy);
		}
	}

	//
	//	Size the columns
	//
	CRect rect;
	m_ListCtrl.GetWindowRect (&rect);
	m_ListCtrl.SetColumnWidth (COL_NAME, (rect.Width () >> 1) - 2);
	m_ListCtrl.SetColumnWidth (COL_ANIMATION, (rect.Width () >> 1) - 2);

	m_ListCtrl.SetFocus ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetTransitionTabClass::Apply_Changes (void)
{
	// Assume success
	bool retval = true;

	//
	//	Free the old transitions
	//
	int count = m_TransitionList->Count ();
	int index;
	for (index = 0; index < count; index ++) {				
		TransitionDataClass *transition = (*m_TransitionList)[index];
		SAFE_DELETE (transition);
	}
	m_TransitionList->Delete_All ();

	//
	//	Now, take all the transitions from the list control and add
	// the to the vector.
	//
	count = m_ListCtrl.GetItemCount ();
	for (index = 0; index < count; index ++) {
		TransitionDataClass *transition = (TransitionDataClass *)m_ListCtrl.GetItemData (index);
		if (transition != NULL) {
			m_TransitionList->Add (transition);
			m_ListCtrl.SetItemData (index, NULL);
		}
	}
		
	// Return true to allow the dialog to close
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Transition
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::Insert_Transition (TransitionDataClass *transition)
{
	LPCTSTR name	= transition->Get_Type_Name (transition->Get_Type ());
	int index		= m_ListCtrl.InsertItem (0xFF, name);
	if (index >= 0) {
		m_ListCtrl.SetItemText (index, COL_ANIMATION, transition->Get_Animation_Name ());
		m_ListCtrl.SetItemData (index, (DWORD_PTR)transition);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAddButton
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::OnAddButton (void)
{
	Create_Render_Obj ();
	
	//
	//	Show the transition editor
	//
	if (m_RenderObj != NULL) {

		//
		//	Create a new transition
		//
		TransitionDataClass *transition = new TransitionDataClass;
		transition->Set_Type ((TransitionDataClass::StyleType)0);
		
		//
		//	Give the transition a starting position and size
		//
		AABoxClass box	= m_RenderObj->Get_Bounding_Box ();
		OBBoxClass zone (Vector3 (0, -(box.Extent.Y+0.5F), 0.5F), Vector3 (1, 1, 1));
		transition->Set_Zone (zone);
		transition->Set_Ending_TM (Matrix3D (zone.Center-Vector3 (0, 0, 0.5F)));

		//
		//	Show the transition edit dialog
		//
		TransitionEditDialogClass dialog (this);
		dialog.Set_Render_Obj (m_RenderObj);
		dialog.Set_Start_Height (m_Height);
		dialog.Set_Transition (transition);
		if (dialog.DoModal () == IDOK) {
			
			//
			//	Update the list ctrl
			//
			Insert_Transition (transition);
		} else {
			SAFE_DELETE (transition);
		}
	}

	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteButton
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::OnDeleteButton (void)
{
	//
	//	Get the currently selected item
	//
	int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index >= 0) {

		//
		//	Delete the transition and remove it from the UI
		//
		TransitionDataClass *transition = (TransitionDataClass *)m_ListCtrl.GetItemData (index);
		if (transition != NULL) {
			SAFE_DELETE (transition);
			m_ListCtrl.SetItemData (index, NULL);
		}
		m_ListCtrl.DeleteItem (index);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnModifyButton
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::OnModifyButton (void)
{
	Create_Render_Obj ();
	if (m_RenderObj != NULL) {

		//
		//	Get the currently selected item
		//
		int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
		if (index >= 0) {

			//
			//	Delete the transition and remove it from the UI
			//
			TransitionDataClass *transition = (TransitionDataClass *)m_ListCtrl.GetItemData (index);
			if (transition != NULL) {
				
				//
				//	Show the transition edit dialog
				//
				TransitionEditDialogClass dialog (this);
				dialog.Set_Render_Obj (m_RenderObj);
				dialog.Set_Start_Height (m_Height);
				dialog.Set_Transition (transition);
				if (dialog.DoModal () == IDOK) {

					//
					//	Update the list ctrl
					//
					LPCTSTR name = transition->Get_Type_Name (transition->Get_Type ());
					m_ListCtrl.SetItemText (index, COL_NAME, name);
					m_ListCtrl.SetItemText (index, COL_ANIMATION, transition->Get_Animation_Name ());
				}
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnItemChangedTransitionList
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::OnItemChangedTransitionList
(
	NMHDR *	/* pNMHDR */,
	LRESULT*	pResult
)
{
	(*pResult) = 0;

	int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index >= 0) {
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_MODIFY_BUTTON), true);
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_DELETE_BUTTON), true);
	} else {
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_MODIFY_BUTTON), false);
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_DELETE_BUTTON), false);
	}		

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Create_Render_Obj
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::Create_Render_Obj (void)
{
	CWaitCursor wait_cursor;

	if (m_RenderObj == NULL && m_Preset != NULL) {
		
		//
		//	Instantiate the node
		//
		NodeClass *node = (NodeClass *)m_Preset->Create ();
		ASSERT (node != NULL);
		if (node != NULL) {
			
			//
			//	Initialize the game object and get its render object pointer
			//
			node->Initialize ();			
			RenderObjClass *render_obj	= node->Peek_Render_Obj ();
			ASSERT (render_obj != NULL);
			if (render_obj != NULL) {

				PhysClass *phys_obj = node->Peek_Physics_Obj ();
				if (phys_obj != NULL && phys_obj->As_DecorationPhysClass () == NULL) {

					SceneEditorClass *scene = ::Get_Scene_Editor ();

					//
					//	Create a 'floor' which we can drop the object onto
					//
					PhysClass *phys_obj		= node->Peek_Physics_Obj ();				
					RenderObjClass *floor	= ::Create_Render_Obj ("GRID");
					if (floor != NULL) {
						phys_obj->Set_Transform (Matrix3D (Vector3(0, 0, DROP_POS)));

						//
						//	Determine what height we should drop the object from
						//
						float drop_height = DROP_HEIGHT;
						if (phys_obj->Peek_Model () != NULL) {
							drop_height = (phys_obj->Peek_Model ()->Get_Bounding_Box ().Extent.Z * 2.0F);
						}
						
						//
						//	Create the floor
						//
						StaticPhysClass *floor_phys = new StaticPhysClass;
						floor_phys->Set_Model (floor);
						floor_phys->Set_Transform (Matrix3D (Vector3(0, 0, DROP_POS-drop_height)));
						floor_phys->Set_Collision_Group (MOUSE_CLICK_COLLISION_GROUP);
						scene->Add_Static_Object (floor_phys);
						scene->Validate_Vis ();

						/*Matrix3D tm;
						tm.Look_At (Vector3 (0, -12, DROP_POS-drop_height+2), Vector3 (0, 0, DROP_POS-drop_height), 0);
						::Get_Camera_Mgr ()->Get_Camera ()->Set_Transform (tm);

						scene->Add_Dynamic_Object (phys_obj);*/

						//
						//	Drop the object onto the floor
						//
						for (int index = 0; index < 50; index ++) {
							phys_obj->Timestep (0.05F);
							if (phys_obj->As_VehiclePhysClass () != NULL) {
								phys_obj->As_VehiclePhysClass ()->Update_Wheels ();
							}
							//::Refresh_Main_View ();
						}

						//
						//	Determine how high the object sits ontop of the ground
						//
						m_Height = phys_obj->Get_Transform ().Get_Translation ().Z - (DROP_POS - drop_height);
						if (WWMath::Is_Valid_Float (m_Height) == false || WWMath::Fabs (m_Height) > 4.0F) {
							m_Height = 0.5F;
						}

						//
						//	Cleanup
						//
						//scene->Remove_Object (phys_obj);
						scene->Remove_Object (floor_phys);
						scene->Validate_Vis ();
						floor_phys->Release_Ref ();
						floor->Release_Ref ();
					}
				}

				//
				//	Keep a copy of the render object for our own uses.
				//
				m_RenderObj = render_obj->Clone ();

				//
				//	Force the object to use its highest LOD
				//
				if (m_RenderObj->Class_ID () == RenderObjClass::CLASSID_HLOD) {
					HLodClass *lod_obj = (HLodClass *)m_RenderObj;
					lod_obj->Set_LOD_Level (lod_obj->Get_LOD_Count () - 1);
				}
			}

			MEMBER_RELEASE (node);
		} else {
			::Message_Box (m_hWnd, IDS_CANT_CREATE_OBJECT_MSG, IDS_CANT_CREATE_OBJECT_TITLE);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemTransitionList
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::OnDeleteitemTransitionList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	//
	//	Delete the transition pointer
	//
	TransitionDataClass *transition = (TransitionDataClass *)m_ListCtrl.GetItemData (pNMListView->iItem);
	if (transition != NULL) {
		SAFE_DELETE (transition);
		m_ListCtrl.SetItemData (pNMListView->iItem, NULL);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemTransitionList
//
/////////////////////////////////////////////////////////////////////////////
void
PresetTransitionTabClass::OnDblclkTransitionList
(
	NMHDR *	/* pNMHDR */,
	LRESULT *pResult
)
{
	(*pResult) = 0;
	
	OnModifyButton ();
	return ;
}
