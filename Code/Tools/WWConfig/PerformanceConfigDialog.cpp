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

// PerformanceConfigDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "WWConfig.h"
#include "PerformanceConfigDialog.h"
#include "registry.h"
#include "ww3d.h"
#include "pscene.h"
#include "rendobj.h"
#include "phys.h"
#include "videoconfigdialog.h"
#include "dx8caps.h"
#include "cpudetect.h"
#include "formconv.h"

#include "dx8wrapper.h"
#include "locale_api.h"
#include "wwconfig_ids.h"
#include "../../Combat/specialbuilds.h"

#include "ini.h"
#include "openw3d.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static bool CanDoMultiPass=true;

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
const int MAX_EXPERT_OPTIONS		= 8;

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
		{IDC_TEXTURE_FILTER_COMBO, TextureClass::TEXTURE_FILTER_BILINEAR},
		{IDC_LIGHTING_MODE_COMBO, WW3D::PRELIT_MODE_VERTEX},
		{IDC_TERRAIN_SHADOW_CHECK, 0}
	},

	{
		{IDC_CHAR_SHADOWS_SLIDER, 1},
		{IDC_TEXTURE_DETAIL_SLIDER, 1},
		{IDC_PARTICLE_DETAIL_SLIDER, 0},
		{IDC_SURFACE_DETAIL_SLIDER, 0},
		{IDC_GEOMETRY_DETAIL_SLIDER, 0},
		{IDC_TEXTURE_FILTER_COMBO, TextureClass::TEXTURE_FILTER_BILINEAR},
		{IDC_LIGHTING_MODE_COMBO, WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE},
		{IDC_TERRAIN_SHADOW_CHECK, 0}
	},

	{
		{IDC_CHAR_SHADOWS_SLIDER, 2},
		{IDC_TEXTURE_DETAIL_SLIDER, 2},
		{IDC_PARTICLE_DETAIL_SLIDER, 1},
		{IDC_SURFACE_DETAIL_SLIDER, 1},
		{IDC_GEOMETRY_DETAIL_SLIDER, 1},
		{IDC_TEXTURE_FILTER_COMBO, TextureClass::TEXTURE_FILTER_TRILINEAR},
		{IDC_LIGHTING_MODE_COMBO, WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE},
		{IDC_TERRAIN_SHADOW_CHECK, 1}
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
		{IDC_TEXTURE_FILTER_COMBO, TextureClass::TEXTURE_FILTER_TRILINEAR},
		{IDC_LIGHTING_MODE_COMBO, WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE},
		{IDC_TERRAIN_SHADOW_CHECK, 1}
	},
};


/////////////////////////////////////////////////////////////////////////////
// Registry Values
/////////////////////////////////////////////////////////////////////////////


/*
#ifdef MULTIPLAYERDEMO
const char *KEY_NAME_SETTINGS				= "Software\\Westwood\\RenegadeMPDemo\\System Settings";
const char *KEY_NAME_OPTIONS				= "Software\\Westwood\\RenegadeMPDemo\\Options";
#else
const char *KEY_NAME_SETTINGS				= "Software\\Westwood\\Renegade\\System Settings";
const char *KEY_NAME_OPTIONS				= "Software\\Westwood\\Renegade\\Options";
#endif // MULTIPLAYERDEMO
*/

#if	defined(FREEDEDICATEDSERVER)
const char *KEY_NAME_SETTINGS				= "Software\\Westwood\\RenegadeFDS\\System Settings";
const char *KEY_NAME_OPTIONS				= "Software\\Westwood\\RenegadeFDS\\Options";
#elif defined(MULTIPLAYERDEMO)
const char *KEY_NAME_SETTINGS				= "Software\\Westwood\\RenegadeMPDemo\\System Settings";
const char *KEY_NAME_OPTIONS				= "Software\\Westwood\\RenegadeMPDemo\\Options";
#elif defined(BETACLIENT)
const char *KEY_NAME_SETTINGS				= "Software\\Westwood\\RenegadeBeta\\System Settings";
const char *KEY_NAME_OPTIONS				= "Software\\Westwood\\RenegadeBeta\\Options";
#else
const char *KEY_NAME_SETTINGS				= "Software\\Westwood\\Renegade\\System Settings";
const char *KEY_NAME_OPTIONS				= "Software\\Westwood\\Renegade\\Options";
#endif

const char *VALUE_NAME_DYN_LOD			= "Dynamic_LOD_Budget";
const char *VALUE_NAME_STATIC_LOD		= "Static_LOD_Budget";
const char *VALUE_NAME_DYN_SHADOWS		= "Dynamic_Projectors";
const char *VALUE_NAME_TEXTURE_FILTER	= "Texture_Filter_Mode";
const char *VALUE_NAME_PRELIT_MODE		= "Prelit_Mode";
const char *VALUE_NAME_SHADOW_MODE		= "Shadow_Mode";
const char *VALUE_NAME_STATIC_SHADOWS	= "Static_Projectors";
const char *VALUE_NAME_TEXTURE_RES		= "Texture_Resolution";
const char *VALUE_NAME_SURFACE_EFFECT	= "Surface_Effect_Detail";
const char *VALUE_NAME_PARTICLE_DETAIL	= "Particle_Detail";

