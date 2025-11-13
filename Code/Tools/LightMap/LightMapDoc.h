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
 *                     $Modtime:: 7/24/01 4:53p       $* 
 *                                                                                             * 
 *                    $Revision:: 39                                                        $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef LIGHTMAPDOC_H
#define LIGHTMAPDOC_H

// Includes.
#include "Rawfile.h"
#include "w3d_file.h"
#include "Chunk.h"
#include "Lightscape.h"
#include "meshmdl.h"

// Defines.
#define MAX_SOLVE_COUNT							2
#define TEMPORARY_SOLVE_FILENAME_COUNT		2

class LightMapDoc : public CDocument
{
	public:

		enum PrelitModeEnum {
			UNLIT,
			VERTEX,
			MULTI_PASS,
			MULTI_TEXTURE,
			COUNT
		};

		// Flags for anomalies in a W3D model.
		// WARNING: Do not exceed no. of bits in unsigned int.
		enum MeshFlagsEnum {
				
			// Unconditional errors.
			MESH_HIDDEN,
			MESH_PRELIT,
			MESH_NO_VERTEX_MATERIALS,
			MESH_NO_SHADERS,
			MESH_HAS_HIERARCHY,
			MESH_NOT_MODULATED_PRIMARY,
			MESH_HAS_SPECULAR,
			MESH_SPECULAR_TO_DIFFUSE,
			MESH_W3D_CHUNK_DEFORM,
			MESH_TWO_SIDED,
			MESH_HAS_EMISSIVE,
			MESH_HAS_SCG,

			// Vertex specific errors.
			MESH_HAS_DIG,

			// Lightmap multi-pass specific errors.
			MESH_TOO_MANY_PASSES,
			MESH_ALPHATEST,
			MESH_PASS_ZERO_DESTBLEND,

			// Lightmap multi-texture specific errors.
			MESH_TOO_MANY_STAGES,

			// General warnings.
			MESH_VERTEX_COLORING,
			
			MESH_FLAG_COUNT,

			// Unconditional errors.
			MESH_ERROR_MASK = (1 << MESH_HIDDEN) |
									(1 << MESH_PRELIT) |
									(1 << MESH_NO_VERTEX_MATERIALS) |
									(1 << MESH_NO_SHADERS) |
									(1 << MESH_HAS_HIERARCHY) |
									(1 << MESH_NOT_MODULATED_PRIMARY) |
									(1 << MESH_HAS_SPECULAR) |
									(1 << MESH_SPECULAR_TO_DIFFUSE) |
									(1 << MESH_W3D_CHUNK_DEFORM) |
									(1 << MESH_TWO_SIDED) |	
								   (1 << MESH_HAS_EMISSIVE) |
									(1 << MESH_HAS_SCG),

			// Anomalies that will prevent a vertex solve from being inserted.
			MESH_VERTEX_ERROR_MASK = MESH_ERROR_MASK |
											 (1 << MESH_HAS_DIG),

			// Anomalies that will prevent a multi-pass lightmap solve from being inserted.
			MESH_MULTI_PASS_ERROR_MASK = MESH_ERROR_MASK |
												  (1 << MESH_TOO_MANY_PASSES) |
												  (1 << MESH_ALPHATEST) |
												  (1 << MESH_PASS_ZERO_DESTBLEND),

			// Anomalies that will prevent a multi-texture lightmap solve from being inserted.
			MESH_MULTI_TEXTURE_ERROR_MASK = MESH_ERROR_MASK |
													  (1 << MESH_TOO_MANY_STAGES)

		};

		// Functions.
		bool				  Is_Open() {return (W3dFile != NULL);}
		bool				  Can_Insert_Solve()									{return (CanInsertSolve && SolveCount < MAX_SOLVE_COUNT);}	

		void				  Insert_Solve (const char *solvedirectoryname, const char *solvefilenamelist, const char *inclusionstring, bool invertselection, bool blendnoise);
		
		bool				  Solve_Inserted()									{return (SolveCount > 0);}
		unsigned			  Mesh_Count()											{return (MeshStatus.Count());}	
		const char		 *Mesh_Name (unsigned index)						{return (MeshStatus [index].Name);}
		
		const char		 *Mesh_Anomalies_String (unsigned meshindex, bool verbose, StringBuilder &string);
		const char		 *Solve_Anomalies_String (unsigned meshindex, bool verbose, StringBuilder &string);
		const char		 *Vertex_Solve_Status_String (unsigned meshindex, StringBuilder &string);
		const char		 *Lightmap_Solve_Status_String (unsigned meshindex, StringBuilder &string);

	private:

