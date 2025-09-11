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
 *                     $Archive:: /Commando/Code/wwphys/staticphys.cpp                        $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/29/02 2:49p                                               $*
 *                                                                                             *
 *                    $Revision:: 49                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   StaticPhysClass::StaticPhysClass -- Constructor                                           *
 *   StaticPhysClass::~StaticPhysClass -- Destructor                                           *
 *   StaticPhysClass::Init -- Initialize this object from a definition                         *
 *   StaticPhysClass::Set_Vis_Sector_ID -- Sets the vis sector ID for this object              *
 *   StaticPhysClass::Set_Model -- Set the model for this static object                        *
 *   StaticPhysClass::Update_Cached_Model_Parameters -- update our state                       *
 *   StaticPhysClass::Render_Vis_Meshes -- renders any vis meshes in this model                *
 *   StaticPhysClass::Get_Bounding_Box -- Returns the bounding box of this object              *
 *   StaticPhysClass::Get_Transform -- Returns the transform of this object                    *
 *   StaticPhysClass::Set_Transform -- Set the transform for this object                       *
 *   StaticPhysClass::Is_Occluder -- Returns whether this static object is an occluder         *
 *   StaticPhysClass::Is_Model_Pre_Lit -- determines if the render object is pre-lit           *
 *   StaticPhysClass::Is_Vis_Sector -- Is_Vis_Sector                                           *
 *   StaticPhysClass::Update_Sun_Status -- determine if this object is in the sun              *
 *   StaticPhysClass::Get_Factory -- Returns the persist factory for StaticPhysClass           *
 *   StaticPhysClass::Save -- Save method, persistant object support                           *
 *   StaticPhysClass::Load -- Load method, persistant object support                           *
 *   StaticPhysClass::On_Post_Load -- Post-Load callback                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "staticphys.h"
#include "colmathaabox.h"
#include "vistable.h"
#include "chunkio.h"
#include "assetmgr.h"
#include "progcall.h"
#include "matrix3.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "wwphysids.h"
#include "wwhack.h"
#include "mesh.h"
#include "meshmdl.h"
#include "lightexclude.h"
#include <memory.h>
#include "part_emt.h"
#include "renegadeterrainpatch.h"

#if UMBRASUPPORT
#include <umbra.hpp>
#include "umbrasupport.h"
#endif

DECLARE_FORCE_LINK(staticphys);

 
/***********************************************************************************************
**
** StaticPhysClass Implementation
**
***********************************************************************************************/

bool StaticPhysClass::_DisableStaticPhysSimulation			= false;
bool StaticPhysClass::_DisableStaticPhysRendering			= false;


/*
** Declare a PersistFactory for StaticPhysClasses
*/
SimplePersistFactoryClass<StaticPhysClass,PHYSICS_CHUNKID_STATICPHYS>	_StaticPhysFactory;


/*
** Chunk-ID's used by StaticPhysClass
*/
enum 
{
	STATICPHYS_CHUNK_PHYS				= 14430100,
	STATICPHYS_CHUNK_VARIABLES,

	STATICPHYS_VARIABLE_VISOBJECTID	= 0x00,
	STATICPHYS_VARIABLE_VISSECTORID,
};


/***********************************************************************************************
 * StaticPhysClass::StaticPhysClass -- Constructor                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * I'm initializing the VisObjectID to zero, the "always visible" object ID                    *
 * I'm initializing the VisSectorID to -1 to indicate that no table is allocated for this obj  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
StaticPhysClass::StaticPhysClass(void) :
	VisSectorID(0xFFFFFFFF)
{
	Set_Collision_Group( 15 );	// HACK?  All terrain should be group 15?	
}


/***********************************************************************************************
 * StaticPhysClass::~StaticPhysClass -- Destructor                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
StaticPhysClass::~StaticPhysClass(void)
{
}


/***********************************************************************************************
 * StaticPhysClass::Init -- Initialize this object from a definition                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
void StaticPhysClass::Init(const StaticPhysDefClass & def)
{
	PhysClass::Init(def);
	if (Model != NULL && Model->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
		((ParticleEmitterClass *)Model)->Start ();
	}
}


/***********************************************************************************************
 * StaticPhysClass::Set_Vis_Sector_ID -- Sets the vis sector ID for this object                *
 *                                                                                             *
 * If a static object contains a VIS mesh, a vis sector ID is allocated for it.  This is the   *
 * ID of the visibility table for this object which tracks what objects are visible when the   *
 * camera is above one of its VIS meshes.                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/27/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void StaticPhysClass::Set_Vis_Sector_ID(int new_id)
{ 
	VisSectorID = new_id;
}

/***********************************************************************************************
 * StaticPhysClass::Set_Model -- Set the model for this static object                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
void StaticPhysClass::Set_Model(RenderObjClass * model)
{
	// call parent class
	PhysClass::Set_Model(model);
	
	Update_Cached_Model_Parameters();
}


/***********************************************************************************************
 * StaticPhysClass::Update_Cached_Model_Parameters -- update our state                         *
 *                                                                                             *
 * This function must be called in three places: In the constructor, whenever the model is     *
 * set, and in the post-load callback                                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/14/00    gth : Created.                                                                 *
 *=============================================================================================*/
