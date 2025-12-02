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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/systemsettings.cpp                  $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/21/02 10:49a                                              $*
 *                                                                                             *
 *                    $Revision:: 38                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "systemsettings.h"
#include "registry.h"
//#include "menuentry.h"
#include "_globals.h"

#include "combat.h"
#include "pscene.h"
#include "consolefunction.h"
#include "ccamera.h"
#include "phys.h"
#include "dx8wrapper.h"
#include "dx8caps.h"
#include "surfaceeffects.h"
#include "dlgconfigvideotab.h"
#include	"wwstring.h"
#include "ini.h"
#include "openw3d.h"


/*
**
*/
DynamicVectorClass<SystemSettingEntry *>	SystemSettings::SettingList;

/*
**
*/
void	SystemSettings::Apply_All( void )
{
	for ( int index = 0; index < SettingList.Count(); index++ ) {
		SettingList[ index ]->Apply();
	}
}

/*
**
*/
void	SystemSettings::Registry_Save( const char * sub_key )
{
	INIClass ini(W3D_CONF_FILE);

	for ( int index = 0; index < SettingList.Count(); index++ ) {
		SettingList[ index ]->INI_Save( ini );
	}
	ini.Save(W3D_CONF_FILE);
}

void	SystemSettings::Registry_Load( const char * sub_key )
{
	INIClass ini(W3D_CONF_FILE);

	if (ini.Is_Present(W3D_SECTION_SYSTEM)) {
		for ( int index = 0; index < SettingList.Count(); index++ ) {
			SettingList[ index ]->INI_Load( ini );
		}
	}
}


/*
**
*/
void	SystemSettings::Add_Console_Functions( DynamicVectorClass<ConsoleFunctionClass *> & list )
{
#ifdef WWDEBUG
	for ( int index = 0; index < SettingList.Count(); index++ ) {
		list.Add( SettingList[ index ]->Create_Console_Function() );
	}
#endif
}


/** BOOL ENTRY *********************************************************************************/

class ConsoleFunctionSettingBool : public ConsoleFunctionClass {
public:
	ConsoleFunctionSettingBool( SystemSettingEntryBool	* entry ) : Entry( entry ) {}
	virtual	const char * Get_Name( void ) override	{ return Entry->Get_Name(); }
	virtual	const char * Get_Help( void ) override	{ return Entry->Get_Help(); }
	virtual	void Activate( const char * input ) override {
		Entry->Set_State( !Entry->Get_State() );
		Print( "%s %s\n", Entry->Get_Name(), Entry->Get_State() ? "ON" : "OFF" );
	}
private:
	SystemSettingEntryBool	* Entry;
};

SystemSettingEntryBool::SystemSettingEntryBool( void ) :
	State( false )
{
}

void	SystemSettingEntryBool::Apply( void )
{
	State = Get_Bool();
}

void	SystemSettingEntryBool::Registry_Save( RegistryClass & registry )
{
	registry.Set_Bool( Get_Name(), Get_State() );
}

void	SystemSettingEntryBool::Registry_Load( RegistryClass & registry )
{
	State = Get_Bool();
	Set_State( registry.Get_Bool( Get_Name(), Get_State() ) );
}

void	SystemSettingEntryBool::INI_Save( INIClass & ini )
{
	ini.Put_Bool( W3D_SECTION_SYSTEM, Get_INI_Name(), Get_State() );
}

void	SystemSettingEntryBool::INI_Load( INIClass & ini )
{
	State = Get_Bool();
	Set_State( ini.Get_Bool( W3D_SECTION_SYSTEM, Get_INI_Name(), Get_State() ) );
}


ConsoleFunctionClass *	SystemSettingEntryBool::Create_Console_Function()
{
	return new ConsoleFunctionSettingBool( this );
}

bool	SystemSettingEntryBool::Get_State( void )
{
	return State;
}

void	SystemSettingEntryBool::Set_State( bool state )
{
	State = state;
	Set_Bool( State );
	State = Get_Bool();
}


/** SLIDER ENTRY *********************************************************************************/

