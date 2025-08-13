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
 *                 Project Name : wwphys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/lightsolve.cpp                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 3/28/02 11:53a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "lightsolve.h"
#include "phys.h"
#include "staticphys.h"
#include "rendobj.h"
#include "mesh.h"
#include "dx8renderer.h"
#include "simplevec.h"
#include "vp.h"
#include "lightphys.h"
#include "light.h"
#include "lightsolvecontext.h"
#include "lightsolveprogress.h"
#include "renegadeterrainpatch.h"


/**
** VertexSolveClass
** This class does the job of generated a vertex solve for a mesh.  
*/
class VertexSolveClass
{
public:
	VertexSolveClass(void) {}
	
	void Light_Mesh(LightSolveContextClass & context,MeshClass * mesh, NonRefPhysListClass & lights);
	void Light_Terrain(LightSolveContextClass & context,RenegadeTerrainPatchClass * patch, NonRefPhysListClass & lights);

protected:

	void Grow_Arrays(int new_vcount);
	void Add_Light_To_Vertex(LightSolveContextClass & context,int idx,LightClass * light_obj);

	SimpleVecClass<Vector3>		Position;
	SimpleVecClass<Vector3>		Normal;
	SimpleVecClass<Vector4>		MeshAmbient;
	SimpleVecClass<Vector4>		MeshDiffuse;

	SimpleVecClass<Vector4>		AmbientSolve;
	SimpleVecClass<Vector4>		DiffuseSolve;
	SimpleVecClass<Vector4>		Solve;

};

static VertexSolveClass TheVertexSolver;

