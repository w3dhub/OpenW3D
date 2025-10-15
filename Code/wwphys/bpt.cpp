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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/bpt.cpp                               $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 9/09/99 11:29a                                              $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   BptClass::BptClass -- constructor                                                         *
 *   BptClass::~BptClass -- destructor                                                         *
 *   BptClass::Free -- releases all assets in use                                              *
 *   BptClass::Build -- build a bpt from the given mesh (generates a new mesh)                 *
 *   BptClass::Load -- Initialize this Bpt object from contents of a W3D file                  *
 *   BptClass::Save -- Save this Bpt object into a W3D file                                    *
 *   BptClass::Render -- submit this object for rendering                                      *
 *   BptClass::Cast_Ray -- Intersect a ray with this Bpt Object                                *
 *   BptClass::Cast_AABox -- Intersect a swept axis-aligned box.                               *
 *   BptClass::Cast_OBBox -- Intersect a swept oriented box with this bpt object               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if 0 // OBSOLETE!!!
#ifdef PORT140

#include "bpt.h"
#include "vector.h"
#include "uarray.h"
#include "mesh.h"
#include "material.h"
#include "chunkio.h"
#include "vector3.h"
#include "tri.h"
#include "wwdebug.h"
#include "meshbuild.h"
#include "camera.h"
#include "sr.hpp"
#include <algorithm>


#define BPT_FRONT						0x01
#define BPT_BACK						0x02
#define BPT_ON							0x04
#define BPT_BOTH						0x08
#define BPT_EPSILON					0.0001f
#define BPT_COINCIDENCE_EPSILON	0.000001f

class BptVertexClass;
class BptPolyClass;
class BptNodeClass;
class BptBuilderClass;
class BptImpNodeClass;
class BptImpClass;
class BptImpBuilderClass;



/*
** NormalHasherClass, used by the code which builds the
** table of unique Normal vectors.
*/
class NormalHasherClass : public HashCalculatorClass<Vector3>
{
public:

	virtual bool	Items_Match(const Vector3 & a, const Vector3 & b) 
	{
		// looking for an exact match for normals...
		return ((a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z));
	}

	virtual void	Compute_Hash(const Vector3 & item)
	{
		HashVal = (int)(item.X*12345.6f + item.Y*1714.38484f + item.Z*27561.3f)&1023; 
	}

	virtual int		Num_Hash_Bits(void) 
	{ 
		return 10;  // 10bit hash value. 
	}
	
	virtual int		Num_Hash_Values(void)
	{
		return 1;	// only one hash value for normals, require *exact* match
	}
	
	virtual int		Get_Hash_Value(int index)
	{
		return HashVal;
	}

private:
	
	int HashVal;

};


/*
** DistanceHasherClass, used by the code which builds the
** table of unique distance values.
*/
class DistanceHasherClass : public HashCalculatorClass<float>
{
	virtual bool	Items_Match(const float & a, const float & b) 
	{
		// looking for an exact match for distances
		return (a == b);
	}

	virtual void	Compute_Hash(const float & item)
	{
		HashVal = (int)(item)&1023; 
	}

	virtual int		Num_Hash_Bits(void) 
	{ 
		return 12;  // 12bit hash value. 
	}
	
	virtual int		Num_Hash_Values(void)
	{
		return 1;	// only one hash value for normals, require *exact* match
	}

	virtual int		Get_Hash_Value(int index) 
	{
		return HashVal;
	}

private:

	int HashVal;

};



 
/*
** BPT Notes
**
** - Probably need to build the initial tree in a big straightforward recursive datastructure
**   with no indirection or anything.  Then, when the whole thing is being built correctly,
**   process the tree down so that everything that can be shared (e.g. plane equations) are
**   shared.
**
** - Hide most of the implementation details even from the bpt header file by using a
**   BptImpClass.  This class will contain the final "optimized" bpt information.
**
** - Maybe need a BptBuilder class that builds the initial "straightforward" tree structure
**   and then compresses it into a BptImp.
**
** - Extend PlaneClass to know about special case planes such as x-y, y-z, x-z.  Optimize
**   the culling and clipping functions to take advantage of them.
** 
** - Separate the normal from the distance in the plane equation?  Many many planes will
**   probably share the same normal but different distances.  This probably should be a
**   very late optimization.
*/


/*
** BptVertexClass
** This is the vertex representation used in building the BPT
*/
class BptVertexClass
{
public:

	BptVertexClass(void);
	BptVertexClass(const BptVertexClass & that);

	BptVertexClass &	operator = (const BptVertexClass & that);
	
	int					Which_Side(const PlaneClass & plane) const;

	Vector3				Position;
	Vector3				Normal;
	Vector3				Color;
	Vector3				TexCoord;
	
	static BptVertexClass Lerp(const BptVertexClass & v0,const BptVertexClass & v1,double lerp);
	static BptVertexClass Intersect_Plane(const BptVertexClass & v0,const BptVertexClass & v1,const PlaneClass & plane);

};


/*
** BptPolyClass
** This is the polygon representation used in building the BPT
*/
class BptPolyClass
{
public:

	enum { BPT_POLY_MAX_VERTS = 16 };

	BptPolyClass(void);
	BptPolyClass(const BptPolyClass & that);
	BptPolyClass(const BptVertexClass * points, int num);

	BptPolyClass &				operator = (const BptPolyClass & that);
	
	const BptVertexClass &	operator[] (int i) const { return Verts[i]; }
	BptVertexClass &			operator[] (int i) { return Verts[i]; }

	void							Compute_Plane(void);
	int							Which_Side(const PlaneClass & plane) const;
	void							Split(const PlaneClass & plane,BptPolyClass & front,BptPolyClass & back) const;
	bool							Is_Degenerate(void);
	bool							Salvage_Degenerate(void);

	int							MaterialId;
	int							NumVerts;
	BptVertexClass				Verts[BPT_POLY_MAX_VERTS];
	PlaneClass					Plane;
};


/*
** BptNodeClass
** This is the node representation used in building the BPT
*/
class BptNodeClass
{

public:

	BptNodeClass(void) : NumPolys(0),Polys(NULL),Front(NULL),Back(NULL),ArrayIndex(-1) { }
	~BptNodeClass(void) { if (Front) delete Front; if (Back) delete Back; if (Polys) delete[] Polys; }
	
	void				Build(int numpolys,BptPolyClass * polys);
	int				Num_Polys(void) const;
	int				Num_Tris(void) const;
	int				Num_Nodes(void) const;
	int				Max_Depth(void) const;
	void				Submit_Polys(MeshBuilderClass & builder) const;

	void				Compute_Bounding_Box(void);
	int				Assign_Array_Indices(int index);
	void				Submit_Nodes(BptImpBuilderClass & impbuilder) const;

private:


	PlaneClass					Plane;		// splitting plane
	uint32						NumPolys;	// num polys on plane
	BptPolyClass *				Polys;		// array of polys
	BptNodeClass *				Front;		// pointer to front tree
	BptNodeClass *				Back;			// pointer to back tree

	int							ArrayIndex;	// used in building the tree into array form.
	
	Vector3						Min;			// bounding box for this tree.
	Vector3						Max;

	/*
	** SplitChoiceStruct.  Whenever a plane is tested to see if it is a suitable splitting
	** plane, we'll cache all of the values it computes so we do not duplicate the effort elsewhere.
	*/
	struct SplitChoiceStruct
	{
		SplitChoiceStruct(void) : Score(0.0f),FrontCount(0),BackCount(0),OnCount(0),SplitCount(0),Plane(0,0,0,0) {}
		
		float			Score;			// 0.0 is worst, 1.0 is "perfect" 
		int			FrontCount;		// number of polys in front of the plane
		int			BackCount;		// number of polys behind the plane
		int			OnCount;			// number of polys on the plane
		int			SplitCount;		// number of polys split by the plane
		PlaneClass	Plane;			// plane equation
	};

	/*
	** SplitArrayStruct.  This struct simply stores the result of a split operation.  The
	** arrays for the front, back and "on" polygons will be allocated and filled in.
	*/
	struct SplitArraysStruct
	{
		SplitArraysStruct(void) : FrontCount(0),BackCount(0),OnCount(0),FrontPolys(NULL),BackPolys(NULL),OnPolys(NULL) {}

		int					FrontCount;
		int					BackCount;
		int					OnCount;
		BptPolyClass *		FrontPolys;
		BptPolyClass *		BackPolys;
		BptPolyClass *		OnPolys;
	};

	SplitChoiceStruct		Compute_Plane_Score(int numpolys,BptPolyClass * polys,const PlaneClass & plane);	
	SplitChoiceStruct		Select_Splitting_Plane(int numpolys,BptPolyClass * polys);

	void						Split_Polys(const int num_polys,
												const BptPolyClass * polys,
												const SplitChoiceStruct	& sc,
												SplitArraysStruct * arrays);

	void						Max_Depth_Internal(int cur_depth, int & max_depth) const;

	friend class BptImpBuilderClass;
};