class ConsoleFunctionSettingSlider : public ConsoleFunctionClass {
public:
	ConsoleFunctionSettingSlider( SystemSettingEntrySlider	* entry ) : Entry( entry ) {}
	virtual	const char * Get_Name( void ) override	{ return Entry->Get_Name(); }
	virtual	const char * Get_Help( void ) override	{ return Entry->Get_Help(); }
	virtual	void Activate( const char * input ) override {
		Entry->Set_Value( atoi( input ) );
		Print( "%s = %d\n", Entry->Get_Name(), Entry->Get_Value() );
	}
private:
	SystemSettingEntrySlider	* Entry;
};

SystemSettingEntrySlider::SystemSettingEntrySlider( void ) :
	Value( 0 ),
	Min( 0 ),
	Max ( 10 ),
	StepSize( 1 )
{
}

void	SystemSettingEntrySlider::Apply( void )
{
	Value = Get_Slider();
}

void	SystemSettingEntrySlider::Registry_Save( RegistryClass & registry )
{
	registry.Set_Int( Get_Name(), Get_Value() );
}

void	SystemSettingEntrySlider::Registry_Load( RegistryClass & registry )
{
	Set_Value( registry.Get_Int( Get_Name(), Get_Value() ) );
}

void	SystemSettingEntrySlider::INI_Save( INIClass & ini )
{
	ini.Put_Int( W3D_SECTION_SYSTEM, Get_INI_Name(), Get_Value() );
}

void	SystemSettingEntrySlider::INI_Load( INIClass & ini )
{
	Set_Value( ini.Get_Int( W3D_SECTION_SYSTEM, Get_INI_Name(), Get_Value() ) );
}

ConsoleFunctionClass *	SystemSettingEntrySlider::Create_Console_Function()
{
	return new ConsoleFunctionSettingSlider( this );
}

int	SystemSettingEntrySlider::Get_Value( void )
{
	return Value;
}

void	SystemSettingEntrySlider::Set_Value( int value )
{
	Value = (value > Max ) ? Max : ((value < Min) ? Min : value);
	Set_Slider( Value );
	Value = Get_Slider();
}

/** ENUM ENTRY *********************************************************************************/

class ConsoleFunctionSettingEnum : public ConsoleFunctionClass {
public:
	ConsoleFunctionSettingEnum( SystemSettingEntryEnum	* entry ) : Entry( entry ) {}
	virtual	const char * Get_Name( void ) override	{ return Entry->Get_Name(); }
	virtual	const char * Get_Help( void ) override	{ return Entry->Get_Help(); }
	virtual	void Activate( const char * input ) override {
		Entry->Set_Selection( input );
		Print( "%s = %s\n", Entry->Get_Name(), Entry->Get_Enum_Name( Entry->Get_Selection() ) );
	}
private:
	SystemSettingEntryEnum	* Entry;
};

SystemSettingEntryEnum::SystemSettingEntryEnum( void ) :
	Selection( 0 )
{
}

void	SystemSettingEntryEnum::Apply( void )
{
	Selection = Get_Enum();
}

void	SystemSettingEntryEnum::Registry_Save( RegistryClass & registry )
{
	registry.Set_Int( Get_Name(), Get_Selection() );
}

void	SystemSettingEntryEnum::Registry_Load( RegistryClass & registry )
{
	Set_Selection( registry.Get_Int( Get_Name(), Get_Selection() ) );
}

void	SystemSettingEntryEnum::INI_Save( INIClass & ini )
{
	ini.Put_Int( W3D_SECTION_SYSTEM, Get_INI_Name(), Get_Selection() );
}

void	SystemSettingEntryEnum::INI_Load( INIClass & ini )
{
	Set_Selection( ini.Get_Int( W3D_SECTION_SYSTEM, Get_INI_Name(), Get_Selection() ) );
}

ConsoleFunctionClass *	SystemSettingEntryEnum::Create_Console_Function()
{
	return new ConsoleFunctionSettingEnum( this );
}

int	SystemSettingEntryEnum::Get_Selection( void )
{
	return Selection;
}