void VertexSolveClass::Light_Mesh(LightSolveContextClass & context,MeshClass * mesh, NonRefPhysListClass & lights)
{
	int vi;
	context.Get_Progress().Set_Current_Mesh_Name(mesh->Get_Name());
	context.Get_Progress().Set_Current_Mesh_Vertex_Count(mesh->Peek_Model()->Get_Vertex_Count());

	/*
	** Get the model and initialize our arrays
	*/
	MeshModelClass * model = mesh->Peek_Model();
	int vcount = model->Get_Vertex_Count();
	Grow_Arrays(vcount);

	/*
	** Transform the positions and normals into world space
	*/
	Matrix3D tm = mesh->Get_Transform();
	const Vector3 * src_verts = model->Get_Vertex_Array();
	const Vector3 * src_norms = model->Get_Vertex_Normal_Array();

	VectorProcessorClass::Transform(&(Position[0]), src_verts, tm, vcount);
	tm.Set_Translation(Vector3(0,0,0));
	VectorProcessorClass::Transform(&(Normal[0]), src_norms, tm, vcount);

	/*
	** Fill the mesh ambient and diffuse arrays with the per vertex material
	** color (either from the material itself or the vertex color array)
	**
	** LIGHT SOLVE GENERATION RULES:
	**	- for each vertex
	**		- for ambient and diffuse
	**			- if this vertex is supposed to use a color array in any pass copy the color from the array
	**			- else copy the material setting from pass0
	**
	** LIGHT SOLVE INSTALL RULES:
	** - for each pass
	**		- for each vertex material
	**			- if this material has any emissive component, leave the vmtl alone
	**			- else, point the emissive source to the new light solve color array, 
	**			  make the diffuse and ambient use the material settings.
	*/
	unsigned * dcg = NULL; 
	for (int pi=0; pi<model->Get_Pass_Count(); pi++) {
		if (model->Get_DCG_Array(pi) != NULL) {
			dcg = model->Get_DCG_Array(pi);
		}
	}
	
	for (vi=0; vi<model->Get_Vertex_Count(); vi++) {

		bool use_array = false;
		for (int pi=0; pi<model->Get_Pass_Count(); pi++) {
			VertexMaterialClass * pass_mtl = model->Peek_Material(vi,pi);
			if (	(pass_mtl != NULL) && 
					(dcg != NULL) && 
					(pass_mtl->Get_Diffuse_Color_Source() == VertexMaterialClass::COLOR1) ) 
			{
				use_array = true;
			}
		}

		/*
		** Set up the ambient color for this vertex
		*/
		VertexMaterialClass * vmtl = model->Peek_Material(vi,0);
		if (use_array) {
			MeshAmbient[vi] = DX8Wrapper::Convert_Color(dcg[vi]);
		} else if (vmtl != NULL) {
			Vector3 ambient;
			vmtl->Get_Ambient(&ambient);
			MeshAmbient[vi] = Vector4(ambient.X,ambient.Y,ambient.Z,1.0f);
		} else {
			MeshAmbient[vi].Set(1,1,1,1);
		}

		/*
		** Set up the diffuse color for this vertex
		*/
		if (use_array) {
			MeshDiffuse[vi] = DX8Wrapper::Convert_Color(dcg[vi]);
		} else if (vmtl != NULL) {
			Vector3 diffuse;
			vmtl->Get_Diffuse(&diffuse);
			MeshDiffuse[vi] = Vector4(diffuse.X,diffuse.Y,diffuse.Z,1.0f);
		} else {
			MeshDiffuse[vi].Set(1,1,1,1);
		}
	}

	/*
	** Compute the light solve
	*/
	LightClass * sun = PhysicsSceneClass::Get_Instance()->Get_Sun_Light();
	Vector3 scene_ambient = PhysicsSceneClass::Get_Instance()->Get_Ambient_Light();
	int callback_counter = 0;

	for (vi=0; vi<model->Get_Vertex_Count(); vi++) {

		AmbientSolve[vi].X = scene_ambient.X;
		AmbientSolve[vi].Y = scene_ambient.Y;
		AmbientSolve[vi].Z = scene_ambient.Z;
		AmbientSolve[vi].W = 1.0f;
		
		DiffuseSolve[vi].X = 0.0f;
		DiffuseSolve[vi].Y = 0.0f;
		DiffuseSolve[vi].Z = 0.0f;
		DiffuseSolve[vi].W = MeshDiffuse[vi].W;

		/*
		** Sun
		*/
		Add_Light_To_Vertex(context,vi,sun);

		/*
		** Other lights
		*/
		NonRefPhysListIterator it(&lights);
		while (!it.Is_Done()) {
			
			LightPhysClass * light = it.Peek_Obj()->As_LightPhysClass();
			if ((light) && (light->Peek_Model()) && (light->Peek_Model()->Class_ID() == RenderObjClass::CLASSID_LIGHT)) {

				LightClass * light_obj = (LightClass*)light->Peek_Model();

				Add_Light_To_Vertex(context,vi,light_obj);
			}
			it.Next();
		}

		context.Get_Progress().Set_Current_Vertex(vi);

		callback_counter++;
		if (callback_counter > 100) {
			callback_counter = 0;
			context.Update_Observer();
		}
	}

	VectorProcessorClass::Clamp(&(AmbientSolve[0]),&(AmbientSolve[0]), 0.0f, 1.0f,vcount);
	VectorProcessorClass::Clamp(&(DiffuseSolve[0]),&(DiffuseSolve[0]), 0.0f, 1.0f,vcount);

	/*
	** Modulate the accumulated light by the material properties
	*/
	for (vi=0; vi<vcount; vi++) {
		Solve[vi].X = AmbientSolve[vi].X * MeshAmbient[vi].X + DiffuseSolve[vi].X * MeshDiffuse[vi].X;
		Solve[vi].Y = AmbientSolve[vi].Y * MeshAmbient[vi].Y + DiffuseSolve[vi].Y * MeshDiffuse[vi].Y;
		Solve[vi].Z = AmbientSolve[vi].Z * MeshAmbient[vi].Z + DiffuseSolve[vi].Z * MeshDiffuse[vi].Z;
		Solve[vi].W = DiffuseSolve[vi].W;
	}
	VectorProcessorClass::Clamp(&(Solve[0]),&(Solve[0]), 0.0f, 1.0f,vcount);

	mesh->Install_User_Lighting_Array(&(Solve[0]));
	context.Update_Observer();
	REF_PTR_RELEASE(sun);
}

static Vector3 _offset (0, 0, 0);

