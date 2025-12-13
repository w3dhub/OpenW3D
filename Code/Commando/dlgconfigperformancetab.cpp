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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgconfigperformancetab.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/08/02 3:03p                                                $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlgconfigperformancetab.h"
#include "registry.h"
#include "comboboxctrl.h"
#include "sliderctrl.h"
#include "rendobj.h"
#include "phys.h"
#include "ww3d.h"
#include "pscene.h"
#include "combat.h"
#include "dx8wrapper.h"
#include "dx8caps.h"
#include "surfaceeffects.h"
#include "_globals.h"
#include "translatedb.h"
#include "string_ids.h"
#include "ini.h"
#include "openw3d.h"
#include <algorithm>


/////////////////////////////////////////////////////////////////////////////
// Structures and typedefs
/////////////////////////////////////////////////////////////////////////////
typedef struct _PERFORMANCE_SETTING
{
	int ctrl_id;
	int value;
} PERFORMANCE_SETTING;


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

const int MAX_PERFORMANCE_LEVELS	= 4;
const int MAX_EXPERT_OPTIONS		= 7;

PERFORMANCE_SETTING _PerformanceLevels[MAX_PERFORMANCE_LEVELS][MAX_EXPERT_OPTIONS] = 
{
	//
	//	Low detail
	//
	{
		{IDC_CHAR_SHADOWS_SLIDER, 0},
		{IDC_TEXTURE_DETAIL_SLIDER, 0},
		{IDC_PARTICLE_DETAIL_SLIDER, 0},
		{IDC_SURFACE_DETAIL_SLIDER, 0},
		{IDC_GEOMETRY_DETAIL_SLIDER, 0},
		{IDC_TERRAIN_SHADOW_CHECK, 0},
		{IDC_NPATCH_CHECK, 0}
	},

	{
		{IDC_CHAR_SHADOWS_SLIDER, 1},
		{IDC_TEXTURE_DETAIL_SLIDER, 1},
		{IDC_PARTICLE_DETAIL_SLIDER, 0},
		{IDC_SURFACE_DETAIL_SLIDER, 0},
		{IDC_GEOMETRY_DETAIL_SLIDER, 0},
		{IDC_TERRAIN_SHADOW_CHECK, 0},
		{IDC_NPATCH_CHECK, 0}
	},

	{
		{IDC_CHAR_SHADOWS_SLIDER, 2},
		{IDC_TEXTURE_DETAIL_SLIDER, 2},
		{IDC_PARTICLE_DETAIL_SLIDER, 1},
		{IDC_SURFACE_DETAIL_SLIDER, 1},
		{IDC_GEOMETRY_DETAIL_SLIDER, 1},
		{IDC_TERRAIN_SHADOW_CHECK, 1},
		{IDC_NPATCH_CHECK, 0}
	},

	//
	//	High detail
	//
	{
		{IDC_CHAR_SHADOWS_SLIDER, 3},
		{IDC_TEXTURE_DETAIL_SLIDER, 2},
		{IDC_PARTICLE_DETAIL_SLIDER, 2},
		{IDC_SURFACE_DETAIL_SLIDER, 2},
		{IDC_GEOMETRY_DETAIL_SLIDER, 2},
		{IDC_TERRAIN_SHADOW_CHECK, 1},
		{IDC_NPATCH_CHECK, 0}
	},
};

const char *VALUE_NAME_DYN_LOD			= "Dynamic_LOD_Budget";
const char *VALUE_NAME_STATIC_LOD		= "Static_LOD_Budget";
const char *VALUE_NAME_DYN_SHADOWS		= "Dynamic_Projectors";
const char *VALUE_NAME_SHADOW_MODE		= "Shadow_Mode";
const char *VALUE_NAME_STATIC_SHADOWS	= "Static_Projectors";
const char *VALUE_NAME_TEXTURE_RES		= "Texture_Resolution";
const char *VALUE_NAME_PARTICLE_DETAIL	= "Particle_Detail";
const char *VALUE_NAME_NPATCHES			= "NPatches";


