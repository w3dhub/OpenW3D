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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VisSectorSampler.cpp         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/27/01 12:21p                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "VisSectorSampler.h"
#include "SceneEditor.h"
#include "VisGenProgress.h"
#include "rendobj.h"
#include "meshbuild.h"
#include "vector.h"
#include "pscene.h"
#include "mesh.h"
#include "meshmdl.h"
#include "vector3.h"
#include "vector4.h"
#include "vector3i.h"
#include "physcoltest.h"
#include "phys.h"

/*
** Compile time options
*/
#define USE_EDGE_SKIPPING	0


/*
** Constants
*/
const float SHRINKAGE_DISTANCE				= 0.3f;			// amount to move in from each edge
const float FLOOR_SAMPLE_HEIGHT				= 2.0f;			// hieght off the floor for first sample
const float CEILING_CHECK_HEIGHT				= 250.0F;		// how high to look for a ceiling

/**
** SectorEdgeClass
** This is a description of a single edge in the sector mesh
*/
class SectorEdgeClass
{
public:

	SectorEdgeClass(void);
	SectorEdgeClass(const Vector3 & p0,const Vector3 & p1,int p0index,int p1index);
	SectorEdgeClass(const SectorEdgeClass & that);
	SectorEdgeClass & operator = (const SectorEdgeClass & that);
	bool					operator == (const SectorEdgeClass & that) const;
	bool					operator != (const SectorEdgeClass & that) const;

	const Vector3 &	Get_P0(void) const									{ return P0; }
	const Vector3 &	Get_P1(void) const 									{ return P1; }
	const int			Get_P0_Index(void) const							{ return P0Index; }
	const int			Get_P1_Index(void) const							{ return P1Index; }

	void					Set_P0(const Vector3 & p,int index)				{ P0 = p; P0Index = index; }
	void					Set_P1(const Vector3 & p,int index)				{ P1 = p; P1Index = index; }

	void					Increment_Instance_Count(void)					{ Counter++; }
	int					Get_Instance_Count(void)							{ return Counter; }
protected:
	
	Vector3				P0;
	Vector3				P1;
	int					P0Index;
	int					P1Index;

	int					Counter;
};	

/**
** SectorEdgeTableClass
** This class is used to build the table of "outer" edges for a vis sector.  Basically the
** goal is to have a list of all of the edges that apear in a mesh only once.  
*/
class SectorEdgeTableClass : public DynamicVectorClass<SectorEdgeClass>
{
public:

	SectorEdgeTableClass(int count) : DynamicVectorClass<SectorEdgeClass> (count)		{ }
	~SectorEdgeTableClass(void)																		{ }

	void Add_Edge_Unique(const SectorEdgeClass & edge);	

};


/************************************************************************************************
**
** SectorEdgeClass Implementation
**
************************************************************************************************/
SectorEdgeClass::SectorEdgeClass(void) :
	P0(0,0,0),
	P1(0,0,0),
	P0Index(0),
	P1Index(0),
	Counter(1)
{
}

SectorEdgeClass::SectorEdgeClass(const Vector3 & p0,const Vector3 & p1,int p0index,int p1index) :
	P0(p0),
	P1(p1),
	P0Index(p0index),
	P1Index(p1index),
	Counter(1)
{
}

SectorEdgeClass::SectorEdgeClass(const SectorEdgeClass & that)
{
	*this = that;
}

SectorEdgeClass & SectorEdgeClass::operator = (const SectorEdgeClass & that)
{
	if (this != &that) {
		P0 = that.P0;
		P1 = that.P1;
		P0Index = that.P0Index;
		P1Index = that.P1Index;
		Counter = 1;
	}
	return *this;
}

bool SectorEdgeClass::operator == (const SectorEdgeClass & that) const
{
	return (	((P0Index == that.P0Index) && (P1Index == that.P1Index)) ||
				((P0Index == that.P1Index) && (P1Index == that.P0Index))  );
}

