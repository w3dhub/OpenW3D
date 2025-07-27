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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : wwphys																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/renegadeterrainmaterialpass.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/05/02 10:56a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadeterrainmaterialpass.h"
#include "terrainmaterial.h"
#include "vector2.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "chunkio.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES					= 0x03040552,
	CHUNKID_VERTEX_ALPHAS,
	CHUNKID_VERTEX_UVS,
	CHUNKID_MATERIAL,
	CHUNKID_QUAD_LIST,
	CHUNKID_VERTEX_RENDER_LIST,
	CHUNKID_VERTEX_INDEX_MAP
};

enum
{
	VARID_VERTEX_COUNT				= 0x01,
};


////////////////////////////////////////////////////////////////
//
//	RenegadeTerrainMaterialPassClass
//
////////////////////////////////////////////////////////////////
RenegadeTerrainMaterialPassClass::RenegadeTerrainMaterialPassClass (void)	:
	VertexAlpha (NULL),
	GridUVs (NULL),
	Material (NULL),
	VertexCount (0)
{
	::memset (VertexIndexMap, 0, sizeof (VertexIndexMap));
	::memset (IndexBuffers, 0, sizeof (IndexBuffers));
	::memset (VertexBuffers, 0, sizeof (VertexBuffers));
	
	//
	//	Prep the lists
	//
	for (int index = 0; index < PASS_COUNT; index ++) {
		QuadList[index].Set_Growth_Step (1000);
		VertexRenderList[index].Set_Growth_Step (1000);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~RenegadeTerrainMaterialPassClass
//
////////////////////////////////////////////////////////////////
RenegadeTerrainMaterialPassClass::~RenegadeTerrainMaterialPassClass (void)
{
	REF_PTR_RELEASE (Material);
	
	//
	//	Free the UV array
	//
	if (GridUVs != NULL) {
		delete [] GridUVs;
		GridUVs = NULL;
	}

	//
	//	Free the vertex alpha array
	//
	if (VertexAlpha != NULL) {
		delete [] VertexAlpha;
		VertexAlpha = NULL;
	}

	//
	//	Free the vertex index arrays
	//
	for (int index = 0; index < PASS_COUNT; index ++) {
		if (VertexIndexMap[index] != NULL) {
			delete [] VertexIndexMap[index];
			VertexIndexMap[index] = NULL;
		}
		
		//
		//	Release our hold on the index and vertex buffers for this pass
		//
		REF_PTR_RELEASE (IndexBuffers[index]);
		REF_PTR_RELEASE (VertexBuffers[index]);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Allocate
//
////////////////////////////////////////////////////////////////
void
RenegadeTerrainMaterialPassClass::Allocate (int vertex_count)
{
	//
	//	Allocate the vertex alpha and UV arrays
	//
	VertexCount		= vertex_count;
	VertexAlpha		= new float[vertex_count];
	GridUVs			= new Vector2[vertex_count];

	//
	//	Initialize the arrays
	//
	int index;
	for (index = 0; index < vertex_count; index ++) {
		VertexAlpha[index] = 1.0F;
		GridUVs[index].Set (0.0F, 0.0F);
	}

	
	//
	//	Allocate the vertex index map
	//
	for (index = 0; index < PASS_COUNT; index ++) {
		VertexIndexMap[index] = new int[vertex_count];
		::memset (VertexIndexMap[index], -1, sizeof (int) * vertex_count);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset
//
////////////////////////////////////////////////////////////////
void
RenegadeTerrainMaterialPassClass::Reset (void)
{
	//
	//	Allocate the vertex index map
	//
	for (int index = 0; index < PASS_COUNT; index ++) {
		::memset (VertexIndexMap[index], -1, sizeof (int) * VertexCount);
		QuadList[index].Reset_Active ();
		VertexRenderList[index].Reset_Active ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainMaterialPassClass::Save (ChunkSaveClass &csave)
{
	//
	//	Write the variables
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_VERTEX_COUNT,	VertexCount);
	csave.End_Chunk ();

	//
	//	Save the vertex alpha array
	//
	csave.Begin_Chunk (CHUNKID_VERTEX_ALPHAS);
		csave.Write (VertexAlpha, sizeof (float) * VertexCount);
	csave.End_Chunk ();

	//
	//	Save the grid UVs array
	//
	csave.Begin_Chunk (CHUNKID_VERTEX_UVS);
		csave.Write (GridUVs, sizeof (Vector2) * VertexCount);
	csave.End_Chunk ();

	//
	//	Save the material
	//
	csave.Begin_Chunk (CHUNKID_MATERIAL);
		Material->Save (csave);
	csave.End_Chunk ();

	//
	//	Save the per-pass lists
	//
	int count = 0;
	for (int index = 0; index < PASS_COUNT; index ++) {
		
		//
		//	Save the quad list
		//
		csave.Begin_Chunk (CHUNKID_QUAD_LIST);
			count = QuadList[index].Count ();
			csave.Write (&count, sizeof (int));
			csave.Write (&QuadList[index][0], sizeof (int) * QuadList[index].Count ());
		csave.End_Chunk ();

		//
		//	Save the vertex render list
		//
		csave.Begin_Chunk (CHUNKID_VERTEX_RENDER_LIST);
			count = VertexRenderList[index].Count ();
			csave.Write (&count, sizeof (int));
			csave.Write (&VertexRenderList[index][0], sizeof (int) * VertexRenderList[index].Count ());
		csave.End_Chunk ();

		//
		//	Save the vertex index map
		//
		csave.Begin_Chunk (CHUNKID_VERTEX_INDEX_MAP);
			csave.Write (VertexIndexMap[index], sizeof (int) * VertexCount);
		csave.End_Chunk ();		
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainMaterialPassClass::Load (ChunkLoadClass &cload)
{
	int pass = -1;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			//
			//	Load all the variables from this chunk
			//
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_VERTEX_ALPHAS:
				cload.Read (VertexAlpha, sizeof (float) * VertexCount);
				break;

			case CHUNKID_VERTEX_UVS:
				cload.Read (GridUVs, sizeof (Vector2) * VertexCount);
				break;

			case CHUNKID_MATERIAL:
			{
				//
				//	Load the material
				//
				Material = new TerrainMaterialClass;
				Material->Load (cload);
				break;
			}			

			case CHUNKID_QUAD_LIST:
			{
				pass ++;
				
				//
				//	Read the number of quads
				//
				int count = 0;
				cload.Read (&count, sizeof (int));

				//
				//	Allocate enough room for this many quads and read them in...
				//
				QuadList[pass].Resize (count);
				if (count > 0) {
					cload.Read (&QuadList[pass][0], sizeof (int) * count);
					QuadList[pass].Set_Active (count);
				}
				break;
			}

			case CHUNKID_VERTEX_RENDER_LIST:
			{
				//
				//	Read the number of vertices
				//
				int count = 0;
				cload.Read (&count, sizeof (int));

				//
				//	Allocate enough room for this many vertices and read them in...
				//
				VertexRenderList[pass].Resize (count);
				if (count > 0) {
					cload.Read (&VertexRenderList[pass][0], sizeof (int) * count);
					VertexRenderList[pass].Set_Active (count);
				}
				break;
			}

			case CHUNKID_VERTEX_INDEX_MAP:
				cload.Read (VertexIndexMap[pass], sizeof (int) * VertexCount);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
RenegadeTerrainMaterialPassClass::Load_Variables (ChunkLoadClass &cload)
{
	int vertex_count = 0;

	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			//
			//	Read each of the microchunks
			//
			READ_MICRO_CHUNK (cload, VARID_VERTEX_COUNT,	vertex_count);
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Allocate the necessary data structures
	//
	Allocate (vertex_count);
	return ;
}