const char *VALUE_INI_DYN_LOD			= "DynamicLODBudget";
const char *VALUE_INI_STATIC_LOD		= "StaticLODBudget";
const char *VALUE_INI_DYN_SHADOWS		= "DynamicProjectors";
const char *VALUE_INI_SHADOW_MODE		= "ShadowMode";
const char *VALUE_INI_STATIC_SHADOWS	= "StaticProjectors";
const char *VALUE_INI_TEXTURE_RES		= "TextureResolution";
const char *VALUE_INI_PARTICLE_DETAIL	= "ParticleDetail";
const char *VALUE_INI_NPATCHES			= "NPatches";

const int MAX_LOD_HIGH	= 10000;
const int MAX_LOD_MED	= 5000;
const int MAX_LOD_LOW	= 0;


////////////////////////////////////////////////////////////////
//
//	DlgConfigPerformanceTabClass
//
////////////////////////////////////////////////////////////////
DlgConfigPerformanceTabClass::DlgConfigPerformanceTabClass (void)	:
	ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CONFIG_PERFORMANCE))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DlgConfigPerformanceTabClass
//
////////////////////////////////////////////////////////////////
DlgConfigPerformanceTabClass::~DlgConfigPerformanceTabClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::On_Init_Dialog (void)
{
	//
	//	Configure the dialog controls
	//
	Setup_Controls ();
	Load_Values ();
	Determine_Performance_Setting ();

	//
	//	Set the visibility of the expert windows controls
	//
	Build_Expert_Window_List ();
	Display_Expert_Settings (false);

	ChildDialogClass::On_Init_Dialog ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Build_Expert_Window_List
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::Build_Expert_Window_List (void)
{
	ExpertWindowList.Delete_All ();

	//
	//	Find the y-position of the expert mode checkbox
	//
	RectClass rect = Get_Dlg_Item (IDC_EXPERT_CHECK)->Get_Window_Rect ();
	float y_pos = rect.Top;

	//
	//	Loop over all the controls of the dialog
	//
	for (	int index = 0; index < Get_Control_Count (); index ++) {
		DialogControlClass *control = Get_Control (index);

		//
		//	If this control is below the expert checkbox then its
		// part of the expert settings
		//
		rect = control->Get_Window_Rect ();
		if (rect.Top > y_pos) {
			ExpertWindowList.Add (control);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Display_Expert_Settings
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::Display_Expert_Settings (bool onoff)
{
	//
	//	Loop over all the expert settings controls and either
	// show or hide them...
	//
	for (int index = 0; index < ExpertWindowList.Count (); index ++) {
		ExpertWindowList[index]->Show (onoff);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Setup_Controls
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::Setup_Controls (void)
{
	//
	//	Configure the main performance slider
	//
	SliderCtrlClass *performance_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_PERFORMANCE_SLIDER);
	performance_slider->Set_Range (0, MAX_PERFORMANCE_LEVELS - 1);

	//
	//	Configure the individual detail-setting sliders
	//
	((SliderCtrlClass *)Get_Dlg_Item (IDC_GEOMETRY_DETAIL_SLIDER))->Set_Range (0, 2);
	((SliderCtrlClass *)Get_Dlg_Item (IDC_CHAR_SHADOWS_SLIDER))->Set_Range (0, 3);
	((SliderCtrlClass *)Get_Dlg_Item (IDC_TEXTURE_DETAIL_SLIDER))->Set_Range (0, 2);
	((SliderCtrlClass *)Get_Dlg_Item (IDC_SURFACE_DETAIL_SLIDER))->Set_Range (0, 2);
	((SliderCtrlClass *)Get_Dlg_Item (IDC_PARTICLE_DETAIL_SLIDER))->Set_Range (0, 2);

	const wchar_t *PRELIT_MODE_NAMES[] =
	{
		TRANSLATE (IDS_MENU_VERTEX),
		TRANSLATE (IDS_MENU_MP_LIGHTMAPS),
		TRANSLATE (IDS_MENU_MT_LIGHTMAPS)
	};
	
	Set_Dlg_Item_Text (IDC_LIGHTING_MODE, PRELIT_MODE_NAMES [WW3D::Get_Prelit_Mode()]);
	Enable_Dlg_Item (IDC_LIGHTING_MODE,	false);

	const wchar_t *TEXTURE_FILTER_NAMES[] =
	{
		TRANSLATE (IDS_BILINEAR),
		TRANSLATE (IDS_TRILINEAR),
		TRANSLATE (IDS_ANISOTROPIC)
	};

	Set_Dlg_Item_Text (IDC_TEXTURE_FILTER, TEXTURE_FILTER_NAMES [WW3D::Get_Texture_Filter()]);
	Enable_Dlg_Item (IDC_TEXTURE_FILTER, false);

	//
	//	Disable the checkbox if NPatches aren't supported
	//
	if (DX8Wrapper::Get_Current_Caps() && DX8Wrapper::Get_Current_Caps()->Support_NPatches () == false) {
		Check_Dlg_Button (IDC_NPATCH_CHECK, false);
		Enable_Dlg_Item (IDC_NPATCH_CHECK, false);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Load_Values
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::Load_Values (void)
{
	SliderCtrlClass *geometry_slider			= (SliderCtrlClass *)Get_Dlg_Item (IDC_GEOMETRY_DETAIL_SLIDER);
	SliderCtrlClass *char_shadows_slider	= (SliderCtrlClass *)Get_Dlg_Item (IDC_CHAR_SHADOWS_SLIDER);
	SliderCtrlClass *texture_slider			= (SliderCtrlClass *)Get_Dlg_Item (IDC_TEXTURE_DETAIL_SLIDER);
	SliderCtrlClass *surface_effect_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_SURFACE_DETAIL_SLIDER);
	SliderCtrlClass *particle_slider			= (SliderCtrlClass *)Get_Dlg_Item (IDC_PARTICLE_DETAIL_SLIDER);

	//
	//	Attempt to open the config file
	//
	INIClass ini(W3D_CONF_FILE);
	if (ini.Is_Present(W3D_SECTION_SYSTEM)) {

		//
		//	Read the values from the config file
		//
		int static_shadows = 1;
		int shadow_mode = PhysicsSceneClass::SHADOW_MODE_BLOBS_PLUS;
		int texture_red = 0;
		int particle_detail = 1;
		int npatches = 0;

		static_shadows	= ini.Get_Bool (W3D_SECTION_SYSTEM, VALUE_INI_STATIC_SHADOWS, static_shadows != 0);
		shadow_mode		= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_SHADOW_MODE, shadow_mode);
		texture_red		= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_RES, texture_red);
		particle_detail	= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_PARTICLE_DETAIL, particle_detail);
		npatches		= ini.Get_Bool (W3D_SECTION_SYSTEM, VALUE_INI_NPATCHES, npatches);


		//
		//	Get the surface effect mode
		//
		int surface_effect = SurfaceEffectsManager::Get_Mode ();

		//
		//	Get the LOD settings
		//
		int dynamic_lod	= 0;
		int static_lod		= 0;
		PhysicsSceneClass::Get_Instance()->Get_Polygon_Budgets (&static_lod, &dynamic_lod);

		//
		//	Set the slider's positions to reflect the loaded values
		//
		char_shadows_slider->Set_Pos (std::min (shadow_mode, 3));
		texture_slider->Set_Pos (std::max (2 - texture_red, 0));
		surface_effect_slider->Set_Pos (surface_effect);
		particle_slider->Set_Pos (particle_detail);

		//
		//	Choose a setting for the geometry slider based on
		// the dynamic and static LOD budgets
		//
		if (dynamic_lod < (MAX_LOD_LOW + 1000)) {
			geometry_slider->Set_Pos (0);
		} else if (dynamic_lod <= MAX_LOD_MED) {
			geometry_slider->Set_Pos (1);
		} else {
			geometry_slider->Set_Pos (2);
		}

		//
		//	Check the checkbox controls (if necessary)
		//
		Check_Dlg_Button (IDC_TERRAIN_SHADOW_CHECK, bool(static_shadows != 0));
		Check_Dlg_Button (IDC_NPATCH_CHECK, bool(npatches != 0));
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Determine_Performance_Setting
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::Determine_Performance_Setting (void)
{
	float option_levels[MAX_EXPERT_OPTIONS] = { 0 };
	int count_per_option[MAX_EXPERT_OPTIONS] = { 0 };

	//
	//	Get the current values of each setting
	//
	DynamicVectorClass<int> option_values;
	Get_Settings (option_values);

	//
	// Find out which level each option is currently set at
	//
	for (int level = MAX_PERFORMANCE_LEVELS - 1; level >= 0; level --) {
		for (int index = 0; index < MAX_EXPERT_OPTIONS; index ++) {

			//
			//	If the values match, then record the current level for this option
			//
			if (option_values[index] == _PerformanceLevels[level][index].value) {
				option_levels[index] += level;
				count_per_option[index] ++;
			}
		}
	}

	//
	//	Add up all the levels
	//
	float level_sum = 0;
	for (int index = 0; index < MAX_EXPERT_OPTIONS; index ++) {
		level_sum += (option_levels[index] / count_per_option[index]);
	}

	//
	//	Take the average rating
	//
	float level_rating = level_sum / MAX_EXPERT_OPTIONS;

	//
	//	Set the slider's position
	//
	SliderCtrlClass *slider_ctrl	= (SliderCtrlClass *)Get_Dlg_Item (IDC_PERFORMANCE_SLIDER);
	slider_ctrl->Set_Pos (std::min (int(level_rating + 0.4F), MAX_EXPERT_OPTIONS - 1), false);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_EXPERT_CHECK:
		{
			bool enabled = Is_Dlg_Button_Checked (IDC_EXPERT_CHECK);
			Display_Expert_Settings (enabled);
			break;
		}
	}

	ChildDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Settings
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::Get_Settings (DynamicVectorClass<int> &settings)
{
	//
	//	Determine what the value of each control is
	//
	for (int index = 0; index < MAX_EXPERT_OPTIONS; index ++) {

		//
		//	Determine what the value of this control is
		//
		int curr_value = 0;
		int ctrl_id = _PerformanceLevels[0][index].ctrl_id;
		switch (ctrl_id) {

			//
			//	Read any of the slider positions
			//
			case IDC_GEOMETRY_DETAIL_SLIDER:
			case IDC_CHAR_SHADOWS_SLIDER:
			case IDC_TEXTURE_DETAIL_SLIDER:
			case IDC_PARTICLE_DETAIL_SLIDER:
			case IDC_SURFACE_DETAIL_SLIDER:
			{
				SliderCtrlClass *slider_ctrl = (SliderCtrlClass *)Get_Dlg_Item (ctrl_id);
				curr_value = slider_ctrl->Get_Pos ();
				break;
			}

			//
			//	Read the values from either checkbox
			//
			case IDC_TERRAIN_SHADOW_CHECK:
			case IDC_NPATCH_CHECK:
				curr_value = Is_Dlg_Button_Checked (ctrl_id);
				break;
		}

		//
		//	Add this value to the list
		//
		settings.Add (curr_value);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Expert_Controls
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::Update_Expert_Controls (int level)
{
	for (int index = 0; index < MAX_EXPERT_OPTIONS; index ++) {

		//
		//	Determine which control we are updating
		//
		int ctrl_id = _PerformanceLevels[level][index].ctrl_id;
		switch (ctrl_id) {

			//
			//	Update any of the sliders via the same mechanism
			//
			case IDC_GEOMETRY_DETAIL_SLIDER:
			case IDC_CHAR_SHADOWS_SLIDER:
			case IDC_TEXTURE_DETAIL_SLIDER:
			case IDC_PARTICLE_DETAIL_SLIDER:
			case IDC_SURFACE_DETAIL_SLIDER:
			{
				SliderCtrlClass *slider_ctrl = (SliderCtrlClass *)Get_Dlg_Item (ctrl_id);
				slider_ctrl->Set_Pos (_PerformanceLevels[level][index].value);
				break;
			}

			//
			//	Update any of the check boxes via the same mechanism
			//
			case IDC_TERRAIN_SHADOW_CHECK:
			case IDC_NPATCH_CHECK:
				Check_Dlg_Button (ctrl_id, bool(_PerformanceLevels[level][index].value == 1));
				break;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_SliderCtrl_Pos_Changed
//
////////////////////////////////////////////////////////////////
void
DlgConfigPerformanceTabClass::On_SliderCtrl_Pos_Changed
(
	SliderCtrlClass *	slider_ctrl,
	int					ctrl_id,
	int					new_pos
)
{
	if (ctrl_id == IDC_PERFORMANCE_SLIDER) {
		Update_Expert_Controls (new_pos);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// On_Apply
//
/////////////////////////////////////////////////////////////////////////////
bool
DlgConfigPerformanceTabClass::On_Apply (void)
{
	SliderCtrlClass *geometry_slider			= (SliderCtrlClass *)Get_Dlg_Item (IDC_GEOMETRY_DETAIL_SLIDER);
	SliderCtrlClass *char_shadows_slider	= (SliderCtrlClass *)Get_Dlg_Item (IDC_CHAR_SHADOWS_SLIDER);
	SliderCtrlClass *texture_slider			= (SliderCtrlClass *)Get_Dlg_Item (IDC_TEXTURE_DETAIL_SLIDER);
	SliderCtrlClass *surface_effect_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_SURFACE_DETAIL_SLIDER);
	SliderCtrlClass *particle_slider			= (SliderCtrlClass *)Get_Dlg_Item (IDC_PARTICLE_DETAIL_SLIDER);
	
	//
	//	Attempt to open the config file
	//
	INIClass ini(W3D_CONF_FILE);
	
	//
	//	Get the current settings from the dialog
	//
	int geometry_detail	= geometry_slider->Get_Pos ();
	int shadow_mode		= char_shadows_slider->Get_Pos ();
	int texture_red		= texture_slider->Get_Pos ();
	int surface_effect	= surface_effect_slider->Get_Pos ();
	int particle_detail	= particle_slider->Get_Pos ();
	int static_shadows	= Is_Dlg_Button_Checked (IDC_TERRAIN_SHADOW_CHECK);
	int npatches			= Is_Dlg_Button_Checked (IDC_NPATCH_CHECK);

	//
	//	Determine a good LOD budget to use
	//
	int lod_budget = 0;
	if (geometry_detail == 0) {
		lod_budget = 0;
	} else if (geometry_detail == 1) {
		lod_budget = MAX_LOD_MED;
	} else if (geometry_detail == 2) {
		lod_budget = MAX_LOD_HIGH;
	}

	//
	//	Store the values in the config file
	//
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_DYN_LOD, lod_budget);
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_STATIC_LOD, lod_budget);

	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_DYN_SHADOWS, (shadow_mode != PhysicsSceneClass::SHADOW_MODE_NONE));
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_STATIC_SHADOWS, static_shadows);

	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_SHADOW_MODE,		shadow_mode);
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_RES,		std::max (2 - texture_red, 0));
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_PARTICLE_DETAIL, particle_detail);



	if (DX8Wrapper::Get_Current_Caps() && DX8Wrapper::Get_Current_Caps()->Support_NPatches ()) {
		ini.Put_Bool (W3D_SECTION_SYSTEM, VALUE_INI_NPATCHES,	npatches != 0);
	}

	//
	//	Pass the values onto the game
	//
	COMBAT_SCENE->Set_Polygon_Budgets (lod_budget, lod_budget);
	COMBAT_SCENE->Enable_Dynamic_Projectors ((shadow_mode != PhysicsSceneClass::SHADOW_MODE_NONE));
	COMBAT_SCENE->Enable_Static_Projectors ((static_shadows != 0));
	// Note! It is important to invalidate all textures when
	// changing the amount of render targets, as render target
	// creation may have problems if the card is running low on
	// texture memory!
	if (COMBAT_SCENE->Get_Shadow_Mode()!=(PhysicsSceneClass::ShadowEnum)shadow_mode) {
		WW3D::_Invalidate_Textures();
		COMBAT_SCENE->Set_Shadow_Mode ((PhysicsSceneClass::ShadowEnum)shadow_mode);
	}
	WW3D::Set_Texture_Reduction (std::max (2 - texture_red, 0));
	SurfaceEffectsManager::Set_Mode ((SurfaceEffectsManager::MODE)surface_effect);
	ini.Save(W3D_CONF_FILE);
	return true;
}