void VertexSolveClass::Light_Terrain(LightSolveContextClass & context,RenegadeTerrainPatchClass * patch, NonRefPhysListClass & lights)
{
	int vi;
	context.Get_Progress().Set_Current_Mesh_Name(patch->Get_Name());
	context.Get_Progress().Set_Current_Mesh_Vertex_Count(patch->Get_Vertex_Count());

	/*
	** Get the model and initialize our arrays
	*/
	int vcount = patch->Get_Vertex_Count();
	Grow_Arrays(vcount);

	/*
	** Transform the positions and normals into world space
	*/
	Matrix3D tm = patch->Get_Transform();
	const Vector3 * src_verts = patch->Get_Vertex_Array();
	const Vector3 * src_norms = patch->Get_Vertex_Normal_Array();

	VectorProcessorClass::Transform(&(Position[0]), src_verts, tm, vcount);
	tm.Set_Translation(Vector3(0,0,0));
	VectorProcessorClass::Transform(&(Normal[0]), src_norms, tm, vcount);

	/*
	** For heightfield terrains, the mesh ambient and diffuse colors are white
	*/
	for (int index = 0; index < vcount; index ++) {
		MeshAmbient[index].Set (1.0F, 1.0F, 1.0F, 1.0F);
		MeshDiffuse[index].Set (1.0F, 1.0F, 1.0F, 1.0F);
	}

	/*
	** Compute the light solve
	*/
	LightClass * sun = PhysicsSceneClass::Get_Instance()->Get_Sun_Light();
	Vector3 scene_ambient = PhysicsSceneClass::Get_Instance()->Get_Ambient_Light();
	int callback_counter = 0;

	for (vi = 0; vi < vcount; vi ++) {

		AmbientSolve[vi].X = scene_ambient.X;
		AmbientSolve[vi].Y = scene_ambient.Y;
		AmbientSolve[vi].Z = scene_ambient.Z;
		AmbientSolve[vi].W = 1.0f;
		
		DiffuseSolve[vi].X = 0.0f;
		DiffuseSolve[vi].Y = 0.0f;
		DiffuseSolve[vi].Z = 0.0f;
		DiffuseSolve[vi].W = MeshDiffuse[vi].W;

		/*
		** Sun
		*/
		Add_Light_To_Vertex(context,vi,sun);

		/*
		** Other lights
		*/
		NonRefPhysListIterator it(&lights);
		while (!it.Is_Done()) {
			
			LightPhysClass * light = it.Peek_Obj()->As_LightPhysClass();
			if ((light) && (light->Peek_Model()) && (light->Peek_Model()->Class_ID() == RenderObjClass::CLASSID_LIGHT)) {

				LightClass * light_obj = (LightClass*)light->Peek_Model();

				Add_Light_To_Vertex(context,vi,light_obj);
			}
			it.Next();
		}

		context.Get_Progress().Set_Current_Vertex(vi);

		callback_counter++;
		if (callback_counter > 100) {
			callback_counter = 0;
			context.Update_Observer();
		}
	}

	VectorProcessorClass::Clamp(&(AmbientSolve[0]),&(AmbientSolve[0]), 0.0f, 1.0f,vcount);
	VectorProcessorClass::Clamp(&(DiffuseSolve[0]),&(DiffuseSolve[0]), 0.0f, 1.0f,vcount);

	/*
	** Modulate the accumulated light by the material properties
	*/
	for (vi=0; vi<vcount; vi++) {
		Solve[vi].X = AmbientSolve[vi].X * MeshAmbient[vi].X + DiffuseSolve[vi].X * MeshDiffuse[vi].X;
		Solve[vi].Y = AmbientSolve[vi].Y * MeshAmbient[vi].Y + DiffuseSolve[vi].Y * MeshDiffuse[vi].Y;
		Solve[vi].Z = AmbientSolve[vi].Z * MeshAmbient[vi].Z + DiffuseSolve[vi].Z * MeshDiffuse[vi].Z;
		Solve[vi].W = DiffuseSolve[vi].W;
	}
	VectorProcessorClass::Clamp(&(Solve[0]),&(Solve[0]), 0.0f, 1.0f,vcount);

	/*
	**	Pass the resultant vertex colors onto the patch
	*/
	for (vi=0; vi<vcount; vi++) {
		patch->Set_Vertex_Color (vi, Vector3 (Solve[vi].X, Solve[vi].Y, Solve[vi].Z));
	}

	//
	//	Let the patch know that it is now prelit
	//
	patch->Set_Is_Prelit (true);
	return ;
}