const char *VALUE_INI_DYN_LOD			= "DynamicLODBudget";
const char *VALUE_INI_STATIC_LOD		= "StaticLODBudget";
const char *VALUE_INI_DYN_SHADOWS		= "DynamicProjectors";
const char *VALUE_INI_TEXTURE_FILTER	= "TextureFilterMode";
const char *VALUE_INI_PRELIT_MODE		= "PrelitMode";
const char *VALUE_INI_SHADOW_MODE		= "ShadowMode";
const char *VALUE_INI_STATIC_SHADOWS	= "StaticProjectors";
const char *VALUE_INI_TEXTURE_RES		= "TextureResolution";
const char *VALUE_INI_SURFACE_EFFECT	= "SurfaceEffectDetail";
const char *VALUE_INI_PARTICLE_DETAIL	= "ParticleDetail";


/////////////////////////////////////////////////////////////////////////////
//
// PerformanceConfigDialogClass
//
/////////////////////////////////////////////////////////////////////////////
PerformanceConfigDialogClass::PerformanceConfigDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog (PerformanceConfigDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(PerformanceConfigDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	CDialog::Create (PerformanceConfigDialogClass::IDD, pParent);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PerformanceConfigDialogClass)
	DDX_Control(pDX, IDC_GEOMETRY_DETAIL_SLIDER, m_GeometrySlider);
	DDX_Control(pDX, IDC_CHAR_SHADOWS_SLIDER, m_CharShadowsSlider);
	DDX_Control(pDX, IDC_TEXTURE_DETAIL_SLIDER, m_TextureDetailSlider);
	DDX_Control(pDX, IDC_SURFACE_DETAIL_SLIDER, m_SurfaceEffectsSlider);
	DDX_Control(pDX, IDC_PERFORMANCE_SLIDER, m_PerformanceSlider);
	DDX_Control(pDX, IDC_PARTICLE_DETAIL_SLIDER, m_ParticleSlider);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PerformanceConfigDialogClass, CDialog)
	//{{AFX_MSG_MAP(PerformanceConfigDialogClass)
	ON_BN_CLICKED(IDC_EXPERT_CHECK, OnExpertCheck)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_AUTO_CONFIG_BUTTON, OnGraphicsAutoSetup)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
