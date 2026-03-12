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
 *                     $Archive:: /Commando/Code/wwphys/umbrasupport.cpp                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/29/00 4:46p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "umbrasupport.h"

#if (UMBRASUPPORT)

#include <umbra.hpp>
#include "mesh.h"
#include "meshmdl.h"
#include "camera.h"
#include "phys.h"

class UmbraCommanderClass;


/*
** Static Variables
*/
static Umbra::Model *					_DummySphere = NULL;
static Umbra::Cell *						_TheCell = NULL;
static Umbra::Camera *					_TheCamera = NULL;
static UmbraCommanderClass *			_TheCommander = NULL;
static bool									_UmbraEnabled = true;

static Umbra::Vector3					_umbra_box_verts[8];
static Umbra::Vector3i					_umbra_box_faces[12];
static int									_umbra_box_vert_count = 8;
static int									_umbra_box_face_count = 12;


/*
**
** Local utility functions
**
*/
static void _convert_matrix_westwood_to_umbra(const Matrix3D & in,Umbra::Matrix4x4 & out)
{
#if 0
	for (int j=0;j<3;j++) {
		for (int i=0;i<4;i++) {
			out.m[j][i] = in[j][i];
		}
	}
	out.m[3][0] = 0.0;
	out.m[3][1] = 0.0;
	out.m[3][2] = 0.0;
	out.m[3][3] = 1.0;
#endif
#if 0
	for (int j=0;j<3;j++) {
		for (int i=0; i<3; i++) {
			out.m[i][j] = in[j][i];
		}
	}

	out.m[0][3] = in[0][3];
	out.m[1][3] = in[1][3];
	out.m[2][3] = in[2][3];

	out.m[3][0] = 0.0;
	out.m[3][1] = 0.0;
	out.m[3][2] = 0.0;
	out.m[3][3] = 1.0;
#endif

#if 1
	for (int j=0;j<3;j++) {
		for (int i=0;i<4;i++) {
			out.m[i][j] = in[j][i];
		}
	}
	out.m[0][3] = 0.0;
	out.m[1][3] = 0.0;
	out.m[2][3] = 0.0;
	out.m[3][3] = 1.0;
#endif
}


static void _convert_frustum_westwood_to_umbra(const CameraClass & camera,Umbra::Frustum & out)
{
	camera.Get_Clip_Planes(out.zNear,out.zFar);

	Vector2 vmin,vmax;
	camera.Get_View_Plane(vmin,vmax);

	out.left = vmin.X;
	out.right = vmax.X;
	out.top = vmax.Y;
	out.bottom = vmin.Y;

//	out.zFar = 1500.0;
}


