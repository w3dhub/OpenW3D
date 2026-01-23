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
 *                     $Archive:: /Commando/Code/wwphys/pscene.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/28/02 6:48p                                               $*
 *                                                                                             *
 *                    $Revision:: 218                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PhysicsSceneClass::PhysicsSceneClass -- Constructor                                       *
 *   PhysicsSceneClass::~PhysicsSceneClass -- Destructor                                       *
 *   PhysicsSceneClass::Update -- Simulates the entire scene forward one timestep              *
 *   PhysicsSceneClass::Add_Dynamic_Object -- Adds a dynamic object to the scene               *
 *   PhysicsSceneClass::Internal_Add_Dynamic_Object -- internal function finishes adding a dyn *
 *   PhysicsSceneClass::Add_Static_Object -- Adds a static object to the scene                 *
 *   PhysicsSceneClass::Internal_Add_Static_Object -- internal add static object...            *
 *   PhysicsSceneClass::Add_Static_Light -- Adds a static light to the system                  *
 *   PhysicsSceneClass::Internal_Add_Static_Light -- internal add static light...              *
 *   PhysicsSceneClass::Delayed_Remove_Object -- registers an object for removal               *
 *   PhysicsSceneClass::Process_Release_List -- Removes all objects in the ReleaseList         *
 *   PhysicsSceneClass::Remove_Object -- Remove an object from the scene                       *
 *   PhysicsSceneClass::Remove_All -- Removes all objects from the scene                       *
 *   PhysicsSceneClass::Contains -- Tests if the scene contains the given object               *
 *   PhysicsSceneClass::Get_Dynamic_Object_Iterator -- returns an iterator for the dynamic obj *
 *   PhysicsSceneClass::Get_Static_Object_Iterator -- return an iterator for the static objs   *
 *   PhysicsSceneClass::Get_Static_Anim_Object_Iterator -- returns an iterator for static anim *
 *   PhysicsSceneClass::Get_Static_Light_Iterator -- returns an iterator for static lights     *
 *   PhysicsSceneClass::Get_Static_Projector_Iterator -- returns an iterator for static projec *
 *   PhysicsSceneClass::Get_Dynamic_Projector_Iterator -- returns an iterator for dynamic proj *
 *   PhysicsSceneClass::Add_To_Dirty_Cull_List -- adds something to the "dirty cull" list      *
 *   PhysicsSceneClass::Remove_From_Dirty_Cull_List -- removes an object from the dirty cull l *
 *   PhysicsSceneClass::Is_In_Dirty_Cull_List -- tests whether an object is in the dirty cull  *
 *   PhysicsSceneClass::Add_Render_Object -- Adds a render object to the scene                 *
 *   PhysicsSceneClass::Remove_Render_Object -- removes a render object from the scene         *
 *   PhysicsSceneClass::Register -- rengisters given render object                             *
 *   PhysicsSceneClass::Unregister -- Unregisters the given render object                      *
 *   PhysicsSceneClass::Set_Vis_Sample_Point -- Set the current vis sample point               *
 *   PhysicsSceneClass::Pre_Render_Processing -- processing which occurs prior to rendering    *
 *   PhysicsSceneClass::Post_Render_Processing -- processing that occurs after rendering       *
 *   PhysicsSceneClass::Optimize_LODs -- Set the LOD level for each object                     *
 *   PhysicsSceneClass::Render -- Render the scene                                             *
 *   PhysicsSceneClass::Render_Objects -- Render the visible objects                           *
 *   PhysicsSceneClass::Render_Object -- Render an individual object                           *
 *   PhysicsSceneClass::Render_Backface_Occluders -- Render backfaces of all occluders         *
 *   PhysicsSceneClass::Re_Partition_Static_Objects -- partition the static objects            *
 *   PhysicsSceneClass::Re_Partition_Static_Lights -- partition the static lights              *
 *   PhysicsSceneClass::Re_Partition_Dynamic_Culling_System -- partition the dynamic culling s *
 *   PhysicsSceneClass::Re_Partition_Static_Projectors -- partition the static projectors      *
 *   PhysicsSceneClass::Update_Culling_System_Bounding_Boxes -- updates the cull systems       *
 *   PhysicsSceneClass::Get_Level_Extents -- returns the bounds of the level                   *
 *   PhysicsSceneClass::Set_Polygon_Budgets -- set the budgets for the LOD system              *
 *   PhysicsSceneClass::Get_Polygon_Budgets -- returns the budgets for the LOD system          *
 *   PhysicsSceneClass::Per_Frame_Statistics_Update -- statistics tracking                     *
 *   PhysicsSceneClass::Get_Statistics -- returns statistics                                   *
 *   PhysicsSceneClass::Find_Static_Object -- Find static object with the given ID             *
 *   PhysicsSceneClass::Shatter_Mesh -- Clips a mesh into pieces and scatters them             *
 *   PhysicsSceneClass::Verify_Culling_Systems -- Verify the culling systems                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "pscene.h"
#include "rinfo.h"
#include "light.h"
#include "wwdebug.h"
#include "wwhack.h"
#include "wwprofile.h"
#include "wwmemlog.h"
#include "physgridcull.h"
#include "staticaabtreecull.h"
#include "dynamicaabtreecull.h"
#include "lightcull.h"
#include "camera.h"
#include "wwphystrig.h"
#include "vertmaterial.h"
#include "predlod.h"
#include "widgets.h"
#include "renderobjphys.h"
#include "Pathfind.h"
#include "lightexclude.h"
#include "physdecalsys.h"
#include "quat.h"
#include "shader.h"
#include "lookuptable.h"
#include "shattersystem.h"
#include "projectile.h"
#include "staticphys.h"
#include "vistable.h"
#include "meshmdl.h"
#include "camerashakesystem.h"
#include "lightenvironment.h"
#include "dx8wrapper.h"
#include "physresourcemgr.h"
#include "phys3.h"

#include "umbrasupport.h"
#include <algorithm>


#define STATISTICS_FRAMES  20					// number of frames to average statistics across
#define SHATTER_DEBUG		0					// debugging, freeze shattered particles in place for 60sec

/*
** Static members of PhysicsSceneClass 
*/
bool						PhysicsSceneClass::AllowCollisionFlags[NUM_COLLISION_FLAGS];
PhysicsSceneClass *	PhysicsSceneClass::TheScene = NULL;

/*
** Constants
*/
const float				MAX_TIMESTEP = 1.0f / 15.0f;

const Vector3			MIN_GRID_CELL_SIZE(15.0f,15.0f,15.0f);
const int				MAX_GRID_CELL_COUNT = 8192;
const float				MAX_DYNAMIC_OBJ_RADIUS = 5.0f;

const int				DEFAULT_DYNAMIC_LOD_BUDGET = 4000;
const int				DEFAULT_STATIC_LOD_BUDGET = 4000;


/******************************************************************************************
**
**
** PhysicsSceneClass Implementation
**
**
******************************************************************************************/