//	Modified: 12/06/2001 by	MML	- Retrieving strings from Locomoto file.
//
/////////////////////////////////////////////////////////////////////////////
BOOL
PerformanceConfigDialogClass::OnInitDialog (void)
{
	char string [_MAX_PATH];

	CDialog::OnInitDialog ();	

	//
	// Set all the static strings for this dialog.
	//
	SetDlgItemText( IDC_DETAIL,			 Locale_GetString( IDS_DETAIL, string ));
	SetDlgItemText( IDC_LOW_DESC,			 Locale_GetString( IDS_LOW_DESC, string ));
	SetDlgItemText( IDC_HIGH_DESC,		 Locale_GetString( IDS_HIGH_DESC, string ));
	SetDlgItemText( IDC_EXPERT_CHECK,	 Locale_GetString( IDS_EXPERT_MODE, string ));
	SetDlgItemText( IDC_EXPERT_SETTINGS, Locale_GetString( IDS_EXPERT_SETTINGS, string ));
	
	SetDlgItemText( IDC_AUTO_CONFIG_BUTTON, Locale_GetString( IDS_AUTOCONFIG, string ));
	
	SetDlgItemText( IDC_GEOMETRY_DETAIL,		 Locale_GetString( IDS_GEOMETRY_DETAIL, string ));
	SetDlgItemText( IDC_CHARACTER_SHADOWS,		 Locale_GetString( IDS_CHARACTER_SHADOWS, string ));
	SetDlgItemText( IDC_TEXTURE_DETAIL,			 Locale_GetString( IDS_TEXTURE_DETAIL, string ));
	SetDlgItemText( IDC_PARTICLE_DETAIL,		 Locale_GetString( IDS_PARTICLE_DETAIL, string ));
	SetDlgItemText( IDC_SURFACE_EFFECT_DETAIL, Locale_GetString( IDS_SURFACE_EFFECT_DETAIL, string ));

	Locale_GetString( IDS_LOW, string );
	SetDlgItemText( IDC_LOW1, string );
	SetDlgItemText( IDC_LOW2, string );
	SetDlgItemText( IDC_LOW3, string );
	SetDlgItemText( IDC_LOW4, string );
	SetDlgItemText( IDC_LOW5, string );

	Locale_GetString( IDS_HIGH, string );
	SetDlgItemText( IDC_HIGH1,	string );
	SetDlgItemText( IDC_HIGH2,	string );
	SetDlgItemText( IDC_HIGH3,	string );
	SetDlgItemText( IDC_HIGH4,	string );
	SetDlgItemText( IDC_HIGH5,	string );
	
	SetDlgItemText( IDC_LIGHTING_MODE,	Locale_GetString( IDS_LIGHTING_MODE, string ));
	SetDlgItemText( IDC_TEXTURE_FILTER,	Locale_GetString( IDS_TEXTURE_FILTER, string ));

	SetDlgItemText( IDC_TERRAIN_SHADOW_CHECK,	Locale_GetString( IDS_TERRAIN_SHADOWS, string ));

	//
	//	Configure the dialog controls
	//
	Setup_Controls ();
	Load_Values ();
//	Determine_Performance_Setting ();

	//
	//	Set the visibility of the expert windows controls
	//
	Build_Expert_Window_List ();
	Display_Expert_Settings (false);	

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Build_Expert_Window_List
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::Build_Expert_Window_List (void)
{
	ExpertWindowList.Delete_All ();

	//
	//	Find the y-position of the expert mode checkbox
	//
	CRect rect;	
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_EXPERT_CHECK), &rect);
	float y_pos = rect.top;

	//
	//	Loop over all the child windows of the dialog
	//
	for (	HWND child_wnd = ::GetWindow (m_hWnd, GW_CHILD);
			child_wnd != NULL;
			child_wnd = ::GetWindow (child_wnd, GW_HWNDNEXT))
	{		
		//
		//	If this child window is below the expert checkbox then its
		// part of the expert settings
		//
		::GetWindowRect (child_wnd, &rect);
		if (rect.top > y_pos) {
			ExpertWindowList.Add (child_wnd);
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
PerformanceConfigDialogClass::Display_Expert_Settings (bool onoff)
{
	//
	//	Loop over all the expert settings windows and either
	// show or hide them...
	//
	for (int index = 0; index < ExpertWindowList.Count (); index ++) {
		::ShowWindow (ExpertWindowList[index], onoff ? SW_SHOW : SW_HIDE);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnExpertCheck
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::OnExpertCheck (void)
{
	bool enabled = bool(SendDlgItemMessage (IDC_EXPERT_CHECK, BM_GETCHECK) == 1);
	Display_Expert_Settings (enabled);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Setup_Controls
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::Setup_Controls (void)
{
	//
	//	Configure the main performance slider
	//
	m_PerformanceSlider.SetRange (0, MAX_PERFORMANCE_LEVELS - 1);

	//
	//	Configure the individual detail-setting sliders
	//
	m_CharShadowsSlider.SetRange (0, 3);
	m_GeometrySlider.SetRange (0, 2);
	m_TextureDetailSlider.SetRange (0, 2);
	m_SurfaceEffectsSlider.SetRange (0, 2);
	m_ParticleSlider.SetRange (0, 2);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Load_Values
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::Load_Values (void)
{
	//
	//	Attempt to open the registry key 
	//
	RegistryClass registry (KEY_NAME_SETTINGS);
	INIClass ini(W3D_CONF_FILE);
	int dynamic_lod = 3000;
	int static_lod = 3000;
	int dynamic_shadows = 1;
	int static_shadows = 1;
	int prelit_mode = WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE;
	int texture_filter = TextureClass::TEXTURE_FILTER_BILINEAR;
	int shadow_mode = PhysicsSceneClass::SHADOW_MODE_BLOBS_PLUS;		
	int texture_red = 0;
	int surface_effect = 1;
	int particle_detail = 1;
	
	if(ini.Is_Present(W3D_SECTION_SYSTEM))
	{
		//
		//	Read the values from the ini
		//
		dynamic_lod		= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_DYN_LOD, 3000);
		static_lod			= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_STATIC_LOD, 3000);

		dynamic_shadows	= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_DYN_SHADOWS, 1);
		static_shadows	= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_STATIC_SHADOWS, 1);

		prelit_mode		= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_PRELIT_MODE, WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
		texture_filter	= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_FILTER, TextureClass::TEXTURE_FILTER_BILINEAR);
		shadow_mode		= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_SHADOW_MODE, PhysicsSceneClass::SHADOW_MODE_BLOBS_PLUS);		
		texture_red		= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_RES, 0);
		surface_effect	= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_SURFACE_EFFECT, 1);
		particle_detail	= ini.Get_Int (W3D_SECTION_SYSTEM, VALUE_INI_PARTICLE_DETAIL, 1);
	} else if (registry.Is_Valid ()) {

		//
		//	Read the values from the registry
		//
		dynamic_lod		= registry.Get_Int (VALUE_NAME_DYN_LOD, 3000);
		static_lod			= registry.Get_Int (VALUE_NAME_STATIC_LOD, 3000);

		dynamic_shadows	= registry.Get_Int (VALUE_NAME_DYN_SHADOWS, 1);
		static_shadows	= registry.Get_Int (VALUE_NAME_STATIC_SHADOWS, 1);

		prelit_mode		= registry.Get_Int (VALUE_NAME_PRELIT_MODE, WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
		texture_filter	= registry.Get_Int (VALUE_NAME_TEXTURE_FILTER, TextureClass::TEXTURE_FILTER_BILINEAR);
		shadow_mode		= registry.Get_Int (VALUE_NAME_SHADOW_MODE, PhysicsSceneClass::SHADOW_MODE_BLOBS_PLUS);		
		texture_red		= registry.Get_Int (VALUE_NAME_TEXTURE_RES, 0);
		surface_effect	= registry.Get_Int (VALUE_NAME_SURFACE_EFFECT, 1);
		particle_detail	= registry.Get_Int (VALUE_NAME_PARTICLE_DETAIL, 1);
	}

	//
	//	Set the slider's positions to reflect the loaded values
	//
	m_CharShadowsSlider.SetPos (std::min (shadow_mode, 3));
	m_TextureDetailSlider.SetPos (std::max (2 - texture_red, 0));		
	m_SurfaceEffectsSlider.SetPos (surface_effect);
	m_ParticleSlider.SetPos (particle_detail);

	//
	//	Choose a setting for the geometry slider based on
	// the dynamic and static LOD budgets
	//
	if (dynamic_lod < 1000 && static_lod < 1000) {
		m_GeometrySlider.SetPos (0);
	} else if (dynamic_lod <= 5000 && static_lod <= 5000) {
		m_GeometrySlider.SetPos (1);
	} else {
		m_GeometrySlider.SetPos (2);
	}

	//
	//	Check the checkbox controls (if necessary)
	//
	SendDlgItemMessage (IDC_TERRAIN_SHADOW_CHECK, BM_SETCHECK, (WPARAM)(static_shadows != 0));
	
	//
	//	Select the correct setting from the lighting mode combo box
	//
	SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_SETCURSEL, prelit_mode);		

	//
	//	Select the correct setting from the texture filtering mode combo box
	//
	SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_SETCURSEL, texture_filter);		

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Determine_Performance_Setting
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::Determine_Performance_Setting (void)
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
	m_PerformanceSlider.SetPos (std::min (int(level_rating + 0.5F), MAX_EXPERT_OPTIONS - 1));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Settings
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::Get_Settings (DynamicVectorClass<int> &settings)
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
				curr_value = SendDlgItemMessage (ctrl_id, TBM_GETPOS);
				break;

			//
			//	Read the values from the checkbox
			//
			case IDC_TERRAIN_SHADOW_CHECK:
				curr_value = SendDlgItemMessage (ctrl_id, BM_GETCHECK);
				break;

			//
			//	Read a value from a combo box.
			//
			case IDC_LIGHTING_MODE_COMBO:
			case IDC_TEXTURE_FILTER_COMBO:
				curr_value = SendDlgItemMessage (ctrl_id, CB_GETCURSEL);
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
PerformanceConfigDialogClass::Update_Expert_Controls (int level)
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
				SendDlgItemMessage (ctrl_id, TBM_SETPOS, true, _PerformanceLevels[level][index].value);
				break;

			//
			//	Update any of the check boxes via the same mechanism
			//
			case IDC_TERRAIN_SHADOW_CHECK:
				SendDlgItemMessage (ctrl_id, BM_SETCHECK, _PerformanceLevels[level][index].value);
				break;

			//
			//	Update any of the combo boxes.
			//
			case IDC_LIGHTING_MODE_COMBO:
			case IDC_TEXTURE_FILTER_COMBO:
				SendDlgItemMessage (ctrl_id, CB_SETCURSEL, _PerformanceLevels[level][index].value);
				break;

		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnHScroll
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::OnHScroll
(
	UINT				nSBCode,
	UINT				nPos,
	CScrollBar *	pScrollBar
)
{
	int ctrl_id = ::GetWindowLong (pScrollBar->m_hWnd, GWL_ID);
	
	//
	//	If this is the overall performance slider, then
	// update the expert controls to reflect the new setting
	//
	if (ctrl_id == IDC_PERFORMANCE_SLIDER) {
		int new_level = m_PerformanceSlider.GetPos ();
		Update_Expert_Controls (new_level);
	}

	CDialog::OnHScroll (nSBCode, nPos, pScrollBar);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::Apply_Changes (void)
{
	//
	//	Get the current settings from the dialog
	//
	int geometry_detail	= 	m_GeometrySlider.GetPos ();
	int shadow_mode		= 	m_CharShadowsSlider.GetPos ();
	int texture_red		= 	m_TextureDetailSlider.GetPos ();
	int surface_effect	= 	m_SurfaceEffectsSlider.GetPos ();
	int particle_detail	= 	m_ParticleSlider.GetPos ();
	int static_shadows	= SendDlgItemMessage (IDC_TERRAIN_SHADOW_CHECK, BM_GETCHECK);		
	int prelit_mode		= SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_GETCURSEL);
	// If card can't do multi-pass, value 1 means multi-texture (multi-pass selection is missing from the combo box)
	if (!CanDoMultiPass) {
		if (prelit_mode==1) prelit_mode=WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE;
	}

	int texture_filter	= SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_GETCURSEL);

	//
	//	Determine a good LOD budget to use
	//
	int lod_budget = 0;
	if (geometry_detail == 0) {
		lod_budget = 0;
	} else if (geometry_detail == 1) {
		lod_budget = 5000;
	} else if (geometry_detail == 2) {
		lod_budget = 10000;
	}

	//
	//	Attempt to open the registry key 
	//
	RegistryClass registry (KEY_NAME_SETTINGS);
	if (registry.Is_Valid ()) {

		//
		//	Store the values in the registry
		//
		registry.Set_Int (VALUE_NAME_DYN_LOD, lod_budget);
		registry.Set_Int (VALUE_NAME_STATIC_LOD, lod_budget);

		registry.Set_Int (VALUE_NAME_DYN_SHADOWS, (shadow_mode != PhysicsSceneClass::SHADOW_MODE_NONE));
		registry.Set_Int (VALUE_NAME_STATIC_SHADOWS, static_shadows);

		registry.Set_Int (VALUE_NAME_PRELIT_MODE, prelit_mode);
		registry.Set_Int (VALUE_NAME_TEXTURE_FILTER, texture_filter);
		registry.Set_Int (VALUE_NAME_SHADOW_MODE, shadow_mode);
		registry.Set_Int (VALUE_NAME_TEXTURE_RES, std::max (2 - texture_red, 0));
		registry.Set_Int (VALUE_NAME_SURFACE_EFFECT, surface_effect);
		registry.Set_Int (VALUE_NAME_PARTICLE_DETAIL, particle_detail);
	}

	INIClass ini(W3D_CONF_FILE);
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_DYN_LOD, lod_budget);
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_STATIC_LOD, lod_budget);

	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_DYN_SHADOWS, (shadow_mode != PhysicsSceneClass::SHADOW_MODE_NONE));
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_STATIC_SHADOWS, static_shadows);

	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_PRELIT_MODE, prelit_mode);
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_FILTER, texture_filter);
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_SHADOW_MODE, shadow_mode);
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_RES, std::max (2 - texture_red, 0));
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_SURFACE_EFFECT, surface_effect);
	ini.Put_Int (W3D_SECTION_SYSTEM, VALUE_INI_PARTICLE_DETAIL, particle_detail);
	ini.Save(W3D_CONF_FILE);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
