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
 *                     $Archive:: /Commando/Code/wwphys/pscene_vis.cpp                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/23/01 8:35p                                              $*
 *                                                                                             *
 *                    $Revision:: 47                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PhysicsSceneClass::Release_Vis_Resources -- Releases resources used by the VIS code       *
 *   PhysicsSceneClass::Reset_Vis -- Sets the internal vis reset flag.                         *
 *   PhysicsSceneClass::Validate_Vis -- Clears the internal vis reset flag                     *
 *   PhysicsSceneClass::Internal_Vis_Reset -- Resets the vis system if needed                  *
 *   PhysicsSceneClass::Get_Vis_Camera -- returns a pointer (ref-counted) to the vis camera    *
 *   PhysicsSceneClass::Get_Vis_Gerd -- returns a pointer to the VIS Gerd                      *
 *   PhysicsSceneClass::Show_Vis_Window -- Makes the vis render window visible to the user     *
 *   PhysicsSceneClass::Is_Vis_Window_Visible -- Returns status of the vis render window       *
 *   PhysicsSceneClass::Get_Static_Light_Count -- returns the number of static lights          *
 *   PhysicsSceneClass::Generate_Vis_For_Light -- generate a PVS for the specified light       *
 *   PhysicsSceneClass::Update_Vis -- Performs a vis sample from the given coord system        *
 *   PhysicsSceneClass::Vis_Render_And_Scan -- Renders the scene and scans for visible objects *
 *   PhysicsSceneClass::Vis_Debug_Render -- Renders the same way VIS does                      *
 *   PhysicsSceneClass::Generate_Vis_Statistics_Report -- Stats about the visibility in the le *
 *   PhysicsSceneClass::Optimize_Visibility_Data -- combines and removes redundant vis data    *
 *   PhysicsSceneClass::Merge_Vis_Sector_IDs -- Merges two sector ID's                         *
 *   PhysicsSceneClass::Merge_Vis_Object_IDs -- combines two vis object ID's                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "pscene.h"
#include "staticaabtreecull.h"
#include "dynamicaabtreecull.h"
#include "staticphys.h"
#include "lightcull.h"
#include "camera.h"
#include "vertmaterial.h"
#include "shader.h"
#include "visrendercontext.h" 
#include "visoptimizationcontext.h"
#include "visoptprogress.h"
#include "light.h"
#include "visrasterizer.h"



/************************************************************************************
**
** PhysicsSceneClass Visibility system
**
************************************************************************************/


void PhysicsSceneClass::Enable_Vis(bool onoff)
{
	VisEnabled = onoff;
}

void PhysicsSceneClass::Invert_Vis(bool onoff)
{
	VisInverted = onoff;
}

void PhysicsSceneClass::Set_Vis_Quick_And_Dirty(bool onoff)
{
	VisQuickAndDirty = onoff;
}

void PhysicsSceneClass::Enable_Vis_Sector_Display(bool onoff)
{
	VisSectorDisplayEnabled = onoff;
}

void PhysicsSceneClass::Enable_Vis_Sector_History_Display(bool onoff)
{
	VisSectorHistoryEnabled = onoff;
}

void PhysicsSceneClass::Enable_Vis_Sector_Fallback(bool onoff)
{
	VisSectorFallbackEnabled = onoff;
}

void PhysicsSceneClass::Enable_Backface_Occluder_Debug(bool onoff)
{
	BackfaceDebugEnabled = onoff;
}

void PhysicsSceneClass::Set_Vis_Grid_Display_Mode(int mode)
{
	VisGridDisplayMode = mode;
}

bool PhysicsSceneClass::Is_Vis_Enabled(void)
{
	return VisEnabled;
}

bool PhysicsSceneClass::Is_Vis_Inverted(void)
{
	return VisInverted;
}

bool PhysicsSceneClass::Is_Vis_Quick_And_Dirty(void)
{
	return VisQuickAndDirty;
}

bool PhysicsSceneClass::Is_Vis_Sector_Display_Enabled(void)
{
	return VisSectorDisplayEnabled;
}

bool PhysicsSceneClass::Is_Vis_Sector_History_Display_Enabled(void)
{
	return VisSectorHistoryEnabled;
}

bool PhysicsSceneClass::Is_Vis_Sector_Fallback_Enabled(void)
{
	return VisSectorFallbackEnabled;
}