bool SectorEdgeClass::operator != (const SectorEdgeClass & that) const
{
	return !(*this == that);
}

/************************************************************************************************
**
** SectorEdgeTableClass Implementation
**
************************************************************************************************/
void SectorEdgeTableClass::Add_Edge_Unique(const SectorEdgeClass & edge)
{
	/*
	** See if we already have this edge in the table.
	*/
	for (int i=0; i<Count(); i++) {
		if (edge == (*this)[i]) {
			(*this)[i].Increment_Instance_Count();
			return;
		}
	}
	
	/*
	** If we fall through to here, add the edge to our array
	*/
	Add(edge);
}


/************************************************************************************************
**
** VisSectorSamplerClass Implementation
**
************************************************************************************************/
VisSectorSamplerClass::VisSectorSamplerClass
(
	SceneEditorClass * scene,
	VisGenProgressClass * stats,
	float min_sample_distance,
	int collision_group
) :
	Scene(NULL),
	MeshBuilder(NULL),
	Stats(stats),
	MinSampleDistance(min_sample_distance),
	CollisionGroup(collision_group),
	EdgeSkipAccum(0.0f)
{
	REF_PTR_SET(Scene,scene);
	MeshBuilder = new MeshBuilderClass;
}

VisSectorSamplerClass::~VisSectorSamplerClass(void)
{
	if (MeshBuilder != NULL) {
		delete MeshBuilder;
		MeshBuilder = NULL;
	}
	REF_PTR_RELEASE(Scene);
}

void VisSectorSamplerClass::Process(RenderObjClass * model)
{
	Reset(model->Get_Num_Polys());
	
	int count = Collect_Polygons(model);
	if (count > 0) {
		Sample_Edges();
	} else {
		WWDEBUG_SAY(("Vis-sector %s had no up-facing polygons!\r\n",model->Get_Name()));
	}

	Stats->Increment_Processed_Node_Count();
}
	
void VisSectorSamplerClass::Reset(int poly_count)
{
	MeshBuilder->Reset(1,poly_count,0.5f*poly_count + 1);
}

int VisSectorSamplerClass::Collect_Polygons(RenderObjClass * renderobj)
{
	WWASSERT(renderobj != NULL);
	int poly_count = 0;

	/*
	** If this render object is a mesh and is marked as a VIS sector, then
	** collect upward-facing polygons and submit them into the mesh builder
	*/
	if (	(renderobj->Get_Collision_Type () & COLLISION_TYPE_VIS) &&
			(renderobj->Class_ID() == RenderObjClass::CLASSID_MESH) )
	{
		MeshModelClass * model = ((MeshClass *)renderobj)->Get_Model();
		if (model != NULL) {		

			MeshBuilderClass::FaceClass face;
			
			/*
			** Grab the relevent data from the mesh.
			*/
			const TriIndex *poly_array		= model->Get_Polygon_Array ();
			const Vector3 *vertex_array	= model->Get_Vertex_Array ();
			const Vector4 *plane_array		= model->Get_Plane_Array (true);

			/*
			** Add each up-facing polygon into our builder
			*/
			for (int pi = 0; pi < model->Get_Polygon_Count(); pi++) {
				
				if (plane_array[pi].Z > 0.3f) {
					
					/*
					** Copy this face into the builder
					*/
					for (int vi=0; vi<3; vi++) {
						face.Verts[vi].Position = vertex_array[poly_array[pi][vi]];
					}

					MeshBuilder->Add_Face(face);
					poly_count++;
				}
			}
		}
	}

	/*
	** Recurse into all sub-meshes
	*/
	int count = renderobj->Get_Num_Sub_Objects();
	for (int index = 0; index < count; index ++) {
		RenderObjClass *sub_object = renderobj->Get_Sub_Object(index);
		if (sub_object != NULL) {
			poly_count += Collect_Polygons(sub_object);
			REF_PTR_RELEASE(sub_object);
		}
	}

	return poly_count;
}