void	SystemSettingEntryEnum::Set_Selection( int selection )
{
	Selection = (selection >= Get_Enum_Count() ) ? Get_Enum_Count()-1 : ((selection < 0) ? 0 : selection);
	Set_Enum( Selection );
	Selection = Get_Enum();
}

void	SystemSettingEntryEnum::Set_Selection( const char * name )
{
	bool found = false;
	for ( int i = 0; i < Get_Enum_Count(); i++ ) {
		if ( stricmp( name, Get_Enum_Name( i ) ) == 0 ) {
			Set_Selection( i );
			found = true;
		}
	}
	if ( !found ) {
		Set_Selection( atoi( name ) );
	}
}


/***********************************************************************************************/

class	SystemSettingEntryStaticProjectors : public SystemSettingEntryBool {
public:
	const char * Get_Name( void ) override	{ return "Static_Projectors"; }
	const char * Get_INI_Name( void ) override	{ return "StaticProjectors"; }
	const char * Get_Help( void ) override	{ return "STATIC_PROJECTORS - toggles static projectors."; }
	virtual bool Get_Bool( void ) override			{ if ( COMBAT_SCENE ) return COMBAT_SCENE->Are_Static_Projectors_Enabled(); return State; }
	virtual void Set_Bool( bool state ) override	{ if ( COMBAT_SCENE ) COMBAT_SCENE->Enable_Static_Projectors( state ); }
};

/***********************************************************************************************/

class	SystemSettingEntryDynamicProjectors : public SystemSettingEntryBool {
public:
	const char * Get_Name( void ) override	{ return "Dynamic_Projectors"; }
	const char * Get_INI_Name( void ) override	{ return "DynamicProjectors"; }
	const char * Get_Help( void ) override	{ return "DYNAMIC_PROJECTORS - toggles dynamic projectors."; }
	virtual bool Get_Bool( void ) override			{ if ( COMBAT_SCENE ) return COMBAT_SCENE->Are_Dynamic_Projectors_Enabled(); return State; }
	virtual void Set_Bool( bool state ) override	{ if ( COMBAT_SCENE ) COMBAT_SCENE->Enable_Dynamic_Projectors( state ); }
};

/***********************************************************************************************/

class	SystemSettingEntryWeaponHelp : public SystemSettingEntryBool {
public:
	const char * Get_Name( void ) override	{ return "Enable_Weapon_Help"; }
	const char * Get_INI_Name( void ) override	{ return "EnableWeaponHelp"; }
	const char * Get_Help( void ) override	{ return "ENABLE_WEAPON_HELP - toggles weapon aiming help."; }
	virtual bool Get_Bool( void ) override			{ if ( COMBAT_CAMERA ) return COMBAT_CAMERA->Is_Weapon_Help_Enabled(); return State; }
	virtual void Set_Bool( bool state ) override	{ if ( COMBAT_CAMERA ) COMBAT_CAMERA->Enable_Weapon_Help( state ); }
};

/***********************************************************************************************/

class	SystemSettingEntryAutoTransitions : public SystemSettingEntryBool {
public:
	const char * Get_Name( void ) override	{ return "Enable_Auto_Transitions"; }
	const char * Get_INI_Name( void ) override	{ return "EnableAutoTransitions"; }
	const char * Get_Help( void ) override	{ return "ENABLE_AUTO_TRANSITIONS - toggles automatic doors and transitions."; }
	virtual bool Get_Bool( void ) override			{ return CombatManager::Are_Transitions_Automatic(); }
	virtual void Set_Bool( bool state ) override	{ CombatManager::Set_Transitions_Automatic( state ); }
};

/***********************************************************************************************/

class	SystemSettingEntryGammaLevel : public SystemSettingEntrySlider {
public:
	SystemSettingEntryGammaLevel( void ) {
		Set_Range( GAMMA_SLIDER_MIN, GAMMA_SLIDER_MAX );
		Set_Step_Size( 1 );
		Set_Value( GAMMA_SLIDER_DEFAULT );
	}
	const char * Get_Name( void ) override	{ return "Gamma_Correction"; }
	const char * Get_INI_Name( void ) override	{ return "GammaCorrection"; }
	const char * Get_Help( void ) override	{ formatstring.Format ("GAMMA_CORRECTION [%d..%d] - Adjusts gamma correction curve for the display.", GAMMA_SLIDER_MIN, GAMMA_SLIDER_MAX); return (formatstring); }
	
