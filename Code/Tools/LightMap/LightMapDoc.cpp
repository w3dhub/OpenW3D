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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : LightMap                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tool $* 
 *                                                                                             * 
 *                      $Author:: Ian_l               $* 
 *                                                                                             * 
 *                     $Modtime:: 7/24/01 4:56p       $* 
 *                                                                                             * 
 *                    $Revision:: 65                                                        $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "LightMap.h"
#include "LightMapDoc.h"
#include "MainFrm.h"
#include "StringBuilder.h"
#include "aabtreebuilder.h"
#include "meshmatdesc.h"
#include "ramfile.h"
#include "vector3i.h"
#include "w3d_file.h"
#include "wwmath.h"
#include <stdlib.h>
#include <winbase.h>


// Static data.
const char *LightMapDoc::_TemporarySolveFilename [TEMPORARY_SOLVE_FILENAME_COUNT] = { // Name of temporary file created when a solve is inserted.
	"~S0",
	"~S1"
};

const char *LightMapDoc::_TemporaryOptimizeFilename =	"~O"; // Name of temporary file created when a document is optimized.
const char *LightMapDoc::_TemporaryReorderFilename	 = "~R"; // Name of temporary file created when a document is reordered.

// The following is maintained by MFC tools.
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(LightMapDoc, CDocument)

BEGIN_MESSAGE_MAP(LightMapDoc, CDocument)
	//{{AFX_MSG_MAP(LightMapDoc)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/***********************************************************************************************
 * LightMapDoc::LightMapDoc -- Constructor																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
LightMapDoc::LightMapDoc() :
	W3dFile (NULL),
	MeshCount (0),
	TriangleCount (0),
	CanInsertSolve (false),
	SolveCount (0)
{
	MeshStatus.Set_Growth_Step (512);
	
	// LightMapDoc implicitly references some Surrender objects. Thus Surrender must be initialized here. 
	srInit();

	// Get the solve to delete any assets left over from a previous session.
	LightscapeSolve::Delete_Assets();
}


/***********************************************************************************************
 * LightMapDoc::~LightMapDoc -- Destructor																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
LightMapDoc::~LightMapDoc()
{
	char pathname [_MAX_PATH];
	
	// Clean-up. Delete any temporary files (if they exist).
	for (unsigned f = 0; f < TEMPORARY_SOLVE_FILENAME_COUNT; f++) {
		strcpy (pathname, theApp.Working_Path());
		strcat (pathname, _TemporarySolveFilename [f]);
		strcat (pathname, theApp.Instance_Name());
		DeleteFile (pathname);
	}
	strcpy (pathname, theApp.Working_Path());
	strcat (pathname, _TemporaryOptimizeFilename);
	strcat (pathname, theApp.Instance_Name());
	DeleteFile (pathname);
	strcpy (pathname, theApp.Working_Path());
	strcat (pathname, _TemporaryReorderFilename);
	strcat (pathname, theApp.Instance_Name());
	DeleteFile (pathname);

	// Get the solve to delete its assets.
	LightscapeSolve::Delete_Assets();

	// LightMapDoc initializes Surrender (see constructor). Thus Surrender must be exited here.
	srExit();
}


/***********************************************************************************************
 * LightMapDoc::OnNewDocument --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
BOOL LightMapDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument()) {
		return (false);
	}

	return (true);
}


/***********************************************************************************************
 * LightMapDoc::OnOpenDocument --  Open a document file (but do not read it yet).				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
BOOL LightMapDoc::OnOpenDocument (LPCTSTR pathname)
{
	const char *openedtext    = "Document opened";		// Status bar messages.
	const char *notopenedtext = "Document not opened";

	if (strlen (pathname) > 0) {

		// Prompt for the existing document to be saved.
		if (SaveModified()) {

			CMainFrame *frameptr  = (CMainFrame*) AfxGetApp()->m_pMainWnd;
			CStatusBar *statusptr = &(frameptr->m_wndStatusBar);

			// Delete contents of any previous document.
			DeleteContents();

			// Open a w3d document for read access.
			W3dFile = new RawFileClass (pathname);
			ASSERT (W3dFile != NULL);

			// NOTE: RawFileClass will not keep a copy of the filename unless Set_Name()
			// is explicitly called.
			W3dFile->Set_Name (pathname);
			
			// Check that this is a valid w3d document.
			if (Check_Document()) {

				// Return success.
				statusptr->SetPaneText (0, openedtext);
			
				// Make a copy of the name of the file for later reference.
				_splitpath (pathname, NULL, NULL, DocumentName, NULL);

				return (true);

			} else {

				// Clean-up.
				delete W3dFile;
				W3dFile = NULL;

				// Return failure.
				statusptr->SetPaneText (0, notopenedtext);
				return (false);
			}

		} else {
			
			// Return failure.
			return (false);
		}
	
	} else {

		// Return failure.
		return (false);
	}
}


/***********************************************************************************************
 * LightMapDoc::Check_Document -- Is the current open document a valid w3d file?					  *
 *																															  *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *	
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
bool LightMapDoc::Check_Document()
{
	ASSERT (W3dFile != NULL);
	W3dFile->Open (FileClass::READ);

	ChunkLoadClass w3dchunk (W3dFile);

	try {

		unsigned meshcount;
		unsigned anomalycount;
		unsigned trianglecount;

		// While there are chunks...
		meshcount = 0;
		while (w3dchunk.Open_Chunk()) {

			// Is this a mesh chunk?
			if (w3dchunk.Cur_Chunk_ID() == W3D_CHUNK_MESH) {

				W3dMeshHeader3Struct	header;
				unsigned					chunkmeshheader3count  = 0;

				// If the first chunk is not the mesh header then the mesh is invalid/corrupt.
				w3dchunk.Open_Chunk();
				if (w3dchunk.Cur_Chunk_ID() != W3D_CHUNK_MESH_HEADER3) throw ("A mesh does not start with a valid mesh header (W3D_CHUNK_MESH_HEADER3).");
				if (w3dchunk.Read (&header, sizeof (header)) != sizeof (header)) throw ("Cannot read data in W3D_CHUNK_MESH_HEADER3.");
				if (header.Version != W3D_CURRENT_MESH_VERSION) throw ("An unsupported mesh version has been found.");

				chunkmeshheader3count++;
				w3dchunk.Close_Chunk();

				while (w3dchunk.Open_Chunk()) {
				
					switch (w3dchunk.Cur_Chunk_ID()) {

						case W3D_CHUNK_MESH_HEADER3:
							chunkmeshheader3count++;
							break;
			
						default:
							
							// Do nothing.
							break;
					}
					w3dchunk.Close_Chunk();
				}
				
				// Check for missing/too many header chunks.
				if (chunkmeshheader3count != 1)	throw ("A mesh does not contain exactly one chunk W3D_CHUNK_MESH_HEADER3.");

				meshcount++;
			}
			w3dchunk.Close_Chunk();
		}

		// While there are chunks...
		W3dFile->Seek (0, SEEK_SET);
		anomalycount  = 0;
		trianglecount = 0;
		while (w3dchunk.Open_Chunk()) {

			// Is this a mesh chunk?
			if (w3dchunk.Cur_Chunk_ID() == W3D_CHUNK_MESH) {

				char *meshfilebuffer;

				meshfilebuffer = new char [w3dchunk.Cur_Chunk_Length()];
				ASSERT (meshfilebuffer != NULL);
				if (w3dchunk.Read (meshfilebuffer, w3dchunk.Cur_Chunk_Length()) != w3dchunk.Cur_Chunk_Length()) throw ("Cannot read data in W3D_CHUNK_MESH.");

				RAMFileClass		meshfile (meshfilebuffer, w3dchunk.Cur_Chunk_Length());
				MeshInfoStruct		meshinfo (meshfile);
				MeshAnomalyStruct	meshanomaly (meshinfo);
				MeshStatusStruct	meshstatus (meshinfo.Header.MeshName, meshinfo.Header.NumTris, meshanomaly.MeshAnomalies);

				if (meshanomaly.MeshAnomalies != 0) anomalycount++;
				trianglecount += meshstatus.TriangleCount;
				MeshStatus.Add (meshstatus);
			  
				// A solve can be inserted if a vertex solve and/or a lightmap solve can be inserted.
				CanInsertSolve |= (meshstatus.Can_Insert_Vertex_Solve() || meshstatus.Can_Insert_Multi_Pass_Solve() || meshstatus.Can_Insert_Multi_Texture_Solve());

				delete [] meshfilebuffer;
			}
			w3dchunk.Close_Chunk();
		}

		W3dFile->Close();
		MeshCount	  = meshcount;
		TriangleCount = trianglecount;

		if (anomalycount > 0) {
			
			const char *anomalytext	= 
"%d meshes have mesh anomalies associated with them. If the anomaly is serious, a\
 vertex or lightmap solve cannot be inserted. Check each mesh for specific anomalies\
 by left-clicking on the mesh name in the list.";

			StringBuilder anomalymessage (8192);

			anomalymessage.Copy (anomalytext, anomalycount);
			MessageBox (NULL, anomalymessage.String(), "Attention", MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		}
		
		return (true);

	} catch (const char *message) {

		const char *invalidprefix = "This w3d file is not valid because:\n\n";

		StringBuilder invalidmessage (8192);

		// Clean-up.
		MeshStatus.Clear();
		W3dFile->Close();
		
		// Report to user why document cannot be opened.
		invalidmessage.Copy (invalidprefix);
		invalidmessage.Concatenate (message);
		MessageBox (NULL, invalidmessage.String(), NULL, MB_ICONERROR | MB_SETFOREGROUND);

		return (false);
	}
}


/***********************************************************************************************
 * MeshInfoStruct::MeshInfoStruct --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
LightMapDoc::MeshInfoStruct::MeshInfoStruct (FileClass &meshfile)
{
	ChunkLoadClass	w3dchunk (&meshfile);

	unsigned s, materialpass;

	// Initialize the member fields.
	memset (&Header, 0x0, sizeof (Header));
	TriangleChunk = NULL;
	VertexChunk	  = NULL;
	for (s = 0; s < PrelitModeEnum::COUNT; s++) {
		memset (&MaterialInfo [s], 0x0, sizeof (MaterialInfo [s]));
		VertexMaterials [s] = NULL; 
		ShaderChunk	[s]	  = NULL;
		for (unsigned p = 0; p < MeshMatDescClass::MAX_PASSES; p++) {
			ShaderIdChunk [s][p]	= NULL;
		}
	}	
	DIGsExist			= false;	
	SCGsExist			= false;
	DeformExists		= false;
	IsMultiStage		= false;
	VertexColorsExist = false;

	meshfile.Open (FileClass::READ);

	try {
	
		materialpass = 0;
		while (w3dchunk.Open_Chunk()) {

			switch (w3dchunk.Cur_Chunk_ID()) {

				case W3D_CHUNK_MESH_HEADER3:
					if (w3dchunk.Read (&Header, sizeof (Header)) != sizeof (Header)) {
						throw ("Cannot read data in W3D_CHUNK_MESH_HEADER3.");
					}
					break;

				case W3D_CHUNK_MATERIAL_INFO:
					if (w3dchunk.Read (&MaterialInfo [UNLIT], sizeof (MaterialInfo [UNLIT])) != sizeof (MaterialInfo [UNLIT])) {
						throw ("Cannot read data in W3D_CHUNK_MATERIAL_INFO.");
					}
					break;

				case W3D_CHUNK_TRIANGLES:
					if (TriangleChunk == NULL) {
						TriangleChunk = new ChunkClass (w3dchunk);
						ASSERT (TriangleChunk != NULL);
					}
					break;

				case W3D_CHUNK_VERTICES:
					if (VertexChunk == NULL) {
						VertexChunk = new ChunkClass (w3dchunk);
						ASSERT (VertexChunk != NULL);
					}
					break;

				case W3D_CHUNK_VERTEX_MATERIALS:
					Parse_Vertex_Materials (w3dchunk, PrelitModeEnum::UNLIT);
		  			break;

				case W3D_CHUNK_SHADERS:
					if (ShaderChunk [PrelitModeEnum::UNLIT] == NULL) {
						ShaderChunk [PrelitModeEnum::UNLIT] = new ChunkClass (w3dchunk);
						ASSERT (ShaderChunk [PrelitModeEnum::UNLIT] != NULL);
					}
					break;

				case W3D_CHUNK_MATERIAL_PASS:
					Parse_Material_Pass (w3dchunk, PrelitModeEnum::UNLIT, materialpass);
					materialpass++;
					break;

				case W3D_CHUNK_PRELIT_UNLIT:
					Parse_Prelit_Chunk (w3dchunk, PrelitModeEnum::UNLIT);
					break;

				case W3D_CHUNK_PRELIT_VERTEX:
					Parse_Prelit_Chunk (w3dchunk, PrelitModeEnum::VERTEX);
					break;

				case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS:
					Parse_Prelit_Chunk (w3dchunk, PrelitModeEnum::MULTI_PASS);
					break;

				case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE:
					Parse_Prelit_Chunk (w3dchunk, PrelitModeEnum::MULTI_TEXTURE);
					break;

				case W3D_CHUNK_DEFORM:
					DeformExists = true;
					break;

				default:
					
					// Do nothing.
					break;
			}
			w3dchunk.Close_Chunk();
		}

 		// Clean-up.
		meshfile.Close();

	} catch (const char *message)	{

		// Clean-up.
		meshfile.Close();

  		// Throw to enclosing catch.
		throw (message);
	}
}


/***********************************************************************************************
 * MeshInfoStruct::Parse_Vertex_Materials --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::MeshInfoStruct::Parse_Vertex_Materials (ChunkLoadClass &w3dchunk, unsigned prelitmode)
{
	if (VertexMaterials [prelitmode] == NULL) {

		W3dVertexMaterialStruct vertexmaterialinfo;

		VertexMaterials [prelitmode] = new DynamicVectorClass <W3dVertexMaterialStruct>;
		ASSERT (VertexMaterials [prelitmode] != NULL);
		while (w3dchunk.Open_Chunk()) {

			while (w3dchunk.Open_Chunk()) {
					
				switch (w3dchunk.Cur_Chunk_ID()) {

					case W3D_CHUNK_VERTEX_MATERIAL_NAME:
					case W3D_CHUNK_VERTEX_MAPPER_ARGS0:
					case W3D_CHUNK_VERTEX_MAPPER_ARGS1:
						break;
					
					case W3D_CHUNK_VERTEX_MATERIAL_INFO:
						if (w3dchunk.Read (&vertexmaterialinfo, sizeof (vertexmaterialinfo)) != sizeof (vertexmaterialinfo)) {
							throw ("Cannot read data in W3D_CHUNK_VERTEX_MATERIAL_INFO.");
						}
						VertexMaterials [prelitmode]->Add (vertexmaterialinfo);
						break;
						
					default:
										
						// Do nothing.
						break;
				}
				w3dchunk.Close_Chunk();
			}
			w3dchunk.Close_Chunk();
		}
	}
}


/***********************************************************************************************
 * MeshInfoStruct::Parse_Material_Pass --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/28/00    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::MeshInfoStruct::Parse_Material_Pass (ChunkLoadClass &w3dchunk, unsigned prelitmode, unsigned materialpass)
{
	unsigned chunkstagecount;

	// While there are material chunks... 
	chunkstagecount = 0;
	while (w3dchunk.Open_Chunk()) {
							
		switch (w3dchunk.Cur_Chunk_ID()) {
							
			case W3D_CHUNK_SHADER_IDS:
				if ((materialpass < MeshMatDescClass::MAX_PASSES) && (ShaderIdChunk [prelitmode][materialpass] == NULL)) {
					ShaderIdChunk [prelitmode][materialpass] = new ChunkClass (w3dchunk);
					ASSERT (ShaderIdChunk [prelitmode][materialpass] != NULL);
				}
				break;

			case W3D_CHUNK_SCG:
				
				// Flag that an SCG chunk has been found.
				SCGsExist = true;
				break;
			
			case W3D_CHUNK_DCG:
			{	
				// Scan the chunk for color information and flag appropriately.
				ChunkClass	  *dcgschunk;
				W3dRGBAStruct *dcgs;

				dcgschunk = new ChunkClass (w3dchunk);
				ASSERT (dcgschunk != NULL);

				dcgs = (W3dRGBAStruct*) dcgschunk->Get_Data();
				for (unsigned v = 0; v < dcgschunk->Get_Size() / sizeof (*dcgs); v++) {
					if ((dcgs [v].R != UCHAR_MAX) ||
						 (dcgs [v].G != UCHAR_MAX) ||
						 (dcgs [v].B != UCHAR_MAX)) {
					
						VertexColorsExist = true;
					}
				}
				
				delete dcgschunk;
				break;
			}

			case W3D_CHUNK_DIG:

				// Flag that a DIG chunk has been found.
				DIGsExist = true;
				break;

			case W3D_CHUNK_TEXTURE_STAGE:
				chunkstagecount++;
				break;

			default:

				// Do nothing.
				break;
		}
		w3dchunk.Close_Chunk();
	}

	// Flag if more than one texture stage exists on this material pass.
	if (chunkstagecount > 1) IsMultiStage = true;
}
	

/***********************************************************************************************
 * MeshInfoStruct::Parse_Prelit_Chunk --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/28/00    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::MeshInfoStruct::Parse_Prelit_Chunk (ChunkLoadClass &w3dchunk, unsigned prelitmode)
{
	unsigned materialpass;

	materialpass = 0;
	while (w3dchunk.Open_Chunk()) {

		switch (w3dchunk.Cur_Chunk_ID()) {

		 	case W3D_CHUNK_MATERIAL_INFO:
				if (w3dchunk.Read (&MaterialInfo [prelitmode], sizeof (MaterialInfo [prelitmode])) != sizeof (MaterialInfo [prelitmode])) {
					throw ("Cannot read data in W3D_CHUNK_MATERIAL_INFO.");
				}
				break;

			case W3D_CHUNK_VERTEX_MATERIALS:
				Parse_Vertex_Materials (w3dchunk, prelitmode);
		  		break;

			case W3D_CHUNK_SHADERS:
				if (ShaderChunk [prelitmode] == NULL) {
					ShaderChunk [prelitmode] = new ChunkClass (w3dchunk);
					ASSERT (ShaderChunk [prelitmode] != NULL);
				}
				break;

			case W3D_CHUNK_MATERIAL_PASS:
				Parse_Material_Pass (w3dchunk, prelitmode, materialpass);
				materialpass++;
				break;

			default:

				// Do nothing.
				break;

		}
		w3dchunk.Close_Chunk();
	}
}


/***********************************************************************************************
 * MeshInfoStruct::~MeshInfoStruct --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
LightMapDoc::MeshInfoStruct::~MeshInfoStruct()
{
	unsigned s;

	if (TriangleChunk != NULL) delete TriangleChunk;
	if (VertexChunk != NULL) delete VertexChunk;
	for (s = 0; s < PrelitModeEnum::COUNT; s++) {
		if (VertexMaterials [s] != NULL) delete VertexMaterials [s];
		if (ShaderChunk [s] != NULL) delete ShaderChunk [s];
		for (unsigned p = 0; p < MeshMatDescClass::MAX_PASSES; p++) {
			if (ShaderIdChunk [s][p] != NULL) delete ShaderIdChunk [s][p];
		}
	}
}


/***********************************************************************************************
 * MeshAnomalyStruct::MeshAnomalyStruct --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
LightMapDoc::MeshAnomalyStruct::MeshAnomalyStruct (const MeshInfoStruct &meshinfo)
{
	const W3dRGBStruct black ((uint8) 0x00, (uint8) 0x00, (uint8) 0x00);
	const W3dRGBStruct white ((uint8) 0xff, (uint8) 0xff, (uint8) 0xff);

	// Initialize the member fields.
	MeshAnomalies = 0;

	// Is the mesh hidden (not rendered)?
	if (meshinfo.Header.Attributes & W3D_MESH_FLAG_HIDDEN) MeshAnomalies |= (1 << MESH_HIDDEN);

	// Has this mesh already been prelit?
	if (meshinfo.Header.Attributes & W3D_MESH_FLAG_PRELIT_MASK) MeshAnomalies |= (1 << MESH_PRELIT);

	// Does mesh have a hierarchy?
	if ((meshinfo.Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) == W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN) MeshAnomalies |= (1 << MESH_HAS_HIERARCHY);

	// Is this mesh 2-sided?
	if (meshinfo.Header.Attributes & W3D_MESH_FLAG_TWO_SIDED) MeshAnomalies |= (1 << MESH_TWO_SIDED);

	// Are there any vertex materials?
	if (meshinfo.MaterialInfo [PrelitModeEnum::UNLIT].VertexMaterialCount == 0) MeshAnomalies |= (1 << MESH_NO_VERTEX_MATERIALS);

	// Are there any shaders?
	if (meshinfo.MaterialInfo [PrelitModeEnum::UNLIT].ShaderCount == 0) MeshAnomalies |= (1 << MESH_NO_SHADERS);

	// Is there a deform chunk?
	if (meshinfo.DeformExists) MeshAnomalies |= (1 << MESH_W3D_CHUNK_DEFORM);

	// Are there too many existing material passes?
	if (meshinfo.MaterialInfo [PrelitModeEnum::UNLIT].PassCount > MeshMatDescClass::MAX_PASSES - 1) MeshAnomalies |= (1 << MESH_TOO_MANY_PASSES);

	// Determine if one or more passes has too many texture stages.
	if (meshinfo.IsMultiStage) MeshAnomalies |= (1 << MESH_TOO_MANY_STAGES);

	// For each vertex material...
	if (meshinfo.VertexMaterials [PrelitModeEnum::UNLIT] != NULL) {
		for (int vm = 0; vm < meshinfo.VertexMaterials [PrelitModeEnum::UNLIT]->Count(); vm++) {

			//	Test for emissive.
			if ((*meshinfo.VertexMaterials [PrelitModeEnum::UNLIT]) [vm].Emissive != black) MeshAnomalies |= (1 << MESH_HAS_EMISSIVE);

			// Test for specular to diffuse.
			if (((*meshinfo.VertexMaterials [PrelitModeEnum::UNLIT]) [vm].Attributes & W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE) != 0)  MeshAnomalies |= (1 << MESH_SPECULAR_TO_DIFFUSE);
		}
	}

	// Test shaders.
	if (meshinfo.ShaderChunk [PrelitModeEnum::UNLIT] != NULL) {
			
		W3dShaderStruct *shaderptr = (W3dShaderStruct*) meshinfo.ShaderChunk [PrelitModeEnum::UNLIT]->Get_Data();
			
		for (unsigned s = 0; s < meshinfo.ShaderChunk [PrelitModeEnum::UNLIT]->Get_Size() / sizeof (W3dShaderStruct); s++) {
				
			if (shaderptr->PriGradient != W3DSHADER_PRIGRADIENT_MODULATE) MeshAnomalies |= (1 << MESH_NOT_MODULATED_PRIMARY);
			if (shaderptr->SecGradient != W3DSHADER_SECGRADIENT_DISABLE) MeshAnomalies |= (1 << MESH_HAS_SPECULAR);
			if (shaderptr->AlphaTest != W3DSHADER_ALPHATEST_DISABLE) MeshAnomalies |= (1 << MESH_ALPHATEST);
			shaderptr++;
		}	
	}

	// Determine if there is an SCG chunk.
	if (meshinfo.SCGsExist) MeshAnomalies |= (1 << MESH_HAS_SCG);

	// Determine if there is a DIG chunk.
	if (meshinfo.DIGsExist) MeshAnomalies |= (1 << MESH_HAS_DIG);

	// Determine if there is destination blending in pass 0.
	if ((meshinfo.ShaderChunk [PrelitModeEnum::UNLIT] != NULL) && (meshinfo.ShaderIdChunk [PrelitModeEnum::UNLIT] [0] != NULL)) {
		for (unsigned s = 0; s < meshinfo.ShaderIdChunk [PrelitModeEnum::UNLIT] [0]->Get_Size() / sizeof (uint32); s++) {
											
			uint32			  shaderid;
			W3dShaderStruct *shaderptr;

			shaderid  = *((uint32*) meshinfo.ShaderIdChunk [PrelitModeEnum::UNLIT][0]->Get_Data() + s);
			shaderptr = ((W3dShaderStruct*) meshinfo.ShaderChunk [PrelitModeEnum::UNLIT]->Get_Data()) + shaderid;
		
			if (shaderptr->DestBlend != W3DSHADER_DESTBLENDFUNC_ZERO) MeshAnomalies |= (1 << MESH_PASS_ZERO_DESTBLEND);
		}
	}

	// Determine if there is vertex coloring.
	if (meshinfo.VertexColorsExist) MeshAnomalies |= (1 << MESH_VERTEX_COLORING);
}


/***********************************************************************************************
 * SplitVertexInfoStruct::SplitVertexInfoStruct --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/04/01    IML : Created.                                                                * 
 *=============================================================================================*/