bool PhysicsSceneClass::Is_Backface_Occluder_Debug_Enabled(void)
{
	return BackfaceDebugEnabled;
}

int PhysicsSceneClass::Get_Vis_Grid_Display_Mode(void)
{
	return VisGridDisplayMode;
}

void PhysicsSceneClass::Vis_Grid_Debug_Reset_Node(void)
{ 
	DynamicObjVisSystem->Debug_Reset_Node(); 
}

bool PhysicsSceneClass::Vis_Grid_Debug_Enter_Parent(void)			
{ 
	return DynamicObjVisSystem->Debug_Enter_Parent(); 
}

bool PhysicsSceneClass::Vis_Grid_Debug_Enter_Sibling(void)			
{ 
	return DynamicObjVisSystem->Debug_Enter_Sibling(); 
}
	
bool PhysicsSceneClass::Vis_Grid_Debug_Enter_Front_Child(void)		
{ 
	return DynamicObjVisSystem->Debug_Enter_Front_Child(); 
}

bool PhysicsSceneClass::Vis_Grid_Debug_Enter_Back_Child(void)		
{ 
	return DynamicObjVisSystem->Debug_Enter_Back_Child(); 
}

bool PhysicsSceneClass::Is_Vis_Sector_Missing(void)
{
	return VisSectorMissing;
}

int PhysicsSceneClass::Allocate_Vis_Object_ID(int count /* = 1*/)
{
	return VisTableManager.Allocate_Vis_Object_ID(count);
}

int PhysicsSceneClass::Allocate_Vis_Sector_ID(int count /* = 1*/)
{
	return VisTableManager.Allocate_Vis_Sector_ID(count);
}

int PhysicsSceneClass::Get_Vis_Table_Size(void)
{
	Internal_Vis_Reset();
	return VisTableManager.Get_Vis_Table_Size();
}

int PhysicsSceneClass::Get_Vis_Table_Count(void)
{
	Internal_Vis_Reset();
	return VisTableManager.Get_Vis_Table_Count();
}

void PhysicsSceneClass::Compute_Vis_Sample_Point(const CameraClass & camera,Vector3 * set_point)
{
	WWASSERT(set_point != NULL);
	if (!VisSamplePointLocked) {
		float nearz,farz;
		camera.Get_Clip_Planes(nearz,farz);
		Matrix3D::Transform_Vector(camera.Get_Transform(),Vector3(0,0,-nearz-0.05f),set_point);
	} else {
		*set_point = LockedVisSamplePoint;
	}
}

VisTableClass * PhysicsSceneClass::Get_Vis_Table(int vis_sector_id)
{
	return VisTableManager.Get_Vis_Table(vis_sector_id);
}

VisTableClass * PhysicsSceneClass::Get_Vis_Table(const Vector3 & point)
{
	int vis_id = StaticCullingSystem->Get_Vis_Sector_ID(point);
	return VisTableManager.Get_Vis_Table(vis_id);
}

VisTableClass * PhysicsSceneClass::Get_Vis_Table(const CameraClass & camera)
{
	Vector3 sample_point;
	Compute_Vis_Sample_Point(camera,&sample_point);
	return Get_Vis_Table(sample_point);
}

VisTableClass * PhysicsSceneClass::Get_Vis_Table_For_Rendering(const CameraClass & camera)
{
	// Decompress the visibility table for the current camera view 
	// Note that if the vis system needs to be reset, we don't want to use any obsolete data
	// Also, if a sample point hasn't been given, we will skip visibility.
	// Also, if we don't find a vis sector, we'll try to use the last valid one that we had.
	int vis_id = -1;
	VisTableClass * pvs = NULL;
	Vector3 vis_sample_point;
	Compute_Vis_Sample_Point(camera,&vis_sample_point);

	if ((!VisResetNeeded) && VisEnabled) {
		
		vis_id = StaticCullingSystem->Get_Vis_Sector_ID(vis_sample_point);

		if (vis_id == -1) {
			VisSectorMissing = true;
			if (VisSectorFallbackEnabled) {
				vis_id = LastValidVisId;
			}
		} else {

			if ((LastValidVisId != vis_id) && VisSectorDisplayEnabled)  {
				[[maybe_unused]] StaticPhysClass * tile = StaticCullingSystem->Find_Vis_Tile(vis_sample_point);
				WWDEBUG_SAY (("Vis Sector: %s\n", tile->Peek_Model ()->Get_Name ()));
			}

			LastValidVisId = vis_id;
			VisSectorMissing = false;
		}
		
		if (vis_id != -1) {
			pvs = VisTableManager.Get_Vis_Table(LastValidVisId);
		}

		if ((VisInverted) && (pvs != NULL)) {
			VisTableClass * new_pvs = NEW_REF(VisTableClass,(*pvs));
			new_pvs->Invert();
			REF_PTR_RELEASE(pvs);
			pvs = new_pvs;
		}
	}

	return pvs;
}