	int	Get_Slider( void ) override		{ Value = DlgConfigVideoTabClass::Get_Gamma (); return Value; }
	void	Set_Slider( int value ) override	{ DlgConfigVideoTabClass::Set_Gamma (value); }
	void	Registry_Save				( RegistryClass & registry ) override { registry.Set_Int (Get_Name (), Get_Slider ()); }

protected:
	StringClass formatstring;
};


/***********************************************************************************************/

class	SystemSettingEntryBrightnessLevel : public SystemSettingEntrySlider {
public:
	SystemSettingEntryBrightnessLevel( void ) {
		Set_Range( BRIGHTNESS_SLIDER_MIN, BRIGHTNESS_SLIDER_MAX );
		Set_Step_Size( 1 );
		Set_Value( BRIGHTNESS_SLIDER_DEFAULT );
	}
	const char * Get_Name( void ) override	{ return "Brightness"; }
	const char * Get_INI_Name( void ) override	{ return "Brightness"; }
	const char * Get_Help( void ) override	{ formatstring.Format ("BRIGHTNESS [%d..%d] - Adjusts brightness for the display.", BRIGHTNESS_SLIDER_MIN, BRIGHTNESS_SLIDER_MAX); return (formatstring); }
	
	int	Get_Slider( void ) override		{ Value = DlgConfigVideoTabClass::Get_Brightness (); return Value; }
	void	Set_Slider( int value ) override	{ DlgConfigVideoTabClass::Set_Brightness (value); }
	void	Registry_Save				( RegistryClass & registry ) override { registry.Set_Int (Get_Name (), Get_Slider ()); }

protected:
	StringClass formatstring;
};

/***********************************************************************************************/

class	SystemSettingEntryContrastLevel : public SystemSettingEntrySlider {
public:
	SystemSettingEntryContrastLevel( void ) {
		Set_Range( CONTRAST_SLIDER_MIN, CONTRAST_SLIDER_MAX );
		Set_Step_Size( 1 );
		Set_Value( CONTRAST_SLIDER_DEFAULT );
	}
	const char * Get_Name( void ) override	{ return "Contrast"; }
	const char * Get_INI_Name( void ) override	{ return "Contrast"; }
	const char * Get_Help( void ) override	{ formatstring.Format ("CONTRAST [%d..%d] - Adjusts contrast for the display.", CONTRAST_SLIDER_MIN, CONTRAST_SLIDER_MAX); return (formatstring); }

	int	Get_Slider( void ) override		{ Value = DlgConfigVideoTabClass::Get_Contrast (); return Value; }
	void	Set_Slider( int value ) override	{ DlgConfigVideoTabClass::Set_Contrast (value); }
	void	Registry_Save				( RegistryClass & registry ) override { registry.Set_Int (Get_Name (), Get_Slider ()); }

protected:
	StringClass formatstring;
};


/***********************************************************************************************/

class	SystemSettingEntryTextureResolution : public SystemSettingEntrySlider {
public:
	SystemSettingEntryTextureResolution( void ) {
		Set_Range( 0, 7 );
		Set_Step_Size( 1 );
		Set_Value( 0 );
	}
	const char * Get_Name( void ) override	{ return "Texture_Resolution"; }
	const char * Get_INI_Name( void ) override	{ return "TextureResolution"; }
	const char * Get_Help( void ) override	{ return "TEXTURE_RESOLUTION [0..7] - sets the texture resolution."; }
	virtual int Get_Slider( void ) override			{ return WW3D::Get_Texture_Reduction(); }
	virtual void Set_Slider( int value ) override	{ if ( WW3D::Get_Texture_Reduction() != value ) { WW3D::Set_Texture_Reduction( value ); } }

};


/***********************************************************************************************/

