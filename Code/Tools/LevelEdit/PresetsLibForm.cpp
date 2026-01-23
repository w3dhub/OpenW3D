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

// PresetsForm.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "presetslibform.h"
#include "Utils.h"
#include "filemgr.h"
#include "definitionfactorymgr.h"
#include "definitionclassids.h"
#include "definitionfactory.h"
#include "definitionmgr.h"
#include "definition.h"
#include "icons.h"
#include "terraindefinition.h"
#include "preset.h"
#include "presetmgr.h"
#include "sceneeditor.h"
#include "saveload.h"
#include "rawfile.h"
#include "chunkio.h"
#include "filelocations.h"
#include "nodecategories.h"
#include "editorchunkids.h"
#include "regkeys.h"
#include "nodemgr.h"
#include "LevelEditView.h"
#include "PlaySoundDialog.h"
#include "AudibleSound.h"
#include "UpdatePresetDialog.h"
#include "definitionutils.h"
#include "presetlogger.h"
#include "rendobj.h"
#include "phys.h"
#include "textfile.h"
#include "node.h"
#include "ramfile.h"
#include "editorbuild.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////
//
//	Helper objects
//
///////////////////////////////////////////////////////////////////////
class SelectionCaptureClass
{
public:
	SelectionCaptureClass (PresetsFormClass *preset_form)
	{
		m_PresetForm							= preset_form;
		HTREEITEM tree_item					= m_PresetForm->m_TreeCtrl.GetSelectedItem ();
		PresetClass *preset					= m_PresetForm->Get_Item_Preset (tree_item);
		DefinitionFactoryClass *factory	= m_PresetForm->Get_Item_Factory (tree_item);
		if (preset != NULL) {
			m_ID = preset->Get_ID ();
			m_IsFactory = false;
		} else {
			m_ID = factory->Get_Class_ID ();
			m_IsFactory = true;
		}
		return ;
	}

	void Restore (void)
	{
		m_PresetForm->Reselect_Item (m_ID, m_IsFactory);
		return ;
	}

private:
	PresetsFormClass *	m_PresetForm;
	int						m_ID;
	bool						m_IsFactory;
};


///////////////////////////////////////////////////////////////////////
//
//	Macros
//
///////////////////////////////////////////////////////////////////////
#ifndef TreeView_SetOverlay
   #define TreeView_SetOverlay(hwndLV, i, overlay) \
      {	\
			TVITEMEX item_info = { TVIF_STATE, i, INDEXTOOVERLAYMASK(static_cast<UINT>(overlay)), TVIS_OVERLAYMASK, 0 };	\
			TreeView_SetItem(hwndLV, &item_info);	\
		}
#endif


///////////////////////////////////////////////////////////////////////
//
//	Constants
//
///////////////////////////////////////////////////////////////////////
typedef enum
{
	TYPE_FACTORY,
	TYPE_PRESET,
} ITEM_TYPE;

typedef struct
{
	ITEM_TYPE	type;
	union
	{
		DefinitionFactoryClass *factory;
		PresetClass *preset;
	};

} ITEM_DATA;


static const int TOOLBAR_HEIGHT					= 36;
static const int TOOLBAR_V_SPACING				= 5;
static const int TOOLBAR_V_BORDER				= TOOLBAR_V_SPACING * 2;
static const int TOOLBAR_H_SPACING				= 5;
static const int TOOLBAR_H_BORDER				= TOOLBAR_H_SPACING * 2;


/////////////////////////////////////////////////////////////////////////////
//
// PresetsFormClass
//
/////////////////////////////////////////////////////////////////////////////
PresetsFormClass::PresetsFormClass(CWnd *parent_wnd)
	:	m_DragItem (NULL),
		m_IsDragging (false),
		m_DragImageList (NULL),
		CDialog(PresetsFormClass::IDD)
{
	//{{AFX_DATA_INIT(PresetsFormClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Create (PresetsFormClass::IDD, parent_wnd);
	return ;
}

PresetsFormClass::~PresetsFormClass()
{
	return ;
}

void PresetsFormClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PresetsFormClass)
	DDX_Control(pDX, IDC_PRESETS_TREE, m_TreeCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PresetsFormClass, CDialog)
	//{{AFX_MSG_MAP(PresetsFormClass)
	ON_WM_SIZE()
	ON_COMMAND(IDC_ADD, OnAdd)
	ON_NOTIFY(TVN_DELETEITEM, IDC_PRESETS_TREE, OnDeleteitemPresetsTree)
	ON_COMMAND(IDC_MODIFY, OnModify)
	ON_COMMAND(IDC_MAKE, OnMake)
	ON_COMMAND(IDC_ADD_TEMP, OnAddTemp)
	ON_COMMAND(IDC_DELETE, OnDelete)
	ON_NOTIFY(TVN_SELCHANGED, IDC_PRESETS_TREE, OnSelchangedPresetsTree)
	ON_COMMAND(IDC_PLAY, OnPlay)
	ON_COMMAND(IDC_INFO, OnInfo)
	ON_COMMAND(IDC_EXTRA, OnExtra)
	ON_UPDATE_COMMAND_UI(IDC_CONVERT, OnUpdateConvert)
	ON_UPDATE_COMMAND_UI(IDC_INFO, OnUpdateInfo)
	ON_UPDATE_COMMAND_UI(IDC_PLAY, OnUpdatePlay)
	ON_UPDATE_COMMAND_UI(IDC_UPDATE_VSS, OnUpdateUpdateVss)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_COMMAND(IDC_CONVERT, OnConvert)
	ON_COMMAND(IDC_UPDATE_VSS, OnUpdateVss)
	ON_COMMAND(IDM_BATCH_IMPORT_TERRAIN, OnBatchImportTerrain)
	ON_COMMAND(IDM_UPDATE_EMBEDDED_NODE_LIST, OnBuildEmbedNodeList)
	ON_COMMAND(IDM_CLEAR_EMBEDDED_NODE_LIST, OnClearEmbeddedNodeList)	
	ON_UPDATE_COMMAND_UI(IDM_BATCH_IMPORT_TERRAIN, OnUpdateBatchImportTerrain)
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_EMBEDDED_NODE_LIST, OnUpdateEmbeddedNodeList)
	ON_UPDATE_COMMAND_UI(IDM_CLEAR_EMBEDDED_NODE_LIST, OnUpdateClearEmbeddedNodeList)	
	ON_NOTIFY(TVN_BEGINDRAG, IDC_PRESETS_TREE, OnBegindragPresetsTree)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PresetsFormClass diagnostics

#ifdef _DEBUG
void PresetsFormClass::AssertValid() const
{
	CDialog::AssertValid();
}

