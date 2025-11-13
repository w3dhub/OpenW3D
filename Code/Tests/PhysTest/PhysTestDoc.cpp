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

// PhysTestDoc.cpp : implementation of the CPhysTestDoc class
//

#include "stdafx.h"
#include "PhysTest.h"

#include "PhysTestDoc.h"
#include "MainFrm.h"
#include "DataView.h"
#include "ww3d.h"
#include "ffactory.h"
#include "pscene.h"
#include "light.h"
#include "rcfile.h"
#include "assetmgr.h"
#include "rendobj.h"
#include "chunkio.h"
#include "lev_file.h"
#include "physlist.h"
#include "saveload.h"
#include "chunkio.h"
#include "rawfile.h"
#include "physstaticsavesystem.h"
#include "physdynamicsavesystem.h"
#include "PhysTestSaveSystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Chunk ID's used by PhysTestDoc

enum 
{
	PHYSTESTDOC_CHUNK_MAINFRAME	= 0x03321990,
	PHYSTESTDOC_CHUNK_SYSTEMS,
};


/////////////////////////////////////////////////////////////////////////////
// CPhysTestDoc

IMPLEMENT_DYNCREATE(CPhysTestDoc, CDocument)

BEGIN_MESSAGE_MAP(CPhysTestDoc, CDocument)
	//{{AFX_MSG_MAP(CPhysTestDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhysTestDoc construction/destruction

CPhysTestDoc::CPhysTestDoc() : 
	Scene(NULL),
	Light(NULL),
	Origin(NULL)
{
}

CPhysTestDoc::~CPhysTestDoc()
{
	if (Scene) { 
		Scene->Remove_All();
	}
	REF_PTR_RELEASE(Scene);
	REF_PTR_RELEASE(Light);
	REF_PTR_RELEASE(Origin);
}


void CPhysTestDoc::Init_Scene(void)
{
	if (Scene == NULL)
	{
		// Instantiate a new scene
		Scene = NEW_REF(PhysicsSceneClass,());
		Scene->Enable_Dynamic_Projectors(true);

		// Were we successful in instantiating the scene object?
		ASSERT(Scene);
		if (Scene != NULL) {

			// Set up some collision groups.
			Scene->Enable_All_Collision_Detections(0);

			// Set some default ambient lighting
			Scene->Set_Ambient_Light(Vector3 (0.5F, 0.5F, 0.5F));

			// Create a new scene light
			if (Light == NULL) {
				Light = NEW_REF(LightClass,(LightClass::DIRECTIONAL));
				ASSERT(Light);

				if (Light) {

					// Create some default light settings
					Matrix3D transform(1);
					transform.Look_At(Vector3(0,0,0),Vector3(10,10,10),0.0f);
					Light->Set_Transform(transform);

					Light->Set_Attenuation_Model(LightClass::ATTENUATION_NONE);
					Light->Set_Flag (LightClass::NEAR_ATTENUATION, false);
					Light->Set_Flag (LightClass::FAR_ATTENUATION, false);
					Light->Set_Far_Attenuation_Range (1000000, 1000000);
					
					Light->Set_Intensity(1.0F);
					Light->Set_Force_Visible(true);
					Light->Set_Ambient(Vector3(0,0,0));
					Light->Set_Diffuse(Vector3(1, 1, 1));
					Light->Set_Specular(Vector3(1, 1, 1));

					// Add this light to the scene
					Light->Add(Scene);
				}
			}

			// Create an object at the Origin
			if (Origin == NULL) {
				ResourceFileClass mesh_file(NULL, "Axes.w3d");
				WW3DAssetManager::Get_Instance()->Load_3D_Assets(mesh_file);

				Origin = WW3DAssetManager::Get_Instance()->Create_Render_Obj("Axes");
				Origin->Set_Transform(Matrix3D(1));
				Scene->Add_Render_Object(Origin);
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CPhysTestDoc serialization

void CPhysTestDoc::Serialize(CArchive& ar)
{
	assert(0);
}

/////////////////////////////////////////////////////////////////////////////
// CPhysTestDoc diagnostics

#ifdef _DEBUG
void CPhysTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPhysTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPhysTestDoc commands

BOOL CPhysTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return false;

	if (Scene) {
		Scene->Remove_All();
		
		WWASSERT(Light);
		if (Light) {
			Scene->Add_Render_Object(Light);
		}

		WWASSERT(Origin);
		if (Origin) {
			Scene->Add_Render_Object(Origin);
		}

		// rebuild the list view
		Get_Data_View()->Rebuild_Tree();
	}

	return true;
}

BOOL CPhysTestDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	Load_PHY_File(lpszPathName); 
	Get_Data_View()->Rebuild_Tree();
	return true;
}

BOOL CPhysTestDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	Save_PHY_File(lpszPathName); 
	return true;
}


///////////////////////////////////////////////////////////////
//
//  Load_LEV_File
//
void CPhysTestDoc::Load_LEV_File(LPCTSTR lpszPathName) 
{
#if 0
	WWASSERT(Scene);

	//
	// HACK HACK -- Force the current directory to be the directory
	//              the file is located in.
	//
	if (::strrchr (lpszPathName, '\\')) {
		CString stringTemp = lpszPathName;
		stringTemp = stringTemp.Left ((int)::strrchr (lpszPathName, '\\') - (int)lpszPathName);
		::SetCurrentDirectory (stringTemp);
		WW3D::Add_Search_Path (stringTemp);
	}

	LPCTSTR extension = ::strrchr (lpszPathName, '.');
	ASSERT(::lstrcmpi(extension,".lev") == 0);

	// load the LEV
	FileClass *file = DefaultFileFactory.Get_File (lpszPathName);
	file->Open();
	ChunkLoadClass cload(file);

	// should see a LEV_CHUNK_LEVEL
	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == LEV_CHUNK_LEVEL);

	// for now only look for a LEV_CHUNK_MAP
	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == LEV_CHUNK_MAP);

	// Physics Scene handles the tile map loading
	Scene->LEV_Load(cload);

	// close the LEV_CHUNK_MAP
	cload.Close_Chunk();

	// close the LEV_CHUNK_LEVEL
	cload.Close_Chunk();

	// done with the file!
	file->Close();
	DefaultFileFactory.Return_File(file);

	// Replace our default objects
	WWASSERT(Light);
	if (Light) {
		Scene->Add_Render_Object(Light);
	}

	WWASSERT(Origin);
	if (Origin) {
		Scene->Add_Render_Object(Origin);
	}