PerformanceConfigDialogClass::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
)
{
	if (message == (WM_USER + 101)) {
		Apply_Changes ();
	}

	return CDialog::WindowProc(message, wParam, lParam);
}
/*
//If really low texture memory such as 4 Mb (if possible to detect) set level 2
- If multtexturing available, default to multitextured lighting, otherwise multi pass
- If a low end card, default to vertex solve
- Default to 640x480, 800x600 or 1024x768 depending on video card
- Default to 32 bit display on HW T&L cards, 16 bit otherwise
- Default to fullscreen (do we even support windowed mode in the release?)
- The same for particle detail
*/

void AutoConfigSettings();


void PerformanceConfigDialogClass::OnGraphicsAutoSetup() 
{
	// TODO: Add your control notification handler code here

	AutoConfigSettings();
	Load_Values();
//	Apply_Changes();
};

/*
		registry.Set_Int (VALUE_NAME_DYN_LOD, lod_budget);
		registry.Set_Int (VALUE_NAME_STATIC_LOD, lod_budget);

		registry.Set_Int (VALUE_NAME_DYN_SHADOWS, (shadow_mode != PhysicsSceneClass::SHADOW_MODE_NONE));
		registry.Set_Int (VALUE_NAME_STATIC_SHADOWS, static_shadows);

		registry.Set_Int (VALUE_NAME_PRELIT_MODE, prelit_mode);
		registry.Set_Int (VALUE_NAME_SHADOW_MODE, shadow_mode);
		registry.Set_Int (VALUE_NAME_TEXTURE_RES, std::max (2 - texture_red, 0));
		registry.Set_Int (VALUE_NAME_SURFACE_EFFECT, surface_effect);
		registry.Set_Int (VALUE_NAME_PARTICLE_DETAIL, particle_detail);
*/
void AutoConfigSettings() 
{
	INIClass ini(W3D_CONF_FILE);

	IDirect3D9* d3d=NULL;
	D3DCAPS9 tmp_caps;
	const D3DCAPS9* d3dcaps=NULL;
	D3DADAPTER_IDENTIFIER9 adapter_id;
	D3DFORMAT display_format;

	VideoConfigDialogClass* video=VideoConfigDialogClass::Get_Instance();
	if (video) {
		WW3D::Set_Texture_Bitdepth(16);
		ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_TEXTURE_DEPTH, 16);
		d3d=DX8Wrapper::_Get_D3D8();
		d3d->AddRef();
		d3dcaps=&video->Get_Current_Caps();
		adapter_id=video->Get_Current_Adapter_Identifier();
		if (video->Get_Current_Bit_Depth()==32) {
			display_format=D3DFMT_A8R8G8B8;
		}
		else {
			display_format=D3DFMT_R5G6B5;
		}
	}
	else {
		// Init D3D
		Init_D3D_To_WW3_Conversion();
		d3d=Direct3DCreate9(D3D_SDK_VERSION);		// TODO: handle failure cases...
		if (!d3d) {
			return;
		}

		// Select device. If there is already a device selected in the registry, use it.

		int current_adapter_index=D3DADAPTER_DEFAULT;

		//
		//	Load the render device settings from the registry
		//
		char device_name[256] = { 0 };
		if (ini.Is_Present(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_NAME)) {
			ini.Get_String(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_NAME, "", device_name, sizeof(device_name));
		}
		
		int adapter_count = d3d->GetAdapterCount();
		for (int adapter_index=0; adapter_index<adapter_count; adapter_index++) {
			D3DADAPTER_IDENTIFIER9 id;
			::ZeroMemory(&id, sizeof(D3DADAPTER_IDENTIFIER9));
			HRESULT res = d3d->GetAdapterIdentifier(adapter_index,0/*NO_WHQL_LEVEL*/,&id);
			// If device ok, check if it matches the currently set adapter name
			if (res == D3D_OK) {
				StringClass name(id.Description,true);
				if (name==device_name) {
					current_adapter_index=adapter_index;
					break;
				}
			}
		}

		if (FAILED(d3d->GetDeviceCaps(
			current_adapter_index,
			D3DDEVTYPE_HAL,
			&tmp_caps))) {
			d3d->Release();
			return;
		}

		::ZeroMemory(&adapter_id, sizeof(D3DADAPTER_IDENTIFIER9));
		if (FAILED( d3d->GetAdapterIdentifier(
			current_adapter_index,
			0/*D3DENUM_WHQL_LEVEL*/,
			&adapter_id))) {
			d3d->Release();
			return;
		}

		// Store device name in config file
		ini.Put_String(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_NAME, adapter_id.Description);
		
		// Set resolution to 800 x 600 x 16
		ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_WIDTH, 800);
		ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_HEIGHT, 600);
		ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_DEPTH, 16);
		ini.Put_Bool(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_WINDOWED, false);
		ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_TEXTURE_DEPTH, 16);

		d3dcaps=&tmp_caps;
		display_format=D3DFMT_R5G6B5;

		// Test if the card if new enough to do 32 bit...
		DX8Caps tmp_caps(d3d,*d3dcaps,WW3D_FORMAT_UNKNOWN,adapter_id);
		switch (tmp_caps.Get_Vendor()) {
		default:
			break;
		case DX8Caps::VENDOR_NVIDIA:
			switch (tmp_caps.Get_Device()) {
			default:
				display_format=D3DFMT_A8R8G8B8;
				ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_DEPTH, 32);
				break;
			case DX8Caps::DEVICE_NVIDIA_TNT2_ALADDIN:
			case DX8Caps::DEVICE_NVIDIA_TNT2:
			case DX8Caps::DEVICE_NVIDIA_TNT2_ULTRA:
			case DX8Caps::DEVICE_NVIDIA_TNT2_VANTA:
			case DX8Caps::DEVICE_NVIDIA_TNT2_M64:
			case DX8Caps::DEVICE_NVIDIA_TNT:
			case DX8Caps::DEVICE_NVIDIA_RIVA_128:
			case DX8Caps::DEVICE_NVIDIA_TNT_VANTA:
			case DX8Caps::DEVICE_NVIDIA_NV1:
				break;
			}
			break;
		case DX8Caps::VENDOR_ATI:
			switch (tmp_caps.Get_Device()) {
			case DX8Caps::DEVICE_ATI_RAGE_II:
			case DX8Caps::DEVICE_ATI_RAGE_II_PLUS:
			case DX8Caps::DEVICE_ATI_RAGE_IIC_PCI:
			case DX8Caps::DEVICE_ATI_RAGE_IIC_AGP:
			case DX8Caps::DEVICE_ATI_RAGE_128_MOBILITY:
			case DX8Caps::DEVICE_ATI_RAGE_128_MOBILITY_M3:
			case DX8Caps::DEVICE_ATI_RAGE_128_MOBILITY_M4:
			case DX8Caps::DEVICE_ATI_RAGE_128_PRO_ULTRA:
			case DX8Caps::DEVICE_ATI_RAGE_128_4X:
			case DX8Caps::DEVICE_ATI_RAGE_128_PRO_GL:
			case DX8Caps::DEVICE_ATI_RAGE_128_PRO_VR:
			case DX8Caps::DEVICE_ATI_RAGE_128_GL:
			case DX8Caps::DEVICE_ATI_RAGE_128_VR:
			case DX8Caps::DEVICE_ATI_RAGE_PRO:
			case DX8Caps::DEVICE_ATI_RAGE_PRO_MOBILITY:
				break;
			default:
				display_format=D3DFMT_A8R8G8B8;
				ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_DEPTH, 32);
				break;
			}
			break;
		}

	}

	DX8Caps caps(d3d,*d3dcaps,D3DFormat_To_WW3DFormat(display_format),adapter_id);
	CanDoMultiPass=caps.Can_Do_Multi_Pass();

	bool high_end_processor=CPUDetectClass::Has_SSE_Instruction_Set();
	if (CPUDetectClass::Get_Processor_Manufacturer()==CPUDetectClass::MANUFACTURER_AMD &&
		CPUDetectClass::Get_AMD_Processor()>=CPUDetectClass::AMD_PROCESSOR_ATHLON_025) {
		high_end_processor=true;
	}


	//	If no texture compression, default to texture resolution 1
	// FIXME Should this be bool for the ini file version?
	if (caps.Support_DXTC()) {
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_RES, 0);
	}
	else {
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_RES, 1);
	}