class	SystemSettingEntryDynamicLODBudget : public SystemSettingEntrySlider {
public:
	SystemSettingEntryDynamicLODBudget( void ) {
		Set_Range( 100, 100000 );
		Set_Step_Size( 100 );
		Set_Value( 10000 );
	}
	const char * Get_Name( void ) override	{ return "Dynamic_LOD_Budget"; }
	const char * Get_INI_Name( void ) override	{ return "DynamicLODBudget"; }
	const char * Get_Help( void ) override	{ return "DYNAMIC_LOD_BUDGET <n> - sets the Dynamic LOD Budget."; }
	virtual int Get_Slider( void ) override	{
		int scount,dcount = Value;
		if ( PhysicsSceneClass::Get_Instance() ) {
			PhysicsSceneClass::Get_Instance()->Get_Polygon_Budgets(&scount,&dcount);
		}
		return dcount;
	}
	virtual void Set_Slider( int value ) override {
		if ( PhysicsSceneClass::Get_Instance() ) {
			int scount,dcount;
			PhysicsSceneClass::Get_Instance()->Get_Polygon_Budgets(&scount,&dcount);
			PhysicsSceneClass::Get_Instance()->Set_Polygon_Budgets(scount,value);
		}
	}
};

/***********************************************************************************************/

class	SystemSettingEntryStaticLODBudget : public SystemSettingEntrySlider {
public:
	SystemSettingEntryStaticLODBudget( void ) {
		Set_Range( 100, 10000 );
		Set_Step_Size( 100 );
		Set_Value( 3000 );
	}
	const char * Get_Name( void ) override	{ return "Static_LOD_Budget"; }
	const char * Get_INI_Name( void ) override	{ return "StaticLODBudget"; }
	const char * Get_Help( void ) override	{ return "STATIC_LOD_BUDGET <n> - sets the Static LOD Budget."; }
	virtual int Get_Slider( void ) override	{
		int dcount,scount = Value;
		if ( PhysicsSceneClass::Get_Instance() ) {
			PhysicsSceneClass::Get_Instance()->Get_Polygon_Budgets(&scount,&dcount);
		}
		return scount;
	}
	virtual void Set_Slider( int value ) override {
		if ( PhysicsSceneClass::Get_Instance() ) {
			int scount,dcount;
			PhysicsSceneClass::Get_Instance()->Get_Polygon_Budgets(&scount,&dcount);
			PhysicsSceneClass::Get_Instance()->Set_Polygon_Budgets(value,dcount);
		}
	}
};

/***********************************************************************************************/

class	SystemSettingEntryShadowMode : public SystemSettingEntryEnum {
public:
	const char * Get_Name( void ) override	{ return "Shadow_Mode"; }
	const char * Get_INI_Name( void ) override	{ return "ShadowMode"; }
	const char * Get_Help( void ) override	{ return "SHADOW_MODE <mode> - 0=none 1=blobs 2=blobs+ 3=projected textures"; }
	virtual	int Get_Enum( void ) override { if (COMBAT_SCENE) return COMBAT_SCENE->Get_Shadow_Mode(); return Selection; }
	virtual	void Set_Enum( int selection ) override { if (COMBAT_SCENE) COMBAT_SCENE->Set_Shadow_Mode( (PhysicsSceneClass::ShadowEnum)selection ); }
	virtual	int Get_Enum_Count( void ) override { return 4; }
	virtual	const char * Get_Enum_Name( int selection ) override {
		static const char * names[ 4 ] = { "None", "Blobs", "Blobs+", "Projections" };
		return names[ selection ];
	}
};

/***********************************************************************************************/

class	SystemSettingEntrySurfaceEffectDetail: public SystemSettingEntryEnum {
public:
	const char * Get_Name( void ) override	{ return "Surface_Effect_Detail"; }
	const char * Get_INI_Name( void ) override	{ return "SurfaceEffectDetail"; }
	const char * Get_Help( void ) override	{ return "SURFACE_EFFECT_DETAIL <level> - 0=off 1=no emitters 2=full"; }
	virtual	int Get_Enum( void ) override { return SurfaceEffectsManager::Get_Mode (); }
	virtual	void Set_Enum( int value ) override { SurfaceEffectsManager::Set_Mode ((SurfaceEffectsManager::MODE)value); }
	virtual	int Get_Enum_Count( void ) override { return 3; }
	virtual	const char * Get_Enum_Name( int selection ) override {
		static const char * names[ 3 ] = { "Off", "No Emitters", "Full" };
		return names[ selection ];
	}
};