uint32 PhysicsSceneClass::Get_Dynamic_Object_Vis_ID
(
	const AABoxClass & obj_bounds,
	int * node_id
)
{
	return DynamicObjVisSystem->Get_Dynamic_Object_Vis_ID(obj_bounds,node_id);
}

void PhysicsSceneClass::Debug_Display_Dynamic_Vis_Node(int node_id)
{
	if (VisGridDisplayMode == VIS_GRID_DISPLAY_OCCUPIED) {
		AABoxClass bounds;
		DynamicObjVisSystem->Get_Node_Bounds(node_id,&bounds);
		bounds.Extent -= Vector3(5,5,5);
		DEBUG_RENDER_AABOX(bounds,Vector3(0,1,1),0.25f);
	}
}

void PhysicsSceneClass::Lock_Vis_Sample_Point(bool onoff)
{
	VisSamplePointLocked = onoff;
	LockedVisSamplePoint = LastCameraPosition;
}

bool PhysicsSceneClass::Is_Vis_Sample_Point_Locked(void)
{
	return VisSamplePointLocked;
}



/***********************************************************************************************
 * PhysicsSceneClass::Release_Vis_Resources -- Releases resources used by the VIS code         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Release_Vis_Resources(void)
{
	REF_PTR_RELEASE(VisCamera);
}


/***********************************************************************************************
 * PhysicsSceneClass::Reset_Vis -- Sets the internal vis reset flag.                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Reset_Vis(bool doitnow)
{
	/*
	** defer the actual reset until later.
	** the Update_Vis call and the Save_Map methods will check
	** this bit and do the real reset
	*/
	VisResetNeeded = true;

	if (doitnow) {
		Internal_Vis_Reset();
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Validate_Vis -- Clears the internal vis reset flag                       *
 *                                                                                             *
 *    This should not normally be called.  It is needed when the level editor loads a level.   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Validate_Vis(void)
{
	/*
	** Clear the reset flag, this is needed after a level is loaded.
	** Normally as static objects are added to the system, the reset
	** flag will be set, but in the case of loading, we are also 
	** loading the visibility data...
	*/
	VisResetNeeded = false;
}


/***********************************************************************************************
 * PhysicsSceneClass::Internal_Vis_Reset -- Resets the vis system if needed                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Internal_Vis_Reset(void)
{
	if (VisResetNeeded) {

		VisResetNeeded = false;
		WWDEBUG_SAY(("Resetting the visibility system.\r\n"));
		
		/*
		** Throw away all visibility data.
		*/
		VisTableManager.Reset();

		/*
		** Re-partition the static object culling systems for the heck of it.
		** We don't re-partition the dynamic object culling system since it is
		** a massive preprocessing step to actually generate it.
		*/
		StaticCullingSystem->Re_Partition();
		StaticLightingSystem->Re_Partition();

		/*
		** Assign Vis IDs.  Each system may allocate both vis sector ID's and
		** vis object IDs.
		*/
		StaticCullingSystem->Assign_Vis_IDs();
		DynamicObjVisSystem->Assign_Vis_IDs();
		StaticLightingSystem->Assign_Vis_IDs();
		
		/*
		** Force all dynamic objects to recompute their visibility id
		*/
		RefPhysListIterator it(&ObjList);
		while (!it.Is_Done()) {
			DynamicPhysClass * pobj = it.Peek_Obj()->As_DynamicPhysClass();
			if (pobj!=NULL) {
				pobj->Update_Visibility_Status();
			}
			it.Next();
		}
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Vis_Camera -- returns a pointer (ref-counted) to the vis camera      *
 *                                                                                             *
 *    The VIS camera is allocated the first time it is needed.  Normally this is only done     *
 *    as a pre-processing step inside the level editor.                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
CameraClass * PhysicsSceneClass::Get_Vis_Camera(void)
{
	if (VisCamera) {
		VisCamera->Add_Ref();
		return VisCamera;
	}

	VisCamera = new CameraClass;
	VisCamera->Set_Clip_Planes(VIS_NEAR_CLIP,VIS_FAR_CLIP);
	VisCamera->Set_View_Plane(DEG_TO_RAD(90.0f),DEG_TO_RAD(90.0f));
	VisCamera->Set_Viewport(Vector2(0,0),Vector2(1,1));
	
	VisCamera->Add_Ref();
	return VisCamera;
}

/***********************************************************************************************
 * PhysicsSceneClass::Update_Vis -- Performs a vis sample from the given coord system          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
VisSampleClass
PhysicsSceneClass::Update_Vis
(
	const Matrix3D &	camera_tm,
	VisDirBitsType		direction_bits
)
{
	return Update_Vis(camera_tm.Get_Translation(),camera_tm,direction_bits);
}


/***********************************************************************************************
 * PhysicsSceneClass::Update_Vis -- Performs a vis sample from the given coord system          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
VisSampleClass
PhysicsSceneClass::Update_Vis
(
	const Vector3 &	sample_point,
	const Matrix3D &	camera_tm,
	VisDirBitsType		direction_bits,
	CameraClass *		alternate_camera,
	int					user_vis_id
)
{
	/*
	** If the visibility system has been invalidated, reset it now
	*/
	Internal_Vis_Reset();

	/*
	** Look up the vis-sector-id for this vis sample
	*/
	int vis_id = 0;
	if (user_vis_id == -1) {
		vis_id = StaticCullingSystem->Get_Vis_Sector_ID(sample_point);
	} else {
		vis_id = user_vis_id;
	}

	/*
	** Get the existing pvs and make a copy of it that we can 
	** modify with the results of our sampling
	*/
	VisTableClass * original_pvs = VisTableManager.Get_Vis_Table(vis_id,true);

	if (original_pvs == NULL) {
		VisSampleClass vis_sample(camera_tm,direction_bits);
		vis_sample.Init_Error();
		WWDEBUG_SAY(("Vis Sample Rejected - No Vis Sector or Vis Sector ID not assigned!\r\n"));
		return vis_sample;
	}

	VisTableClass * pvs = new VisTableClass(*original_pvs);

	/*
	** Grab the camera, GERD, shader, vertex material...
	*/
	CameraClass * camera = NULL;
	if (alternate_camera) {				// if user is supplying an alternate camera, use it
		camera = alternate_camera;
		camera->Add_Ref();
	} else {									// otherwise use the vis camera
		camera = Get_Vis_Camera();
	}
	WWASSERT (camera != NULL);

	/*
	** Create the render context, initialize the GERD, install the Vis shader and vertex material
	*/
	VisRenderContextClass context(*camera,*pvs);
	context.Set_Resolution(VIS_RENDER_WIDTH,VIS_RENDER_HEIGHT);
	context.Set_Vis_Quick_And_Dirty(VisQuickAndDirty);

	/*
	** Evaluate visiblity for the six views from this point
	*/
	VisSampleClass vis_sample(camera_tm,direction_bits);

	WWDEBUG_SAY(("Generating Vis for sector %d...  ",vis_id));

	for (int i=0; i<VIS_DIRECTIONS; i++) {
		if (vis_sample.Direction_Enabled((VisDirType)i)) {
			if (!vis_sample.Sample_Useless() || (direction_bits & VIS_FORCE_ACCEPT)) {
				vis_sample.Set_Cur_Direction((VisDirType)i);
				context.Camera.Set_Transform(vis_sample.Get_Camera_Transform((VisDirType)i));
				Vis_Render_And_Scan(context,vis_sample);
			}
		}
	}

	/*
	** The static culling system needs to propogate the leaf visibility bits through its
	** hierarchy.
	*/
	StaticCullingSystem->Propogate_Hierarchical_Visibility(pvs);

	/*
	** Record how many bits this sample actually changed
	*/
	vis_sample.Set_Bits_Changed(original_pvs->Count_Differences(*pvs));

	/*
	** Accept the sample if it was not rejected -OR- the user has set the VIS_FORCE_ACCEPT bit
	*/
	bool accept_sample = (!vis_sample.Sample_Rejected()) || (direction_bits & VIS_FORCE_ACCEPT);
	if (accept_sample) {
		VisTableManager.Update_Vis_Table(vis_id,pvs);
		WWDEBUG_SAY(("Done! (%d bits changed) \r\n",vis_sample.Get_Bits_Changed()));
	} else {
		WWDEBUG_SAY(("Rejected!\r\n"));
	}

	/*
	** Release assets
	*/
	REF_PTR_RELEASE(camera);
	REF_PTR_RELEASE(original_pvs);
	REF_PTR_RELEASE(pvs);

	return vis_sample;
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Static_Light_Count -- returns the number of static lights            *
 *                                                                                             *
 *    Each static light allocates a PVS.  This accessor will let you loop over each light      *
 *    and update its PVS.                                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/19/2000  gth : Created.                                                                 *
 *=============================================================================================*/
int PhysicsSceneClass::Get_Static_Light_Count(void)
{
	/*
	** Count the lights in the list
	*/
	int counter = 0;
	RefPhysListIterator light_it = Get_Static_Light_Iterator();
	for (light_it.First();!light_it.Is_Done(); light_it.Next()) {
		counter++;
	}
	
	return counter;
}


/***********************************************************************************************
 * PhysicsSceneClass::Generate_Vis_For_Light -- generate a PVS for the specified light         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/19/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Generate_Vis_For_Light(int light_index)
{
	/*
	** For lights, we're going to use the VIS camera but we're going to tweak the far
	** clip plane to be the outer-radius of the light's falloff sphere.
	*/
	CameraClass * camera = Get_Vis_Camera();

	/*
	** Find the specified light. Array-like access into a list...
	*/
	RefPhysListIterator light_it = Get_Static_Light_Iterator();
	for (; light_index > 0; light_index--) {
		light_it.Next(); 
	}
	
	/*
	** Update vis for the specified light.
	*/
	LightPhysClass * light = light_it.Peek_Obj()->As_LightPhysClass();
	if (	(light != NULL) && 
			(light->Get_Vis_Sector_ID() != 0xFFFFFFFF) &&
			(light->Peek_Model() != NULL) &&
			(light->Peek_Model()->Class_ID() == RenderObjClass::CLASSID_LIGHT)) 
	{
		/*
		** Perform the vis sample
		*/
		LightClass * lightmodel = (LightClass *)light->Peek_Model();
		camera->Set_Clip_Planes(0.01f,lightmodel->Get_Attenuation_Range());
		camera->Set_Transform(light->Get_Transform());

		Update_Vis(	light->Get_Transform().Get_Translation(),
						light->Get_Transform(),
						(VisDirBitsType)(VIS_ALL | VIS_FORCE_ACCEPT),
						camera,
						light->Get_Vis_Sector_ID()	);

	}

	/*
	** Done, reset the camera clip planes, release resources
	*/
	camera->Set_Clip_Planes(VIS_NEAR_CLIP,VIS_FAR_CLIP);
	REF_PTR_RELEASE(camera);
}


