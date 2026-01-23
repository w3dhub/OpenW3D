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

// LevelEditDoc.cpp : implementation of the CLevelEditDoc class
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "leveleditdoc.h"
#include "Utils.h"
#include "physcoltest.h"
#include "gridcull.h"
#include "aabtreecull.h"
#include "light.h"
#include "ffactory.h"
#include "LevelEditView.h"
#include "filemgr.h"
#include "profilesection.h"
#include "editorphys.h"
#include "sceneeditor.h"
#include "nodemgr.h"
#include "node.h"
#include "cameramgr.h"
#include "regkeys.h"
#include "editorassetmgr.h"
#include "_assetmgr.h"
#include "rcfile.h"
#include "editorini.h"
#include "updateassetsdialog.h"
#include "filelocations.h"
#include "ww3d.h"
#include "editorsaveload.h"
#include "presetslibform.h"
#include "rawfile.h"
#include "progressui.h"
#include "verchk.h"
#include "overlappage.h"
#include "stringsmgr.h"
#include "dazzle.h"
#include "texture.h"
#include "conversationpage.h"
#include "conversationeditormgr.h"
#include "dx8renderer.h"
#include "presetmgr.h"
#include "combatchunkid.h"
#include "combat.h"
#include "backgroundmgr.h"
#include "WeatherMgr.h"
#include "gameobjmanager.h"
#include "scriptman.h"
#include "SoundEnvironment.h"
#include "soldier.h"
#include "editorbuild.h"
#include "export.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////
const Vector3			DEF_BACK_COLOR			(0.2F, 0.4F, 0.6F);
const Vector3			DEF_AMBIENT_COLOR		(0.3F, 0.3F, 0.3F);


/////////////////////////////////////////////////////////////////////////////
//	Static member initialization
/////////////////////////////////////////////////////////////////////////////
bool	CLevelEditDoc::IsBatchExportMode = false;


/////////////////////////////////////////////////////////////////////////////
// CLevelEditDoc

IMPLEMENT_DYNCREATE(CLevelEditDoc, CDocument)

BEGIN_MESSAGE_MAP(CLevelEditDoc, CDocument)
	//{{AFX_MSG_MAP(CLevelEditDoc)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLevelEditDoc construction/destruction

////////////////////////////////////////////////////////////
//
//	CLevelEditDoc
//
CLevelEditDoc::CLevelEditDoc (void)
	:	m_pScene (NULL),
		m_bPerformanceSampling (false),
		m_DisplayPathWeb (false),
		m_DisplayFullPaths (false),
		m_CoordSystem (COORD_WORLD),
		m_AxisRestriction (RESTRICT_NONE),
		m_ModeModifiers (0),
		m_bSnapPointsActive (true),
		m_bNeedRepartition (false),
		m_DebugScriptsMode (false)
{
	// Allocate new managers
	m_pFileMgr = new FileMgrClass;

	//
	//	Read the settings from the registry
	//
	m_DebugScriptsMode = (theApp.GetProfileInt (CONFIG_KEY, "ScriptDebugMode", 0) == 1);
	return;
}