void VertexSolveClass::Add_Light_To_Vertex(LightSolveContextClass & context,int vi,LightClass * light_obj)
{				
	if (light_obj->Is_Within_Attenuation_Radius(Position[vi])) {
	
		// cast ray...
		bool is_occluded = false;

// FIXME need a collision group for light occlusion here...
		if (context.Is_Occlusion_Enabled()) {
			CastResultStruct res;
			Vector3 p0 = Position[vi];
			Vector3 p1 = light_obj->Get_Position();

			if (light_obj->Get_Type() == LightClass::DIRECTIONAL) {
				Vector3 dir = -(light_obj->Get_Transform().Get_Z_Vector());
				p1 = p0 + dir * light_obj->Get_Attenuation_Range();
			}

			const float MOVE_AMOUNT = 0.25f;		// can't be occluded closer than this
			Vector3 delta = p1-p0;
			delta.Normalize();
			p0 += MOVE_AMOUNT * delta;

			p0 += _offset;
			p1 += _offset;

			LineSegClass ray(p0,p1);
			PhysRayCollisionTestClass raytest(ray,&res,0,COLLISION_TYPE_PROJECTILE);
			raytest.CheckDynamicObjs = false;
			PhysicsSceneClass::Get_Instance()->Cast_Ray(raytest);

			if (res.Fraction < 1.0f) {
				is_occluded = true;
			}
		}

		if (!is_occluded) {

			Vector3 ambient;
			Vector3 diffuse;
			light_obj->Compute_Lighting(Position[vi],Normal[vi],&ambient,&diffuse);

			AmbientSolve[vi] += Vector4(ambient.X,ambient.Y,ambient.Z,0.0f);
			DiffuseSolve[vi] += Vector4(diffuse.X,diffuse.Y,diffuse.Z,0.0f);
		}
	}
}

void VertexSolveClass::Grow_Arrays(int vcount)
{
	Position.Uninitialised_Grow(vcount);
	Normal.Uninitialised_Grow(vcount);
	AmbientSolve.Uninitialised_Grow(vcount);
	DiffuseSolve.Uninitialised_Grow(vcount);
	Solve.Uninitialised_Grow(vcount);
	MeshAmbient.Uninitialised_Grow(vcount);
	MeshDiffuse.Uninitialised_Grow(vcount);
}




/*****************************************************************************
**
** LightSolveClass Implementation
**
*****************************************************************************/

/*
  Global Solve:
  - start with StaticObjList
  - pass list to cull function 
  - cull function gens new list with objects that will not be lit removed
  - pass to solve function which accepts a list
  - iterate list, pass each to solve function for an object

  List Solve:
  - pass list to cull function
  - cull function gens new list with objects that will not be lit removed
  - pass to solve function which accepts a list
  - iterate list, pass each to solve function for an object
*/


void LightSolveClass::Generate_Static_Light_Solve(LightSolveContextClass & context)
{
	RefPhysListIterator it = PhysicsSceneClass::Get_Instance()->Get_Static_Object_Iterator();
	
	/*
	** Build a list of the objects to be solved
	*/
	RefPhysListClass solve_list;
	while (!it.Is_Done()) {
		StaticPhysClass * obj = it.Peek_Obj()->As_StaticPhysClass();
		if ((obj != NULL) && (Does_Obj_Get_Static_Light_Solve(obj))) {
			solve_list.Add(obj);
		}
		it.Next();
	}


	/*
	** Pass the list to the solve function
	*/
	Compute_Solve(context,solve_list);
}

void LightSolveClass::Generate_Static_Light_Solve(LightSolveContextClass & context,RefPhysListClass & input_list)
{
	RefPhysListClass solve_list;

	/*
	** Cull out objects which should not get a lighting solve
	*/
	RefPhysListIterator it(&input_list);
	while (!it.Is_Done()) {
		StaticPhysClass * obj = it.Peek_Obj()->As_StaticPhysClass();
		if ((obj != NULL) && (Does_Obj_Get_Static_Light_Solve(obj))) {
			solve_list.Add(obj);
		}
		it.Next();
	}

	/*
	** Pass the list to the solve function
	*/
	Compute_Solve(context,solve_list);
}