/*
** BptBuilderClass
** This class builds the initial straightforward binary tree representation of
** the mesh given to it.  It then "optimizes" the tree into a BptImpClass to
** save ram.
*/
class BptBuilderClass
{
public:

	BptBuilderClass(void);
	~BptBuilderClass(void);

	BptImpClass * Build(MeshClass * mesh);
	BptImpClass * Build(int numpolys,BptPolyClass * polyarray,MaterialInfoClass * matinfo);
	
private:

	void						unwind_mesh(MeshClass * mesh,int & set_numpolys,BptPolyClass * & set_polyarray);
	BptImpClass *			build_imp(void);
	
	BptNodeClass *			Root;
	MaterialInfoClass *	MatInfo;
	int						InputPolyCount;
	int						OutputPolyCount;
	int						InputTriCount;
	int						OutputTriCount;
	int						NodeCount;
	int						MaxDepth;
};


/*
** BptImpNodeClass
** This is the node representation in the final BPT implementation.
** It allows for sharing of plane equations, indirection to the polygons, etc
*/
class BptImpNodeClass
{
public:

	float							MinX,MinY,MinZ;
	float							MaxX,MaxY,MaxZ;

	BptImpNodeClass *			Front;
	BptImpNodeClass *			Back;

	uint16						FirstPoly;
	uint16						PolyCount;

	uint16						NormalIndex;
	uint16						DistanceIndex;

	bool							Is_Visible(const CameraClass & camera);
	bool							Cast_AABox_To_Polys(PhysAABoxCollisionTestClass & coltest);
};


/*
** BptImpClass
** This class contains the actual run-time binary partition tree information
** and the code for building a bpt out of an arbitrary list of polygons.
*/
class BptImpClass
{
public:

	~BptImpClass(void);
	void Free(void);

	void Build_Apt(const CameraClass & camera);
	bool Cast_Ray(PhysRayCollisionTestClass & raytest) const;
	bool Cast_AABox(PhysAABoxCollisionTestClass & boxtest) const;
	bool Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest) const;

private:

	// These functions are used by the BptImpBuilder to construct
	// the BptImp...
	BptImpClass(void);
	
	void Set_Mesh(MeshClass * mesh);
	void Set_Node_Array(int numnodes, BptImpNodeClass * nodes);
	void Set_Normal_Array(const UniqueArrayClass<Vector3> & normals);
	void Set_Distance_Array(const UniqueArrayClass<float> & distances);
	void Set_Poly_Index_Array(int numremaps,int * polyremaps);

	void Begin_Apt(void);
	void Add_Polys_To_Apt(BptImpNodeClass * node);
	void End_Apt(void);
	void Build_Apt_Recursive(BptImpNodeClass * node,const CameraClass & cam);

	bool Cast_Ray_Recursive(BptImpNodeClass * node,PhysRayCollisionTestClass & boxtest) const;
	bool Cast_AABox_Recursive(BptImpNodeClass * node,PhysAABoxCollisionTestClass & boxtest) const;
	bool Cast_OBBox_Recursive(BptImpNodeClass * node,PhysOBBoxCollisionTestClass & boxtest) const;
	
	bool Cast_Ray_To_Polys(BptImpNodeClass * node,PhysAABoxCollisionTestClass & boxtest) const { assert(0); return false; }
	bool Cast_AABox_To_Polys(BptImpNodeClass * node,PhysAABoxCollisionTestClass & boxtest) const;
	bool Cast_OBBox_To_Polys(BptImpNodeClass * node,PhysAABoxCollisionTestClass & boxtest) const { assert(0); return false; }

	void Verify_Node_Bounding_Volume(BptImpNodeClass * node,const Vector3 & min,const Vector3 & max) const;
	
	// Polygon Mesh in w3d-friendly form
	MeshClass *					Mesh;

	// Binary partition nodes
	int							NodeCount;
	BptImpNodeClass *			Nodes;
	
	// Node Polygon remap arrays
	int							PolyIndexCount;
	int *							PolyIndices;

	// Plane equation normals, these are stored separate from the distances
	// because many many of the planes will share the same normal but not dist.
	int							NormalCount;
	Vector3 *					Normals;

	// Plane equation distances, again, stored separately from the normals
	int							DistanceCount;
	float *						Distances;

	// Active Poly Table support.  These variables are used internally by
	// the culling code.  The ActivePolyTable pointer is set to point into
	// the surrender mesh's apt.
	int							ActivePolyCount;
	SRDWORD *					ActivePolyTable;

	friend class BptClass;
	friend class BptImpBuilderClass;

};


/*
** BptImpBuilderClass
** This guy takes a completed bpt tree (given as a pointer to the root node)
** and churns through the thing, generating a BptImpClass.
*/
class BptImpBuilderClass
{
public:

	BptImpBuilderClass(void);
	~BptImpBuilderClass(void);

	void					Free(void);

	BptImpClass *		Build_Bpt_Imp(BptNodeClass * root,MaterialInfoClass * matinfo);
	void					Add_Bpt_Node(const BptNodeClass * node);

private:

	void					Remap_Triangle_Indices(void);
	
	MeshBuilderClass					MeshBuilder;			// accumulate the polygons

	NormalHasherClass					NormalHasher;			// computes hash values for the normals
	DistanceHasherClass				DistanceHasher;		// computes hash values for the distances
	
	UniqueArrayClass<Vector3> *	UniqueNormals;			// accumulates the unique normals
	UniqueArrayClass<float>	*		UniqueDistances;		// accumulates the unique distances

	int									NodeCount;				// number of nodes in the tree
	BptImpNodeClass *					Nodes;					// array of all of the nodes

	int									TriCount;				// number of triangles in the tree
	int									CurTriIndex;			// current remap index
	int *									TriIndexArray;			// array of triangle remap indices

};








/***********************************************************************************************
 * BptClass::BptClass -- constructor                                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
BptClass::BptClass(void) :
	BptImp(NULL)
{

}


/***********************************************************************************************
 * BptClass::~BptClass -- destructor                                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
BptClass::~BptClass(void) 
{
	Free();
}


/***********************************************************************************************
 * BptClass::Free -- releases all assets in use                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
void BptClass::Free(void)
{
	if (BptImp) {
		delete BptImp;
		BptImp = NULL;
	}
}


/***********************************************************************************************
 * BptClass::Build -- build a bpt from the given mesh (generates a new mesh)                   *
 *                                                                                             *
 * This function will process the given mesh into a binary partition tree.  It has to generate *
 * a new mesh due to the fact that some polygons may be split in the partioning process.       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
void BptClass::Build(MeshClass * mesh)
{
	assert(mesh);

	/*
	** First, release everything that we have!
	*/
	Free();

	/*
	** Create a new BptImp object using the given mesh
	*/
	BptBuilderClass builder;
	BptImp = builder.Build(mesh);
}


/***********************************************************************************************
 * BptClass::Load -- Initialize this Bpt object from contents of a W3D file                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
int BptClass::Load(ChunkLoadClass & cload)
{
	assert(0);
	return 0;
}


/***********************************************************************************************
 * BptClass::Save -- Save this Bpt object into a W3D file                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
int BptClass::Save(ChunkSaveClass & csave)
{
	assert(0);
	return 0;
}


/***********************************************************************************************
 * BptClass::Render -- submit this object for rendering                                        *
 *                                                                                             *
 * This function sets the active polygon table for the mesh based on the camera frustum.  The  *
 * bpt is used to cull out portions of the level which are not inside the frustum.  Then the   *
 * mesh is added to the given surrender scene.                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *   5/29/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void BptClass::Render( srScene * scene, const CameraClass &camera )
{
	/*
	** Do our APT stuff
	*/
	BptImp->Build_Apt(camera);
	
	/*
	** Then let Mesh render itself
	*/
	BptImp->Mesh->Render(scene,camera);
}
	

void BptClass::Update_Cached_Bounding_Volumes(void) const
{
	CachedBoundingSphere = BptImp->Mesh->Get_Bounding_Sphere();
	CachedBoundingBox = BptImp->Mesh->Get_Bounding_Box();
	Validate_Cached_Bounding_Volumes();
}

#if 0
MeshClass * BptClass::Get_Mesh(void)
{
	if (BptImp) {
		if (BptImp->Mesh) BptImp->Mesh->Add_Ref();
		return BptImp->Mesh;
	} else {
		return NULL;
	}
}
#endif
/***********************************************************************************************
 * BptClass::Cast_Ray -- Intersect a ray with this Bpt Object                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
bool BptClass::Cast_Ray(PhysRayCollisionTestClass & raytest) const
{
	assert(BptImp);
	return BptImp->Cast_Ray(raytest);
}


/***********************************************************************************************
 * BptClass::Cast_AABox -- Intersect a swept axis-aligned box.                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
bool BptClass::Cast_AABox(PhysAABoxCollisionTestClass & aaboxtest) const
{
	assert(BptImp);
	return BptImp->Cast_AABox(aaboxtest);
}


/***********************************************************************************************
 * BptClass::Cast_OBBox -- Intersect a swept oriented box with this bpt object                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
bool BptClass::Cast_OBBox(PhysOBBoxCollisionTestClass & obboxtest) const
{
	assert(BptImp);
	return BptImp->Cast_OBBox(obboxtest);
}


/**************************************************************

	BptVertexClass Implementation

**************************************************************/