/***********************************************************************************************/

class	SystemSettingEntryMeshDrawMode : public SystemSettingEntryEnum {
	static const char * names[7];
public:
	const char * Get_Name( void ) override	{ return "Mesh_Draw_Mode"; }
	const char * Get_INI_Name( void ) override	{ return "MeshDrawMode"; }
	const char * Get_Help( void ) override	{ return "MESH_DRAW_MODE <mode> - 0=old 1=new 2=debug 3=debug clip 4=box 5=none 6=dx8 only"; }
	virtual	int Get_Enum( void ) override { return WW3D::Get_Mesh_Draw_Mode(); }
	virtual	void Set_Enum( int selection ) override {
		WW3D::Set_Mesh_Draw_Mode( (WW3D::MeshDrawModeEnum) selection );
	}
	virtual	int Get_Enum_Count( void ) override { return sizeof(names)/sizeof(char*); }
	virtual	const char * Get_Enum_Name( int selection ) override {
		WWASSERT(selection>=0 && selection<sizeof(names)/sizeof(char*));
		return names[ selection ];
	}
};
const char * SystemSettingEntryMeshDrawMode::names[] = { "Old", "New", "Debug Draw", "Debug Clip", "Box", "None", "DX8 only" };

/***********************************************************************************************/

class	SystemSettingEntryNPatchGapFillingMode : public SystemSettingEntryEnum {
	static const char * names[3];
public:
	const char * Get_Name( void ) override	{ return "NPatches_Gap_Filling_Mode"; }
	const char * Get_INI_Name( void ) override	{ return "NPatchesFillMode"; }
	const char * Get_Help( void ) override	{ return "NPATCHES_GAP_FILLING_MODE <mode> - 0=disabled 1=enabled 2=force"; }
	virtual	int Get_Enum( void ) override { return WW3D::Get_NPatches_Gap_Filling_Mode(); }
	virtual	void Set_Enum( int selection ) override {
		WW3D::Set_NPatches_Gap_Filling_Mode( (WW3D::NPatchesGapFillingModeEnum) selection );
	}
	virtual	int Get_Enum_Count( void ) override { return sizeof(names)/sizeof(char*); }
	virtual	const char * Get_Enum_Name( int selection ) override {
		WWASSERT(selection>=0 && selection<sizeof(names)/sizeof(char*));
		return names[ selection ];
	}
};
const char * SystemSettingEntryNPatchGapFillingMode::names[] = { "Disabled", "Enabled", "FORCE (Enabled even when not using n-patches)" };

/***********************************************************************************************/

class	SystemSettingEntryNPatches : public SystemSettingEntryEnum {
	static const char * names[9];
	StringClass Name;
public:
	SystemSettingEntryNPatches()
	{
		Name="NPatches";
	}
	virtual ~SystemSettingEntryNPatches(void){};
	const char * Get_Name( void ) override	{ return Name; }
	const char * Get_INI_Name( void ) override	{ return "NPatchesLevel"; }
	const char * Get_Help( void ) override	{ return "NPatches <level> - 1=default, 8=max"; }
	virtual	int Get_Enum( void ) override { return WW3D::Get_NPatches_Level(); }
	virtual	void Set_Enum( int selection ) override {
		WW3D::Set_NPatches_Level( selection );
		if (DX8Wrapper::Get_Current_Caps() && !DX8Wrapper::Get_Current_Caps()->Support_NPatches()) {
			Name="NPatches (NOT SUPPORTED BY HARDWARE)";
		}
	}
	virtual	int Get_Enum_Count( void ) override { return sizeof(names)/sizeof(char*); }
	virtual	const char * Get_Enum_Name( int selection ) override {
		WWASSERT(selection>=0 && selection<sizeof(names)/sizeof(char*));
		return names[ selection ];
	}
};
const char * SystemSettingEntryNPatches::names[] = {
	"NPatches DISABLED",
	"NPatches DISABLED",
	"NPatches level 2",
	"NPatches level 3",
	"NPatches level 4",
	"NPatches level 5",
	"NPatches level 6",
	"NPatches level 7",
	"NPatches level 8"
};

