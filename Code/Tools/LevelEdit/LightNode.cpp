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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/LightNode.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/14/02 5:14p                                               $*
 *                                                                                             *
 *                    $Revision:: 21                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "lightnode.h"
#include "sceneeditor.h"
#include "lightdefinition.h"
#include "filemgr.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "w3d_file.h"
#include "cameramgr.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "light.h"
#include "nodeinfopage.h"
#include "positionpage.h"
#include "preset.h"
#include "editorpropsheet.h"
#include "presetsettingstab.h"
#include "nodemgr.h"
#include "matinfo.h"
#include "modelutils.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<LightNodeClass, CHUNKID_NODE_LIGHT> _LightNodePersistFactory;

enum
{
	CHUNKID_VARIABLES				= 0x01040345,
	CHUNKID_BASE_CLASS,
	CHUNKID_INSTANCE_SETTINGS
};

enum
{
	VARID_VIS_SECTOR_ID			= 1,
};


//////////////////////////////////////////////////////////////////////////////
//
//	LightNodeClass
//
//////////////////////////////////////////////////////////////////////////////
LightNodeClass::LightNodeClass (PresetClass *preset)
	:	m_LightPhysObj (NULL),
		m_DisplayObj (NULL),
		m_UsePreset (true),
		m_Sphere (NULL),
		m_VisSectorID (0xFFFFFFFF),
		NodeClass (preset)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	LightNodeClass
//
//////////////////////////////////////////////////////////////////////////////
LightNodeClass::LightNodeClass (const LightNodeClass &src)
	:	m_LightPhysObj (NULL),
		m_DisplayObj (NULL),
		m_UsePreset (true),
		m_Sphere (NULL),
		m_VisSectorID (0xFFFFFFFF),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~LightNodeClass