void StaticPhysClass::Update_Cached_Model_Parameters(void)
{
	if (Model == NULL) return;

	// if the model is a light-mapped mesh, we want to exclude it from the static lights
	Enable_Is_Pre_Lit(Is_Model_Pre_Lit());

	// if our model is a mesh and its in world space, enable the world-space-mesh optimizations
	if ((Model->Class_ID() == RenderObjClass::CLASSID_MESH) && (Get_Transform() == Matrix3D::Identity)) {
		Enable_Is_World_Space_Mesh(true);
	} else {
		Enable_Is_World_Space_Mesh(false);
	}

#if (UMBRASUPPORT)
	// update the umbra model(s)
	Umbra::Model * test_model = NULL;
	
	if (Model->Class_ID() == RenderObjClass::CLASSID_MESH) {
		MeshClass & mesh = *((MeshClass *)Model);
		test_model = UmbraSupport::Create_Mesh_Model(mesh);		
	} else {
		AABoxClass obj_box;
		Model->Get_Obj_Space_Bounding_Box(obj_box);
		test_model = UmbraSupport::Create_Box_Model(obj_box);
	}

	UmbraObject->setTestModel(test_model);
	if ((Model->Class_ID() == RenderObjClass::CLASSID_MESH) && (Is_Occluder())) {
		test_model->set(Umbra::Model::WRITABLE,true);
		UmbraObject->setWriteModel(test_model);
	}
	//UmbraObject->setCost(100000,100000,5);
#endif //UMBRASUPPORT

}


