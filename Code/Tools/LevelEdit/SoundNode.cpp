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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/SoundNode.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/16/01 5:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "soundnode.h"
#include "sceneeditor.h"
#include "filemgr.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "w3d_file.h"
#include "cameramgr.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "Sound3D.h"
#include "soundsettingspage.h"
#include "nodeinfopage.h"
#include "positionpage.h"
#include "editorpropsheet.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factories
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<SoundNodeClass, CHUNKID_NODE_NEW_SOUND> _NewSoundNodePersistFactory;

class SoundNodePersistFactoryClass : public SimplePersistFactoryClass<SoundNodeClass, CHUNKID_NODE_OLD_SOUND>
{
public:
	PersistClass *	Load (ChunkLoadClass &cload) const
	{
		SoundNodeClass * new_obj = new SoundNodeClass;
		SoundNodeClass * old_obj = NULL;

		//
		//	Load the object's old pointer
		//
		cload.Open_Chunk();
			WWASSERT(cload.Cur_Chunk_ID() == SIMPLEFACTORY_CHUNKID_OBJPOINTER);
			cload.Read(&old_obj,sizeof(SoundNodeClass *));
		cload.Close_Chunk();

		//
		//	Load the sound node object using a legacy load
		//
		cload.Open_Chunk();
			WWASSERT(cload.Cur_Chunk_ID() == SIMPLEFACTORY_CHUNKID_OBJDATA);
			new_obj->Use_Legacy_Load (true);
			new_obj->Load(cload);
		cload.Close_Chunk();

		SaveLoadSystemClass::Register_Pointer(old_obj,new_obj);
		return new_obj;
	}
};

SoundNodePersistFactoryClass _LegacySoundNodePersistFactory;


//////////////////////////////////////////////////////////////////////////////
//	Save/load constants
//////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_BASE_CLASS				= 0x07160304,
	CHUNKID_INSTANCE_SETTINGS
};


//////////////////////////////////////////////////////////////////////////////
//
//	SoundNodeClass
//
//////////////////////////////////////////////////////////////////////////////
SoundNodeClass::SoundNodeClass (PresetClass *preset)
	:	PhysObj (NULL),
		SoundObj (NULL),
		Sphere (NULL),
		OverridePreset (false),
		UseLegacyLoad (false),
		NodeClass (preset)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	SoundNodeClass
//
//////////////////////////////////////////////////////////////////////////////
SoundNodeClass::SoundNodeClass (const SoundNodeClass &src)
	:	PhysObj (NULL),
		SoundObj (NULL),
		Sphere (NULL),
		OverridePreset (false),
		UseLegacyLoad (false),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~SoundNodeClass