LightMapDoc::SplitVertexInfoStruct::SplitVertexInfoStruct (const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve)
{
	const unsigned verticesperface = 3;	// No. of vertices in a triangle.
	const unsigned facecount		 = meshinfo.TriangleChunk->Get_Size() / sizeof (W3dTriStruct);
	const unsigned vertexcount		 = meshinfo.VertexChunk->Get_Size() / sizeof (W3dVectorStruct);
	const unsigned facevertexcount = facecount * verticesperface;										

	W3dTriStruct *triangles;
	unsigned		  worstcasevertexcount;	
	unsigned		  splitvertexindex, facevertexindex, t, v;

	worstcasevertexcount = MAX (vertexcount, facevertexcount);

	// Allocate a remap table of worst case size ie. assume 3 vertices per triangle.
	RemapTable = new uint32 [worstcasevertexcount];
	ASSERT (RemapTable != NULL);

	IndexTable = new uint32 [facevertexcount];
	ASSERT (IndexTable != NULL);

	// Allocate and copy a table of lightmap UV's.
	UVTable = new W3dTexCoordStruct [facevertexcount];
	ASSERT (UVTable != NULL);
	if (meshsolve.Get_Statistics()->Valid_Lightmap_Solve()) {
		for (t = 0; t < facevertexcount; t++) {
			UVTable [t] = meshsolve.Lightmap_UV (t);
		}
	} else {
		for (t = 0; t < facevertexcount; t++) {
			UVTable [t].U = 0.0f;
			UVTable [t].V = 0.0f;
		}
	}

	// Allocate and initialize a table of lightmap UV ptrs of worst case size.
	UVPtrTable = new W3dTexCoordStruct* [worstcasevertexcount];
	ASSERT (UVPtrTable != NULL);
	for (t = 0; t < worstcasevertexcount; t++) {
		UVPtrTable [t] = NULL;
	}

	// Get the base address of the triangle chunk data.
	triangles = (W3dTriStruct*) meshinfo.TriangleChunk->Get_Data();
	ASSERT (triangles != NULL);

	// For each face...
	splitvertexindex = vertexcount;
	facevertexindex = 0;
	for (unsigned f = 0; f < facecount; f++) {
		
		// For each face vertex...
		for (v = 0; v < verticesperface; v++) {

			uint32 vertexindex;

			vertexindex = triangles [f].Vindex [v];
			ASSERT (vertexindex < vertexcount);

			// Has this index not been used before?
			if (UVPtrTable [vertexindex] == NULL) {
				
				RemapTable [vertexindex]	  = vertexindex;
				IndexTable [facevertexindex] = vertexindex;
				UVPtrTable [vertexindex]	  = UVTable + facevertexindex;
					
			} else {

				// Does this vertex reference the same UV coordinate as the vertex that has already been indexed?
				if (*UVPtrTable [vertexindex] == UVTable [facevertexindex]) {
					
					IndexTable [facevertexindex] = vertexindex;

				} else {

					// Split the vertex.
					RemapTable [splitvertexindex] = vertexindex;
					IndexTable [facevertexindex]  = splitvertexindex;
					UVPtrTable [splitvertexindex] = UVTable + facevertexindex;
					splitvertexindex++;
				}
			}

			facevertexindex++;
		}
	}

	// Iterate over the original vertices to see if any were unreferenced by a triangle.
	// NOTE: The existence of these suggests some redundancy in the mesh data!
	for (v = 0; v < vertexcount; v++) {
		
		if (UVPtrTable [v] == NULL) {

			// Remap it to the zeroth vertex in the zeroth triangle.
			RemapTable [v] = RemapTable [IndexTable [0]];
			UVPtrTable [v] = UVPtrTable [IndexTable [0]];
		}
	}

	VertexCount		 = splitvertexindex;
	FaceVertexCount = facevertexcount;
}


/***********************************************************************************************
 * SplitVertexInfoStruct::~SplitVertexInfoStruct --														  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/04/01    IML : Created.                                                                * 
 *=============================================================================================*/
LightMapDoc::SplitVertexInfoStruct::~SplitVertexInfoStruct()
{
	// Clean-up.
	delete [] UVPtrTable;
	delete [] UVTable;
	delete [] IndexTable;
	delete [] RemapTable;
}


/***********************************************************************************************
 * LightMapDoc::Mesh_Anomalies_String --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/02/00    IML : Created.                                                                * 
 *=============================================================================================*/
