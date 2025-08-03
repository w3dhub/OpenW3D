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

// NodeInfoPageClass.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "NodeInfoPage.h"
#include "NodeMgr.H"
#include "rendobj.h"
#include "node.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// NodeInfoPageClass
//
/////////////////////////////////////////////////////////////////////////////
NodeInfoPageClass::NodeInfoPageClass (void)
	:	m_pNode (NULL),
		DockableFormClass(NodeInfoPageClass::IDD)
{
	//{{AFX_DATA_INIT(NodeInfoPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}

/////////////////////////////////////////////////////////////////////////////
//
// NodeInfoPageClass
//
/////////////////////////////////////////////////////////////////////////////
NodeInfoPageClass::NodeInfoPageClass (NodeClass *node)
	:	m_pNode (node),
		DockableFormClass(NodeInfoPageClass::IDD)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~NodeInfoPageClass
//
/////////////////////////////////////////////////////////////////////////////
NodeInfoPageClass::~NodeInfoPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
NodeInfoPageClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NodeInfoPageClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(NodeInfoPageClass, DockableFormClass)
	//{{AFX_MSG_MAP(NodeInfoPageClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NodeInfoPageClass diagnostics

#ifdef _DEBUG
void NodeInfoPageClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void NodeInfoPageClass::Dump(CDumpContext& dc) const
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
NodeInfoPageClass::HandleInitDialog (void)
{
	ASSERT (m_pNode != NULL);

	CString model_name;
	RenderObjClass *render_obj = m_pNode->Peek_Render_Obj ();
	if (render_obj != NULL) {
		model_name = render_obj->Get_Name ();
	}

	// Set the text of the respective dialog controls
	SetDlgItemText (IDC_NAME_EDIT,		m_pNode->Get_Name ());
	SetDlgItemText (IDC_MODELNAME_EDIT, model_name);
	SetDlgItemText (IDC_COMMENTS_EDIT,	m_pNode->Get_Comments ());
	SetDlgItemInt (IDC_ID_EDIT,			m_pNode->Get_ID ());
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
NodeInfoPageClass::Apply_Changes (void)
{
	bool retval = true;

	// Pass the new name onto the node
	CString name;
	GetDlgItemText (IDC_NAME_EDIT, name);	
	m_pNode->Set_Name (name);

	// Pass the new comments onto the base
	CString comments;
	GetDlgItemText (IDC_COMMENTS_EDIT, comments);
	m_pNode->Set_Comments (comments);
	
	// If the ID was changed then verify that it is unique before actually
	// changing it for the object.
	uint32 id = GetDlgItemInt(IDC_ID_EDIT, NULL, FALSE);	
	if (m_pNode->Get_ID() != id) {

		//
		//	Is this ID already used?
		//
		if (::Get_Node_Mgr().Verify_Unique_ID (id)) {

			//
			//	Is this ID in the range you were assigned?
			//
			uint32 min_id = 0;
			uint32 max_id = 0;
			NodeMgrClass::Get_ID_Range (m_pNode->Get_Type (), &min_id, &max_id);
			if (id >= min_id && id < max_id) {
				m_pNode->Set_ID (id);
			} else {
				MessageBox ("This ID is not in your allocated range.\n", "ID Error!");
				retval = false;
			}

		} else {
			MessageBox ("ID already in use!\nPlease enter a unique ID\n", "ID Error!");
			retval = false;
		}
	}

	return retval;
}