BptVertexClass::BptVertexClass(void) :
	Position(0,0,0),
	Normal(0,0,0),
	Color(0,0,0),
	TexCoord(0,0,0)
{
}

BptVertexClass::BptVertexClass(const BptVertexClass & that)
{
	Position = that.Position;
	Normal = that.Normal;
	Color = that.Color;
	TexCoord = that.TexCoord;
}

BptVertexClass & BptVertexClass::operator = (const BptVertexClass & that)
{
	if (this != &that) {
		Position = that.Position;
		Normal = that.Normal;
		Color = that.Color;
		TexCoord = that.TexCoord;
	}
	return *this;
}

BptVertexClass BptVertexClass::Lerp(const BptVertexClass & v0,const BptVertexClass & v1,double lerp)
{
	assert(lerp >= -BPT_EPSILON);
	assert(lerp <= 1.0 + BPT_EPSILON);
	
	// interpolate position
	BptVertexClass res;
	res.Position = ::Lerp(v0.Position,v1.Position,lerp);

	// interpolate normal, renormalize
	res.Normal = ::Lerp(v0.Normal,v1.Normal,lerp);
	res.Normal.Normalize();

	// interpolate color
	res.Color = ::Lerp(v0.Color,v1.Color,lerp);

	// interpolate texture coordinates
	res.TexCoord = ::Lerp(v0.TexCoord,v1.TexCoord,lerp);

	return res;
}

int BptVertexClass::Which_Side(const PlaneClass & plane) const
{
	float d = Vector3::Dot_Product(plane.N,Position);
	d -= plane.D;

	if (d > BPT_EPSILON) {
		return BPT_FRONT;
	} 

	if (d < -BPT_EPSILON) {
		return BPT_BACK;
	}

	return BPT_ON;
}

BptVertexClass BptVertexClass::Intersect_Plane
(
	const BptVertexClass & p0,
	const BptVertexClass & p1,
	const PlaneClass & plane
)
{
	double alpha = 0.0f;
	plane.Compute_Intersection(p0.Position,p1.Position,&alpha);
	return BptVertexClass::Lerp(p0,p1,alpha);				
}

/**************************************************************

	BptPolyClass Implementation

**************************************************************/
BptPolyClass::BptPolyClass(void) :
	NumVerts(0)
{
}

BptPolyClass::BptPolyClass(const BptPolyClass & that)
{
	NumVerts = that.NumVerts;
	for (int i=0;i<NumVerts;i++) {
		Verts[i] = that.Verts[i];
	}
}

BptPolyClass::BptPolyClass(const BptVertexClass * points, int num)
{
	NumVerts = num;
	for (int i=0; i<NumVerts; i++) {
		Verts[i] = points[i];
	}
}

BptPolyClass & BptPolyClass::operator = (const BptPolyClass & that)
{
	if (this != &that) {
		MaterialId = that.MaterialId;
		NumVerts = that.NumVerts;
		Plane = that.Plane;
		for (int i=0;i<NumVerts;i++) {
			Verts[i] = that.Verts[i];
		}
	}
	return * this;
}

void BptPolyClass::Compute_Plane(void)
{
	double nx = 0;
	double ny = 0;
	double nz = 0;
	double ax = 0;
	double ay = 0;
	double az = 0;

	int i,j;

	for (i=0; i<NumVerts; i++) {
		j = (i+1) % NumVerts;
		
		nx += (double)(Verts[i].Position.Y - Verts[j].Position.Y) * (double)(Verts[i].Position.Z + Verts[j].Position.Z);
		ny += (double)(Verts[i].Position.Z - Verts[j].Position.Z) * (double)(Verts[i].Position.X + Verts[j].Position.X);
		nz += (double)(Verts[i].Position.X - Verts[j].Position.X) * (double)(Verts[i].Position.Y + Verts[j].Position.Y);

		ax += (double)Verts[i].Position.X;
		ay += (double)Verts[i].Position.Y;
		az += (double)Verts[i].Position.Z;
	}

	ax /= (double)NumVerts;
	ay /= (double)NumVerts;
	az /= (double)NumVerts;

	double len = sqrt(nx*nx + ny*ny + nz*nz);
	nx /= len;
	ny /= len;
	nz /= len;
	
	Plane.Set(Vector3(nx,ny,nz),Vector3(ax,ay,az));
}

int BptPolyClass::Which_Side(const PlaneClass & plane) const
{
	int side_mask = 0;
	for (int i=0; i<NumVerts;i++) {
		
		side_mask |= Verts[i].Which_Side(plane);

	}

	// check if all verts are "ON"
	if (side_mask == BPT_ON) {
		return BPT_ON;
	}

	// check if all verts are either "ON" or "FRONT"
	if ((side_mask & ~(BPT_FRONT | BPT_ON)) == 0) {
		return BPT_FRONT;
	}

	// check if all verts are either "ON" or "BACK"
	if ((side_mask & ~(BPT_BACK | BPT_ON)) == 0) {
		return BPT_BACK;
	}

	// otherwise, poly spans the plane.
	return BPT_BOTH;
}

void BptPolyClass::Split(const PlaneClass & plane,BptPolyClass & front,BptPolyClass & back) const
{
	front = *this;
	back = *this;
	front.NumVerts = back.NumVerts = 0;
	assert(Which_Side(plane) == BPT_BOTH);

	BptVertexClass point;
	front.NumVerts = 0; 
	back.NumVerts = 0;

	// find a vertex on one side or the other
	int side = BPT_ON;
	int i;
	for (i = 0; (i < NumVerts) && ((side = Verts[i].Which_Side(plane)) == BPT_ON); i++);

	// perform clipping
	int iprev = i;
	int sideprev = side;
	int sidelastdefinite = 0;
	
	i = (i+1) % NumVerts;

	for (int j=0; j<NumVerts; j++) { 
		
		side = Verts[i].Which_Side(plane);
		
		if (sideprev == BPT_FRONT) {

			if (side == BPT_FRONT) {
			
				// Previous vertex was in front of plane and this vertex is in
				// front of the plane so we emit this vertex in the front poly
				front.Verts[(front.NumVerts)++] = Verts[i];

			} else if (side == BPT_ON) {

				// Previous vert was in front, this vert is "on" so emit
				// the vertex into the front poly.
				sidelastdefinite = BPT_FRONT;
				front.Verts[(front.NumVerts)++] = Verts[i];
			
			} else { // side == BPT_BACK

				// Previous vert was in front, this vert is behind, compute
				// the intersection and emit the point in both the front
				// and back polys.  Then continue the edge into the back halfspace
				point = BptVertexClass::Intersect_Plane(Verts[iprev],Verts[i],plane);
				front.Verts[(front.NumVerts)++] = point;
				back.Verts[(back.NumVerts)++] = point;
				back.Verts[(back.NumVerts)++] = Verts[i];

			}

		} else if (sideprev == BPT_BACK) {

			if (side == BPT_FRONT) {

				// segment is going from the back halfspace to the front halfspace
				// compute the intersection and emit it in both polys, then continue
				// the edge into the front halfspace.
				point = BptVertexClass::Intersect_Plane(Verts[iprev],Verts[i],plane);
				back.Verts[(back.NumVerts)++] = point;
				front.Verts[(front.NumVerts)++] = point;
				front.Verts[(front.NumVerts)++] = Verts[i];
			
			} else if (side == BPT_ON) {
			
				// segment went from back halfspace to "on" the plane.  Emit
				// the vertex into the back poly and remember that we came 
				// from the back halfspace.
				sidelastdefinite = BPT_BACK;
				back.Verts[(back.NumVerts)++] = Verts[i];

			} else {  // side == BPT_BACK

				// segment is completely in the back halfspace, just emit the
				// vertex into the back poly
				back.Verts[(back.NumVerts)++] = Verts[i];

			}
		
		} else if (sideprev == BPT_ON) {

			if (side == BPT_FRONT) {

				// segment is on the plane
				if (sidelastdefinite == BPT_BACK) {
					front.Verts[(front.NumVerts)++] =	Verts[iprev];
				}
				front.Verts[(front.NumVerts)++] = Verts[i];
			
			} else if (side == BPT_ON) {

				if (sidelastdefinite == BPT_FRONT) {
					front.Verts[(front.NumVerts)++] = Verts[i];
				} else {
					back.Verts[(back.NumVerts)++] = Verts[i];
				}
			
			} else { // side == BPT_BACK

				if (sidelastdefinite == BPT_FRONT) {
					back.Verts[(back.NumVerts)++] =	Verts[iprev];
				}
				back.Verts[(back.NumVerts)++] = Verts[i];
			}
		} else {
			WWASSERT_PRINT(0,"BptPolyClass::Split : invalid side\n");
		}

		sideprev = side;
		iprev = i;
		i = (i+1)%NumVerts;

	}

	front.Compute_Plane();
	back.Compute_Plane();

	// check the two polygons
	if (front.Is_Degenerate()) {
		front.Salvage_Degenerate();
	}

	if (back.Is_Degenerate()) {
		back.Salvage_Degenerate();
	}
}