/***********************************************************************************************
 * PhysicsSceneClass::PhysicsSceneClass -- Constructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
PhysicsSceneClass::PhysicsSceneClass(void) :
	FrameNum(0),
	LastCameraPosition(0,0,0),
	LastValidVisId(-1),
	DebugDisplayEnabled(false),
	ProjectorDebugDisplayEnabled(false),
	DirtyCullDebugDisplayEnabled(false),
	LightingDebugDisplayEnabled(false),
	StaticCullingSystem(NULL),
	DynamicCullingSystem(NULL),
	StaticLightingSystem(NULL),
	StaticProjectorCullingSystem(NULL),
	DynamicProjectorCullingSystem(NULL),
	DynamicPolyBudget(DEFAULT_DYNAMIC_LOD_BUDGET),
	StaticPolyBudget(DEFAULT_STATIC_LOD_BUDGET),
	LightingMode(LIGHTING_MODE_CHEAP),
	SceneAmbientLight(0.5f,0.5f,0.5f),
	UseSun(false),
	SunLight(NULL),
	VisEnabled(true),
	VisInverted(false),
	VisQuickAndDirty(false),
	VisResetNeeded(false),
	VisSectorDisplayEnabled(false),
	VisSectorHistoryEnabled(false),
	VisGridDisplayMode(VIS_GRID_DISPLAY_NONE),
	VisSectorMissing(false),
	VisSectorFallbackEnabled(true),
	BackfaceDebugEnabled(false),
	VisSamplePointLocked(false),
	LockedVisSamplePoint(0,0,0),
	VisCamera(NULL),
	CurrentVisTable(NULL),
	StaticProjectorsEnabled(false),
	DynamicProjectorsEnabled(false), 
	ShadowMode(SHADOW_MODE_NONE),
	ShadowAttenStart(25.0f),
	ShadowAttenEnd(40.0f),
	ShadowNormalIntensity(0.45f),
	ShadowBlobTexture(NULL),
	ShadowCamera(NULL),
	ShadowRenderContext(NULL),
	ShadowMaterialPass(NULL),
	ShadowResWidth(128),
	ShadowResHeight(128),
	DecalSystem(NULL),
	Pathfinder(NULL),
	CameraShakeSystem(NULL),
	HighlightMaterialPass(NULL),
	UpdateOnlyVisibleObjects(false),
	CurrentFrameNumber(0)
{
	WWASSERT_PRINT(TheScene == NULL,"Only one instance of the PhysicsSceneClass is allowed.\r\n");
	WWMEMLOG(MEM_PHYSICSDATA);
	TheScene = this;
	
	/*
	** Initialize Umbra
	*/
#if (UMBRASUPPORT)
	UmbraSupport::Init();
#endif

	/*
	** Clear the collision flags
	*/
	memset(AllowCollisionFlags,0,NUM_COLLISION_FLAGS);

	/*
	** Allocate culling systems
	*/
	StaticCullingSystem = new StaticAABTreeCullClass(this);
	DynamicCullingSystem = new PhysGridCullClass(this);
	DynamicObjVisSystem = new DynamicAABTreeCullClass(this);
	StaticLightingSystem = new StaticLightCullClass;
	StaticProjectorCullingSystem = new TypedAABTreeCullSystemClass<TexProjectClass>;
	DynamicProjectorCullingSystem = new TypedGridCullSystemClass<TexProjectClass>;

	/*
	** Allocate pathfind object
	*/
	Pathfinder = new PathfindClass;

	/*
	** Allocate the camera shake system
	*/
	CameraShakeSystem = new CameraShakeSystemClass;

	/*
	** Allocate the sun light
	*/
	SunLight = NEW_REF(LightClass,(LightClass::DIRECTIONAL));
	Reset_Sun_Light();

	/*
	** Initialize the debug code
	*/
	WidgetSystem::Init_Debug_Widgets();
	
	/*
	** Allocate decal resources
	*/
	Allocate_Decal_Resources();

}


/***********************************************************************************************
 * PhysicsSceneClass::~PhysicsSceneClass -- Destructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
PhysicsSceneClass::~PhysicsSceneClass(void)
{
	Remove_All();

	delete StaticCullingSystem;
	delete DynamicCullingSystem;
	delete DynamicObjVisSystem;
	delete StaticLightingSystem;	
	delete StaticProjectorCullingSystem;
	delete DynamicProjectorCullingSystem;
	delete Pathfinder;
	delete CameraShakeSystem;

	REF_PTR_RELEASE(SunLight);

	Release_Vis_Resources();
	Release_Projector_Resources();
	Release_Decal_Resources();
	WidgetSystem::Release_Debug_Widgets();

	WWASSERT(TheScene == this);
	TheScene = NULL;	

	/*
	** Shutdown UMBRA
	*/
#if (UMBRASUPPORT)
	UmbraSupport::Shutdown();
#endif
}	


/***********************************************************************************************
 * PhysicsSceneClass::Update -- Simulates the entire scene forward one timestep                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Update(float dt,int frameid)
{
	WWPROFILE("PhysicsScene::Update");

	Generate_Static_Shadow_Projectors();

	FrameNum = frameid;

	if (dt == 0.0f) {
		return;
	}

	/*
	** Timestep all of the physics objects
	*/
	{
		WWPROFILE("Timestep");
		float remaining = dt;
		
		while (remaining > 0) {
			
			float step = std::min(remaining,MAX_TIMESTEP);

			/*
			** Loop through each object telling each to time-step itself.
			*/
			RefPhysListIterator it(&TimestepList);
			for (it.First(); !it.Is_Done(); it.Next()) {
				PhysClass* phys_obj=it.Peek_Obj();
				// Little optimization hack - only update vehicles that are visible (for now update all other physics
				// objects regardless of the visibility to avoid problems, vehicles are the most expensive anyway).
				// This same thing is done to Post Timestep couple lines lower.
				if (phys_obj->Is_Object_Simulating()) {
					if (!UpdateOnlyVisibleObjects	||
						phys_obj->Get_Last_Visible_Frame()==CurrentFrameNumber ||
						!phys_obj->As_VehiclePhysClass()) {
						phys_obj->Timestep(step);
					}
				}
			}

			remaining -= step;
		}
	}

	{ 
		WWPROFILE("Post Timestep");
		RefPhysListIterator it(&TimestepList);
		for (it.First(); !it.Is_Done(); it.Next()) {
//			if (it.Peek_Obj()->Is_Object_Simulating()) {
//				if (!UpdateOnlyVisibleObjects || it.Peek_Obj()->Get_Last_Visible_Frame()==CurrentFrameNumber) {
			PhysClass* phys_obj=it.Peek_Obj();
			if (phys_obj->Is_Object_Simulating()) {
				if (!UpdateOnlyVisibleObjects	||
					phys_obj->Get_Last_Visible_Frame()==CurrentFrameNumber ||
					!phys_obj->As_VehiclePhysClass()) {
					phys_obj->Post_Timestep_Process();
				}
			}
		}
	}

	/*
	** Timestep the camera shakers
	*/
	{
		WWPROFILE("CameraShakeSystem");
		CameraShakeSystem->Timestep(dt);
	}

	/*
	** Timestep the material effects
	*/
	{
		WWPROFILE("MaterialEffects");
		MaterialEffectClass::Timestep_All_Effects(dt);
	}

	/*
	** Process pending release requests
	** Put here prior to rendering to insure items won't draw too many times.
	*/
	Process_Release_List();
	CurrentFrameNumber++;
}