// High geometry detail on T&L cards, medium on fast processors and low on slow ones
	if (caps.Support_TnL()) {
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_DYN_LOD, 10000);
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_STATIC_LOD, 10000);
	}
	else {
		// If T&L hardware present, set to medium or low. Set to medium if high end cpu.
		// TODO: Set to medium if Athlon detected.
		if (high_end_processor) {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_DYN_LOD, 5000);
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_STATIC_LOD, 5000);
		}
		else {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_DYN_LOD, 0);
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_STATIC_LOD, 0);
		}
	}

// Set high shadow detail if render to texture is supported AND HWTL available
// Set medium shadow detail if render to texture is supported but no HWTL
// Set low shadow detail if no render to texture is available
	if (caps.Support_Render_To_Texture_Format(D3DFormat_To_WW3DFormat(display_format))) {
		if (caps.Support_TnL()) {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_SHADOW_MODE, 3);
			ini.Put_Bool(W3D_SECTION_SYSTEM, VALUE_INI_STATIC_SHADOWS, true);
		}
		else {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_SHADOW_MODE, 2);
			ini.Put_Bool(W3D_SECTION_SYSTEM, VALUE_INI_STATIC_SHADOWS, false);
		}
	}
	else {
		ini.Put_Bool(W3D_SECTION_SYSTEM, VALUE_INI_STATIC_SHADOWS, false);

		// Set to medium if high end cpu detected.
		// TODO: Set to medium if Athlon detected.
		if (high_end_processor) {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_SHADOW_MODE, 1);
		}
		else {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_SHADOW_MODE, 0);
		}
	}