	  	struct MeshInfoStruct {

			public:

				 MeshInfoStruct (FileClass &meshfile);
				~MeshInfoStruct ();
				
				unsigned Lightmap_Vertex_Material_Count() const {return (1);}
				unsigned Lightmap_Shader_Count()				const {return (1);}

				W3dMeshHeader3Struct									 Header;
				W3dMaterialInfoStruct								 MaterialInfo [PrelitModeEnum::COUNT];
				ChunkClass												*TriangleChunk;
				ChunkClass												*VertexChunk;
				DynamicVectorClass <W3dVertexMaterialStruct> *VertexMaterials [PrelitModeEnum::COUNT];
				ChunkClass												*ShaderChunk [PrelitModeEnum::COUNT];
				ChunkClass												*ShaderIdChunk [PrelitModeEnum::COUNT][MeshMatDescClass::MAX_PASSES];
				ChunkClass												*SCGChunk [PrelitModeEnum::COUNT][MeshMatDescClass::MAX_PASSES];
				bool														 SCGsExist;	
				bool														 DIGsExist;
				bool														 DeformExists;	
				bool														 IsMultiStage;
				bool														 VertexColorsExist;	

			private:

				void Parse_Vertex_Materials (ChunkLoadClass &w3dchunk, unsigned prelitmode);
				void Parse_Material_Pass (ChunkLoadClass &w3dchunk, unsigned prelitmode, unsigned materialpass);
				void Parse_Prelit_Chunk (ChunkLoadClass &w3dchunk, unsigned prelitmode);
		};

		struct MeshAnomalyStruct {

			public:

				MeshAnomalyStruct (const MeshInfoStruct &meshinfo);

				unsigned int MeshAnomalies;
		};

		struct MeshStatusStruct {

			// Equality operator.
			bool operator == (const MeshStatusStruct &m) {
				return (strcmp (Name, m.Name) == 0);
			}

			// Inequality operator.
			bool operator != (const MeshStatusStruct &m) {
				return (!(*this == m));
			}

			MeshStatusStruct() {}

			MeshStatusStruct (const char *name, unsigned trianglecount, unsigned int meshanomalies)
			{
				strcpy (Name, name);
				TriangleCount = trianglecount;
				MeshAnomalies = meshanomalies;
				InsertedFlags = 0;
			}

			bool Can_Insert_Vertex_Solve() { 
				return (((MeshAnomalies & MESH_VERTEX_ERROR_MASK) == 0) && SolveStatistics.Valid_Vertex_Solve());
			}

			bool Can_Insert_Multi_Pass_Solve() { 
				return (((MeshAnomalies & MESH_MULTI_PASS_ERROR_MASK) == 0) && SolveStatistics.Valid_Lightmap_Solve());
			}

			bool Can_Insert_Multi_Texture_Solve() { 
				return (((MeshAnomalies & MESH_MULTI_TEXTURE_ERROR_MASK) == 0) && SolveStatistics.Valid_Lightmap_Solve());
			}

			bool Inserted_Vertex_Solve()			{return ((InsertedFlags & W3D_MESH_FLAG_PRELIT_VERTEX) != 0);}
			bool Inserted_Multi_Pass_Solve()		{return ((InsertedFlags & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS) != 0);}
			bool Inserted_Multi_Texture_Solve() {return ((InsertedFlags & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE) != 0);}

			unsigned int Prelit_Flags() {

				unsigned int prelitflags;

				// If a vertex solve cannot be inserted substitute an unlit solve.
				if (Can_Insert_Vertex_Solve()) {
					prelitflags = W3D_MESH_FLAG_PRELIT_VERTEX;
				} else {
					prelitflags = W3D_MESH_FLAG_PRELIT_UNLIT;
				}
				if (Can_Insert_Multi_Pass_Solve())	  prelitflags |= W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS;
				if (Can_Insert_Multi_Texture_Solve()) prelitflags |= W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE;
				return (prelitflags);
			}

			char				 Name [W3D_NAME_LEN];
			unsigned			 TriangleCount;	
			unsigned int	 MeshAnomalies;
			unsigned int	 InsertedFlags;			// Which prelit mode types were inserted with the most recent solve?
			SolveStatistics SolveStatistics;
		};

		struct SplitVertexInfoStruct {

			public:
				 SplitVertexInfoStruct (const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve);
				~SplitVertexInfoStruct();

				unsigned				 Vertex_Count() const	  {return (VertexCount);}
				uint32				 Remap (unsigned v) const {ASSERT (v < Vertex_Count());	 return (RemapTable [v]);}
				uint32				 Index (unsigned v) const {ASSERT (v < FaceVertexCount);	 return (IndexTable [v]);}
				W3dTexCoordStruct	*UV	 (unsigned v) const {ASSERT (v < Vertex_Count());	 return (UVPtrTable [v]);}

