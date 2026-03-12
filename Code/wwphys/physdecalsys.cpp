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
 *                     $Archive:: /Commando/Code/wwphys/physdecalsys.cpp                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 7/17/01 9:52p                                               $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "physdecalsys.h"
#include "colmathaabox.h"
#include "pscene.h"
#include "phys.h"
#include "camera.h"
#include "mesh.h"
#include "decalmsh.h"
#include "assetmgr.h"
#include "wwdebug.h"
#include "robjlist.h"
#include "texture.h"


#define DEBUG_DECALS 0

const float DECAL_BACKFACE_THRESHHOLD	= 0.4f;		// backface rejection threshhold
const float DECAL_HALF_SLAB_THICKNESS	= 0.2f;		// thickness of the bounding volume for decals






PhysDecalSysClass::PhysDecalSysClass(PhysicsSceneClass * parent_scene) :
	ParentScene(parent_scene),
	CreatePermanentDecals(false),
	NextTempDecalIndex(0),
	DecalMaterial(NULL),
	DecalShader(0)
{
	allocate_resources();
	Set_Temporary_Decal_Pool_Size(50);
}

PhysDecalSysClass::~PhysDecalSysClass(void)
{
	release_resources();
}

void PhysDecalSysClass::Update_Decal_Fade_Distances(const CameraClass & camera)
{
	/*
	** Since we don't have decal fading any more, set the decal fade distance to the far clip plane
	*/
	float znear,zfar;
	camera.Get_Clip_Planes(znear,zfar);
	WW3D::Set_Decal_Rejection_Distance(zfar);
}

int PhysDecalSysClass::Create_Decal
(
	const Matrix3D &	tm,
	const char *		texture_name,
	float					radius,
	bool					is_permanent,
	bool					apply_to_translucent_meshes,
	PhysClass *			only_this_obj
)
{
	/*
	** Allocate the decal generator
	*/
	CreatePermanentDecals = is_permanent;
	DecalGeneratorClass * gen = Lock_Decal_Generator();
	WWASSERT(gen != NULL);

	/*
	** Set up the transform, projection, and bounding volume parameters
	** I want a thin bounding volume.  Since the transform given is right at the impact point,
	** we need to back-up some amount and then set the near and far z depths such that
	** they bound the point
	*/
	float backup_dist = DECAL_HALF_SLAB_THICKNESS + 0.01f;

	Matrix3D transform = tm;
	transform.Translate_Z(backup_dist);
	gen->Set_Transform(transform);
	gen->Set_Ortho_Projection(-radius,radius,-radius,radius,0.01f,backup_dist + 2.0f * DECAL_HALF_SLAB_THICKNESS);
	gen->Set_Backface_Threshhold(DECAL_BACKFACE_THRESHHOLD);
	gen->Apply_To_Translucent_Meshes(apply_to_translucent_meshes);

#if DEBUG_DECALS
	ParentScene->Add_Debug_OBBox(gen->Get_Bounding_Volume(),Vector3(0,0,1));
	ParentScene->Add_Debug_Axes(tm,Vector3(1,1,1));
	ParentScene->Add_Debug_Axes(transform,Vector3(1,1,1));
#endif

	/*
	** Set up the material settings.  Just plug in the standard alpha shader and the
	** vertex material which all decals use.  Then grab the texture which the user
	** specified...
	*/
	MaterialPassClass	* material = gen->Get_Material();

	material->Set_Shader(DecalShader);
	material->Set_Material(DecalMaterial);

	TextureClass * tex = WW3DAssetManager::Get_Instance()->Get_Texture(texture_name,TextureClass::MIP_LEVELS_ALL);
	tex->Set_U_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
	tex->Set_V_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);

	material->Set_Texture(tex);
	tex->Release_Ref();
	material->Release_Ref();

	/*
	** Collect objects to apply the decal to
	*/
	NonRefPhysListClass list;
	if (only_this_obj == NULL) {
		ParentScene->Collect_Objects(gen->Get_Bounding_Volume(),true,false,&list);
	} else {
		list.Add(only_this_obj);
	}

	/*
	** Apply!
	*/
	NonRefPhysListIterator it(&list);
	for (it.First(); !it.Is_Done(); it.Next()) {
		it.Peek_Obj()->Peek_Model()->Create_Decal(gen);
	}

	/*
	** Done.  Return the logical id of the decal if we actually created anything,
	** otherwise return -1
	*/
	int return_id = -1;
	if (gen->Get_Mesh_List().Is_Empty() == false) {
		return_id = gen->Get_Decal_ID();
	}

	Unlock_Decal_Generator(gen);
	return return_id;
}

bool PhysDecalSysClass::Remove_Decal(uint32 id)
{
	return internal_remove_decal(id,NULL);
}

void PhysDecalSysClass::Unlock_Decal_Generator(DecalGeneratorClass * generator)
{
	/*
	** If the generator actually created decal polygons, add the results
	** to our internal pools
	*/
	if (generator->Get_Mesh_List().Is_Empty() == false) {

		if (is_decal_id_permanent(generator->Get_Decal_ID())) {

			LogicalDecalClass * newdecal = new LogicalDecalClass;
			newdecal->Init(generator);
			PermanentDecals.Add(newdecal);

		} else {

			/*
			** the id should match the "next temp decal index"
			*/
			WWASSERT(generator->Get_Decal_ID() == NextTempDecalIndex);
			TempDecals[NextTempDecalIndex].Init(generator);

			/*
			** bump the index forward
			*/
			NextTempDecalIndex = (NextTempDecalIndex + 1) % TempDecals.Length();

		}
	}

	DecalSystemClass::Unlock_Decal_Generator(generator);
}