/***********************************************************************************************/

class	SystemSettingEntryPrelitMode : public SystemSettingEntryEnum {
public:
	const char * Get_Name( void ) override	{ return "Prelit_Mode"; }
	const char * Get_INI_Name( void ) override	{ return "PrelitMode"; }
	const char * Get_Help( void ) override	{ return "PRELIT_MODE <mode> - 0=vertex 1=multi-pass 2=multi-texture."; }
	virtual	int Get_Enum( void ) override { return WW3D::Get_Prelit_Mode(); }
	virtual	void Set_Enum( int selection ) override {
		if ( WW3D::Supports_Prelit_Mode( (WW3D::PrelitModeEnum) selection ) ) {
			WW3D::Set_Prelit_Mode( (WW3D::PrelitModeEnum) selection );
		}
	}
	virtual	int Get_Enum_Count( void ) override { return 3; }
	virtual	const char * Get_Enum_Name( int selection ) override {
		static const char * names[ 3 ] = { "Vertex", "Multi-pass", "Multi-texture" };
		return names[ selection ];
	}
};

/***********************************************************************************************/

class	SystemSettingEntryTextureFilterMode : public SystemSettingEntryEnum {
public:
	const char * Get_Name( void ) override	{ return "Texture_Filter_Mode"; }
	const char * Get_INI_Name( void ) override	{ return "TextureFilterMode"; }
	const char * Get_Help( void ) override	{ return "TEXTURE_FILTER_MODE <mode> - 0=bilinear 1=trilinear 2=anisotropic."; }
	virtual	int Get_Enum( void ) override { return WW3D::Get_Texture_Filter(); }
	virtual	void Set_Enum( int selection ) override {
		WW3D::Set_Texture_Filter( selection );
	}
	virtual	int Get_Enum_Count( void ) override { return 3; }
	virtual	const char * Get_Enum_Name( int selection ) override {
		static const char * names[ 3 ] = { "Bilinear", "Trilinear", "Anisotropic" };
		return names[ selection ];
	}
};

/***********************************************************************************************/

class	SystemSettingEntryDifficulty : public SystemSettingEntryEnum {
public:
	SystemSettingEntryDifficulty( void ) {	Selection = Get_Enum(); }
	const char * Get_Name( void ) override	{ return "Difficulty"; }
	const char * Get_INI_Name( void ) override	{ return "Difficulty"; }
	const char * Get_Help( void ) override	{ return "DIFFICULTY [0..2] - sets the difficulty level."; }
	virtual int Get_Enum( void ) override			{ return CombatManager::Get_Difficulty_Level(); }
	virtual void Set_Enum( int value ) override	{ CombatManager::Set_Difficulty_Level( value ); }
	virtual	int Get_Enum_Count( void ) override { return 3; }
	virtual	const char * Get_Enum_Name( int selection ) override {
		static const char * names[ 3 ] = { "Easy", "Average", "Difficult" };
		return names[ selection ];
	}
};

/***********************************************************************************************/