const char *LightMapDoc::Mesh_Anomalies_String (unsigned meshindex, bool verbose, StringBuilder &string)
{
	static char *_meshtext [2][LightMapDoc::MESH_FLAG_COUNT] = {{
"Hidden",
"Prelit",
"No vertex materials",
"No shaders",
"Hierarchy",
"Not modulated primary",
"Specular",
"Specular->diffuse",
"Deform chunk",
"2-sided",
"Emissive",
"SCG",
"DIG",
"Too many passes",
"Alpha-test",
"Not opaque",
"Too many stages",
"Vertex colors"
},
{
": mesh is flagged as hidden. This mesh will not receive a solve. Is this what you intended?",
": mesh already contains a vertex and/or lightmap solve. Please ensure that you have selected the correct W3D model file.",
": mesh does not contain any vertex materials. A valid mesh must have at least one vertex material.",
": mesh does not contain any shaders. A valid mesh must have at least one shader.",
": mesh contains a hierarchy and cannot be solved. Please export this model from MAX with the 'geometry only' option.",
": mesh contains a shader where the primary gradient is not set to 'modulate'. This is not supported. Please check that the\
 shaders for this mesh have been defined correctly.",
": mesh has a shader where the secondary gradient is set to 'enabled' (specular lighting). Specular lighting cannot be\
calculated for prelit meshes. If you wish to insert a solve for this mesh re-export the model from MAX with the secondary\
gradient disabled.",
": mesh has a vertex material with 'copy specular to diffuse' enabled (specular lighting). Specular lighting cannot be\
 calculated for prelit meshes. If you want to insert a solve for this mesh re-export the model from MAX with this option\
 unchecked.",
": unsupported chunk (DEFORM chunk).",
": mesh is two-sided. A solve cannot be inserted.",
": mesh has a vertex material with an emissive component. A solve cannot be inserted.",
": mesh has one or more SCG chunks. A solve cannot be inserted.",
": mesh has one or more DIG chunks. A vertex solve for this mesh cannot be inserted.",
": mesh has too many existing material passes (maximum supported passes is 3). A multi-pass lightmap solve for this mesh\
cannot be inserted.",
": mesh uses alpha testing in one or more passes. A multi-pass lightmap solve cannot be inserted.",
": mesh has destination blending in the zeroth pass. A multi-pass lightmap solve cannot be inserted.",
": mesh has a material pass with more than one texture stage. A multi-texture lightmap solve cannot be inserted.",
": mesh has vertex coloring. Is this what you intended?"
}};
	
	unsigned long meshanomalies = MeshStatus [meshindex].MeshAnomalies;

	if (verbose) {
		string.Copy ("");
		for (unsigned meshflagindex = 0; meshflagindex < MESH_FLAG_COUNT; meshflagindex++) {
			if (meshanomalies & (1 << meshflagindex)) {
				string.Concatenate ("%s%s\r\n\r\n", _meshtext [0][meshflagindex], _meshtext [1][meshflagindex]);
			}
		}
		if (string.String() [0] == '\0') string.Copy ("No mesh anomalies.\r\n\r\n");

	} else {
		string.Copy ("");
		for (unsigned meshflagindex = 0; meshflagindex < LightMapDoc::MESH_FLAG_COUNT; meshflagindex++) {
			if (meshanomalies & (1 << meshflagindex)) {
				if (string.String() [0] != '\0') string.Concatenate (", ");
				string.Concatenate (_meshtext [0][meshflagindex]);
			}
		}
	}

	return (string.String());
}


/***********************************************************************************************
 * LightMapDoc::Solve_Anomalies_String --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/02/00    IML : Created.                                                                * 
 *=============================================================================================*/
const char *LightMapDoc::Solve_Anomalies_String (unsigned meshindex, bool verbose, StringBuilder &string)
{
	static char *_solvetext [2][SolveStatistics::SOLVE_STATISTIC_COUNT] = {{
"Vertices not found",
"No color data", 
"Faces not found",
"No lightmaps",
"Not smooth",
"Ambiguous",
"Degenerate",
"[U]",
"[E]",
"Fill color",
"Placeholder used"
},
{
": %d vertices that exist in the W3D mesh cannot be located in the Lightscape solve\
 file (.ls). These vertices will show up hot pink in the solve. First, ensure that the MAX\
 model that was used to create the .w3d model is identical to that used to create the solve.\
 Second, ensure that the MAX model and the Lightscape model use the same units of measurement.\
 Third, try increasing the Spatial Tolerance in Tools | Options. Fourth, Lightscape may\
 have eliminated some vertices during mesh initiation. Please ensure that the Lightscape\
 Length Tolerance was set to a reasonable value prior to generating the solve.",
": %d vertices in the W3D mesh have no corresponding color data in the Lightscape\
 solve file. A vertex solve for this mesh cannot be inserted. If you wish to insert a vertex\
 solve, ensure that one or more solve files contain a Lightscape radiosty mesh.", 
": %d faces that exist in the W3D mesh cannot be located in the Lightscape solve file (.ls).\
 For these faces, a placeholder texture ('Placeholder.tga') has been substituted for the\
 missing lightmap. To correct this problem, first ensure that the MAX model that was used to\
 create the .w3d model is identical to that used to create the solve. Second, ensure that\
 the MAX model and the Lightscape model use the same units of measurement. Third, Lightscape\
 may have eliminated some small or degenerate faces. Please ensure that the Lightscape\
 Length Tolerance was set to a reasonable value prior to generating the solve.",
": %d faces in the W3D mesh have no associated lightmap in the Lightscape solve file.\
 A lightmap solve for this mesh cannot be inserted. If you want to insert a lightmap solve,\
 ensure that one or more solve files contain lightmaps for this mesh (ie. it must have been\
 processed with the Lightscape 'Mesh to Texture' option).",
": %d vertices in the W3D mesh have not been smoothed. They can only be matched up with\
 vertices in the Lightscape solve that are outside the smoothing angle. This is most likely\
 caused by small or degenerate faces. Save the model and then check this mesh for visual\
 artefacts.",
": %d faces can be matched with more than one face in the Lightscape solve. This is most\
 likely caused by coincident faces (ie. two identical faces at the same location).\
 Although this will not cause any visual artefacts, in the interests of efficiency, you\
 may wish to remove them from the MAX model and re-export the w3d model (the solve does\
 not need to be recalculated).",
": %d faces are degenerate (ie. two or more points are identical). Although this will not\
 cause any visual artefacts, it may cause unpredicatable run-time behaviour. You should\
 remove them from the MAX model and re-export the w3d model (the solve does not need to\
 be recalculated).",
": Diagnostic information only. %d faces are 'undersize'.",
": Diagnostic information only. %d faces have been 'edge matched'.",
": %d lightmaps contained only fill color texels. These texels have been automatically\
 replaced with texel data from the nearest valid texels on this page. If you are not satisfied\
 with the results you may wish to fix the abberrant lightmaps and re-insert the solve.",
 ": %d faces that exist in the W3D mesh cannot be located in the Lightscape solve\
 file (.ls). For these faces, a placeholder lightmap ('Placeholder.tga') has been substituted\
 for the missing lightmap."
}};
	
	SolveStatistics *solvestatisticsptr = &(MeshStatus [meshindex].SolveStatistics);

	if (verbose) {
		string.Copy ("");
		if (Solve_Inserted()) {
			for (unsigned index = 0; index < SolveStatistics::SOLVE_STATISTIC_COUNT; index++) {
				if (solvestatisticsptr->Count [index] > 0) {
					string.Concatenate (_solvetext [0][index]);
					string.Concatenate (_solvetext [1][index], solvestatisticsptr->Count [index]);
					string.Concatenate ("\r\n\r\n");
				}
			}
			if (string.String() [0] == '\0') string.Copy ("No solve anomalies.\r\n\r\n");
		}

	} else {
		string.Copy ("");
		for (unsigned index = 0; index < SolveStatistics::SOLVE_STATISTIC_COUNT; index++) {
			if (solvestatisticsptr->Count [index] > 0) {
				if (string.String() [0] != '\0') string.Concatenate (", ");
				string.Concatenate (_solvetext [0][index]);
			}
		}
	}
	return (string.String());
}


/***********************************************************************************************
 * LightMapDoc::Vertex_Solve_Status_String --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/02/00    IML : Created.                                                                * 
 *=============================================================================================*/
const char *LightMapDoc::Vertex_Solve_Status_String (unsigned meshindex, StringBuilder &string)
{
	static char *_insertedtext [2][2] = {
		{"Cannot insert",	""},
		{"Cannot insert", "Inserted"}
	};

	if (!Solve_Inserted()) {
		string.Copy (_insertedtext [0][MeshStatus [meshindex].Can_Insert_Vertex_Solve()]);
	} else {
		string.Copy (_insertedtext [1][MeshStatus [meshindex].Inserted_Vertex_Solve()]);
	}

	return (string.String());
}


/***********************************************************************************************
 * LightMapDoc::Lightmap_Solve_Status_String --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/02/00    IML : Created.                                                                * 
 *=============================================================================================*/
const char *LightMapDoc::Lightmap_Solve_Status_String (unsigned meshindex, StringBuilder &string)
{
	static char *_insertedtext [2][4] = {
		{"Cannot insert",	"Multi-pass only", "Multi-texture only", ""},
		{"Cannot insert", "Multi-pass inserted", "Multi-texture inserted", "Inserted"}
	};

	unsigned code;

	if (!Solve_Inserted()) {
		code =  (MeshStatus [meshindex].Can_Insert_Multi_Pass_Solve() ? 1 : 0)
				+ (MeshStatus [meshindex].Can_Insert_Multi_Texture_Solve() ? 2 : 0);
		string.Copy (_insertedtext [0][code]);

	} else {
		code =  (MeshStatus [meshindex].Inserted_Multi_Pass_Solve() ? 1 : 0)
				+ (MeshStatus [meshindex].Inserted_Multi_Texture_Solve() ? 2 : 0);
		string.Copy (_insertedtext [1][code]);
	}

	return (string.String());
}