//
//////////////////////////////////////////////////////////////////////////////
SoundNodeClass::~SoundNodeClass (void)
{	
	Remove_From_Scene ();
	Release_Sound ();
	MEMBER_RELEASE (PhysObj);
	MEMBER_RELEASE (Sphere);
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Release_Sound
//
//////////////////////////////////////////////////////////////////////////////
void
SoundNodeClass::Release_Sound (void)
{
	if (SoundObj != NULL) {
		SoundObj->Remove_From_Scene ();
		SoundObj->Stop ();
		SoundObj->Release_Ref ();
		SoundObj = NULL;
	}

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
SoundNodeClass::Initialize (void)
{
	//
	//	Start fresh
	//
	Show_Attenuation_Spheres (false);
	Release_Sound ();
	MEMBER_RELEASE (PhysObj);	

	AudibleSoundDefinitionClass *definition = NULL;
	definition = static_cast<AudibleSoundDefinitionClass *> (m_Preset->Get_Definition ());
	if (	definition != NULL &&
			::SuperClassID_From_ClassID (definition->Get_Class_ID ()) == CLASSID_SOUND)
	{
		//
		//	Copy the preset's settings to our instance (if necessary)
		//
		if (OverridePreset == false) {
			InstanceSettings.Set_Volume (definition->Get_Volume ());		
			InstanceSettings.Set_DropOff_Radius (definition->Get_DropOff_Radius ());
			InstanceSettings.Set_Start_Offset (definition->Get_Start_Offset ());		
			InstanceSettings.Set_Pitch_Factor (definition->Get_Pitch_Factor ());		
			InstanceSettings.Set_Max_Vol_Radius (definition->Get_Max_Vol_Radius ());
		}

		//
		//	Create the render object this tile depends on
		//
		RenderObjClass *render_obj = ::Create_Render_Obj ("SPEAKER");
		WWASSERT (render_obj != NULL);
		if (render_obj != NULL) {
			
			// Create the new physics object
			PhysObj = new DecorationPhysClass;
			
			//
			// Configure the physics object with information about
			// its new render object and collision data.
			//
			PhysObj->Set_Model (render_obj);
			PhysObj->Set_Transform (Matrix3D(1));
			PhysObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
			PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, FALSE);
			PhysObj->Peek_Model ()->Set_Collision_Type (COLLISION_TYPE_6);
			PhysObj->Set_Transform (m_Transform);
			
			//
			// Release our hold on the render object pointer
			//
			MEMBER_RELEASE (render_obj);

			//
			//	Create the sound object
			//
			Update_Sound ();

			//
			//	Update the position and orientation
			//
			Set_Transform (m_Transform);
		}
	}

	//
	//	Make sure the attenuation spheres are correctly displayed
	//
	Show_Attenuation_Spheres (::Get_Scene_Editor ()->Are_Sound_Spheres_Displayed ());
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
SoundNodeClass::Get_Factory (void) const
{	
	return _NewSoundNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const SoundNodeClass &
SoundNodeClass::operator= (const SoundNodeClass &src)
{
	NodeClass::operator= (src);
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////
void
SoundNodeClass::Add_To_Scene (void)
{
	if (SoundObj != NULL && SoundObj->Is_In_Scene () == false) {
		SoundObj->Add_To_Scene ();
	}

	if (Sphere != NULL) {
		Sphere->Display_Around_Node (*this);
	}

	NodeClass::Add_To_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////
void
SoundNodeClass::Remove_From_Scene (void)
{
	if (SoundObj != NULL && SoundObj->Is_In_Scene ()) {
		SoundObj->Stop ();
		SoundObj->Remove_From_Scene ();
	}

	if (Sphere != NULL) {
		Sphere->Remove_From_Scene ();
	}

	NodeClass::Remove_From_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
SoundNodeClass::Pre_Export (void)
{
	Add_Ref ();
	if (PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (PhysObj);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
SoundNodeClass::Post_Export (void)
{
	if (PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (PhysObj);
	}

	Release_Ref ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Show_Attenuation_Spheres
//
//////////////////////////////////////////////////////////////////////
void
SoundNodeClass::Show_Attenuation_Spheres (bool onoff)
{
	if (onoff && Sphere == NULL) {
		
		Sphere = new AttenuationSphereClass;
		Sphere->Display_Around_Node (*this);		
		Sphere->Set_Opacity (0.25F);
		Sphere->Set_Radius (Get_Attenuation_Radius ());

		//
		//	Determine what color to make the sphere
		//
		if (m_Preset != NULL && m_Preset->Get_Definition () != NULL) {
			AudibleSoundDefinitionClass *definition = NULL;
			definition = static_cast<AudibleSoundDefinitionClass *> (m_Preset->Get_Definition ());
			if (::SuperClassID_From_ClassID (definition->Get_Class_ID ()) == CLASSID_SOUND) {
				Sphere->Set_Color (definition->Get_Sphere_Color ());
			}
		} else {
			Sphere->Set_Color (Vector3 (0, 0.75F, 0.75F));
		}

	} else if (onoff == false && Sphere != NULL) {
		Sphere->Remove_From_Scene ();
		MEMBER_RELEASE (Sphere);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Set_ID
//
/////////////////////////////////////////////////////////////////
void
SoundNodeClass::Set_ID (uint32 id)
{
	NodeClass::Set_ID (id);
	if (SoundObj != NULL) {
		SoundObj->Set_ID (m_ID);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Update_Sound
//
/////////////////////////////////////////////////////////////////
void
SoundNodeClass::Update_Sound (void)
{
	if (m_Preset == NULL) {
		return ;
	}

	//
	//	Dig the definition out of the preset
	//
	AudibleSoundDefinitionClass *definition = NULL;
	definition = static_cast<AudibleSoundDefinitionClass *> (m_Preset->Get_Definition ());
	if (definition == NULL) {
		return ;
	}

	//
	//	Start fresh
	//
	Release_Sound ();

	//
	//	Make sure we 'get' all the assets locally that we need
	// for the preset.
	//
	m_Preset->Load_All_Assets ();

	//
	//	Setup the current directory so the file system can find the sound
	// file we are about to load.
	//
	CString full_path = ::Get_File_Mgr ()->Make_Full_Path (definition->Get_Filename ());
	CString path_only = ::Strip_Filename_From_Path (full_path);
	::Set_Current_Directory (path_only);

	//
	//	Create and initialize the sound object
	//			
	SoundObj = (AudibleSoundClass *)definition->Create ();
	if (SoundObj != NULL) {
		SoundObj->Set_Transform (m_Transform);
		SoundObj->Set_ID (m_ID);		
		
		//
		//	Make sure the sound object is 'static'
		//
		Sound3DClass *sound3d = SoundObj->As_Sound3DClass ();
		if (sound3d != NULL) {
			sound3d->Make_Static (true);
		}

		//
		//	Override the preset settings with our instance settings
		//
		if (OverridePreset) {
			SoundObj->Set_Volume (InstanceSettings.Get_Volume ());		
			SoundObj->Set_DropOff_Radius (InstanceSettings.Get_DropOff_Radius ());
			SoundObj->Set_Start_Offset (InstanceSettings.Get_Start_Offset ());		
			SoundObj->Set_Pitch_Factor (InstanceSettings.Get_Pitch_Factor ());
			if (sound3d != NULL) {
				sound3d->Set_Max_Vol_Radius (InstanceSettings.Get_Max_Vol_Radius ());
			}
		}

		//
		//	Add the sound to the scene (if necessary)
		//
		if (In_Scene ()) {
			SoundObj->Add_To_Scene ();
		}

	} else {
		WWDEBUG_SAY (("Unable to create sound object for preset %s\r\n", definition->Get_Name ()));
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Show_Settings_Dialog
//
/////////////////////////////////////////////////////////////////
bool
SoundNodeClass::Show_Settings_Dialog (void)
{	
	NodeInfoPageClass	info_tab (this);
	PositionPageClass	pos_tab (this);
	
	//
	//	Display the settings tab for instance overrides
	//
	SoundSettingsPageClass settings_tab;
	settings_tab.Set_Definition (&InstanceSettings);

	//
	//	Add each tab to the property sheet
	//
	EditorPropSheetClass prop_sheet;
	prop_sheet.Add_Page (&info_tab);
	prop_sheet.Add_Page (&pos_tab);
	prop_sheet.Add_Page (&settings_tab);

	//
	// Show the property sheet
	//
	UINT ret_code = prop_sheet.DoModal ();
	if (ret_code == IDOK) {
		OverridePreset = true;
		Update_Sound ();		
	}
	
	// Return true if the user clicked OK
	return (ret_code == IDOK);
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
SoundNodeClass::Save (ChunkSaveClass &csave)
{
	//
	//	Save the base class
	//
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();

	//
	//	Save the parent class
	//
	if (OverridePreset) {
		csave.Begin_Chunk (CHUNKID_INSTANCE_SETTINGS);
			InstanceSettings.Save (csave);
		csave.End_Chunk ();
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
SoundNodeClass::Load (ChunkLoadClass &cload)
{
	//
	//	Go straight to the base class if we need to suport
	// legacy loads
	//
	if (UseLegacyLoad) {
		return NodeClass::Load (cload);
	}

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				NodeClass::Load (cload);
				break;

			case CHUNKID_INSTANCE_SETTINGS:
				InstanceSettings.Load (cload);
				OverridePreset = true;
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}