			private:
				SplitVertexInfoStruct() {ASSERT (false);}

				unsigned				  VertexCount;
				unsigned				  FaceVertexCount;	
				uint32				 *RemapTable;
				uint32				 *IndexTable;
				W3dTexCoordStruct	 *UVTable;
				W3dTexCoordStruct **UVPtrTable;
		};	

		struct MeshReorderStruct {

			// Equality operator.
			bool operator == (const MeshReorderStruct &m) {
				return ((Chunk == m.Chunk) && (Position == m.Position));
			}

			// Inequality operator.
			bool operator != (const MeshReorderStruct &m) {
				return (!(*this == m));
			}

			public:
				ChunkClass *Chunk;
				Vector3		Position;
		};

		// Functions.
		bool Check_Document();
		
		void Reorder();

		void Optimize();
		void Optimize_Prelit_Vertex_Material_Pass (ChunkLoadClass &w3dchunk, ChunkSaveClass &optimizechunk);

		void Rename_Mesh (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename, const char *lightmapdirectory);
		void Rename_Mesh_Header (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename);
		void Rename_Prelit_Chunks (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *lightmapdirectory);
		void Rename_Lightmaps (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *lightmapdirectory);
		void Rename_Collection (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename);
		void Rename_HLOD (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename);
		void Rename_Dazzle (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk, const char *filename);
		
		void Save_Lights (const char *pathname);

 		void Translate_Mesh_Header3 (ChunkLoadClass &w3dchunk, unsigned int prelitflags, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Vertices (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Vertex_Normals (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Vertex_Influences (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Triangles (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Vertex_Shade_Indices (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo);
		void Insert_Solve (PrelitModeEnum inputmode, FileClass &meshfile, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo);
		void Insert_Solve (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Material_Info (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve);
		void Translate_Vertex_Materials (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo);
		void Translate_Shaders (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo);
		void Translate_Textures (ChunkLoadClass &w3dchunk, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const LightscapeMeshSolve &meshsolve);
		void Add_Lightmap_Textures (ChunkSaveClass &solvechunk, const LightscapeMeshSolve &meshsolve);
		void Translate_Material_Pass (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, unsigned materialpass, PrelitModeEnum outputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_DCGs (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_DIGs (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, unsigned materialpass, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo);
		void Add_DIGs (PrelitModeEnum inputmode, unsigned materialpass, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Vertex_Material_IDs (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Texture_Stage (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Stage_Texcoords (ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const SplitVertexInfoStruct &splitvertexinfo);
		void Add_Lightmap_Material_Pass (PrelitModeEnum inputmode, unsigned materialpass, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo);
		void Add_Lightmap_Stage (PrelitModeEnum inputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo);
		void Translate_Lightmap_Stage (PrelitModeEnum inputmode, ChunkLoadClass &w3dchunk, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo);
		void Add_Lightmap_Stage_Chunks (PrelitModeEnum inputmode, ChunkSaveClass &solvechunk, const MeshInfoStruct &meshinfo, const LightscapeMeshSolve &meshsolve, const SplitVertexInfoStruct &splitvertexinfo);
		void Copy_Chunk (ChunkLoadClass &loadchunk, ChunkSaveClass &savechunk);

		// Member data.
		char												DocumentName [_MAX_FNAME];	// Name of this document.
		RawFileClass								  *W3dFile;							// Ptr to current open w3d document.
		unsigned											MeshCount;						// Total no. of meshes in this w3d document.
		unsigned											TriangleCount;					// Total no. of triangles in this w3d document.
		bool											   CanInsertSolve;				// Can a solve be inserted?						
		unsigned											SolveCount;						// No. of solves inserted so far.
		DynamicVectorClass <MeshStatusStruct>  MeshStatus;						// Mesh information database.
		DynamicVectorClass <LightClass*>			Lights [MAX_SOLVE_COUNT];	// All active lights in the solves.

		// Static data.
		static const char							  *_TemporarySolveFilename [TEMPORARY_SOLVE_FILENAME_COUNT]; 
		static const char							  *_TemporaryOptimizeFilename;
		static const char							  *_TemporaryReorderFilename;


// The following is maintained by MFC tools.
protected: // create from serialization only
	LightMapDoc();
	DECLARE_DYNCREATE(LightMapDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LightMapDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~LightMapDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(LightMapDoc)
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif // LIGHTMAPDOC_H