static void _init_umbra_box(const AABoxClass & obj_bound_box)
{
	_umbra_box_verts[0].v[0] = obj_bound_box.Center.X + obj_bound_box.Extent.X;
	_umbra_box_verts[0].v[1] = obj_bound_box.Center.Y + obj_bound_box.Extent.Y;
	_umbra_box_verts[0].v[2] = obj_bound_box.Center.Z - obj_bound_box.Extent.Z;

	_umbra_box_verts[1].v[0] = obj_bound_box.Center.X - obj_bound_box.Extent.X;
	_umbra_box_verts[1].v[1] = obj_bound_box.Center.Y + obj_bound_box.Extent.Y;
	_umbra_box_verts[1].v[2] = obj_bound_box.Center.Z - obj_bound_box.Extent.Z;

	_umbra_box_verts[2].v[0] = obj_bound_box.Center.X - obj_bound_box.Extent.X;
	_umbra_box_verts[2].v[1] = obj_bound_box.Center.Y - obj_bound_box.Extent.Y;
	_umbra_box_verts[2].v[2] = obj_bound_box.Center.Z - obj_bound_box.Extent.Z;

	_umbra_box_verts[3].v[0] = obj_bound_box.Center.X + obj_bound_box.Extent.X;
	_umbra_box_verts[3].v[1] = obj_bound_box.Center.Y - obj_bound_box.Extent.Y;
	_umbra_box_verts[3].v[2] = obj_bound_box.Center.Z - obj_bound_box.Extent.Z;

	_umbra_box_verts[4].v[0] = obj_bound_box.Center.X + obj_bound_box.Extent.X;
	_umbra_box_verts[4].v[1] = obj_bound_box.Center.Y + obj_bound_box.Extent.Y;
	_umbra_box_verts[4].v[2] = obj_bound_box.Center.Z + obj_bound_box.Extent.Z;

	_umbra_box_verts[5].v[0] = obj_bound_box.Center.X - obj_bound_box.Extent.X;
	_umbra_box_verts[5].v[1] = obj_bound_box.Center.Y + obj_bound_box.Extent.Y;
	_umbra_box_verts[5].v[2] = obj_bound_box.Center.Z + obj_bound_box.Extent.Z;

	_umbra_box_verts[6].v[0] = obj_bound_box.Center.X - obj_bound_box.Extent.X;
	_umbra_box_verts[6].v[1] = obj_bound_box.Center.Y - obj_bound_box.Extent.Y;
	_umbra_box_verts[6].v[2] = obj_bound_box.Center.Z + obj_bound_box.Extent.Z;

	_umbra_box_verts[7].v[0] = obj_bound_box.Center.X + obj_bound_box.Extent.X;
	_umbra_box_verts[7].v[1] = obj_bound_box.Center.Y - obj_bound_box.Extent.Y;
	_umbra_box_verts[7].v[2] = obj_bound_box.Center.Z + obj_bound_box.Extent.Z;


	// -z face
	_umbra_box_faces[0].i = 2; _umbra_box_faces[0].j = 0; _umbra_box_faces[0].k = 3;
	_umbra_box_faces[1].i = 2; _umbra_box_faces[1].j = 1; _umbra_box_faces[1].k = 0;

	// +z face
	_umbra_box_faces[2].i = 6; _umbra_box_faces[2].j = 7; _umbra_box_faces[2].k = 4;
	_umbra_box_faces[3].i = 6; _umbra_box_faces[3].j = 4; _umbra_box_faces[3].k = 5;

	// +x face
	_umbra_box_faces[4].i = 7; _umbra_box_faces[4].j = 0; _umbra_box_faces[4].k = 4;
	_umbra_box_faces[5].i = 7; _umbra_box_faces[5].j = 3; _umbra_box_faces[5].k = 0;

	// -x face
	_umbra_box_faces[6].i = 6; _umbra_box_faces[6].j = 5; _umbra_box_faces[6].k = 1;
	_umbra_box_faces[7].i = 6; _umbra_box_faces[7].j = 1; _umbra_box_faces[7].k = 2;

	// +y face
	_umbra_box_faces[8].i = 5; _umbra_box_faces[8].j = 4; _umbra_box_faces[8].k = 0;
	_umbra_box_faces[9].i = 5; _umbra_box_faces[9].j = 0; _umbra_box_faces[9].k = 1;

	// -y face
	_umbra_box_faces[10].i = 2; _umbra_box_faces[10].j = 7; _umbra_box_faces[10].k = 6;
	_umbra_box_faces[11].i = 2; _umbra_box_faces[11].j = 3; _umbra_box_faces[11].k = 7;
}






/**
** UmbraCommanderClass
** This is our commander for recording the results of a visibility query to Umbra
*/
class UmbraCommanderClass : public Umbra::Commander
{
public:

	UmbraCommanderClass(void) : VisObjList(NULL)
	{
	}

	void Set_List(RefPhysListClass & visible_obj_list)
	{
		VisObjList = &visible_obj_list;
	}

	virtual void command(Command cmd)
	{
		WWASSERT(VisObjList != NULL);
		if (cmd == Umbra::Commander::INSTANCE_VISIBLE)
		{
			const Umbra::Object * obj = getInstance()->getObject();
			if (obj != NULL) {
				VisObjList->Add((PhysClass *)(obj->getUserPointer()));
			}
		}
	}

protected:

	RefPhysListClass * VisObjList;
};





/*
** UmbraSupport implementation
*/
void UmbraSupport::Init(void)
{
	/*
	** Initialize UMBRA
	*/
	Umbra::Library::init(Umbra::LibraryDefs::COLUMN_MAJOR);

	/*
	** Create the dummy sphere model
	*/
	Umbra::Vector3 center;
	center.v[0] = 0.0; center.v[1] = 0.0; center.v[2] = 0.0;
	_DummySphere = new Umbra::SphereModel(center,1.0);

	/*
	** Create a Cell to put everything (we don't have portals so everything is in one cell)
	*/
	_TheCell = new Umbra::Cell;

	/*
	** Create the camera
	*/
	_TheCamera = new Umbra::Camera;
	_TheCamera->setCell(_TheCell);

//	_TheCamera->setParameters(	640,480,0);
//	_TheCamera->setParameters(	640,480,Umbra::Camera::VIEWFRUSTUM_CULLING);
	_TheCamera->setParameters(	640,480,Umbra::Camera::VIEWFRUSTUM_CULLING |	Umbra::Camera::OCCLUSION_CULLING);

	/*
	** Create the commander
	*/
	_TheCommander = new UmbraCommanderClass;
}

