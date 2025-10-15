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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/heightdb.cpp                          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/19/01 1:48p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "heightdb.h"
#include "pscene.h"
#include "coltest.h"
#include "phys.h"
#include "physcoltest.h"
#include "mesh.h"
#include "meshmdl.h"
#include "chunkio.h"
#include <algorithm>


/////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////
const float	HEIGHT_OFFSET	= 2.0F;

enum
{
	CHUNKID_VARIABLES			= 0x07310202,
	CHUNKID_HEIGHT_ARRAY
};

enum
{
	VARID_NUM_POINTS_X		= 0x01,
	VARID_NUM_POINTS_Y,
	VARID_PATCH_SIZE,
	VARID_LEV_MIN,
	VARID_LEV_MAX
};


/////////////////////////////////////////////////////////////////////////
//	Static member initialization
/////////////////////////////////////////////////////////////////////////
float *	HeightDBClass::m_HeightArray	= NULL;
int		HeightDBClass::m_NumPointsX	= 0;
int		HeightDBClass::m_NumPointsY	= 0;
float		HeightDBClass::m_PatchSize		= 8;
Vector3	HeightDBClass::m_LevelMin (0, 0, 0);
Vector3	HeightDBClass::m_LevelMax (0, 0, 0);


/////////////////////////////////////////////////////////////////////////
//
//	HeightDBClass
//
/////////////////////////////////////////////////////////////////////////
HeightDBClass::HeightDBClass (void)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	~HeightDBClass
//
/////////////////////////////////////////////////////////////////////////
HeightDBClass::~HeightDBClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
/////////////////////////////////////////////////////////////////////////
void
HeightDBClass::Initialize (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
/////////////////////////////////////////////////////////////////////////
void
HeightDBClass::Shutdown (void)
{
	Free_Data ();
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Get_Height
//
/////////////////////////////////////////////////////////////////////////
float
HeightDBClass::Get_Height (const Vector3 &pos)
{
	float height = pos.Z;

	if (m_HeightArray != NULL && m_NumPointsX > 0 && m_NumPointsY > 0) {

		float percent_x = (pos.X - m_LevelMin.X) / (m_LevelMax.X - m_LevelMin.X);
		float percent_y = (pos.Y - m_LevelMin.Y) / (m_LevelMax.Y - m_LevelMin.Y);

		//
		//	Is the position inside our data set?
		//
		if (percent_x >= 0 && percent_x <= 1.0F && percent_y >= 0 && percent_y <= 1.0F) {
			
			int entry_ul_x = (m_NumPointsX - 1) * percent_x;
			int entry_ul_y = (m_NumPointsY - 1) * percent_y;
			int entry_ur_x = entry_ul_x + 1;
			int entry_ur_y = entry_ul_y;
			int entry_lr_x = entry_ul_x + 1;
			int entry_lr_y = entry_ul_y + 1;
			int entry_ll_x = entry_ul_x;
			int entry_ll_y = entry_ul_y + 1;

			if (entry_ul_x + 1 >= m_NumPointsX) {
				entry_ur_x = entry_ul_x;
				entry_lr_x = entry_ul_x;
			}

			if (entry_ul_y + 1 >= m_NumPointsY) {
				entry_lr_y = entry_ul_y;
				entry_ll_y = entry_ul_y;
			}

			float *ul_entry = Get_Height_Entry (entry_ul_y, entry_ul_x);
			float *ur_entry = Get_Height_Entry (entry_ur_y, entry_ur_x);
			float *lr_entry = Get_Height_Entry (entry_lr_y, entry_lr_x);
			float *ll_entry = Get_Height_Entry (entry_ll_y, entry_ll_x);
						
			float local_per_x = ((pos.X - m_LevelMin.X) - (entry_ul_x * m_PatchSize)) / m_PatchSize;
			float local_per_y = ((pos.Y - m_LevelMin.Y) - (entry_ul_y * m_PatchSize)) / m_PatchSize;

			//
			//	Take the weighted average of the 4 corner values
			//
			float h1 = (1 - local_per_x)	* (1 - local_per_y)	* (*ul_entry);
			float h2 = (local_per_x)		* (1 - local_per_y)	* (*ur_entry);
			float h3 = (local_per_x)		* (local_per_y)		* (*lr_entry);
			float h4 = (1 - local_per_x)	* (local_per_y)		* (*ll_entry);

			height = h1 + h2 + h3 + h4;
		}		
	}

	return height;
}


/////////////////////////////////////////////////////////////////////////
//
//	Generate
//
/////////////////////////////////////////////////////////////////////////
void
HeightDBClass::Generate (void)
{
	Free_Data ();

	PhysicsSceneClass::Get_Instance ()->Get_Level_Extents (m_LevelMin, m_LevelMax);

	float generate_patch_size = m_PatchSize / 2;

	//
	//	Determine how big a heightmap we will need to store data for this level
	//
	m_NumPointsX	= ((m_LevelMax.X - m_LevelMin.X) / generate_patch_size) + 1;
	m_NumPointsY	= ((m_LevelMax.Y - m_LevelMin.Y) / generate_patch_size) + 1;
	int entries		= m_NumPointsX * m_NumPointsY;

	m_HeightArray	= new float[entries];

	//
	//	Build a heightmap by casting rays through the level vertically at each grid intersection
	//
	int row = 0;
	int col = 0;
	for (float y_pos = m_LevelMin.Y; y_pos < m_LevelMax.Y; y_pos += generate_patch_size) {
		col = 0;
		for (float x_pos = m_LevelMin.X; x_pos < m_LevelMax.X; x_pos += generate_patch_size) {

			float z_pos		= m_LevelMin.Z;
			float start_z	= m_LevelMax.Z + 250.0F;
			float end_z		= m_LevelMin.Z - 250.0F;

			//
			//	Setup a raycast at this (x,y) position that goes through the world
			//
			CastResultStruct result;
			LineSegClass ray (Vector3 (x_pos, y_pos, start_z), Vector3 (x_pos, y_pos, end_z));
			PhysRayCollisionTestClass raytest (ray, &result, 0, COLLISION_TYPE_PROJECTILE);
			PhysicsSceneClass::Get_Instance ()->Cast_Ray (raytest);

			//
			//	Return a pointer to the collided physics object if the
			// cast hit something
			//
			if (result.Fraction < 1.0F) {
				z_pos = start_z + (end_z - start_z) * result.Fraction;
			}

			//
			//	Store this height value in our array
			//
			float *height_value = Get_Height_Entry (row, col);
			if (height_value != NULL) {
				(*height_value) = (z_pos + HEIGHT_OFFSET);
			}
				
			//
			//	Move over one columne
			//
			col ++;
		}

		//
		//	Move down one row
		//
		row ++;
	}

	Examine_Level_Geometry ();
	
	//
	//	Now average out the height values so we don't get large jumps in height
	//
	for (row = 1; row < m_NumPointsY - 1; row ++) {
		for (col = 1; col < m_NumPointsX - 1; col ++) {

			//
			//	Lookup the height values of the 9 entries surrounding the current entry
			//
			float *z_val1 = Get_Height_Entry (row - 1, col - 1);
			float *z_val2 = Get_Height_Entry (row - 1, col);
			float *z_val3 = Get_Height_Entry (row - 1, col + 1);

			float *z_val4 = Get_Height_Entry (row, col - 1);
			float *z_val5 = Get_Height_Entry (row, col);
			float *z_val6 = Get_Height_Entry (row, col + 1);

			float *z_val7 = Get_Height_Entry (row + 1, col - 1);
			float *z_val8 = Get_Height_Entry (row + 1, col);
			float *z_val9 = Get_Height_Entry (row + 1, col + 1);

			//
			//	Average the 9 height values
			//
			float average = (	*z_val1 + *z_val2 + *z_val3 +
									*z_val4 + *z_val5 + *z_val6 +
									*z_val7 + *z_val8 + *z_val9) / 9.0F;

			//
			//	Now store the larger of the old value and the average in each of these
			// 9 entries
			//
			float edge_avg = average * 0.98F;
			*z_val1 = std::max (*z_val1, edge_avg);
			*z_val2 = std::max (*z_val2, edge_avg);
			*z_val3 = std::max (*z_val3, edge_avg);

			*z_val4 = std::max (*z_val4, edge_avg);
			*z_val5 = std::max (*z_val5, average);
			*z_val6 = std::max (*z_val6, edge_avg);

			*z_val7 = std::max (*z_val7, edge_avg);
			*z_val8 = std::max (*z_val8, edge_avg);
			*z_val9 = std::max (*z_val9, edge_avg);
		}
	}

	int temp_points_x = m_NumPointsX / 2;
	int temp_points_y = m_NumPointsY / 2;
	float *temp_height_array = new float[temp_points_x * temp_points_y];

	for (row = 0; row < temp_points_y; row ++) {
		for (col = 0; col < temp_points_x; col ++) {
			float *z_val = Get_Height_Entry (row * 2, col * 2);
			if (z_val != NULL) {
				temp_height_array[(row * temp_points_x) + col] = *z_val;
			} else {
				temp_height_array[(row * temp_points_x) + col] = 0;
			}
		}
	}

	Free_Data ();
	m_HeightArray	= temp_height_array;
	m_NumPointsX	= temp_points_x;
	m_NumPointsY	= temp_points_y;
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Free_Data
//
/////////////////////////////////////////////////////////////////////////
void
HeightDBClass::Free_Data (void)
{
	if (m_HeightArray != NULL) {
		delete [] m_HeightArray;
		m_HeightArray = NULL;
	}

	m_NumPointsX = 0;
	m_NumPointsY = 0;	
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Examine_Level_Geometry
//
/////////////////////////////////////////////////////////////////////////
void
HeightDBClass::Examine_Level_Geometry (void)
{
	RefPhysListIterator it1 = PhysicsSceneClass::Get_Instance ()->Get_Static_Object_Iterator ();
	RefPhysListIterator it2 = PhysicsSceneClass::Get_Instance ()->Get_Static_Anim_Object_Iterator ();
	
	//
	//	Process the static meshes
	//
	for (it1.First (); !it1.Is_Done (); it1.Next ()) {
		PhysClass *phys_obj = it1.Peek_Obj ();
		if (phys_obj != NULL) {
			RenderObjClass *model = phys_obj->Peek_Model ();
			if (model != NULL) {
				Process_Render_Obj (model);
			}
		}
	}

	//
	//	Process the static animated meshes
	//
	for (it2.First (); !it2.Is_Done (); it2.Next ()) {
		PhysClass *phys_obj = it2.Peek_Obj ();
		if (phys_obj != NULL) {
			RenderObjClass *model = phys_obj->Peek_Model ();
			if (model != NULL) {
				Process_Render_Obj (model);
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Process_Render_Obj
//
/////////////////////////////////////////////////////////////////////////
void
HeightDBClass::Process_Render_Obj (RenderObjClass *render_obj)
{
	//
	// Loop through all the render objects sub-objects
	//
	int count = render_obj->Get_Num_Sub_Objects ();
	for (int index = 0; index < count; index ++) {
		
		// Get a pointer to this subobject
		RenderObjClass *sub_object = render_obj->Get_Sub_Object (index);
		if (sub_object != NULL) {
			
			Process_Render_Obj (sub_object);
			sub_object->Release_Ref ();
		}
	}	

	//
	// Is this render object a mesh?
	//
	if ((render_obj->Class_ID () == RenderObjClass::CLASSID_MESH) &&
		 (render_obj->Get_Collision_Type () & COLLISION_TYPE_PHYSICAL))
	{
		//
		//	Pass this mesh off to the generator
		//
		MeshClass *mesh = static_cast<MeshClass *> (render_obj);
		Submit_Mesh (*mesh);
	}	

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Submit_Mesh
//
/////////////////////////////////////////////////////////////////////////
void
HeightDBClass::Submit_Mesh (MeshClass &mesh)
{
	//
	//	Get this mesh's polygon information
	//
	MeshModelClass *model = mesh.Get_Model ();
	if (model != NULL) {		
		
		const Vector3 *vertex_array	= model->Get_Vertex_Array ();
		int count							= model->Get_Vertex_Count ();

		//
		//	Loop over all the vertices in the mesh, looking for the
		// highest z-value in each cell of our grid
		//
		for (int index = 0; index < count; index ++) {
			Vector3 vertex = mesh.Get_Transform () * vertex_array[index];

			float percent_x = (vertex.X - m_LevelMin.X) / (m_LevelMax.X - m_LevelMin.X);
			float percent_y = (vertex.Y - m_LevelMin.Y) / (m_LevelMax.Y - m_LevelMin.Y);

			//
			//	Is the position inside our data set?
			//
			if (percent_x >= 0 && percent_x <= 1.0F && percent_y >= 0 && percent_y <= 1.0F) {
				
				int entry_ul_x = (m_NumPointsX - 1) * percent_x;
				int entry_ul_y = (m_NumPointsY - 1) * percent_y;
				int entry_ur_x = entry_ul_x + 1;
				int entry_ur_y = entry_ul_y;
				int entry_lr_x = entry_ul_x + 1;
				int entry_lr_y = entry_ul_y + 1;
				int entry_ll_x = entry_ul_x;
				int entry_ll_y = entry_ul_y + 1;

				if (entry_ul_x + 1 >= m_NumPointsX) {
					entry_ur_x = entry_ul_x;
					entry_lr_x = entry_ul_x;
				}

				if (entry_ul_y + 1 >= m_NumPointsY) {
					entry_lr_y = entry_ul_y;
					entry_ll_y = entry_ul_y;
				}

				float *ul_entry = Get_Height_Entry (entry_ul_y, entry_ul_x);
				float *ur_entry = Get_Height_Entry (entry_ur_y, entry_ur_x);
				float *lr_entry = Get_Height_Entry (entry_lr_y, entry_lr_x);
				float *ll_entry = Get_Height_Entry (entry_ll_y, entry_ll_x);

				//
				//	Now, move each of the corner entries up to this new z-value if necessary
				//
				(*ul_entry) = std::max (*ul_entry, vertex.Z + HEIGHT_OFFSET);
				(*ur_entry) = std::max (*ur_entry, vertex.Z + HEIGHT_OFFSET);
				(*lr_entry) = std::max (*lr_entry, vertex.Z + HEIGHT_OFFSET);
				(*ll_entry) = std::max (*ll_entry, vertex.Z + HEIGHT_OFFSET);
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////
bool
HeightDBClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;

	//
	//	Save the variables to their own chunk
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);	

		WRITE_MICRO_CHUNK (csave, VARID_NUM_POINTS_X,	m_NumPointsX);
		WRITE_MICRO_CHUNK (csave, VARID_NUM_POINTS_Y,	m_NumPointsY);
		WRITE_MICRO_CHUNK (csave, VARID_PATCH_SIZE,		m_PatchSize);
		WRITE_MICRO_CHUNK (csave, VARID_LEV_MIN,			m_LevelMin);
		WRITE_MICRO_CHUNK (csave, VARID_LEV_MAX,			m_LevelMax);
	
	csave.End_Chunk ();

	//
	//	Write the height data to a chunk
	//
	csave.Begin_Chunk (CHUNKID_HEIGHT_ARRAY);
		int entries = m_NumPointsX * m_NumPointsY;
		csave.Write (m_HeightArray, entries * sizeof (float));
	csave.End_Chunk ();

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////
bool
HeightDBClass::Load (ChunkLoadClass &cload)
{
	Free_Data ();

	bool retval = true;	
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			//
			//	Read the height data from the chunk
			//
			case CHUNKID_HEIGHT_ARRAY:
			{
				//
				//	Allocate enough entries to hold the height data
				//
				int entries = m_NumPointsX * m_NumPointsY;
				m_HeightArray = new float[entries];
				cload.Read (m_HeightArray, entries * sizeof (float));
			}
			break;

			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
HeightDBClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_NUM_POINTS_X,	m_NumPointsX);
			READ_MICRO_CHUNK (cload, VARID_NUM_POINTS_Y,	m_NumPointsY);
			READ_MICRO_CHUNK (cload, VARID_PATCH_SIZE,	m_PatchSize);
			READ_MICRO_CHUNK (cload, VARID_LEV_MIN,		m_LevelMin);
			READ_MICRO_CHUNK (cload, VARID_LEV_MAX,		m_LevelMax);
		}

		cload.Close_Micro_Chunk ();
	}

	return retval;
}