bool BptPolyClass::Is_Degenerate(void)
{
	int i,j;

	if (NumVerts == 2) {
		WWDEBUG_SAY(("Degenerate Poly - fewer than 3 vertices\r\n"));
		return true;
	}

	for (i=0; i<NumVerts; i++) {
		for (j = i+1; j < NumVerts; j++) {
			
			float delta = (Verts[i].Position - Verts[j].Position).Length();
			if (delta < BPT_COINCIDENCE_EPSILON) {
				WWDEBUG_SAY(("Degenerate Poly - coincident vertices!\r\n"));
				return true;
			}
		}
	}

	for (i=0; i<NumVerts; i++) {
		int side = Verts[i].Which_Side(Plane);
		if (side != BPT_ON) {

			// hmmm, try to recalculate the plane, if it is still bad, then give up
			Compute_Plane();

			if (Verts[i].Which_Side(Plane) != BPT_ON) {
				WWDEBUG_SAY(("Degenerate Poly - invalid plane!\r\n"));
				return true;
			}
		}
	}

	return false;
}

bool BptPolyClass::Salvage_Degenerate(void)
{
	/*
	** About all we can do is combine sequential vertices which are co-incident
	*/
	int i = 0;
	while (i < NumVerts) {

		float delta = (Verts[i].Position - Verts[i+1].Position).Length();
		if (delta < BPT_COINCIDENCE_EPSILON) {
			
			for (int j=i+1; j<NumVerts-1; j++) {
				Verts[j] = Verts[j+1];
			}

			NumVerts--;
		
		} else {

			i++;
		}
	}

	return !Is_Degenerate();
}

/**************************************************************

	BptBuilderClass Implementation

**************************************************************/
BptBuilderClass::BptBuilderClass(void) : 
	Root(NULL),
	MatInfo(NULL),
	InputPolyCount(0),
	OutputPolyCount(0),
	NodeCount(0),
	MaxDepth(0)
{
}

BptBuilderClass::~BptBuilderClass(void) 
{ 
	if (Root) delete Root; 
	if (MatInfo) MatInfo->Release_Ref(); 
}

BptImpClass * BptBuilderClass::Build(MeshClass * mesh)
{
	/*
	** Here we go, unwind the mesh into BptVerts and BptPolys
	*/
	int numpolys;
	BptPolyClass * polys;

	unwind_mesh(mesh,numpolys,polys);
	assert(numpolys > 0);
	
	/*
	** Jump into the generalized build method
	*/
	MaterialInfoClass * matinfo = mesh->Get_Material_Info();
	BptImpClass * result = Build(numpolys,polys,matinfo);
	matinfo->Release_Ref();

	/*
	** Build function deletes the poly array...
	*/
	polys = NULL;

	return result;
}

BptImpClass * BptBuilderClass::Build(int numpolys,BptPolyClass * polys,MaterialInfoClass * matinfo)
{
	/*
	** Create a copy of all of the materials
	*/
	MatInfo = (MaterialInfoClass *)matinfo->Clone();


	/*
	** Store the input counts
	*/
	InputPolyCount = numpolys;
	InputTriCount = 0;
	for (int i=0; i<numpolys; i++) {
		InputTriCount += polys[i].NumVerts-2;
	}

	WWDEBUG_SAY(("Building Binary Tree\r\n"));

	/*
	** Build the fat tree
	*/
	assert(Root == NULL);
	Root = new BptNodeClass;
	Root->Build(numpolys,polys);

	/*
	** Compile some statistics about the tree
	*/
	OutputPolyCount = Root->Num_Polys();
	OutputTriCount = Root->Num_Tris();
	NodeCount = Root->Num_Nodes();
	MaxDepth = Root->Max_Depth();

	WWDEBUG_SAY(("Binary Tree Generated!\r\n"));
	WWDEBUG_SAY(("Node Count:            %d\r\n",NodeCount));
	WWDEBUG_SAY(("Max Depth:             %d\r\n",MaxDepth));
	WWDEBUG_SAY(("Input Poly Count:      %d\r\n",InputPolyCount));
	WWDEBUG_SAY(("Output Poly Count:     %d\r\n",OutputPolyCount));
	WWDEBUG_SAY(("Input Tri Count:       %d\r\n",InputTriCount));
	WWDEBUG_SAY(("Output Tri Count:      %d\r\n",OutputTriCount));
	WWDEBUG_SAY(("Out/In Poly Ratio:     %f\r\n",(float)OutputPolyCount / (float)InputPolyCount));
	WWDEBUG_SAY(("Out/In Tri Ratio:      %f\r\n",(float)OutputTriCount / (float)InputTriCount));
	WWDEBUG_SAY(("\r\n"));

	/*
	** Build the optimized BptImp and return it.
	*/
	return build_imp();
}	

void BptBuilderClass::unwind_mesh(MeshClass * mesh,int & set_numpolys,BptPolyClass * & set_polyarray)
{
	srMeshModel * srmesh = mesh->Get_Surrender_Mesh();

	// get pointers to the interesting stuff in the mesh
	srVector3 * loc = srmesh->getVertexLoc();
	srVector3 * norm = srmesh->getVertexNormal();
	srVector3 * uv = srmesh->getVertexUVW(srMaterial::DIFFUSE_COLOR,false);
	srVector3 * dcg = srmesh->getVertexDCG(false);
	srVector3i * polyvert = srmesh->getPolyVertex();
	srMaterial ** polymtl = srmesh->getPolyMaterial(false);

	// allocate a bunch of BptPolygons
	set_numpolys = srmesh->getPolygonCount();
	set_polyarray = new BptPolyClass[set_numpolys];
	
	int src_idx = 0;
	int dst_idx = 0;

	for (src_idx = 0; src_idx<set_numpolys; src_idx++) {
		
		set_polyarray[dst_idx].NumVerts = 3;
		
		for (int vrt_idx=0; vrt_idx<3; vrt_idx++) {
					
			// copy the three positions
			set_polyarray[dst_idx][vrt_idx].Position.X = loc[polyvert[src_idx][vrt_idx]].x;
			set_polyarray[dst_idx][vrt_idx].Position.Y = loc[polyvert[src_idx][vrt_idx]].y;
			set_polyarray[dst_idx][vrt_idx].Position.Z = loc[polyvert[src_idx][vrt_idx]].z;
		
			// copy the three normals
			if (norm) {
				set_polyarray[dst_idx][vrt_idx].Normal.X = norm[polyvert[src_idx][vrt_idx]].x;
				set_polyarray[dst_idx][vrt_idx].Normal.Y = norm[polyvert[src_idx][vrt_idx]].y;
				set_polyarray[dst_idx][vrt_idx].Normal.Z = norm[polyvert[src_idx][vrt_idx]].z;
			}

			// copy the three u-v's
			if (uv) {
				set_polyarray[dst_idx][vrt_idx].TexCoord.X = uv[polyvert[src_idx][vrt_idx]].x;
				set_polyarray[dst_idx][vrt_idx].TexCoord.Y = uv[polyvert[src_idx][vrt_idx]].y;
				set_polyarray[dst_idx][vrt_idx].TexCoord.Z = uv[polyvert[src_idx][vrt_idx]].z;
			}

			// copy the three colors
			if (dcg) {
				set_polyarray[dst_idx][vrt_idx].Color.X = dcg[polyvert[src_idx][vrt_idx]].x;
				set_polyarray[dst_idx][vrt_idx].Color.Y = dcg[polyvert[src_idx][vrt_idx]].y;
				set_polyarray[dst_idx][vrt_idx].Color.Z = dcg[polyvert[src_idx][vrt_idx]].z;
			}
			
			// store the material ID
			if (polymtl) {
				set_polyarray[dst_idx].MaterialId = ((MaterialClass *)(polymtl[src_idx]))->Get_Mat_Info_Index();
			} else {
				set_polyarray[dst_idx].MaterialId = 0;
			}
		}

		set_polyarray[dst_idx].Compute_Plane();

		// if the triangle is not degenerate, accept it; increment the counter
		if (!set_polyarray[dst_idx].Is_Degenerate()) {
			dst_idx++;
		}

	}

	// set the number of polys to the actual number kept
	set_numpolys = dst_idx;
	assert(set_numpolys <= srmesh->getPolygonCount());

	WWDEBUG_SAY(("Unwinding Input Mesh\r\n"));
	WWDEBUG_SAY(("Input poly count:                   %d\r\n",srmesh->getPolygonCount()));
	WWDEBUG_SAY(("Count after removing degenerates:   %d\r\n",set_numpolys));
	WWDEBUG_SAY(("\r\n"));
}

BptImpClass * BptBuilderClass::build_imp(void)
{

	BptImpBuilderClass impbuilder;
	BptImpClass * imp = impbuilder.Build_Bpt_Imp(Root,MatInfo);
	return imp;
}