////////////////////////////////////////////////////////////
//
//	~CLevelEditDoc
//
CLevelEditDoc::~CLevelEditDoc (void)
{
	// Used to ensure all dynamically allocated data is freed
	CleanupResources ();
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Set_Debug_Script_Mode
//
///////////////////////////////////////////////////////////////
void
CLevelEditDoc::Set_Debug_Script_Mode (bool onoff)
{
	m_DebugScriptsMode = onoff;
	theApp.WriteProfileInt (CONFIG_KEY, "ScriptDebugMode", m_DebugScriptsMode);
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Initialize_VSS
//
bool
CLevelEditDoc::Initialize_VSS (void)
{
	bool retval = false;
	ASSERT (m_pFileMgr != NULL);

#ifdef PUBLIC_EDITOR_VER
	m_pFileMgr->Initialize_VSS ("");
	retval = true;
#else

	// Get the default VSS database to open
	CString database_path = theApp.GetProfileString (CONFIG_KEY,
																	 VSSDB_VALUE,
																	 DEF_VSS_DB);

	// Look up the user's Source Safe logon name
	TCHAR user_name[256] = { 0 };
	bool blogon_success = false;
	if (::GetEnvironmentVariable ("SSUSER", user_name, sizeof (user_name)) == 0) {

		// Let the user know they need to setup this variable
		::Message_Box (NULL,
							IDS_NOVSSUSER_MSG,
							IDS_NOVSS_TITLE,
							MB_ICONERROR | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
	} else {

		// Attempt to log onto the database using the correct username
		blogon_success = m_pFileMgr->Initialize_VSS (database_path, user_name);
	}

	// Were we successful in logging on with the username?
	if (blogon_success == false) {
		
		// Attempt to logon with read-only access
		blogon_success = m_pFileMgr->Initialize_VSS (database_path);
		if (!blogon_success) {
		
			// If we were unsuccessful, let the user know
			::Message_Box (NULL,
								IDS_NOVSS_MSG,
								IDS_NOVSS_TITLE,
								MB_ICONERROR | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
		}

		// Ensure or is VSS interface read-only
		m_pFileMgr->Set_Read_Only_VSS (true);
	}
	
	if (m_pFileMgr->Is_VSS_Read_Only ()) {

		// Let the user know they are in read-only mode
		::Message_Box (NULL,
							IDS_VSS_READONLY_MSG,
							IDS_NOVSS_TITLE,
							MB_ICONEXCLAMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
	}

	// If at some point we were successful, then cache the database
	// path in the registry and return success
	if (blogon_success) {
		theApp.WriteProfileString (CONFIG_KEY, VSSDB_VALUE, database_path);	
		retval = true;
	}		

#endif //PUBLIC_EDITOR_VER
	
	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Change_Base_Path
//
bool
CLevelEditDoc::Change_Base_Path (void)	
{
	// Assume failure
	bool retval = false;

	// Ask the user to pick a base path
	CString base_path;
	if (::Browse_For_Folder (base_path,
									 NULL,
									 NULL,
									 "Please select a location to keep your models.  A subdir named Assets will be created automatically.")) {
		
		// Concat the Assets subdir onto the path
		if (base_path[::lstrlen (base_path)-1] != '\\') {
			base_path += "\\";
		}
		base_path += "Assets";

		// Let the file manager know what the new base path is
		m_pFileMgr->Set_Base_Path (base_path);
		retval = true;
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Update_Core_Files
//
///////////////////////////////////////////////////////////////
void
CLevelEditDoc::Update_Core_Files (void)
{	
	// Ensure that VSS is initialized
	if (Initialize_VSS ()) {
		
		// Do we already have a base path?
		if (::lstrlen (m_pFileMgr->Get_Base_Path ()) == 0) {
								
			// Ask the user for a base path
			if (!Change_Base_Path ()) {

				// Pick a path for the user
				CString path = ::Get_Startup_Directory ().Left (3);
				path += DEF_ASSET_PATH;
				m_pFileMgr->Set_Base_Path (path);
			}
		}

		//
		//	Add some search paths to the file factory
		//		
		EditorFileFactoryClass::Add_Search_Path (m_pFileMgr->Make_Full_Path (SHATTER_DIR));
		EditorFileFactoryClass::Add_Search_Path (m_pFileMgr->Make_Full_Path (WEATHER_DIR));
		EditorFileFactoryClass::Add_Search_Path (m_pFileMgr->Make_Full_Path (SKY_DIR));
		EditorFileFactoryClass::Add_Search_Path (m_pFileMgr->Make_Full_Path ("always\\sound\\weather"));
		EditorFileFactoryClass::Add_Search_Path (m_pFileMgr->Make_Full_Path ("always\\effects"));

		ProgressUIMgrClass::Set_Status_Text ("Updating core asset files...");

#ifdef PUBLIC_EDITOR_VER
		Update_Assets_For_Mod_Tree ();
#else

		//
		// Make sure we have the latest versions of the core files		
		//
		m_pFileMgr->Update_File (PRESETS_PATH);
		
		if (::Is_Silent_Mode () == false) {
			m_pFileMgr->Update_File (ALWAYS_INI_PATH);
			m_pFileMgr->Update_File (SERIAL_INI_PATH);
			m_pFileMgr->Update_File (UPDATE_INI_PATH);		
			m_pFileMgr->Get_Subproject (ALWAYS_INI_DIR);
			m_pFileMgr->Update_File (HUMAN_SKEL_PATH);
			m_pFileMgr->Update_File (SKY_DIR);
			m_pFileMgr->Update_File (WEATHER_DIR);
			m_pFileMgr->Update_File (SHATTER_DIR);
			m_pFileMgr->Build_Global_Include_List ();

			if (m_DebugScriptsMode == false) {
				m_pFileMgr->Update_File (SCRIPTS_PATH);
			}

			//
			// Ensure the asset tree is updated
			//
			Update_Asset_Tree ();
		}

#endif //PUBLIC_EDITOR_VER

		//
		//	Update the strings database
		//
		StringsMgrClass::Create_Database_If_Necessary ();
		StringsMgrClass::Load_Translation_Database ();
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Update_Assets_For_Mod_Tree
//
///////////////////////////////////////////////////////////////
void
CLevelEditDoc::Update_Assets_For_Mod_Tree (void)
{
	//
	//	Get all INI files to the always\INI directory
	//
	CString full_ini_path = ::Get_File_Mgr ()->Make_Full_Path (ALWAYS_INI_DIR);
	m_pFileMgr->Update_All_Files (full_ini_path, "*.ini");

	//
	//	Make sure to get the human skeleton
	//
	m_pFileMgr->Update_File ("presets\\objects.ddb");

	//
	//	Make sure to get the human skeleton
	//
	m_pFileMgr->Update_File (HUMAN_SKEL_PATH);
	return ;	
}


///////////////////////////////////////////////////////////////
//
//  Update_Asset_Tree
//
///////////////////////////////////////////////////////////////
void
CLevelEditDoc::Update_Asset_Tree (void)
{	
	// Get a pointer to the INI file from the asset manager
	CString path = ::Get_File_Mgr ()->Make_Full_Path (UPDATE_INI_PATH);
	EditorINIClass *pini = _pThe3DAssetManager->Get_INI (path);
	
	// Were we successful in getting a pointer to the INI file?
	ASSERT (pini != NULL);
	if (pini != NULL) {

		// Determine what the last time we updated was and when the
		// last time the database was modified
		int update_id = theApp.GetProfileInt (CONFIG_KEY, LAST_UPDATE_VALUE, 0);
		int curr_update_id = pini->Get_Int ("General", "ID");

		// Do we need to update?
		if (update_id < curr_update_id) {
			
			// Get the comments from the file
			TCHAR comments[512];
			pini->Get_TextBlock ("Comments", comments, sizeof (comments));
			for (LPSTR newline = ::strchr (comments, '|');
			     newline != NULL;
				  newline = ::strchr (newline, '|'))
			{
				newline[0] = '\r';
				newline[1] = '\n';
			}

			//
			//	Build a list of directories that we need to update...
			//
			STRING_LIST directory_list;
			bool update_all = false;
			for (int index = update_id + 1; index <= curr_update_id; index ++) {
				
				//
				//	Get the directory associated with this update-id
				//
				CString number_string;
				number_string.Format ("%d", index);
				StringClass filename;
				pini->Get_String (filename, "Details", number_string);

				//
				//	If we couldn't get the directory, then update all files
				//
				if (filename.Get_Length () > 0) {
					directory_list.Add (::Get_File_Mgr ()->Make_Full_Path (filename));
				} else {
					update_all = true;
				}
			}

			//
			// Show a dialog to the user that will ask them
			// if they want to update now.
			//
			//if (::Is_Silent_Mode () == false) {
				UpdateAssetsDialogClass dialog (CString (comments), directory_list, update_all, NULL);
				if (dialog.DoModal () == IDOK) {

					// Remember that we've updated ourselves
					theApp.WriteProfileInt (CONFIG_KEY, LAST_UPDATE_VALUE, curr_update_id);
				}
			//}
		}

		// Free the INI object
		SAFE_DELETE (pini);
	}
	
	return ;
}


///////////////////////////////////////////////////////////////
//
//  CleanupResources
//
void
CLevelEditDoc::CleanupResources (void)
{
	//
	// Release our hold on the scene
	//
	MEMBER_RELEASE (m_pScene);
	SAFE_DELETE (m_pFileMgr);
	return ;
}


////////////////////////////////////////////////////////////
//
//	OnNewDocument
//
BOOL
CLevelEditDoc::OnNewDocument (void)
{
	// Allow the base class to process this message
	if (!CDocument::OnNewDocument())
		return false;

	//
	// Keep the view from refreshing
	//
	::Get_Main_View ()->Allow_Repaint (false);

	//
	// Instantiate a new scene
	//
	MEMBER_RELEASE (m_pScene);
	m_pScene = new SceneEditorClass;
	m_pScene->Set_Ambient_Light (DEF_AMBIENT_COLOR);
	m_pScene->Update_Lighting ();
	m_pScene->Enable_Collision_Detection (0, 0);
   m_pScene->Set_Sun_Light_Orientation (0, DEG_TO_RADF (50));
	m_pScene->Update_Lighting();

	// Reset sound environment, background and weather systems.
	CombatManager::Get_Sound_Environment()->Reset();
	BackgroundMgrClass::Reset();
	WeatherMgrClass::Reset();

	//
	// Reset the scene, camera, and file manager
	//	
	m_pFileMgr->Initialize ();
	::Get_Main_View ()->Reset_View ();
	::Get_Scene_Editor ()->Re_Partition_Dynamic_Culling_System ();
	SetModifiedFlag (false);

	//
	//	Force free the assets...
	//
	WW3DAssetManager::Get_Instance ()->Free_Assets ();
	TheDX8MeshRenderer.Invalidate ();
	Preload_Human_Data ();
	Preload_Tool_Assets ();	

	//
	//	Reset the overlap-sphere page
	//
	OverlapPageClass *form = ::Get_Overlap_Form ();
	if (form != NULL) {
		form->Reset_Tree ();
	}

	//
	//	Reset the conversation page
	//
	ConversationPageClass *conversation_form = ::Get_Conversation_Form ();
	if (conversation_form != NULL) {
		conversation_form->Reload_Data ();
	}

	// Allow a refresh
	::Get_Main_View ()->Allow_Repaint (true);
	::Refresh_Main_View ();
	return true;
}


////////////////////////////////////////////////////////////
//
//	Serialize
//
////////////////////////////////////////////////////////////
void
CLevelEditDoc::Serialize (CArchive& /* ar */)
{
	SetModifiedFlag (false);
	return ;
}

/////////////////////////////////////////////////////////////////////////////
// CLevelEditDoc diagnostics

#ifdef _DEBUG
void CLevelEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLevelEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


///////////////////////////////////////////////////////////////
//
//  Do_Version_Check
//
////////////////////////////////////////////////////////////
bool
CLevelEditDoc::Do_Version_Check (void)
{	
	bool retval = true; 

#ifndef PUBLIC_EDITOR_VER
	//
	//	Check the version of the level editor that is out on the network
	// against the version we are running.  We don't want people making modifications
	// with and older version
	//
	if (Check_Editor_Version () == false) {
		if (Message_Box (NULL, IDS_VERSION_ERROR_MSG, IDS_VERSION_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OKCANCEL | MB_SETFOREGROUND | MB_SYSTEMMODAL) == IDCANCEL) {
			
			//
			//	Just for kicks, exit the process pretty violently
			//
			ExitProcess (1);
			retval = false;
		}
		m_pFileMgr->Set_Read_Only_VSS (true);
	}

#endif //PUBLIC_EDITOR_VER

	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Preload_Assets
//
////////////////////////////////////////////////////////////
void
CLevelEditDoc::Preload_Assets (void)
{
	Preload_Human_Data ();
	Preload_Tool_Assets ();

	//
	//	Initialize the dazzle system
	//
	CString dazzle_ini_path = ::Get_File_Mgr ()->Make_Full_Path (DAZZLE_INI_PATH);
	FileClass * dazzle_ini_file = _TheFileFactory->Get_File (dazzle_ini_path);
	if (dazzle_ini_file) { 
		INIClass dazzle_ini (*dazzle_ini_file);
		DazzleRenderObjClass::Init_From_INI (&dazzle_ini);
		_TheFileFactory->Return_File (dazzle_ini_file);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Preload_Tool_Assets
//
////////////////////////////////////////////////////////////
void
CLevelEditDoc::Preload_Tool_Assets (void)
{
	//
	// Load the tool-art textures into the asset manager
	//
	_pThe3DAssetManager->Load_Resource_Texture ("METER.TGA");
	_pThe3DAssetManager->Load_Resource_Texture ("TA_CEMENT.TGA");
	_pThe3DAssetManager->Load_Resource_Texture ("TA_FRONT.TGA");
	_pThe3DAssetManager->Load_Resource_Texture ("TA_DUMMY.TGA");

	//
	// Load the tool-art models into the asset manager
	//
	ResourceFileClass way_start (NULL, "WayStart.w3d");
	ResourceFileClass way_mid (NULL, "WayMid.w3d");
	ResourceFileClass way_end (NULL, "WayStop.w3d");
	ResourceFileClass way_r (NULL, "Way_R.w3d");
	ResourceFileClass way_g (NULL, "Way_G.w3d");
	ResourceFileClass way_b (NULL, "Way_B.w3d");
	ResourceFileClass point_light (NULL, "PointLight.w3d");
	ResourceFileClass speaker1 (NULL, "Speaker.w3d");
	ResourceFileClass speaker2 (NULL, "Speaker2.w3d");
	ResourceFileClass body_box (NULL, "BODYBOX.W3D");
	ResourceFileClass camera_model (NULL, "CAMERA.W3D");
	ResourceFileClass grid_model (NULL, "GRID.W3D");
	ResourceFileClass trans_box_model (NULL, "TRANSBOX.W3D");
	ResourceFileClass dummy_model (NULL, "DUMMY.W3D");
	ResourceFileClass cover_spot_model (NULL, "COVERSPOT.W3D");
	ResourceFileClass building_model (NULL, "BUILDINGICON.W3D");
	ResourceFileClass carmarker_model (NULL, "CARMARKER.W3D");	
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (way_start);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (way_mid);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (way_end);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (way_r);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (way_g);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (way_b);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (point_light);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (speaker1);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (speaker2);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (body_box);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (camera_model);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (grid_model);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (trans_box_model);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (dummy_model);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (cover_spot_model);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (building_model);
	WW3DAssetManager::Get_Instance()->Load_3D_Assets (carmarker_model);

	//
	//	Build a list of all the textures in the sky and weather directories
	//
	/*CString path1 = ::Get_File_Mgr()->Make_Full_Path (SKY_DIR);
	CString path2 = ::Get_File_Mgr()->Make_Full_Path (WEATHER_DIR);
	DynamicVectorClass<CString> file_list;
	::Get_File_Mgr ()->Find_Files (path1, "*.tga", file_list, false);
	::Get_File_Mgr ()->Find_Files (path2, "*.tga", file_list, false);

	//
	//	Preload all of these textures
	//
	for (int index = 0; index < file_list.Count (); index ++) {
		CString &filename = file_list[index];

		TextureClass *texture = WW3DAssetManager::Get_Instance()->Get_Texture (filename);
		REF_PTR_RELEASE (texture);
	}*/

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Preload_Human_Data
//
////////////////////////////////////////////////////////////
void
CLevelEditDoc::Preload_Human_Data (void)
{
	//
	//	Make sure the human skeletons are loaded
	//	
	_pThe3DAssetManager->Load_3D_Assets (HUMAN_SKEL_PATH);

	//
	//	Force load the commando model
	//	
	PresetClass *preset = PresetMgrClass::Find_Typed_Preset (CLASSID_GAME_OBJECT_DEF_SOLDIER, "Commando");
	if (preset != NULL) {
		preset->Load_All_Assets ();
		SoldierGameObj *soldier = (SoldierGameObj *)preset->Get_Definition ()->Create ();
		if (soldier != NULL) {
			soldier->Set_Delete_Pending ();
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Init_Scene
//
////////////////////////////////////////////////////////////
void
CLevelEditDoc::Init_Scene (void)
{
	if (m_pScene == NULL) {

		// Ensure we have the latest core files
		Update_Core_Files ();		

		//
		// Add the texture cache to the search path
		//
		EditorFileFactoryClass::Add_Search_Path (::Get_File_Mgr ()->Get_Texture_Cache_Path ());

		//
		//	Load the global conversations
		//
		ConversationEditorMgrClass::Get_Latest_Version ();
		ConversationEditorMgrClass::Load_Global_Database ();

		//
		//	Make sure we are using the right version of the editor
		//
		Do_Version_Check ();

		//
		//	Make sure certain *required* assets are loaded into memory...
		//
		Preload_Assets ();

		//
		// Instantiate a new scene
		//
		m_pScene = new SceneEditorClass;
		m_pScene->Set_Ambient_Light (DEF_AMBIENT_COLOR);
		m_pScene->Update_Lighting ();
		m_pScene->Enable_Collision_Detection (0, 0);		
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Get_Coord_System_Transform
//
///////////////////////////////////////////////////////////////
Matrix3D
CLevelEditDoc::Get_Coord_System_Transform (NodeClass *node)
{
	Matrix3D matrix (1);

	// What coordinate system are we working in?
	switch (m_CoordSystem) {
		
		case CLevelEditDoc::COORD_WORLD:
			matrix = Matrix3D (1);
			break;

		case CLevelEditDoc::COORD_OBJECT:
			matrix = node->Get_Transform ();
			break;

		case CLevelEditDoc::COORD_VIEW:
			matrix = ::Get_Camera_Mgr ()->Get_Camera()->Get_Transform ();
			break;
	}

	// Return the matrix
	return matrix;
}


///////////////////////////////////////////////////////////////
//
//  OnOpenDocument
//
///////////////////////////////////////////////////////////////
BOOL
CLevelEditDoc::OnOpenDocument (LPCTSTR path) 
{
	::Get_Main_View ()->Allow_Repaint (false);

	DWORD start_time = ::GetTickCount ();
	
	//
	// Start a new document
	//
	OnNewDocument ();

	//
	//	Load the level
	//
	EditorSaveLoadClass::Load_Level (path);
	SetModifiedFlag (false);

	//
	//	Reload the conversation page
	//
	ConversationPageClass *conversation_form = ::Get_Conversation_Form ();
	if (conversation_form != NULL) {
		conversation_form->Reload_Data ();
	}

	//
	//	Load the level load time
	//
	CString load_time_message;
	load_time_message.Format ("Level load took %d seconds\r\n", int((::GetTickCount () - start_time) / 1000));
	::Output_Message (load_time_message);
		
	::Get_Main_View ()->Allow_Repaint (true);
	return true;
}


///////////////////////////////////////////////////////////////
//
//  Set_Current_Path
//
///////////////////////////////////////////////////////////////
void
CLevelEditDoc::Set_Current_Path (const char *path)
{
	m_strPathName = path;		
	return ;	
}


///////////////////////////////////////////////////////////////
//
//  OnSaveDocument
//
///////////////////////////////////////////////////////////////
BOOL
CLevelEditDoc::OnSaveDocument (LPCTSTR path) 
{	
	theApp.WriteProfileString (CONFIG_KEY,
										LAST_SAVE_DIR_VALUE,
										::Strip_Filename_From_Path (path));

	// Peform the save operation
	EditorSaveLoadClass::Save_Level (path);
	return true;
}


///////////////////////////////////////////////////////////////
//
//  Repartition_If_Necessary
//
///////////////////////////////////////////////////////////////
void
CLevelEditDoc::Repartition_If_Necessary (void) 
{
	if (m_bNeedRepartition) {
		
		// (gth) can't re-partition the object culling systems
		// or we'll lose hierarchical vis...
		m_pScene->Re_Partition_Static_Lights ();
		m_pScene->Re_Partition_Audio_System ();

		m_bNeedRepartition = false;
	}
		
	return ;
}


///////////////////////////////////////////////////////////////
//
//  PreCloseFrame
//
///////////////////////////////////////////////////////////////
void
CLevelEditDoc::PreCloseFrame (CFrameWnd * /* pFrame */)
{
	//
	//	Cleanup any dangling embedded objects
	//
	PresetMgrClass::Free_All_Embedded_Nodes ();
	NetworkObjectMgrClass::Delete_Pending ();
	GameObjObserverManager::Delete_Pending ();
	ScriptManager::Destroy_Pending ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	OnFileSave
//
//////////////////////////////////////////////////////////////////////////
void
CLevelEditDoc::OnFileSave (void)
{

#ifdef PUBLIC_EDITOR_VER

	if (m_strPathName.IsEmpty () || ::GetFileAttributes (m_strPathName) == 0xFFFFFFFF) {
		
		//
		//	Pick a new path and save to it
		//
		CString full_path;
		if (Pick_Save_Path (full_path)) {
			Save_Level_File (full_path);
		}

	} else {
		
		//
		//	Save to this path
		//
		Save_Level_File (m_strPathName);
	}

#else	
	CDocument::OnFileSave ();		
#endif //PUBLIC_EDITOR_VER
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	OnFileSaveAs
//
//////////////////////////////////////////////////////////////////////////
void
CLevelEditDoc::OnFileSaveAs (void)
{
#ifdef PUBLIC_EDITOR_VER

	//
	//	Pick a new path and save to it
	//
	CString full_path;
	if (Pick_Save_Path (full_path)) {
		Save_Level_File (full_path);
	}

#else	
	CDocument::OnFileSaveAs ();		
#endif //PUBLIC_EDITOR_VER

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Pick_Save_Path
//
//////////////////////////////////////////////////////////////////////////
bool
CLevelEditDoc::Pick_Save_Path (CString &save_path)
{
	bool retval = false;

	//
	// Determine which directory we should save to
	//
	CString full_path = ::Get_File_Mgr ()->Make_Full_Path (LEVELS_ASSET_DIR);		
	_pThe3DAssetManager->Set_Current_Directory (full_path);

	//
	// Make a default filename from the name of the document
	//
	CString default_name = GetTitle ();
	if (default_name.GetLength () > 0) {
		//default_name += ".lvl";
	}

	CFileDialog dialog (	false,
								".lvl",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Level Editor Files (*.lvl)|*.lvl||",
								::AfxGetMainWnd ());

	//
	// Set the path, so it opens in the correct directory
	//
	dialog.m_ofn.lpstrInitialDir = full_path;

	//
	// Ask the user where they want to save this level
	//
	if (dialog.DoModal () == IDOK) {
		
		//
		//	Return the path to the caller
		//
		retval		= true;
		save_path	= dialog.GetPathName ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////
//
//	Save_Level_File
//
//////////////////////////////////////////////////////////////////////////
void
CLevelEditDoc::Save_Level_File (const char *full_path)
{
	//
	// Reset the title and change the document name
	//
	SetPathName (full_path);

	//
	//	Save in level editor format
	//
	OnSaveDocument (full_path);

	//
	//	Save in game format
	//
	ExporterClass exporter;
	exporter.Export_Level_Only (full_path);
	return ;
}