void SystemSettings::Init( void )
{
	Add_Setting( new SystemSettingEntryStaticProjectors );
	Add_Setting( new SystemSettingEntryDynamicProjectors );
	Add_Setting( new SystemSettingEntryTextureResolution );
	Add_Setting( new SystemSettingEntryDynamicLODBudget );
	Add_Setting( new SystemSettingEntryStaticLODBudget );
	Add_Setting( new SystemSettingEntryShadowMode );
	Add_Setting( new SystemSettingEntryPrelitMode );
	Add_Setting( new SystemSettingEntryMeshDrawMode );
	Add_Setting( new SystemSettingEntryNPatches );
	Add_Setting( new SystemSettingEntryNPatchGapFillingMode );
//	Add_Setting( new SystemSettingEntryTextureCompressionMode );
	Add_Setting( new SystemSettingEntryTextureFilterMode );
//	Add_Setting( new SystemSettingEntryTextureThumbnailMode );
//	Add_Setting( new SystemSettingEntryWeaponHelp );
	Add_Setting( new SystemSettingEntryAutoTransitions );
	Add_Setting( new SystemSettingEntryDifficulty );
	Add_Setting( new SystemSettingEntrySurfaceEffectDetail )	;
	Add_Setting( new SystemSettingEntryGammaLevel )	;
	Add_Setting( new SystemSettingEntryBrightnessLevel )	;
	Add_Setting( new SystemSettingEntryContrastLevel )	;

	Registry_Load( APPLICATION_SUB_KEY_NAME_SYSTEM_SETTINGS );
}

void SystemSettings::Shutdown( void )
{
	Registry_Save( APPLICATION_SUB_KEY_NAME_SYSTEM_SETTINGS );

	/*
	** Free the system setting so they don't look like memory leaks when we exit. ST - 6/11/2001 8:32PM
	*/
	while (SettingList.Count()) {
		delete SettingList[0];
		SettingList.Delete(0);
	}
}














/*
#if DEADMENU
//
//
//
MenuEntryClass	* SystemSettings::Create_Menu_Entry( const char * setting_name )
{
	for ( int index = 0; index < SettingList.Count(); index++ ) {
		if ( !stricmp( SettingList[ index ]->Get_Name(), setting_name ) ) {
			MenuEntryClass	* entry = SettingList[ index ]->Create_Menu_Entry();
			if ( entry ) {
				entry->Set_Name( setting_name );
			}
			return entry;
		}
	}
	return NULL;
}
#endif //DEADMENU

#if DEADMENU
class	MenuEntrySettingBool : public MenuEntryToggleClass {
public:
	MenuEntrySettingBool( SystemSettingEntryBool	* entry ) : Entry( entry ) {}
	virtual	bool	Get_State( void )	{ return Entry->Get_State(); }
	virtual	void	Toggle( void )		{ Entry->Set_State( !Entry->Get_State() ); }
private:
	SystemSettingEntryBool	* Entry;
};
#endif // DEADMENU

#if DEADMENU
MenuEntryClass	*	SystemSettingEntryBool::Create_Menu_Entry( void )
{
	return new MenuEntrySettingBool( this );
}
#endif // DEADMENU

#if DEADMENU
class	MenuEntrySettingSlider : public MenuEntryIntSliderClass {
public:
	MenuEntrySettingSlider( SystemSettingEntrySlider	* entry ) : Entry( entry ) {}
	virtual	int	Get_Value( void )			{ return Entry->Get_Value(); }
	virtual	void	Set_Value( int value )	{ Entry->Set_Value( value ); }
	virtual	int	Get_Step_Size( void )	{ return Entry->Get_Step_Size(); }

private:
	SystemSettingEntrySlider	* Entry;
};
#endif // DEADMENU

#if DEADMENU
MenuEntryClass	*	SystemSettingEntrySlider::Create_Menu_Entry( void )
{
	return new MenuEntrySettingSlider( this );
}
#endif // DEADMENU


#if DEADMENU
class	MenuEntrySettingEnum : public MenuEntryEnumClass {
public:
	MenuEntrySettingEnum( SystemSettingEntryEnum	* entry ) : Entry( entry ) {}
	virtual	const char * Get_Selection_Name( void )
	{
		return Entry->Get_Enum_Name( Entry->Get_Selection() );
	}
	virtual	void	Activate( void )
	{
		int selection = Entry->Get_Selection() + 1;
		if ( selection == Entry->Get_Enum_Count() ) {
			selection = 0;
		}
		Entry->Set_Selection( selection );
	}
private:
	SystemSettingEntryEnum	* Entry;
};
#endif // DEADMENU

#if DEADMENU
MenuEntryClass	*	SystemSettingEntryEnum::Create_Menu_Entry( void )
{
	return new MenuEntrySettingEnum( this );
}
#endif // DEADMENU

*/