/**************************************************************

	BptNodeClass Implementation

**************************************************************/



int BptNodeClass::Num_Polys(void) const
{ 
	int count = NumPolys; 
	if (Front) count += Front->Num_Polys();
	if (Back) count += Back->Num_Polys();
	return count;
}

int BptNodeClass::Num_Tris(void) const
{
	int count = 0; 
	for (unsigned int i=0; i<NumPolys; i++) {
		count += Polys[i].NumVerts - 2;
	}
	if (Front) count += Front->Num_Tris();
	if (Back) count += Back->Num_Tris();
	return count;
}

int BptNodeClass::Num_Nodes(void) const
{
	int count = 1;
	if (Front) count += Front->Num_Nodes();
	if (Back) count += Back->Num_Nodes();
	return count;
}

int BptNodeClass::Max_Depth(void) const
{
	int max_depth = 0;
	Max_Depth_Internal(0,max_depth);
	return max_depth;
}

void BptNodeClass::Max_Depth_Internal(int cur_depth,int & max_depth) const
{
	cur_depth++;
	if (cur_depth > max_depth) {
		max_depth = cur_depth;
	}

	if (Front) {
		Front->Max_Depth_Internal(cur_depth,max_depth);
	}

	if (Back) {
		Back->Max_Depth_Internal(cur_depth,max_depth);
	}
}

void BptNodeClass::Build(int numpolys,BptPolyClass * polys)
{
	/*
	** Select and assign the splitting plane
	*/
	SplitChoiceStruct sc = Select_Splitting_Plane(numpolys,polys);
	Plane = sc.Plane;
	
	/*
	** Split the polygons
	*/
	SplitArraysStruct arrays;
	Split_Polys(numpolys,polys,sc,&arrays);

	/*
	** Free the memory in use by polys!
	*/
	delete[] polys;

	/*
	** Store the "on" polys in this node
	*/
	NumPolys = arrays.OnCount;
	Polys = arrays.OnPolys;
	arrays.OnPolys = NULL;

	/*
	** Build a front tree if necessary.  Remember that the Build function
	** deletes the poly array.
	*/
	if (arrays.FrontCount) {
		assert(arrays.FrontPolys != NULL);

		Front = new BptNodeClass;
		Front->Build(arrays.FrontCount,arrays.FrontPolys);
		arrays.FrontPolys = NULL;

	}
	
	/*
	** Build a back tree if necessary. Remember that the build function
	** deletes the poly array.
	*/
	if (arrays.BackCount) {
		assert(arrays.BackPolys != NULL);

		Back = new BptNodeClass;
		Back->Build(arrays.BackCount,arrays.BackPolys);
		arrays.BackPolys = NULL;

	}
}


BptNodeClass::SplitChoiceStruct 
BptNodeClass::Compute_Plane_Score(int numpolys,BptPolyClass * polys,const PlaneClass & plane)
{
	/*
	** Suitability of a plane as a partition plane is based on the following factors:
	** - How many polygons will be split
	** - How many polygons end up on each side of the tree
	** - How many polygons end up on the plane (do NOT want many on the plane)
	** - How evenly the volume of space is divided.
	** The following weights will determine how much influence each factor
	** has on the final score.
	*/
	const float POLY_SPLIT_WEIGHT =					5.0f;
	const float POLY_COUNT_BALANCE_WEIGHT =		1.0f;
	const float POLYS_ON_PLANE_WEIGHT =				1.0f;
	const float TOTAL_WEIGHT = POLY_SPLIT_WEIGHT + POLY_COUNT_BALANCE_WEIGHT + POLYS_ON_PLANE_WEIGHT;

	const int MAX_POLYS_ON_PLANE =					10;
	const int MAX_SPLITS =								10;

	/*
	** Count up the cases
	*/
	SplitChoiceStruct sc;
	sc.Plane = plane;

	for (int i=0; i<numpolys; i++) {
		switch(polys[i].Which_Side(plane)) {

			case BPT_FRONT:	sc.FrontCount++;		break;
			case BPT_BACK:		sc.BackCount++;		break;
			case BPT_ON:		sc.OnCount++;			break;
			case BPT_BOTH:		sc.SplitCount++;		break;

		}
	}
	
	/*
	** Compute the score
	*/
	float max_splits = std::min(MAX_SPLITS,numpolys);
	float max_on = std::min(MAX_POLYS_ON_PLANE,numpolys);
	
	float split_score = (float)(max_splits - sc.SplitCount) / max_splits;
	if (split_score < 0.0f) split_score = 0.0f;

	float on_score = 1.0f - (float)sc.OnCount / max_on;
	if (on_score < 0.0f) on_score = 0.0f;

	float balance_score = 1.0f - (fabs(sc.FrontCount - sc.BackCount) / ((float)numpolys / 2.0f));

	sc.Score = 0.0f;
	sc.Score += split_score * POLY_SPLIT_WEIGHT;
	sc.Score += balance_score * POLY_COUNT_BALANCE_WEIGHT;
	sc.Score += on_score * POLYS_ON_PLANE_WEIGHT;
	sc.Score /= TOTAL_WEIGHT;

	/*
	**	Reject the plane if any of the following conditions are true:
	** - plane does not reduce the tree! (all polys in front or behind)
	** - more than MAX_POLYS_ON_PLANE polygons on the splitting plane
	*/
	if ((abs(sc.FrontCount-sc.BackCount) == numpolys) && (sc.OnCount == 0)) {
		sc.Score = 0.0f;
	}
	if (sc.OnCount > MAX_POLYS_ON_PLANE) {
		sc.Score *= 0.4f;
	}

	return sc;
}


BptNodeClass::SplitChoiceStruct
BptNodeClass::Select_Splitting_Plane(int numpolys,BptPolyClass * polys)
{
doitagain:
	const int NUM_VERTEX_TRYS = 30;
	const int NUM_POLY_TRYS = 30;

	SplitChoiceStruct best_plane_stats;
	SplitChoiceStruct considered_plane_stats;
	PlaneClass plane;

	/*
	** Try putting axis-aligned planes through some random vertices
	*/
	for (int vertex_trys = 0; vertex_trys < std::min(NUM_VERTEX_TRYS,3*numpolys); vertex_trys++) {
	
		int poly_index = rand() % numpolys;
		int vert_index = rand() % polys[poly_index].NumVerts;;
		Vector3 pos = polys[poly_index].Verts[vert_index].Position;

		/*
		** Try the x-axis
		*/
		plane.Set(Vector3(1,0,0),pos);
		considered_plane_stats = Compute_Plane_Score(numpolys,polys,plane);
		if (considered_plane_stats.Score > best_plane_stats.Score) {
			best_plane_stats = considered_plane_stats;
		}

		/*
		** Try the y-axis
		*/
		plane.Set(Vector3(0,1,0),pos);
		considered_plane_stats = Compute_Plane_Score(numpolys,polys,plane);
		if (considered_plane_stats.Score > best_plane_stats.Score) {
			best_plane_stats = considered_plane_stats;
		}
	}

	/*
	** Now try "autopartition" planes which come from the input polygons
	*/
	for (int poly_trys=0; poly_trys<min(NUM_POLY_TRYS,numpolys); poly_trys++) {
		
		int poly_index = rand() % numpolys;

		assert(
			(polys[poly_index][0].Which_Side(polys[poly_index].Plane) == BPT_ON) &&
			(polys[poly_index][1].Which_Side(polys[poly_index].Plane) == BPT_ON) &&
			(polys[poly_index][2].Which_Side(polys[poly_index].Plane) == BPT_ON)
		);
		
		considered_plane_stats = Compute_Plane_Score(numpolys,polys,polys[poly_index].Plane);
		
		if (considered_plane_stats.Score > best_plane_stats.Score) {
			best_plane_stats = considered_plane_stats;
		}
	}

	if (best_plane_stats.Score <= 0.0f) {
		_asm int 0x03;
		goto doitagain;
	}
	//assert(best_plane_stats.Score > 0.0f);

	return best_plane_stats;
}