void VisSectorSamplerClass::Sample_Edges(void)
{
	/*
	** Tell the mesh builder to process its input.
	*/
	MeshBuilder->Build_Mesh(false);

	/*
	** Build an array of all of the edges, marking duplicates for removal
	*/
	SectorEdgeTableClass edgetable(3 * MeshBuilder->Get_Face_Count());

	for (int fi=0; fi<MeshBuilder->Get_Face_Count(); fi++) {
		const MeshBuilderClass::FaceClass & face = MeshBuilder->Get_Face(fi);
		for (int vi=0; vi<3; vi++) {
			int v0 = vi;
			int v1 = (vi + 1) % 3;
			const MeshBuilderClass::VertClass & vertex0 = MeshBuilder->Get_Vertex(face.VertIdx[v0]);
			const MeshBuilderClass::VertClass & vertex1 = MeshBuilder->Get_Vertex(face.VertIdx[v1]);
			edgetable.Add_Edge_Unique(SectorEdgeClass(vertex0.Position,vertex1.Position,face.VertIdx[v0],face.VertIdx[v1]));
		}
	}

	/*
	** For each mesh that has an instance count of 1, recursively sample vis along it
	*/
	for (int ei=0; ei<edgetable.Count(); ei++) {
		if (edgetable[ei].Get_Instance_Count() == 1) {
			
			Vector3 edge_dir = edgetable[ei].Get_P1() - edgetable[ei].Get_P0();
			edge_dir.Z = 0.0f;
			float edge_len = edge_dir.Length();
			edge_dir /= edge_len;
			
			if (edge_len > 0.0f) {
			
				/*
				** Skip the edge if it is short
				*/
#if (USE_EDGE_SKIPPING)
				if (edge_len + EdgeSkipAccum < MinSampleDistance) { 

					EdgeSkipAccum += edge_len;

				} else {
#endif	
					EdgeSkipAccum = 0.0f;

					/*
					** Move "inward" and up in Z to hopefully generate valid points which don't
					** cause the camera to intersect walls.
					*/
					Vector3 offset(-edge_dir.Y,edge_dir.X,0.0f);		
					offset *= SHRINKAGE_DISTANCE;
					offset.Z += FLOOR_SAMPLE_HEIGHT;

					Vector3 p0 = edgetable[ei].Get_P0() + offset;
					Vector3 p1 = edgetable[ei].Get_P1() + offset;
					Sample_Edge(p0,p1);
					Stats->Increment_Edge_Count();
#if (USE_EDGE_SKIPPING)
				}
#endif
			}
		}

		if (Stats->Is_Cancel_Requested()) {
			return;
		}
	}
}


void VisSectorSamplerClass::Sample_Edge(const Vector3 & p0,const Vector3 & p1)
{
	/*
	** Perform a vis sample at the center of the edge both near the ground
	** and at the maximum height.
	*/
	int bits_changed = 0;
	Vector3 sample_point = 0.5f * (p0 + p1);

	bits_changed = Sample_Point(sample_point);

	/*
	** Should we subdivide this edge and keep sampling?
	*/
	if (	(bits_changed > 0) && 
			((p1-p0).Quick_Length() > 2.0f*MinSampleDistance) ) 
	{
		Sample_Edge(p0,sample_point);
		Sample_Edge(sample_point,p1);
	}
}