/***********************************************************************************************
 * StaticPhysClass::Render_Vis_Meshes -- renders any vis meshes in this model                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/17/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void StaticPhysClass::Render_Vis_Meshes(RenderInfoClass & rinfo)
{
	if (Model == NULL) return;
	
	// Note, this only works with Mesh vis sectors.  Theoretically vis sectors could
	// be embedded in a hierarchical model as well...
	if (Model->Get_Collision_Type() & COLLISION_TYPE_VIS) { 

		// Force hidden meshes to render...
		int is_hidden = Model->Is_Hidden();
		int is_anim_hidden = Model->Is_Animation_Hidden();
		Model->Set_Hidden(false);
		Model->Set_Animation_Hidden(false);

		Model->Render(rinfo);

		Model->Set_Hidden(is_hidden);
		Model->Set_Animation_Hidden(is_anim_hidden);
	}
}


/***********************************************************************************************
 * StaticPhysClass::Get_Bounding_Box -- Returns the bounding box of this object                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
const AABoxClass & StaticPhysClass::Get_Bounding_Box(void) const
{
	assert(Model);
	return Model->Get_Bounding_Box();
}


/***********************************************************************************************
 * StaticPhysClass::Get_Transform -- Returns the transform of this object                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
const Matrix3D & StaticPhysClass::Get_Transform(void) const
{
	assert(Model);
	return Model->Get_Transform();
}


/***********************************************************************************************
 * StaticPhysClass::Set_Transform -- Set the transform for this object                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The transform should not be changed in-game!                                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
void StaticPhysClass::Set_Transform(const Matrix3D & m)
{
	// Note: this kind of object never collides with others so we
	// can just warp it to the users desired position.  However,
	// we do need to tell the scene so that it can update us in 
	// the culling system

	// Note #2: In-Game, these objects should never move!!! this
	// feature is used in the editor.  Moving one of these will
	// invalidate some or all of the nice pre-calculated lighting 
	// and culling data!

	assert(Model);
	Model->Set_Transform(m);
	Update_Cull_Box();
	Invalidate_Static_Lighting_Cache();

	// if our model is a mesh and its in world space, enable the world-space-mesh optimizations
	if ((Model->Class_ID() == RenderObjClass::CLASSID_MESH) && (m == Matrix3D::Identity)) {
		Enable_Is_World_Space_Mesh(true);
	} else {
		Enable_Is_World_Space_Mesh(false);
	}
}


/***********************************************************************************************
 * StaticPhysClass::Is_Occluder -- Returns whether this static object is an occluder           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
int StaticPhysClass::Is_Occluder(void)
{
	StaticPhysDefClass * def = Get_StaticPhysDef();
	
	if ((def != NULL) && (def->IsNonOccluder)) {
		return false;
	} else if (Model) {
		if (Model->Is_Translucent()) {
			return false;
		}
		if (Model->Class_ID() == RenderObjClass::CLASSID_DAZZLE) {
			return false;
		}
	} 
	return true;
}


/***********************************************************************************************
 * StaticPhysClass::Is_Model_Pre_Lit -- determines if the render object is pre-lit             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/20/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool StaticPhysClass::Is_Model_Pre_Lit(void)
{
	// if the model is a light-mapped mesh, we want to exclude it from the static lights
	if (Model->Class_ID() == RenderObjClass::CLASSID_MESH) {
		MeshModelClass * mesh_model = ((MeshClass *)Model)->Get_Model();
		WWASSERT(mesh_model != NULL);
		if (mesh_model) {
			bool isprelit = (mesh_model->Get_Flag(MeshModelClass::PRELIT_MASK) != 0);
			mesh_model->Release_Ref();
			return isprelit;
		}
	} else if (Model->Class_ID() == RenderObjClass::CLASSID_RENEGADE_TERRAIN) {
		//return ((RenegadeTerrainPatchClass *)Model)->Is_Prelit ();
	}

	return false;
}

bool StaticPhysClass::Is_Model_User_Lit(void)
{
	if (Model != NULL) {
		return !!(Model->Has_User_Lighting());
	}
	return false;
}


/***********************************************************************************************
 * StaticPhysClass::Is_Vis_Sector -- Is_Vis_Sector                                             *
 *                                                                                             *
 *    Searches the model to determine whether it defines a vis sector.  I.e. any of its polys  *
 *    are marked as VIS polys                                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *    model - typically leave the default of NULL, this function will be called recursively    *
 *            on sub-objects                                                                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool StaticPhysClass::Is_Vis_Sector(RenderObjClass * model) const
{
	bool retval = false;

	/*
	** If NULL was passed in, start with the render object for this StaticPhysClass.
	*/
	if (model == NULL) {
		model = Model;
	}


	/*
	** If we have a valid model to check; either recurse into its sub objects or 
	** check if it is a vis-collideable mesh
	*/
	if (model != NULL) {

		/*
		** Check each sub-object
		*/
		int count = model->Get_Num_Sub_Objects ();
		for (int index = 0; (index < count) && !retval; index ++) {
			
			RenderObjClass *sub_object = model->Get_Sub_Object (index);
			if (sub_object != NULL) {
				retval |= Is_Vis_Sector(sub_object);
				REF_PTR_RELEASE(sub_object);
			}
		}	

		/*
		** Check the model itself
		*/
		if ((model->Class_ID () == RenderObjClass::CLASSID_MESH) &&
			 (model->Get_Collision_Type () & COLLISION_TYPE_VIS))
		{
			retval = true;
		}
	}

	return retval;	
}


/***********************************************************************************************
 * StaticPhysClass::Update_Sun_Status -- determine if this object is in the sun                *
 *                                                                                             *
 * This updates the internal IS_IN_THE_SUN flag.                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void StaticPhysClass::Update_Sun_Status(void)
{
	/*
	** Cant do sun occlusion very well for static objects... 
	** Just let the sun always apply?  
	*/
	Set_Flag(IS_IN_THE_SUN,true);
}


/***********************************************************************************************
 * StaticPhysClass::Get_Factory -- Returns the persist factory for StaticPhysClass             *
 *                                                                                             *
 *    All persistant object types need a persist factory which ties them to a unique chunk ID  *
 *    and links them into the save-load system                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
const PersistFactoryClass & StaticPhysClass::Get_Factory(void) const
{
	return _StaticPhysFactory;
}


/***********************************************************************************************
 * StaticPhysClass::Save -- Save method, persistant object support                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool StaticPhysClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(STATICPHYS_CHUNK_PHYS);
	PhysClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(STATICPHYS_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,STATICPHYS_VARIABLE_VISOBJECTID,VisObjectID);
	WRITE_MICRO_CHUNK(csave,STATICPHYS_VARIABLE_VISSECTORID,VisSectorID);
	csave.End_Chunk();

	return true;
}


/***********************************************************************************************
 * StaticPhysClass::Load -- Load method, persistant object support                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool StaticPhysClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case STATICPHYS_CHUNK_PHYS:
				PhysClass::Load(cload);
				break;

			case STATICPHYS_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,STATICPHYS_VARIABLE_VISOBJECTID,VisObjectID);
						READ_MICRO_CHUNK(cload,STATICPHYS_VARIABLE_VISSECTORID,VisSectorID);
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}


/***********************************************************************************************
 * StaticPhysClass::On_Post_Load -- Post-Load callback                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/10/00    gth : Created.                                                                 *
 *=============================================================================================*/