void BptNodeClass::Split_Polys
(
	const int						num_polys,
	const BptPolyClass *			polys,
	const SplitChoiceStruct	&	sc,
	SplitArraysStruct	*			arrays
)
{
	// Note that this routine allocates three arrays of polygons, an array of polys in
	// front of the plane, an array of polys behind the plane, and an array of polys on the plane.
	// The caller is then responsible for keeping track of the memory this routine allocates.

	if (sc.FrontCount + sc.SplitCount > 0) {
		arrays->FrontPolys = new BptPolyClass[sc.FrontCount + sc.SplitCount];
	}

	if (sc.BackCount + sc.SplitCount > 0) {
		arrays->BackPolys = new BptPolyClass[sc.BackCount + sc.SplitCount];
	}

	if (sc.OnCount > 0) {
		arrays->OnPolys = new BptPolyClass[sc.OnCount];
	}

	arrays->FrontCount = 0;
	arrays->BackCount = 0;
	arrays->OnCount = 0;

	for (int i=0; i<num_polys; i++) {

		switch(polys[i].Which_Side(sc.Plane)) {

			case BPT_FRONT:
				arrays->FrontPolys[arrays->FrontCount++] = polys[i];
				break;

			case BPT_BACK:
				arrays->BackPolys[arrays->BackCount++] = polys[i];
				break;

			case BPT_ON:
				arrays->OnPolys[arrays->OnCount++] = polys[i];
				break;

			case BPT_BOTH:
				polys[i].Split(sc.Plane,arrays->FrontPolys[arrays->FrontCount],arrays->BackPolys[arrays->BackCount]);
				if (!arrays->FrontPolys[arrays->FrontCount].Is_Degenerate()) {
					arrays->FrontCount++;
				}
				if (!arrays->BackPolys[arrays->BackCount].Is_Degenerate()) {
					arrays->BackCount++;
				}
				break;
		}
	}

	// when we are all done, the counts should match. 
	assert(arrays->FrontCount <= sc.FrontCount + sc.SplitCount);
	assert(arrays->BackCount <= sc.BackCount + sc.SplitCount);
	assert(arrays->OnCount == sc.OnCount);

	/*
	** Check if we threw away all of the polys
	** (this can only happen if all of them were degenerate...)
	*/
	if ((arrays->OnCount == 0) && (arrays->OnPolys != NULL)) {
		delete[] arrays->OnPolys;
		arrays->OnPolys = NULL;
	}
	if ((arrays->BackCount == 0) && (arrays->BackPolys != NULL)) {
		delete[] arrays->BackPolys;
		arrays->BackPolys = NULL;
	}
	if ((arrays->FrontCount == 0) && (arrays->FrontPolys != NULL)) {
		delete[] arrays->FrontPolys;
		arrays->FrontPolys = NULL;
	}
}


void BptNodeClass::Submit_Polys(MeshBuilderClass & builder) const
{
	/*
	** Submit the back tree
	*/
	if (Back) {
		Back->Submit_Polys(builder);
	}
	
	/*
	** Triangulate each poly and submit it to the mesh builder
	*/
	MeshBuilderClass::FaceClass face;

	for (unsigned int fi=0; fi<NumPolys; fi++) {

		BptPolyClass * poly = &(Polys[fi]);

		for (int ti = 2;ti < poly->NumVerts; ti++) {
			
			face.MaterialIdx = poly->MaterialId;
			face.SmGroup = 1; //TODO: get the correct smooth group???
			face.Index = 0; //TODO: do we have an actual index?
			face.Attributes = 0;

			face.Verts[0].Position =	poly->Verts[0].Position;
			face.Verts[0].TexCoord =	poly->Verts[0].TexCoord;
			face.Verts[0].Color =		poly->Verts[0].Color;

			face.Verts[1].Position =	poly->Verts[ti-1].Position;
			face.Verts[1].TexCoord =	poly->Verts[ti-1].TexCoord;
			face.Verts[1].Color =		poly->Verts[ti-1].Color;
			
			face.Verts[2].Position =	poly->Verts[ti].Position;
			face.Verts[2].TexCoord =	poly->Verts[ti].TexCoord;
			face.Verts[2].Color =		poly->Verts[ti].Color;

			builder.Add_Face(face);
		}
	}

	/*
	** Submit the front tree
	*/
	if (Front) {
		Front->Submit_Polys(builder);
	}
}

void BptNodeClass::Compute_Bounding_Box(void)
{
	/*
	** make the children update their boxes first
	*/
	if (Front) {
		Front->Compute_Bounding_Box();
	}

	if (Back) { 
		Back->Compute_Bounding_Box();
	}

	Min.Set(10000.0f,10000.0f,10000.0f);
	Max.Set(-10000.0f,-10000.0f,-10000.0f);

	/*
	** Bound the polys in this node
	*/
	for (unsigned int poly_index = 0; poly_index < NumPolys; poly_index++) {
		for (int vert_index = 0; vert_index < Polys[poly_index].NumVerts; vert_index++) {

			Vector3 & point = Polys[poly_index].Verts[vert_index].Position;
			
			if (point.X - BPT_EPSILON < Min.X) Min.X = point.X - BPT_EPSILON;
			if (point.X + BPT_EPSILON > Max.X) Max.X = point.X + BPT_EPSILON;

			if (point.Y - BPT_EPSILON < Min.Y) Min.Y = point.Y - BPT_EPSILON;
			if (point.Y + BPT_EPSILON > Max.Y) Max.Y = point.Y + BPT_EPSILON;

			if (point.Z - BPT_EPSILON < Min.Z) Min.Z = point.Z - BPT_EPSILON;
			if (point.Z + BPT_EPSILON > Max.Z) Max.Z = point.Z + BPT_EPSILON;

		}
	}

	/*
	** bound the polys in the front child node
	*/
	if (Front) {
		if (Front->Min.X < Min.X) Min.X = Front->Min.X;
		if (Front->Max.X > Max.X) Max.X = Front->Max.X;

		if (Front->Min.Y < Min.Y) Min.Y = Front->Min.Y;
		if (Front->Max.Y > Max.Y) Max.Y = Front->Max.Y;
		
		if (Front->Min.Z < Min.Z) Min.Z = Front->Min.Z;
		if (Front->Max.Z > Max.Z) Max.Z = Front->Max.Z;
	}

	/*
	** bound the polys in the back child node
	*/
	if (Back) {
		if (Back->Min.X < Min.X) Min.X = Back->Min.X;
		if (Back->Max.X > Max.X) Max.X = Back->Max.X;

		if (Back->Min.Y < Min.Y) Min.Y = Back->Min.Y;
		if (Back->Max.Y > Max.Y) Max.Y = Back->Max.Y;
		
		if (Back->Min.Z < Min.Z) Min.Z = Back->Min.Z;
		if (Back->Max.Z > Max.Z) Max.Z = Back->Max.Z;
	}

	assert(Min.X != 10000.0f);
	assert(Min.Y != 10000.0f);
	assert(Min.Z != 10000.0f);
	assert(Max.X != -10000.0f);
	assert(Max.Y != -10000.0f);
	assert(Max.Z != -10000.0f);

}

int BptNodeClass::Assign_Array_Indices(int index)
{
	/*
	** This function is used to assign a sequential index to
	** each node in the tree.  The BptImp stores its nodes in 
	** an array so this index is used to determine which slot
	** in the array to put each node into.
	*/
	ArrayIndex = index;
	index++;

	if (Front) {
		index = Front->Assign_Array_Indices(index);
	}

	if (Back) {
		index = Back->Assign_Array_Indices(index);
	}

	return index;
}


void BptNodeClass::Submit_Nodes(BptImpBuilderClass & imp_builder) const
{
	imp_builder.Add_Bpt_Node(this);

	if (Front) {
		Front->Submit_Nodes(imp_builder);
	}

	if (Back) {
		Back->Submit_Nodes(imp_builder);
	}
}


/**************************************************************

	BptImpClass Implementation

**************************************************************/
inline bool BptImpNodeClass::Is_Visible(const CameraClass & camera)
{
	return !camera.Cull_Box(Vector3(MinX,MinY,MinZ),Vector3(MaxX,MaxY,MaxZ));
}

/**************************************************************

	BptImpClass Implementation

**************************************************************/
BptImpClass::BptImpClass(void) : 
	Mesh(NULL),
	NodeCount(0),
	Nodes(NULL),
	PolyIndexCount(0),
	PolyIndices(NULL),
	NormalCount(0),
	Normals(NULL),
	DistanceCount(0),
	Distances(NULL),
	ActivePolyCount(0),
	ActivePolyTable(NULL)

{
} 
	
BptImpClass::~BptImpClass(void)
{
	Free();
}


void BptImpClass::Free(void) 
{
	if (Mesh!=NULL) {
		Mesh->Release_Ref();
		Mesh = NULL;
	}

	NodeCount = 0;
	if (Nodes != NULL) {
		delete[] Nodes;
		Nodes = NULL;
	}

	PolyIndexCount = 0;
	if (PolyIndices != NULL) {
		delete[] PolyIndices;
		PolyIndices = NULL;
	}

	NormalCount = 0;
	if (Normals != NULL) {
		delete[] Normals;
		Normals = NULL;
	}

	DistanceCount = 0;
	if (Distances != NULL) {
		delete[] Distances;
		Distances = NULL;
	}
}

void BptImpClass::Set_Mesh(MeshClass * mesh) 
{ 
	if (Mesh) Mesh->Release_Ref(); 
	Mesh = mesh; 
	if (Mesh) Mesh->Add_Ref(); 
}

void BptImpClass::Set_Node_Array(int numnodes, BptImpNodeClass * nodes)
{ 
	if (Nodes) delete[] Nodes; 
	Nodes = nodes; 
	NodeCount = numnodes; 
}

void BptImpClass::Set_Normal_Array(const UniqueArrayClass<Vector3> & normals)
{
	if (normals.Count() != NormalCount) {
		if (Normals) delete[] Normals;
		Normals = new Vector3[normals.Count()];
	}
	
	NormalCount = normals.Count();

	for (int i=0; i<NormalCount; i++) {
		Normals[i] = normals.Get(i);
	}
}