void PresetsFormClass::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnSize
(
	UINT	nType,
	int	cx,
	int	cy
)
{
	CDialog::OnSize(nType, cx, cy);
	
	if (::IsWindow (m_TreeCtrl) && (cx > 0) && (cy > 0)) {

		// Get the bounding rectangle of the form window
		CRect parentrect;
		GetWindowRect (&parentrect);
		
		// Get the bounding rectangle of the toolbar
		CRect toolbar_rect;
		m_Toolbar.GetWindowRect (&toolbar_rect);
		ScreenToClient (&toolbar_rect);

		// Move the toolbar so it is in its correct position
		m_Toolbar.SetWindowPos (NULL,
								TOOLBAR_H_SPACING,
								(cy - TOOLBAR_V_SPACING) - toolbar_rect.Height (),
								cx - TOOLBAR_H_BORDER,
								toolbar_rect.Height (),
								SWP_NOZORDER);

		// Get the bounding rectnagle of the list ctrl
		RECT list_rect;
		m_TreeCtrl.GetWindowRect (&list_rect);
				
		CRect client_rect = list_rect;
		ScreenToClient (&client_rect);
		int list_height = ((cy - TOOLBAR_V_BORDER) - toolbar_rect.Height ()) - client_rect.top;

		//
		// Resize the tab control to fill the empty contents of the client area
		//
		m_TreeCtrl.SetWindowPos (	NULL,
									0,
									0,
									cx-((list_rect.left-parentrect.left) << 1),
									list_height,
									SWP_NOZORDER | SWP_NOMOVE);	
	}
		
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
PresetsFormClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	CRect rect;
	GetClientRect (&rect);
	m_Toolbar.CreateEx (this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP, CRect(0, 0, 0, 0), 101);
	m_Toolbar.SetOwner (this);
	m_Toolbar.LoadToolBar (IDR_PRESET_TOOLBAR1);
	m_Toolbar.SetBarStyle (m_Toolbar.GetBarStyle () | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_Toolbar.GetToolBarCtrl ().SetExtendedStyle (TBSTYLE_EX_DRAWDDARROWS);
	
	TBBUTTONINFO button_info = { sizeof (TBBUTTONINFO), TBIF_STYLE, 0 };
	m_Toolbar.GetToolBarCtrl ().GetButtonInfo (IDC_EXTRA, &button_info);
	button_info.fsStyle |= TBSTYLE_DROPDOWN;
	m_Toolbar.GetToolBarCtrl ().SetButtonInfo (IDC_EXTRA, &button_info);

	//
	// Get the bounding rectangle of the form window
	//
	CRect parentrect;
	GetWindowRect (&parentrect);
	m_Toolbar.SetWindowPos (NULL, 0, 0, parentrect.Width () - TOOLBAR_H_BORDER, TOOLBAR_HEIGHT, SWP_NOZORDER | SWP_NOMOVE);	
	m_Toolbar.Enable_Button (IDC_ADD,		Enable_Button (IDC_ADD));
	m_Toolbar.Enable_Button (IDC_DELETE,	Enable_Button (IDC_DELETE));

	//
	// Pass the general use imagelist onto the tree control
	//
	m_TreeCtrl.SetImageList (::Get_Global_Image_List (), TVSIL_NORMAL);
	Reload_Presets ();
	
	//
	//	Select the first item in the tree control
	//
	HTREEITEM first_item = m_TreeCtrl.GetNextItem (NULL, TVGN_CHILD);
	if (first_item != NULL) {
		m_TreeCtrl.SelectItem (first_item);
	}

	SetProp (m_TreeCtrl, "TRANS_ACCS", (HANDLE)1);
	SetProp (m_hWnd, "TRANS_ACCS", (HANDLE)1);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Reload_Presets
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Reload_Presets (void)
{
	m_TreeCtrl.SetRedraw (false);
	m_TreeCtrl.SelectItem (NULL);
	m_TreeCtrl.DeleteAllItems ();

	//
	//	Loop through and add all the registered definition factories to
	// the tree control
	//
	for (int index = 0; index < PRESET_CATEGORY_COUNT; index ++) {

		//
		//	Add this factory to the tree
		//
		HTREEITEM tree_item = m_TreeCtrl.InsertItem (PRESET_CATEGORIES[index].name, FOLDER_ICON, FOLDER_ICON);
			
		//
		//	Is there a factory to create this class of defintion?
		//
		DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Find_Factory (PRESET_CATEGORIES[index].clsid);
		if (factory != NULL) {
				
			//
			//	Add all presets for this factory into the tree
			//
			if (factory->Is_Displayed ()) {
				Set_Item_Data (tree_item, factory);
				Fill_In_Presets (tree_item);
			}
		} else {

			//
			//	Find all the sub-factories
			//			
			for (	factory = DefinitionFactoryMgrClass::Get_First (PRESET_CATEGORIES[index].clsid);
					factory != NULL;
					factory = DefinitionFactoryMgrClass::Get_Next (factory, PRESET_CATEGORIES[index].clsid))
			{
				//
				//	Add this sub-factory and all its definitions to the tree
				//
				if (factory->Is_Displayed ()) {
					HTREEITEM child_item = m_TreeCtrl.InsertItem (factory->Get_Name (), FOLDER_ICON, FOLDER_ICON, tree_item);
					Set_Item_Data (child_item, factory);
					Fill_In_Presets (child_item);
				}
			}
		}
	}
	
	Sort_Nodes (TVI_ROOT);
	m_TreeCtrl.SetRedraw (true);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_In_Presets
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Fill_In_Presets (HTREEITEM root_item)
{
	DefinitionFactoryClass *factory = Get_Item_Factory (root_item);
	if (factory != NULL && factory->Is_Displayed ()) {
		uint32 class_id = factory->Get_Class_ID ();

		//
		//	Find all the presets that belong to this class
		//
		PresetClass *preset = NULL;
		for (	preset = PresetMgrClass::Get_First (class_id, PresetMgrClass::ID_CLASS);
				preset != NULL;
				preset = PresetMgrClass::Get_Next (preset, class_id, PresetMgrClass::ID_CLASS))
		{
			
			//
			//	Add this preset to the tree
			//
			if (preset->Get_Parent () == NULL) {
				HTREEITEM tree_item = Insert_Preset (root_item, preset);
				if (tree_item != NULL) {

					//
					//	Recursively fill in this definition's 'children'
					//
					Fill_In_Preset_Children (tree_item, preset->Get_ID ());
				}
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_In_Preset_Children
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Fill_In_Preset_Children
(
	HTREEITEM	root_item,
	uint32		parent_id
)
{
	//
	//	Lookup the parent preset
	//
	PresetClass *parent_preset = PresetMgrClass::Find_Preset (parent_id);	
	if (parent_preset == NULL) {
		return ;
	}

	//
	//	Loop over all the children of this preset
	//
	int count = parent_preset->Get_Child_Preset_Count ();
	for (int index = 0; index < count; index ++) {
		PresetClass *child_preset = parent_preset->Get_Child_Preset (index);
		if (child_preset != NULL) {

			//
			//	Check to make sure we have no recursive links
			//
			PresetClass *parent = child_preset->Get_Parent ();
			if (parent == child_preset || parent->Get_ID () == child_preset->Get_ID ()) {
				CString message;
				message.Format ("Preset '%s' is recursively linked to itself.\r\n", child_preset->Get_Name ());
				MessageBox (message, "Preset Error", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
				continue ;
			}

			//
			//	Check to make sure the parent is correct
			//
			if (parent != NULL && child_preset->Get_Class_ID () != parent->Get_Class_ID ()) {
				CString message;
				message.Format ("Preset '%s' is incorrectly linked as a child of '%s'.\r\n", child_preset->Get_Name (), parent->Get_Name ());
				MessageBox (message, "Preset Error", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
			}
			
			//
			//	Add this preset to the tree
			//
			HTREEITEM tree_item = Insert_Preset (root_item, child_preset);
			if (tree_item != NULL) {

				//
				//	Recursively fill in this definition's 'children'
				//
				Fill_In_Preset_Children (tree_item, child_preset->Get_ID ());
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Find_Preset
//
/////////////////////////////////////////////////////////////////////////////
HTREEITEM
PresetsFormClass::Find_Preset (HTREEITEM root_item, uint32 id)
{
	HTREEITEM preset_item = NULL;

	//
	//	Look for the preset in this item's children (recursive)
	//
	for (	HTREEITEM tree_item = m_TreeCtrl.GetChildItem (root_item);
			(tree_item != NULL) && (preset_item == NULL);
			tree_item = m_TreeCtrl.GetNextSiblingItem (tree_item))
	{
		DefinitionClass *definition = Get_Item_Definition (tree_item);

		//
		// Is this the preset we were looking for?
		//
		if ((definition != NULL) && (definition->Get_ID () == id)) {
			preset_item = tree_item;
		} else if (m_TreeCtrl.ItemHasChildren (tree_item)) {
			
			//
			// Recursively call this method for all sub-folders in this tree
			//
			preset_item = Find_Preset (tree_item, id);
		}
	}

	return preset_item;
}


/////////////////////////////////////////////////////////////////////////////
//
// Find_Factory
//
/////////////////////////////////////////////////////////////////////////////
HTREEITEM
PresetsFormClass::Find_Factory (HTREEITEM root_item, uint32 id)
{
	HTREEITEM factory_item = NULL;

	//
	//	Look for the preset in this item's children (recursive)
	//
	for (	HTREEITEM tree_item = m_TreeCtrl.GetChildItem (root_item);
			(tree_item != NULL) && (factory_item == NULL);
			tree_item = m_TreeCtrl.GetNextSiblingItem (tree_item))
	{
		DefinitionFactoryClass *factory = Get_Item_Factory (tree_item);

		//
		// Is this the factory we were looking for?
		//
		if ((factory != NULL) && (factory->Get_Class_ID () == id)) {
			factory_item = tree_item;
		} else if (m_TreeCtrl.ItemHasChildren (tree_item)) {
			
			//
			// Recursively call this method for all sub-folders in this tree
			//
			factory_item = Find_Factory (tree_item, id);
		}
	}

	return factory_item;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Selected_Factory
//
/////////////////////////////////////////////////////////////////////////////
DefinitionFactoryClass *
PresetsFormClass::Get_Selected_Factory (void)
{	
	HTREEITEM curr_item					= m_TreeCtrl.GetSelectedItem ();
	DefinitionFactoryClass *factory	= Get_Item_Factory (curr_item);
		
	// Does this item represent a definition?
	if (factory == NULL) {		
		
		//
		//	Find the defintion's factory (if possible)
		//
		DefinitionClass *definition = Get_Item_Definition (curr_item);
		if (definition != NULL) {
			uint32 class_id = definition->Get_Class_ID ();
			factory = DefinitionFactoryMgrClass::Find_Factory (class_id);
		} 		
	}

	return factory;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Item_Data
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Set_Item_Data (HTREEITEM item, DefinitionFactoryClass *factory)
{
	//
	//	Allocate a new wrapper if we need to
	//
	ITEM_DATA *item_data = (ITEM_DATA *)m_TreeCtrl.GetItemData (item);
	if (item_data == NULL) {
		item_data = new ITEM_DATA;
	}

	//
	//	Set a flag in the wrapper so we know what kind of data
	// this tree item contains.
	//
	item_data->type		= TYPE_FACTORY;
	item_data->factory	= factory;
	m_TreeCtrl.SetItemData (item, (DWORD_PTR)item_data);
	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Item_Data
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Set_Item_Data (HTREEITEM item, PresetClass *preset)
{
	//
	//	Allocate a new wrapper if we need to
	//
	ITEM_DATA *item_data = (ITEM_DATA *)m_TreeCtrl.GetItemData (item);
	if (item_data == NULL) {
		item_data = new ITEM_DATA;
	}

	//
	//	Set a flag in the wrapper so we know what kind of data
	// this tree item contains.
	//
	item_data->type	= TYPE_PRESET;
	item_data->preset	= preset;
	m_TreeCtrl.SetItemData (item, (DWORD_PTR)item_data);
	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Item_Factory
//
/////////////////////////////////////////////////////////////////////////////
DefinitionFactoryClass *
PresetsFormClass::Get_Item_Factory (HTREEITEM item)
{
	DefinitionFactoryClass *factory = NULL;
	if (item != NULL) {

		//
		//	If this item represents a factory, then return the factory
		//	pointer to the caller.
		//
		ITEM_DATA *item_data = (ITEM_DATA *)m_TreeCtrl.GetItemData (item);
		if ((item_data != NULL) && (item_data->type == TYPE_FACTORY)) {
			factory = item_data->factory;
		}
	}

	return factory;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Item_Definition
//
/////////////////////////////////////////////////////////////////////////////
DefinitionClass *
PresetsFormClass::Get_Item_Definition (HTREEITEM item)
{
	DefinitionClass *definition = NULL;
	if (item != NULL) {

		//
		//	Get the definition from the preset this item
		// represents
		//
		PresetClass *preset = Get_Item_Preset (item);
		if (preset != NULL) {
			definition = preset->Get_Definition ();
		}
	}

	return definition;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Item_Preset
//
/////////////////////////////////////////////////////////////////////////////
PresetClass *
PresetsFormClass::Get_Item_Preset (HTREEITEM item)
{
	PresetClass *preset = NULL;
	if (item != NULL) {

		//
		//	If this item represents a definition, then return the definition
		//	pointer to the caller.
		//
		ITEM_DATA *item_data = (ITEM_DATA *)m_TreeCtrl.GetItemData (item);
		if ((item_data != NULL) && (item_data->type == TYPE_PRESET)) {
			preset = item_data->preset;
		}
	}

	return preset;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Icon
//
/////////////////////////////////////////////////////////////////////////////
int
PresetsFormClass::Get_Icon (PresetClass *preset)
{
	int icon = FOLDER_ICON;
	
	DefinitionClass *definition = preset->Get_Definition ();
	if (definition != NULL) {
		int class_id		= definition->Get_Class_ID ();
		int superclass_id	= ::SuperClassID_From_ClassID (class_id);
		switch (superclass_id) {

			case CLASSID_TERRAIN:
				icon = TERRAIN_ICON;
				break;

			case CLASSID_TILE:
				icon = TILE_ICON;
				break;

			case CLASSID_GAME_OBJECTS:
			case CLASSID_MUNITIONS:
			case CLASSID_DUMMY_OBJECTS:
				if (class_id == CLASSID_GAME_OBJECT_DEF_BUILDING) {
					icon = BUILDING_ICON;
				} else {
					icon = OBJECT_ICON;
				}
				break;

			case CLASSID_LIGHT:
				icon = LIGHT_ICON;
				break;

			case CLASSID_SOUND:
				icon = SOUND_ICON;
				break;

			case CLASSID_WAYPATH:
				icon = WAYPATH_ICON;
				break;

			case CLASSID_ZONE:
				icon = ZONE_ICON;
				break;

			case CLASSID_TRANSITION:
				icon = TRANSITION_ICON;
				break;

			case CLASSID_TWIDDLERS:
				icon = RAND_ICON;
				break;

			case CLASSID_BUILDINGS:
				icon = BUILDING_ICON;
				break;

			case CLASSID_EDITOR_OBJECTS:
				if (class_id == CLASSID_VIS_POINT_DEF) {
					icon = VIS_ICON;
				} else  if (class_id == CLASSID_PATHFIND_START_DEF) {
					icon = PATHFIND_ICON;
				}
				break;

			case CLASSID_GLOBAL_SETTINGS:
				icon = FILE_ICON;
				break;
		}
	}

	if (preset->Get_IsTemporary ()) {
		icon = TEMP_ICON;
	}

	return icon;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Preset
//
/////////////////////////////////////////////////////////////////////////////
HTREEITEM
PresetsFormClass::Insert_Preset
(
	HTREEITEM		root_item,
	PresetClass *	preset
)
{
	int icon = Get_Icon (preset);

	//
	//	Insert a new node into the tree to represent this item
	//
	HTREEITEM tree_item = m_TreeCtrl.InsertItem (preset->Get_Name (), icon, icon, root_item);
	if (tree_item != NULL) {
		Set_Item_Data (tree_item, preset);

		bool needs_overlay = (preset->Get_Node_List ().Count () > 0);
		TreeView_SetOverlay (m_TreeCtrl, tree_item, needs_overlay);
	}	

	return tree_item;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeleteitemPresetsTree
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnDeleteitemPresetsTree
(
	NMHDR *	pNMHDR,
	LRESULT *pResult
)
{
	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;
	(*pResult) = 0;

	//
	// Free the wrapper we installed around this item
	//
	ITEM_DATA *item_data = (ITEM_DATA *)m_TreeCtrl.GetItemData (pNMTreeView->itemOld.hItem);
	SAFE_DELETE (item_data);
	m_TreeCtrl.SetItemData (pNMTreeView->itemOld.hItem, 0L);	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnModify
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Modify_Preset (void)
{
	bool retval = false;

	HTREEITEM current_item	= m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset		= Get_Item_Preset (current_item);
	if (preset != NULL) {

		StringClass old_name = preset->Get_Name ();

		//
		//	Show the properties dialog for this preset
		//
		if (preset->Show_Properties ()) {
			
			//
			//	Check to see if the user has renamed the preset
			//
			StringClass new_name = preset->Get_Name ();
			if (::lstrcmp (old_name, new_name) != 0) {
				
				//
				//	Rename the item in the tree
				//
				m_TreeCtrl.SetRedraw (false);
				m_TreeCtrl.SetItemText (current_item, preset->Get_Name ());
				m_TreeCtrl.SetRedraw (true);
				m_TreeCtrl.InvalidateRect (NULL, true);

				//
				//	Log the rename operation
				//
				if (preset->Get_IsTemporary () == false) {
					PresetLoggerClass::Log_Renamed (old_name, new_name);
				}
			}

			retval = true;
		}
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnModify
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnModify (void) 
{
	HTREEITEM current_item	= m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset		= Get_Item_Preset (current_item);
	if (preset != NULL) {

		if (preset->Get_IsTemporary () == false) {

			//
			//	Check out the definition database
			//
			SelectionCaptureClass sel_capture (this);
			bool undo_on_err	= false;
			uint32 class_id	= preset->Get_Class_ID ();
			if (PresetMgrClass::Check_Out_Database (class_id, &undo_on_err)) {
				sel_capture.Restore ();

				//
				//	Re-get the preset (its pointer has changed, the
				// old one is crap).
				//
				current_item	= m_TreeCtrl.GetSelectedItem ();
				preset			= Get_Item_Preset (current_item);
				if (preset != NULL) {

					//
					//	Show the preset settings
					//
					if (Modify_Preset ()) {
						Save_Global_Presets (class_id);
						PresetMgrClass::Add_Dirty_Preset (preset->Get_ID ());						
					} else if (undo_on_err) {
						PresetMgrClass::Undo_Database_Check_Out (class_id);
					}
				}
			}

		} else {
			
			//
			//	Show the preset settings
			//
			if (Modify_Preset ()) {
				Save_Temp_Presets ();				
			}
		}

		NodeMgrClass::Reload_Nodes (preset);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Load_Databases
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Load_Databases (void)
{
	// Start fresh
	PresetMgrClass::Free_Presets ();
	DefinitionMgrClass::Free_Definitions ();
	
	bool retval = false;

#ifdef PUBLIC_EDITOR_VER
	
	//
	//	Reload the presets
	//
	retval = Old_Load_Presets ();
	retval	&=	Load_Temp_Presets ();

#else
	
	//
	//	Reload the presets
	//
	retval	=	Load_Preset_Libraries ();
	retval	&=	Load_Temp_Presets ();

#endif //PUBLIC_EDITOR_VER
	
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Load_Preset_Libraries
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Load_Preset_Libraries (void)
{
	//
	//	Try to load the library for each factory
	//
	DefinitionFactoryClass *factory = NULL;
	for (	factory = DefinitionFactoryMgrClass::Get_First ();
			factory != NULL;
			factory = DefinitionFactoryMgrClass::Get_Next (factory))
	{
		if (factory->Is_Displayed ()) {
			Load_Presets (factory->Get_Class_ID ());
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Load_Presets
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Load_Presets (uint32 class_id)
{
	//
	//	Build a filename for the presets library
	//
	CString path;
	::Get_File_Mgr ()->Get_Preset_Library_Path (class_id, false, path);

	//
	//	Open the file
	//
	HANDLE file = ::CreateFile (	path,
											GENERIC_READ,
											FILE_SHARE_READ,
											NULL,
											OPEN_EXISTING,
											0L,
											NULL);

	if (file != INVALID_HANDLE_VALUE) {

		//
		//	Determine how large the file is
		//
		int file_size = ::GetFileSize (file, NULL);
		if (file_size > 0 && file_size < 99999999) 
		{
			//
			//	Read the file into memory
			//
			unsigned char *buffer = new unsigned char[file_size];
			DWORD bytes_read = 0;
			::ReadFile (file, buffer, file_size, &bytes_read, NULL);

			//
			//	Close the file
			//
			::CloseHandle (file);
			file = INVALID_HANDLE_VALUE;

			//
			//	Use a RAM file for the loading
			//
			RAMFileClass file_obj (buffer, file_size);
			file_obj.Open (FileClass::READ);
			ChunkLoadClass chunk_load (&file_obj);

			//
			//	Load the file
			//
			SaveLoadSystemClass::Load (chunk_load);

			//
			//	Free the buffer
			//
			delete [] buffer;
			buffer = NULL;
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Old_Load_Presets
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Old_Load_Presets (void)
{
	CString path = ::Get_File_Mgr ()->Make_Full_Path (OBJECTS_DDB_PATH);

	//
	//	Open the file
	//
	HANDLE file = ::CreateFile (path,
										  GENERIC_READ,
										  FILE_SHARE_READ,
										  NULL,
										  OPEN_EXISTING,
										  0L,
										  NULL);

	if (file != INVALID_HANDLE_VALUE) {

		//
		//	Determine how large the file is
		//
		int file_size = ::GetFileSize (file, NULL);
		if (file_size > 0 && file_size < 99999999) 
		{
			//
			//	Read the file into memory
			//
			unsigned char *buffer = new unsigned char[file_size];
			DWORD bytes_read = 0;
			::ReadFile (file, buffer, file_size, &bytes_read, NULL);

			//
			//	Close the file
			//
			::CloseHandle (file);
			file = INVALID_HANDLE_VALUE;

			//
			//	Use a RAM file for the loading
			//
			RAMFileClass file_obj (buffer, file_size);
			file_obj.Open (FileClass::READ);
			ChunkLoadClass chunk_load (&file_obj);

			//
			//	Load the file
			//
			SaveLoadSystemClass::Load (chunk_load);

			//
			//	Free the buffer
			//
			delete [] buffer;
			buffer = NULL;
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Load_Temp_Presets
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Load_Temp_Presets (void)
{
	CString path = ::Get_File_Mgr ()->Make_Full_Path (TEMP_DB_PATH);

	//
	//	Open the file
	//
	HANDLE hfile = ::CreateFile (path,
										  GENERIC_READ,
										  FILE_SHARE_READ,
										  NULL,
										  OPEN_EXISTING,
										  0L,
										  NULL);

	if (hfile != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (hfile);
		ChunkLoadClass chunk_load (&file_obj);

		//
		//	Load the file
		//
		SaveLoadSystemClass::Load (chunk_load);
	}
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Save_Temp_Presets
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Save_Temp_Presets (void)
{
	Save_Presets (0, true);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Save_Global_Presets
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Save_Global_Presets ([[maybe_unused]] uint32 class_id)
{

#ifndef PUBLIC_EDITOR_VER
	
	Save_Presets (class_id, false);

#endif //PUBLIC_EDITOR_VER

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Build_DDB_File_List
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Build_DDB_File_List
(
	DynamicVectorClass<CString> &	file_list,
	HTREEITEM							parent_item
)
{
	//
	//	Loop over all the immediate children
	//
	for (	HTREEITEM child_item = m_TreeCtrl.GetChildItem (parent_item);
			child_item != NULL;
			child_item = m_TreeCtrl.GetNextSiblingItem (child_item))
	{
		//
		//	If this tree item represents a definition factory, then
		// we should have a preset library to match
		//
		DefinitionFactoryClass *factory = Get_Item_Factory (child_item);
		if (factory != NULL) {			

			//
			//	Build a filename for this preset library
			//
			CString path;
			::Get_File_Mgr ()->Get_Preset_Library_Path (factory->Get_Class_ID (), false, path);

			//
			//	Add this preset library to the list
			//
			file_list.Add (path);

		} else if (m_TreeCtrl.ItemHasChildren (child_item)) {
			
			//
			// Recursively call this method for all sub-folders in this tree
			//		
			Build_DDB_File_List (file_list, child_item);
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Save_Preset_Libraries
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Save_Preset_Libraries ([[maybe_unused]] HTREEITEM parent_item)
{

#ifdef PUBLIC_EDITOR_VER

	//
	//	Simply save the presets together in one file
	//
	CString path = ::Get_File_Mgr ()->Make_Full_Path (OBJECTS_DDB_PATH);
	Save_Presets (path, 0, false, false);		

#else

	//
	//	Loop over all the immediate children
	//
	for (	HTREEITEM child_item = m_TreeCtrl.GetChildItem (parent_item);
			child_item != NULL;
			child_item = m_TreeCtrl.GetNextSiblingItem (child_item))
	{
		//
		//	If this tree item represents a definition factory, then save
		// a preset library for all presets of this class ID
		//
		DefinitionFactoryClass *factory = Get_Item_Factory (child_item);
		if (factory != NULL) {			
			Save_Global_Presets (factory->Get_Class_ID ());
		} else if (m_TreeCtrl.ItemHasChildren (child_item)) {
			
			//
			// Recursively call this method for all sub-folders in this tree
			//		
			Save_Preset_Libraries (child_item);
		}
	}

#endif //PUBLIC_EDITOR_VER
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Save_Presets
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Save_Presets
(
	LPCTSTR	path,
	uint32	class_id,
	bool		class_id_matters,
	bool		temps_only
)
{
	///
	// Delete the file if it already exists
	//
	if (::GetFileAttributes (path) != 0xFFFFFFFF) {
			DWORD attributes = ::GetFileAttributes (path);
		::SetFileAttributes (path, attributes & (~FILE_ATTRIBUTE_READONLY));
		::DeleteFile (path);
	}

	//
	//	Create the file at the specified location
	//
	HANDLE file		= ::CreateFile (	path,
												GENERIC_WRITE,
												0,
												NULL,
												CREATE_ALWAYS,
												0L,
												NULL);

	ASSERT (file != INVALID_HANDLE_VALUE);
	if (file != INVALID_HANDLE_VALUE) {
		
		//
		//	Now write the presets and definitions out to this file
		//
		Save_Presets (file, class_id, class_id_matters, temps_only);
			
	} else {
		
		//
		//	Warn the user
		//
		DWORD last_error = ::GetLastError ();
		CString message;		
		message.Format ("Unable to create database file.\nPath:%s\nError Code:%d", path, last_error);
		::MessageBox (::AfxGetMainWnd ()->m_hWnd, message, "File I/O Error", MB_ICONERROR | MB_OK);
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Save_Presets
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Save_Presets (uint32 class_id, bool temps_only)
{
	//
	//	Build a filename for the presets library
	//
	CString path;
	::Get_File_Mgr ()->Get_Preset_Library_Path (class_id, temps_only, path);

	//
	//	Now save the presets to this file
	//
	return Save_Presets (path, class_id, !temps_only, temps_only);
}


/////////////////////////////////////////////////////////////////////////////
//
// Save_Presets
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Save_Presets (HANDLE file, uint32 class_id, bool class_id_matters, bool temps_only)
{
	//
	//	Create a chunk IO object that we can use to save our subsystems
	//
	RawFileClass file_obj;
	file_obj.Attach (file);
	ChunkSaveClass chunk_save (&file_obj);

	//
	//	Remove all the presets that don't match our criteria
	//
	PRESET_LIST preset_list;
	preset_list.Set_Growth_Step (2000);
	PresetMgrClass::Remove_Non_Matching_Presets (class_id, class_id_matters, temps_only, preset_list);

	//
	//	Turn saving off for all definitions
	//
	DefinitionClass *definition		= NULL;
	for (	definition = DefinitionMgrClass::Get_First ();
			definition != NULL;
			definition = DefinitionMgrClass::Get_Next (definition))
	{
		definition->Enable_Save (false);
	}

	//
	//	Now turn saving on for all definitions that are needed by
	// one of the presets.
	//
	DEFINITION_LIST defs_to_save;
	PresetClass *preset = NULL;
	for (	preset = PresetMgrClass::Get_First ();
			preset != NULL;
			preset = PresetMgrClass::Get_Next (preset))
	{
		//
		//	Build a list of definitions this preset depends on
		//
		DEFINITION_LIST curr_defs;
		preset->Collect_Definitions (curr_defs);

		//
		//	Turn saving on for all these definitions
		//
		for (int index = 0; index < curr_defs.Count (); index ++) {
			DefinitionClass *definition = curr_defs[index];
			if (definition != NULL) {
				definition->Enable_Save (true);
			}
		}
	}

	//
	//	The definition database consists of the definitions (global)
	//	and the presets (editor only).
	//
	SaveLoadSystemClass::Save (chunk_save, _TheDefinitionMgr);
	SaveLoadSystemClass::Save (chunk_save, _ThePresetMgr);

	//
	//	Turn saving back on for all definitions
	//
	for (	definition = DefinitionMgrClass::Get_First ();
			definition != NULL;
			definition = DefinitionMgrClass::Get_Next (definition))
	{
		definition->Enable_Save (true);
	}

	//
	//	Now add back all the presets we removed
	//
	PresetMgrClass::Put_Presets_Back (preset_list);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Reselect_Item
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Reselect_Item (int id, bool is_factory)
{
	HTREEITEM tree_item = NULL;
	if (is_factory) {
		tree_item = Find_Factory (TVI_ROOT, id);
	} else {
		tree_item = Find_Preset (TVI_ROOT, id);
	}

	m_TreeCtrl.Select (tree_item, TVGN_FIRSTVISIBLE);
	m_TreeCtrl.SelectItem (tree_item);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAddTemp
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnAddTemp (void)
{
	DefinitionFactoryClass *factory = Get_Selected_Factory ();
	if (factory != NULL) {

		if (Add_New_Preset (NULL, true) != NULL) {
			Save_Temp_Presets ();
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAdd
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnAdd (void)
{	
	DefinitionFactoryClass *factory = Get_Selected_Factory ();
	if (factory != NULL) {

		//
		//	Check out the definition database
		//
		SelectionCaptureClass sel_capture (this);
		bool undo_check_out = false;
		if (PresetMgrClass::Check_Out_Database (factory->Get_Class_ID (), &undo_check_out)) {
			sel_capture.Restore ();

			//
			//	Create a new preset
			//
			PresetClass *preset = Add_New_Preset (NULL);
			if (preset != NULL) {
				
				//
				//	Log creation
				//
				PresetLoggerClass::Log_Created (preset);

				//
				//	Register this new preset for check-in
				//
				Save_Global_Presets (factory->Get_Class_ID ());
				PresetMgrClass::Add_Dirty_Preset (preset->Get_ID ());				
			} else if (undo_check_out) {
				PresetMgrClass::Undo_Database_Check_Out (factory->Get_Class_ID ());
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnMake
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnMake (void)
{
	HTREEITEM current_item	= m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset		= Get_Item_Preset (current_item);
	if (preset != NULL) {
		::Get_Scene_Editor ()->Create_Node (preset);

		//
		//	Repartition the scene if the user placed a terrain
		//
		if (preset->Get_Class_ID () == CLASSID_TERRAIN) {
			::Get_Scene_Editor ()->Re_Partition_Static_Projectors ();
			::Get_Scene_Editor ()->Re_Partition_Static_Lights ();
			::Get_Scene_Editor ()->Re_Partition_Static_Objects ();			
		}

		::Refresh_Main_View ();
	}

	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_New_Preset
//
/////////////////////////////////////////////////////////////////////////////
PresetClass *
PresetsFormClass::Add_New_Preset (LPCTSTR name, bool is_temp, bool sel_new)
{
	PresetClass *new_preset = NULL;

	DefinitionFactoryClass *factory = Get_Selected_Factory ();
	if (factory != NULL) {

		HTREEITEM current_item		= m_TreeCtrl.GetSelectedItem ();
		PresetClass *parent_preset	= Get_Item_Preset (current_item);

		//
		//	Create a new preset
		//
		new_preset = PresetMgrClass::Create_Preset (factory->Get_Class_ID (), name, is_temp);
		new_preset->Set_Parent (parent_preset);

		//
		//	Give the new preset its parent's values by default
		//
		if (parent_preset != NULL) {		
			new_preset->Copy_Properties (*parent_preset);
		}

		//
		//	Edit the preset's properties
		//
		if (name != NULL || new_preset->Show_Properties ()) {
			
			//
			//	Add this preset to the framework
			//
			HTREEITEM new_item = Insert_Preset (current_item, new_preset);
			PresetMgrClass::Add_Preset (new_preset);
			DefinitionMgrClass::Register_Definition (new_preset->Get_Definition ());

			if (parent_preset != NULL) {
				parent_preset->Add_Child_Preset (new_preset->Get_ID ());
			}

			//
			//	Select the new entry
			//
			if (sel_new) {
				m_TreeCtrl.Select (new_item, TVGN_FIRSTVISIBLE);
				m_TreeCtrl.SelectItem (new_item);				
			}
			m_TreeCtrl.SortChildren (current_item);
			m_TreeCtrl.InvalidateRect (NULL, true);

		} else {			
			DefinitionClass *definition = new_preset->Get_Definition ();
			SAFE_DELETE (definition);
			SAFE_DELETE (new_preset);
		}
	}

	return new_preset;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDelete
//
/////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnDelete (void)
{
	HTREEITEM current_item	= m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset		= Get_Item_Preset (current_item);
	if (preset != NULL) {

		//
		//	Make sure the user really wants to do this
		//
		if (::Message_Box (	m_hWnd,
									IDS_DELETE_PRESET_MSG,
									IDS_DELETE_PRESET_TITLE,
									MB_ICONQUESTION | MB_YESNO) == IDYES	)
		{
			CWaitCursor wait_cursor;

			//
			//	Check out the definition database (unless its a temp)
			//
			SelectionCaptureClass sel_capture (this);
			bool undo_on_err = false;
			bool was_temp = preset->Get_IsTemporary ();;
			if (was_temp || PresetMgrClass::Check_Out_Database (preset->Get_Class_ID (), &undo_on_err)) {
				sel_capture.Restore ();

				//
				//	Re-get the preset (its pointer has changed, the
				// old one is crap).
				//
				current_item	= m_TreeCtrl.GetSelectedItem ();
				preset			= Get_Item_Preset (current_item);
				if (preset != NULL) {

					//
					//	Log the delete operation
					//
					if (preset->Get_IsTemporary () == false) {
						PresetLoggerClass::Log_Deleted (preset->Get_Name ());
					}

					//
					//	Build a list of all the sub-items that are affected by this operation
					//
					DynamicVectorClass<HTREEITEM> tree_item_list;
					tree_item_list.Add (current_item);
					Build_Child_List (current_item, tree_item_list);

					//
					//	Determine what class-id's these presets belong to (they all belong
					// to the same class)
					//
					uint32 class_id = 0;
					DefinitionClass *definition = preset->Get_Definition ();
					if (definition != NULL) {
						class_id = definition->Get_Class_ID ();
					}
					
					//
					//	Free all the nodes in the scene that use any of these presets.
					// Also unregister the preset and its definition
					//
					bool save_temp_lib = was_temp;
					for (int index = 0; index < tree_item_list.Count (); index ++) {
						PresetClass *preset = Get_Item_Preset (tree_item_list[index]);
						if (preset != NULL) {
							::Get_Scene_Editor ()->Delete_Nodes (preset);
							PresetMgrClass::Remove_Preset (preset);													
							
							//
							//	Determine if we need to save the temporary preset
							// library later or not.
							//
							if (preset->Get_IsTemporary ()) {
								save_temp_lib = true;
							}

							//
							//	Unlink the definition
							//
							preset->Set_Definition (NULL);

							//
							//	Free the definition
							//
							DefinitionClass *definition = preset->Get_Definition ();
							if (definition != NULL) {
								DefinitionMgrClass::Unregister_Definition (definition);
								preset->Set_Definition (NULL);
								SAFE_DELETE (definition);
							}
							
							//
							//	Free the preset
							//
							SAFE_DELETE (preset);
						}
					}

					//
					//	Save the temporary presets library if necessary.
					//
					if (save_temp_lib) {
						Save_Temp_Presets ();
					}

					//
					//	Save the global presets library if necessary
					//
					if (was_temp == false) {
						Save_Global_Presets (class_id);

						if (PresetMgrClass::Get_Immediate_Check_In_Mode ()) {
							PresetMgrClass::Check_In_Presets ();
						} else {							
							PresetMgrClass::Set_Presets_Dirty ();
						}
					}

					m_TreeCtrl.DeleteItem (current_item);
					::Refresh_Main_View ();
				}
			}
		}
	}

	return ;	
}


////////////////////////////////////////////////////////////////////////////
//
//  Sort_Nodes
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Sort_Nodes (HTREEITEM root_item, bool recursive)
{
	for (	HTREEITEM tree_item = m_TreeCtrl.GetChildItem (root_item);
			tree_item != NULL;
			tree_item = m_TreeCtrl.GetNextSiblingItem (tree_item))
	{
		//
		//	Sort the children
		//
		m_TreeCtrl.SortChildren (tree_item);

		//
		//	Recurse through the hierarchy
		//
		if (recursive && m_TreeCtrl.ItemHasChildren (tree_item)) {
			Sort_Nodes (tree_item);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Create_Preset
//
////////////////////////////////////////////////////////////////////////////
PresetClass *
PresetsFormClass::Create_Preset (DefinitionFactoryClass *factory, bool is_temp)
{
	//
	//	Create a new definition for the preset
	//
	DefinitionClass *definition = factory->Create ();

	//
	//	Assign the definition a new ID
	//
	if (is_temp) {		
		definition->Set_ID (::Get_Next_Temp_ID ());
	} else {
		definition->Set_ID (DefinitionMgrClass::Get_New_ID (definition->Get_Class_ID ()));
	}
	
	//
	//	Create the new preset
	//
	PresetClass *new_preset = new PresetClass;
	new_preset->Set_Definition (definition);	
	new_preset->Set_IsTemporary (is_temp);
	return new_preset;
}


////////////////////////////////////////////////////////////////////////////
//
//  Build_Child_List
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Build_Child_List
(
	HTREEITEM								parent_item,
	DynamicVectorClass<HTREEITEM> &	list
)
{
	//
	//	Loop over all the immediate children
	//
	for (	HTREEITEM child_item = m_TreeCtrl.GetChildItem (parent_item);
			child_item != NULL;
			child_item = m_TreeCtrl.GetNextSiblingItem (child_item))
	{
		list.Add (child_item);
		if (m_TreeCtrl.ItemHasChildren (child_item)) {
			
			//
			// Recursively call this method for all sub-folders in this tree
			//
			Build_Child_List (child_item, list);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Enable_Button
//
////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Enable_Button (int cmd_id)
{
	bool retval = false;

	HTREEITEM current_item					= m_TreeCtrl.GetSelectedItem ();
	if (current_item != NULL) {
		PresetClass *preset					= Get_Item_Preset (current_item);
		DefinitionFactoryClass *factory	= Get_Item_Factory (current_item);
		bool read_only							= ::Get_File_Mgr()->Is_VSS_Read_Only ();
		bool temp								= (preset != NULL && preset->Get_IsTemporary ()) ? true : false;
		bool is_proxy_test_folder			= false;

		//
		//	Determine if this is the proxy-test folder or not (special rules
		// apply to this folder).
		//
		if (preset != NULL) {
			is_proxy_test_folder = preset->Is_A_Parent (PROXY_TESTS_FOLDER);
		}

		//
		//	Change the read_only state depending on the user class
		//
		if (read_only == false && ::Get_File_Mgr ()->Is_Special_User ()) {
			read_only = true;
			if (preset != NULL) {															
				if (preset->Is_A_Parent (SPECIAL_USER_FOLDER)) {
					read_only = false;

					//
					//	Don't let special users modify the folder itself (just sub-folders)
					//
					if (	::lstrcmpi (preset->Get_Name (), SPECIAL_USER_FOLDER) == 0 &&
							(cmd_id == IDC_MODIFY || cmd_id == IDC_DELETE))
					{
						read_only = true;
					}
				}
			}
		}

		//
		//	What operation is this?
		//
		switch (cmd_id)
		{
			case IDC_ADD:
				retval = ((preset || factory) && !read_only && !temp && !is_proxy_test_folder);
				break;

			case IDC_ADD_TEMP:
				retval = (preset || factory);
				break;

			case IDC_MAKE:
				retval = (preset != NULL);
				break;

			case IDC_MODIFY:
				retval = (preset && (!read_only || temp));
				break;

			case IDC_DELETE:
				retval = (preset && (!read_only || temp));
				break;

			case IDC_PLAY:
				retval = (preset != NULL && preset->Is_Valid_Sound_Preset ()) ? true : false;
				break;

			case IDC_UPDATE_VSS:
				retval = ((preset != NULL) && !read_only);
				break;				

			case IDC_INFO:
				retval = (preset != NULL);
				break;

			case IDC_CONVERT:
				retval = ((preset != NULL) && temp && !read_only && !is_proxy_test_folder);
				if (	preset != NULL &&
						preset->Get_Parent () != NULL &&
						preset->Get_Parent ()->Get_IsTemporary ())
				{
					retval = false;
				}
				break;

			case IDM_BATCH_IMPORT_TERRAIN:
			{
				if (preset != NULL && preset->Get_Class_ID () == CLASSID_TERRAIN) {
					retval = true;
				}
			}
			break;

			case IDM_CLEAR_EMBEDDED_NODE_LIST:
			{
				retval = (preset && (!read_only || temp));

				if (preset != NULL && preset->Get_Node_List ().Count () == 0) {
					retval = false;
				}
			}
			break;

			case IDM_UPDATE_EMBEDDED_NODE_LIST:
				retval = (preset && (!read_only || temp));

				//
				//	Only allow this option for terrain and tile presets
				//
				if (preset != NULL) {
					if (	preset->Get_Class_ID () != CLASSID_TERRAIN &&
							preset->Get_Class_ID () != CLASSID_TILE)
					{
						retval = false;
					}
				}

				break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnSelchangedPresetsTree
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnSelchangedPresetsTree (NMHDR * /* pNMHDR */, LRESULT *pResult)
{
	(*pResult) = 0;

	//
	//	Enable/disable the toolbar buttons
	//
	m_Toolbar.Enable_Button (IDC_ADD,		Enable_Button (IDC_ADD));
	m_Toolbar.Enable_Button (IDC_ADD_TEMP,	Enable_Button (IDC_ADD_TEMP));
	m_Toolbar.Enable_Button (IDC_MAKE,		Enable_Button (IDC_MAKE));
	m_Toolbar.Enable_Button (IDC_MODIFY,	Enable_Button (IDC_MODIFY));
	m_Toolbar.Enable_Button (IDC_DELETE,	Enable_Button (IDC_DELETE));
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Propagate_Changes
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Propagate_Changes
(
	HTREEITEM				modified_item,
	DERIVED_PARAM_LIST &	list
)
{
	CWaitCursor wait_cursor;
	CLevelEditView::Allow_Repaint (false);

	PresetClass *preset				= Get_Item_Preset (modified_item);

	//
	//	Loop over the list of derived parameters that need to
	// be updated with the new values.
	//
	for (int index = 0; index < list.Count (); index ++) {
		DERIVED_PARAM_INFO &info			= list[index];

		//
		//	Copy all the inherited parameter values from the parent preset
		//
		int list_count = info.base_param_list.Count ();
		for (int list_index = 0; list_index < list_count; list_index ++) {
			//int param_index						= info.inherited_param_list[list_index];

			ParameterClass *parameter			= info.base_param_list[list_index];
			ParameterClass *curr_parameter	= info.derived_param_list[list_index];
			ASSERT (parameter != NULL);
			ASSERT (curr_parameter != NULL);

			//
			//	Copy the parameter value from the parent.
			//
			curr_parameter->Copy_Value (*parameter);

			//
			//	Release the parameter pointers
			//
			//definition->Unlock_Parameter (param_index);
			//curr_definition->Unlock_Parameter (param_index);
		}

		//
		//	Now reload any nodes in the scene that refer to the
		// changed preset.
		//
		if (info.preset != NULL) {
			NodeMgrClass::Reload_Nodes (info.preset);
		}
	}

	//
	//	Now reload any nodes in the scene that refer to the
	// changed preset.
	//
	NodeMgrClass::Reload_Nodes (preset);
	CLevelEditView::Allow_Repaint (true);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Compare_Derived_Parameters
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Compare_Derived_Parameters
(
	PresetClass *			curr_preset,
	DefinitionClass *		base_def,
	DefinitionClass *		derived_def,
	DERIVED_PARAM_LIST &	list
)
{
	DERIVED_PARAM_INFO info;
	info.preset = curr_preset;
		
	//
	//	Compare all the parameters with this derived definition to
	// find any that are not overridden.
	//
	int param_count = base_def->Get_Parameter_Count ();
	for (int param_index = 0; param_index < param_count; param_index ++) {
		ParameterClass *parameter			= base_def->Lock_Parameter (param_index);
		ParameterClass *curr_parameter	= derived_def->Lock_Parameter (param_index);
		ASSERT (parameter != NULL);
		ASSERT (curr_parameter != NULL);

		//
		//	If the parameter values are exactly the same, then
		// we will assume this parameter is derived and not overridden
		//
		// OpenW3D @fix OmniBlade 05/08/2005
		// This cast is apparently needed to avoid some new C++20 overload resolution issues.
		if (*curr_parameter == static_cast<const ParameterClass &>(*parameter)) {
			info.base_param_list.Add (parameter);
			info.derived_param_list.Add (curr_parameter);
		} else if (parameter->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
			
			//
			//	Lookup the definitions that these parameters reference
			//
			int base_id		= ((ModelDefParameterClass *)parameter)->Get_Value ();
			int derived_id = ((ModelDefParameterClass *)curr_parameter)->Get_Value ();
			DefinitionClass *base_model_def		= DefinitionMgrClass::Find_Definition (base_id, false);
			DefinitionClass *derived_model_def	= DefinitionMgrClass::Find_Definition (derived_id, false);

			//
			//	Recursively add parameters from these definitions to the list
			//
			if (	base_model_def != NULL && derived_model_def != NULL &&
					base_model_def->Get_Class_ID () == derived_model_def->Get_Class_ID ())
			{
				Compare_Derived_Parameters (NULL, base_model_def, derived_model_def, list);
			}
		}

		//
		//	Release the parameter pointers
		//
		base_def->Unlock_Parameter (param_index);
		derived_def->Unlock_Parameter (param_index);
	}

	//
	//	If this preset contains at least one derived (not overridden) value,
	// then add it to the list.
	//
	if (info.base_param_list.Count () > 0) {
		list.Add (info);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Build_Inherited_Param_List
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Build_Inherited_Param_List
(
	HTREEITEM				parent_item,
	DERIVED_PARAM_LIST &	list
)
{
	PresetClass *preset				= Get_Item_Preset (parent_item);
	DefinitionClass *definition	= preset->Get_Definition ();

	//
	//	Build a list of children (derived presets)
	//
	DynamicVectorClass<HTREEITEM> child_list;	
	Build_Child_List (parent_item, child_list);

	//
	//	Loop over all the derived presets and build up the list
	// of non-overridden parameter values
	//
	for (int index = 0; index < child_list.Count (); index ++) {
		HTREEITEM tree_item = child_list[index];

		PresetClass *curr_preset			= Get_Item_Preset (tree_item);
		DefinitionClass *curr_definition	= curr_preset->Get_Definition ();
		ASSERT (curr_definition->Get_Class_ID () == definition->Get_Class_ID ());

		//
		//	Build the list of parameters that are the same as the base
		//
		Compare_Derived_Parameters (curr_preset, definition, curr_definition, list);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnPlay
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnPlay (void)
{
	HTREEITEM tree_item = m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset = Get_Item_Preset (tree_item);
	if (preset != NULL && preset->Get_Definition () != NULL) {

		AudibleSoundDefinitionClass *definition = NULL;
		definition = (AudibleSoundDefinitionClass *)preset->Get_Definition ();

		//
		//	Make sure we have a local copy of this sound
		//
		CString filename = static_cast<const char *>(definition->Get_Filename ());
		if (::Get_File_Mgr ()->Does_File_Exist (filename, true)) {
		
			//
			//	Pass the sound filename onto the dialog
			//
			PlaySoundDialogClass dialog (filename, this);
			dialog.DoModal ();
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnInfo
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnInfo (void)
{
	HTREEITEM current_item	= m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset		= Get_Item_Preset (current_item);
	if (preset != NULL) {

		//
		//	Show the (read-only) properties dialog for this preset
		//
		preset->Show_Properties (true);
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnExtra
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnExtra (void) 
{
	//
	//	Determine the screen coords of the toolbar button
	//
	CRect rect;
	m_Toolbar.GetItemRect (m_Toolbar.CommandToIndex (IDC_EXTRA), &rect);
	m_Toolbar.ClientToScreen (&rect);

	//
	//	Load the menu we want to display
	//
	HMENU main_menu = ::LoadMenu (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDR_XTRA_POPUP));
	HMENU menu = ::GetSubMenu (main_menu, 0);

	//
	//	Make the menu commands owner-draw so we can paint icons next to them
	//
	::ModifyMenu (menu, IDC_PLAY, MF_BYCOMMAND | MF_OWNERDRAW, IDC_PLAY, NULL);
	::ModifyMenu (menu, IDC_CONVERT, MF_BYCOMMAND | MF_OWNERDRAW, IDC_CONVERT, NULL);
	::ModifyMenu (menu, IDC_UPDATE_VSS, MF_BYCOMMAND | MF_OWNERDRAW, IDC_UPDATE_VSS, NULL);
	::ModifyMenu (menu, IDM_BATCH_IMPORT_TERRAIN, MF_BYCOMMAND | MF_OWNERDRAW, IDM_BATCH_IMPORT_TERRAIN, NULL);
	::ModifyMenu (menu, IDM_UPDATE_EMBEDDED_NODE_LIST, MF_BYCOMMAND | MF_OWNERDRAW, IDM_UPDATE_EMBEDDED_NODE_LIST, NULL);	
	::ModifyMenu (menu, IDM_CLEAR_EMBEDDED_NODE_LIST, MF_BYCOMMAND | MF_OWNERDRAW, IDM_CLEAR_EMBEDDED_NODE_LIST, NULL);		
	
	//
	//	Disable any buttons if necessary
	//
	if (Enable_Button (IDC_PLAY) == false) {
		::EnableMenuItem (menu, IDC_PLAY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if (Enable_Button (IDC_CONVERT) == false) {
		::EnableMenuItem (menu, IDC_CONVERT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if (Enable_Button (IDC_UPDATE_VSS) == false) {
		::EnableMenuItem (menu, IDC_UPDATE_VSS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if (Enable_Button (IDM_BATCH_IMPORT_TERRAIN) == false) {
		::EnableMenuItem (menu, IDM_BATCH_IMPORT_TERRAIN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if (Enable_Button (IDM_UPDATE_EMBEDDED_NODE_LIST) == false) {
		::EnableMenuItem (menu, IDM_UPDATE_EMBEDDED_NODE_LIST, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if (Enable_Button (IDM_CLEAR_EMBEDDED_NODE_LIST) == false) {
		::EnableMenuItem (menu, IDM_CLEAR_EMBEDDED_NODE_LIST, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}	

	//
	//	Display the menu
	//
	::TrackPopupMenu (menu, TPM_LEFTALIGN | TPM_TOPALIGN, rect.left, rect.bottom, 0, m_hWnd, NULL);	

	//
	//	Cleanup
	//
	::DestroyMenu (main_menu);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnNotify
//
////////////////////////////////////////////////////////////////////////////
BOOL
PresetsFormClass::OnNotify
(
	WPARAM wParam,
	LPARAM lParam,
	LRESULT* pResult
) 
{
	if (((NMHDR *)lParam)->code == TBN_DROPDOWN) {

		OnExtra ();
		(*pResult) = TBDDRET_DEFAULT;
		return true;
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}


////////////////////////////////////////////////////////////////////////////
//
//  OnUpdateConvert
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnUpdateConvert (CCmdUI *pCmdUI) 
{
	pCmdUI->Enable (Enable_Button (IDC_CONVERT));
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnUpdateInfo
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnUpdateInfo (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (Enable_Button (IDC_INFO));
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnUpdatePlay
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnUpdatePlay (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (Enable_Button (IDC_PLAY));
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnUpdateUpdateVss
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnUpdateUpdateVss (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (Enable_Button (IDC_UPDATE_VSS));
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Menu_Text
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Get_Menu_Text (int menu_id, CString &text)
{
	switch (menu_id)
	{
		case IDC_PLAY:
			text = "Play...";
			break;

		case IDC_CONVERT:
			text = "Convert Temp...";
			break;

		case IDC_UPDATE_VSS:
			text = "Update VSS...";
			break;

		case IDM_BATCH_IMPORT_TERRAIN:
			text = "Batch Import Terrain...";
			break;
					
		case IDM_CLEAR_EMBEDDED_NODE_LIST:
			text = "Clear Embedded Node List";
			break;

		case IDM_UPDATE_EMBEDDED_NODE_LIST:
			text = "Embed Nodes...";
			break;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDrawItem
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnDrawItem
(
	int					nIDCtl,
	LPDRAWITEMSTRUCT	lpDrawItemStruct
) 
{
	HICON icon = NULL;

	CString text;
	Get_Menu_Text (lpDrawItemStruct->itemID, text);

	switch (lpDrawItemStruct->itemID)
	{
		case IDC_PLAY:
			icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_PLAY), IMAGE_ICON, 16, 16, LR_SHARED);
			break;

		case IDC_CONVERT:
			icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_CONVERT), IMAGE_ICON, 16, 16, LR_SHARED);
			break;

		case IDC_UPDATE_VSS:
			icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_UPDATE), IMAGE_ICON, 16, 16, LR_SHARED);
			break;

		case IDM_BATCH_IMPORT_TERRAIN:
			icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_TERRAIN), IMAGE_ICON, 16, 16, LR_SHARED);
			break;

		case IDM_UPDATE_EMBEDDED_NODE_LIST:
			icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_OBJECT), IMAGE_ICON, 16, 16, LR_SHARED);
			break;

		case IDM_CLEAR_EMBEDDED_NODE_LIST:
			icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_DELETE), IMAGE_ICON, 16, 16, LR_SHARED);
			break;			
	}

	CRect rect = lpDrawItemStruct->rcItem;

	int dss_state = DSS_NORMAL;
	DWORD_PTR bk_color = 0;
		
	//
	//	Deteremine what background and text color to use
	//
	if (lpDrawItemStruct->itemState & ODS_GRAYED) {
		bk_color = COLOR_MENU;
		::SetBkColor (lpDrawItemStruct->hDC, ::GetSysColor (COLOR_MENU));
		::SetTextColor (lpDrawItemStruct->hDC, ::GetSysColor (COLOR_GRAYTEXT));
		dss_state = DSS_DISABLED;
	} else if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		bk_color = COLOR_HIGHLIGHT;
		::SetBkColor (lpDrawItemStruct->hDC, ::GetSysColor (COLOR_HIGHLIGHT));
		::SetTextColor (lpDrawItemStruct->hDC, ::GetSysColor (COLOR_HIGHLIGHTTEXT));
	} else {
		bk_color = COLOR_MENU;
		::SetBkColor (lpDrawItemStruct->hDC, ::GetSysColor (COLOR_MENU));
		::SetTextColor (lpDrawItemStruct->hDC, ::GetSysColor (COLOR_MENUTEXT));
	}

	//
	//	Determine the height of the text
	//
	CSize text_size;
	::GetTextExtentPoint32 (lpDrawItemStruct->hDC, text, text.GetLength (), &text_size);
	
	//
	//	Render the text
	//
	rect.top += 2;
	rect.bottom -= 2;
	::FillRect (lpDrawItemStruct->hDC, &rect, (HBRUSH) (bk_color+1));

	::SetBkMode (lpDrawItemStruct->hDC, TRANSPARENT);
	::DrawState (	lpDrawItemStruct->hDC,
						NULL,
						NULL,
						(LPARAM)(LPCTSTR)text,
						0,
						rect.left + 24,
						rect.top + (rect.Height () >> 1) - (text_size.cy >> 1),
						0, 0, DST_TEXT | dss_state);

	//
	//	Draw the associated icon (if necessary)
	//
	if (icon != NULL) {
		::DrawState ( lpDrawItemStruct->hDC, NULL, NULL, (LPARAM)icon, 0, rect.left + 4, rect.top + 4, 16, 16, DST_ICON | dss_state);
	}	

	
	CDialog::OnDrawItem (nIDCtl, lpDrawItemStruct);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnMeasureItem
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnMeasureItem
(
	int						/* nIDCtl */,
	LPMEASUREITEMSTRUCT	lpMeasureItemStruct
) 
{
	CString text;
	Get_Menu_Text (lpMeasureItemStruct->itemID, text);

	NONCLIENTMETRICS metrics = { sizeof (NONCLIENTMETRICS), 0 };
	::SystemParametersInfo (SPI_GETNONCLIENTMETRICS, 0, &metrics, false);
	
	HDC mem_dc = ::CreateCompatibleDC (NULL);	
	HFONT font = ::CreateFontIndirect (&metrics.lfMenuFont);
	HFONT old_font = (HFONT)::SelectObject (mem_dc, font);

	CSize text_size;
	::GetTextExtentPoint32 (mem_dc, text, text.GetLength (), &text_size);
	
	::SelectObject (mem_dc, old_font);
	::DeleteObject (font);
	::DeleteDC (mem_dc);

	lpMeasureItemStruct->itemWidth = text_size.cx + 24;
	lpMeasureItemStruct->itemHeight = 24;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnConvert
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnConvert (void)
{
	HTREEITEM item = m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset = Get_Item_Preset (item);
	if (preset != NULL) {

		//
		//	Make sure the user really wants to do this...
		//
		CString message;
		message.Format ("Are you sure you wish to convert '%s' from a temporary preset to a shared preset?", preset->Get_Name ());
		if (MessageBox (message, "Preset Convert", MB_ICONQUESTION | MB_YESNO) == IDYES) {
			CWaitCursor wait_cursor;

			//
			//	Check out the definition database
			//
			SelectionCaptureClass sel_capture (this);
			bool undo_check_out	= false;
			uint32 class_id		= preset->Get_Class_ID ();
			if (PresetMgrClass::Check_Out_Database (class_id, &undo_check_out)) {
				sel_capture.Restore ();

				item = m_TreeCtrl.GetSelectedItem ();
				PresetClass *preset = Get_Item_Preset (item);
				if (preset != NULL) {

					//
					//	Change the preset's ID from temp-range to normal-range
					//
					preset->Set_IsTemporary (false);
					preset->Set_ID (DefinitionMgrClass::Get_New_ID (preset->Get_Class_ID ()));

					//
					//	Now change the IDs of any embedded definition objects
					//
					Fix_Embedded_Definition_IDs (preset->Get_Definition ());

					//
					//	Now save-out the databases (they have both changed)
					//
					Save_Global_Presets (class_id);
					PresetMgrClass::Add_Dirty_Preset (preset->Get_ID ());					
					Save_Temp_Presets ();

					//
					//	Finally change the preset's icon
					//
					int icon = Get_Icon (preset);
					m_TreeCtrl.SetItemImage (item, icon, icon);

					//
					//	Force a re-paint
					//
					m_TreeCtrl.SelectItem (item);
					m_TreeCtrl.InvalidateRect (NULL, true);
				} else if (undo_check_out) {
					MessageBox ("Unable to find preset.", "Preset Error", MB_ICONERROR | MB_OK);
					PresetMgrClass::Undo_Database_Check_Out (class_id);
				}
			}
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnUpdateVss
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnUpdateVss (void)
{
	HTREEITEM item = m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset = Get_Item_Preset (item);
	if (preset != NULL) {

		//
		//	Show the update-preset dialog
		//
		UpdatePresetDialogClass dialog (preset, this);
		dialog.DoModal ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnClearEmbeddedNodeList
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnClearEmbeddedNodeList (void)
{
	CWaitCursor wait_cursor;

	//
	//	Get the currently selected preset
	//
	HTREEITEM current_item	= m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset		= Get_Item_Preset (current_item);
	if (preset != NULL) {

		if (preset->Get_IsTemporary () == false) {

			//
			//	Check out the definition database
			//
			SelectionCaptureClass sel_capture (this);
			bool undo_on_err	= false;
			uint32 class_id	= preset->Get_Class_ID ();
			if (PresetMgrClass::Check_Out_Database (class_id, &undo_on_err)) {
				sel_capture.Restore ();

				//
				//	Re-get the preset (its pointer has changed, the
				// old one is crap).
				//
				current_item	= m_TreeCtrl.GetSelectedItem ();
				preset			= Get_Item_Preset (current_item);
				
				//
				//	Now, free the embedded nodes
				//
				if (preset != NULL) {
					preset->Free_Node_List ();
					Save_Global_Presets (preset->Get_Class_ID ());
					PresetMgrClass::Add_Dirty_Preset (preset->Get_ID ());
				}
			}

		} else {
			
			//
			//	Free the embedded nodes
			//
			preset->Free_Node_List ();
			Save_Temp_Presets ();				
		}

		Reset_Embedded_Node_Instances (preset);
		NodeMgrClass::Reload_Nodes (preset);

		if (preset != NULL) {

			//
			//	Update the overlay icon on the preset
			//
			bool needs_overlay = (preset->Get_Node_List ().Count () > 0);
			TreeView_SetOverlay (m_TreeCtrl, current_item, needs_overlay);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_Embedded_Node_Instances
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Reset_Embedded_Node_Instances (PresetClass *preset)
{
	if (preset == NULL) {
		return ;
	}

	//
	//	Loop over all the nodes in the scene
	//
	for (	NodeClass *node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		//
		//	If this was an embedded node, then convert it to
		// a "normal" node.
		//
		if (node->Get_Container_Preset_ID () == preset->Get_ID ()) {
			node->Set_Is_Proxied (false);
			node->Set_Needs_Save (true);
			node->Set_Container_Preset_ID (0);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnBuildEmbedNodeList
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnBuildEmbedNodeList (void)
{
	//
	//	Prompt the user
	//
	if (::MessageBox (m_hWnd,
				"This operation will embed a copy of all the nodes in scene into this preset.  Are you sure you wish to continue?",
				"Embed Nodes", MB_ICONQUESTION | MB_YESNO) == IDNO)
	{
		return ;
	}

	CWaitCursor wait_cursor;

	//
	//	Get the currently selected preset
	//
	HTREEITEM current_item	= m_TreeCtrl.GetSelectedItem ();
	PresetClass *preset		= Get_Item_Preset (current_item);
	if (preset != NULL) {

		if (preset->Get_IsTemporary () == false) {

			//
			//	Check out the definition database
			//
			SelectionCaptureClass sel_capture (this);
			bool undo_on_err	= false;
			uint32 class_id	= preset->Get_Class_ID ();
			if (PresetMgrClass::Check_Out_Database (class_id, &undo_on_err)) {
				sel_capture.Restore ();

				//
				//	Re-get the preset (its pointer has changed, the
				// old one is crap).
				//
				current_item	= m_TreeCtrl.GetSelectedItem ();
				preset			= Get_Item_Preset (current_item);
				
				//
				//	Now, update the embedded node information for this preset
				//
				if (preset != NULL) {
					Update_Embedded_Nodes (preset);
					Save_Global_Presets (preset->Get_Class_ID ());
					PresetMgrClass::Add_Dirty_Preset (preset->Get_ID ());						
				}
			}

		} else {
			
			//
			//	Update the embedded node information for this preset
			//
			Update_Embedded_Nodes (preset);
			Save_Temp_Presets ();				
		}

		NodeMgrClass::Reload_Nodes (preset);

		//
		//	Update the overlay icon on the preset
		//
		if (preset != NULL) {
			bool needs_overlay = (preset->Get_Node_List ().Count () > 0);
			TreeView_SetOverlay (m_TreeCtrl, current_item, needs_overlay);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Embedded_Nodes
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Update_Embedded_Nodes (PresetClass *preset)
{
	Reset_Embedded_Node_Instances (preset);

	//
	//	Loop over all the nodes in the scene
	//
	NodeClass *node;
	for (	node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		//
		//	Just find the first node that came from this preset
		//
		if (node->Get_Preset () == preset) {
			break;
		}
	}

	//
	//	Generate the node list
	//
	if (node != NULL) {
		preset->Build_Node_List (node);
	}
		
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnBatchImportTerrain
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnBatchImportTerrain (void)
{
	CFileDialog dialog (	true,
								".w3d",
								NULL,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT | OFN_EXPLORER,
								"Westwood 3D Files (*.w3d)|*.w3d||",
								this);

	TCHAR filename_list[MAX_PATH*20] = { 0 };
	dialog.m_ofn.lpstrFile = filename_list;
	dialog.m_ofn.nMaxFile = sizeof (filename_list);

	//
	// Ask the user what files they wish to import
	//
	if (dialog.DoModal () == IDOK) {

      //
		// Loop through all the selected files
		//
		POSITION pos = dialog.GetStartPosition (); 
		while (pos != NULL) {

			//
			//	Add this preset to the tree
			//
			Add_Terrain_Preset (dialog.GetNextPathName (pos));
      }

		//
		//	Save the temp presets lib
		//
		Save_Temp_Presets ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnUpdateClearEmbeddedNodeList
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnUpdateClearEmbeddedNodeList (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (Enable_Button (IDM_CLEAR_EMBEDDED_NODE_LIST));
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnUpdateEmbeddedNodeList
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnUpdateEmbeddedNodeList (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (Enable_Button (IDM_UPDATE_EMBEDDED_NODE_LIST));
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnUpdateBatchImportTerrain
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnUpdateBatchImportTerrain (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (Enable_Button (IDM_BATCH_IMPORT_TERRAIN));
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Add_Terrain_Preset
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Add_Terrain_Preset (LPCTSTR filename)
{
	if (::Is_W3D_Filename (filename)) {

		//
		//	Make an asset name from the filename
		//
		CString preset_name = ::Asset_Name_From_Filename (filename);
		if (preset_name.GetLength () > 0) {
			
			//
			//	Add the new preset
			//
			PresetClass *preset = Add_New_Preset (preset_name, true, false);
			if (preset != NULL && preset->Get_Class_ID () == CLASSID_TERRAIN) {
				
				//
				//	Configure the preset
				//
				TerrainDefinitionClass *definition = (TerrainDefinitionClass *)preset->Get_Definition ();
				definition->Set_Model_Name (::Get_File_Mgr ()->Make_Relative_Path (filename));
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnBegindragPresetsTree
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnBegindragPresetsTree
(
	NMHDR *	pNMHDR,
	LRESULT *pResult
)
{
	NM_TREEVIEW *tv_info = (NM_TREEVIEW *)pNMHDR;
	(*pResult) = 0;

	if (Is_Drag_OK (tv_info->itemNew.hItem)) {
		
		//
		//	Set-up for our drag operation
		//
		m_DragItem = tv_info->itemNew.hItem;
		if (m_DragItem != NULL) {
			
			m_DragImageList = m_TreeCtrl.CreateDragImage (m_DragItem);
			m_DragImageList->BeginDrag (0, CPoint (-20, -20));			
			m_IsDragging = true;
			SetCapture ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnLButtonUp
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnLButtonUp (UINT nFlags, CPoint point) 
{
	if (m_IsDragging) {

		//
		//	Convert the tab mouse-position to the tree's mouse-position
		//
		CPoint tree_pt = point;
		ClientToScreen (&tree_pt);
		m_TreeCtrl.ScreenToClient (&tree_pt);
				
		//
		//	Reset the state of the tree
		//
		CImageList::EndDrag ();
		ReleaseCapture ();
		m_TreeCtrl.SelectDropTarget (NULL);
		m_TreeCtrl.SelectItem (m_DragItem);		

		//
		//	Check to ensure its OK to move the preset
		//
		UINT flags					= TVHT_ONITEM;
		HTREEITEM drop_target	= m_TreeCtrl.HitTest (tree_pt, &flags);
		if (Is_Drop_OK (drop_target)) {
			
			//
			//	Perform the move operation
			//
			m_TreeCtrl.SelectItem (drop_target);
			Do_Drop (m_DragItem, drop_target);
		}

		m_DragItem		= NULL;
		m_IsDragging	= false;
	}

	CDialog::OnLButtonUp (nFlags, point);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnMouseMove
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnMouseMove (UINT nFlags, CPoint point) 
{
	if (m_IsDragging) {
		
		//
		//	Convert the tab mouse-position to the tree's mouse-position
		//
		CPoint tree_pt = point;
		ClientToScreen (&tree_pt);
		m_TreeCtrl.ScreenToClient (&tree_pt);
	
		//
		//	Draw the outline of the drag operation
		//
		CImageList::DragMove (tree_pt);
		m_DragImageList->DragLeave (&m_TreeCtrl);

		//
		//	Determine if we should hilight one of the entries in the tree control
		//
		UINT flags					= TVHT_ONITEM;
		HTREEITEM drop_target	= m_TreeCtrl.HitTest (tree_pt, &flags);
		if (drop_target != NULL) {
			m_TreeCtrl.SelectDropTarget (drop_target);
		}

		m_DragImageList->DragEnter (&m_TreeCtrl, tree_pt);

		//
		//	Choose a cursor
		//
		if (drop_target == NULL || Is_Drop_OK (drop_target) == false) {
			SetCursor (::LoadCursor (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDC_NODROP)));
		} else {
			SetCursor (::LoadCursor (NULL, IDC_ARROW));
		}
	}
		
	CDialog::OnMouseMove (nFlags, point);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Is_Drag_OK
//
////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Is_Drag_OK (HTREEITEM drag_item)
{
	bool retval = false;

	PresetClass *preset	= Get_Item_Preset (drag_item);
	bool read_only			= ::Get_File_Mgr()->Is_VSS_Read_Only ();
	bool temp				= (preset != NULL && preset->Get_IsTemporary ()) ? true : false;

	//
	//	We allow the user to drag a preset if its a temp or they have write
	// access to the database.
	//
	if (preset != NULL && (read_only == false || temp == true)) {
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  Is_Drop_OK
//
////////////////////////////////////////////////////////////////////////////
bool
PresetsFormClass::Is_Drop_OK (HTREEITEM drop_item)
{
	bool retval = false;

	if (m_DragItem != NULL && drop_item != NULL) {
		
		PresetClass *drag_preset			= Get_Item_Preset (m_DragItem);
		PresetClass *drop_preset			= Get_Item_Preset (drop_item);
		DefinitionFactoryClass *factory	= Get_Item_Factory (drop_item);
		if (drag_preset != NULL && (drop_preset != NULL || factory != NULL)) {

			//
			//	Check to make sure we don't try to move a preset to one of its children
			//
			if (drop_preset == NULL || drop_preset->Is_A_Parent (drag_preset) == false) {

				//
				//	Get information about what 'type' of presets these are
				//
				int drag_classid = drag_preset->Get_Class_ID ();
				int drop_classid = 0;
				if (drop_preset != NULL) {
					drop_classid = drop_preset->Get_Class_ID ();
				} else {
					drop_classid = factory->Get_Class_ID ();
				}

				//
				//	Are the preset's factories the same?
				//
				if (drag_classid == drop_classid) {

					bool is_drop_temp	= (drop_preset != NULL && drop_preset->Get_IsTemporary ()) ? true : false;
					bool is_drag_temp	= (drag_preset != NULL && drag_preset->Get_IsTemporary ()) ? true : false;

					//
					//	We allow the drop if the target is a factory, or both the drag and drops are non-temp
					//
					if (factory != NULL || is_drag_temp || is_drop_temp == false) {					
						retval = true;
					}
				}
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  Do_Drop
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Do_Drop (HTREEITEM drag_item, HTREEITEM drop_item)
{
	CWaitCursor wait_cursor;

	PresetClass *drag_preset = Get_Item_Preset (drag_item);
	bool is_drag_temp	= (drag_preset != NULL && drag_preset->Get_IsTemporary ()) ? true : false;

	//
	//	Does this operation affect only shared presets or only temps?
	//
	if (is_drag_temp == false) {
		uint32 drag_preset_id = drag_preset->Get_Definition ()->Get_ID ();

		//
		//	Check out the definition database
		//
		SelectionCaptureClass sel_capture (this);
		bool undo_on_err	= false;
		uint32 class_id	= drag_preset->Get_Class_ID ();
		if (PresetMgrClass::Check_Out_Database (class_id, &undo_on_err)) {
			sel_capture.Restore ();

			//
			//	Look-up the preset tree items
			//
			drag_item = Find_Preset (TVI_ROOT, drag_preset_id);
			drop_item = m_TreeCtrl.GetSelectedItem ();
			drag_preset = Get_Item_Preset (drag_item);

			//
			//	Are the drag and drop locations still valid?
			//
			if (	drag_preset != NULL &&
					drag_preset->Get_Definition ()->Get_ID () == drag_preset_id &&
					drag_item != NULL && drop_item != NULL && drag_item != drop_item)
			{				
				//
				//	Perform the operation and save the presets...
				//
				Move_Preset (drag_item, drop_item);	
				Save_Global_Presets (class_id);
				PresetMgrClass::Add_Dirty_Preset (drag_preset->Get_ID ());				

			} else {
				
				//
				//	Inform the user that something has changed...
				//
				::MessageBox (	NULL,
									"Either the drag source or the drop target no longer exists, please try again.",
									"Drag/Drop Error",
									MB_OK | MB_ICONERROR);

				if (undo_on_err) {
					PresetMgrClass::Undo_Database_Check_Out (class_id);
				}
			}
		}

	} else {
		
		//
		//	Simply move the preset and save the temp library
		//
		Move_Preset (drag_item, drop_item);
		if (drag_preset != NULL) {
			Save_Temp_Presets ();
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Move_Preset
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Move_Preset (HTREEITEM preset_item, HTREEITEM parent_item)
{
	ASSERT (preset_item != NULL);
	ASSERT (parent_item != NULL);

	PresetClass *preset = Get_Item_Preset (preset_item);
	if (preset != NULL) {
		
		//
		//	Let the preset know who its new parent is
		//
		PresetClass *parent_preset	= Get_Item_Preset (parent_item);
		preset->Set_Parent (parent_preset);
		if (parent_preset != NULL) {
			parent_preset->Add_Child_Preset (preset->Get_ID ());
		}

		//
		//	Make a log entry about the new parent
		//
		if (preset->Get_IsTemporary () == false) {

			//
			//	Get the name of the new parent
			//
			const char *parent_name = NULL;
			if (parent_preset != NULL) {
				parent_name = parent_preset->Get_Name ();
			}

			//
			//	Log the operation
			//
			PresetLoggerClass::Log_Moved (preset, parent_name);
		}

		//
		//	Move the tree item (and all its children)
		//
		m_TreeCtrl.SetRedraw (false);
		Copy_Preset_Items (preset_item, parent_item);
		m_TreeCtrl.DeleteItem (preset_item);
		m_TreeCtrl.SetRedraw (true);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Copy_Preset_Items
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Copy_Preset_Items (HTREEITEM preset_item, HTREEITEM parent_item)
{
	//
	//	Copy the preset to its new parent
	//
	PresetClass *preset = Get_Item_Preset (preset_item);
	HTREEITEM new_item = Insert_Preset (parent_item, preset);

	//
	//	Recursive copy all child items
	//
	for (	HTREEITEM child_item = m_TreeCtrl.GetChildItem (preset_item);
			child_item != NULL;
			child_item = m_TreeCtrl.GetNextSiblingItem (child_item))
	{
		Copy_Preset_Items (child_item, new_item);
	}

	// Ensure the entries are sorted correctly
	m_TreeCtrl.SortChildren (parent_item);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDestroy
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::OnDestroy (void)
{
	::RemoveProp (m_TreeCtrl, "TRANS_ACCS");
	::RemoveProp (m_hWnd, "TRANS_ACCS");

	CDialog::OnDestroy ();
	return ;
	
}


////////////////////////////////////////////////////////////////////////////
//
//  Export_File_Dependencies
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Export_File_Dependencies (const char *filename)
{
	//
	//	Create the file
	//
	TextFileClass file (filename);
	if (file.Open (RawFileClass::WRITE)) {

		//
		//	Build a list of all the entries in the tree
		//
		DynamicVectorClass<HTREEITEM> entry_list;
		Build_Child_List (TVI_ROOT, entry_list);

		//
		//	Loop over all the presets in the list
		//
		for (int index = 0; index < entry_list.Count (); index ++) {
			PresetClass *preset = Get_Item_Preset (entry_list[index]);
			if (preset != NULL) {
				DefinitionClass *definition = preset->Get_Definition ();
				if (definition != NULL) {
					
					//
					//	Generate the list of file's that this preset is dependent on
					//
					DynamicVectorClass<StringClass> file_list;
					Build_File_Dependencies_For_Definition (file_list, definition);

					//
					//	If the list wasn't empty, then export the filenames to the text file.
					//
					if (file_list.Count () > 0) {
						
						//
						//	Start the entry off with the name of the preset
						//
						StringClass entry;
						entry.Format ("%s\t", (const char *)definition->Get_Name ());

						//
						//	Now, spit out the list of files
						//
						for (int index = 0; index < file_list.Count (); index ++) {
							entry += file_list[index];
							entry += "\t";
						}

						//
						//	Add the entry to the text file
						//
						file.Write_Line (entry);
					}
				}
			}
		}

		//
		//	Close the file
		//
		file.Close ();
	}

	return ;
	
}


////////////////////////////////////////////////////////////////////////////
//
//  Build_File_Dependencies_For_Definition
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Build_File_Dependencies_For_Definition
(
	DynamicVectorClass<StringClass> &	file_list,
	DefinitionClass *							definition
)
{	
	//
	//	Loop over all the parameters of this definition
	//
	int count = definition->Get_Parameter_Count ();
	for (int index = 0; index < count; index ++) {
		ParameterClass *parameter = definition->Lock_Parameter (index);

		//
		//	Handle the different file types
		//
		if (parameter->Is_Type (ParameterClass::TYPE_FILENAME)) {
			FilenameParameterClass *filename_param = reinterpret_cast<FilenameParameterClass *> (parameter);
			
			//
			//	Add the filename to the list if its not empty
			//
			StringClass filename = filename_param->Get_String ();			
			if (filename.Get_Length () > 0) {
				file_list.Add (filename);
			}

		} else if (parameter->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
			int def_id = ((ModelDefParameterClass *)parameter)->Get_Value ();
			
			//
			//	Recurse into the model's definition (if necessary)
			//
			DefinitionClass *model_definition = DefinitionMgrClass::Find_Definition (def_id, false);
			if (model_definition != NULL) {
				Build_File_Dependencies_For_Definition (file_list, model_definition);
			}

		} else if (parameter->Get_Type () == ParameterClass::TYPE_PHYSDEFINITIONID) {
			int def_id = ((PhysDefParameterClass *)parameter)->Get_Value ();

			//
			//	Recurse into the physic object's definition (if necessary)
			//
			DefinitionClass *phys_definition = DefinitionMgrClass::Find_Definition (def_id, false);
			if (phys_definition != NULL) {
				Build_File_Dependencies_For_Definition (file_list, phys_definition);
			}
		}

		definition->Unlock_Parameter (index);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Add_Preset
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Add_Preset (PresetClass *preset)
{
	uint32 class_id = preset->Get_Class_ID ();

	//
	//	Find the root item 
	//
	HTREEITEM root_item = Find_Factory (TVI_ROOT, class_id);
	if (root_item != NULL) {
		
		//
		//	Add the preset to the tree
		//
		Insert_Preset (root_item, preset);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Sort_Items
//
////////////////////////////////////////////////////////////////////////////
void
PresetsFormClass::Sort_Items (uint32 class_id)
{
	//
	//	Find the root item 
	//
	HTREEITEM root_item = Find_Factory (TVI_ROOT, class_id);
	if (root_item != NULL) {

		//
		//	Sort all items under this root
		//
		m_TreeCtrl.SetRedraw (false);
		Sort_Nodes (root_item);
		m_TreeCtrl.SetRedraw (true);
	}

	return ;
}