void StaticPhysClass::On_Post_Load(void)
{
	PhysClass::On_Post_Load();
	WWASSERT(Model);
	if (Model) {

		// Set our cull box but don't let the culling system re-insert us.
		Set_Cull_Box(Model->Get_Bounding_Box(),true);

		// If this is a light-mapped mesh, exclude ourselves from the static lights
		Update_Cached_Model_Parameters();

		// If this is a particle emitter, start it emitting
		if (Model->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
			((ParticleEmitterClass *)Model)->Start ();
		}
	}
}


float	StaticPhysClass::Compute_Vis_Mesh_Ram(RenderObjClass * model)
{
	float total = 0.0f;

	if (model == NULL) {
		model = Model;
	}

	if (model != NULL) {
		if (model->Class_ID() == RenderObjClass::CLASSID_MESH) {

			if (model->Get_Collision_Type() & COLLISION_TYPE_VIS) {
				MeshModelClass * mdl = ((MeshClass *)model)->Get_Model();
				if (mdl != NULL) {
					total += sizeof(MeshClass) + sizeof(MeshModelClass);
					total += sizeof(Vector3) * mdl->Get_Vertex_Count();
					total += sizeof(TriIndex) * mdl->Get_Polygon_Count();
					total += sizeof(PlaneClass) * mdl->Get_Polygon_Count();
					mdl->Release_Ref();
				}
			}

		} else {
			for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
				RenderObjClass * sub_obj = model->Get_Sub_Object(i);
				if (sub_obj != NULL) {
					total += Compute_Vis_Mesh_Ram(sub_obj);
					REF_PTR_RELEASE(sub_obj);
				}
			}
		}
	}

	return total;
}


/**************************************************************************************
**
** StaticPhysDefClass Implementation
**
**************************************************************************************/

/*
** Persist factory for StaticPhysDefClass's
*/
SimplePersistFactoryClass<StaticPhysDefClass,PHYSICS_CHUNKID_STATICPHYSDEF>	_StaticPhysDefFactory;

/*
** Definition factory for StaticPhysDefClass.  This makes it show up in the editor
*/
DECLARE_DEFINITION_FACTORY(StaticPhysDefClass, CLASSID_STATICPHYSDEF, "StaticPhys") _StaticPhysDefDefFactory;

/*
** Chunk ID's used by StaticPhysDefClass
*/
enum 
{
	STATICPHYSDEF_CHUNK_PHYSDEF						= 0x01070002,			// (parent class)
	STATICPHYSDEF_CHUNK_VARIABLES,

	STATICPHYSDEF_VARIABLE_ISNONOCCLUDER			= 0,
};


StaticPhysDefClass::StaticPhysDefClass(void) :
	IsNonOccluder(true)
{
	EDITABLE_PARAM(StaticPhysDefClass, ParameterClass::TYPE_BOOL, IsNonOccluder);
}

uint32 StaticPhysDefClass::Get_Class_ID (void) const
{
	return CLASSID_STATICPHYSDEF; 
}

PersistClass * StaticPhysDefClass::Create(void) const
{
	StaticPhysClass * new_obj = NEW_REF(StaticPhysClass,());
	new_obj->Init(*this);
	return new_obj;
}

const char * StaticPhysDefClass::Get_Type_Name(void)
{ 
	return "StaticPhysDef"; 
}

bool StaticPhysDefClass::Is_Type(const char * type_name)
{
	if (openw3d::stricmp(type_name,StaticPhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return PhysDefClass::Is_Type(type_name);
	}
}

const PersistFactoryClass & StaticPhysDefClass::Get_Factory (void) const
{
	return _StaticPhysDefFactory;
}

bool StaticPhysDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(STATICPHYSDEF_CHUNK_PHYSDEF);
	PhysDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(STATICPHYSDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,STATICPHYSDEF_VARIABLE_ISNONOCCLUDER,IsNonOccluder);	
	csave.End_Chunk();
	return true;
}

bool StaticPhysDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case STATICPHYSDEF_CHUNK_PHYSDEF:
				PhysDefClass::Load(cload);
				break;

			case STATICPHYSDEF_CHUNK_VARIABLES:				
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,STATICPHYSDEF_VARIABLE_ISNONOCCLUDER,IsNonOccluder);	
					}
					cload.Close_Micro_Chunk();	
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}

		cload.Close_Chunk();
	}
	return true;
}