void BptImpClass::Set_Distance_Array(const UniqueArrayClass<float> & distances)
{
	if (distances.Count() != DistanceCount) {
		if (Distances) delete[] Distances;
		Distances = new float[distances.Count()];
	}
	DistanceCount = distances.Count();

	for (int i=0; i<DistanceCount; i++) {
		Distances[i] = distances.Get(i);
	}
}

void BptImpClass::Set_Poly_Index_Array(int numremaps,int * polyremaps)
{
	if (PolyIndices) delete[] PolyIndices;
	PolyIndices = polyremaps;
	PolyIndexCount = numremaps;
}

void BptImpClass::Build_Apt(const CameraClass & camera)
{
	WWDEBUG_PROFILE_START("BPT::Build_Apt");
	Begin_Apt();	
	Build_Apt_Recursive(&(Nodes[0]),camera);
	End_Apt();
	WWDEBUG_PROFILE_STOP("BPT::Build_Apt");
}

void BptImpClass::Begin_Apt(void)
{
	ActivePolyCount = 0;
	ActivePolyTable = Mesh->Get_Surrender_Mesh()->getActivePolygonTable();
}

void BptImpClass::Add_Polys_To_Apt(BptImpNodeClass * node)
{
	assert(node != NULL);
	
#if 0
	memcpy(&(ActivePolyTable[ActivePolyCount]),&(PolyIndices[node->FirstPoly]),node->PolyCount);
	ActivePolyCount += node->PolyCount;
#else
	for (int poly_index=0; poly_index<node->PolyCount; poly_index++) {
		ActivePolyTable[ActivePolyCount++] = PolyIndices[node->FirstPoly + poly_index];
	}
#endif
}

void BptImpClass::End_Apt(void)
{
	Mesh->Get_Surrender_Mesh()->setActivePolygonCount(ActivePolyCount);
//	WWDEBUG_SAY(("Active Polys: %d\r\n",ActivePolyCount));
}

void BptImpClass::Build_Apt_Recursive(BptImpNodeClass * node,const CameraClass & cam)
{
	/*
	** Check the camera's frustum against this node's
	** bounding volume
	*/
	if (!node->Is_Visible(cam)) {
		Verify_Node_Bounding_Volume(node,Vector3(node->MinX,node->MinY,node->MinZ),Vector3(node->MaxX,node->MaxY,node->MaxZ));
		return;
	}

	/*
	** Ok part of this tree may be visible, compute
	** which side of the splitting plane the camera is on
	*/
	float dist = Vector3::Dot_Product(Normals[node->NormalIndex],cam.Get_Position());
	if (dist > Distances[node->DistanceIndex]) {
		
		/*
		** FRONT: 
		** We are in front of the plane so proceed in the following order:
		** Visit the 'Back' node
		** Add our polys to the APT
		** Visit the 'Front' node
		*/
		if (node->Back) Build_Apt_Recursive(node->Back,cam);
		Add_Polys_To_Apt(node);
		if (node->Front) Build_Apt_Recursive(node->Front,cam);

	} else {
		/*
		** BACK: 
		** We are in back of the plane so proceed in the following order:
		** Visit the 'Front' node
		** Add our polys to the APT
		** Visit the 'Back' node
		*/
		if (node->Front) Build_Apt_Recursive(node->Front,cam);
		Add_Polys_To_Apt(node);
		if (node->Back) Build_Apt_Recursive(node->Back,cam);
	}
}



bool BptImpClass::Cast_Ray(PhysRayCollisionTestClass & raytest) const
{
	return Cast_Ray_Recursive(&(Nodes[0]),raytest);
}

bool BptImpClass::Cast_AABox(PhysAABoxCollisionTestClass & boxtest) const
{
	/*
	** Test the sweeping AABox against the BPT recursively!
	*/
	WWDEBUG_PROFILE_START("BPT::Cast_AABox");
	bool res = Cast_AABox_Recursive(&(Nodes[0]),boxtest);
	WWDEBUG_PROFILE_STOP("BPT::Cast_AABox");
	return res;
}

bool BptImpClass::Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest) const
{
	return Cast_OBBox_Recursive(&(Nodes[0]),boxtest);
}

bool BptImpClass::Cast_Ray_Recursive
(
	BptImpNodeClass *				node,
	PhysRayCollisionTestClass	&	raytest
) const
{
	return false;
}

bool BptImpClass::Cast_AABox_Recursive
(
	BptImpNodeClass *				node,
	PhysAABoxCollisionTestClass	&	boxtest
) const
{
	/*
	** First, check the bounding box of the move against the bounding box
	** of this tree, if they don't intersect, bail out
	*/
	if ((boxtest.SweepMin.X > node->MaxX) || (boxtest.SweepMax.X < node->MinX)) {
		return false;
	}

	if ((boxtest.SweepMin.Y > node->MaxY) || (boxtest.SweepMax.Y < node->MinY)) {
		return false;
	}

	if ((boxtest.SweepMin.Z > node->MaxZ) || (boxtest.SweepMax.Z < node->MinZ)) {
		return false;
	}

	if (node->Front) {
		Cast_AABox_Recursive(node->Front,boxtest);
	}
	Cast_AABox_To_Polys(node,boxtest);
	if (node->Back) {
		Cast_AABox_Recursive(node->Back,boxtest);
	}

	return false;
}


bool BptImpClass::Cast_OBBox_Recursive
(
	BptImpNodeClass *					node,
	PhysOBBoxCollisionTestClass &	boxtest
) const
{
	return false;
}

bool BptImpClass::Cast_AABox_To_Polys(BptImpNodeClass * node,PhysAABoxCollisionTestClass & boxtest) const
{
	/*
	** Simply loop through the polys in this node, checking each for collision
	*/
	TriClass tri;

	srMeshModel * srmesh = Mesh->Get_Surrender_Mesh();
	srVector3 * loc = srmesh->getVertexLoc();
	srVector4 * norms = srmesh->getPolyEq();
	srVector3i * polyverts = srmesh->getPolyVertex();

	int polyhit = -1;
	for (int poly_counter=0; poly_counter<node->PolyCount; poly_counter++) {

		int poly_index = PolyIndices[node->FirstPoly + poly_counter];

		tri.V[0] = As_Vector3(&(loc[ polyverts[poly_index][0] ]));
		tri.V[1] = As_Vector3(&(loc[ polyverts[poly_index][1] ]));
		tri.V[2] = As_Vector3(&(loc[ polyverts[poly_index][2] ]));
		tri.N = (Vector3 *)&(norms[poly_index]);

		if ( boxtest.Box.Cast_To_Tri(boxtest.Move,tri,boxtest.Result) ) {;
			polyhit = poly_index;
		}

		if (boxtest.Result->StartBad) {
			return true;
		}
	}
	if (polyhit != -1) {
		return true;
	}
	return false;
}

void BptImpClass::Verify_Node_Bounding_Volume(BptImpNodeClass * node,const Vector3 & min,const Vector3 & max) const
{
			
	srMeshModel * srmesh = Mesh->Get_Surrender_Mesh();
	srVector3 * loc = srmesh->getVertexLoc();
	srVector3i * polyverts = srmesh->getPolyVertex();

	Vector3 * v;

	for (int remap_index = node->FirstPoly; remap_index < node->FirstPoly + node->PolyCount; remap_index++) {

		int poly_index = PolyIndices[remap_index];

		v = As_Vector3(&loc[ polyverts[poly_index][0] ]);

		assert(v->X >= min.X);
		assert(v->Y >= min.Y);
		assert(v->Z >= min.Z);
		
		assert(v->X <= max.X);
		assert(v->Y <= max.Y);
		assert(v->Z <= max.Z);

		v = As_Vector3(&loc[ polyverts[poly_index][1] ]);

		assert(v->X >= min.X);
		assert(v->Y >= min.Y);
		assert(v->Z >= min.Z);
		
		assert(v->X <= max.X);
		assert(v->Y <= max.Y);
		assert(v->Z <= max.Z);

		v = As_Vector3(&loc[ polyverts[poly_index][2] ]);

		assert(v->X >= min.X);
		assert(v->Y >= min.Y);
		assert(v->Z >= min.Z);
		
		assert(v->X <= max.X);
		assert(v->Y <= max.Y);
		assert(v->Z <= max.Z);
	}

	if (node->Back) {
		Verify_Node_Bounding_Volume(node->Back,min,max);
	}
	if (node->Front) {
		Verify_Node_Bounding_Volume(node->Front,min,max);
	}

}

/**************************************************************

	BptImpBuilderClass Implementation

**************************************************************/

BptImpBuilderClass::BptImpBuilderClass(void) :
	MeshBuilder(true,10000,5000),						// TODO: should move this to Build_Bpt_Imp so it is dynamically initialzed
	UniqueNormals(NULL),
	UniqueDistances(NULL),
	NodeCount(0),
	Nodes(NULL),
	TriCount(0),
	CurTriIndex(0),
	TriIndexArray(NULL)
{
}

BptImpBuilderClass::~BptImpBuilderClass(void)
{
	Free();
}