#endif
}


///////////////////////////////////////////////////////////////
//
//  Load_W3D_File
//
void CPhysTestDoc::Load_W3D_File(LPCTSTR lpszPathName) 
{
	WW3DAssetManager::Get_Instance()->Load_3D_Assets(lpszPathName);

	// rebuild the list view
	Get_Data_View()->Rebuild_Tree();
}


///////////////////////////////////////////////////////////////
//
//  Load_PHY_File
//
void CPhysTestDoc::Load_PHY_File(LPCTSTR lpszPathName)
{
	RawFileClass file(lpszPathName);
	WWASSERT(file);
	file.Open(FileClass::READ);
	ChunkLoadClass cload(&file);

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {
			case PHYSTESTDOC_CHUNK_SYSTEMS:
				SaveLoadSystemClass::Load( cload );
				break;
		};
		cload.Close_Chunk();
	}

	file.Close();
}


///////////////////////////////////////////////////////////////
//
//  Save_PHY_File
//
void CPhysTestDoc::Save_PHY_File(LPCTSTR lpszPathName)
{
	RawFileClass file(lpszPathName);
	WWASSERT(file);
	file.Open(FileClass::WRITE);
	ChunkSaveClass csave(&file);

	csave.Begin_Chunk(PHYSTESTDOC_CHUNK_SYSTEMS);
	SaveLoadSystemClass::Save( csave, _PhysTestSaveSystem );
	SaveLoadSystemClass::Save( csave, _PhysStaticDataSaveSystem );
	SaveLoadSystemClass::Save( csave, _PhysStaticObjectsSaveSystem );
	SaveLoadSystemClass::Save( csave, _PhysDynamicSaveSystem );
	csave.End_Chunk();

	file.Close();
}

///////////////////////////////////////////////////////////////
//
//  Get_Data_View
//
CDataView * CPhysTestDoc::Get_Data_View(void)
{
    CDataView * view = NULL;

    // Get a pointer to the main window
    CMainFrame * mainwnd = (CMainFrame *)::AfxGetMainWnd();
    
	 if (mainwnd) {
        // Get the pane from the main window
        view = (CDataView *)mainwnd->Get_Pane(0,0);
    }

    // Return a pointer to the tree view
    return view;
}

///////////////////////////////////////////////////////////////
//
//  Get_Graphic_View
//
CGraphicView * CPhysTestDoc::Get_Graphic_View(void)
{
    CGraphicView * view = NULL;

    // Get a pointer to the main window
    CMainFrame * mainwnd = (CMainFrame *)::AfxGetMainWnd();
    
	 if (mainwnd) {
        // Get the pane from the main window
        view = (CGraphicView *)mainwnd->Get_Pane(0,1);
    }

    // Return a pointer to the graphic view
    return view;
}

///////////////////////////////////////////////////////////////
//
//  Add_Physics_Object - add a new object to be simulated
//
void CPhysTestDoc::Add_Physics_Object(PhysClass * obj)
{
	if (obj == NULL) return;
	if (Scene == NULL) return;

	Scene->Add_Dynamic_Object(obj);

	// rebuild the list view
	Get_Data_View()->Rebuild_Tree();
}

int CPhysTestDoc::Get_Physics_Object_Count(void)
{
	PhysListIterator it = Scene->Get_Dynamic_Object_Iterator();

	int count = 0;
	for (it.First(); !it.Is_Done(); it.Next()) {
		count++;
	}
	return count;
}

