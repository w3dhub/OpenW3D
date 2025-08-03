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

// PhysObjEditDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "PhysObjEditTab.h"
#include "SpecSheet.h"
#include "definitionmgr.h"
#include "definitionfactorymgr.h"
#include "definition.h"
#include "definitionfactory.h"
#include "Utils.h"
#include "phys.h"
#include "parameter.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// PhysObjEditTabClass
//
/////////////////////////////////////////////////////////////////////////////
PhysObjEditTabClass::PhysObjEditTabClass (void)
	:	m_ParamSheet (NULL),
		m_PhysDefParam (NULL),
		m_DefinitionID (0),
		m_IsTemp (false),
		m_ReadOnly (false),
		DockableFormClass (PhysObjEditTabClass::IDD)
{
	//{{AFX_DATA_INIT(PhysObjEditTabClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~PhysObjEditTabClass
//
/////////////////////////////////////////////////////////////////////////////
PhysObjEditTabClass::~PhysObjEditTabClass (void)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
PhysObjEditTabClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PhysObjEditTabClass)
	DDX_Control(pDX, IDC_TYPE_COMBO, m_ObjTypeCombo);
	DDX_Control(pDX, IDC_SETTINGS_GROUP, m_SettingsGroup);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PhysObjEditTabClass, DockableFormClass)
	//{{AFX_MSG_MAP(PhysObjEditTabClass)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_TYPE_COMBO, OnSelChangeTypeCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
/////////////////////////////////////////////////////////////////////////////
void
PhysObjEditTabClass::HandleInitDialog (void)
{
	DefinitionClass *default_definition = DefinitionMgrClass::Find_Definition (m_DefinitionID, false);
	uint32 default_classid = 0;
	
	if (default_definition != NULL) {
		default_classid = default_definition->Get_Class_ID ();
	}
	
	//
	//	Fill in the list of all the definition types
	//
	DefinitionFactoryClass *factory = NULL;
	for (	factory = DefinitionFactoryMgrClass::Get_First (CLASSID_PHYSICS);
			factory != NULL;
			factory = DefinitionFactoryMgrClass::Get_Next (factory, CLASSID_PHYSICS))
	{
		//
		//	If this is the default definition, then add the existing
		// definition instead of creating a new one.
		//
		if (default_classid == factory->Get_Class_ID ()) {
			int index = m_ObjTypeCombo.AddString (factory->Get_Name ());
			m_ObjTypeCombo.SetItemData (index, (ULONG)default_definition);			
			m_ObjTypeCombo.SetCurSel (index);
		} else {

			DefinitionClass *definition = factory->Create ();
			if (definition != NULL) {
				
				//
				//	If this definition passes the filter, then add it to
				// the combo-box, otherwise delete it
				//
				if (((PhysDefClass *)definition)->Is_Type (m_FilterString)) {
					int index = m_ObjTypeCombo.AddString (factory->Get_Name ());
					m_ObjTypeCombo.SetItemData (index, (ULONG)definition);				
				} else {
					SAFE_DELETE (definition);
				}
			}
		}
	}

	if (default_classid == 0) {
		m_ObjTypeCombo.SetCurSel (0);
	}

	if (m_ReadOnly) {
		m_ObjTypeCombo.EnableWindow (FALSE);
	}

	// Update the spec sheet based on the current object type
	OnSelChangeTypeCombo ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
PhysObjEditTabClass::Apply_Changes (void)
{
	CWaitCursor wait_cursor;

	if (m_ParamSheet != NULL) {		
		
		// Save the UI changes to the definition
		m_ParamSheet->Apply ();

		//
		// Determine what definition ID to store
		//
		DefinitionClass *definition	= m_ParamSheet->Get_Definition ();
		m_DefinitionID						= 0;
		if (definition != NULL) {
			
			//
			//	Does the definition already have an ID?
			//
			m_DefinitionID = definition->Get_ID ();
			if (m_DefinitionID == 0) {
				
				//
				//	Give the definition a new ID
				//
				if (m_IsTemp) {
					m_DefinitionID  = ::Get_Next_Temp_ID ();
				} else {
					m_DefinitionID = DefinitionMgrClass::Get_New_ID (definition->Get_Class_ID ());
				}
				definition->Set_ID (m_DefinitionID);
			}

			//
			//	Incorporate this definition into the framework
			//
			DefinitionMgrClass::Register_Definition (definition);

			//
			//	Pass this new definition ID onto the parameter-object we
			//	are editing (if there was one...)
			//
			if (m_PhysDefParam != NULL) {
				m_PhysDefParam->Set_Value (m_DefinitionID);
			}
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
PhysObjEditTabClass::OnDestroy (void)
{
	SAFE_DELETE (m_ParamSheet);

	for (int index = 0; index < m_ObjTypeCombo.GetCount (); index ++) {
		DefinitionClass *definition = (DefinitionClass *)m_ObjTypeCombo.GetItemData (index);
		if (definition != NULL) {
			
			//
			//	Delete all the definitions EXCEPT for the one we
			// will be returning to the caller.
			//
			if (definition->Get_ID () != m_DefinitionID) {
				DefinitionMgrClass::Unregister_Definition (definition);
				delete definition;
			}			
		}
	}

	DockableFormClass::OnDestroy ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelChangeTypeCombo
//
/////////////////////////////////////////////////////////////////////////////
void
PhysObjEditTabClass::OnSelChangeTypeCombo (void)
{
	if (::IsWindow (m_ObjTypeCombo)) {
		SetRedraw (false);		

		//
		//	Free the old param sheet if necessary
		//
		if (m_ParamSheet != NULL) {
			m_ParamSheet->Apply ();
			m_ParamSheet->DestroyWindow ();
			SAFE_DELETE (m_ParamSheet);
		}

		//
		//	Determine which definition to display
		//
		int index = m_ObjTypeCombo.GetCurSel ();
		if (index != CB_ERR) {
			DefinitionClass *definition = (DefinitionClass *)m_ObjTypeCombo.GetItemData (index);

			//
			//	Create the param sheet that is used to edit the definition's settings
			//
			CRect rect;
			m_SettingsGroup.GetWindowRect (&rect);
			ScreenToClient (&rect);
			rect.left	+= 10;
			rect.right	-= 10;
			rect.top		+= 15;
			rect.bottom	-= 10;
			m_ParamSheet = new SpecSheetClass (definition);
			m_ParamSheet->Set_Is_Temp (m_IsTemp);
			m_ParamSheet->Set_Read_Only (m_ReadOnly);
			m_ParamSheet->Create ("static", "", WS_CHILD | WS_VISIBLE, rect, this, 101);
		}

		SetRedraw (true);
		InvalidateRect (NULL, FALSE);
		UpdateWindow ();
	}

	return ;
}