int VisSectorSamplerClass::Sample_Point(const Vector3 & point)
{
	int bits_changed = 0;
	float ceiling_distance = 0.0f;

	/*
	** Check the ceiling height
	*/
	if ((Check_Ceiling(point,&ceiling_distance) == true) && (ceiling_distance > 1.0f)) {
	
		if (ceiling_distance > 20.0f) {
			ceiling_distance = 20.0f;
		}

		Vector3 ceiling_point = point;
		ceiling_point.Z += ceiling_distance - 0.3f;

		/*
		** Now, sample the bottom and top of the vertical segment, recording
		** the amount of changes made to the vis vector
		*/	
		int bits0 = 0,bits1 = 0;
		bits0 = Update_Vis(point);
		if (bits0 > 0) {
			bits1 = Update_Vis(ceiling_point);
		}
		bits_changed += bits0 + bits1;

		/*
		** Recursively continue to sample vertically until we are making no
		** more changes or the points get too close together
		*/
		if (bits1 > 0) {
			bits_changed += Sample_Vertical_Segment(point,ceiling_point);
		}
	}
	
	return bits_changed;
}


int VisSectorSamplerClass::Sample_Vertical_Segment(const Vector3 & p0,const Vector3 & p1)
{
	Vector3 sample_point = 0.5f * (p0 + p1);
	int bits_changed = Update_Vis(sample_point);

	if (	(bits_changed > 0) && 
			(p1.Z-p0.Z > 2.0f*MinSampleDistance) ) 
	{
		bits_changed += Sample_Vertical_Segment(p0,sample_point);
		bits_changed += Sample_Vertical_Segment(sample_point,p1);
	}
	return bits_changed;
}

int VisSectorSamplerClass::Update_Vis(const Vector3 & point)
{
	/*
	** Perform the vis sample
	*/
	Matrix3D transform(Matrix3(1),point);
	VisSampleClass sample = Scene->Update_Vis(point,transform);
	Stats->Increment_Sample_Count();
	
	/*
	** Log the results with the scene editor
	*/
	VisLogClass &vis_log = Scene->Get_Vis_Log();
	vis_log.Log_Sample(sample);
	Scene->Create_Vis_Point(transform);

	/*
	** Return the number of bits changed by this sample
	*/
	if (sample.Sample_Rejected()) {
		return 0;
	} else {
		return sample.Get_Bits_Changed();
	}
}