// If a low end system turn surface effects off
	if (caps.Support_TnL()) {
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_SURFACE_EFFECT, 2);
	}
	else {
		// Set to medium if high end cpu detected.
		// TODO: Set to medium if Athlon detected.
		if (high_end_processor) {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_SURFACE_EFFECT, 1);
		}
		else {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_SURFACE_EFFECT, 0);
		}
	}

// If HWTL and high end cpu, use highest particle detail
	if (caps.Support_TnL() && high_end_processor) {
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_PARTICLE_DETAIL, 2);
	}
	// If one or the other, use medium particle detail
	else if (caps.Support_TnL() || high_end_processor) {
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_PARTICLE_DETAIL, 1);
	}
	else {
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_PARTICLE_DETAIL, 0);
	}


	// If the system has less than 100 Megs (64 or 96 most likely) of RAM, select vertex solve to
	// save memory.
	// If card can't do multi pass (which is the case if we've seen z-fighting problems when multi-passing)
	// select vertex solve.
	if (!caps.Can_Do_Multi_Pass() || CPUDetectClass::Get_Total_Physical_Memory()<100*1024*1024) {
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_PRELIT_MODE, 0);
	}
	// Otherwise select multitexturing if card can do it, or multipass...
	else {
		if (caps.Get_Max_Textures_Per_Pass()>=2) {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_PRELIT_MODE, 2);
		}
		else {
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_PRELIT_MODE, 1);
		}
	}

	RegistryClass registry_options (KEY_NAME_OPTIONS);
	if (!registry_options.Is_Valid()) return;

	// The uv bias setting for most of the cards
	// (PowerVR Kyro and Kyro II and ATI Rage Pro need different UV bias and the rest of the cards.)
	registry_options.Set_Int( "ScreenUVBias", 1 );

	switch (caps.Get_Vendor()) {
	default:
		ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_FILTER, 1);
		break;
	case DX8Caps::VENDOR_NVIDIA:
		switch (caps.Get_Device()) {
		case DX8Caps::DEVICE_NVIDIA_TNT2_ALADDIN:
		case DX8Caps::DEVICE_NVIDIA_TNT2:
		case DX8Caps::DEVICE_NVIDIA_TNT2_ULTRA:
		case DX8Caps::DEVICE_NVIDIA_TNT2_VANTA:
		case DX8Caps::DEVICE_NVIDIA_TNT2_M64:
		case DX8Caps::DEVICE_NVIDIA_TNT:
		case DX8Caps::DEVICE_NVIDIA_RIVA_128:
		case DX8Caps::DEVICE_NVIDIA_TNT_VANTA:
		case DX8Caps::DEVICE_NVIDIA_NV1:
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_FILTER, 0);
			break;
		default:
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_FILTER, 1);
		}
		break;
	case DX8Caps::VENDOR_ATI:
		switch (caps.Get_Device()) {
		case DX8Caps::DEVICE_ATI_RAGE_II:
		case DX8Caps::DEVICE_ATI_RAGE_II_PLUS:
		case DX8Caps::DEVICE_ATI_RAGE_IIC_PCI:
		case DX8Caps::DEVICE_ATI_RAGE_IIC_AGP:
		case DX8Caps::DEVICE_ATI_RAGE_128_MOBILITY:
		case DX8Caps::DEVICE_ATI_RAGE_128_MOBILITY_M3:
		case DX8Caps::DEVICE_ATI_RAGE_128_MOBILITY_M4:
		case DX8Caps::DEVICE_ATI_RAGE_128_PRO_ULTRA:
		case DX8Caps::DEVICE_ATI_RAGE_128_4X:
		case DX8Caps::DEVICE_ATI_RAGE_128_PRO_GL:
		case DX8Caps::DEVICE_ATI_RAGE_128_PRO_VR:
		case DX8Caps::DEVICE_ATI_RAGE_128_GL:
		case DX8Caps::DEVICE_ATI_RAGE_128_VR:
		case DX8Caps::DEVICE_ATI_RAGE_PRO:
		case DX8Caps::DEVICE_ATI_RAGE_PRO_MOBILITY:
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_FILTER, 0);

			// It seems the bias needs to be adjusted on Rage128 as well...
			if (caps.Get_Device()==DX8Caps::DEVICE_ATI_RAGE_PRO ||
				caps.Get_Device()==DX8Caps::DEVICE_ATI_RAGE_PRO_MOBILITY ||
				caps.Get_Device()==DX8Caps::DEVICE_ATI_RAGE_128_MOBILITY ||
				caps.Get_Device()==DX8Caps::DEVICE_ATI_RAGE_128_MOBILITY_M3 ||
				caps.Get_Device()==DX8Caps::DEVICE_ATI_RAGE_128_MOBILITY_M4 ||
				caps.Get_Device()==DX8Caps::DEVICE_ATI_RAGE_128_PRO_ULTRA) {
				registry_options.Set_Int( "ScreenUVBias", 0 );
			}
			break;
		default:
			ini.Put_Int(W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_FILTER, 1);
			break;
		}
		break;
	case DX8Caps::VENDOR_POWERVR:
		// not sure how this goes. Kyro at the office requires 1, but some other powervr
		// card seems to require 0...
		registry_options.Set_Int( "ScreenUVBias", 0 );
		switch (caps.Get_Device()) {
		case DX8Caps::DEVICE_POWERVR_KYRO:
			registry_options.Set_Int( "ScreenUVBias", 1 );
			break;
		}
		break;

	}

	d3d->Release();
	ini.Save(W3D_CONF_FILE);
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnShowWindow
//
//	Modified: 12/06/2001 by	MML	- Retrieving strings from Locomoto file.
//
/////////////////////////////////////////////////////////////////////////////
void
PerformanceConfigDialogClass::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	char string[ _MAX_PATH ];

	CDialog::OnShowWindow(bShow, nStatus);
	if (bShow) {

		//
		//	Insert code here
		//

		INIClass ini(W3D_CONF_FILE);

		VideoConfigDialogClass* video=VideoConfigDialogClass::Get_Instance();
		if (video) {

			IDirect3D9* d3d=DX8Wrapper::_Get_D3D8();
			d3d->AddRef();
			D3DADAPTER_IDENTIFIER9 adapter_id=video->Get_Current_Adapter_Identifier();

			D3DFORMAT display_format;
			if (video->Get_Current_Bit_Depth()==32) {
				display_format=D3DFMT_A8R8G8B8;
			}
			else {
				display_format=D3DFMT_R5G6B5;
			}

			DX8Caps caps(d3d,video->Get_Current_Caps(),D3DFormat_To_WW3DFormat(display_format),adapter_id);
			CanDoMultiPass=caps.Can_Do_Multi_Pass();

			//
			//	Populate the lighting combobox
			//

			// Get the current selection
			char cur_sel_string[256];
			unsigned sel=SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_GETCURSEL, 0, 0);
			if (sel!=CB_ERR) {
				SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_GETLBTEXT, sel, (LPARAM)cur_sel_string);
			}
			else {
				cur_sel_string[0]=0;
				if (ini.Is_Present(W3D_SECTION_SYSTEM, VALUE_INI_PRELIT_MODE)) {
					sel = ini.Get_Int(W3D_SECTION_SYSTEM, VALUE_INI_PRELIT_MODE, WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
				}
			}

			// Reset content and add available modes
			SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString( IDS_VERTEX, string ));
			if (caps.Can_Do_Multi_Pass()) {
				SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString( IDS_MULTI_PASS, string ));
			}
			if (caps.Get_Max_Textures_Per_Pass()>1) {
				SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString( IDS_MULTI_TEXTURE, string ));	
			}

			// Try to set the previous selection
			unsigned res=SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_FINDSTRINGEXACT, WPARAM(-1), (LPARAM)cur_sel_string);
			if (res==CB_ERR) {
				if (sel==0) res=0;
				else {
					res=SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_GETCOUNT, 0, 0)-1;
					if (sel<res) res=sel;
				}
			}
			SendDlgItemMessage (IDC_LIGHTING_MODE_COMBO, CB_SETCURSEL, res, 0);

			//
			//	Populate the texture filtering combobox
			//

			// Get the current selection
			sel=SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_GETCURSEL, 0, 0);
			if (sel!=CB_ERR) {
				SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_GETLBTEXT, sel, (LPARAM)cur_sel_string);
			}
			else {
				cur_sel_string[0]=0;
				
				if (ini.Is_Present(W3D_SECTION_SYSTEM, VALUE_INI_PRELIT_MODE)) {
					sel = ini.Get_Int(W3D_SECTION_SYSTEM, VALUE_INI_TEXTURE_FILTER, TextureClass::TEXTURE_FILTER_BILINEAR);
				}
			}
			SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString( IDS_BILINEAR, string ));
			SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString( IDS_TRILINEAR, string ));
			if (caps.Support_Anisotropic_Filtering()) {
				SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString( IDS_ANISOTROPIC, string ));	
			}

			// Try to set the previous selection
			res=SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_FINDSTRINGEXACT, WPARAM(-1), (LPARAM)cur_sel_string);
			if (res==CB_ERR) {
				if (sel==0) res=0;
				else {
					res=SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_GETCOUNT, 0, 0)-1;
					if (sel<res) res=sel;
				}
			}
			SendDlgItemMessage (IDC_TEXTURE_FILTER_COMBO, CB_SETCURSEL, res, 0);

			d3d->Release();

			Determine_Performance_Setting ();
		}
	}

	return ;
}