/***********************************************************************************************
 * LightMapDoc::Reorder - reorder the meshes within the document for optimal spatial coherence.*										  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/01    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Reorder()
{
	RawFileClass *reorderfile = NULL;
	
	try {

		DynamicVectorClass <MeshReorderStruct> meshes;

		{
			W3dFile->Open (FileClass::READ);
		
			ChunkLoadClass w3dchunk	(W3dFile);

			// Read the (unordered) mesh chunks.
			while (w3dchunk.Open_Chunk()) {

				// Is this a mesh chunk?
				if (w3dchunk.Cur_Chunk_ID() == W3D_CHUNK_MESH) {

					ChunkClass *chunk = new ChunkClass (w3dchunk);
					
					RAMFileClass		meshfile (chunk->Get_Data(), chunk->Get_Size());
					MeshInfoStruct		meshinfo (meshfile);
					MeshReorderStruct mesh;

					mesh.Chunk = chunk;
					mesh.Position.Set (meshinfo.Header.SphCenter.X, meshinfo.Header.SphCenter.Y, meshinfo.Header.SphCenter.Z);
					meshes.Add (mesh);
				}
				w3dchunk.Close_Chunk();
			}

			W3dFile->Close();
		}

		// If more than one chunk, spatially sort the mesh chunks.
		if (meshes.Count() > 1) {
		
			const float floatmax = 0.5f * (sqrt (FLT_MAX / 3.0f));
			
			int					m;
			float					distance, mindistance;
			Vector3				seed;
			MeshReorderStruct t;

			// Find an extreme position to use as the seed.
			mindistance = FLT_MAX;
			for (m = 0; m < meshes.Count(); m++) {
				distance = meshes [m].Position.X + meshes [m].Position.Y + meshes [m].Position.Z;
				if (distance < mindistance) {
					mindistance = distance;
					seed			= meshes [m].Position;
				}
			}
				
			for (m = 0; m < meshes.Count() - 1; m++) {
				
				float		distancesquared, mindistancesquared;
				unsigned	nearestmeshindex;

	 			mindistancesquared = FLT_MAX;
				for (int n = m; n < meshes.Count(); n++) {
					distancesquared =	(meshes [n].Position - seed).Length2();
					if (distancesquared < mindistancesquared) {
						nearestmeshindex	 = n;	
						mindistancesquared = distancesquared;
					}
				}
				
				// Swap mesh positions.
				t = meshes [m];
				meshes [m] = meshes [nearestmeshindex];
				meshes [nearestmeshindex] = t;

				// If it exists, swap mesh status.
				if (MeshStatus.Count() > 0) {
					
					MeshStatusStruct s;
					
					s = MeshStatus [m];
					MeshStatus [m] = MeshStatus [nearestmeshindex];
					MeshStatus [nearestmeshindex] = s;
				}	

				seed = meshes [m].Position;
			}
		}

		// Write out the reordered mesh chunks.
		{
			char temporarypathname [_MAX_PATH];
			bool outputmeshchunks;

			W3dFile->Open (FileClass::READ);

			// Create a temporary file to contain the results of the solve.
			strcpy (temporarypathname, theApp.Working_Path());
			strcat (temporarypathname, _TemporaryReorderFilename);
			strcat (temporarypathname, theApp.Instance_Name());
			reorderfile = new RawFileClass (temporarypathname);
			ASSERT (reorderfile != NULL);
	
			// NOTE: RawFileClass will not keep a copy of the filename unless Set_Name() is explicitly called.
			reorderfile->Set_Name (temporarypathname);
	
			// NOTE: WRITE only access will truncate any existing temporary file.
			reorderfile->Open (FileClass::WRITE);

			ChunkLoadClass w3dchunk		 (W3dFile);
			ChunkSaveClass reorderchunk (reorderfile);

			outputmeshchunks = false;
			while (w3dchunk.Open_Chunk()) {

				// Is this a mesh chunk?
				if (w3dchunk.Cur_Chunk_ID() == W3D_CHUNK_MESH) {
					if (!outputmeshchunks) {
						for (int m = 0; m < meshes.Count(); m++) {
							reorderchunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
							reorderchunk.Write (meshes [m].Chunk->Get_Data(), meshes [m].Chunk->Get_Size());
							reorderchunk.End_Chunk();

							// Clean-up.
							delete meshes [m].Chunk;
						}
						outputmeshchunks = true;
					}
				} else {
					Copy_Chunk (w3dchunk, reorderchunk);
				}

				w3dchunk.Close_Chunk();
			}
		}

		// Clean-up.
		delete W3dFile;
		reorderfile->Close();
	
		// Make the reorder file the w3d file.
		W3dFile = reorderfile;

	} catch (const char *errormessage) {

		W3dFile->Close();
		if (reorderfile != NULL) delete reorderfile;

		// Throw to caller.
		throw (errormessage);
	}	
}


/***********************************************************************************************
 * LightMapDoc::Optimize -- Simplify the document (if necessary).										  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/01    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Optimize()
{
	RawFileClass *optimizefile = NULL;

	try {

		char temporarypathname [_MAX_PATH];

		W3dFile->Open (FileClass::READ);

		// Create a temporary file to contain the results of the solve.
		strcpy (temporarypathname, theApp.Working_Path());
		strcat (temporarypathname, _TemporaryOptimizeFilename);
		strcat (temporarypathname, theApp.Instance_Name());
		optimizefile = new RawFileClass (temporarypathname);
		ASSERT (optimizefile != NULL);
	
		// NOTE: RawFileClass will not keep a copy of the filename unless Set_Name() is explicitly called.
		optimizefile->Set_Name (temporarypathname);
	
		// NOTE: WRITE only access will truncate any existing temporary file.
		optimizefile->Open (FileClass::WRITE);
	
		ChunkLoadClass w3dchunk		  (W3dFile);
		ChunkSaveClass optimizechunk (optimizefile);

		while (w3dchunk.Open_Chunk()) {

			// Is this a mesh chunk?
			if (w3dchunk.Cur_Chunk_ID() == W3D_CHUNK_MESH) {

				optimizechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
				while (w3dchunk.Open_Chunk()) {
				  
					switch (w3dchunk.Cur_Chunk_ID()) {
						
						case W3D_CHUNK_PRELIT_VERTEX:
							
							optimizechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
							while (w3dchunk.Open_Chunk()) {
								
								switch (w3dchunk.Cur_Chunk_ID()) {
									
									case W3D_CHUNK_MATERIAL_PASS:
										Optimize_Prelit_Vertex_Material_Pass (w3dchunk, optimizechunk);
										break;

									default:
										Copy_Chunk (w3dchunk, optimizechunk);
										break;
								}
								w3dchunk.Close_Chunk();
							}
							optimizechunk.End_Chunk();
							break;

						default:
							Copy_Chunk(w3dchunk, optimizechunk);
							break;
					}
					w3dchunk.Close_Chunk();
				}
				optimizechunk.End_Chunk();
			
			} else {
				Copy_Chunk (w3dchunk, optimizechunk);
			}
			w3dchunk.Close_Chunk();
		}

		// Clean-up.
		delete W3dFile;
		optimizefile->Close();
	
		// Make the optimize file the w3d file.
		W3dFile = optimizefile;

		// Once a document has been optimized, a solve can no longer be inserted because some
		// information has been lost (namely DIG colors).
		CanInsertSolve = false;

	} catch (const char *errormessage) {

		W3dFile->Close();
		if (optimizefile != NULL) delete optimizefile;

		// Throw to caller.
		throw (errormessage);
	}	
}


/***********************************************************************************************
 * LightMapDoc::Optimize_Prelit_Vertex_Material_Pass --													  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/01    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Optimize_Prelit_Vertex_Material_Pass (ChunkLoadClass &w3dchunk, ChunkSaveClass &optimizechunk)
{
	unsigned		i, digcount;
	ChunkClass *dcgschunk;
	ChunkClass *digschunk [MAX_SOLVE_COUNT];

	optimizechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	
	digcount	 = 0;
	dcgschunk = NULL;
	while (w3dchunk.Open_Chunk()) {

		switch (w3dchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_DCG:
				dcgschunk = new ChunkClass (w3dchunk);
				break;

			case W3D_CHUNK_DIG:
				ASSERT (digcount <= MAX_SOLVE_COUNT);
				digschunk [digcount] = new ChunkClass (w3dchunk);
				digcount++;
				break;

			default:
				Copy_Chunk (w3dchunk, optimizechunk);
				break;
		}

		w3dchunk.Close_Chunk();
	}
	
	// For each DIG chunk...
	for (i = 0; i < digcount; i++) {

		W3dRGBStruct  *digs;
		W3dRGBAStruct *dcgs;

		// Copy the DIG chunk into a DCG chunk, blend the DCG chunk (if it exists).
		optimizechunk.Begin_Chunk (W3D_CHUNK_DCG);
			
		digs = (W3dRGBStruct*) digschunk [i]->Get_Data();
		if (dcgschunk != NULL) {
			dcgs = (W3dRGBAStruct*) dcgschunk->Get_Data();
		} else {
			dcgs = NULL;
		}
		for (unsigned v = 0; v < digschunk [i]->Get_Size() / sizeof (*digs); v++) {

			W3dRGBStruct  blendcolor;
			unsigned char alpha;
			W3dRGBAStruct outputcolor;

			blendcolor = digs [v]; 
			if (dcgs != NULL) {
		
				W3dRGBStruct color;
					
				color.Set (dcgs [v].R, dcgs [v].G, dcgs [v].B);
				blendcolor *= color;
				alpha = dcgs [v].A;
			} else {
				alpha = UCHAR_MAX;
			}
				
			outputcolor.R = blendcolor.R;
			outputcolor.G = blendcolor.G;
			outputcolor.B = blendcolor.B;
			outputcolor.A = alpha;

			optimizechunk.Write (&outputcolor, sizeof (outputcolor));
		}	
		
		optimizechunk.End_Chunk();
	}

	// If there were no DIG chunks write the DCG chunk as is (if it exists).
	if ((digcount == 0) && (dcgschunk != NULL)) {
		optimizechunk.Begin_Chunk (W3D_CHUNK_DCG);
		optimizechunk.Write ((W3dRGBAStruct*) dcgschunk->Get_Data(), dcgschunk->Get_Size());
		optimizechunk.End_Chunk();
	}

	optimizechunk.End_Chunk();

	// Clean-up.
	if (dcgschunk != NULL) delete dcgschunk;
	for (i = 0; i < digcount; i++) {
		if (digschunk [i] != NULL) delete digschunk [i];
	}
}
 

/***********************************************************************************************
 * LightMapDoc::OnFileSave --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::OnFileSave() 
{
	OnSaveDocument (GetPathName());
}


/***********************************************************************************************
 * LightMapDoc::OnFileSaveAs --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::OnFileSaveAs() 
{
	CFileDialog dialog (false, LightMapApp::Document_File_Extension(), NULL, LightMapApp::File_Dialog_Flags(), LightMapApp::File_Dialog_Filter());

	if (dialog.DoModal() == IDOK) OnSaveDocument (dialog.GetPathName());
}


/***********************************************************************************************
 * LightMapDoc::OnSaveDocument -- Save the currently open document.									  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
BOOL LightMapDoc::OnSaveDocument (LPCTSTR pathname) 
{
	const char *savingmodeltext  = "Step 1/3: Saving Westwood 3D file (.W3D)";
	const char *savinglightstext = "Step 2/3: Saving Westwood light file (.WLT)";
	const char *savingassetstext = "Step 3/3: Saving lightmaps";
	const char *savedtext		  = "Document saved";
	const char *notsavedtext	  = "Document not saved";

	CMainFrame* frameptr  = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	CStatusBar* statusptr = &(frameptr->m_wndStatusBar);
	
	StringBuilder errormessage (8192);

	try {
	
		ASSERT (pathname != NULL);

		// Inform the user that the save has started.
		CWaitCursor	waitcursor;

		// Optimize the document.
		Optimize();

		// If the filename of the current document and the save document do not match
		// or the document has been modified then the document must be saved.
		if (IsModified() || stricmp (W3dFile->File_Name(), pathname) != 0) {

			char				 drivename [_MAX_DRIVE];
			char				 directoryname [_MAX_DIR];
			char				 filename [_MAX_FNAME];
			unsigned			 filenamesize;
			char				 lightmapdirectory [_MAX_DIR];
			RawFileClass	 savefile (pathname);
			ChunkLoadClass  loadchunk (W3dFile);
			ChunkSaveClass  savechunk (&savefile);
			char				 lightmappathname [_MAX_PATH];	

			// Extract the file name from the path name, check that the length is okay.
			_splitpath (pathname, NULL, NULL, filename, NULL);
			filenamesize = strlen (filename) + 1;
			if (filenamesize > W3D_NAME_LEN) {
				
				const char *controlstring = "File name is too long. Please use %d characters or less.";
				
				throw (errormessage.Copy (controlstring, W3D_NAME_LEN));
			}

			strcpy (lightmapdirectory, filename);

			// Convert filename to uppercase.
			_strupr (filename);

			W3dFile->Open (FileClass::READ);

			// Open a save file with write access.
			if (!savefile.Open (FileClass::WRITE)) throw ("Cannot create file with this file name.");
			
			{
				CProgressCtrl progressbar;
				CRect			  srect, trect;
				BOOL			  success;
		
				statusptr->GetItemRect (0, &srect);
				statusptr->SetPaneText (0, savingmodeltext);
				statusptr->GetDC()->DrawText (savingmodeltext, -1, &trect, DT_CALCRECT);

				CRect	prect (srect.TopLeft().x + trect.Width(), srect.TopLeft().y, srect.BottomRight().x, srect.BottomRight().y);
		
				success = progressbar.Create (WS_CHILD | WS_VISIBLE | PBS_SMOOTH, prect, statusptr, 0);
				ASSERT (success);
				progressbar.SetRange32 (0, MeshCount);
				progressbar.SetStep (1);

				// While there are load chunks...
				while (loadchunk.Open_Chunk()) {
				
					switch (loadchunk.Cur_Chunk_ID()) {

						case W3D_CHUNK_MESH:
							Rename_Mesh	(loadchunk, savechunk, filename, lightmapdirectory);
							progressbar.StepIt();
							break;

						case W3D_CHUNK_COLLECTION:
							Rename_Collection (loadchunk, savechunk, filename);
							break;
						
						case W3D_CHUNK_HLOD:
							Rename_HLOD (loadchunk, savechunk, filename);
							break;

						case W3D_CHUNK_DAZZLE:
							Rename_Dazzle (loadchunk, savechunk, filename);
							break;

						default:

							// Any other chunk.
							Copy_Chunk (loadchunk, savechunk);
							break;
					}
					loadchunk.Close_Chunk();
				}
			}

			// Copy the lightmap assets.
			_splitpath (pathname, drivename, directoryname, NULL, NULL);
			strcpy (lightmappathname, drivename);
			strcat (lightmappathname, directoryname);
			strcat (lightmappathname, lightmapdirectory);
			statusptr->SetPaneText (0, savingassetstext);
			LightscapeSolve::Copy_Assets (lightmappathname);

			// Save the lights.
			statusptr->SetPaneText (0, savinglightstext);
			Save_Lights (pathname);
			
			// Clean-up.
			W3dFile->Close();
			savefile.Close();
		}

		// Return success.
		SetModifiedFlag (false);
		statusptr->SetPaneText (0, savedtext);
		return (true);
	
	} catch (const char *message) {

		// Clean-up.
		// NOTE 0: Can safely call close even if files are not open.
		// NOTE 1: Save file will be closed when it goes out of scope.
		W3dFile->Close();
		
		// Report why save failed to user.
		MessageBox (NULL, message, NULL, MB_ICONERROR | MB_SETFOREGROUND);
		
		// Return failure.
		statusptr->SetPaneText (0, notsavedtext);
		return (false);
	}
}


/***********************************************************************************************
 * LightMapDoc::Rename_Mesh --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Rename_Mesh (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename, const char *lightmapdirectory)
{
	savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
	while (loadchunk.Open_Chunk()) {
		
		switch (loadchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_MESH_HEADER3:
				Rename_Mesh_Header (loadchunk, savechunk, filename);
				break;

			case W3D_CHUNK_TEXTURES:
				Rename_Lightmaps (loadchunk, savechunk, lightmapdirectory);
				break;

			case W3D_CHUNK_PRELIT_UNLIT:
			case W3D_CHUNK_PRELIT_VERTEX:
			case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS:
			case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE:
				Rename_Prelit_Chunks (loadchunk, savechunk, lightmapdirectory);
				break;

			default:
				// Other mesh chunks.
				Copy_Chunk (loadchunk, savechunk);
				break;
		}
		loadchunk.Close_Chunk();
	}
	savechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Rename_Mesh_Header --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Rename_Mesh_Header (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename)
{
	unsigned filenamesize = strlen (filename) + 1;

	W3dMeshHeader3Struct header;
				
	if (loadchunk.Read (&header, loadchunk.Cur_Chunk_Length()) != loadchunk.Cur_Chunk_Length()) {
		throw ("Cannot read data in W3D_CHUNK_MESH_HEADER3.");
	}

	// If there is a container modify the container name, otherwise modify the mesh name. 
	if (strlen (header.ContainerName) > 0) {
		ASSERT (filenamesize <= sizeof (header.ContainerName));
		strcpy (header.ContainerName, filename);
	} else {
		ASSERT (filenamesize <= sizeof (header.MeshName));
		strcpy (header.MeshName, filename);
	}

	savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
	savechunk.Write (&header, sizeof (header));
	savechunk.End_Chunk();
}	


/***********************************************************************************************
 * LightMapDoc::Rename_Prelit_Chunks --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/02/00    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Rename_Prelit_Chunks (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *lightmapdirectory)
{
	savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
	while (loadchunk.Open_Chunk()) {
		
		switch (loadchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_TEXTURES:
				Rename_Lightmaps (loadchunk, savechunk, lightmapdirectory);
				break;

			default:
				// Other mesh chunks.
				Copy_Chunk (loadchunk, savechunk);
				break;
		}
		loadchunk.Close_Chunk();
	}
	savechunk.End_Chunk();
}	


/***********************************************************************************************
 * LightMapDoc::Rename_Lightmaps --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Rename_Lightmaps (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *lightmapdirectory)
{
	savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
	while (loadchunk.Open_Chunk()) {
					
		savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
		while (loadchunk.Open_Chunk()) {
			
			switch (loadchunk.Cur_Chunk_ID()) {

				case W3D_CHUNK_TEXTURE_NAME:
				{	
					ChunkClass *loadpathchunk;
					char			loaddrivename [_MAX_DRIVE];
					char			loaddirectoryname [_MAX_DIR];
					char			loadfilename [_MAX_FNAME];
					char			loadextname [_MAX_EXT];
					char			pathname [_MAX_PATH];

					loadpathchunk = new ChunkClass (loadchunk);
					ASSERT (loadpathchunk != NULL);
	
					// Break the load pathname into its component parts.
					_splitpath ((const char*) loadpathchunk->Get_Data(), loaddrivename, loaddirectoryname, loadfilename, loadextname);

					// If the directory component matches that of the existing lightmap assets
					// then a lightmap is being references - modify the path to match the destination.
					// Otherwise leave it unmodified.
					// NOTE: Ignore case sensitivity in the directory name comparison.
					savechunk.Begin_Chunk (loadpathchunk->ChunkType);
					if ((strlen (loaddrivename) == 0) && (stricmp (loaddirectoryname, LightscapeSolve::Asset_Directory()) == 0)) {
						strcpy (pathname, LightscapeSolve::Asset_Directory (lightmapdirectory));
						strcat (pathname, loadfilename);
						strcat (pathname, loadextname);
						savechunk.Write (pathname, strlen (pathname) + 1);
					} else {
						savechunk.Write (loadpathchunk->Get_Data(), loadpathchunk->Get_Size());
					}
					savechunk.End_Chunk();
					delete loadpathchunk;
					break;
				}

				default:
				
					// Copy other texture chunks unmodified.
					Copy_Chunk (loadchunk, savechunk);
					break;

			}
			loadchunk.Close_Chunk();
		}
		loadchunk.Close_Chunk();
		savechunk.End_Chunk();
	}
	savechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Rename_Collection --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Rename_Collection (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename)
{
	unsigned filenamesize = strlen (filename) + 1;

	savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
	
	while (loadchunk.Open_Chunk()) {
			
		switch (loadchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_COLLECTION_HEADER:
			{
				W3dCollectionHeaderStruct header;

				ASSERT (filenamesize <= sizeof (header.Name));
				
				if (loadchunk.Read (&header, loadchunk.Cur_Chunk_Length()) != loadchunk.Cur_Chunk_Length()) {
					throw ("Cannot read data in W3D_CHUNK_COLLECTION_HEADER.");
				}
				strcpy (header.Name, filename);
				savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
				savechunk.Write (&header, sizeof (header));
				savechunk.End_Chunk();
				break;
			}

			case W3D_CHUNK_COLLECTION_OBJ_NAME:

				savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
				if (loadchunk.Cur_Chunk_Length() > 0) {

					char *loadname, *savename, *suffix;

					loadname = new char [loadchunk.Cur_Chunk_Length()];
					ASSERT (loadname != NULL);

					// Allocate a string long enough to hold the save string.
					savename = new char [loadchunk.Cur_Chunk_Length() + filenamesize];
					ASSERT (savename != NULL);

					if (loadchunk.Read (loadname, loadchunk.Cur_Chunk_Length()) != loadchunk.Cur_Chunk_Length()) {
						throw ("Cannot read data in W3D_CHUNK_COLLECTION_OBJ_NAME.");
					}

					strcpy (savename, filename);
					suffix = strrchr (loadname, '.');
					if (suffix != NULL) {
						strcat (savename, suffix);
					}

					savechunk.Write (savename, strlen (savename) + 1);

					// Clean-up.
					delete [] savename;
					delete [] loadname;
				}
				savechunk.End_Chunk();
				break;

			default:
				
				// Other collection chunks.
				Copy_Chunk (loadchunk, savechunk);
				break;
		}
		loadchunk.Close_Chunk();
	}
	
	savechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Rename_HLOD --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Rename_HLOD (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename)
{
	unsigned filenamesize = strlen (filename) + 1;

	savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
	while (loadchunk.Open_Chunk()) {
			
		switch (loadchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_HLOD_HEADER:
			{
				W3dHLodHeaderStruct header;

				ASSERT (filenamesize <= sizeof (header.Name));
				
				if (loadchunk.Read (&header, loadchunk.Cur_Chunk_Length()) != loadchunk.Cur_Chunk_Length()) {
					throw ("Cannot read data in W3D_CHUNK_HLOD_HEADER.");
				}
				strcpy (header.Name, filename);
				savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
				savechunk.Write (&header, sizeof (header));
				savechunk.End_Chunk();
				break;
			}

			case W3D_CHUNK_HLOD_LOD_ARRAY:

				savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
				while (loadchunk.Open_Chunk()) {

					switch (loadchunk.Cur_Chunk_ID()) {

						case W3D_CHUNK_HLOD_SUB_OBJECT:
						{
							W3dHLodSubObjectStruct  subobject;
							char						   subobjectname [sizeof (subobject.Name)];
							char						  *suffix;	

							if (loadchunk.Read (&subobject, loadchunk.Cur_Chunk_Length()) != loadchunk.Cur_Chunk_Length()) {
								throw ("Cannot read data in W3D_CHUNK_HLOD_LOD_ARRAY.");
							}
							
							ASSERT (filenamesize <= sizeof (subobjectname));
							strcpy (subobjectname, filename);
							suffix = strrchr (subobject.Name, '.');
							if (suffix != NULL) {
								ASSERT ((filenamesize + strlen (suffix)) <= sizeof (subobjectname));
								strcat (subobjectname, suffix);
							}
							strcpy (subobject.Name, subobjectname);

							savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
							savechunk.Write (&subobject, sizeof (subobject));
							savechunk.End_Chunk();
							break;
						}

						default:
							
							// Other collection chunks.
							Copy_Chunk (loadchunk, savechunk);
							break;
					}
					loadchunk.Close_Chunk();
				}
				savechunk.End_Chunk();
				break;

			default:
			  
				// Other collection chunks.
				Copy_Chunk (loadchunk, savechunk);
				break;
		}
		loadchunk.Close_Chunk();
	}
	savechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Rename_Dazzle --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Rename_Dazzle (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename)
{
	unsigned filenamesize = strlen (filename) + 1;

	savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
	
	while (loadchunk.Open_Chunk()) {
			
		switch (loadchunk.Cur_Chunk_ID()) {

	  		case W3D_CHUNK_DAZZLE_NAME:
			{
				savechunk.Begin_Chunk (loadchunk.Cur_Chunk_ID());
				if (loadchunk.Cur_Chunk_Length() > 0) {

					char *loadname, *savename, *suffix;

					loadname = new char [loadchunk.Cur_Chunk_Length()];
					ASSERT (loadname != NULL);

					// Allocate a string long enough to hold the save string.
					savename = new char [loadchunk.Cur_Chunk_Length() + filenamesize];
					ASSERT (savename != NULL);

					if (loadchunk.Read (loadname, loadchunk.Cur_Chunk_Length()) != loadchunk.Cur_Chunk_Length()) {
						throw ("Cannot read data in W3D_CHUNK_COLLECTION_OBJ_NAME.");
					}

					strcpy (savename, filename);
					suffix = strrchr (loadname, '.');
					if (suffix != NULL) {
						strcat (savename, suffix);
					}

					savechunk.Write (savename, strlen (savename) + 1);

					// Clean-up.
					delete [] savename;
					delete [] loadname;
				}
				savechunk.End_Chunk();
				break;
			}

			default:
				// Other dazzle chunks.
				Copy_Chunk (loadchunk, savechunk);
				break;
		}
		loadchunk.Close_Chunk();
	}

	savechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Save_Lights --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *  07/04/00    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightMapDoc::Save_Lights (const char *pathname)
{
	const char *extension = ".wlt";		// Extension for Westwood light file.

	char lightpathname [_MAX_PATH];
	char drivename [_MAX_DRIVE];
	char directoryname [_MAX_DIR];
	char filename [_MAX_FNAME];

	_splitpath (pathname, drivename, directoryname, filename, NULL);
	strcpy (lightpathname, drivename);
	strcat (lightpathname, directoryname);
	strcat (lightpathname, filename);
	strcat (lightpathname, extension);
		
	RawFileClass *lightfile = new RawFileClass (lightpathname);
	ASSERT (lightfile != NULL);

	// NOTE: RawFileClass will not keep a copy of the filename unless Set_Name() is explicitly called.
	lightfile->Set_Name (lightpathname);

	// NOTE: WRITE only access will truncate any existing temporary file.
	lightfile->Open (FileClass::WRITE);

	ChunkSaveClass lightchunk (lightfile);
			
	for (unsigned s = 0; s < MAX_SOLVE_COUNT; s++) {

		lightchunk.Begin_Chunk (W3D_CHUNK_LIGHTSCAPE);

		// For each light...
		for (int l = 0; l < Lights [s].Count(); l++) {
					
			const Matrix3D						t			 = Lights [s][l]->Get_Transform();
			const W3dLightTransformStruct transform = {t [0][0], t [0][1], t [0][2], t [0][3],
																	 t [1][0], t [1][1], t [1][2], t [1][3],
																	 t [2][0], t [2][1], t [2][2], t [2][3]};

			// Save light attributes.
			lightchunk.Begin_Chunk (W3D_CHUNK_LIGHTSCAPE_LIGHT);

			Lights [s][l]->Save_W3D (lightchunk);

			// Save light position and orientation.
			lightchunk.Begin_Chunk (W3D_CHUNK_LIGHT_TRANSFORM);
			lightchunk.Write (&transform, sizeof (transform));
			lightchunk.End_Chunk();

			lightchunk.End_Chunk();
		}
		lightchunk.End_Chunk();
	}
	lightfile->Close();

	// Clean-up.
	delete lightfile;
}


/***********************************************************************************************
 * LightMapDoc::DeleteContents -- Delete the document's data (close the document file).		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::DeleteContents() 
{
	// Get the solve to delete its assets.
	LightscapeSolve::Delete_Assets();

	// Close any previously opened document.
	if (W3dFile != NULL) {
		delete W3dFile;
		W3dFile = NULL;
	}

	// Re-initialize data.
	MeshCount			= 0;
	TriangleCount		= 0;	
	CanInsertSolve		= false;
	SolveCount			= 0;
	MeshStatus.Clear();

	// Release references to lights.
	for (unsigned s = 0; s < MAX_SOLVE_COUNT; s++) {
		for (int l = 0; l < Lights [s].Count(); l++) {
			Lights [s][l]->Release_Ref();
		}
		Lights [s].Clear();
	}
}


/***********************************************************************************************
 * LightMapDoc::OnUpdateFileSave --	Is the 'File->Save' menu option selectable?					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::OnUpdateFileSave (CCmdUI* cmdui) 
{
	cmdui->Enable (Is_Open() && IsModified());
}


/***********************************************************************************************
 * LightMapDoc::OnUpdateFileSaveAs -- Is the 'File->Save As...' menu option selectable?		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::OnUpdateFileSaveAs (CCmdUI* cmdui) 
{
	cmdui->Enable (Is_Open());
}


/***********************************************************************************************
 * LightMapDoc::Insert_Solve -- Add a lightmap solve to the current open document file.		  *
 *																															  *
 *	Add a lightmap pass to the current open w3d file using the supplied solvepathname as the	  *
 * solve file. For each chunk translate as appropriate, writing the results to a	temporary	  *
 * file. If the temporary file is successfully created close the current document and make the *
 * temporary file the current open document.																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Insert_Solve (const char *solvedirectoryname, const char *solvefilenamelist, const char *inclusionstring, bool invertselection, bool blendnoise)
{
	const char *buildingtext	  = "Step 1/2: Building solve database";	// Status bar message.
	const char *insertingtext	  = "Step 2/2: Inserting solve";				// Status bar message.
	const char *solvetext		  = "Solve #";										// Status bar message.
	const char *insertedtext	  = " inserted";									// Status bar message.
	const char *notinsertedtext  = " not inserted";								// Status bar message.

	char  temporarypathname [_MAX_PATH];
	char *meshfilebuffer = NULL;

	ASSERT (CanInsertSolve);
	ASSERT (SolveCount < MAX_SOLVE_COUNT);

	CMainFrame* frameptr		= (CMainFrame*) AfxGetApp()->m_pMainWnd;
	CStatusBar* statusptr	= &(frameptr->m_wndStatusBar);
	RawFileClass *solvefile = NULL;

	try {

		// Reorder the meshes within the document.
		Reorder();

		// Inform the user that the insertion process has started.
		CWaitCursor	waitcursor;

		W3dFile->Open (FileClass::READ);

		// Create a temporary file to contain the results of the solve.
		strcpy (temporarypathname, theApp.Working_Path());
		strcat (temporarypathname, _TemporarySolveFilename [SolveCount % TEMPORARY_SOLVE_FILENAME_COUNT]);
		strcat (temporarypathname, theApp.Instance_Name());
		solvefile = new RawFileClass (temporarypathname);
		ASSERT (solvefile != NULL);

		// NOTE: RawFileClass will not keep a copy of the filename unless Set_Name() is explicitly called.
		solvefile->Set_Name (temporarypathname);
		
		// NOTE: WRITE only access will truncate any existing temporary file.
		solvefile->Open (FileClass::WRITE);

		ChunkLoadClass w3dchunk   (W3dFile);
		ChunkSaveClass solvechunk (solvefile);
		CProgressCtrl  progressbar;
		CRect			   srect, trect;
		BOOL			   success;
		unsigned		   meshindex;	
		unsigned		   anomalycount;	

		// Create the solve for the entire model from the solution file(s).
		LightscapeSolve solve (solvedirectoryname, solvefilenamelist, statusptr, buildingtext, blendnoise);

		statusptr->GetItemRect (0, &srect);
		statusptr->SetPaneText (0, insertingtext);
		statusptr->GetDC()->DrawText (insertingtext, -1, &trect, DT_CALCRECT);

		CRect	prect (srect.TopLeft().x + trect.Width(), srect.TopLeft().y, srect.BottomRight().x, srect.BottomRight().y);
	
		success = progressbar.Create (WS_CHILD | WS_VISIBLE | PBS_SMOOTH, prect, statusptr, 0);
		ASSERT (success);
		progressbar.SetRange32 (0, TriangleCount);

		// While there are w3d chunks...
		meshindex = 0;
		while (w3dchunk.Open_Chunk()) {

			// Is this a mesh chunk?
			if (w3dchunk.Cur_Chunk_ID() == W3D_CHUNK_MESH) {
				
				bool includemesh;

				// Currently, nothing has been inserted.
				unsigned long insertedflags = 0;

				// If this mesh name contains the inclusion character (if not null) and if a vertex solve and/or a lightmap solve can be inserted...
				if (inclusionstring == NULL) {
					includemesh = true;
				} else {
					includemesh = strstr (MeshStatus [meshindex].Name, inclusionstring) != NULL;
					if (invertselection) includemesh = !includemesh;
				}
				if (includemesh && (MeshStatus [meshindex].Can_Insert_Vertex_Solve() || MeshStatus [meshindex].Can_Insert_Multi_Pass_Solve() || MeshStatus [meshindex].Can_Insert_Multi_Texture_Solve())) {
				
					unsigned long	fileposition;
					ChunkLoadClass meshchunk (w3dchunk);

					meshfilebuffer = new char [w3dchunk.Cur_Chunk_Length()];
					ASSERT (meshfilebuffer != NULL);
					fileposition = W3dFile->Seek (0, SEEK_CUR);
					if (meshchunk.Read (meshfilebuffer, w3dchunk.Cur_Chunk_Length()) != w3dchunk.Cur_Chunk_Length()) throw ("Cannot read data in W3D_CHUNK_MESH.");
					W3dFile->Seek (fileposition, SEEK_SET);

					RAMFileClass		    meshfile (meshfilebuffer, w3dchunk.Cur_Chunk_Length());
					MeshInfoStruct		    meshinfo (meshfile);
					LightscapeMeshSolve   meshsolve (solve, *meshinfo.TriangleChunk, *meshinfo.VertexChunk);
					SplitVertexInfoStruct splitvertexinfo (meshinfo, meshsolve);
					unsigned long		    prelitflags;

					// Update the status for this mesh.
					// Analyze the mesh status to see if it is compatible with each solve type (vertex, lightmap or both).
					MeshStatus [meshindex].SolveStatistics = *meshsolve.Get_Statistics();
					prelitflags	= MeshStatus [meshindex].Prelit_Flags();
					
					solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());

					// While there are mesh chunks...
					while (w3dchunk.Open_Chunk()) {
							
						switch (w3dchunk.Cur_Chunk_ID()) {

							case W3D_CHUNK_MESH_HEADER3:
								Translate_Mesh_Header3 (w3dchunk, prelitflags, solvechunk, splitvertexinfo);
								break;
							
							case W3D_CHUNK_VERTICES:
								Translate_Vertices (w3dchunk, solvechunk, splitvertexinfo);
								break;
								
							case W3D_CHUNK_VERTEX_NORMALS:
								Translate_Vertex_Normals (w3dchunk, solvechunk, splitvertexinfo);
								break;

							case W3D_CHUNK_VERTEX_INFLUENCES:
								Translate_Vertex_Influences (w3dchunk, solvechunk, splitvertexinfo);
								break;

							case W3D_CHUNK_TRIANGLES:
								Translate_Triangles (w3dchunk, solvechunk, splitvertexinfo);
								break;

							case W3D_CHUNK_VERTEX_SHADE_INDICES:
								Translate_Vertex_Shade_Indices (w3dchunk, solvechunk, splitvertexinfo);
								break;

							case W3D_CHUNK_MATERIAL_INFO:
								if (prelitflags & W3D_MESH_FLAG_PRELIT_UNLIT) Insert_Solve (UNLIT, meshfile, UNLIT, solvechunk, meshinfo, meshsolve, splitvertexinfo);
								if (prelitflags & W3D_MESH_FLAG_PRELIT_VERTEX) {
									Insert_Solve (UNLIT, meshfile, VERTEX, solvechunk, meshinfo, meshsolve, splitvertexinfo);
									insertedflags |= W3D_MESH_FLAG_PRELIT_VERTEX;
								}
								if (prelitflags & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS) {
									Insert_Solve (UNLIT, meshfile, MULTI_PASS, solvechunk, meshinfo, meshsolve, splitvertexinfo);
									insertedflags |= W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS;
								}
								if (prelitflags & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE) {
									Insert_Solve (UNLIT, meshfile, MULTI_TEXTURE, solvechunk, meshinfo, meshsolve, splitvertexinfo);
									insertedflags |= W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE;
								}
								break;

							case W3D_CHUNK_VERTEX_MATERIALS:
							case W3D_CHUNK_SHADERS:
							case W3D_CHUNK_TEXTURES:
							case W3D_CHUNK_MATERIAL_PASS:
								// Skip these chunks.
								break;

						  	case W3D_CHUNK_PRELIT_VERTEX:
								if (prelitflags & W3D_MESH_FLAG_PRELIT_VERTEX) {
									Insert_Solve (VERTEX, w3dchunk, VERTEX, solvechunk, meshinfo, meshsolve, splitvertexinfo);
									insertedflags |= W3D_MESH_FLAG_PRELIT_VERTEX;
								} else {
									Copy_Chunk (w3dchunk, solvechunk);
								}
								break;
							
							case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS:
								if (prelitflags & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS) {
									Insert_Solve (MULTI_PASS, w3dchunk, MULTI_PASS, solvechunk, meshinfo, meshsolve, splitvertexinfo);
									insertedflags |= W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS;
								} else {
									Copy_Chunk (w3dchunk, solvechunk);
								}
								break;
							
							case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE:
								if (prelitflags & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE) {
									Insert_Solve (MULTI_TEXTURE, w3dchunk, MULTI_TEXTURE, solvechunk, meshinfo, meshsolve, splitvertexinfo);
									insertedflags |= W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE;
								} else {
									Copy_Chunk (w3dchunk, solvechunk);
								}
								break;

							default:
						
								// This chunk does not need to be modified. Just copy it.
								Copy_Chunk (w3dchunk, solvechunk);
								break;
						}
						w3dchunk.Close_Chunk();
  					}
				
					solvechunk.End_Chunk();

			  		// Clean-up.
					delete [] meshfilebuffer;
					meshfilebuffer = NULL;

				} else {
					
					// No solves can be inserted. Just copy the existing mesh chunk.
					Copy_Chunk (w3dchunk, solvechunk);
				}

				// Update mesh status.
				MeshStatus [meshindex].InsertedFlags = insertedflags;

			  	// Advance progress bar.
				progressbar.SetStep (MeshStatus [meshindex].TriangleCount);
				progressbar.StepIt();

				// Advance mesh counter.
				meshindex++;

			} else {
				Copy_Chunk (w3dchunk, solvechunk);
			}
			w3dchunk.Close_Chunk();
	  	}

		solve.Finish();
		progressbar.ShowWindow (SW_HIDE);

		// Scan the mesh status array to see if there were any anomalies.
		anomalycount = 0;
		for (unsigned m = 0; m < MeshCount; m++) {
			if (!(MeshStatus [m].SolveStatistics == 0)) anomalycount++;
		}
		
		if (anomalycount > 0) {
			
			const char *anomalytext	= 
"%d meshes have solve anomalies associated with them. If the anomaly is serious, a\
 vertex or lightmap solve may not have been inserted. Check each mesh for specific\
 anomalies by left-clicking on the mesh name in the list.";

			StringBuilder anomalymessage (8192);

			anomalymessage.Copy (anomalytext, anomalycount);
			MessageBox (NULL, anomalymessage.String(), "Attention", MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		}

		// Add lights to the light array.
		// NOTE: Add references to the lights from the solve.
		//			This will ensure that the lights will not be deleted when the solve goes out of scope.
		for (unsigned l = 0; l < solve.Light_Count(); l++) {
			
			LightClass *light = solve.Get_Light (l);

			Lights [SolveCount].Add (light);
			light->Add_Ref();
		}

		// Clean-up.
		delete W3dFile;
		solvefile->Close();
	
		// Make the current solve file the w3d file.
		W3dFile = solvefile;

		// Flag that the document has been modified.
		SetModifiedFlag (true);

		// Increment no. of solves inserted so far.
		SolveCount++;

		// Inform user that solve has been inserted.
		StringBuilder string (256);
		string.Copy ("%s%d%s", solvetext, SolveCount, insertedtext);
		statusptr->SetPaneText (0, string.String());

		UpdateAllViews (NULL);

	} catch (const char *errormessage) {

		const char *prefixmessage = "The solve cannot be inserted.\n\n";
		
		char *messagebuffer;

		// Clean-up.
		if (meshfilebuffer != NULL) delete [] meshfilebuffer;
		W3dFile->Close();
		if (solvefile != NULL) delete solvefile;

		// Prefix the error message.
		messagebuffer = new char [strlen (prefixmessage) + strlen (errormessage) + 1];
		ASSERT (messagebuffer);
		strcpy (messagebuffer, prefixmessage);
		strcat (messagebuffer, errormessage);
		MessageBox (NULL, messagebuffer, NULL, MB_ICONERROR | MB_SETFOREGROUND);
		delete messagebuffer;
		
		// Inform user that solve has NOT been inserted.
		StringBuilder string (256);
		string.Copy ("%s%d%s", solvetext, SolveCount, notinsertedtext);
		statusptr->SetPaneText (0, string.String());
		
		UpdateAllViews (NULL);
	}
}


/***********************************************************************************************
 * LightMapDoc::Translate_Mesh_Header3 --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Translate_Mesh_Header3 (ChunkLoadClass &w3dchunk, unsigned long prelitflags, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo)
{
	W3dMeshHeader3Struct header;
	
	if (w3dchunk.Read (&header, sizeof (header)) != sizeof (header)) throw ("Cannot read data in W3D_CHUNK_MESH_HEADER3.");

	// Flag that this is a lightmapped mesh.
	header.Attributes |= prelitflags;

	// For asset tracking purposes, stamp the version no. of this tool into the mesh.
	header.PrelitVersion = theApp.Application_Version();

	// Set no. of vertices to accout for vertex splitting.
	header.NumVertices = splitvertexinfo.Vertex_Count();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	solvechunk.Write (&header, sizeof (header));
	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Translate_Vertices --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/06/01    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Translate_Vertices (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo)
{
	ChunkClass		 *vertexchunk;
	W3dVectorStruct *vertices;

	vertexchunk = new ChunkClass (w3dchunk);
	ASSERT (vertexchunk != NULL);

	vertices = (W3dVectorStruct*) vertexchunk->Get_Data();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {
		solvechunk.Write (vertices + splitvertexinfo.Remap (v), sizeof (W3dVectorStruct));
	}
	solvechunk.End_Chunk();

	delete vertexchunk;
}


/***********************************************************************************************
 * LightMapDoc::Translate_Vertex_Normals --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/06/01    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Translate_Vertex_Normals (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo)
{
	ChunkClass		 *vertexnormalchunk;
	W3dVectorStruct *vertexnormals;

	vertexnormalchunk = new ChunkClass (w3dchunk);
	ASSERT (vertexnormalchunk != NULL);

	vertexnormals = (W3dVectorStruct*) vertexnormalchunk->Get_Data();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	for (unsigned n = 0; n < splitvertexinfo.Vertex_Count(); n++) {
		solvechunk.Write (vertexnormals + splitvertexinfo.Remap (n), sizeof (W3dVectorStruct));
	}
	solvechunk.End_Chunk();

	delete vertexnormalchunk;
}


/***********************************************************************************************
 * LightMapDoc::Translate_Vertex_Influences --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/06/01    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Translate_Vertex_Influences (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo)
{
	ChunkClass		  *vertexinfluencechunk;
	W3dVertInfStruct *vertexinfluences;

	vertexinfluencechunk = new ChunkClass (w3dchunk);
	ASSERT (vertexinfluencechunk != NULL);

	vertexinfluences = (W3dVertInfStruct*) vertexinfluencechunk->Get_Data();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {
		solvechunk.Write (vertexinfluences + splitvertexinfo.Remap (v), sizeof (W3dVertInfStruct));
	}
	solvechunk.End_Chunk();

	delete vertexinfluencechunk;
}


/***********************************************************************************************
 * LightMapDoc::Translate_Triangles --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/06/01    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Translate_Triangles (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo)
{
	const unsigned verticesperface = 3;	// No. of vertices in a triangle.

	ChunkClass	 *trianglechunk;
	unsigned		  facecount, facevertexindex;	
	W3dTriStruct *triangles;

	trianglechunk = new ChunkClass (w3dchunk);
	ASSERT (trianglechunk != NULL);
	facecount = trianglechunk->Get_Size() / sizeof (W3dTriStruct);
	triangles = (W3dTriStruct*) trianglechunk->Get_Data();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	
	facevertexindex = 0;
	for (unsigned f = 0; f < facecount; f++) {

		W3dTriStruct triangle = triangles [f];

		for (unsigned v = 0; v < verticesperface; v++) {
			triangle.Vindex [v] = splitvertexinfo.Index (facevertexindex);
			facevertexindex++;
		}
		solvechunk.Write (&triangle, sizeof (triangle));
	}

	solvechunk.End_Chunk();

	delete trianglechunk;
}



/***********************************************************************************************
 * LightMapDoc::Translate_Vertex_Shade_Indices --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/06/01    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Translate_Vertex_Shade_Indices (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo)
{
	ChunkClass *vertexshadeindiceschunk;
	uint32	  *vertexshadeindices;

	vertexshadeindiceschunk = new ChunkClass (w3dchunk);
	ASSERT (vertexshadeindiceschunk != NULL);

	vertexshadeindices = (uint32*) vertexshadeindiceschunk->Get_Data();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {
		solvechunk.Write (vertexshadeindices + splitvertexinfo.Remap (v), sizeof (uint32));
	}
	solvechunk.End_Chunk();

	delete vertexshadeindiceschunk;
}


/***********************************************************************************************
 * LightMapDoc::Insert_Solve --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Insert_Solve (PrelitModeEnum inputmode, FileClass &meshfile, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo)
{
	ChunkLoadClass w3dchunk (&meshfile);
	unsigned			materialpass;
	bool				hastexturechunk;

	// This routine can only parse unlit W3D chunks.
	ASSERT (inputmode == UNLIT);

	// Output the appropriate prelit wrapper chunk.
	switch (outputmode) {

		case UNLIT:
			solvechunk.Begin_Chunk (W3D_CHUNK_PRELIT_UNLIT);
			break;

		case VERTEX:
			solvechunk.Begin_Chunk (W3D_CHUNK_PRELIT_VERTEX);
			break;

		case MULTI_PASS:
			solvechunk.Begin_Chunk (W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS);
			break;

		case MULTI_TEXTURE:
			solvechunk.Begin_Chunk (W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE);
			break;
	}
	
	meshfile.Open (FileClass::READ);
	
	// While there are mesh chunks...
	materialpass = 0;
	hastexturechunk = false;
	while (w3dchunk.Open_Chunk()) {
					
		switch (w3dchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_MESH_HEADER3:

				// Ignore these chunk types.
				break;
							
			case W3D_CHUNK_MATERIAL_INFO:
				Translate_Material_Info (inputmode, w3dchunk, outputmode, solvechunk, meshinfo, meshsolve);
				break;

			case W3D_CHUNK_VERTEX_MATERIALS:
				Translate_Vertex_Materials (inputmode, w3dchunk, outputmode, solvechunk, meshinfo);
				break;

			case W3D_CHUNK_SHADERS:
				Translate_Shaders (inputmode, w3dchunk, outputmode, solvechunk, meshinfo);
				break;

			case W3D_CHUNK_TEXTURES:
				Translate_Textures (w3dchunk, outputmode, solvechunk, meshsolve);
				hastexturechunk = true;			
				break;

			case W3D_CHUNK_MATERIAL_PASS:

				// If adding lightmaps and no texture chunk has been encountered so far add one now.
				// NOTE: WW3D expects the texture chunk to precede the material pass chunk. 
				if (!hastexturechunk && ((outputmode == MULTI_PASS) || (outputmode == MULTI_TEXTURE))) {
					solvechunk.Begin_Chunk (W3D_CHUNK_TEXTURES);
					Add_Lightmap_Textures (solvechunk, meshsolve);
					solvechunk.End_Chunk();
				}

				Translate_Material_Pass (inputmode, w3dchunk, materialpass, outputmode, solvechunk, meshinfo, meshsolve, splitvertexinfo);
				materialpass++;
				break;

		  	default:

				// Ignore these chunk types.
				break;

		}
		w3dchunk.Close_Chunk();
	}
			
	// If this is the last pass, add an additional material pass now.
	if ((materialpass == meshinfo.MaterialInfo [inputmode].PassCount) && (outputmode == MULTI_PASS)) {
		Add_Lightmap_Material_Pass (inputmode, materialpass - 1, solvechunk, meshinfo, meshsolve, splitvertexinfo);
	}

	meshfile.Close();
	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Insert_Solve --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Insert_Solve (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo)
{
	unsigned	materialpass;

	// This routine can only parse prelit W3D chunks.
	ASSERT (inputmode != UNLIT);

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	
	// While there are mesh chunks...
	materialpass = 0;
	while (w3dchunk.Open_Chunk()) {
					
		switch (w3dchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_MATERIAL_INFO:
				Translate_Material_Info (inputmode, w3dchunk, outputmode, solvechunk, meshinfo, meshsolve);
				break;

			case W3D_CHUNK_VERTEX_MATERIALS:
				Translate_Vertex_Materials (inputmode, w3dchunk, outputmode, solvechunk, meshinfo);
				break;

			case W3D_CHUNK_SHADERS:
				Copy_Chunk (w3dchunk, solvechunk);
				break;

			case W3D_CHUNK_TEXTURES:
				Translate_Textures (w3dchunk, outputmode, solvechunk, meshsolve);
				break;

			case W3D_CHUNK_MATERIAL_PASS:
				Translate_Material_Pass (inputmode, w3dchunk, materialpass, outputmode, solvechunk, meshinfo, meshsolve, splitvertexinfo);
				materialpass++;
				break;

			default:
				Copy_Chunk (w3dchunk, solvechunk);
				break;
		}
		w3dchunk.Close_Chunk();
	}
			
	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Translate_Material_Info --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Translate_Material_Info (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve)
{
	W3dMaterialInfoStruct materialinfo;

	if (w3dchunk.Read (&materialinfo, sizeof (materialinfo)) != sizeof (materialinfo)) {
		throw ("Cannot read data in W3D_CHUNK_MATERIAL_INFO.");
	}

	if ((outputmode == MULTI_PASS) || (outputmode == MULTI_TEXTURE)) {
		if (outputmode == MULTI_PASS) {
			if (inputmode == UNLIT) {

				// Increment pass count to accomodate lightmap pass.
				materialinfo.PassCount++;
	
				materialinfo.VertexMaterialCount += meshinfo.Lightmap_Vertex_Material_Count();
				materialinfo.ShaderCount += meshinfo.Lightmap_Shader_Count();
			}
		}
	
		//	Increment texture count to accomodate additional lightmap textures.
		ASSERT (materialinfo.TextureCount == meshinfo.MaterialInfo [inputmode].TextureCount);
		materialinfo.TextureCount += meshsolve.Lightmap_Count();
	}

	WWASSERT (MAX_SOLVE_COUNT == 2);
	if (inputmode != UNLIT) {
		materialinfo.VertexMaterialCount *= 2;
	}

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	solvechunk.Write (&materialinfo, sizeof (materialinfo));
	solvechunk.End_Chunk();
}	


/***********************************************************************************************
 * LightMapDoc::Translate_Vertex_Materials --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Translate_Vertex_Materials (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo)
{
	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	
	ASSERT (meshinfo.MaterialInfo [inputmode].VertexMaterialCount > 0);

	if (inputmode == UNLIT) {
	
		const W3dRGBStruct black ((uint8) 0x00, (uint8) 0x00, (uint8) 0x00);
		const W3dRGBStruct white ((uint8) 0xff, (uint8) 0xff, (uint8) 0xff);

		W3dVertexMaterialStruct vertexmaterialinfo;

		// Translate the existing vertex materials.
		while (w3dchunk.Open_Chunk()) {

			solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
			while (w3dchunk.Open_Chunk()) {
					
				switch (w3dchunk.Cur_Chunk_ID()) {

					case W3D_CHUNK_VERTEX_MATERIAL_NAME:
							
						// Copy chunk unmodified.
						Copy_Chunk (w3dchunk, solvechunk);
						break;

					case W3D_CHUNK_VERTEX_MAPPER_ARGS0:
						
						switch (outputmode) {

							case UNLIT:
							case VERTEX:
							case MULTI_PASS:

								// Copy chunk unmodified.
								Copy_Chunk (w3dchunk, solvechunk);
								break;

							case MULTI_TEXTURE:
							{
								char *argbuffer;

								// Any args in stage 0 must be moved to stage 1 because the base texture is in stage 1.
								argbuffer = new char [w3dchunk.Cur_Chunk_Length()];
								if (w3dchunk.Read (argbuffer, w3dchunk.Cur_Chunk_Length()) != w3dchunk.Cur_Chunk_Length()) {
									throw ("Cannot read data in W3D_CHUNK_VERTEX_MAPPER_ARGS.");
								}
								solvechunk.Begin_Chunk (W3D_CHUNK_VERTEX_MAPPER_ARGS1);
								solvechunk.Write (argbuffer, w3dchunk.Cur_Chunk_Length());
								solvechunk.End_Chunk();
								delete [] argbuffer;
								break;
							}
						}

					case W3D_CHUNK_VERTEX_MAPPER_ARGS1:

						// Remove this chunk.
						break;

					case W3D_CHUNK_VERTEX_MATERIAL_INFO:
					{
						W3dRGBStruct diffuse;

						// Read the chunk.
						if (w3dchunk.Read (&vertexmaterialinfo, sizeof (vertexmaterialinfo)) != sizeof (vertexmaterialinfo)) {
							throw ("Cannot read data in W3D_CHUNK_VERTEX_MATERIAL_INFO.");
						}

						switch (outputmode) {

							case UNLIT:
								// No change.
								break;

							case MULTI_TEXTURE:
							{	
								uint32 stage0attributes, stage1attributes;

								// If there are any mappers in stage 0 it must be shifted to stage 1 because
								// the base texture is now in stage 1.
								stage0attributes = vertexmaterialinfo.Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK;
								if (W3DVERTMAT_STAGE0_MAPPING_MASK > W3DVERTMAT_STAGE1_MAPPING_MASK) {
									stage1attributes = stage0attributes / (W3DVERTMAT_STAGE0_MAPPING_MASK / W3DVERTMAT_STAGE1_MAPPING_MASK);
								} else {
									stage1attributes = stage0attributes * (W3DVERTMAT_STAGE1_MAPPING_MASK / W3DVERTMAT_STAGE0_MAPPING_MASK);
								}

								// Clear the stage 0 and stage 1 attributes.
								// NOTE: Stage 1 attributes should be unused but clear them anyway as a precaution.
								vertexmaterialinfo.Attributes &= ~(W3DVERTMAT_STAGE0_MAPPING_MASK | W3DVERTMAT_STAGE1_MAPPING_MASK);
									
								// Add in the new stage 1 attributes.
								vertexmaterialinfo.Attributes |= stage1attributes;
									
								// NOTE: Fall thru.
							}

							default:
								vertexmaterialinfo.Emissive	  = vertexmaterialinfo.Diffuse;
								vertexmaterialinfo.Ambient		  = black;
								vertexmaterialinfo.Diffuse		  = black;
								vertexmaterialinfo.Specular	  = black;
								vertexmaterialinfo.Shininess	  = 1.0f;
								vertexmaterialinfo.Translucency = 0.0f;
								break;
						}

						// Write the modified chunk.
						solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
						solvechunk.Write (&vertexmaterialinfo, sizeof (vertexmaterialinfo));
						solvechunk.End_Chunk();
						break;
					}

					default:
								
						// Any other chunk type can be copied unmodified.
						Copy_Chunk (w3dchunk, solvechunk);
						break;
				}
				w3dchunk.Close_Chunk();
			}
  			w3dchunk.Close_Chunk();
			solvechunk.End_Chunk();
		}

		// Write out a vertex material for the lightmap pass.
		if (outputmode == MULTI_PASS) {

			const char *vertexmaterialname = "Lightmap";

			W3dVertexMaterialStruct vertexmaterialinfo;

			// Only 1 lightmap vertex material is supported.
			ASSERT (meshinfo.Lightmap_Vertex_Material_Count() == 1);

			W3d_Vertex_Material_Reset (&vertexmaterialinfo);
			vertexmaterialinfo.Emissive	  = white;
			vertexmaterialinfo.Ambient		  = black;
			vertexmaterialinfo.Diffuse		  = black;
			vertexmaterialinfo.Specular	  = black;
			vertexmaterialinfo.Shininess	  = 1.0f;
			vertexmaterialinfo.Translucency = 0.0f;

			solvechunk.Begin_Chunk (W3D_CHUNK_VERTEX_MATERIAL);
			solvechunk.Begin_Chunk (W3D_CHUNK_VERTEX_MATERIAL_NAME);
			solvechunk.Write (vertexmaterialname, strlen (vertexmaterialname) + 1);
			solvechunk.End_Chunk();
			solvechunk.Begin_Chunk (W3D_CHUNK_VERTEX_MATERIAL_INFO);
			solvechunk.Write (&vertexmaterialinfo, sizeof (vertexmaterialinfo));
			solvechunk.End_Chunk();
			solvechunk.End_Chunk();
		}
	
	} else {
	
		DynamicVectorClass <ChunkClass*> vertexmaterialchunks;

		// Duplicate all of the existing vertex materials.
		
		while (w3dchunk.Open_Chunk()) {
			
			ChunkClass *vertexmaterialchunkptr;

			vertexmaterialchunkptr = new ChunkClass (w3dchunk);
			
			solvechunk.Begin_Chunk (vertexmaterialchunkptr->ChunkType);
			solvechunk.Write (vertexmaterialchunkptr->Get_Data(), vertexmaterialchunkptr->Get_Size());
			solvechunk.End_Chunk();
			
			vertexmaterialchunks.Add (vertexmaterialchunkptr);
			w3dchunk.Close_Chunk();
		}

		for (int vm = 0; vm < vertexmaterialchunks.Count(); vm++) {
			solvechunk.Begin_Chunk (vertexmaterialchunks [vm]->ChunkType);
			solvechunk.Write (vertexmaterialchunks [vm]->Get_Data(), vertexmaterialchunks [vm]->Get_Size());
			solvechunk.End_Chunk();

			delete vertexmaterialchunks [vm];
		}
	}

	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Translate_Shaders --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Translate_Shaders (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo)
{
	unsigned			  s;
	W3dShaderStruct  shader;

	ASSERT (meshinfo.MaterialInfo [inputmode].ShaderCount > 0);

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());

	for (s = 0; s < meshinfo.MaterialInfo [inputmode].ShaderCount; s++) {
		
		if (w3dchunk.Read (&shader, sizeof (shader)) != sizeof (shader)) {
			throw ("Cannot read data in W3D_CHUNK_SHADERS.");
		}

		switch (outputmode) {

			case UNLIT:
			case VERTEX:
			case MULTI_PASS:

				// No change.
				break;

			case MULTI_TEXTURE:
				// If texturing is not currently enabled then enable single-texture rendering -
				// otherwise enable multi-texture rendering.
				if (W3d_Shader_Get_Texturing (&shader) == W3DSHADER_TEXTURING_DISABLE) {

					W3d_Shader_Set_Texturing (&shader, W3DSHADER_TEXTURING_ENABLE);

				} else {
					
					ASSERT (W3d_Shader_Get_Texturing (&shader) == W3DSHADER_TEXTURING_ENABLE);

					// Enable detail texture.
					W3d_Shader_Set_Detail_Color_Func (&shader, W3DSHADER_DETAILCOLORFUNC_SCALE);
					W3d_Shader_Set_Detail_Alpha_Func (&shader, W3DSHADER_DETAILALPHAFUNC_SCALE);
					W3d_Shader_Set_Post_Detail_Color_Func (&shader, W3DSHADER_DETAILCOLORFUNC_SCALE);
					W3d_Shader_Set_Post_Detail_Alpha_Func (&shader, W3DSHADER_DETAILALPHAFUNC_SCALE);
				}
				break;
		}
		
		solvechunk.Write (&shader, sizeof (shader));
	}

	if (outputmode == MULTI_PASS) {

		// Write out a set of shaders for the lightmap pass.
		for (s = 0; s < meshinfo.Lightmap_Shader_Count(); s++) {
	
			W3d_Shader_Reset (&shader);

			W3d_Shader_Set_Depth_Compare (&shader, W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL);			 
			W3d_Shader_Set_Depth_Mask (&shader, W3DSHADER_DEPTHMASK_WRITE_DISABLE);				 
			W3d_Shader_Set_Src_Blend_Func (&shader, W3DSHADER_SRCBLENDFUNC_ZERO);			 
			W3d_Shader_Set_Dest_Blend_Func (&shader, W3DSHADER_DESTBLENDFUNC_SRC_COLOR);			 
			W3d_Shader_Set_Pri_Gradient (&shader, W3DSHADER_PRIGRADIENT_DISABLE);
			W3d_Shader_Set_Sec_Gradient (&shader, W3DSHADER_SECGRADIENT_DISABLE);				 
			W3d_Shader_Set_Texturing (&shader, W3DSHADER_TEXTURING_ENABLE);
			W3d_Shader_Set_Detail_Color_Func (&shader, W3DSHADER_DETAILCOLORFUNC_DISABLE);		 
  			W3d_Shader_Set_Detail_Alpha_Func (&shader, W3DSHADER_DETAILALPHAFUNC_DISABLE);		 
  			W3d_Shader_Set_Alpha_Test (&shader, W3DSHADER_ALPHATEST_DISABLE);				 
  			W3d_Shader_Set_Post_Detail_Color_Func (&shader, W3DSHADER_DETAILCOLORFUNC_DISABLE);
  			W3d_Shader_Set_Post_Detail_Alpha_Func (&shader, W3DSHADER_DETAILALPHAFUNC_DISABLE); 
  
			solvechunk.Write (&shader, sizeof (shader));
		}
	}

	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Translate_Textures --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Translate_Textures (ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const LightscapeMeshSolve &meshsolve)
{
	switch (outputmode) {

		case UNLIT:
		case VERTEX:
			Copy_Chunk (w3dchunk, solvechunk);
			break;

		case MULTI_PASS:
		case MULTI_TEXTURE:
	
			solvechunk.Begin_Chunk (W3D_CHUNK_TEXTURES);

			// Copy the existing texture chunks.
			while (w3dchunk.Open_Chunk()) {
		
				ChunkClass *datachunk;

				datachunk = new ChunkClass (w3dchunk);
				ASSERT (datachunk != NULL);

				solvechunk.Begin_Chunk (datachunk->ChunkType);
				solvechunk.Write (datachunk->Get_Data(), datachunk->Get_Size());
				solvechunk.End_Chunk();
		
				w3dchunk.Close_Chunk();

				// Clean-up.
				delete datachunk;
			}

			Add_Lightmap_Textures (solvechunk, meshsolve);
			solvechunk.End_Chunk();
			break;
	}
}


/***********************************************************************************************
 * LightMapDoc::Add_Lightmap_Textures --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Add_Lightmap_Textures (ChunkSaveClass &solvechunk, const LightscapeMeshSolve &meshsolve)
{
	const int undefined = 0;	// Arbitrary value for any unused data field.

	W3dTextureInfoStruct textureinfo;

	// Write additional texture chunks, one chunk per lightmap.
	// NOTE: The allowable no. of mip-map levels is dependant upon and increases with the
	//			edge blend thickness specified in the lightmap packer.
	textureinfo.Attributes = W3DTEXTURE_MIP_LEVELS_2 | W3DTEXTURE_CLAMP_U | W3DTEXTURE_CLAMP_V;
	textureinfo.AnimType	  = (uint16) undefined;
	textureinfo.FrameCount = 1;
	textureinfo.FrameRate  = (float32) undefined;
	for (unsigned l = 0; l < meshsolve.Lightmap_Count(); l++) {
	
		solvechunk.Begin_Chunk (W3D_CHUNK_TEXTURE);

		solvechunk.Begin_Chunk (W3D_CHUNK_TEXTURE_NAME);
		solvechunk.Write (meshsolve.Lightmap_Pathname (l), strlen (meshsolve.Lightmap_Pathname (l)) + 1);
		solvechunk.End_Chunk();

		solvechunk.Begin_Chunk (W3D_CHUNK_TEXTURE_INFO);
		solvechunk.Write (&textureinfo, sizeof (textureinfo));
		solvechunk.End_Chunk();
		
		solvechunk.End_Chunk();
	}
}


/***********************************************************************************************
 * LightMapDoc::Translate_Material_Pass --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Translate_Material_Pass (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, unsigned materialpass, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo)
{
	bool		hastexturestage;
	unsigned stage;

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());

	// While there are material chunks... 
	hastexturestage = false;
	stage = 0;
	while (w3dchunk.Open_Chunk()) {

		switch (w3dchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_DCG:
				Translate_DCGs (w3dchunk, solvechunk, splitvertexinfo); 
				break;

			case W3D_CHUNK_DIG:
				Translate_DIGs (inputmode, w3dchunk, materialpass, solvechunk, meshinfo, meshsolve, splitvertexinfo);
				break;

			case W3D_CHUNK_VERTEX_MATERIAL_IDS:
				Translate_Vertex_Material_IDs (inputmode, w3dchunk, solvechunk, meshinfo, splitvertexinfo);
				break;

			case W3D_CHUNK_TEXTURE_STAGE:
				if (inputmode == UNLIT) {

					// Stage 0 is lightmap, stage 1 is base texture.
					if (outputmode == MULTI_TEXTURE) {
						Add_Lightmap_Stage (inputmode, solvechunk, meshinfo, meshsolve, splitvertexinfo);
					}
					Translate_Texture_Stage (w3dchunk, solvechunk, splitvertexinfo);
					hastexturestage = true;
					
				} else {

					// NOTE: If multi-pass, last material pass contains lightmap stage.
					//			If multi-texture, zeroth stage is lightmap stage.
					if (((outputmode == MULTI_PASS) && (materialpass == meshinfo.MaterialInfo [inputmode].PassCount - 1)) ||
						 ((outputmode == MULTI_TEXTURE) && (stage == 0))) {

						Translate_Lightmap_Stage (inputmode, w3dchunk, solvechunk, meshinfo, meshsolve, splitvertexinfo);

					} else {

						Translate_Texture_Stage (w3dchunk, solvechunk, splitvertexinfo);
					}
					stage++;
				}	
				break;

			default:
						
				// Any other chunk type can be copied unmodified.
				Copy_Chunk (w3dchunk, solvechunk);
				break;
		}
		w3dchunk.Close_Chunk();
	}

	if (inputmode == UNLIT) {

		if (!hastexturestage && (outputmode == MULTI_TEXTURE)) {
				
			// Stage 0 is lightmap, stage 1 is unused. 
			Add_Lightmap_Stage (inputmode, solvechunk, meshinfo, meshsolve, splitvertexinfo);
		}	
			
		if (outputmode == VERTEX) {
			Add_DIGs (inputmode, materialpass, solvechunk, meshinfo, meshsolve, splitvertexinfo);
		}
	}
	
	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Translate_DCGs --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/26/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightMapDoc::Translate_DCGs (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo)
{
	ChunkClass	  *dcgschunk;
	W3dRGBAStruct *dcgs;

	dcgschunk = new ChunkClass (w3dchunk);
	ASSERT (dcgschunk != NULL);

	dcgs = (W3dRGBAStruct*) dcgschunk->Get_Data();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {
		solvechunk.Write (dcgs + splitvertexinfo.Remap (v), sizeof (W3dRGBAStruct));
	}
	solvechunk.End_Chunk();

	delete dcgschunk;
}


/***********************************************************************************************
 * LightMapDoc::Translate_DIGs --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/26/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightMapDoc::Translate_DIGs (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, unsigned materialpass, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo)
{
	ChunkClass	 *digschunk;
	W3dRGBStruct *digs;

	digschunk = new ChunkClass (w3dchunk);
	ASSERT (digschunk != NULL);

	digs = (W3dRGBStruct*) digschunk->Get_Data();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {
		solvechunk.Write (digs + splitvertexinfo.Remap (v), sizeof (W3dRGBStruct));
	}
	solvechunk.End_Chunk();

	delete digschunk;

	Add_DIGs (inputmode, materialpass, solvechunk, meshinfo, meshsolve, splitvertexinfo);
}


/***********************************************************************************************
 * LightMapDoc::Add_DIGs --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/26/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightMapDoc::Add_DIGs (PrelitModeEnum inputmode, unsigned materialpass, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo)
{
	// Output the pre-calculated vertex lighting chunk.
	solvechunk.Begin_Chunk (W3D_CHUNK_DIG);
	for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {

		W3dRGBStruct vertexcolor;

		vertexcolor  = meshsolve.Vertex_Color (splitvertexinfo.Remap (v));
		solvechunk.Write (&vertexcolor, sizeof (vertexcolor));
	}
	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Translate_Vertex_Material_IDs --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/26/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightMapDoc::Translate_Vertex_Material_IDs (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const SplitVertexInfoStruct &splitvertexinfo)
{
	ChunkClass *vertexmaterialidchunk;
	uint32	  *vertexmaterialids;
	unsigned		vertexmaterialidcount;

	vertexmaterialidchunk = new ChunkClass (w3dchunk);
	ASSERT (vertexmaterialidchunk != NULL);
	vertexmaterialidcount = vertexmaterialidchunk->Get_Size() / sizeof (uint32);

	vertexmaterialids = (uint32*) vertexmaterialidchunk->Get_Data();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	if (vertexmaterialidcount == 1) {
		solvechunk.Write (vertexmaterialids, sizeof (uint32));
	} else {
		for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {
			solvechunk.Write (vertexmaterialids + splitvertexinfo.Remap (v), sizeof (uint32));
		}
	}
	solvechunk.End_Chunk();

	if (inputmode != UNLIT) {

		uint32 expandedid;

		solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
		if (vertexmaterialidcount == 1) {
			expandedid = (*vertexmaterialids) + meshinfo.MaterialInfo [inputmode].VertexMaterialCount;
			solvechunk.Write (&expandedid, sizeof (uint32));
		} else {
			for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {
				expandedid = *(vertexmaterialids + splitvertexinfo.Remap (v)) + meshinfo.MaterialInfo [inputmode].VertexMaterialCount;
				solvechunk.Write (&expandedid, sizeof (uint32));
			}
		}
		solvechunk.End_Chunk();
	}

	delete vertexmaterialidchunk;
}


/***********************************************************************************************
 * LightMapDoc::Translate_Texture_Stage --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/26/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightMapDoc::Translate_Texture_Stage (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo)
{
	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	
	while (w3dchunk.Open_Chunk()) {

		switch (w3dchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_STAGE_TEXCOORDS:
				Translate_Stage_Texcoords (w3dchunk, solvechunk, splitvertexinfo);
				break;

			default:
				Copy_Chunk (w3dchunk, solvechunk);
				break;
		}
		
		w3dchunk.Close_Chunk();
	}

	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Translate_Stage_Texcoords --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/26/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightMapDoc::Translate_Stage_Texcoords (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo)
{
	ChunkClass		   *texcoordchunk;
	W3dTexCoordStruct *texcoords;

	texcoordchunk = new ChunkClass (w3dchunk);
	ASSERT (texcoordchunk != NULL);

	texcoords = (W3dTexCoordStruct*) texcoordchunk->Get_Data();

	solvechunk.Begin_Chunk (w3dchunk.Cur_Chunk_ID());
	for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {
		solvechunk.Write (texcoords + splitvertexinfo.Remap (v), sizeof (W3dTexCoordStruct));
	}
	solvechunk.End_Chunk();

	delete texcoordchunk;
}


/***********************************************************************************************
 * LightMapDoc::Add_Lightmap_Material_Pass --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/26/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightMapDoc::Add_Lightmap_Material_Pass (PrelitModeEnum inputmode, unsigned materialpass, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo)
{
	uint32 vertexmaterialid;
	uint32 shaderid;	

	// This routine only supports unlit meshes.
	ASSERT (inputmode == UNLIT);

	// Begin the lightmap material pass.
	solvechunk.Begin_Chunk (W3D_CHUNK_MATERIAL_PASS);

	// Write vertex material ID chunk.
	solvechunk.Begin_Chunk (W3D_CHUNK_VERTEX_MATERIAL_IDS);
	ASSERT (meshinfo.Lightmap_Vertex_Material_Count() == 1);
	vertexmaterialid = meshinfo.MaterialInfo [inputmode].VertexMaterialCount;	
	solvechunk.Write (&vertexmaterialid, sizeof (vertexmaterialid));
	solvechunk.End_Chunk();

	// Write shader ID chunk.
	solvechunk.Begin_Chunk (W3D_CHUNK_SHADER_IDS);
	if (meshinfo.Lightmap_Shader_Count() == 1) {
		shaderid = meshinfo.MaterialInfo [inputmode].ShaderCount;
		solvechunk.Write (&shaderid, sizeof (shaderid));
	} else {
		ASSERT (meshinfo.ShaderIdChunk [inputmode][materialpass] != NULL);
		for (unsigned s = 0; s < meshinfo.ShaderIdChunk [inputmode][materialpass]->Get_Size() / sizeof (uint32); s++) {
			shaderid = meshinfo.MaterialInfo [inputmode].ShaderCount + *((uint32*) meshinfo.ShaderIdChunk [inputmode][materialpass]->Get_Data() + s);
			solvechunk.Write (&shaderid, sizeof (shaderid));
		}
	}
	solvechunk.End_Chunk();

	Add_Lightmap_Stage (inputmode, solvechunk, meshinfo, meshsolve, splitvertexinfo);

	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Add_Lightmap_Stage --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/25/00    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Add_Lightmap_Stage (PrelitModeEnum inputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo)
{
	solvechunk.Begin_Chunk (W3D_CHUNK_TEXTURE_STAGE);
	Add_Lightmap_Stage_Chunks (inputmode, solvechunk, meshinfo, meshsolve, splitvertexinfo);
	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Translate_Lightmap_Stage --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/25/00    IML : Created.                                                                * 
 *=============================================================================================*/