/////////////////////////////////////////////////////////////////////////
//
//	Check_Ceiling
//
//	This method casts a ray straight up from the candiate vis-point
// and checks to see if the object it hits (if any) is a valid
// vis 'ceiling'.  This makes sure we don't generate points that are
// inside of hills or mountains.
//
/////////////////////////////////////////////////////////////////////////
bool VisSectorSamplerClass::Check_Ceiling (const Vector3 &position, float *ceiling_dist)
{
	//
	//	This method casts a ray straight up from the candiate vis-point
	// and checks to see if the object it hits (if any) is a valid
	// vis 'ceiling'.  This makes sure we don't generate points that are
	// inside of hills or mountains.
	//
	bool retval = true;

	//
	//	Build a ray from the given position up 100 meters in the air.
	//
	Vector3 start_point = position + Vector3 (0, 0, 0.001F);
	Vector3 end_point = position + Vector3 (0, 0, CEILING_CHECK_HEIGHT);
	LineSegClass ray (start_point, end_point);
	
	//
	// Cast the ray into the world and see what it hits.
	//
	CastResultStruct res;	
	PhysRayCollisionTestClass raytest (ray, &res, CollisionGroup, COLLISION_TYPE_PHYSICAL);
	Scene->Cast_Ray (raytest);

	//
	//	Return how far above us the ceiling is.
	//
	if (ceiling_dist != NULL) {
		(*ceiling_dist) = (res.Fraction * (end_point.Z - start_point.Z));
	}

	//
	//	Did we hit anything?
	//
	if (res.Fraction < 1.0F) {

		// Get the physics object we hit
		PhysClass *physobj = raytest.CollidedPhysObj;
		if ((physobj != NULL) && (physobj->As_StaticPhysClass() != NULL)) {

			// If this polygon is facing up, then make sure it satisfies
			// our 'ceiling' requirements for back-face polygons.
			if ((res.Normal.Z > 0) &&
				 Is_Object_Invalid_Roof (physobj->Peek_Model ()))
			{
				retval = false;
			}
		}
	}	

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Is_Object_Invalid_Roof
//
//	This method determines if it is valid for a vis-point to be generated
// underneath a given object.
//
//	An object is considered 'invalid' for a vis-point's 'roof' if:
//
//		- The mesh's polygons are visible.
//		- The mesh's polygons are single-sided
//		- The mesh's polygons can be physically collideable.
//		- The mesh is marked for vis generation itself.
//
/////////////////////////////////////////////////////////////////////////
bool VisSectorSamplerClass::Is_Object_Invalid_Roof(RenderObjClass *render_obj)
{
	bool retval = true;

	//
	// Recursively check all sub-objects
	//
	int count = render_obj->Get_Num_Sub_Objects ();
	for (int index = 0; (index < count) && retval; index ++) {
		
		//
		// Check this subobject
		//
		RenderObjClass *sub_object = render_obj->Get_Sub_Object (index);
		if (sub_object != NULL) {			
			retval &= Is_Object_Invalid_Roof (sub_object);
			REF_PTR_RELEASE(sub_object);
		}
	}	

	
	//
	// Is this render object a mesh?
	//
	if (render_obj->Class_ID () == RenderObjClass::CLASSID_MESH) {
		MeshModelClass *model = ((MeshClass *)render_obj)->Get_Model ();
		if (model != NULL) {

			//
			//	The mesh is invalid if:
			//
			//		a) The mesh's polys are single-sided AND
			//		b) The mesh is a vis-sector AND
			//		c) The mesh is physically collideable AND
			//		d) The mesh visible
			//
			retval &= (model->Get_Flag (MeshModelClass::TWO_SIDED) != MeshModelClass::TWO_SIDED);
			//retval &= ((render_obj->Get_Collision_Type () & COLLISION_TYPE_VIS) == COLLISION_TYPE_VIS);
			retval &= ((render_obj->Get_Collision_Type () & COLLISION_TYPE_PHYSICAL) == COLLISION_TYPE_PHYSICAL);
			retval &= (render_obj->Is_Not_Hidden_At_All () != 0);

			REF_PTR_RELEASE(model);
		}		
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Do_View_Planes_Pass
//
//	This method checks the view-plane in each of the six directions around
// the candidate point and determines if any of them intersect a 'wall'.
//
/////////////////////////////////////////////////////////////////////////
bool VisSectorSamplerClass::Do_View_Planes_Pass (const Matrix3D &/* vis_transform */)
{
	bool retval = true;

#if 0 // TODO!
	Vector3 center = vis_transform.Get_Translation ();
	Matrix3 orig_basis (vis_transform);
	
	//
	//	Loop through and test each of the 6 orienations
	// of the view plane to make sure none of them intersect
	// a 'wall'.
	//	
	/*CastResultStruct result;
	for (int index = 0; (index < VIS_RENDER_DIRECTIONS) && retval; index ++) {

		//
		//	Build the orientation of the view-plane
		//
		Matrix3 basis = orig_basis;
		basis.Rotate_X (VIS_RENDER_TABLE[index].X);
		basis.Rotate_Y (VIS_RENDER_TABLE[index].Y);
		basis.Rotate_Z (VIS_RENDER_TABLE[index].Z);*/

		//
		//	Create a box representing the view plane
		//
		//Vector3
		AABoxClass box (center, m_ViewPlaneExtent);
		
		//
		// Check to see if this viewplane 'box' collides
		// with anything.
		//
		//result.Reset ();
		CastResultStruct result;
		PhysAABoxCollisionTestClass collision_test (box, Vector3(0,0,0), &result, CollisionGroup, COLLISION_TYPE_PHYSICAL);
		Scene->Cast_AABox (collision_test);
		retval = (result.StartBad == false) && !(result.Fraction < 1.0F);
	//}
#endif

	return retval;
}