void PhysDecalSysClass::Decal_Mesh_Destroyed(uint32 decal_id,DecalMeshClass * mesh)
{
	/*
	** Must remove this mesh from any decals
	*/
	internal_remove_decal(decal_id,mesh->Peek_Parent());
}

void PhysDecalSysClass::Set_Temporary_Decal_Pool_Size(int count)
{
	WWASSERT(count > 0);

	/*
	** If we are shrinking, remove all decals in the slots that are going away
	*/
	if (count < TempDecals.Length()) {
		for (int i=count; i<TempDecals.Length(); i++) {
			TempDecals[i].Reset();
		}
	}

	/*
	** Resize the array
	*/
	TempDecals.Resize(count);

	/*
	** Make sure that our NextTempDecalIndex is valid, if not wrap it around
	*/
	if (NextTempDecalIndex >= (uint32)TempDecals.Length()) {
		NextTempDecalIndex = 0;
	}
}

int PhysDecalSysClass::Get_Temporary_Decal_Pool_Size(void)
{
	return TempDecals.Length();
}

uint32 PhysDecalSysClass::Generate_Decal_Id(void)
{
	uint32 id;
	if (CreatePermanentDecals) {

		id = Generate_Unique_Global_Decal_Id() & 0x7FFFFFFF;
		id |= 0x80000000;

	} else {

		id = NextTempDecalIndex;

	}
	return id;
}

bool PhysDecalSysClass::is_decal_id_permanent(uint32 id)
{
	if (id & 0x80000000) {
		return true;
	} else {
		return false;
	}
}

void PhysDecalSysClass::allocate_resources(void)
{
	WWASSERT(DecalMaterial == NULL);
	DecalMaterial = NEW_REF(VertexMaterialClass,());
	DecalMaterial->Set_Ambient(0,0,0);
	DecalMaterial->Set_Diffuse(0,0,0);
	DecalMaterial->Set_Specular(0,0,0);
	DecalMaterial->Set_Emissive(1,1,1);
	DecalMaterial->Set_Opacity(1.0f);
	DecalMaterial->Set_Shininess(0.0f);

	DecalShader = ShaderClass::_PresetAlphaShader;
}

void PhysDecalSysClass::release_resources(void)
{
	REF_PTR_RELEASE(DecalMaterial);
}

bool PhysDecalSysClass::internal_remove_decal(uint32 id,MeshClass * deleted_mesh)
{
	bool success = false;
	if (is_decal_id_permanent(id)) {

		/*
		** Find the decal with the given id in the Permanent decal array
		*/
		MultiListIterator<LogicalDecalClass> it(&PermanentDecals);
		for (it.First(); !it.Is_Done(); it.Next()) {

			LogicalDecalClass * decal = it.Get_Obj();

			if (decal->DecalID == id) {

				/*
				** Just remove the deleted mesh from our list.  We don't need to delete
				** its decals since it is telling us that it has been deleted already.
				*/
				if (deleted_mesh != NULL) {
					decal->Meshes.Delete(deleted_mesh);
				}

				/*
				** Now destroy this logical decal, this results in all decal-meshes that
				** are referenced by this logical decal being told to remove this decal id.
				*/
				PermanentDecals.Remove(decal);
				delete decal;
				success = true;
				break;
			}
		}

	} else {

		/*
		** Temporary decals use their index as the id
		*/
		WWASSERT(id >= 0);
		WWASSERT(id < (uint32)TempDecals.Length());

		if (deleted_mesh != NULL) {
			TempDecals[id].Meshes.Delete(deleted_mesh);
		}

		TempDecals[id].Reset();
		success = true;
	}
	return success;
}


/*
** LogicalDecalClass Implementation
*/

PhysDecalSysClass::LogicalDecalClass::LogicalDecalClass(void) :
	DecalID(0xFFFFFFFF)
{
}

PhysDecalSysClass::LogicalDecalClass::~LogicalDecalClass(void)
{
	Reset();
}

void PhysDecalSysClass::LogicalDecalClass::Init(DecalGeneratorClass * gen)
{
	/*
	** Reset ourselves in case we were being used to track a previous decal.
	** This causes that decal to get removed from the system.
	*/
	Reset();

	/*
	** Record the data for this decal
	*/
	DecalID = gen->Get_Decal_ID();
	NonRefRenderObjListIterator it(&(gen->Get_Mesh_List()));
	for (it.First(); !it.Is_Done(); it.Next()) {
		MeshClass * mesh = (MeshClass *)it.Get_Obj();
		WWASSERT(mesh->Class_ID()==RenderObjClass::CLASSID_MESH);
		Meshes.Add(mesh);
	}
}

void PhysDecalSysClass::LogicalDecalClass::Reset(void)
{
	for (int i=0; i<Meshes.Count(); i++) {
		Meshes[i]->Delete_Decal(DecalID);
	}
	Meshes.Delete_All();
	DecalID = 0xFFFFFFFF;
}