void LightMapDoc::Translate_Lightmap_Stage (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo)
{
	solvechunk.Begin_Chunk (W3D_CHUNK_TEXTURE_STAGE);

	while (w3dchunk.Open_Chunk()) {

		switch (w3dchunk.Cur_Chunk_ID()) {

			case W3D_CHUNK_STAGE_TEXCOORDS:
				Translate_Stage_Texcoords (w3dchunk, solvechunk, splitvertexinfo);
				break;

			default:
				Copy_Chunk (w3dchunk, solvechunk);
				break;
		}
		
		w3dchunk.Close_Chunk();
	}

	Add_Lightmap_Stage_Chunks (inputmode, solvechunk, meshinfo, meshsolve, splitvertexinfo);
	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Add_Lightmap_Stage_Chunks -- Add lightmap texture coordinate chunks.			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Add_Lightmap_Stage_Chunks (PrelitModeEnum inputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo)
{
	const unsigned facecount = meshinfo.TriangleChunk->Get_Size() / sizeof (W3dTriStruct);
	
	unsigned	f; 
   unsigned	textureindex;
	bool		writesingleindex;	

	// Write texture ID's (one per face in face order).
	// If all of the texture indexes are the same then just write a single ID (this will be interpeted as an ID that applies to all faces in the mesh).
	solvechunk.Begin_Chunk (W3D_CHUNK_TEXTURE_IDS);
	writesingleindex = true;
	textureindex = meshinfo.MaterialInfo [inputmode].TextureCount + meshsolve.Lightmap_Index (0);
	for (f = 1; f < facecount; f++) {
		if ((meshinfo.MaterialInfo [inputmode].TextureCount + meshsolve.Lightmap_Index (f)) != textureindex) {
			writesingleindex = false;
			break;
		}
	}
	if (writesingleindex) {
		solvechunk.Write (&textureindex, sizeof (textureindex));
	} else {
		for (f = 0; f < facecount; f++) {
			textureindex = meshinfo.MaterialInfo [inputmode].TextureCount + meshsolve.Lightmap_Index (f);
			solvechunk.Write (&textureindex, sizeof (textureindex));
		}
	}
	solvechunk.End_Chunk();

	// Write texture coordinates.
	solvechunk.Begin_Chunk (W3D_CHUNK_STAGE_TEXCOORDS);
	for (unsigned v = 0; v < splitvertexinfo.Vertex_Count(); v++) {
		solvechunk.Write (splitvertexinfo.UV (v), sizeof (W3dTexCoordStruct));
	}
	solvechunk.End_Chunk();
}


/***********************************************************************************************
 * LightMapDoc::Copy_Chunk -- Copy chunk from load file to save file.								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapDoc::Copy_Chunk (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk)
{
	ChunkClass *chunk;

	chunk = new ChunkClass (loadchunk);
	ASSERT (chunk != NULL);

	savechunk.Begin_Chunk (chunk->ChunkType);
	savechunk.Write (chunk->Get_Data(), chunk->Get_Size());
	savechunk.End_Chunk();

	delete chunk;
}


/***********************************************************************************************
 * ChunkClass::ChunkClass -- Create a chunk from load file.												  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
ChunkClass::ChunkClass (ChunkLoadClass &loadchunk)	: ChunkHeader (loadchunk.Cur_Chunk_ID(), loadchunk.Cur_Chunk_Length()) 
{
	const char *controlstring = "Cannot read data in chunk %d.";

	ChunkType = loadchunk.Cur_Chunk_ID();
	ChunkSize = loadchunk.Cur_Chunk_Length();

	Data = new char [ChunkSize];
	ASSERT (Data != NULL);
	
	if (loadchunk.Read (Data, ChunkSize) != ChunkSize) {
 
		static char _messagebuffer [256];

		StringBuilder errormessage (_messagebuffer, sizeof (_messagebuffer));

		// Clean-up.
		delete [] Data;
		Data = NULL;

		throw (errormessage.Copy (controlstring, ChunkType));
	}
}


// The following is maintained by MFC tools.
void LightMapDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


#ifdef _DEBUG
void LightMapDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void LightMapDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}

#endif //_DEBUG