/***********************************************************************************************
 * PhysicsSceneClass::Vis_Render_And_Scan -- Renders the scene and scans for visible objects   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Vis_Render_And_Scan(VisRenderContextClass & context,VisSampleClass & vis_sample)
{
	/*
	** Have the static culling system evaluate visibility for the occluders
	** and build the z-buffer in the process.
	*/
	WWASSERT(context.VisRasterizer != NULL);
	context.VisRasterizer->Set_Render_Mode(IDBufferClass::OCCLUDER_MODE);
	StaticCullingSystem->Evaluate_Occluder_Visibility(context,vis_sample);
	
	On_Vis_Occluders_Rendered(context,vis_sample);

	/*
	** Evaluate the visibility of the non-occluders in all systems
	*/
	if (!vis_sample.Sample_Rejected()) {
		context.VisRasterizer->Set_Render_Mode(IDBufferClass::NON_OCCLUDER_MODE);
		StaticCullingSystem->Evaluate_Non_Occluder_Visibility(context);
		DynamicObjVisSystem->Evaluate_Non_Occluder_Visibility(context);
	}

}


/***********************************************************************************************
 * PhysicsSceneClass::Generate_Vis_Statistics_Report -- Stats about the visibility in the leve *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Generate_Vis_Statistics_Report(DynamicVectorClass<VisSectorStatsClass> & report)
{
	/*
	** Loop over each of the vis sectors in the level, adding up how many polygons and 
	** and how much texture memory can be seen from each one.
	*/
	VisSectorStatsClass stats;
	RefPhysListIterator it(&StaticObjList);

	for (it.First(); !it.Is_Done(); it.Next()) {
		StaticPhysClass * obj = it.Peek_Obj()->As_StaticPhysClass();
		if (obj && obj->Is_Vis_Sector()) {
			VisTableClass * vistbl = VisTableManager.Get_Vis_Table(obj->Get_Vis_Sector_ID());
			stats.Compute_Stats(obj,vistbl);
			report.Add(stats);
			REF_PTR_RELEASE(vistbl);
		}
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Optimize_Visibility_Data -- combines and removes redundant vis data      *
 *                                                                                             *
 *    This function does three things.                                                         *
 *    - The dynamic object AABTree removes useless leaf nodes (nodes with visibility identical *
 *      to their parent)                                                                       *
 *    - All vis "objects" are checked and combined if their visibility is nearly the same      *
 *    - All vis "sectors" are checked and combined if their visibility is nearly the same      *
 *                                                                                             *
 * INPUT:                                                                                      *
 * progress_status - class the the physics scene will update as the process proceeds.          *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Optimize_Visibility_Data(VisOptProgressClass & stats)
{

	/*
	** Initialize the progress_status.
	** I'm goint to count one operation per each of the following
	** - each node in the dynamic aabtree for the "Prune_Redundant_Leaf_Nodes" process
	** - each vis object for the "Combine_Redundant_Vis_Objects" process
	** - each vis sector for the "Combine_Redundant_Vis_Sectors" process
	*/
	stats.Reset(DynamicObjVisSystem->Partition_Node_Count() + Get_Vis_Table_Size() + Get_Vis_Table_Count());
	stats.Set_Initial_Bit_Count(VisTableManager.Get_Vis_Table_Size() * VisTableManager.Get_Vis_Table_Count());
	stats.Set_Initial_Sector_Count(Get_Vis_Table_Count());
	stats.Set_Initial_Object_Count(Get_Vis_Table_Size());
	stats.Set_Initial_Dynamic_Cell_Count(DynamicObjVisSystem->Partition_Node_Count());

	/*
	** Create a VisOptimizationContextClass and optimize the vis data
	*/
	VisOptimizationContextClass optcontext(this,stats);
	optcontext.Optimize(&VisTableManager,DynamicObjVisSystem);

	/*
	** Reset all of dynamic objects' cached vis-ID and cull node.
	*/
	RefPhysListIterator it(&ObjList);

	for (it.First(); !it.Is_Done(); it.Next()) {
		DynamicPhysClass * obj = it.Peek_Obj()->As_DynamicPhysClass();
		if (obj != NULL) {
			obj->Update_Visibility_Status();
		}
	}

	/*
	** Finish the statistics
	*/
	stats.Set_Final_Bit_Count(VisTableManager.Get_Vis_Table_Size() * VisTableManager.Get_Vis_Table_Count());
	stats.Set_Final_Sector_Count(Get_Vis_Table_Count());
	stats.Set_Final_Object_Count(Get_Vis_Table_Size());
	stats.Set_Final_Dynamic_Cell_Count(DynamicObjVisSystem->Partition_Node_Count());
}


/***********************************************************************************************
 * PhysicsSceneClass::Merge_Vis_Sector_IDs -- Merges two sector ID's                           *
 *                                                                                             *
 *    The greater ID will be set equal to the lesser ID.  This should only be called by the    *
 *    vis optimization process and relies on a VisOptimizationContext to track the changes     *
 *    to the actual VIS bit vectors.                                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Merge_Vis_Sector_IDs(uint32 id0,uint32 id1)
{
	StaticCullingSystem->Merge_Vis_Sector_IDs(id0,id1);
	StaticLightingSystem->Merge_Vis_Sector_IDs(id0,id1);
}


/***********************************************************************************************
 * PhysicsSceneClass::Merge_Vis_Object_IDs -- combines two vis object ID's                     *
 *                                                                                             *
 *    The greater ID will be set equal to the lesser ID.  This should only be called by the    *
 *    vis optimization process and relies on a VisOptimizationContext to track the changes     *
 *    to the actual VIS bit vectors.                                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Merge_Vis_Object_IDs(uint32 id0,uint32 id1)
{
	StaticCullingSystem->Merge_Vis_Object_IDs(id0,id1);
	DynamicObjVisSystem->Merge_Vis_Object_IDs(id0,id1);
}