void BptImpBuilderClass::Free(void)
{
	if (UniqueNormals != NULL) {
		delete UniqueNormals;
		UniqueNormals = NULL;
	}

	if (UniqueDistances != NULL) {
		delete UniqueDistances;
		UniqueDistances = NULL;
	}

	NodeCount = 0;
	if (Nodes != NULL) {
		delete[] Nodes;
		Nodes = NULL;
	}

	TriCount = 0;
	CurTriIndex = 0;
	if (TriIndexArray != NULL) {
		delete[] TriIndexArray;
		TriIndexArray = NULL;
	}
}

BptImpClass * BptImpBuilderClass::Build_Bpt_Imp(BptNodeClass * root,MaterialInfoClass * matinfo)
{
	Free();
	WWDEBUG_SAY(("Building the Run-Time Bpt\r\n"));

	TriCount = root->Num_Tris();
	
	/*
	** Allocate the UniqueNormals and UniqueDistances objects
	*/
	int size = TriCount;
	int growth = TriCount / 3;
	if (growth < 5) growth = 5;

	UniqueNormals = new UniqueArrayClass<Vector3>(size,growth,&NormalHasher);
	UniqueDistances = new UniqueArrayClass<float>(size,growth,&DistanceHasher);

	/*
	** Set up the mesh builder
	*/
	MeshBuilder.Reset(true,TriCount,growth);
	MeshBuilder.Disable_All_Vertex_Channels();
	MeshBuilder.Enable_Vertex_Channel(MeshBuilderClass::VERTEX_CHANNEL_NORMAL);
//	MeshBuilder.Enable_Vertex_Channel(MeshBuilderClass::VERTEX_CHANNEL_COLOR);
	MeshBuilder.Enable_Vertex_Channel(MeshBuilderClass::VERTEX_CHANNEL_TEXCOORD);

	/*
	** Allocate the array of triangle indices, set
	** CurTriIndex to point to the top.
	*/
	TriIndexArray = new int[TriCount];
	CurTriIndex = 0;
	
	/*
	** Allocate the array of nodes
	*/
	NodeCount = root->Num_Nodes();
	Nodes = new BptImpNodeClass[NodeCount];

	/*
	** Compute the hierarchical bounding boxes
	*/
	WWDEBUG_SAY(("Computing Hierarchical Bounding Volumes.\r\n"));
	root->Compute_Bounding_Box();
	WWDEBUG_SAY(("Hierarchical Bounding Volumes Done.\r\n"));

	/*
	** Have the tree "unwind" itself into our arrays
	*/
	root->Assign_Array_Indices(0);
	root->Submit_Nodes(*this);

	/*
	** Process the mesh
	*/
	MeshBuilder.Build_Mesh(true);

	/*
	** Remap all of the polygon indices (account for the 
	** mesh builder re-ording polygons)
	*/
	Remap_Triangle_Indices();

	/*
	** Create the BptImpClass
	*/
	MeshClass * mesh = new MeshClass();
	mesh->Init(MeshBuilder,matinfo,"BspMesh","");

	BptImpClass * bptimp = new BptImpClass();

	bptimp->Set_Mesh(mesh);
	bptimp->Set_Node_Array(NodeCount,Nodes);
	bptimp->Set_Poly_Index_Array(TriCount,TriIndexArray);
	bptimp->Set_Normal_Array(*UniqueNormals);
	bptimp->Set_Distance_Array(*UniqueDistances);

	// the "imp" assumed ownership of this stuff
	mesh->Release_Ref();
	mesh = NULL;
	Nodes = NULL;
	TriIndexArray = NULL;

	Free();

	WWDEBUG_SAY(("Run-Time Bpt Done\r\n"));
	WWDEBUG_SAY(("\r\n"));

	return bptimp;
}


void BptImpBuilderClass::Add_Bpt_Node(const BptNodeClass * node)
{
	BptImpNodeClass newnode;

	/*
	** initialize the front pointer
	*/
	if (node->Front) {
		assert(node->Front->ArrayIndex >= 0);
		assert(node->Front->ArrayIndex < NodeCount);
		newnode.Front = &(Nodes[node->Front->ArrayIndex]);
	} else {
		newnode.Front = NULL; //BptImpNodeClass::NULL_NODE_INDEX;
	}

	/*
	** initialize the back pointer
	*/
	if (node->Back) {
		assert(node->Back->ArrayIndex >= 0);
		assert(node->Back->ArrayIndex < NodeCount);
		newnode.Back = &(Nodes[node->Back->ArrayIndex]);
	} else {
		newnode.Back = NULL; //BptImpNodeClass::NULL_NODE_INDEX;
	}

	/*
	** Triangulate each poly and submit it to the mesh builder
	** install the Triangle Remap Indices as we go
	*/
	newnode.FirstPoly = CurTriIndex;
	newnode.PolyCount = 0;

	MeshBuilderClass::FaceClass face;

	for (unsigned int fi=0; fi<node->NumPolys; fi++) {

		BptPolyClass * poly = &(node->Polys[fi]);

		for (int ti = 2;ti < poly->NumVerts; ti++) {
			
			face.MaterialIdx = poly->MaterialId;
			face.SmGroup = 1; //TODO: get the correct smooth group???
			face.Index = 0; //TODO: do we have an actual index?
			face.Attributes = 0;

			face.Verts[0].Position =	poly->Verts[0].Position;
			face.Verts[0].TexCoord =	poly->Verts[0].TexCoord;
			face.Verts[0].Color =		poly->Verts[0].Color;

			face.Verts[1].Position =	poly->Verts[ti-1].Position;
			face.Verts[1].TexCoord =	poly->Verts[ti-1].TexCoord;
			face.Verts[1].Color =		poly->Verts[ti-1].Color;
			
			face.Verts[2].Position =	poly->Verts[ti].Position;
			face.Verts[2].TexCoord =	poly->Verts[ti].TexCoord;
			face.Verts[2].Color =		poly->Verts[ti].Color;

			TriIndexArray[CurTriIndex] = MeshBuilder.Add_Face(face);
			CurTriIndex++;
			newnode.PolyCount++;

			/*
			** just verifying that all verts are inside the bounding box
			*/
			assert(face.Verts[0].Position.X >= node->Min.X);
			assert(face.Verts[0].Position.Y >= node->Min.Y);
			assert(face.Verts[0].Position.Z >= node->Min.Z);

			assert(face.Verts[1].Position.X >= node->Min.X);
			assert(face.Verts[1].Position.Y >= node->Min.Y);
			assert(face.Verts[1].Position.Z >= node->Min.Z);

			assert(face.Verts[2].Position.X >= node->Min.X);
			assert(face.Verts[2].Position.Y >= node->Min.Y);
			assert(face.Verts[2].Position.Z >= node->Min.Z);

			assert(face.Verts[0].Position.X <= node->Max.X);
			assert(face.Verts[0].Position.Y <= node->Max.Y);
			assert(face.Verts[0].Position.Z <= node->Max.Z);

			assert(face.Verts[1].Position.X <= node->Max.X);
			assert(face.Verts[1].Position.Y <= node->Max.Y);
			assert(face.Verts[1].Position.Z <= node->Max.Z);

			assert(face.Verts[2].Position.X <= node->Max.X);
			assert(face.Verts[2].Position.Y <= node->Max.Y);
			assert(face.Verts[2].Position.Z <= node->Max.Z);
	
		}
	}
	
	/*
	** Bounding box
	*/
	newnode.MinX = node->Min.X;
	newnode.MinY = node->Min.Y;
	newnode.MinZ = node->Min.Z;
	newnode.MaxX = node->Max.X;
	newnode.MaxY = node->Max.Y;
	newnode.MaxZ = node->Max.Z;

	assert(newnode.MinX <= newnode.MaxX);
	assert(newnode.MinY <= newnode.MaxY);
	assert(newnode.MinZ <= newnode.MaxZ);

	/*
	** Plane Equation
	*/
	assert(UniqueNormals);
	assert(UniqueDistances);
	newnode.NormalIndex = UniqueNormals->Add(node->Plane.N);
	newnode.DistanceIndex = UniqueDistances->Add(node->Plane.D);

	/*
	** install the node!
	*/
	Nodes[node->ArrayIndex] = newnode;
}

void BptImpBuilderClass::Remap_Triangle_Indices(void)
{
	int pi;
	int * index_remap = new int[MeshBuilder.Get_Face_Count()];

	assert(MeshBuilder.Get_Face_Count() == TriCount);
	for (pi=0; pi < MeshBuilder.Get_Face_Count(); pi++) {
		const MeshBuilderClass::FaceClass & face = MeshBuilder.Get_Face(pi);
		assert(face.AddIndex >= 0);
		assert(face.AddIndex < TriCount);
		index_remap[ face.AddIndex ] = pi;
	}

	for (pi=0; pi < TriCount; pi++) {
		TriIndexArray[pi] = index_remap[TriIndexArray[pi]];
	}

	delete[] index_remap;

}

#endif // PORT140
#endif // OBSOLETE!