void LightSolveClass::Compute_Solve(LightSolveContextClass & context,RefPhysListClass & obj_list)
{
	RefPhysListIterator it(&obj_list);

	/*
	** Count the objects for the progress display
	*/
	int count = 0;
	while (!it.Is_Done()) {
		count++;
		it.Next();
	}
	context.Get_Progress().Set_Object_Count(count);
	
	/*
	** Generate a light solve for each static object
	*/
	it.First();
	while (!it.Is_Done() && !context.Get_Progress().Is_Cancel_Requested()) {
		StaticPhysClass * obj = it.Peek_Obj()->As_StaticPhysClass();
		if (obj != NULL) {
			Compute_Solve(context,obj);
			context.Get_Progress().Increment_Processed_Object_Count();
		}
		it.Next();
	}

	/*
	** Re-gen all VB's, IB's, etc
	*/
	TheDX8MeshRenderer.Invalidate();
}

void LightSolveClass::Compute_Solve(LightSolveContextClass & context,StaticPhysClass * obj)
{
	WWASSERT(obj != NULL);
	if (context.Get_Progress().Get_Object_Count() == 0) {
		context.Get_Progress().Set_Object_Count(1);
	}

	/*
	** Generate a light solve for each mesh in this object
	*/
	if (obj->Is_Model_Pre_Lit() == false) {
		NonRefPhysListClass light_list;
		PhysicsSceneClass::Get_Instance()->Collect_Lights(obj->Get_Cull_Box(),true,false,&light_list);
		Compute_Solve(context,obj->Peek_Model(),light_list);
	}
}

void LightSolveClass::Compute_Solve(LightSolveContextClass & context,RenderObjClass * obj,NonRefPhysListClass & light_list)
{
	/*
	** Mark this render object as containing a static lighting solve
	*/
	obj->Set_Has_User_Lighting(true);

	/*
	** Recurse through all sub-objects
	*/
	WWASSERT(obj != NULL);
	for (int i=0; i<obj->Get_Num_Sub_Objects(); i++) {
		RenderObjClass * sub_obj = obj->Get_Sub_Object(i);
		if (sub_obj != NULL) {
			Compute_Solve(context,sub_obj,light_list);
			REF_PTR_RELEASE(sub_obj);
		}
	}

	/*
	** For all mesh objects, generate a vertex lighting solution.
	*/
	if (obj->Class_ID() == RenderObjClass::CLASSID_MESH) {
		MeshClass * mesh = (MeshClass*)obj;
		WWDEBUG_SAY(("Generating Solve for Mesh: %s\r\n",mesh->Get_Name()));
		TheVertexSolver.Light_Mesh(context,mesh,light_list);		
	} else if (obj->Class_ID() == RenderObjClass::CLASSID_RENEGADE_TERRAIN) {
		TheVertexSolver.Light_Terrain(context,(RenegadeTerrainPatchClass *)obj,light_list);
	}
}

bool LightSolveClass::Does_Obj_Get_Static_Light_Solve(StaticPhysClass * obj)
{
	if (obj->Is_Model_Pre_Lit()) {
		return false;
	}

	return Does_Model_Get_Static_Light_Solve(obj->Peek_Model());
}

bool LightSolveClass::Does_Model_Get_Static_Light_Solve(RenderObjClass * model)
{
	if (model == NULL) {
		return false;
	}

	if (!model->Is_Not_Hidden_At_All()) {
		return false;
	}

	if (model->Get_Num_Sub_Objects() > 0) {
		bool any_sub_objs_visible = false;
		for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
			RenderObjClass * sub_obj = model->Get_Sub_Object(i);
			if (sub_obj != NULL) {
				if (sub_obj->Is_Not_Hidden_At_All()) {
					any_sub_objs_visible = true;
				}
				REF_PTR_RELEASE(sub_obj);
			}
		}	

		if (any_sub_objs_visible == false) {
			return false;
		}
	}

	// TODO: check if the entire model is emissive???

	return true;
}