void UmbraSupport::Shutdown(void)
{
	/*
	** Release everything
	*/
	delete _TheCommander;
	_TheCommander = NULL;

	_TheCamera->release();
	_TheCamera = NULL;

	_TheCell->release();
	_TheCell = NULL;

	_DummySphere->release();
	_DummySphere = NULL;

	/*
	** UMBRA shutdown
	*/
	Umbra::Library::exit();
}

void UmbraSupport::Enable_Umbra(bool onoff)
{
	_UmbraEnabled = onoff;
}

bool UmbraSupport::Is_Umbra_Enabled(void)
{
	return _UmbraEnabled;
}

float UmbraSupport::Get_Umbra_Memory_Consumption(void)
{
	return Umbra::Library::getStatistic(Umbra::Library::STAT_MEMORYUSED);
}

Umbra::Model * UmbraSupport::Peek_Dummy_Sphere(void)
{
	return _DummySphere;
}

Umbra::Cell * UmbraSupport::Peek_Umbra_Cell(void)
{
	return _TheCell;
}

Umbra::Camera * UmbraSupport::Peek_Umbra_Camera(void)
{
	return _TheCamera;
}

Umbra::Model * UmbraSupport::Create_Box_Model(const AABoxClass & inputbox)
{
	AABoxClass objbox(inputbox);
	if (objbox.Extent.Length2() < 0.001f) {
		objbox.Extent.Set(1,1,1);
	}

	_init_umbra_box(objbox);
	Umbra::Model * test_model = new Umbra::MeshModel(_umbra_box_verts,_umbra_box_faces,_umbra_box_vert_count,_umbra_box_face_count);
	test_model->autoRelease();
	return test_model;
}

Umbra::Model * UmbraSupport::Create_Mesh_Model(MeshClass & mesh)
{
	MeshModelClass * mdl = mesh.Get_Model();
	Umbra::Model * new_model = NULL;

	if (mdl != NULL) {
		int vcount = mdl->Get_Vertex_Count();
		int fcount = mdl->Get_Polygon_Count();

		if ((vcount > 0) && (fcount > 0)) {

			Umbra::Vector3 * uverts = new Umbra::Vector3[vcount];
			Umbra::Vector3i * ufaces = new Umbra::Vector3i[fcount];
			const Vector3 * wverts = mdl->Get_Vertex_Array();
			const Vector3i * wfaces = mdl->Get_Polygon_Array();

			for (int vi = 0; vi<vcount; vi++) {
				uverts[vi].v[0] = wverts[vi].X;
				uverts[vi].v[1] = wverts[vi].Y;
				uverts[vi].v[2] = wverts[vi].Z;
			}
			for (int fi = 0; fi<fcount; fi++) {
				ufaces[fi].i = wfaces[fi].I;
				ufaces[fi].j = wfaces[fi].J;
				ufaces[fi].k = wfaces[fi].K;
			}

			new_model = new Umbra::MeshModel(uverts,ufaces,vcount,fcount);
			new_model->autoRelease();
		}

		REF_PTR_RELEASE(mdl);
	}

	if (new_model != NULL) {
		return new_model;
	} else {
		return Peek_Dummy_Sphere();
	}
}


void UmbraSupport::Collect_Visible_Objects(const CameraClass & camera,RefPhysListClass & visible_obj_list)
{
	Umbra::Matrix4x4 camtocell;
	Matrix3D camtm;
	camtm = camera.Get_Transform();
	camtm.Rotate_X(DEG_TO_RAD(180.0f));
	_convert_matrix_westwood_to_umbra(camtm,camtocell);

//	_convert_matrix_westwood_to_umbra(camera.Get_Transform(),camtocell);
	_TheCamera->setCameraToCellMatrix(camtocell);

	Umbra::Frustum frustum;
	_convert_frustum_westwood_to_umbra(camera,frustum);
	_TheCamera->setFrustum(frustum);

	_TheCommander->Set_List(visible_obj_list);
	_TheCamera->resolveVisibility(_TheCommander,1,0.0);
}


void UmbraSupport::Update_Umbra_Object(PhysClass * obj)
{
	Umbra::Matrix4x4 objtm;
	_convert_matrix_westwood_to_umbra(obj->Get_Transform(),objtm);
	obj->Peek_Umbra_Object()->setObjectToCellMatrix(objtm);
}

void UmbraSupport::Install_Umbra_Object(PhysClass * obj)
{
	obj->Peek_Umbra_Object()->setCell(_TheCell);
}

void UmbraSupport::Remove_Umbra_Object(PhysClass * obj)
{
	obj->Peek_Umbra_Object()->setCell(NULL);
}

#endif //UMBRASUPPORT