/***********************************************************************************************
 * PhysicsSceneClass::Add_Dynamic_Object -- Adds a dynamic object to the scene                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Add_Dynamic_Object(PhysClass * newobj)
{
	WWASSERT(newobj != NULL);
	WWASSERT(newobj->Peek_Model() != NULL);
	WWASSERT(newobj->Get_Culling_System() == NULL);

	// Add the object to the dynamic culling system
	DynamicCullingSystem->Add_Object(newobj);

	// Add the object to the lists in the physics scene
	Internal_Add_Dynamic_Object(newobj);

	// Clean up any cached visibility data that may have been in the object
	DynamicPhysClass * dynobj = newobj->As_DynamicPhysClass();
	WWASSERT(dynobj != NULL);	
	if (dynobj != NULL) {
		dynobj->Update_Visibility_Status();
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Internal_Add_Dynamic_Object -- internal function finishes adding a dynam *
 *                                                                                             *
 *    This function is called both by the loading code and the Add_Dynamic_Object function.    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Internal_Add_Dynamic_Object(PhysClass * newobj)
{
	// Add the object to the appropriate lists
	ObjList.Add(newobj);
	SceneClass::Add_Render_Object(newobj->Peek_Model());
	if (newobj->Needs_Timestep()) {
		TimestepList.Add(newobj);
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Add_Static_Object -- Adds a static object to the scene                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Add_Static_Object(StaticPhysClass * newtile,int cull_node_id/*=-1*/)
{
	WWASSERT(newtile != NULL);
	WWASSERT(newtile->Peek_Model() != NULL);
	WWASSERT(newtile->Get_Culling_System() == NULL);

	// Add the object to the static culling system
	StaticCullingSystem->Add_Object(newtile,cull_node_id);

	// Add the object to the lists in the physics scene
	Internal_Add_Static_Object(newtile);
}


/***********************************************************************************************
 * PhysicsSceneClass::Internal_Add_Static_Object -- internal add static object...              *
 *                                                                                             *
 *    This function is called by both the loading code and the Add_Static_Object function      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Internal_Add_Static_Object(StaticPhysClass * newtile)
{
	// Add the object to the appropriate lists
	StaticObjList.Add(newtile);
	SceneClass::Add_Render_Object(newtile->Peek_Model());
	if (newtile->Needs_Timestep()) {
		TimestepList.Add(newtile);
	}
	if (newtile->As_StaticAnimPhysClass() != NULL) {
		StaticAnimList.Add(newtile);
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Add_Static_Light -- Adds a static light to the system                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Add_Static_Light(LightPhysClass * newlight,int /* cull_node_id */)
{
	WWASSERT(newlight != NULL);
	WWASSERT(newlight->Peek_Model() != NULL);
	WWASSERT(newlight->Get_Culling_System() == NULL);

	// Add the light to the static light culling system
	StaticLightingSystem->Add_Object(newlight);

	// Add the light to all of the appropriate lists
	Internal_Add_Static_Light(newlight);
}


/***********************************************************************************************
 * PhysicsSceneClass::Internal_Add_Static_Light -- internal add static light...                *
 *                                                                                             *
 *    This function is called by both the loading code and the Add_Static_Light function       *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Internal_Add_Static_Light(LightPhysClass * newlight)
{	
	// Add the object to the appropriate lists
	StaticLightList.Add(newlight);
	
	//(gth) hack, I don't want static lights to get registered as vertex processors.
	//so don't let the scene class know about them ;-)  I should probably come up with
	//a better mechanism for this...
	//SceneClass::Add_Render_Object(newlight->Peek_Model());
	
	if (newlight->Needs_Timestep()) {
		TimestepList.Add(newlight);
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Delayed_Remove_Object -- registers an object for removal                 *
 *                                                                                             *
 *    The object will be released by the scene at the end of the next frame.                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Delayed_Remove_Object(PhysClass * obj)
{
	if (!ReleaseList.Contains(obj)) {
		ReleaseList.Add(obj);
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Process_Release_List -- Removes all objects in the ReleaseList           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Process_Release_List(void)
{
	while(!ReleaseList.Is_Empty()) {
		PhysClass * obj = ReleaseList.Remove_Head();
		Remove_Object(obj);
		obj->Release_Ref();
	}	
}


/***********************************************************************************************
 * PhysicsSceneClass::Remove_Object -- Remove an object from the scene                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Remove_Object(PhysClass * obj)
{
	// NOTE: sometimes the game tries to 'remove' objects that havent been added...
	if (!Contains(obj)) return;

	// Assert that the object is valid
	WWASSERT(obj != NULL);
	WWASSERT(obj->Peek_Model() != NULL);

	// Notify the model that it is being removed from the scene
	SceneClass::Remove_Render_Object(obj->Peek_Model());

	// Notify the observer (if it has one)
	if (obj->Get_Observer() != NULL) {
		obj->Get_Observer()->Object_Removed_From_Scene(obj);
	}

	// Pull the object out of any of the "extra-processing" lists
	Remove_From_Dirty_Cull_List(obj);
	TimestepList.Remove(obj);
	StaticAnimList.Remove(obj);

	// Pull the physics object out of whatever system it is in
	CullSystemClass * cullsys = obj->Get_Culling_System();

	if (cullsys == DynamicCullingSystem) {

		DynamicCullingSystem->Remove_Object(obj);
		ObjList.Remove(obj);
	
	} else if (cullsys == StaticCullingSystem) {

		WWASSERT(obj->As_StaticPhysClass() != NULL);
		StaticCullingSystem->Remove_Object(obj->As_StaticPhysClass());
		StaticObjList.Remove(obj);

	} else if (cullsys == StaticLightingSystem) {

		WWASSERT(obj->As_LightPhysClass() != NULL);
		StaticLightingSystem->Remove_Object(obj->As_LightPhysClass());
		StaticLightList.Remove(obj);
	
	} else {
		
		WWASSERT(0); // should never happen!

	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Remove_All -- Removes all objects from the scene                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Remove_All(void)
{
	PhysClass * obj = ObjList.Peek_Head();
	while (obj) {
		Remove_Object(obj);
		obj = ObjList.Peek_Head();
	}

	StaticPhysClass * tile = (StaticPhysClass *)StaticObjList.Peek_Head();
	while (tile) {
		Remove_Object(tile);
		tile = (StaticPhysClass *)StaticObjList.Peek_Head();
	}

	LightPhysClass * light = (LightPhysClass *)StaticLightList.Peek_Head();
	while(light) {
		Remove_Object(light);
		light = (LightPhysClass *)StaticLightList.Peek_Head();
	}

	TexProjectClass * static_proj = StaticProjectorList.Peek_Head();
	while(static_proj) {
		Remove_Static_Texture_Projector(static_proj);
		static_proj = StaticProjectorList.Peek_Head();
	}

	TexProjectClass * dynamic_proj = DynamicProjectorList.Peek_Head();
	while(dynamic_proj) {
		Remove_Dynamic_Texture_Projector(dynamic_proj);
		dynamic_proj = DynamicProjectorList.Peek_Head();
	}

	Pathfinder->Reset_Sectors ();
}


/***********************************************************************************************
 * PhysicsSceneClass::Contains -- Tests if the scene contains the given object                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool PhysicsSceneClass::Contains(PhysClass * obj)
{
	if (ObjList.Is_In_List(obj)) return true;
	if (StaticObjList.Is_In_List(obj)) return true;
	if (StaticLightList.Is_In_List(obj)) return true;
	return false;
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Dynamic_Object_Iterator -- returns an iterator for the dynamic objec *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/2000 gth : Created.                                                                 *
 *=============================================================================================*/