//
//////////////////////////////////////////////////////////////////////////////
LightNodeClass::~LightNodeClass (void)
{	
	Remove_From_Scene ();
	MEMBER_RELEASE (m_LightPhysObj);
	MEMBER_RELEASE (m_DisplayObj);	
	MEMBER_RELEASE (m_Sphere);
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//	Note:  This may be called more than once.  It is used as an 'initialize'
// and a 're-initialize'.
//
//////////////////////////////////////////////////////////////////////////////
void
LightNodeClass::Initialize (void)
{
	MEMBER_RELEASE (m_LightPhysObj);
	MEMBER_RELEASE (m_DisplayObj);	

	//
	//	Use the preset's settings (if desired)
	//
	if (m_UsePreset == true && m_Preset != NULL) {
		LightDefinitionClass *definition = static_cast<LightDefinitionClass *> (m_Preset->Get_Definition ());
		if (definition != NULL) {
			m_InstanceSettings = *definition;
		}
	}

	//
	//	Create the light render-object this node depends on
	//
	LightClass *light_obj	= new LightClass;
	m_LightPhysObj				= new LightPhysClass(true);
	m_DisplayObj				= new DecorationPhysClass;
		
	//
	// Configure the physics object with information about
	// its new render object and collision data.
	//
	m_LightPhysObj->Set_Model (light_obj);
	m_DisplayObj->Set_Model_By_Name ("POINTLIGHT");
	m_DisplayObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, FALSE);
	::Set_Model_Collision_Type (m_DisplayObj->Peek_Model (), COLLISION_TYPE_6);
	
	m_LightPhysObj->Set_Transform (m_Transform);
	m_DisplayObj->Set_Transform (m_Transform);

	//
	//	Make sure the light settings are correct
	//
	Update_Light ();
	
	// Release our hold on the light object pointer
	MEMBER_RELEASE (light_obj);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
LightNodeClass::Get_Factory (void) const
{	
	return _LightNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const LightNodeClass &
LightNodeClass::operator= (const LightNodeClass &src)
{
	NodeClass::operator= (src);
	m_InstanceSettings = src.m_InstanceSettings;
	return *this;
}


/////////////////////////////////////////////////////////////////
//
//	Show_Settings_Dialog
//
/////////////////////////////////////////////////////////////////
bool
LightNodeClass::Show_Settings_Dialog (void)
{	
	NodeInfoPageClass	info_tab (this);
	PositionPageClass	pos_tab (this);
	
	//
	//	Display the settings tab for instance overrides
	//
	PresetClass dummy_preset;
	dummy_preset.Set_Definition (&m_InstanceSettings);
	PresetSettingsTabClass settings_tab (&dummy_preset);

	//
	//	Add each tab to the property sheet
	//
	EditorPropSheetClass prop_sheet;
	prop_sheet.Add_Page (&info_tab);
	prop_sheet.Add_Page (&pos_tab);
	prop_sheet.Add_Page (&settings_tab);

	// Show the property sheet
	UINT ret_code = prop_sheet.DoModal ();
	if (ret_code == IDOK) {
		Update_Light ();
		m_UsePreset = false;
	}
	
	// Return true if the user clicked OK
	return (ret_code == IDOK);
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
LightNodeClass::Pre_Export (void)
{
	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (m_DisplayObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (m_DisplayObj);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
LightNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (m_DisplayObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (m_DisplayObj);
	}
	Release_Ref ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
LightNodeClass::Add_To_Scene (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();

	if (	m_LightPhysObj != NULL &&
			scene->Contains (m_LightPhysObj) == false) {
		
		//
		//	Add the static light to the scene and the display object
		// the editor uses as a visual representation of the light
		//
		scene->Add_Static_Light (m_LightPhysObj);
		scene->Add_Dynamic_Object (m_DisplayObj);
		if (m_Sphere != NULL) {
			m_Sphere->Display_Around_Node (*this);
		}
	}
	
	m_IsInScene = true;	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
LightNodeClass::Remove_From_Scene (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();

	if (	m_LightPhysObj != NULL &&
			scene->Contains (m_LightPhysObj)) {
		
		//
		//	Remove the static light from the scene and the display object
		// the editor uses as a visual representation of the light
		//
		scene->Remove_Object (m_LightPhysObj);
		scene->Remove_Object (m_DisplayObj);		
		if (m_Sphere != NULL) {
			m_Sphere->Remove_From_Scene ();
		}
	}
	
	m_IsInScene = false;	
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
LightNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();

	if (m_UsePreset == false) {
		csave.Begin_Chunk (CHUNKID_INSTANCE_SETTINGS);
			m_InstanceSettings.Save (csave);
		csave.End_Chunk ();
	}

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
LightNodeClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				NodeClass::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
			
			case CHUNKID_INSTANCE_SETTINGS:
				m_InstanceSettings.Load (cload);
				m_UsePreset = false;
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
LightNodeClass::Save_Variables (ChunkSaveClass &csave)
{
	bool retval = true;

	WRITE_MICRO_CHUNK (csave, VARID_VIS_SECTOR_ID, m_VisSectorID)
	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
LightNodeClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			READ_MICRO_CHUNK (cload, VARID_VIS_SECTOR_ID, m_VisSectorID)
		}

		cload.Close_Micro_Chunk ();
	}

	return retval;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Light
//
/////////////////////////////////////////////////////////////////
void
LightNodeClass::Update_Light (void)
{
	//
	//	Do we have a light to setup?
	//
	if (	m_LightPhysObj != NULL &&
			m_LightPhysObj->Peek_Model () != NULL )
	{
		LightClass *light_obj = new LightClass (m_InstanceSettings.Get_Light_Type ());

		Vector3 ambient		= m_InstanceSettings.Get_Ambient_Color ();
		Vector3 diffuse		= m_InstanceSettings.Get_Diffuse_Color ();
		Vector3 specular		= m_InstanceSettings.Get_Specular_Color ();
		float intensity		= m_InstanceSettings.Get_Intensity ();
		double inner_radius	= m_InstanceSettings.Get_Inner_Radius ();
		double outer_radius	= m_InstanceSettings.Get_Outer_Radius ();
		float spot_angle		= m_InstanceSettings.Get_Spot_Angle ();
		Vector3 spot_dir		= m_InstanceSettings.Get_Spot_Direction ();
		float spot_exp			= m_InstanceSettings.Get_Spot_Exponent ();
		bool casts_shadows	= m_InstanceSettings.Does_Cast_Shadows ();

		//
		//	Configure the light object
		//
		light_obj->Set_Flag (LightClass::NEAR_ATTENUATION, true);
		light_obj->Set_Flag (LightClass::FAR_ATTENUATION, true);
		light_obj->Set_Intensity (intensity);
		light_obj->Set_Ambient (ambient);
		light_obj->Set_Diffuse (diffuse);
		light_obj->Set_Specular (specular);
		light_obj->Set_Far_Attenuation_Range (inner_radius, outer_radius);
		light_obj->Set_Spot_Angle (spot_angle);
		light_obj->Set_Spot_Direction (spot_dir);
		light_obj->Set_Spot_Exponent (spot_exp);
		light_obj->Enable_Shadows (casts_shadows);

		//
		//	Pass the new model onto the light physics object
		//
		m_LightPhysObj->Set_Model (light_obj);
		m_LightPhysObj->Set_Transform (m_Transform);
		m_LightPhysObj->Set_Vis_Sector_ID (m_VisSectorID);		

		//
		// Loop through all the vertext materials in the display object
		//	and make them match the light...
		//
		MaterialInfoClass *material_info = light_obj->Get_Material_Info ();
		if (material_info != NULL) {
			
			int counter = material_info->Vertex_Material_Count ();
			while (counter --) {
				VertexMaterialClass *vertex_mat = material_info->Peek_Vertex_Material (counter);
				if (vertex_mat != NULL) {					
					vertex_mat->Set_Ambient (ambient);
					vertex_mat->Set_Emissive (ambient);
					vertex_mat->Set_Diffuse (diffuse);
					vertex_mat->Set_Specular (specular);
				}
			}

			MEMBER_RELEASE (material_info);
		}

		//
		//	Update the sphere's settings
		//
		if (m_Sphere != NULL) {
			m_Sphere->Set_Radius (outer_radius);
			m_Sphere->Set_Color (ambient);
		}

		//
		//	Release our hold on the light object
		//
		MEMBER_RELEASE (light_obj);
	}

	//
	//	Make sure the attenuation spheres are correctly displayed
	//
	Show_Attenuation_Spheres (::Get_Scene_Editor ()->Are_Light_Spheres_Displayed ());
	::Get_Scene_Editor ()->Update_Lighting ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Initialize_From_Light
//
/////////////////////////////////////////////////////////////////
void
LightNodeClass::Initialize_From_Light (LightClass *light)
{
	m_UsePreset = false;	

	//
	//	Read the settings we care about from the light
	//
	Vector3 ambient_color (0, 0, 0);
	Vector3 diffuse_color (0, 0, 0);
	Vector3 specular_color (0, 0, 0);	
	light->Get_Ambient (&ambient_color);
	light->Get_Diffuse (&diffuse_color);
	light->Get_Specular (&specular_color);

	double inner = 0;
	double outer = 0;
	light->Get_Far_Attenuation_Range (inner, outer);

	float intensity				= light->Get_Intensity ();
	LightClass::LightType type = light->Get_Type ();

	float spot_angle	= light->Get_Spot_Angle ();
	float spot_exp		= light->Get_Spot_Exponent ();
	Vector3 spot_dir (0, 0, 0);
	light->Get_Spot_Direction (spot_dir);

	bool casts_shadows = light->Are_Shadows_Enabled ();

	//
	//	Store the new settings
	//
	m_InstanceSettings.Set_Ambient_Color (ambient_color);
	m_InstanceSettings.Set_Diffuse_Color (diffuse_color);
	m_InstanceSettings.Set_Specular_Color (specular_color);
	m_InstanceSettings.Set_Intensity (intensity);
	m_InstanceSettings.Set_Inner_Radius (inner);
	m_InstanceSettings.Set_Outer_Radius (outer);
	m_InstanceSettings.Set_Light_Type (type);
	m_InstanceSettings.Set_Spot_Angle (spot_angle);
	m_InstanceSettings.Set_Spot_Direction (spot_dir);
	m_InstanceSettings.Set_Spot_Exponent (spot_exp);
	m_InstanceSettings.Casts_Shadows (casts_shadows);

	//
	//	Update the light we use...
	//
	Update_Light ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Show_Attenuation_Spheres
//
//////////////////////////////////////////////////////////////////////
void
LightNodeClass::Show_Attenuation_Spheres (bool onoff)
{
	if (onoff && m_Sphere == NULL) {
		m_Sphere = new AttenuationSphereClass;
		m_Sphere->Display_Around_Node (*this);
		m_Sphere->Set_Color (m_InstanceSettings.Get_Ambient_Color ());
		m_Sphere->Set_Radius (Get_Attenuation_Radius ());
		m_Sphere->Set_Opacity (0.25F);
	} else if (onoff == false && m_Sphere != NULL) {
		m_Sphere->Remove_From_Scene ();
		MEMBER_RELEASE (m_Sphere);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Peek_Light
//
//////////////////////////////////////////////////////////////////////
LightClass *
LightNodeClass::Peek_Light (void)
{
	LightClass *light = NULL;

	//
	//	Check to make sure we have a light objects
	//
	if (	m_LightPhysObj != NULL &&
			m_LightPhysObj->Peek_Model () != NULL )
	{
		//
		//	Can we get at the light from its phys-wrapper?
		//
		RenderObjClass *model = m_LightPhysObj->Peek_Model ();
		if (model != NULL && model->Class_ID () == RenderObjClass::CLASSID_LIGHT) {
			light = (LightClass *)model;
		}		
	}

	return light;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Vis_Sector_ID
//
//////////////////////////////////////////////////////////////////////
void
LightNodeClass::Set_Vis_Sector_ID (uint32 vis_id)
{
	m_VisSectorID = vis_id;

	//
	//	Update the physics object as well
	//
	if (m_LightPhysObj != NULL) {
		m_LightPhysObj->Set_Vis_Sector_ID (m_VisSectorID);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Cached_Vis_IDs
//
/////////////////////////////////////////////////////////////////
void
LightNodeClass::Update_Cached_Vis_IDs (void)
{
	if (m_LightPhysObj != NULL) {
		m_VisSectorID = m_LightPhysObj->Get_Vis_Sector_ID ();
	}

	return ;
}