RefPhysListIterator PhysicsSceneClass::Get_Dynamic_Object_Iterator(void)		
{ 
	return RefPhysListIterator(&ObjList); 
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Static_Object_Iterator -- return an iterator for the static objs     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/2000 gth : Created.                                                                 *
 *=============================================================================================*/
RefPhysListIterator PhysicsSceneClass::Get_Static_Object_Iterator(void)			
{ 
	return RefPhysListIterator(&StaticObjList); 
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Static_Anim_Object_Iterator -- returns an iterator for static anim o *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/2000 gth : Created.                                                                 *
 *=============================================================================================*/
RefPhysListIterator PhysicsSceneClass::Get_Static_Anim_Object_Iterator(void)	
{ 
	return RefPhysListIterator(&StaticAnimList); 
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Static_Light_Iterator -- returns an iterator for static lights       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/2000 gth : Created.                                                                 *
 *=============================================================================================*/
RefPhysListIterator PhysicsSceneClass::Get_Static_Light_Iterator(void)			
{ 
	return RefPhysListIterator(&StaticLightList); 
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Static_Projector_Iterator -- returns an iterator for static projecto *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/2000 gth : Created.                                                                 *
 *=============================================================================================*/
TexProjListIterator PhysicsSceneClass::Get_Static_Projector_Iterator(void)		
{ 
	return TexProjListIterator(&StaticProjectorList); 
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Dynamic_Projector_Iterator -- returns an iterator for dynamic projec *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/2000 gth : Created.                                                                 *
 *=============================================================================================*/
TexProjListIterator PhysicsSceneClass::Get_Dynamic_Projector_Iterator(void)	
{ 
	return TexProjListIterator(&DynamicProjectorList); 
}


/***********************************************************************************************
 * PhysicsSceneClass::Add_To_Dirty_Cull_List -- adds something to the "dirty cull" list        *
 *                                                                                             *
 *    objects in the dirty cull list have their visiblity and cull box re-computed each        *
 *    frame.  We should try to keep things like this to a minimum.  Particle buffers are       *
 *    an example of something that would end up in the "dirty cull" list                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Add_To_Dirty_Cull_List(PhysClass *obj)
{ 
	DirtyCullList.Add (obj); 
}


/***********************************************************************************************
 * PhysicsSceneClass::Remove_From_Dirty_Cull_List -- removes an object from the dirty cull lis *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Remove_From_Dirty_Cull_List(PhysClass *obj)			
{ 
	DirtyCullList.Remove (obj); 
}


/***********************************************************************************************
 * PhysicsSceneClass::Is_In_Dirty_Cull_List -- tests whether an object is in the dirty cull li *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool PhysicsSceneClass::Is_In_Dirty_Cull_List(PhysClass *obj)					
{ 
	return DirtyCullList.Contains (obj); 
}

/***********************************************************************************************
 * PhysicsSceneClass::Add_Render_Object -- Adds a render object to the scene                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This interface is less efficient than wrapping your render objects in some kind of physics  *
 * object.                                                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Add_Render_Object(RenderObjClass * obj)
{
	WWMEMLOG(MEM_PHYSICSDATA);	// Right category?

	// NOTE: The *ONLY* way this code should get activated is when the user
	// or some deep dark w3d code is directly adding and removing render objects
	// in the physics scene.  In this case, I wrap the render objects with 
	// RenderObjPhysClass's and set the UserData pointer to point back to this
	// wrapper.
	WWASSERT(obj != NULL);
	RenderObjPhysClass * cullnode = NEW_REF(RenderObjPhysClass,());
	cullnode->Set_Model(obj);
	Add_Dynamic_Object(cullnode);
	Add_To_Dirty_Cull_List(cullnode);
	
	//
	//	Make sure we don't save particle buffers or lines
	//
	if ((obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEBUFFER) ||
		(obj->Class_ID () == RenderObjClass::CLASSID_SEGLINE))
	{
		cullnode->Enable_Dont_Save(true);
	}

	cullnode->Release_Ref();

	SceneClass::Add_Render_Object(obj);
}


/***********************************************************************************************
 * PhysicsSceneClass::Remove_Render_Object -- removes a render object from the scene           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Remove_Render_Object(RenderObjClass * obj)
{
	// NOTE: The *ONLY* way this code should get activated is when the user
	// or some deep dark w3d code is directly adding and removing render objects
	// in the physics scene.  In this case, I wrap the render objects with 
	// RenderObjPhysClass's and set the UserData pointer to point back to this
	// wrapper.
	SceneClass::Remove_Render_Object(obj);

	WWASSERT(obj != NULL);
	PhysClass * cullnode = (PhysClass *)obj->Get_User_Data();

	WWASSERT(cullnode != NULL);
	WWASSERT(cullnode->As_RenderObjPhysClass() != NULL);
	Remove_Object(cullnode);
}


/***********************************************************************************************
 * PhysicsSceneClass::Register -- rengisters given render object                               *
 *                                                                                             *
 *    Render objects can be registered for On_Frame_Update calls, added to the vertex          *
 *    processor list, or added to the release list.                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Register(RenderObjClass * obj,RegType for_what)
{
	WWASSERT(obj != NULL);
	switch (for_what)
	{
		case ON_FRAME_UPDATE: 
			UpdateList.Add(obj); 
			break;
		case LIGHT: 
			VertexProcList.Add(obj); 
			break;
		case RELEASE: 
			{
				if (obj->Get_Container() != NULL) {
					obj->Get_Container()->Remove_Sub_Object(obj);
				} else {
					PhysClass * wrapper = (PhysClass *)obj->Get_User_Data();
					
					/*
					** If there is no wrapper, the object isn't actually in the scene so do nothing
					*/
					if (wrapper != NULL) {
						Delayed_Remove_Object(wrapper);
					}
				}
			}
			break;
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Unregister -- Unregisters the given render object                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Unregister(RenderObjClass * obj,RegType for_what)
{
	WWASSERT(obj != NULL);
	switch (for_what)
	{
		case ON_FRAME_UPDATE: 
			UpdateList.Remove(obj); 
			break;
		case LIGHT: 
			VertexProcList.Remove(obj); 
			break;
		case RELEASE: 
			WWASSERT_PRINT(0,("Error! Object %s tried to un-register from the release list\r\n",obj->Get_Name()));
			break;
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Pre_Render_Processing -- processing which occurs prior to rendering      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Pre_Render_Processing(CameraClass & camera)
{
	WWPROFILE("Pre_Render_Processing");

	LastCameraPosition = camera.Get_Position();

	DecalSystem->Update_Decal_Fade_Distances(camera);

	// Do the needed 'On_Frame_Update's
	RefRenderObjListIterator rit(&UpdateList);
	for (rit.First(); !rit.Is_Done(); rit.Next()) {
		rit.Peek_Obj()->On_Frame_Update();
	}	

	// Update culling info for all of the objects in the "dirty cull" list (these are
	// objects which were added to the scene as pure render objects so I don't assume
	// that the I have control over when their transform or bounding box is changed...)  
	RefPhysListIterator it(&DirtyCullList);
	for (it.First(); !it.Is_Done(); it.Next()) {
		PhysClass * obj = it.Peek_Obj();
		obj->Update_Cull_Box();
		if (obj->As_DynamicPhysClass()) {
			obj->As_DynamicPhysClass()->Update_Visibility_Status();
		}
		if (DirtyCullDebugDisplayEnabled) {
			const AABoxClass & box = obj->Get_Cull_Box();
			if (box.Extent.Length2() > 0.0f) {
				DEBUG_RENDER_AABOX(obj->Get_Cull_Box(),Vector3(1,1,1),0.25f);
			}
		}
	}

	// Get the pvs
	VisTableClass * pvs = Get_Vis_Table_For_Rendering(camera);

	// Collect the visible objects
	bool use_umbra = false;
#if (UMBRASUPPORT) 
	use_umbra = UmbraSupport::Is_Umbra_Enabled();
#endif

	if (!use_umbra) {
		// Get the lists of visible objects
		{ 
			WWPROFILE( "Collect Static Objs" );
			StaticCullingSystem->Collect_Visible_Objects(camera.Get_Frustum(),pvs,VisibleStaticObjectList,VisibleWSMeshList);
		}

		// Collect list of the visible dynamic objects
		{
			WWPROFILE( "Collect Dynamic Objs" );
			DynamicCullingSystem->Collect_Visible_Objects(camera.Get_Frustum(),pvs,VisibleDynamicObjectList);
		}

		// LOD processing 
		Optimize_LODs(camera,&VisibleDynamicObjectList,&VisibleStaticObjectList,&VisibleWSMeshList);

		// Texture projectors
		Apply_Projectors(camera);

	} else {
#if (UMBRASUPPORT)
		RefPhysListClass vis_obj_list;
		{
			WWPROFILE("Umbra!");
			UmbraSupport::Collect_Visible_Objects(camera,VisibleDynamicObjectList);
		}
#endif
	}

	// release the visibility table
	REF_PTR_RELEASE(pvs);
}

/***********************************************************************************************
 * PhysicsSceneClass::Post_Render_Processing -- processing that occurs after rendering         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Post_Render_Processing(void)
{
	WWPROFILE("Post_Render_Processing");

	// Release the visible object lists, release the shadow textures
	VisibleDynamicObjectList.Reset_List();
	VisibleStaticObjectList.Reset_List();
	VisibleWSMeshList.Reset_List();
	

	// Update statistics
	Per_Frame_Statistics_Update();
}


/***********************************************************************************************
 * PhysicsSceneClass::Optimize_LODs -- Set the LOD level for each object                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Optimize_LODs
(
	CameraClass & camera,
	RefPhysListClass * dyn_objs,
	RefPhysListClass * static_objs,
	RefPhysListClass * static_ws_meshes
)
{
	WWPROFILE("Optimize_LODs");

	// process the dynamic objects
	PredictiveLODOptimizerClass::Clear();
	RefPhysListIterator it(dyn_objs);
	for (it.First(); !it.Is_Done(); it.Next()) {
		it.Peek_Obj()->Peek_Model()->Prepare_LOD(camera);
		it.Peek_Obj()->Set_Last_Visible_Frame(CurrentFrameNumber);
	}
	PredictiveLODOptimizerClass::Optimize_LODs(DynamicPolyBudget);

	// process the static objects
	PredictiveLODOptimizerClass::Clear();
	for (it.First(static_objs); !it.Is_Done(); it.Next()) {
		it.Peek_Obj()->Peek_Model()->Prepare_LOD(camera);
		it.Peek_Obj()->Set_Last_Visible_Frame(CurrentFrameNumber);
	}
	for (it.First(static_ws_meshes); !it.Is_Done(); it.Next()) {
		it.Peek_Obj()->Peek_Model()->Prepare_LOD(camera);
		it.Peek_Obj()->Set_Last_Visible_Frame(CurrentFrameNumber);
	}
	PredictiveLODOptimizerClass::Optimize_LODs(StaticPolyBudget);
}



/***********************************************************************************************
 * PhysicsSceneClass::Render -- Render the scene                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Customized_Render(RenderInfoClass & rinfo)
{
	WWPROFILE("PhysicsSceneClass::Render");

	/*
	** Render the normal models
	*/
	Render_Objects(rinfo,&VisibleWSMeshList,&VisibleStaticObjectList,&VisibleDynamicObjectList);

	/*
	** Backface Debug rendering
	*/
#ifdef WWDEBUG
	Render_Backface_Occluders(rinfo,&VisibleWSMeshList,&VisibleStaticObjectList);
#endif

	/*
	** Debug widget rendering
	*/
	{
		static LightEnvironmentClass _defaultlightenvironment;
		_defaultlightenvironment.Reset(Vector3(0,0,0),Get_Ambient_Light());
		_defaultlightenvironment.Add_Light(*SunLight);

		rinfo.light_environment = &_defaultlightenvironment;

		Render_Debug_Widgets(rinfo);
		Reset_Debug_Widget_List();

		rinfo.light_environment = NULL;
	}

	if (Pathfinder != NULL) {
		Pathfinder->Render_Debug_Widgets(rinfo);
	}

	/*
	** Vis Sector Debugging
	*/
	if (VisSectorDisplayEnabled || VisSectorHistoryEnabled) {
	
		// list of previous vis sectors so we can render them
		static StaticPhysClass * old_vis_sectors[3] = { NULL, NULL, NULL };
		static int old_vis_index = 0;

		Vector3 vis_sample_point;
		Compute_Vis_Sample_Point(rinfo.Camera,&vis_sample_point);
		StaticPhysClass * vis_sector = StaticCullingSystem->Find_Vis_Tile(vis_sample_point);
		
		if (vis_sector != NULL) {
			MaterialPassClass * matpass = PhysResourceMgrClass::Get_Highlight_Material_Pass();
			if (matpass) {			
				
				// flush all rendering, set wireframe render mode
				WW3D::Flush(rinfo);
				DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_WIREFRAME);

				// set wireframe mode and draw the vis sector
				rinfo.Push_Material_Pass(matpass);
				rinfo.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);

				if (VisSectorHistoryEnabled) {
					// render the previous vis sectors
					for (int i=0; i<3; i++) {
						if (old_vis_sectors[i] != NULL) {
							old_vis_sectors[i]->Render_Vis_Meshes(rinfo);
						}
					}
					// add this one to the history if it is new
					int prev_index = (old_vis_index + 2) % 3;
					if (old_vis_sectors[prev_index] != vis_sector) {
						old_vis_sectors[old_vis_index] = vis_sector;
						old_vis_index = (old_vis_index + 1) % 3;
					}
				} else {
					// Just draw the current vis sector
					vis_sector->Render_Vis_Meshes(rinfo);
				}

				WW3D::Flush(rinfo);
				rinfo.Pop_Material_Pass();
				rinfo.Pop_Override_Flags();

				// restore previous render mode
				switch(Get_Polygon_Mode()) {
					case POINT:
						DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_POINT);
						break;
					case LINE:
						DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
						break;
					case FILL:
						DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_SOLID);
						break;
				}

				// release resources
				REF_PTR_RELEASE(matpass);
			}
		}
	}

	/*
	** Dynamic Vis, Virtual Occludee Debugging
	*/
	if (VisGridDisplayMode == VIS_GRID_DISPLAY_ACTUAL_BOXES) {
		VisTableClass * pvs = Get_Vis_Table_For_Rendering(rinfo.Camera);
		if ((pvs != NULL) && (DynamicObjVisSystem != NULL)) {
			DynamicObjVisSystem->Render_Visible_Cells(rinfo,pvs,DynamicAABTreeCullClass::DISPLAY_ACTUAL_BOXES);
		}
		REF_PTR_RELEASE(pvs);
	}
	
	/*
	** Light Source Debugging, render debug widgets at each light transform
	*/
	static bool ACTIVE_ONLY = true;

	if (LightingDebugDisplayEnabled) {
		StaticLightingSystem->Reset_Collection();
		StaticLightingSystem->Collect_Objects(rinfo.Camera.Get_Frustum());

		LightPhysClass * light = StaticLightingSystem->Peek_First_Collected_Object();
		while (light != NULL) {
			if ((!light->Is_Disabled()) || (!ACTIVE_ONLY)) {
				DEBUG_RENDER_AXES(light->Get_Transform(),Vector3(1.0f,1.0f,1.0f));
			}
			light = StaticLightingSystem->Peek_Next_Collected_Object(light);
		}
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Render_Objects -- Render the visible objects                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/24/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Render_Objects(
	RenderInfoClass& rinfo,
	RefPhysListClass * static_ws_list,
	RefPhysListClass * static_list,
	RefPhysListClass * dyn_list)
{
	WWPROFILE("Render_Meshes");

	RefPhysListIterator it(static_ws_list);

	
	if (WW3D::Get_Mesh_Draw_Mode()!=WW3D::MESH_DRAW_MODE_NONE) {
		if (Is_Backface_Occluder_Debug_Enabled()) {

			// render the static world-space meshes
			{
				WWPROFILE("world-space meshes");

				for (it.First(static_ws_list); !it.Is_Done(); it.Next()) {
					// Only render occluders when backface debug is on
					if (it.Peek_Obj()->As_StaticPhysClass()->Is_Occluder() != 0) {
						Render_Object(rinfo,it.Peek_Obj());
					}
				}
			}

			// render the other static objects
			{
				WWPROFILE("static objects");
				for (it.First(static_list); !it.Is_Done(); it.Next()) {
					// Only render occluders when backface debug is on
					if (it.Peek_Obj()->As_StaticPhysClass()->Is_Occluder() != 0) {
						Render_Object(rinfo,it.Peek_Obj());
					}
				}
			}
		
		} else {

			// render the static world-space meshes
			{
				WWPROFILE("world-space meshes");

				for (it.First(static_ws_list); !it.Is_Done(); it.Next()) {
					Render_Object(rinfo,it.Peek_Obj());
				}
			}

			// render the other static objects
			{
				WWPROFILE("static objects");
				for (it.First(static_list); !it.Is_Done(); it.Next()) {
					Render_Object(rinfo,it.Peek_Obj());
				}
			}
		}
	}

	// render the dynamic objects
	// render dynamic objects even if the render type is "NONE"
	{
		WWPROFILE("dynamic objects");
		for (it.First(dyn_list); !it.Is_Done(); it.Next()) {
			Render_Object(rinfo,it.Peek_Obj());
		}
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Render_Object -- Render an individual object                             *
 *                                                                                             *
 * This function installs the lighting environment and renders the object.                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/24/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Render_Object(RenderInfoClass & context,PhysClass * obj)
{
	if ((obj->Peek_Model() == NULL) || (obj->Is_Rendering_Disabled())) {
		return;
	}

	/*
	** Set up the lighting environment for this object
	*/
	bool do_lighting = (	(obj->Is_Pre_Lit() == false) && 
								(obj->Peek_Model()->Is_Not_Hidden_At_All())	);
	
	if (do_lighting) {

		WWPROFILE("setup lights");

		LightEnvironmentClass & light_env = *(obj->Get_Static_Lighting_Environment());
		light_env.Pre_Render_Update(context.Camera.Get_Transform());

		/*
		** If lighting debugging is enabled, display a vector to each light source
		*/
#if WWDEBUG
		Vector3 pos = obj->Peek_Model()->Get_Bounding_Box().Center;
		if (LightingDebugDisplayEnabled) {
			if ((pos - context.Camera.Get_Position()).Length2() < 30.0f * 30.0f) {
				for (int i=0; i<light_env.Get_Light_Count(); i++) {
					DEBUG_RENDER_VECTOR(pos,light_env.Get_Light_Direction(i),light_env.Get_Light_Diffuse(i));
				}
			}
		}
#endif
		context.light_environment = &light_env;
	
	} else {

		static LightEnvironmentClass _emptylightenvironment;
		_emptylightenvironment.Reset(Vector3(0,0,0),Vector3(1,1,1));
		context.light_environment = &_emptylightenvironment;

	}

	/*
	** Render the object
	*/
	{
		WWPROFILE("render");
		obj->Render(context);
	}

	/*
	** Remove the lighting environment
	*/
	if (do_lighting) {
		context.light_environment = NULL;
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Render_Backface_Occluders -- Render backfaces of all occluders           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/24/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Render_Backface_Occluders
(
	RenderInfoClass & context,
	RefPhysListClass * static_ws_list,
	RefPhysListClass * static_list
)
{
	static LightEnvironmentClass lenv;

	if (BackfaceDebugEnabled) {
		
		MaterialPassClass * matpass = PhysResourceMgrClass::Get_Highlight_Material_Pass();
		if (matpass != NULL) {
		
			/*
			** Flush the system and invert the backface culling check
			*/
			WW3D::Flush(context);
			ShaderClass::Invert_Backface_Culling(true);
			
			/*
			** Set up the render context to render everything bright green
			*/
			ShaderClass shader = matpass->Peek_Shader();
			shader.Set_Depth_Compare(ShaderClass::PASS_LESS);
			matpass->Set_Shader(shader);

			context.Push_Material_Pass(matpass);
			context.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);

			lenv.Reset(Vector3(0,0,0),Vector3(1,1,1));
			context.light_environment = &lenv;

			/*
			** Render the world-space mesh that are occluders
			*/
			RefPhysListIterator it(static_ws_list);
			while (!it.Is_Done()) {
				StaticPhysClass * sphys = it.Peek_Obj()->As_StaticPhysClass();
				if (sphys && sphys->Is_Occluder()) {
					sphys->Render(context);				
				}
				it.Next();
			}

			/*
			** Render the static objects that are occluders
			*/
			it.First(static_list);
			while (!it.Is_Done()) {
				StaticPhysClass * sphys = it.Peek_Obj()->As_StaticPhysClass();
				if (sphys && sphys->Is_Occluder()) {
					sphys->Render(context);				
				}
				it.Next();
			}

			/*
			** Flush all rendering and restore the normal backface culling state
			*/
			WW3D::Flush(context);
			ShaderClass::Invert_Backface_Culling(false);
			context.Pop_Material_Pass();
			context.Pop_Override_Flags();

			REF_PTR_RELEASE(matpass);
		}
	}
}



/***********************************************************************************************
 * PhysicsSceneClass::Re_Partition_Static_Objects -- partition the static objects              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This should be done in the editor.  VIS data is invalidated when you do this...             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Re_Partition_Static_Objects(void)
{
	StaticCullingSystem->Re_Partition();
}


/***********************************************************************************************
 * PhysicsSceneClass::Re_Partition_Static_Lights -- partition the static lights                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This should be done in the editor.  VIS data is invalidated when you do this...             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Re_Partition_Static_Lights(void)
{
	StaticLightingSystem->Re_Partition();
}


/***********************************************************************************************
 * PhysicsSceneClass::Re_Partition_Dynamic_Culling_System -- partition the dynamic culling sys *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This should be done in the editor.  VIS data is invalidated when you do this...             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Re_Partition_Dynamic_Culling_System(void)
{
	Vector3 wmin,wmax;
	Get_Level_Extents(wmin,wmax);

	DynamicCullingSystem->Re_Partition(	wmin,wmax,
													MAX_DYNAMIC_OBJ_RADIUS	);

	DynamicObjVisSystem->Re_Partition(	NULL,
													wmin,wmax,
													MIN_GRID_CELL_SIZE,
													MAX_GRID_CELL_COUNT,
													MAX_DYNAMIC_OBJ_RADIUS	);

	Reset_Vis();
}


/***********************************************************************************************
 * PhysicsSceneClass::Re_Partition_Dynamic_Culling_System -- partition the dynamic culling sys *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This should be done in the editor.  VIS data is invalidated when you do this...             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Re_Partition_Dynamic_Culling_System(DynamicVectorClass<AABoxClass> & virtual_occludees)
{
	Vector3 wmin,wmax;
	Get_Level_Extents(wmin,wmax);

	DynamicCullingSystem->Re_Partition(	wmin,wmax,
													MAX_DYNAMIC_OBJ_RADIUS	);

	AABoxClass bounds(virtual_occludees[0]);
	for (int i=0; i<virtual_occludees.Count(); i++) {
		bounds.Add_Box(virtual_occludees[i]);
	}
	
	DynamicObjVisSystem->Re_Partition(	&virtual_occludees,
													bounds.Center - bounds.Extent,
													bounds.Center + bounds.Extent,
													MIN_GRID_CELL_SIZE,
													MAX_GRID_CELL_COUNT,
													MAX_DYNAMIC_OBJ_RADIUS	);

	Reset_Vis();
}


/***********************************************************************************************
 * PhysicsSceneClass::Re_Partition_Static_Projectors -- partition the static projectors        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Re_Partition_Static_Projectors(void)
{
	StaticProjectorCullingSystem->Re_Partition();
}


/***********************************************************************************************
 * PhysicsSceneClass::Update_Culling_System_Bounding_Boxes -- updates the cull systems         *
 *                                                                                             *
 * This should be performed by the level editor when geometry may have changed but we do not   *
 * want to do a re-partition due to the loss of hierarchical VIS data.  Basically, the editor  *
 * will call this every time it loads an LVL in case the user has changed the geometry.  It    *
 * causes all bounding boxes in the static culling systems to be validated.                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/3/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Update_Culling_System_Bounding_Boxes(void)
{
	StaticCullingSystem->Update_Bounding_Boxes();
	StaticLightingSystem->Update_Bounding_Boxes();
}


/***********************************************************************************************
 * PhysicsSceneClass::Verify_Culling_Systems -- Verify the culling systems                     *
 *                                                                                             *
 *    This function asks the culling systems to verify that all of thier bounding boxes        *
 *    are properly constructed.  I.e. they contain the objects they claim to contain, etc.     *
 *                                                                                             *
 * INPUT:                                                                                      *
 * error_report - will be filled in with a description of any errors encountered               *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/28/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool PhysicsSceneClass::Verify_Culling_Systems(StringClass & set_error_report)
{
	return StaticCullingSystem->Verify(set_error_report);
}

/***********************************************************************************************
 * PhysicsSceneClass::Get_Level_Extents -- returns the bounds of the level                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Get_Level_Extents(Vector3 &min, Vector3 &max)
{
	const AABoxClass & level_bounds = StaticCullingSystem->Get_Bounding_Box();
	min = level_bounds.Center - level_bounds.Extent;
	max = level_bounds.Center + level_bounds.Extent;
}


/***********************************************************************************************
 * PhysicsSceneClass::Set_Polygon_Budgets -- set the budgets for the LOD system                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Set_Polygon_Budgets(int static_count,int dynamic_count)
{
	StaticPolyBudget = static_count;
	DynamicPolyBudget = dynamic_count;
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Polygon_Budgets -- returns the budgets for the LOD system            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Get_Polygon_Budgets(int * static_count,int * dynamic_count)
{
	*static_count = StaticPolyBudget;
	*dynamic_count = DynamicPolyBudget;
}


/***********************************************************************************************
 * PhysicsSceneClass::Per_Frame_Statistics_Update -- statistics tracking                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Per_Frame_Statistics_Update(void)
{
	CurrentStats.FrameCount ++;

	if (CurrentStats.FrameCount >= STATISTICS_FRAMES) {
		
		/*
		** Collect the culling system stats
		*/
		const GridCullSystemClass::StatsStruct & dyn_stats = DynamicCullingSystem->Get_Statistics();
		const AABTreeCullSystemClass::StatsStruct & static_stats = StaticCullingSystem->Get_Statistics();
		const AABTreeCullSystemClass::StatsStruct & light_stats = StaticLightingSystem->Get_Statistics();

		CurrentStats.CullNodeCount = dyn_stats.NodeCount + static_stats.NodeCount + light_stats.NodeCount;
		CurrentStats.CullNodesAccepted = dyn_stats.NodesAccepted + static_stats.NodesAccepted + light_stats.NodesAccepted;
		CurrentStats.CullNodesTriviallyAccepted = dyn_stats.NodesTriviallyAccepted + static_stats.NodesTriviallyAccepted + light_stats.NodesTriviallyAccepted;
		CurrentStats.CullNodesRejected = dyn_stats.NodesRejected + static_stats.NodesRejected + light_stats.NodesRejected;

		DynamicCullingSystem->Reset_Statistics();
		StaticCullingSystem->Reset_Statistics();
		StaticLightingSystem->Reset_Statistics();

		/*
		** Copy over LastValidStats, reset
		*/
		LastValidStats = CurrentStats;
		CurrentStats.Reset();
	}
}


/***********************************************************************************************
 * PhysicsSceneClass::Get_Statistics -- returns statistics                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
const PhysicsSceneClass::StatsStruct & PhysicsSceneClass::Get_Statistics(void)
{
	return LastValidStats;
}


/***********************************************************************************************
 * PhysicsSceneClass::Find_Static_Object -- Find static object with the given ID               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
StaticPhysClass * PhysicsSceneClass::Find_Static_Object( int instance_id )
{
	RefPhysListIterator it(&StaticObjList);
	for (it.First(); !it.Is_Done(); it.Next()) {
		if ( (int)(it.Peek_Obj()->Get_ID()) == instance_id ) {
			return (StaticPhysClass*)it.Peek_Obj();
		}
	}
	return NULL;
}



/***********************************************************************************************
 * PhysicsSceneClass::Shatter_Mesh -- Clips a mesh into pieces and scatters them               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void PhysicsSceneClass::Shatter_Mesh
(
	MeshClass * mesh,
	const Vector3 & impact_point,
	const Vector3 & impact_normal,
	const Vector3 & impact_velocity
)
{
	LookupTableClass * vel_table = LookupTableMgrClass::Get_Table("ShatterVel.tbl");
	LookupTableClass * avel_table = LookupTableMgrClass::Get_Table("ShatterAVel.tbl");

	/*
	** Clip the mesh into fragments
	*/
	ShatterSystem::Shatter_Mesh(mesh,impact_point,impact_normal);
	
	/*
	** Wake up any dynamic objects in the area
	*/
	if (mesh->Get_Collision_Type() & COLLISION_TYPE_PHYSICAL) {
		AABoxClass bounds = mesh->Get_Bounding_Box();
		bounds.Extent *= 1.05f;
		Force_Dynamic_Objects_Awake(bounds);
	}

	/*
	** Disable the original mesh so that it doesn't shatter repeatedly
	*/
	mesh->Set_Hidden(true);
	mesh->Set_Collision_Type(0);

	/*
	** Now create new auto-destructing projectiles for each fragment
	*/
	for (int i=0; i<ShatterSystem::Get_Fragment_Count(); i++) {

		ProjectileClass * frag = NEW_REF(ProjectileClass,());
		RenderObjClass * frag_model = ShatterSystem::Peek_Fragment(i);
		Matrix3D frag_tm = frag_model->Get_Transform();
		
		frag->Set_Model(frag_model);
		frag->Set_Transform(frag_tm);
		frag->Set_Orientation_Mode_Tumbling();
		
		frag->Set_Lifetime(3.0f);
		frag->Set_Gravity_Multiplier(2.0f);
		frag->Set_Bounce_Count(1);		

		/*
		** Vector from the impact to the center of the fragment is 
		** used in generating the initial velocity and rotation of the fragment
		*/
		Vector3 dc = frag_tm.Get_Translation() - impact_point;
		float dclen = dc.Length();
		dc /= dclen;
		
		/*
		** Generate a suitable velocity for the fragment
		*/
		Vector3 frag_vel = impact_velocity;
		frag_vel.Normalize();
		if (rand() & 0x00000100) {
			frag_vel *= -1.0f;
		}
		frag_vel += 0.2f*dc;
		frag_vel *= vel_table->Get_Value(dclen);
		frag->Set_Velocity(frag_vel);
		
		/*
		** Generate a rotation for the fragment.  The axis it rotates about
		** will be in the plane of the impact, perpendicular to a vector from
		** the impact to the CM
		*/
		Vector3 axis;
		Vector3::Cross_Product(impact_velocity,dc,&axis);
		frag->Set_Orientation_Mode_Tumbling(axis,avel_table->Get_Value(dclen));
		
		/*
		** We cannot allow these object to get saved since their model was
		** procedurally generated and it will not get loaded...
		*/
		frag->Enable_Dont_Save(true);

		/*
		** Debugging, Stop the velocity, set life to infinite, move along initial velocity to offset pieces
		*/
#if (SHATTER_DEBUG)
		Matrix3D tm = frag->Get_Transform();
		Vector3 pos,vel;
		tm.Get_Translation(&pos);
		frag->Get_Velocity(&vel);
		tm.Set_Translation(pos+vel*0.1f);
		frag->Set_Transform(tm);
		frag->Set_Velocity(Vector3(0,0,0));
		frag->Set_Lifetime(60.0f);
		frag->Set_Gravity_Multiplier(0.0f);
		frag->Set_Bounce_Count(100);
		frag->Set_Orientation_Mode_Fixed();		
#endif

		/*
		** Add the fragment to the scene
		*/
		Add_Dynamic_Object(frag);
		REF_PTR_RELEASE(frag);
	}

	/*
	** Done, have the shatter system release the fragments
	*/
	ShatterSystem::Release_Fragments();
	vel_table->Release_Ref();
	avel_table->Release_Ref();

}


void PhysicsSceneClass::Add_Camera_Shake
(
	const Vector3 & position,
	float radius,
	float duration,
	float power
)
{
	WWASSERT(CameraShakeSystem != NULL);
	CameraShakeSystem->Add_Camera_Shake(position,radius,duration,power);
}

void PhysicsSceneClass::Apply_Camera_Shakes(CameraClass & camera)
{
	CameraShakeSystem->Update_Camera(camera);
}

float PhysicsSceneClass::Compute_Vis_Mesh_Ram(void)
{
	float total = 0.0f;

	RefPhysListIterator it(&StaticObjList);
	for (it.First(); !it.Is_Done(); it.Next()) {
		total += ((StaticPhysClass *)it.Peek_Obj())->Compute_Vis_Mesh_Ram();
	}
	return total;
}

void PhysicsSceneClass::Force_Dynamic_Objects_Awake(const AABoxClass & box)
{
	/*
	** Collect the objects that touch this box and wake them up.
	*/
	NonRefPhysListClass list;
	Collect_Objects(box,false,true,&list);
	NonRefPhysListIterator it(&list);
	while (!it.Is_Done()) {
		PhysClass * obj = it.Peek_Obj();
		obj->Force_Awake();
		if (obj->As_Phys3Class() != NULL) {
			obj->As_Phys3Class()->Invalidate_Ground_State();
		}
		it.Next();
	}
}


/******************************************************************************************
**
**
** PhysicsSceneClass::StatsStruct Implementation
**
**
******************************************************************************************/
PhysicsSceneClass::StatsStruct::StatsStruct(void)
{ 
	Reset();
}

void PhysicsSceneClass::StatsStruct::Reset(void)
{
	FrameCount = 0; 
	CullNodeCount = 0;
	CullNodesAccepted = 0;
	CullNodesTriviallyAccepted = 0;
	CullNodesRejected = 0;
}


/*
** Force-Link relevant modules from WWPhys
*/
void Force_Link_Modules(void) 
{
	FORCE_LINK(decophys);
	FORCE_LINK(humanphys);
	FORCE_LINK(lightphys);
	FORCE_LINK(motorcycle);
	FORCE_LINK(movephys);
	FORCE_LINK(phys3);
	FORCE_LINK(rbody);
	FORCE_LINK(staticphys);
	FORCE_LINK(staticanimphys);
	FORCE_LINK(wheelvehicle);
	FORCE_LINK(projectile);
	FORCE_LINK(timeddecophys);
	FORCE_LINK(trackedvehicle);
	FORCE_LINK(vtolvehicle);
	FORCE_LINK(waypath);
	FORCE_LINK(waypoint);
	FORCE_LINK(dynamicanimphys);
	FORCE_LINK(shakeablestaticphys);
	FORCE_LINK(RenegadeTerrainPatch);
}


