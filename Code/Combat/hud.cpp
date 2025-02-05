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
 *                     $Archive:: /Commando/Code/Combat/hud.cpp                               $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/11/02 2:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 228                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "hud.h"

#include <stdio.h>

#include "wwprofile.h"
#include "assets.h"
#include "font3d.h"
#include "combat.h"
#include "soldier.h"
#include "debug.h"
#include "ccamera.h"
#include "vehicle.h"
#include "weapons.h"
#include "radar.h"
#include "texture.h"
#include "phys.h"
#include "render2d.h"
#include "hudinfo.h"
#include "globalsettings.h"
#include "translatedb.h"
#include "playerdata.h"
#include "playertype.h"
#include "sniper.h"
#include "render2dsentence.h"
#include "input.h"
#include "building.h"
#include "objectives.h"
#include "weaponbag.h"
#include "string_ids.h"
#include "gametype.h"
#include "stylemgr.h"


static void Generate_WChar_Text_From_Number(WCHAR* text,int digits,int min_digits,int value)
{
	text[digits]=0;
	while (digits) {
		digits--;
		text[digits]=value%10+'0';
		value/=10;
		min_digits--;
		if (value==0 && (min_digits == 0)) break;
	}
	if ((min_digits==0) && digits) {
		int i=0;
		while (WCHAR c=text[digits++]) {
			text[i++]=c;
		}
		text[i]=0;
	}
}

/*
**
*/
Vector2	INFO_UV_SCALE( 1.0f/256.0f, 1.0f/256.0f );

Vector2 INFO_OFFSET( 7, -179 );
Vector2 FRAME_1_UV_UL( 96, 105 );
Vector2 FRAME_1_UV_LR( 214, 255 );
Vector2 FRAME_1_OFFSET( -3, -1 );
Vector2 FRAME_2_UV_UL( 215, 125 );
Vector2 FRAME_2_UV_LR( 255, 192 );
Vector2 FRAME_2_OFFSET( 114, 57 );
Vector2 FRAME_3_UV_UL( 218, 192 );
Vector2 FRAME_3_UV_LR( 255, 201 );
Vector2 FRAME_3_OFFSET( 154, 115 );
Vector2 FRAME_4_UV_UL( 216, 200 );
Vector2 FRAME_4_UV_LR( 255, 255 );
Vector2 FRAME_4_OFFSET( 191, 115 );
Vector2 FRAME_5_UV_UL( 80, 203 );
Vector2 FRAME_5_UV_LR( 100, 258 );
Vector2 FRAME_5_OFFSET( 230, 116 );
Vector2 FRAME_6_UV_UL( 216, 101 );
Vector2 FRAME_6_UV_LR( 240, 125 );
Vector2 FRAME_6_OFFSET( 74, 149 );
Vector2 HEALTH_BACK_UV_UL( 183, 241 );
Vector2 HEALTH_BACK_UV_LR( 186, 248 );
Vector2 HEALTH_BACK_UL( 98, 122 );
Vector2 HEALTH_BACK_LR( 224, 168 );
Vector2 GRADIENT_BLACK_UV_UL( 3, 135 );
Vector2 GRADIENT_BLACK_UV_LR( 44, 144 );
Vector2 HEALTH_TEXT_BACK_UL( 77, 124 );
Vector2 HEALTH_TEXT_BACK_LR( 163, 150 );
Vector2 HEALTH_UV_UL( 94, 52 );
Vector2 HEALTH_UV_LR( 249, 100 );
Vector2 HEALTH_OFFSET( 73, 121 );
Vector2 SHIELD_UV_UL( 66, 97 );
Vector2 SHIELD_UV_LR( 96, 132 );
Vector2 SHIELD_OFFSET( 211, 140 );
Vector2 KEY_1_UV_UL( 30, 180 );
Vector2 KEY_1_UV_LR( 57, 197 );
Vector2 KEY_1_OFFSET( 32, 134 );
Vector2 KEY_2_UV_UL( 0, 181 );
Vector2 KEY_2_UV_LR( 30, 197 );
Vector2 KEY_2_OFFSET( 41, 140 );
Vector2 KEY_3_UV_UL( 69, 133 );
Vector2 KEY_3_UV_LR( 97, 149 );
Vector2 KEY_3_OFFSET( 50, 148 );
Vector2 HEALTH_CROSS_1_UV_UL( 33, 199 );
Vector2 HEALTH_CROSS_1_UV_LR( 63, 226 );
Vector2 HEALTH_CROSS_1_OFFSET( 77, 124 );
Vector2 HEALTH_CROSS_2_UV_UL( 33, 228 );
Vector2 HEALTH_CROSS_2_UV_LR( 63, 258 );
Vector2 HEALTH_CROSS_2_OFFSET( 77, 124 );
Vector2 TIME_BACK_UL( 150, 97 );
Vector2 TIME_BACK_LR( 213, 116 );
Vector2 TIME_CLOCK_UV_UL( 46, 125 );
Vector2 TIME_CLOCK_UV_LR( 66, 148 );
Vector2 TIME_CLOCK_OFFSET( 186, 94 );
Vector2 WEAPON_BOX_UV_UL( 0, 0 );
Vector2 WEAPON_BOX_UV_LR( 95, 95 );
Vector2 WEAPON_BOX_OFFSET( 195, -53 );
Vector2 POWER_OUT_ICON_UV_UL( 0, 95 );
Vector2 POWER_OUT_ICON_UV_LR( 30, 129 );
Vector2 POWER_OUT_ICON_OFFSET( 143, 41 );
Vector2 REPAIR_ICON_UV_UL( 31, 97 );
Vector2 REPAIR_ICON_UV_LR( 61, 126 );
Vector2 REPAIR_ICON_OFFSET( 181, 43 );
Vector2 GDI_ICON_UV_UL( 14, 222 );
Vector2 GDI_ICON_UV_LR( 34, 238 );
Vector2 GDI_ICON_OFFSET( 217, 77 );
Vector2 NOD_ICON_UV_UL( 14, 205 );
Vector2 NOD_ICON_UV_LR( 34, 221 );
Vector2 NOD_ICON_OFFSET( 238, 76 );
Vector2 NEUTRAL_ICON_UV_UL( 14, 238 );
Vector2 NEUTRAL_ICON_UV_LR( 33, 257 );
Vector2 NEUTRAL_ICON_OFFSET( 260, 77 );
Vector2 BULLET_ICON_UV_UL( 2, 211 );
Vector2 BULLET_ICON_UV_LR( 13, 255 );
Vector2 BULLET_ICON_OFFSET( -20, -30 );
Vector2 RADAR_CENTER_OFFSET( 55, 78 );
Vector2 DAMAGE_1_UV_UL( 65, 184 );
Vector2 DAMAGE_1_UV_LR( 78, 255 );
Vector2 DAMAGE_2_UV_UL( 200, 3 );
Vector2 DAMAGE_2_UV_LR( 248, 51 );
Vector2 HORIZ_DAMAGE_SIZE( 81, 14 );
Vector2 VERT_DAMAGE_SIZE( 15, 78 );
Vector2 DIAG_DAMAGE_SIZE( 53, 59 );
Vector2 HV_DAMAGE_OFFSET( 170, 168 );
Vector2 DIAG_DAMAGE_OFFSET( 119, 117 );
//Vector2 POWERUP_BOX_UV_UL( 48, 0 );
//Vector2 POWERUP_BOX_UV_LR( 129, 54 );
Vector2 POWERUP_BOX_UV_UL( 50, 1 );
Vector2 POWERUP_BOX_UV_LR( 127, 52 );
Vector2 RADAR_RINGS_UV_UL( 95, 0 );
Vector2 RADAR_RINGS_UV_LR( 197, 53 );
Vector2 RADAR_RINGS_L_OFFSET( -51, -50 );
Vector2 RADAR_RINGS_R_OFFSET( 0, -50 );
Vector2 RADAR_STAR_UV_UL( 241, 103 );
Vector2 RADAR_STAR_UV_LR( 249, 111 );
Vector2 RADAR_STAR_OFFSET( 280, 50 );
Vector2 RADAR_SQUARE_UV_UL( 247, 85 );
Vector2 RADAR_SQUARE_UV_LR( 255, 93 );
Vector2 RADAR_SQUARE_OFFSET( 290, 50 );
Vector2 RADAR_TRIANGLE_UV_UL( 247, 93 );
Vector2 RADAR_TRIANGLE_UV_LR( 255, 101 );
Vector2 RADAR_TRIANGLE_OFFSET( 300, 50 );
Vector2 RADAR_CIRCLE_UV_UL( 247, 77 );
Vector2 RADAR_CIRCLE_UV_LR( 255, 85 );
Vector2 RADAR_CIRCLE_OFFSET( 310, 50 );
Vector2 RADAR_BRACKET_UV_UL( 241, 114 );
Vector2 RADAR_BRACKET_UV_LR( 249, 122 );
Vector2 RADAR_BRACKET_OFFSET( 320, 50 );
Vector2 RADAR_SWEEP_UV_UL( 80, 182 );
Vector2 RADAR_SWEEP_UV_LR( 95, 191 );
Vector2 RADAR_SWEEP_OFFSET( 330, 50 );

Vector2 TARGET_HEALTH_L_UV_UL( 0, 165 );
Vector2 TARGET_HEALTH_L_UV_LR( 20, 181 );
Vector2 TARGET_HEALTH_R_UV_UL( 20, 165 );
//Vector2 TARGET_HEALTH_R_UV_LR( 96, 181 );
Vector2 TARGET_HEALTH_R_UV_LR( 96, 173 );
Vector2 TARGET_HEALTH_OFFSET( 123, 5 );
Vector2 TARGET_NAME_UV_UL( 1, 149 );
Vector2 TARGET_NAME_UV_LR( 91, 164 );
Vector2 TARGET_NAME_OFFSET( 125, 24 );


	// Reticle
#define	RETICLE_WIDTH	(64.0f/640.0f)
#define	RETICLE_HEIGHT	(64.0f/480.0f)


//

Vector2 TARGET_ENTERABLE_UV_UL( 45, 209 );
Vector2 TARGET_ENTERABLE_UV_LR( 51, 215 );
Vector2 TARGET_ENTERABLE_SIZE( 32, 32 );
float	  TARGET_ENTERABLE_BOUNCE	 = 4;

/*
**
*/
//#define		LARGE_FONT		"FONT24x36.TGA"
#define		LARGE_FONT		"FONT12x16.TGA"
#define		MEDIUM_FONT		"FONT12x16.TGA"
#define		SMALL_FONT		"FONT6x8.TGA"

enum
{
	HUD_HELP_TEXT_DISPLAYING = 0,
	HUD_HELP_TEXT_FADING,
	HUD_HELP_TEXT_DONE
};

/*
**
*/
#define		HUD_MAIN_TEXTURE		"HUD_MAIN.TGA"
#define		HUD_CHATPBOX_TEXTURE	"HUD_CHATPBOX.TGA"
#define		HUD_WEAPONS_TEXTURE	"hud_TibRVoltR.tga"			/* This should be removed */
Render2DClass *			InfoRenderer;
Render2DSentenceClass * HUDHelpTextRenderer;
Vector2						HUDHelpTextExtents (0, 0);
float							HUDHelpTextTimer = 0;
int							HUDHelpTextState = HUD_HELP_TEXT_DISPLAYING;


/*
**
*/
unsigned long COLOR( float alpha, unsigned long color = 0x00FFFFFF )
{
	alpha = WWMath::Clamp( alpha, 0, 1 ) * 255.0f;
	color &= 0x00FFFFFF;
	color |= ((int)alpha) << 24;
	return color;
}

unsigned long Get_Health_Color( float percent )
{
	Vector3	color = HUDGlobalSettingsDef::Get_Instance()->Get_Health_High_Color();
	if ( percent <= 0.5f ) {
		color = HUDGlobalSettingsDef::Get_Instance()->Get_Health_Med_Color();
	}
	if ( percent <= 0.25f ) {
		color = HUDGlobalSettingsDef::Get_Instance()->Get_Health_Low_Color();
	}

	return color.Convert_To_ARGB();
}

/*
** Powerup Display
*/
struct PowerupIconStruct {

	PowerupIconStruct( void ) : Renderer( NULL ), Number( 0 )	{}

	~PowerupIconStruct( void )	{
		if ( Renderer != NULL ) {
			delete Renderer;
			Renderer = NULL;
		}
	}

	WideStringClass	Name;
	int				Number;
	Render2DClass * Renderer;
	RectClass		UV;
	RectClass		IconBox;
	float			Timer;
};

// Two lists.  A right one for weapons, health, and armor
static	DynamicVectorClass<PowerupIconStruct*>	RightPowerupIconList;
// and a left one for keys
static	DynamicVectorClass<PowerupIconStruct*>	LeftPowerupIconList;

Render2DClass * PowerupBoxRenderer;
Render2DSentenceClass * PowerupTextRenderer;

#define		MAX_ICONS		5
#define		POWERUP_TIME	6

static	void	Powerup_Init( void )
{
	PowerupBoxRenderer = new Render2DClass();
	PowerupBoxRenderer->Set_Texture( HUD_CHATPBOX_TEXTURE );
	PowerupBoxRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	PowerupBoxRenderer->Enable_Additive( true );

	FontCharsClass * font = StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_TXT );
	PowerupTextRenderer = new Render2DSentenceClass();
	PowerupTextRenderer->Set_Font( font );

	WWASSERT( LeftPowerupIconList.Count() == 0 );
	WWASSERT( RightPowerupIconList.Count() == 0 );
}

static	void	Powerup_Shutdown( void )
{
	delete PowerupBoxRenderer;
	PowerupBoxRenderer = NULL;

	delete PowerupTextRenderer;
	PowerupTextRenderer = NULL;

	int i;
	for ( i = 0; i < LeftPowerupIconList.Count(); i++ ) {
		delete LeftPowerupIconList[i];
	}
	LeftPowerupIconList.Delete_All();
	for ( i = 0; i < RightPowerupIconList.Count(); i++ ) {
		delete RightPowerupIconList[i];
	}
	RightPowerupIconList.Delete_All();
}

static	void	Powerup_Add( const WCHAR * name, int number, const char * texture_name, const RectClass & uv, const Vector2 & offset, bool right_list = true )
{
	PowerupIconStruct * data = new PowerupIconStruct();
	data->Renderer = new Render2DClass();
	StringClass new_name(true);
	Strip_Path_From_Filename( new_name, texture_name );
	data->Renderer->Set_Texture( new_name );
	data->Renderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
//	data->Renderer->Enable_Additive( true );

//	SurfaceClass::SurfaceDescription surface_desc;
//	data->Renderer->Peek_Texture()->Get_Level_Description( surface_desc );
//	float size = surface_desc.Width;		// Assume square
	float size = data->Renderer->Peek_Texture()->Get_Width(); // Assume square
	data->UV = uv;
	if ( size > 0 ) {
		data->UV.Scale( Vector2( 1/size, 1/size ) );
	}
	data->IconBox = uv;
	data->IconBox += offset + Vector2( 0, -40.0f ) - data->IconBox.Upper_Left();
	data->Name = name;
	data->Number = number;
 	data->Timer = POWERUP_TIME;

	if ( right_list ) {
		RightPowerupIconList.Add( data );
	} else {
		LeftPowerupIconList.Add( data );
	}
}

void 	Powerup_Reset( void )
{
	while ( LeftPowerupIconList.Count() ) {
		delete LeftPowerupIconList[0];
		LeftPowerupIconList.Delete(0);
	}
	while ( RightPowerupIconList.Count() ) {
		delete RightPowerupIconList[0];
		RightPowerupIconList.Delete(0);
	}
}

#define		POWERUP_BOX_BASE		POWERUP_BOX_WIDTH + 6, 112
#define		POWERUP_BOX_WIDTH		80
#define		POWERUP_BOX_HEIGHT		55
#define		POWERUP_BOX_SPACING		(POWERUP_BOX_HEIGHT + 10)
#define		POWERUP_BOX_UV			127,0,207,55

static	void	Powerup_Update( void )
{
	PowerupBoxRenderer->Reset();
	PowerupTextRenderer->Reset();

	RectClass box_uv( POWERUP_BOX_UV_UL, POWERUP_BOX_UV_LR );
	box_uv.Scale( Vector2( 1.0f/128.0f, 1.0f/128.0f ) );

#define	ANIMATE_TIME	1.0f

	// Two timers
	static float LeftAnimateTimer = 0;
	static float RightAnimateTimer = 0;

	if ( LeftPowerupIconList.Count() > 0 && LeftPowerupIconList[0]->Timer < 0 ) {
		LeftAnimateTimer += TimeManager::Get_Frame_Seconds();
		if ( LeftAnimateTimer > ANIMATE_TIME ) {
			LeftAnimateTimer = 0;
			if ( LeftPowerupIconList.Count() ) {
				delete LeftPowerupIconList[0];
				LeftPowerupIconList.Delete(0);
			}
		}
	} else {
		LeftAnimateTimer = 0;
	}

	if ( RightPowerupIconList.Count() > 0 && RightPowerupIconList[0]->Timer < 0 ) {
		RightAnimateTimer += TimeManager::Get_Frame_Seconds();
		if ( RightAnimateTimer > ANIMATE_TIME ) {
			RightAnimateTimer = 0;
			if ( RightPowerupIconList.Count() ) {
				delete RightPowerupIconList[0];
				RightPowerupIconList.Delete(0);
			}
		}
	} else {
		RightAnimateTimer = 0;
	}

 	RectClass box( POWERUP_BOX_UV_UL, POWERUP_BOX_UV_LR );
	Vector2 start = Render2DClass::Get_Screen_Resolution().Lower_Right() - Vector2( POWERUP_BOX_BASE );
	box += start - box.Lower_Left();
	box -= Vector2( box.Left - 6, +75 );

	int i;
	for ( i = 0; i < MAX_ICONS && i < LeftPowerupIconList.Count(); i++ ) {

		LeftPowerupIconList[i]->Timer -= TimeManager::Get_Frame_Seconds();

		RectClass draw_box = box;

		int green = 0xFF00FF00;
		int white = 0xFFFFFFFF;

		// Slide the bottom (first) icon away
		if ( i == 0 && LeftAnimateTimer > 0.0f ) {
			green = COLOR( WWMath::Clamp( 1.0f - (LeftAnimateTimer/ANIMATE_TIME), 0, 1 ), green ) ;
			white = COLOR( WWMath::Clamp( 1.0f - (LeftAnimateTimer/ANIMATE_TIME), 0, 1 ), white ) ;
		}

#if 0		// No boxes
		// The box is drawn additive, so his color is green alpha
		int box_color = (white & 0xFF000000) | ((white >> 16) & 0x0000FF00);
		PowerupBoxRenderer->Add_Quad( draw_box, box_uv, box_color );
#endif

		// Draw powerup name
		PowerupTextRenderer->Build_Sentence( LeftPowerupIconList[i]->Name );
		PowerupTextRenderer->Set_Location( Vector2( draw_box.Left + 1, draw_box.Top + POWERUP_BOX_HEIGHT - 15 ) );
		PowerupTextRenderer->Draw_Sentence( white );

#if 0
		// Draw powerup count
		if ( LeftPowerupIconList[i]->Number != 0 ) {
			WideStringClass num(0,true);
			num.Format( L"%d", LeftPowerupIconList[i]->Number );
			PowerupTextRenderer->Build_Sentence( num );
			PowerupTextRenderer->Set_Location( Vector2( draw_box.Right - 12, draw_box.Top + 1 ) );
			PowerupTextRenderer->Draw_Sentence( white );
		}
#endif

		RectClass	icon_box = LeftPowerupIconList[i]->IconBox;
		icon_box += draw_box.Upper_Left();

		LeftPowerupIconList[i]->Renderer->Reset();
		LeftPowerupIconList[i]->Renderer->Add_Quad( icon_box, LeftPowerupIconList[i]->UV, white );

		// Drop the remaining icons down
		if ( i == 0 && LeftAnimateTimer > ANIMATE_TIME * 0.5f ) {
			box += Vector2( 0, ((2*LeftAnimateTimer/ANIMATE_TIME)-1) * POWERUP_BOX_SPACING );
		}

		box -= Vector2( 0, POWERUP_BOX_SPACING );
	}

	// Setup box for right
 	box = RectClass( POWERUP_BOX_UV_UL, POWERUP_BOX_UV_LR );
	start = Render2DClass::Get_Screen_Resolution().Lower_Right() - Vector2( POWERUP_BOX_BASE );
	box += start - box.Lower_Left();

	for ( i = 0; i < MAX_ICONS && i < RightPowerupIconList.Count(); i++ ) {

		RightPowerupIconList[i]->Timer -= TimeManager::Get_Frame_Seconds();

		RectClass draw_box = box;

		int green = 0xFF00FF00;
		int white = 0xFFFFFFFF;

		// Slide the bottom (first) icon away
		if ( i == 0 && RightAnimateTimer > 0.0f ) {
			green = COLOR( WWMath::Clamp( 1.0f - (RightAnimateTimer/ANIMATE_TIME), 0, 1 ), green ) ;
			white = COLOR( WWMath::Clamp( 1.0f - (RightAnimateTimer/ANIMATE_TIME), 0, 1 ), white ) ;
		}

#if 0		// No boxes
		// The box is drawn additive, so his color is green alpha
		int box_color = (white & 0xFF000000) | ((white >> 16) & 0x0000FF00);
		PowerupBoxRenderer->Add_Quad( draw_box, box_uv, box_color );
#endif

		// Draw powerup name
		PowerupTextRenderer->Build_Sentence( RightPowerupIconList[i]->Name );
		float left_edge = draw_box.Left + 1;
		Vector2 extents = PowerupTextRenderer->Get_Text_Extents( RightPowerupIconList[i]->Name );
		if ( left_edge + extents.X + 1> Render2DClass::Get_Screen_Resolution().Right ) {
			left_edge = Render2DClass::Get_Screen_Resolution().Right - extents.X - 1;
		}
		PowerupTextRenderer->Set_Location( Vector2( left_edge, draw_box.Top + POWERUP_BOX_HEIGHT - 15 ) );
		PowerupTextRenderer->Draw_Sentence( white );

		// Draw powerup count
		if ( RightPowerupIconList[i]->Number != 0 ) {
			WideStringClass num(0,true);
			num.Format( L"%d", RightPowerupIconList[i]->Number );
			PowerupTextRenderer->Build_Sentence( num );
			PowerupTextRenderer->Set_Location( Vector2( draw_box.Right - 12, draw_box.Top + 1 ) );
			PowerupTextRenderer->Draw_Sentence( white );
		}

		RectClass	icon_box = RightPowerupIconList[i]->IconBox;
		icon_box += draw_box.Upper_Left();

		RightPowerupIconList[i]->Renderer->Reset();
		RightPowerupIconList[i]->Renderer->Add_Quad( icon_box, RightPowerupIconList[i]->UV, green );

		// Drop the remaining icons down
		if ( i == 0 && RightAnimateTimer > ANIMATE_TIME * 0.5f ) {
			box += Vector2( 0, ((2*RightAnimateTimer/ANIMATE_TIME)-1) * POWERUP_BOX_SPACING );
		}

		box -= Vector2( 0, POWERUP_BOX_SPACING );
	}

}

static	void	Powerup_Render( void )
{
#if 0		// No boxes
	PowerupBoxRenderer->Render();
#endif

	int i;
	for ( i = 0; i < MAX_ICONS && i < LeftPowerupIconList.Count(); i++ ) {
		LeftPowerupIconList[i]->Renderer->Render();
	}
	for ( i = 0; i < MAX_ICONS && i < RightPowerupIconList.Count(); i++ ) {
		RightPowerupIconList[i]->Renderer->Render();
	}

	PowerupTextRenderer->Render();
}


/*
** Weapon Display
*/
Render2DClass * WeaponBoxRenderer;
Render2DClass * WeaponImageRenderer;
Render2DTextClass * WeaponClipCountRenderer;
Render2DTextClass * WeaponTotalCountRenderer;
Render2DSentenceClass * WeaponNameRenderer;
Vector2			WeaponBase;

WeaponClass *	_LastHUDWeapon = NULL;
int				_LastVehicleSeat = -1;


#define		WEAPON_OFFSET			100, 110
//#define		WEAPON_BOX_UV			0,0,95,95

#define		SNIPER_UV				90,0,255,53
#define		SNIPER_OFFSET			-80,-4


static void HUD_Help_Text_Init( void )
{
	//
	//	Load the font...
	//
	FontCharsClass *font = StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_BIG_TXT );
	WWASSERT (font != NULL);

	//
	//	Configure the renderer
	//
	HUDHelpTextRenderer = new Render2DSentenceClass;
	HUDHelpTextRenderer->Set_Font( font );
	HUDHelpTextExtents.Set (0, 0);
	HUDHelpTextTimer = 0;
	HUDHelpTextState = HUD_HELP_TEXT_DISPLAYING;
	return ;
}

static	void	HUD_Help_Text_Render( void )
{
	const float HUD_HELP_TEXT_FADE_TIME = 2.0F;
	const float HUD_HELP_TEXT_DISPLAY_TIME = 2.0F;

	const WideStringClass &string = HUDInfo::Get_HUD_Help_Text();
	bool is_empty = string.Is_Empty();

	//
	//	Rebuild the renderer (if necessary)
	//
	if (HUDInfo::Is_HUD_Help_Text_Dirty()) {
		HUDHelpTextState = HUD_HELP_TEXT_DISPLAYING;
		HUDInfo::Set_Is_HUD_Help_Text_Dirty( false );
		HUDHelpTextRenderer->Reset();
		if (is_empty == false) {
			HUDHelpTextRenderer->Build_Sentence( string );
			HUDHelpTextTimer = HUD_HELP_TEXT_DISPLAY_TIME;
		}
		HUDHelpTextExtents = HUDHelpTextRenderer->Get_Text_Extents (string);
	}

	//
	//	Draw the text if there's something to draw
	//
	if ( is_empty == false ) {
		HUDHelpTextRenderer->Reset_Polys();

		//
		//	Render the text above the reticle
		//
		Vector2 reticle_offset = COMBAT_CAMERA->Get_Camera_Target_2D_Offset();

		const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution();
		float x_pos = screen_rect.Center ().X - (HUDHelpTextExtents.X * 0.5F);
		float y_pos = screen_rect.Center ().Y - (HUDHelpTextExtents.Y);
		y_pos -= (RETICLE_HEIGHT * screen_rect.Height () * 0.25F);

		HUDHelpTextRenderer->Set_Location( Vector2( x_pos, y_pos ) );
		HUDHelpTextRenderer->Draw_Sentence( RGB_TO_INT32 (0, 0, 0) );
		HUDHelpTextRenderer->Set_Location( Vector2( x_pos-1, y_pos-1 ) );
		HUDHelpTextRenderer->Draw_Sentence( VRGB_TO_INT32 (HUDInfo::Get_HUD_Help_Text_Color ()) );

		//
		//	Fade the text out (if necessary)
		//
		if (HUDHelpTextState == HUD_HELP_TEXT_FADING) {
			float percent = WWMath::Clamp ((HUDHelpTextTimer / HUD_HELP_TEXT_FADE_TIME), 0.0F, 1.0F);
			HUDHelpTextRenderer->Force_Alpha (percent * 0.5F);
		} else {
			HUDHelpTextRenderer->Force_Alpha (0.5F);
		}

		//
		//	Render the quads
		//
		HUDHelpTextRenderer->Render();

		//
		//	Advance to the next state
		//
		HUDHelpTextTimer -= TimeManager::Get_Frame_Seconds ();
		if (HUDHelpTextTimer <= 0) {
			HUDHelpTextState ++;

			//
			//	Clear the text if necessary
			//
			if (HUDHelpTextState >= HUD_HELP_TEXT_DONE) {
				HUDInfo::Set_HUD_Help_Text( L"" );
				HUDHelpTextRenderer->Reset();
			} else if (HUDHelpTextState == HUD_HELP_TEXT_FADING) {
				HUDHelpTextTimer = HUD_HELP_TEXT_FADE_TIME;
			}
		}
	}

	return ;
}

static	void	HUD_Help_Text_Shutdown( void )
{
	delete HUDHelpTextRenderer;
	HUDHelpTextRenderer = NULL;
	return ;
}

static	void	Weapon_Init( void )
{
	WeaponBoxRenderer = new Render2DClass();
	WeaponBoxRenderer->Set_Texture( HUD_MAIN_TEXTURE );
	WeaponBoxRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	RectClass box_uv( WEAPON_BOX_UV_UL, WEAPON_BOX_UV_LR );
	RectClass draw_box = box_uv;
	box_uv.Scale( Vector2( 1.0f/256.0f, 1.0f/256.0f ) );
	draw_box += Render2DClass::Get_Screen_Resolution().Lower_Right() - Vector2( WEAPON_OFFSET ) - draw_box.Upper_Left();
	WeaponBoxRenderer->Add_Quad( draw_box, box_uv );
	WeaponBase = draw_box.Upper_Left();

	WeaponImageRenderer = new Render2DClass();
	WeaponImageRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

	FontCharsClass *chars_font = StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_TXT );
	WeaponNameRenderer = new Render2DSentenceClass();
	WeaponNameRenderer->Set_Font( chars_font );
//	WeaponNameRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

	Font3DInstanceClass * font = WW3DAssetManager::Get_Instance()->Get_Font3DInstance( LARGE_FONT );
	SET_REF_OWNER( font );
	WeaponClipCountRenderer = new Render2DTextClass( font );
	WeaponClipCountRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	font->Release_Ref();

	font = WW3DAssetManager::Get_Instance()->Get_Font3DInstance( SMALL_FONT );
	SET_REF_OWNER( font );
	WeaponTotalCountRenderer = new Render2DTextClass( font );
	WeaponTotalCountRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	font->Release_Ref();

	_LastHUDWeapon = NULL;
	_LastVehicleSeat = -1;


}

static	void	Weapon_Shutdown( void )
{
	delete WeaponBoxRenderer;
	WeaponBoxRenderer = NULL;

	delete WeaponImageRenderer;
	WeaponImageRenderer = NULL;

	delete WeaponNameRenderer;
	WeaponNameRenderer = NULL;

	delete WeaponClipCountRenderer;
	WeaponClipCountRenderer = NULL;

	delete WeaponTotalCountRenderer;
	WeaponTotalCountRenderer = NULL;
}

#define		CLIP_ROUNDS_OFFSET		15,27
#define		TOTAL_ROUNDS_OFFSET		65,34
#define		WEAPON_NAME_OFFSET		-10, 95

float	LastClipCount = 0;
float	CenterClipCountTimer = 0;
const float	CENTER_CLIP_COUNT_TIME	= 2.0f;

enum {
	SEAT_DRIVER = 0,
	SEAT_GUNNER,
	SEAT_PASENGER,
};

const char * _Seat_Textures[3] = {
	"hud_driverseat.tga",	// SEAT_DRIVER = 0,
	"hud_gunseat.tga",	// SEAT_GUNNER,
	"hud_passseat.tga",	// SEAT_PASENGER,
};

static	void	Weapon_Reset( void ) 
{
	_LastHUDWeapon = (WeaponClass *)0xFFFFFFFF;		// force weapon to re-draw next
	_LastVehicleSeat = -1;	// force vehicle seat to re-draw next
}

static	void	Weapon_Update( void ) 
{
	WeaponClass * weapon = NULL;
	if ( COMBAT_STAR ) {
		weapon = COMBAT_STAR->Get_Weapon();

		if ( COMBAT_STAR->Get_Vehicle() ) {
			weapon = COMBAT_STAR->Get_Vehicle()->Get_Weapon();
		}

	}

	WeaponClipCountRenderer->Reset();
	WeaponTotalCountRenderer->Reset();

	if ( weapon != NULL ) {
//		StringClass	text;
//		text.Format( "%03d", weapon->Get_Clip_Rounds() );
		WCHAR tmp_text[5];
		if ( weapon->Get_Clip_Rounds() == -1 ) {
			//text.Format( "999", weapon->Get_Total_Rounds() );
			tmp_text[0]='9';
			tmp_text[1]='9';
			tmp_text[2]='9';
			tmp_text[3]=0;
		} else {
			Generate_WChar_Text_From_Number(tmp_text,3,3,weapon->Get_Clip_Rounds());
		}

		WeaponClipCountRenderer->Set_Location( WeaponBase + Vector2( CLIP_ROUNDS_OFFSET ) );
		WeaponClipCountRenderer->Draw_Text( tmp_text );

		if ( LastClipCount != weapon->Get_Clip_Rounds() ) {
			LastClipCount = weapon->Get_Clip_Rounds();
			CenterClipCountTimer = CENTER_CLIP_COUNT_TIME;
		}

		if ( CenterClipCountTimer > 0 ) {
			// Also draw the above at the center

			Vector2	center_clip_count_offset = Render2DClass::Get_Screen_Resolution().Center();
			center_clip_count_offset.X *= 1.5;

			float fade = WWMath::Clamp( CenterClipCountTimer, 0, 1 );

			RectClass uv;
			uv.Set( BULLET_ICON_UV_UL, BULLET_ICON_UV_LR );
			RectClass draw = uv;
			uv.Scale( INFO_UV_SCALE );
			draw += center_clip_count_offset + BULLET_ICON_OFFSET - draw.Upper_Left();
			InfoRenderer->Add_Quad( draw, uv, COLOR( fade ) );

			WeaponClipCountRenderer->Set_Location( draw.Upper_Right() + Vector2( 4, 15 ) );
			WeaponClipCountRenderer->Draw_Text( tmp_text, COLOR( fade ) );

			CenterClipCountTimer -= TimeManager::Get_Frame_Seconds();
		}


		if ( weapon->Get_Total_Rounds() == -1 ) {
			//text.Format( "999", weapon->Get_Total_Rounds() );
			tmp_text[0]='9';
			tmp_text[1]='9';
			tmp_text[2]='9';
			tmp_text[3]=0;
		} else {
//			text.Format( "%03d", weapon->Get_Total_Rounds() - weapon->Get_Clip_Rounds() );
			Generate_WChar_Text_From_Number(tmp_text,3,3,weapon->Get_Total_Rounds() - weapon->Get_Clip_Rounds());
		}
		WeaponTotalCountRenderer->Set_Location( WeaponBase + Vector2( TOTAL_ROUNDS_OFFSET ) );
		WeaponTotalCountRenderer->Draw_Text( tmp_text );
	}

	// If in vehicle, don't draw the weapon icon and name,
	// draw a seat icon and the vehicle name
	if ( COMBAT_STAR->Get_Vehicle() ) {
		int seat = SEAT_PASENGER;

		if ( COMBAT_STAR->Get_Vehicle()->Get_Driver() == COMBAT_STAR ) {
			seat = SEAT_DRIVER;
		} else if ( COMBAT_STAR->Get_Vehicle()->Get_Gunner() == COMBAT_STAR ) {
			seat = SEAT_GUNNER;
		}

		if ( _LastVehicleSeat != seat ) {
			_LastVehicleSeat = seat;
			_LastHUDWeapon = (WeaponClass *)0xFFFFFFFF;		// force weapon to re-draw next

			WeaponImageRenderer->Reset();
			StringClass filename = _Seat_Textures[seat];
			WeaponImageRenderer->Set_Texture( filename );
			Vector2		offset( 16, 34 );
			RectClass icon_box( 0,0,64,64 );
			icon_box += WeaponBase + offset - icon_box.Upper_Left();
			WeaponImageRenderer->Add_Quad( icon_box );

			// Draw Name Backdrop
			WeaponNameRenderer->Reset();
			WideStringClass name(COMBAT_STAR->Get_Vehicle()->Get_Vehicle_Name(),true);
			WeaponNameRenderer->Build_Sentence( name );
			Vector2 text_size = WeaponNameRenderer->Get_Text_Extents( name );
			WeaponNameRenderer->Set_Location( Render2DClass::Get_Screen_Resolution().Lower_Right() - text_size );
			WeaponNameRenderer->Draw_Sentence();

		}

	} else if ( _LastHUDWeapon != weapon ) {		// Update the weapon icon
		WWPROFILE( "Weapon Change" );
		_LastHUDWeapon = weapon;
		_LastVehicleSeat = -1;	// force vehicle seat to re-draw next


		WeaponImageRenderer->Reset();
		WeaponNameRenderer->Reset();

		if ( weapon != NULL ) {

			StringClass filename( HUD_WEAPONS_TEXTURE, true );
			RectClass	uv( 0,64,128,128 );
			Vector2		offset( -24, 38  );

			const WeaponDefinitionClass * def = weapon->Get_Definition();
			if ( !def->IconTextureName.Is_Empty() ) {
				Strip_Path_From_Filename( filename, def->IconTextureName );
				uv = def->IconTextureUV;
				offset = def->IconOffset;

#if 0		// Used to find offsets
				static	Vector2	tweak_add(0,0);
				Vector2 add(0,0);
				add.X += Input::Get_Amount( INPUT_FUNCTION_MOVE_LEFT ) - Input::Get_Amount( INPUT_FUNCTION_MOVE_RIGHT );
				add.Y += Input::Get_Amount( INPUT_FUNCTION_MOVE_FORWARD ) - Input::Get_Amount( INPUT_FUNCTION_MOVE_BACKWARD );
				tweak_add -= add;
				offset += tweak_add;
				_LastHUDWeapon = NULL;
				if ( add.Length() != 0 ) {
					Debug_Say(( "%s offset %f %f\n", def->Get_Name(), offset.X, offset.Y ));
				}
#endif
			}

			RectClass icon_box = uv;
			icon_box += WeaponBase + offset - icon_box.Upper_Left();

			{
				StringClass new_name(true);
				Strip_Path_From_Filename( new_name, filename );
				WeaponImageRenderer->Set_Texture( new_name );
			}

//			SurfaceClass::SurfaceDescription surface_desc;
//			WeaponImageRenderer->Peek_Texture()->Get_Level_Description( surface_desc );
//			float size = surface_desc.Width;		// Assume square
			float size = WeaponImageRenderer->Peek_Texture()->Get_Width(); // Assume square
			if ( size > 0 ) {
				uv.Scale( Vector2( 1/size, 1/size ) );
			}

			int color = 0xFF00FF00; // green
			WeaponImageRenderer->Add_Quad( icon_box, uv, color );

			// Draw Name Backdrop

			// Right justify Name

			WideStringClass name(0,true); // = L"Rocket Launcher";
			name = TranslateDBClass::Get_String( def->IconNameID );
			WeaponNameRenderer->Build_Sentence( name );
			Vector2 text_size = WeaponNameRenderer->Get_Text_Extents( name ) + Vector2( 1, 0 );
			WeaponNameRenderer->Set_Location( Render2DClass::Get_Screen_Resolution().Lower_Right() - text_size );
			WeaponNameRenderer->Draw_Sentence();
		}
	}
}

static	void	Weapon_Render( void )
{
	WeaponBoxRenderer->Render();
	WeaponImageRenderer->Render();
	WeaponNameRenderer->Render();
	WeaponClipCountRenderer->Render();
	WeaponTotalCountRenderer->Render();
}


/*
** Weapon Chart Display
*/
Render2DClass * WeaponChartBoxRenderer;
static	DynamicVectorClass<Render2DClass *>	WeaponChartIcons;
Render2DSentenceClass * WeaponChartKeynameRenderer;
float	WeaponChartTimer;
#define	WEAPON_CHART_TIME	3.0f

static	void	Weapon_Chart_Init( void )
{
	WeaponChartBoxRenderer = new Render2DClass();
	WeaponChartBoxRenderer->Enable_Texturing( false );
	WeaponChartBoxRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

	FontCharsClass *chars_font = StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_TXT );
	WeaponChartKeynameRenderer = new Render2DSentenceClass();
	WeaponChartKeynameRenderer->Set_Font( chars_font );
}

static	void	Weapon_Chart_Shutdown( void )
{
	delete WeaponChartBoxRenderer;
	WeaponChartBoxRenderer = NULL;

	for ( int i = 0; i < WeaponChartIcons.Count(); i++) {
		delete WeaponChartIcons[i];
		WeaponChartIcons[i] = NULL;
	}
	WeaponChartIcons.Delete_All();

	delete WeaponChartKeynameRenderer;
	WeaponChartKeynameRenderer = NULL;
}

Vector2	WeaponChartBase( 0.11f, 0.05f );
Vector2	WeaponChartSpacing( 0.075f, 0.05f );
float		WeaponChartIconScale = 0.45f / 640.0f;

static	void	Clear_Weapon_Chart_Icons( void )
{
	if ( WeaponChartIcons.Count() != 0 ) {
		int i;
		for ( i = 0; i < WeaponChartIcons.Count(); i++) {
			delete WeaponChartIcons[i];
			WeaponChartIcons[i] = NULL;
		}
		WeaponChartIcons.Delete_All();
		WeaponChartKeynameRenderer->Reset();
	}
}

static	void	Build_Weapon_Chart_Icons( void )
{
	WeaponBagClass * weapon_bag = COMBAT_STAR->Get_Weapon_Bag();

	Clear_Weapon_Chart_Icons();

	float screen_scale = Render2DClass::Get_Screen_Resolution().Width();
	Vector2	pos = WeaponChartBase * screen_scale;

	// For each column...
	for ( int column = 1; column <= 10; column++ ) {

		// Add column header
		int key = Input::Get_Primary_Key_For_Function(INPUT_FUNCTION_SELECT_WEAPON_0 + (column%10) );
		WideStringClass name(0,true);
		Input::Get_Translated_Key_Name( key, name );
		WeaponChartKeynameRenderer->Build_Sentence( name );
		Vector2 text_size = WeaponChartKeynameRenderer->Get_Text_Extents( name );
		Vector2 text_offset = pos - text_size/2 - Vector2( 0, WeaponChartSpacing.Y * screen_scale * 0.75f );
		text_offset.X = (int)text_offset.X;
		text_offset.Y = (int)text_offset.Y;
		WeaponChartKeynameRenderer->Set_Location( text_offset );
		WeaponChartKeynameRenderer->Draw_Sentence();

		// Draw weapons in that column
		for ( int i = 0; i < weapon_bag->Get_Count(); i++ ) {
			WeaponClass *	weapon = weapon_bag->Peek_Weapon( i );
			if ( weapon != NULL ) {

				if ( (int)weapon->Get_Key_Number() != (column%10) ) {
					continue;
				}

				StringClass filename( HUD_WEAPONS_TEXTURE, true );
				RectClass	uv( 0,64,128,128 );
				Vector2		offset( -40, 40 );

				const WeaponDefinitionClass * def = weapon->Get_Definition();
				if ( !def->IconTextureName.Is_Empty() ) {
					Strip_Path_From_Filename( filename, def->IconTextureName );
					uv = def->IconTextureUV;
					offset = def->IconOffset;
				}

				RectClass icon_box = uv;
	//			icon_box += pos - icon_box.Upper_Left();
	//			icon_box += offset - icon_box.Upper_Left() + pos;
				icon_box += pos - icon_box.Center();
				icon_box.Scale_Relative_Center( WeaponChartIconScale * screen_scale );
				pos.Y += WeaponChartSpacing.Y * screen_scale;

				// Draw Icon
				Render2DClass * renderer = new Render2DClass();
				WeaponChartIcons.Add( renderer );
				renderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
				StringClass new_name(true);
				Strip_Path_From_Filename( new_name, filename );
				renderer->Set_Texture( new_name );
				if ( renderer->Peek_Texture() != NULL ) {
//					SurfaceClass::SurfaceDescription surface_desc;
//					renderer->Peek_Texture()->Get_Level_Description( surface_desc );
//					float size = surface_desc.Width;		// Assume square
					float size = renderer->Peek_Texture()->Get_Width(); // Assume square
					if ( size > 0 ) {
						uv.Scale( Vector2( 1/size, 1/size ) );
					}
				}
				renderer->Add_Quad( icon_box, uv );
			}
		}

		// Goto next column
		pos.Y = WeaponChartBase.Y * screen_scale;
		pos.X += WeaponChartSpacing.X * screen_scale;
	}

}

bool	ForceChartRebuild = false;

void	HUDClass::Force_Weapon_Chart_Update( void )
{
	ForceChartRebuild = true;
}

void	HUDClass::Force_Weapon_Chart_Display( void )
{
	WeaponChartTimer = WEAPON_CHART_TIME;
}

static	void	Weapon_Chart_Update( void )
{
	if ( COMBAT_STAR == NULL || COMBAT_STAR->Get_Weapon_Bag() == NULL || COMBAT_STAR->Get_Vehicle() != NULL ) {
		Clear_Weapon_Chart_Icons();
		return;
	}

	int i;
	WeaponBagClass * weapon_bag = COMBAT_STAR->Get_Weapon_Bag();
	if ( weapon_bag->HUD_Is_Changed() ) {
		weapon_bag->HUD_Reset_Changed();
		WeaponChartTimer = WEAPON_CHART_TIME;
	}

	WeaponChartTimer -= TimeManager::Get_Frame_Seconds();

	if ( WeaponChartTimer <= 0 ) {
		return;
	}

	// Rebuild the icons if necessary
	// subtract 1 because the first is always null
	if ( ForceChartRebuild || (weapon_bag->Get_Count()-1 != WeaponChartIcons.Count()) ) {
		ForceChartRebuild = false;
		Debug_Say(( "Build icons\n" ));
		Build_Weapon_Chart_Icons();
	}

	float alpha = WWMath::Clamp( WeaponChartTimer, 0, 1 );

	WeaponChartBoxRenderer->Reset();

	// For each column...
	int index = 0;
	for ( int column = 1; column <= 10; column++ ) {

		// Update intensities in that column
		for ( i = 0; i < weapon_bag->Get_Count(); i++ ) {
			WeaponClass *	weapon = weapon_bag->Peek_Weapon( i );
			if ( weapon != NULL ) {

				if ( (int)weapon->Get_Key_Number() != (column%10) ) {
					continue;
				}

				long color = 0x000FF00;		// Dim
				if ( weapon_bag->Get_Index() == i ) {
					color = 0x0000FF00;			// Bright
				}

				// Draw yellow if empty or reloading
//				if ( weapon->Is_Reloading() || weapon->Is_Switching() || weapon->Get_Total_Rounds() == 0 ) {
//				if ( weapon->Is_Switching() || weapon->Get_Total_Rounds() == 0 ) {
				if ( weapon->Get_Total_Rounds() == 0 ) {
					color = 0x00FFFF00;		// Dim yellow
					if ( weapon_bag->Get_Index() == i ) {
						color = 0x00FFFF00;			// Bright yellow
					}
				}

#if 01
				if ( weapon_bag->Get_Index() == i ) {
					color = COLOR( alpha, color );
				} else {
					color = COLOR( alpha * 0.5f, color );
				}
#endif


				WeaponChartIcons[index]->Force_Color( color );
				index++;
			}
		}
	}

	WeaponChartKeynameRenderer->Force_Alpha( alpha );
}

static	void	Weapon_Chart_Render( void )
{
	if ( WeaponChartTimer <= 0 ) {
		return;
	}

	WeaponChartBoxRenderer->Render();
	for ( int i = 0; i < WeaponChartIcons.Count(); i++) {
		WeaponChartIcons[i]->Render();
	}
	WeaponChartKeynameRenderer->Render();
}

/*
** Damage Display
*/
Render2DClass * DamageRenderer;

const int NUM_DAMAGE_INDICATORS	= 8;
float	DamageIndicatorIntensity[ NUM_DAMAGE_INDICATORS ];
bool	DamageIndicatorIntensityChanging;
bool	DamageIndicatorOrientation;

static	void	Damage_Reset( void ) 
{
	for ( int i = 0; i < NUM_DAMAGE_INDICATORS; i++ ) {
		DamageIndicatorIntensity[ i ] = 0;
	}
	DamageIndicatorIntensityChanging = true;
	CombatManager::Clear_Star_Damage_Direction();
}

static	void	Damage_Init( void ) 
{
	DamageRenderer = new Render2DClass();
	DamageRenderer->Set_Texture( HUD_MAIN_TEXTURE );
	DamageRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	DamageRenderer->Enable_Additive( true );

	Damage_Reset();
}

static	void	Damage_Shutdown( void )
{
	delete DamageRenderer;
	DamageRenderer = NULL;
}

static	void	Damage_Add_Indicator( int index, float start_x, float start_y, float end_x, float end_y )
{
	Vector2	vert[4];
	vert[0] = Vector2( start_x, start_y );
	vert[1] = Vector2( start_x, end_y );
	vert[2] = Vector2( end_x, start_y );
	vert[3] = Vector2( end_x, end_y );

	if ( !CombatManager::Is_First_Person() ) {
		for ( int i = 0; i < 4; i++ ) {
			vert[i].X *= WWMath::Fabs(1 + vert[i].Y);		// Skew
			vert[i].Y /= 2;										// Squash vertical
			vert[i].Y += 0.25f;									// Lower
		}
	}

	for ( int i = 0; i < 4; i++ ) {
		// Numbers are -0.5 to 0.5, switch them to pixels, to match the info_renderer mode
		vert[i].X	= (vert[i].X + 0.5f) * Render2DClass::Get_Screen_Resolution().Width();
		vert[i].Y	= (vert[i].Y + 0.5f) * Render2DClass::Get_Screen_Resolution().Height();
	}

	RectClass uv( DAMAGE_1_UV_UL, DAMAGE_1_UV_LR );
	if ( index & 1 ) {
		uv = RectClass( DAMAGE_2_UV_UL, DAMAGE_2_UV_LR );
	}
	uv.Scale( INFO_UV_SCALE );

	unsigned long color_bits = (int)(DamageIndicatorIntensity[index] * 255) & 0x000000FF;
  	unsigned long color = color_bits | color_bits<<8 | color_bits<<16;

	switch (index) {
		case 3:
		case 4:	DamageRenderer->Add_Quad(	vert[1], vert[3], vert[0], vert[2], uv, color );
					break;
		case 5:
		case 6:	DamageRenderer->Add_Quad(	vert[0], vert[1], vert[2], vert[3], uv, color );
					break;
		case 7:
		case 0:	DamageRenderer->Add_Quad(	vert[2], vert[0], vert[3], vert[1], uv, color );
					break;
		case 1:
		case 2:	DamageRenderer->Add_Quad(	vert[3], vert[2], vert[1], vert[0], uv, color );
					break;
	}

}

static	void	Damage_Update( void )
{
	DamageRenderer->Reset();

	if (Input::Are_Damage_Indicators_Enabled () == false) {
		return ;
	}

	int new_damage = CombatManager::Get_Star_Damage_Direction();
	//new_damage = 0xFF;

	if ( new_damage != 0 ) {
		DamageIndicatorIntensityChanging = true;
	}

	if ( !DamageIndicatorIntensityChanging &&
		DamageIndicatorOrientation == CombatManager::Is_First_Person() ) {
		return;
	}

//	DamageRenderer->Reset();

	DamageIndicatorOrientation = CombatManager::Is_First_Person();
	DamageIndicatorIntensityChanging = false;

	// Update the intensities
	CombatManager::Clear_Star_Damage_Direction();
	for ( int i = 0; i < NUM_DAMAGE_INDICATORS; i++ ) {
		if ( new_damage & (1<<i) ) {					// Apply new damage
			DamageIndicatorIntensity[ i ] = 1;
			DamageIndicatorIntensityChanging = true;
		} else {
			if ( DamageIndicatorIntensity[ i ] > 0 ) {
				DamageIndicatorIntensity[ i ] -= TimeManager::Get_Frame_Seconds();	// and fade it away
				DamageIndicatorIntensity[ i ] = WWMath::Clamp( DamageIndicatorIntensity[ i ], 0, 1 );
				DamageIndicatorIntensityChanging = true;
			}
		}
	}

	// Redraw the indicators

#if 0
	const float HORIZ_WIDTH		=	 58.0f/640.0f;
	const float HORIZ_HEIGHT	=	 20.0f/480.0f;
	const float VERT_WIDTH		=	 20.0f/640.0f;
	const float VERT_HEIGHT		=	 58.0f/480.0f;
	const float OFFSET_X		=	220.0f/640.0f;
	const float OFFSET_Y		=	190.0f/480.0f;

	const float DIAG_WIDTH		=	 46.0f/640.0f;
	const float DIAG_HEIGHT		=	 46.0f/480.0f;
	const float DIAG_OFFSET_X	=	160.0f/640.0f;
	const float DIAG_OFFSET_Y	=	130.0f/480.0f;
#else
	float HORIZ_WIDTH		=	HORIZ_DAMAGE_SIZE.U/640.0f;
	float HORIZ_HEIGHT		=	HORIZ_DAMAGE_SIZE.V/480.0f;
	float VERT_WIDTH		=	VERT_DAMAGE_SIZE.U/640.0f;
	float VERT_HEIGHT		=	VERT_DAMAGE_SIZE.V/480.0f;
	float OFFSET_X			=	HV_DAMAGE_OFFSET.U/640.0f;
	float OFFSET_Y			=	HV_DAMAGE_OFFSET.V/480.0f;

	float DIAG_WIDTH		=	DIAG_DAMAGE_SIZE.U/640.0f;
	float DIAG_HEIGHT		=	DIAG_DAMAGE_SIZE.V/480.0f;
	float DIAG_OFFSET_X		=	DIAG_DAMAGE_OFFSET.U/640.0f;
	float DIAG_OFFSET_Y		=	DIAG_DAMAGE_OFFSET.V/480.0f;
#endif

	Damage_Add_Indicator( 0,	-HORIZ_WIDTH/2,					-OFFSET_Y-HORIZ_HEIGHT/2,		HORIZ_WIDTH/2,					-OFFSET_Y+HORIZ_HEIGHT/2		);
	Damage_Add_Indicator( 2,	OFFSET_X-VERT_WIDTH/2,			-VERT_HEIGHT/2,					OFFSET_X+VERT_WIDTH/2,			VERT_HEIGHT/2					);
	Damage_Add_Indicator( 4,	-HORIZ_WIDTH/2,					OFFSET_Y-HORIZ_HEIGHT/2,		HORIZ_WIDTH/2,					OFFSET_Y+HORIZ_HEIGHT/2			);
	Damage_Add_Indicator( 6,	-OFFSET_X-VERT_WIDTH/2,			-VERT_HEIGHT/2,					-OFFSET_X+VERT_WIDTH/2,			VERT_HEIGHT/2					);
	Damage_Add_Indicator( 1,	DIAG_OFFSET_X-DIAG_WIDTH/2,		-DIAG_OFFSET_Y-DIAG_HEIGHT/2,	DIAG_OFFSET_X+DIAG_WIDTH/2,		-DIAG_OFFSET_Y+DIAG_HEIGHT/2	);
	Damage_Add_Indicator( 3,	DIAG_OFFSET_X-DIAG_WIDTH/2,		DIAG_OFFSET_Y-DIAG_HEIGHT/2,	DIAG_OFFSET_X+DIAG_WIDTH/2,		DIAG_OFFSET_Y+DIAG_HEIGHT/2 	);
	Damage_Add_Indicator( 5,	-DIAG_OFFSET_X-DIAG_WIDTH/2,	DIAG_OFFSET_Y-DIAG_HEIGHT/2,	-DIAG_OFFSET_X+DIAG_WIDTH/2,	DIAG_OFFSET_Y+DIAG_HEIGHT/2		);
	Damage_Add_Indicator( 7,	-DIAG_OFFSET_X-DIAG_WIDTH/2,	-DIAG_OFFSET_Y-DIAG_HEIGHT/2,	-DIAG_OFFSET_X+DIAG_WIDTH/2,	-DIAG_OFFSET_Y+DIAG_HEIGHT/2	);
}

void	HUDClass::Damage_Render( void )
{
	DamageRenderer->Render();
}

/*
** Target Display
*/
Render2DClass * TargetRenderer;
Render2DClass * TargetBoxRenderer;
Render2DSentenceClass * TargetNameRenderer;
Render2DSentenceClass * InfoDebugRenderer;
WideStringClass TargetNameString;			// Cache the current target name string so that we know when the string changes
Vector2 TargetNameLocation;	// Cache the name location as well

static	void	Target_Init( void )
{
	TargetRenderer = new Render2DClass();
	TargetRenderer->Set_Texture( HUD_MAIN_TEXTURE );
	TargetRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

	TargetBoxRenderer = new Render2DClass();
	TargetBoxRenderer->Enable_Texturing( false );
	TargetBoxRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

	FontCharsClass *chars_font = StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_TXT );
	TargetNameRenderer = new Render2DSentenceClass();
	TargetNameRenderer->Set_Font( chars_font );
	InfoDebugRenderer = new Render2DSentenceClass();
	InfoDebugRenderer->Set_Font( chars_font );
//	TargetNameRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
}


static	void	Target_Shutdown( void )
{
	delete TargetRenderer;
	TargetRenderer = NULL;

	delete TargetBoxRenderer;
	TargetBoxRenderer = NULL;

	delete TargetNameRenderer;
	TargetNameRenderer = NULL;
	TargetNameString="";

	delete InfoDebugRenderer;
	InfoDebugRenderer = NULL;

}

		bool	InfoDebug = false;

static RectClass Get_Target_Box( PhysicalGameObj * obj );
static void	Target_Box_Edge( const Vector2 & a, const Vector2 & b, unsigned int color );

static	void	Target_Update( void )
{
	TargetRenderer->Reset();
	TargetBoxRenderer->Reset();
//	TargetNameRenderer->Reset();
	InfoDebugRenderer->Reset();

	HUDInfo::Update_Info_Object();

	static float box_zoom_size = 0;

	DamageableGameObj * d_obj = HUDInfo::Get_Info_Object();
	if ( d_obj != NULL ) {

		PhysicalGameObj * p_obj = d_obj->As_PhysicalGameObj();
		BuildingGameObj * building = d_obj->As_BuildingGameObj();

		DamageableGameObj * obj = d_obj;

		if ( obj && COMBAT_STAR ) {

			box_zoom_size += TimeManager::Get_Frame_Seconds() * 4;
			box_zoom_size = WWMath::Clamp( box_zoom_size, 0, 1 );

			int color = HUDGlobalSettingsDef::Get_Instance()->Get_No_Relation_Color().Convert_To_ARGB();
			if ( p_obj != NULL ) {
				if ( COMBAT_STAR->Is_Teammate(p_obj) ) {
					color = HUDGlobalSettingsDef::Get_Instance()->Get_Friendly_Color().Convert_To_ARGB();
				} else if ( COMBAT_STAR->Is_Enemy(p_obj) ) {
					color = HUDGlobalSettingsDef::Get_Instance()->Get_Enemy_Color().Convert_To_ARGB();
				}
			}

			if ( building != NULL ) {
//				if ( building->Is_GDI() ) {
				if ( COMBAT_STAR->Is_Teammate(building) ) {
					color = HUDGlobalSettingsDef::Get_Instance()->Get_Friendly_Color().Convert_To_ARGB();
//				} else if ( building->Is_Nod() ) {
				} else if ( COMBAT_STAR->Is_Enemy(building) ) {
					color = HUDGlobalSettingsDef::Get_Instance()->Get_Enemy_Color().Convert_To_ARGB();
				}
			}

			RectClass box = Render2DClass::Get_Screen_Resolution();

			if ( obj->As_PhysicalGameObj() ) {
				box = Get_Target_Box( obj->As_PhysicalGameObj() );
			} else {
				// Build a box for the buildings
				box.Scale_Relative_Center( 0.3f );
				// center on cursor reticle
				Vector2 new_center = (COMBAT_CAMERA->Get_Camera_Target_2D_Offset() * 0.5f );
				new_center.Y *= -1.0f;
				new_center += Vector2( 0.5f, 0.5f );
				new_center.X *= Render2DClass::Get_Screen_Resolution().Right;
				new_center.Y *= Render2DClass::Get_Screen_Resolution().Bottom;
				box += new_center - box.Center();
			}

			// Scale the box to let it zoom in
			if ( box_zoom_size < 1 ) {
				box.Scale_Relative_Center( 1 + ((1-box_zoom_size) * 0.3f) );
			}
			box.Snap_To_Units( Vector2( 1, 1 ) );

			// Clamp box to fit on screen;
			RectClass res = Render2DClass::Get_Screen_Resolution();
			if ( box.Top < 0 ) {
				box.Top = 0;
			}
			if ( box.Left < 0 ) {
				box.Left = 0;
			}
			if ( box.Right > res.Right-1 ) {
				box.Right = res.Right-1;
			}
			// Leave room for info at the bottom
			if ( box.Bottom > res.Bottom - 26 ) {
				box.Bottom = res.Bottom - 26;
			}

			Target_Box_Edge( box.Upper_Left(), box.Upper_Right(), color );
			Target_Box_Edge( box.Upper_Left(), box.Lower_Left(), color );
			Target_Box_Edge( box.Lower_Right(), box.Upper_Right(), color );
			Target_Box_Edge( box.Lower_Right(), box.Lower_Left(), color );

			RectClass	uv( TARGET_HEALTH_R_UV_UL, TARGET_HEALTH_R_UV_LR );
			RectClass	draw = uv;
			RectClass	draw2;
			bool draw_health = obj->Is_Health_Bar_Displayed();
			if ( draw_health ) {
				float health_percent = 0;
	//			float health = 0;
				const DefenseObjectClass	* def = obj->Get_Defense_Object();
				if ( def ) {
	//				health = def->Get_Health();
	//				health_percent = WWMath::Clamp( def->Get_Health() / def->Get_Health_Max(), 0, 1 );

					float health_max = def->Get_Health_Max();
					float shield_max = def->Get_Shield_Strength_Max();
					float max = health_max + shield_max;
					float health = def->Get_Health();
					float shield = def->Get_Shield_Strength();
	//				if ( health_max && ((health/health_max) < 0.25) ) {
	//					shield *= ((health/health_max) / 0.25f);
	//				}
					// Display less shield as health drops
					if ( health_max ) {
						shield *= (health/health_max);
					}
					float total = health + shield;
					if ( max != 0 ) {
						health_percent = total / max;
						health_percent = WWMath::Clamp( health_percent, 0, 1 );
					}
				}
				int health_color = Get_Health_Color( health_percent );

				// Draw Health
				static float power_flash = 0;
				power_flash += TimeManager::Get_Frame_Seconds();
				power_flash = WWMath::Wrap( power_flash, 0, 2 );

				RectClass uv3( POWER_OUT_ICON_UV_UL, POWER_OUT_ICON_UV_LR );
				RectClass draw3 = uv3;
				uv3.Scale( INFO_UV_SCALE );

				RectClass	black( HEALTH_BACK_UV_UL, HEALTH_BACK_UV_LR );
				black.Scale( INFO_UV_SCALE );

				uv.Scale( INFO_UV_SCALE );
	//			draw += box.Upper_Left() - draw.Lower_Left() - Vector2( 0, 1 );
				draw += box.Lower_Left() - draw.Upper_Left() + Vector2( 0, 18 );
	//			draw += Vector2( (box.Width() - draw.Width())/2 + 10, 0 );		// Center
				draw += Vector2( (box.Width() - draw.Width())/2 + 2, 0 );		// Center
				draw3 += draw.Center() - draw3.Center();

				// Draw background
				draw.Inflate( Vector2( 1, 1 ) );
				TargetRenderer->Add_Quad( draw, black );
				draw.Inflate( Vector2( -1, -1 ) );

				draw.Right = draw.Left + draw.Width() * health_percent;
				uv.Right = uv.Left + uv.Width() * health_percent;
				TargetRenderer->Add_Quad( draw, uv, health_color );
				// power out
				if ( power_flash < 1 ) {
	//				TargetRenderer->Add_Quad( draw3, uv3 );
				}

				uv.Set( TARGET_HEALTH_L_UV_UL, TARGET_HEALTH_L_UV_LR );
				draw2 = uv;
	#if 0
				uv.Scale( INFO_UV_SCALE );
				draw2 += draw.Upper_Left() - draw2.Upper_Right();
				TargetRenderer->Add_Quad( draw2, uv, health_color );

				WideStringClass num(0,true);
				if ( health < 1 && health > 0 ) {
					health = 1;
				}
				num.Format( L"%d", (int)health );
				TargetNameRenderer->Build_Sentence( num );
				Vector2 text_size = TargetNameRenderer->Get_Text_Extents( num );
				text_size.U -= 1;
				text_size.V = -1;
				draw -= text_size;
				draw.Snap_To_Units( Vector2( 1, 1 ) );
				TargetNameRenderer->Set_Location( draw.Upper_Left() );
				TargetNameRenderer->Draw_Sentence( health_color );
	#endif
			}

			// Draw Name
			uv.Set( TARGET_NAME_UV_UL, TARGET_NAME_UV_LR );
			draw = uv;
			uv.Scale( INFO_UV_SCALE );
			draw += box.Lower_Left() - draw.Upper_Left() + Vector2( 0, 1 );
			draw += Vector2( (box.Width() - draw.Width())/2 + 10, 0 );
			draw.Snap_To_Units( Vector2( 1, 1 ) );
			TargetRenderer->Add_Quad( draw, uv, color );

			if ( obj->Get_Translated_Name_ID() != 0 ) {
				TDBObjClass *translate_obj = TranslateDBClass::Find_Object( obj->Get_Translated_Name_ID() );
				if ( translate_obj ) {
					WideStringClass translate_string=translate_obj->Get_String();

					if ( building != NULL && HUDInfo::Get_Info_Object_Is_MCT() ) {
						translate_string=TRANSLATE( IDS_Enc_Struct_Nod_MCT_Name );
					}

					Vector2 string_loc=draw.Upper_Left() + Vector2( 3.0f,1.0f );
					if (translate_string!=TargetNameString) {
						TargetNameRenderer->Reset();
						TargetNameString=translate_string;
						TargetNameLocation = string_loc;
						TargetNameRenderer->Build_Sentence( TargetNameString );
						TargetNameRenderer->Set_Location( TargetNameLocation );
						TargetNameRenderer->Set_Base_Location( TargetNameLocation );
						TargetNameRenderer->Draw_Sentence( color );
					}
					if (string_loc!=TargetNameLocation) {
						TargetNameLocation=string_loc;
						TargetNameRenderer->Set_Base_Location( TargetNameLocation );
					}
				}
				else {
					TargetNameRenderer->Reset();
					TargetNameString="";
					TargetNameLocation = Vector2(0.0f,0.0f);
				}
			}
			else {
				TargetNameRenderer->Reset();
				TargetNameString="";
				TargetNameLocation = Vector2(0.0f,0.0f);
			}

			// Draw Team Icon
			int team = PLAYERTYPE_GDI;
			if ( p_obj != NULL ) {
				team = p_obj->Get_Player_Type();
			}

			if ( building != NULL ) {
				if ( building->Is_GDI() ) {
					team = PLAYERTYPE_GDI;
				} else if ( building->Is_Nod() ) {
					team = PLAYERTYPE_NOD;
				} else {
					team = PLAYERTYPE_NEUTRAL;
				}
			}

			if ( team == PLAYERTYPE_NOD ) {
				uv.Set( NOD_ICON_UV_UL, NOD_ICON_UV_LR );
			} else if ( team == PLAYERTYPE_GDI ) {
				uv.Set( GDI_ICON_UV_UL, GDI_ICON_UV_LR );
			} else {
				uv.Set( NEUTRAL_ICON_UV_UL, NEUTRAL_ICON_UV_LR );
			}
			draw2 = uv;
			uv.Scale( INFO_UV_SCALE );
			draw2 += draw.Upper_Left() - draw2.Upper_Right();
			TargetRenderer->Add_Quad( draw2, uv );

			// Draw chevrons
			bool draw_chevrons = false;
			if ( p_obj != NULL && p_obj->Is_HUD_Pokable_Indicator_Enabled() ) {
				draw_chevrons = true;
			}
			if ( obj->As_SmartGameObj() != NULL &&
				  obj->As_SmartGameObj()->As_VehicleGameObj() != NULL &&
				  COMBAT_STAR->Is_Permitted_To_Enter_Vehicle() &&
				  obj->As_SmartGameObj()->As_VehicleGameObj()->Is_Entry_Permitted( COMBAT_STAR ) ) {
				draw_chevrons = true;
			}
			if ( draw_chevrons ) {
				RectClass enterable_box( Vector2(0,0), TARGET_ENTERABLE_SIZE );
				enterable_box += Vector2( box.Center().X - enterable_box.Center().X ,
								box.Top - enterable_box.Bottom );
				static float enterable_bounce = 0;
				enterable_bounce += TimeManager::Get_Frame_Seconds() * 5;
				enterable_bounce = WWMath::Wrap( enterable_bounce, 0, DEG_TO_RADF( 360 ) );
				enterable_box += Vector2( 0, TARGET_ENTERABLE_BOUNCE * (WWMath::Sin( enterable_bounce ) - 1) );

				uv.Set( TARGET_ENTERABLE_UV_UL, TARGET_ENTERABLE_UV_LR );
				uv.Scale( INFO_UV_SCALE );

				color = Get_Health_Color( 1 );

//				TargetRenderer->Add_Quad( enterable_box, uv );
				enterable_box -= Vector2( 0, enterable_box.Height() * 0.6f );
				TargetRenderer->Add_Tri(	enterable_box.Upper_Right(), enterable_box.Upper_Left(), enterable_box.Center(),
					uv.Upper_Right(), uv.Upper_Left(), uv.Center(), color );
				enterable_box += Vector2( 0, enterable_box.Height() * 0.6f );
				TargetRenderer->Add_Tri(	enterable_box.Upper_Right(), enterable_box.Upper_Left(), enterable_box.Center(),
					uv.Upper_Right(), uv.Upper_Left(), uv.Center(), color );
				enterable_box += Vector2( 0, enterable_box.Height() * 0.6f );
				TargetRenderer->Add_Tri(	enterable_box.Upper_Right(), enterable_box.Upper_Left(), enterable_box.Center(),
					uv.Upper_Right(), uv.Upper_Left(), uv.Center(), color );
			}
		}

		if ( InfoDebug ) {
			StringClass	info(0,true);
			obj->Get_Information( info );

			WideStringClass str(0,true);
			str.Convert_From( info );
			InfoDebugRenderer->Build_Sentence( str );
			InfoDebugRenderer->Set_Location( Vector2( 520, 240 ) );
//			InfoDebugRenderer->Set_Base_Location( Vector2( 520, 240 ) );
			InfoDebugRenderer->Draw_Sentence();
		}

	} else {
		TargetNameRenderer->Reset();
		TargetNameString="";
		TargetNameLocation = Vector2(0.0f,0.0f);

		box_zoom_size = 0;
		HUDInfo::Clear_Info_Object();
	}
}

static	void	Target_Render( void )
{
	TargetRenderer->Render();
	TargetBoxRenderer->Render();
	TargetNameRenderer->Render();
	InfoDebugRenderer->Render();
}

static RectClass Get_Target_Box( PhysicalGameObj * obj )
{
	// Lets draw a box around the target
	WWASSERT( obj );
	PhysClass * po = obj->Peek_Physical_Object();

	Vector2	top(0,0);
	Vector2	bottom(0,0);

	if ( po != NULL ) {
		// Get the Obj AABox in Camera Space
		AABoxClass obj_box;
		po->Get_Shadow_Blob_Box( &obj_box );
		Matrix3D obj_tm = po->Get_Transform();

		Matrix3D boxview_tm;
		Matrix3D boxview_inv_tm;
		boxview_tm.Look_At(COMBAT_CAMERA->Get_Transform().Get_Translation(),obj_tm * obj_box.Center,0.0f);
		boxview_tm.Get_Orthogonal_Inverse(boxview_inv_tm);
		Matrix3D obj_to_boxview_tm = boxview_inv_tm * obj_tm;

		AABoxClass boxview_box;
		obj_to_boxview_tm.Transform_Center_Extent_AABox( obj_box.Center, obj_box.Extent, &boxview_box.Center, &boxview_box.Extent );

		Matrix3D camera_tm = COMBAT_CAMERA->Get_Transform();
		Matrix3D camera_inv_tm;
		camera_tm.Get_Orthogonal_Inverse(camera_inv_tm);
		Matrix3D boxview_to_camera_tm = camera_inv_tm * boxview_tm;

		AABoxClass camera_box;
		boxview_to_camera_tm.Transform_Center_Extent_AABox( boxview_box.Center, boxview_box.Extent, &camera_box.Center, &camera_box.Extent);

		camera_box.Extent.Z = 0.0f;
		Vector3 ctop = camera_box.Center - camera_box.Extent;
		Vector3 cbottom = camera_box.Center + camera_box.Extent;

		Vector3	temp;
		COMBAT_CAMERA->Project_Camera_Space_Point( temp, ctop );
		top.X = temp.X;
		top.Y = temp.Y;
		COMBAT_CAMERA->Project_Camera_Space_Point( temp, cbottom );
		bottom.X = temp.X;
		bottom.Y = temp.Y;
	}

	// Get Box in proper convention
	RectClass	screen = Render2DClass::Get_Screen_Resolution();
	top.X = top.X * 0.5f + 0.5f;
	top.Y = top.Y * -0.5f + 0.5f;
	bottom.X = bottom.X * 0.5f + 0.5f;
	bottom.Y = bottom.Y * -0.5f + 0.5f;
	float temp = top.Y;
	top.Y = bottom.Y;
	bottom.Y = temp;
	static RectClass info_box;
	info_box.Set( top.X * screen.Right, top.Y * screen.Bottom, bottom.X * screen.Right, bottom.Y * screen.Bottom );			// convention 0,0 - 1,1

	return info_box;
}

static void	Target_Box_Edge( const Vector2 & a, const Vector2 & b, unsigned int color )
{
	float percent = 0.2f;
	Vector2 a_ = b - a;
	a_ *= percent;
	a_ += a;
	TargetBoxRenderer->Add_Line( a, a_, 2, color );
	Vector2 b_ = a - b;
	b_ *= percent;
	b_ += b;
	TargetBoxRenderer->Add_Line( b, b_, 2, color );
}

/*
** Score Display
*/
/*
Render2DTextClass * ScoreRenderer;

static	void	Score_Init( void )
{
	Font3DInstanceClass * font = WW3DAssetManager::Get_Instance()->Get_Font3DInstance( LARGE_FONT );
	SET_REF_OWNER( font );
	ScoreRenderer = new Render2DTextClass( font );
	ScoreRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	font->Release_Ref();
}

static	void	Score_Shutdown( void )
{
	delete ScoreRenderer;
	ScoreRenderer = NULL;
}

static	void	Score_Update( void )
{
	ScoreRenderer->Reset();

	//TSS2001e if ( COMBAT_STAR ) {
	if ( COMBAT_STAR && COMBAT_STAR->Get_Player_Data() ) {
		int score = COMBAT_STAR->Get_Player_Data()->Get_Score();
//		WideStringClass	scorestring;
//		scorestring.Format( L"%d", score );
		WCHAR score_string[12];	// 12 digits ought to be enough...
		Generate_WChar_Text_From_Number(score_string,sizeof(score_string),false,score);

		Vector2 position = Render2DClass::Get_Screen_Resolution().Center();
		position.Y = Render2DClass::Get_Screen_Resolution().Bottom;
		Vector2 size = ScoreRenderer->Get_Text_Extents( score_string );
		size.X *= 0.5f;
		position -= size;
		ScoreRenderer->Set_Location( position );
		ScoreRenderer->Draw_Text( score_string );
	}

}

static	void	Score_Render( void )
{
	ScoreRenderer->Render();
}
*/

/*
** Objective Display
*/
DynamicVectorClass<Render2DClass *>	ObjectivePogRenderers;
Render2DClass * ObjectiveArrowRenderer;
Render2DSentenceClass * ObjectiveTextRenderer;

int	CurrentObjectiveIndex = 0;
void * CurrentObjective = NULL;
static int CachedObjectiveIndex=-1;
int CachedRange=0;

#define	OBJECTIVE_ARROW_TEXTURE		"HUD_obje_arrow.TGA"

static	void	Objective_Init( void )
{
	ObjectiveArrowRenderer = new Render2DClass();
	ObjectiveArrowRenderer->Set_Texture( OBJECTIVE_ARROW_TEXTURE );
	ObjectiveArrowRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

	FontCharsClass *font = StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_TXT );
	ObjectiveTextRenderer = new Render2DSentenceClass();
	ObjectiveTextRenderer->Set_Font( font );

	CurrentObjectiveIndex=0;
	CurrentObjective=NULL;
	CachedObjectiveIndex=-1;
	CachedRange=0;
}

static	void	Objective_Release_Pogs( void )
{
	for ( int i = 0; i < ObjectivePogRenderers.Count(); i++ ) {
		delete ObjectivePogRenderers[i];
		ObjectivePogRenderers[i] = NULL;
	}
	ObjectivePogRenderers.Delete_All();
}

static	void	Objective_Shutdown( void )
{
	Objective_Release_Pogs();

	delete ObjectiveArrowRenderer;
	ObjectiveArrowRenderer = NULL;

	delete ObjectiveTextRenderer;
	ObjectiveTextRenderer = NULL;
}

#define	POG_FLY_TIME	2.0f

static	void	Objective_Update( void )
{
	RectClass	pog_box( 0, 0, 64, 64 );
	pog_box += Render2DClass::Get_Screen_Resolution().Upper_Right() - pog_box.Upper_Right();
	pog_box += Vector2( -16, 8 );

#define	POG_SPACING		Vector2( 10, 0 )

	bool rebuild = false;

	int objective_count = ObjectiveManager::Get_Num_HUD_Objectives();
	if ( objective_count > 0 ) {
		// maintain the index
		if ( CurrentObjective != ObjectiveManager::Get_Objective( CurrentObjectiveIndex ) ) {
			CurrentObjectiveIndex = 0;
			rebuild = true;
#if 0 // no, just go for the first
			// Try to find the last objective
			for ( int i = 0; i < objective_count; i++ ) {
				if ( CurrentObjective == ObjectiveManager::Get_Objective( i ) ) {
					CurrentObjectiveIndex = i;
				}
			}
#endif
		}

		if ( IS_MISSION && Input::Get_State( INPUT_FUNCTION_CYCLE_POG ) ) {
			rebuild = true;
			CurrentObjectiveIndex++;
			if ( CurrentObjectiveIndex >= objective_count ) {
				CurrentObjectiveIndex = 0;
			}
		}

 		CurrentObjective = ObjectiveManager::Get_Objective( CurrentObjectiveIndex );
	} else {
		if (CurrentObjectiveIndex!=0 || CurrentObjective!=NULL) {
			rebuild=true;
		}
		CurrentObjectiveIndex = 0;
		CurrentObjective = NULL;
	}

	// re-Create the Pogs, if needed
	if ( rebuild || ObjectiveManager::Are_HUD_Objectives_Changed() )
	{
//		Debug_Say(( "Rebuild Hud Pogs\n" ));

		Objective_Release_Pogs();
		// Reset text
		ObjectiveTextRenderer->Reset();
		CachedRange=0;
		CachedObjectiveIndex=-1;

		int count = objective_count;
		pog_box += POG_SPACING * (float)count;

		bool dont_clear = false;

		for ( int i = count - 1; i >= 0; i-- ) {
			int index = (i + CurrentObjectiveIndex) % objective_count;
			pog_box -= POG_SPACING;
			// Add Pog
			Render2DClass * renderer = new Render2DClass();
			if ( renderer ) {
				renderer->Set_Texture( ObjectiveManager::Get_HUD_Objectives_Pog_Texture_Name( index ) );
				renderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
				float	age = ObjectiveManager::Get_HUD_Objectives_Age( index );
				if ( age < POG_FLY_TIME ) {
					Vector2 offset = Render2DClass::Get_Screen_Resolution().Center() - Render2DClass::Get_Screen_Resolution().Upper_Right();
					offset.X *= 0.75f;
					float fly = WWMath::Clamp( (2.0f-(2.0f*age/POG_FLY_TIME)), 0, 1 );
					offset *= fly;
					pog_box += offset;
					renderer->Add_Quad( pog_box );
					pog_box -= offset;
					dont_clear = true;

					// AND, make an extra renderer for the radar star
					Render2DClass * renderer = new Render2DClass();
					if ( renderer ) {
						renderer->Set_Texture( "HUD_STAR.TGA" );
						renderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

						RectClass	star_box( -32, -32, 32, 32 );
						star_box.Scale( fly );

						Vector2	star_fly_start = Render2DClass::Get_Screen_Resolution().Center();
						Vector2	star_fly_end = Render2DClass::Get_Screen_Resolution().Lower_Left();
						star_fly_start.X *= 0.85f;
						star_fly_start.Y *= 1.175f;
						star_fly_end.X += 40.0f;
						star_fly_end.Y *= 0.8f;

						star_box += star_fly_end;
						Vector2	offset = star_fly_start - star_fly_end;
						offset *= fly;
						star_box += offset;

						Vector3 color3( 0,1,0 );
						if ( ObjectiveManager::Get_Objective(index) != NULL ) {
							color3 = ObjectiveManager::Get_Objective(index)->Type_To_Color();
						}
						unsigned int color = color3.Convert_To_ARGB();
						renderer->Add_Quad( star_box, color  );
						ObjectivePogRenderers.Add( renderer );
					}

				} else {
					renderer->Add_Quad( pog_box );
				}
				ObjectivePogRenderers.Add( renderer );
			}
		}
		if ( !dont_clear ) {
			ObjectiveManager::Clear_HUD_Objectives_Changed();
		}
	}

	ObjectiveArrowRenderer->Reset();

	float range = 0;
	int count = objective_count;
	if ( count > 0 ) {
		// Create Arrow
		float angle = 0;
		Vector3	objective_pos = ObjectiveManager::Get_HUD_Objectives_Location( CurrentObjectiveIndex );

		if ( COMBAT_STAR ) {
			Vector3 rel_pos;
			Matrix3D::Inverse_Transform_Vector( COMBAT_STAR->Get_Transform(), objective_pos, &rel_pos );
			angle = ::atan2( rel_pos.Y, rel_pos.X );
			range = rel_pos.Length();
		}

		Vector2 arrow_vertex;
		arrow_vertex.X = WWMath::Fast_Sin( angle + DEG_TO_RAD( 180 + 45 ) );
		arrow_vertex.Y = WWMath::Fast_Cos( angle + DEG_TO_RAD( 180 + 45 ) );
		Vector2 verts[4];
		verts[0] = Vector2( arrow_vertex.X, arrow_vertex.Y );
		verts[1] = Vector2( arrow_vertex.Y, -arrow_vertex.X );
		verts[2] = Vector2( -arrow_vertex.Y, arrow_vertex.X );
		verts[3] = Vector2( -arrow_vertex.X, -arrow_vertex.Y );
		Vector2 offset;
		offset.Y = WWMath::Fast_Sin( -angle + DEG_TO_RAD( -90 ) );
		offset.X = WWMath::Fast_Cos( -angle + DEG_TO_RAD( -90 ) );
		offset *= 35;
		offset += pog_box.Center();

		const float SIN_45=0.70710678118654752440084436210485f;
		for ( int i = 0; i < 4; i++ ) {
			verts[i] *= 0.5f * 16 / SIN_45;//::sin( DEG_TO_RAD( 45 ) );
			verts[i] += offset;
		}
		ObjectiveArrowRenderer->Add_Quad( verts[0], verts[1], verts[2], verts[3] );
	}

	int irange=(int)(range);
	irange=(irange/10)*10;
	if (CachedObjectiveIndex!=CurrentObjectiveIndex || irange!=CachedRange) {
		ObjectiveTextRenderer->Reset();

		if (objective_count>0) {
			CachedRange=irange;
			CachedObjectiveIndex=CurrentObjectiveIndex;

			// Draw message
			Vector2 position = pog_box.Lower_Left();
			position += Vector2( 0, -15 );
			WideStringClass str(ObjectiveManager::Get_HUD_Objectives_Message( CurrentObjectiveIndex ),true);
			ObjectiveTextRenderer->Build_Sentence( str );
			Vector2 text_size = ObjectiveTextRenderer->Get_Text_Extents( str );
			position.X = (int)(pog_box.Center().X - (text_size.X/2));
			ObjectiveTextRenderer->Set_Location( position );
			ObjectiveTextRenderer->Draw_Sentence();

			// Draw range
			position = pog_box.Lower_Left();
			str.Format( TRANSLATE(IDS_HUD_RANGE), irange );
			ObjectiveTextRenderer->Build_Sentence( str );
			text_size = ObjectiveTextRenderer->Get_Text_Extents( str );
			position.X = (int)(pog_box.Center().X - (text_size.X/2));
			ObjectiveTextRenderer->Set_Location( position );
			ObjectiveTextRenderer->Draw_Sentence();
		}
		else {
			CachedRange=0;
			CachedObjectiveIndex=-1;
		}
	}
}

static	void	Objective_Render( void )
{
	for ( int i = 0; i < ObjectivePogRenderers.Count(); i++ ) {
		ObjectivePogRenderers[i]->Render();
	}
	ObjectiveArrowRenderer->Render();
	ObjectiveTextRenderer->Render();
}

/*
** Info Display
*/
struct InfoEditorField {
	const char * Name;
	Vector2	*	 Value;
	bool operator == (InfoEditorField const & rec) const	{ return false; }
	bool operator != (InfoEditorField const & rec) const	{ return true; }
};

static	DynamicVectorClass<InfoEditorField>	InfoEditorFieldList;

#define	INFO_EDITOR_ADD(x)	  field.Name = #x; field.Value = &x; InfoEditorFieldList.Add( field );

static	void	Info_Editor_Init( void )
{
#if 0
	InfoEditorField field;

	INFO_EDITOR_ADD( INFO_OFFSET );
	INFO_EDITOR_ADD( FRAME_1_UV_UL );
	INFO_EDITOR_ADD( FRAME_1_UV_LR );
	INFO_EDITOR_ADD( FRAME_1_OFFSET );
	INFO_EDITOR_ADD( FRAME_2_UV_UL );
	INFO_EDITOR_ADD( FRAME_2_UV_LR );
	INFO_EDITOR_ADD( FRAME_2_OFFSET );
	INFO_EDITOR_ADD( FRAME_3_UV_UL );
	INFO_EDITOR_ADD( FRAME_3_UV_LR );
	INFO_EDITOR_ADD( FRAME_3_OFFSET );
	INFO_EDITOR_ADD( FRAME_4_UV_UL );
	INFO_EDITOR_ADD( FRAME_4_UV_LR );
	INFO_EDITOR_ADD( FRAME_4_OFFSET );
	INFO_EDITOR_ADD( FRAME_5_UV_UL );
	INFO_EDITOR_ADD( FRAME_5_UV_LR );
	INFO_EDITOR_ADD( FRAME_5_OFFSET );
	INFO_EDITOR_ADD( FRAME_6_UV_UL );
	INFO_EDITOR_ADD( FRAME_6_UV_LR );
	INFO_EDITOR_ADD( FRAME_6_OFFSET );
	INFO_EDITOR_ADD( HEALTH_BACK_UV_UL );
	INFO_EDITOR_ADD( HEALTH_BACK_UV_LR );
	INFO_EDITOR_ADD( HEALTH_BACK_UL );
	INFO_EDITOR_ADD( HEALTH_BACK_LR );
	INFO_EDITOR_ADD( GRADIENT_BLACK_UV_UL );
	INFO_EDITOR_ADD( GRADIENT_BLACK_UV_LR );
	INFO_EDITOR_ADD( HEALTH_TEXT_BACK_UL );
	INFO_EDITOR_ADD( HEALTH_TEXT_BACK_LR );
	INFO_EDITOR_ADD( HEALTH_UV_UL );
	INFO_EDITOR_ADD( HEALTH_UV_LR );
	INFO_EDITOR_ADD( HEALTH_OFFSET );
	INFO_EDITOR_ADD( SHIELD_UV_UL );
	INFO_EDITOR_ADD( SHIELD_UV_LR );
	INFO_EDITOR_ADD( SHIELD_OFFSET );
	INFO_EDITOR_ADD( KEY_1_UV_UL );
	INFO_EDITOR_ADD( KEY_1_UV_LR );
	INFO_EDITOR_ADD( KEY_1_OFFSET );
	INFO_EDITOR_ADD( KEY_2_UV_UL );
	INFO_EDITOR_ADD( KEY_2_UV_LR );
	INFO_EDITOR_ADD( KEY_2_OFFSET );
	INFO_EDITOR_ADD( KEY_3_UV_UL );
	INFO_EDITOR_ADD( KEY_3_UV_LR );
	INFO_EDITOR_ADD( KEY_3_OFFSET );
	INFO_EDITOR_ADD( HEALTH_CROSS_1_UV_UL );
	INFO_EDITOR_ADD( HEALTH_CROSS_1_UV_LR );
	INFO_EDITOR_ADD( HEALTH_CROSS_1_OFFSET );
	INFO_EDITOR_ADD( HEALTH_CROSS_2_UV_UL );
	INFO_EDITOR_ADD( HEALTH_CROSS_2_UV_LR );
	INFO_EDITOR_ADD( HEALTH_CROSS_2_OFFSET );
	INFO_EDITOR_ADD( TIME_BACK_UL );
	INFO_EDITOR_ADD( TIME_BACK_LR );
	INFO_EDITOR_ADD( TIME_CLOCK_UV_UL );
	INFO_EDITOR_ADD( TIME_CLOCK_UV_LR );
	INFO_EDITOR_ADD( TIME_CLOCK_OFFSET );
	INFO_EDITOR_ADD( WEAPON_BOX_UV_UL );
	INFO_EDITOR_ADD( WEAPON_BOX_UV_LR );
	INFO_EDITOR_ADD( WEAPON_BOX_OFFSET );
	INFO_EDITOR_ADD( POWER_OUT_ICON_UV_UL );
	INFO_EDITOR_ADD( POWER_OUT_ICON_UV_LR );
	INFO_EDITOR_ADD( POWER_OUT_ICON_OFFSET );
	INFO_EDITOR_ADD( REPAIR_ICON_UV_UL );
	INFO_EDITOR_ADD( REPAIR_ICON_UV_LR );
	INFO_EDITOR_ADD( REPAIR_ICON_OFFSET );
	INFO_EDITOR_ADD( GDI_ICON_UV_UL );
	INFO_EDITOR_ADD( GDI_ICON_UV_LR );
	INFO_EDITOR_ADD( GDI_ICON_OFFSET );
	INFO_EDITOR_ADD( NOD_ICON_UV_UL );
	INFO_EDITOR_ADD( NOD_ICON_UV_LR );
	INFO_EDITOR_ADD( NOD_ICON_OFFSET );
	INFO_EDITOR_ADD( NEUTRAL_ICON_UV_UL );
	INFO_EDITOR_ADD( NEUTRAL_ICON_UV_LR );
	INFO_EDITOR_ADD( NEUTRAL_ICON_OFFSET );
	INFO_EDITOR_ADD( BULLET_ICON_UV_UL );
	INFO_EDITOR_ADD( BULLET_ICON_UV_LR );
	INFO_EDITOR_ADD( BULLET_ICON_OFFSET );
	INFO_EDITOR_ADD( RADAR_CENTER_OFFSET );
	INFO_EDITOR_ADD( DAMAGE_1_UV_UL );
	INFO_EDITOR_ADD( DAMAGE_1_UV_LR );
	INFO_EDITOR_ADD( DAMAGE_2_UV_UL );
	INFO_EDITOR_ADD( DAMAGE_2_UV_LR );
	INFO_EDITOR_ADD( HORIZ_DAMAGE_SIZE );
	INFO_EDITOR_ADD( VERT_DAMAGE_SIZE );
	INFO_EDITOR_ADD( DIAG_DAMAGE_SIZE );
	INFO_EDITOR_ADD( HV_DAMAGE_OFFSET );
	INFO_EDITOR_ADD( DIAG_DAMAGE_OFFSET );
	INFO_EDITOR_ADD( POWERUP_BOX_UV_UL );
	INFO_EDITOR_ADD( POWERUP_BOX_UV_LR );
	INFO_EDITOR_ADD( RADAR_RINGS_UV_UL );
	INFO_EDITOR_ADD( RADAR_RINGS_UV_LR );
	INFO_EDITOR_ADD( RADAR_RINGS_L_OFFSET );
	INFO_EDITOR_ADD( RADAR_RINGS_R_OFFSET );
	INFO_EDITOR_ADD( RADAR_STAR_UV_UL );
	INFO_EDITOR_ADD( RADAR_STAR_UV_LR );
	INFO_EDITOR_ADD( RADAR_STAR_OFFSET );
	INFO_EDITOR_ADD( RADAR_SQUARE_UV_UL );
	INFO_EDITOR_ADD( RADAR_SQUARE_UV_LR );
	INFO_EDITOR_ADD( RADAR_SQUARE_OFFSET);
	INFO_EDITOR_ADD( RADAR_TRIANGLE_UV_UL);
	INFO_EDITOR_ADD( RADAR_TRIANGLE_UV_LR);
	INFO_EDITOR_ADD( RADAR_TRIANGLE_OFFSET );
	INFO_EDITOR_ADD( RADAR_CIRCLE_UV_UL );
	INFO_EDITOR_ADD( RADAR_CIRCLE_UV_LR );
	INFO_EDITOR_ADD( RADAR_CIRCLE_OFFSET );
	INFO_EDITOR_ADD( RADAR_BRACKET_UV_UL );
	INFO_EDITOR_ADD( RADAR_BRACKET_UV_LR );
	INFO_EDITOR_ADD( RADAR_BRACKET_OFFSET );
	INFO_EDITOR_ADD( RADAR_SWEEP_UV_UL );
	INFO_EDITOR_ADD( RADAR_SWEEP_UV_LR );
	INFO_EDITOR_ADD( RADAR_SWEEP_OFFSET );
	INFO_EDITOR_ADD( TARGET_HEALTH_L_UV_UL );
	INFO_EDITOR_ADD( TARGET_HEALTH_L_UV_LR );
	INFO_EDITOR_ADD( TARGET_HEALTH_R_UV_UL );
	INFO_EDITOR_ADD( TARGET_HEALTH_R_UV_LR );
	INFO_EDITOR_ADD( TARGET_HEALTH_OFFSET );
	INFO_EDITOR_ADD( TARGET_NAME_UV_UL );
	INFO_EDITOR_ADD( TARGET_NAME_UV_LR );
	INFO_EDITOR_ADD( TARGET_NAME_OFFSET );
#endif
}

static	void	Info_Editor_Update( void )
{
return;
	bool changed = false;
	static	int index = 0;

	static float _move = 0;
	float move = Input::Get_Amount( INPUT_FUNCTION_MOVE_LEFT ) - Input::Get_Amount( INPUT_FUNCTION_MOVE_RIGHT );
	if ( _move != move ) {
		_move = move;
		index += (int)-move;
		index = (int)WWMath::Wrap( index, (float)0, (float)(InfoEditorFieldList.Count()) );
		if ( move != 0 ) {
			changed = true;
		}
	}

	float _forward = 0;
	float forward = Input::Get_Amount( INPUT_FUNCTION_MOVE_FORWARD ) - Input::Get_Amount( INPUT_FUNCTION_MOVE_BACKWARD );
	forward *= 10;
	forward = WWMath::Clamp( forward, -1, 1 );
	if ( _forward != forward ) {
		_forward = forward;
		InfoEditorFieldList[index].Value->V += forward;
		if ( forward != 0 ) {
			changed = true;
		}
	}

	float _left = 0;
	float left = Input::Get_Amount( INPUT_FUNCTION_TURN_LEFT ) - Input::Get_Amount( INPUT_FUNCTION_TURN_RIGHT );
	left *= 10;
	left = WWMath::Clamp( left, -1, 1 );
	if ( _left != left ) {
		_left = left;
		InfoEditorFieldList[index].Value->U += left;
		if ( left != 0 ) {
			changed = true;
		}
	}

	if ( changed ) {
		Debug_Say(( "Vector2 %s( %d, %d );\n", InfoEditorFieldList[index].Name, (int)InfoEditorFieldList[index].Value->U, (int)InfoEditorFieldList[index].Value->V ));
	}

}

static	void	Info_Editor_Shutdown( void )
{
	while ( InfoEditorFieldList.Count() > 0 ) {
		Debug_Say(( "Vector2 %s( %d, %d );\n", InfoEditorFieldList[0].Name, (int)InfoEditorFieldList[0].Value->U, (int)InfoEditorFieldList[0].Value->V ));
		InfoEditorFieldList.Delete(0);
	}
}


/*
**
*/
Render2DTextClass * InfoHealthCountRenderer;
Render2DTextClass * InfoShieldCountRenderer;

Vector2	InfoBase(0,0);

float	LastHealth = 0;
float	CenterHealthTimer = 0;
const float	CENTER_HEALTH_TIME	= 2.0f;

static	void	Info_Init( void )
{
	Info_Editor_Init();

	InfoRenderer = new Render2DClass();
	InfoRenderer->Set_Texture( HUD_MAIN_TEXTURE );
	InfoRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

	Font3DInstanceClass * font = WW3DAssetManager::Get_Instance()->Get_Font3DInstance( LARGE_FONT );
	SET_REF_OWNER( font );
	InfoHealthCountRenderer = new Render2DTextClass( font );
	InfoHealthCountRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	font->Release_Ref();

	font = WW3DAssetManager::Get_Instance()->Get_Font3DInstance( SMALL_FONT );
	SET_REF_OWNER( font );
	InfoShieldCountRenderer = new Render2DTextClass( font );
	InfoShieldCountRenderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	font->Release_Ref();

	InfoBase = Render2DClass::Get_Screen_Resolution().Lower_Left() + Vector2( INFO_OFFSET );
}

static	void	Info_Shutdown( void )
{
	Info_Editor_Shutdown() ;

	delete InfoRenderer;
	InfoRenderer = NULL;

	delete InfoHealthCountRenderer;
	InfoHealthCountRenderer = NULL;

	delete InfoShieldCountRenderer;
	InfoShieldCountRenderer = NULL;
}

static	void	Info_Update_Health_Shield( void )
{
	// Draw Health
	float health = 0;
	float health_percent = 0;
	float shield = 0;
	float shield_percent = 0;

	if ( COMBAT_STAR ) {
		const DefenseObjectClass	* def = COMBAT_STAR->Get_Defense_Object();
		if ( COMBAT_STAR->Get_Vehicle() ) {
			def = COMBAT_STAR->Get_Vehicle()->Get_Defense_Object();
		}
		if ( def && def->Get_Health_Max() ) {
			health = def->Get_Health();
			health_percent = WWMath::Clamp( def->Get_Health() / def->Get_Health_Max(), 0, 1 );
		}
		if ( def && def->Get_Shield_Strength_Max() ) {
			shield = def->Get_Shield_Strength();
			shield_percent = WWMath::Clamp( def->Get_Shield_Strength() / def->Get_Shield_Strength_Max(), 0, 1 );
		}
	}

	RectClass uv;
	RectClass draw;

	// Draw Health Bar
	uv.Set( HEALTH_UV_UL, HEALTH_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + HEALTH_OFFSET - draw.Upper_Left();

	// Scale bars
	static float _last_health_percent = 0;
	float diff = health_percent - _last_health_percent;
	float max_change = TimeManager::Get_Frame_Seconds();
	_last_health_percent += WWMath::Clamp( diff, -max_change, max_change );
	uv.Right = uv.Left + uv.Width() * _last_health_percent;
	draw.Right = draw.Left + draw.Width() * _last_health_percent;

	// Get bar color
	float color_percent = MAX( _last_health_percent, health_percent );
	InfoRenderer->Add_Quad( draw, uv, Get_Health_Color( color_percent ) );

	uv.Set( GRADIENT_BLACK_UV_UL, GRADIENT_BLACK_UV_LR );
	uv.Scale( INFO_UV_SCALE );
	draw.Set( HEALTH_TEXT_BACK_UL, HEALTH_TEXT_BACK_LR );
	draw += InfoBase;
	InfoRenderer->Add_Quad( draw, uv );

	// Draw Cross
	static float flash = 0;
	flash += TimeManager::Get_Frame_Seconds() * 4;
	if ( flash > 2 ) {
		flash -= 2;
	}
	if ( health_percent > 0.25f ) {
		flash = 0;
	}
	float intensity = flash;
	if ( flash > 1 ) {
		intensity = 2 - flash;
	}

	int health_color = Get_Health_Color( color_percent );

	uv.Set( HEALTH_CROSS_1_UV_UL, HEALTH_CROSS_1_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + HEALTH_CROSS_1_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv, COLOR( intensity, health_color ) );

	RectClass	uv2;
	uv2.Set( HEALTH_CROSS_2_UV_UL, HEALTH_CROSS_2_UV_LR );
	uv2.Scale( INFO_UV_SCALE );
	InfoRenderer->Add_Quad( draw, uv2, COLOR( 1-intensity, health_color ) );

	// Draw Health Number
	InfoHealthCountRenderer->Reset();
//	StringClass	text;
	if ( health < 1 && health > 0 ) {
		health = 1;
	}
	//text.Format( "%03d", (int)health );
	long lhealth=WWMath::Float_To_Long(health);
	WCHAR tmp_text[5];
	Generate_WChar_Text_From_Number(tmp_text,4,3,lhealth);

	InfoHealthCountRenderer->Set_Location( draw.Upper_Right() + Vector2( 4,4) );
	InfoHealthCountRenderer->Draw_Text( tmp_text, health_color );

	if ( health != LastHealth || health_percent <= 0.25f ) {
		LastHealth = health;
		CenterHealthTimer = CENTER_HEALTH_TIME;
	}

	if ( CenterHealthTimer > 0 ) {
		// Also draw the above at the center

		Vector2	health_center_offset = Render2DClass::Get_Screen_Resolution().Center();
		health_center_offset.X *= 0.5;
		health_center_offset.Y -= draw.Height() / 2;

		health_center_offset -= HEALTH_CROSS_1_OFFSET;

		float fade = WWMath::Clamp( CenterHealthTimer, 0, 1 );

	 	uv.Set( GRADIENT_BLACK_UV_UL, GRADIENT_BLACK_UV_LR );
		uv.Scale( INFO_UV_SCALE );
		draw.Set( HEALTH_TEXT_BACK_UL, HEALTH_TEXT_BACK_LR );
		draw += health_center_offset;
		InfoRenderer->Add_Quad( draw, uv, COLOR( fade ) );

		uv.Set( HEALTH_CROSS_1_UV_UL, HEALTH_CROSS_1_UV_LR );
		draw = uv;
		uv.Scale( INFO_UV_SCALE );
		draw += health_center_offset + HEALTH_CROSS_1_OFFSET - draw.Upper_Left();
		InfoRenderer->Add_Quad( draw, uv, COLOR( fade * intensity, health_color ) );
		InfoRenderer->Add_Quad( draw, uv2, COLOR( fade * (1-intensity), health_color ) );

		InfoHealthCountRenderer->Set_Location( draw.Upper_Right() + Vector2( 4,4) );
		InfoHealthCountRenderer->Draw_Text( tmp_text, COLOR( fade, health_color ) );
		CenterHealthTimer -= TimeManager::Get_Frame_Seconds();
	}

	// Draw Shield
	static float _last_shield_percent = 0;
	diff = shield_percent - _last_shield_percent;
	max_change = TimeManager::Get_Frame_Seconds();
	_last_shield_percent += WWMath::Clamp( diff, -max_change, max_change );
	shield_percent = _last_shield_percent;
	uv.Right = uv.Left + uv.Width() * shield_percent;
	draw.Right = draw.Left + draw.Width() * shield_percent;

	if ( shield_percent > 0 ) {
		#define	TOTAL_SHIELD_MOVEMENT		80
		for ( float percent = 0; percent < shield_percent; percent += 0.1f ) {
			uv.Set( SHIELD_UV_UL, SHIELD_UV_LR );
			draw = uv;
			uv.Scale( INFO_UV_SCALE );
			draw += InfoBase + SHIELD_OFFSET - draw.Upper_Left();
			draw += Vector2( (int)(-percent * TOTAL_SHIELD_MOVEMENT), 0 );
			InfoRenderer->Add_Quad( draw, uv );
		}

		uv.Set( SHIELD_UV_UL, SHIELD_UV_LR );
		draw = uv;
		uv.Scale( INFO_UV_SCALE );
		draw += InfoBase + SHIELD_OFFSET - draw.Upper_Left();
		draw += Vector2( (int)(-shield_percent * TOTAL_SHIELD_MOVEMENT), 0 );
		InfoRenderer->Add_Quad( draw, uv );

		// Draw Shield Number
		InfoShieldCountRenderer->Reset();
//		StringClass	text;
//		text.Format( "%03d", (int)shield );
		long lshield=WWMath::Float_To_Long(shield);
		WCHAR tmp_text[5];
		Generate_WChar_Text_From_Number(tmp_text,4,3,lshield);
		InfoShieldCountRenderer->Set_Location( draw.Upper_Left() + Vector2( 4,4) );
		InfoShieldCountRenderer->Draw_Text( tmp_text );
	} else {
		InfoShieldCountRenderer->Reset();
	}
}

static	void	Info_Update( void )
{
//	Info_Editor_Update();

	// Clear the renderer
	InfoRenderer->Reset();

	// Draw the Frame
	RectClass uv;
	uv.Set( FRAME_1_UV_UL, FRAME_1_UV_LR );
	RectClass draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + FRAME_1_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( FRAME_2_UV_UL, FRAME_2_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + FRAME_2_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( FRAME_3_UV_UL, FRAME_3_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + FRAME_3_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( FRAME_4_UV_UL, FRAME_4_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + FRAME_4_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( FRAME_5_UV_UL, FRAME_5_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + FRAME_5_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( FRAME_6_UV_UL, FRAME_6_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + FRAME_6_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( HEALTH_BACK_UV_UL, HEALTH_BACK_UV_LR );
	uv.Scale( INFO_UV_SCALE );
	draw.Set( HEALTH_BACK_UL, HEALTH_BACK_LR );
	draw += InfoBase;
	InfoRenderer->Add_Quad( draw, uv );

	// Draw Health and Shield
	Info_Update_Health_Shield();

	if ( COMBAT_STAR ) {

		if ( COMBAT_STAR->Has_Key( 1 ) ) {
			uv.Set( KEY_1_UV_UL, KEY_1_UV_LR );
			draw = uv;
			uv.Scale( INFO_UV_SCALE );
			draw += InfoBase + KEY_1_OFFSET - draw.Upper_Left();
			InfoRenderer->Add_Quad( draw, uv );
		}

		if ( COMBAT_STAR->Has_Key( 2 ) ) {
			uv.Set( KEY_2_UV_UL, KEY_2_UV_LR );
			draw = uv;
			uv.Scale( INFO_UV_SCALE );
			draw += InfoBase + KEY_2_OFFSET - draw.Upper_Left();
			InfoRenderer->Add_Quad( draw, uv );
		}

		if ( COMBAT_STAR->Has_Key( 3 ) ) {
			uv.Set( KEY_3_UV_UL, KEY_3_UV_LR );
			draw = uv;
			uv.Scale( INFO_UV_SCALE );
			draw += InfoBase + KEY_3_OFFSET - draw.Upper_Left();
			InfoRenderer->Add_Quad( draw, uv );
		}
	}

#if 0
return;

	uv.Set( TARGET_HEALTH_L_UV_UL, TARGET_HEALTH_L_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + TARGET_HEALTH_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( TARGET_HEALTH_R_UV_UL, TARGET_HEALTH_R_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + TARGET_HEALTH_OFFSET - draw.Upper_Left() + Vector2( 0, 20 );
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( TARGET_NAME_UV_UL, TARGET_NAME_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + TARGET_NAME_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );


	uv.Set( GRADIENT_BLACK_UV_UL, GRADIENT_BLACK_UV_LR );
	uv.Scale( INFO_UV_SCALE );
	draw.Set( TIME_BACK_UL, TIME_BACK_LR );
	draw += InfoBase;
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( TIME_CLOCK_UV_UL, TIME_CLOCK_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + TIME_CLOCK_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( WEAPON_BOX_UV_UL, WEAPON_BOX_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + WEAPON_BOX_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( POWER_OUT_ICON_UV_UL, POWER_OUT_ICON_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + POWER_OUT_ICON_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( REPAIR_ICON_UV_UL, REPAIR_ICON_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + REPAIR_ICON_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( GDI_ICON_UV_UL, GDI_ICON_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + GDI_ICON_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( NOD_ICON_UV_UL, NOD_ICON_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + NOD_ICON_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( NEUTRAL_ICON_UV_UL, NEUTRAL_ICON_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + NEUTRAL_ICON_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	// Radar
	uv.Set( RADAR_STAR_UV_UL, RADAR_STAR_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + RADAR_STAR_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( RADAR_STAR_UV_UL, RADAR_STAR_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + RADAR_STAR_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( RADAR_SQUARE_UV_UL, RADAR_SQUARE_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + RADAR_SQUARE_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( RADAR_TRIANGLE_UV_UL, RADAR_TRIANGLE_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + RADAR_TRIANGLE_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( RADAR_CIRCLE_UV_UL, RADAR_CIRCLE_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + RADAR_CIRCLE_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( RADAR_BRACKET_UV_UL, RADAR_BRACKET_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + RADAR_BRACKET_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );

	uv.Set( RADAR_SWEEP_UV_UL, RADAR_SWEEP_UV_LR );
	draw = uv;
	uv.Scale( INFO_UV_SCALE );
	draw += InfoBase + RADAR_SWEEP_OFFSET - draw.Upper_Left();
	InfoRenderer->Add_Quad( draw, uv );
#endif
}

static	void	Info_Render( void )
{
	InfoRenderer->Render();
	InfoHealthCountRenderer->Render();
	InfoShieldCountRenderer->Render();
}


/*
** OLD HUD STUFF
*/
static bool		_HUDEnabled = true;
static bool		_HUDInited = false;

#ifdef ATI_DEMO_HACK
enum {
	DEMO_HUD_IMAGE=0,
	DEMO_HUD_IMAGE2,
	NUM_RENDER_IMAGES,
};
#else
enum {
	RETICLE	= 0,
	RETICLE_HIT,
	ACTION_STATUSBAR_RENDERER,
	DEMO_HUD_IMAGE,
	DEMO_HUD_IMAGE2,
	NUM_RENDER_IMAGES,
};
#endif
Render2DClass		*	RenderImages[ NUM_RENDER_IMAGES ];

/*
**
*/
static bool	Is_HUD_Displayed( void );

/*
**
*/
void 	HUDClass::Init(bool render_available)
{
	for( int i = 0; i < NUM_RENDER_IMAGES; i++ ) {
		RenderImages[i] = NULL;
	}

	// Reticles
#ifndef ATI_DEMO_HACK
	RenderImages[RETICLE] = new Render2DClass();
	RenderImages[RETICLE]->Set_Texture( "HD_reticle.tga" );
	RenderImages[RETICLE]->Set_Hidden( true );
	RenderImages[RETICLE_HIT] = new Render2DClass();
	RenderImages[RETICLE_HIT]->Set_Texture( "HD_reticle_hit.tga" );
	RenderImages[RETICLE_HIT]->Set_Hidden( true );
	// Action Status Bar (arming/disarming explosives, etc)
	RenderImages[ACTION_STATUSBAR_RENDERER] = new Render2DClass();
	RenderImages[ACTION_STATUSBAR_RENDERER]->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
#else
	RectClass rect=Render2DClass::Get_Screen_Resolution();

	RenderImages[DEMO_HUD_IMAGE] = new Render2DClass();
	RenderImages[DEMO_HUD_IMAGE]->Reset();
	RenderImages[DEMO_HUD_IMAGE]->Set_Texture( "renegade_demo.tga" );
	RenderImages[DEMO_HUD_IMAGE]->Set_Hidden( false );
	RenderImages[DEMO_HUD_IMAGE]->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	RenderImages[DEMO_HUD_IMAGE]->Add_Quad( RectClass( float(8),float(-8),float(256+8),float(128-8)), RectClass(0.0f,0.0f,1.0f,0.5f),0xffffffff);
	RenderImages[DEMO_HUD_IMAGE2] = new Render2DClass();
	RenderImages[DEMO_HUD_IMAGE2]->Reset();
	RenderImages[DEMO_HUD_IMAGE2]->Set_Texture( "renegade_demo.tga" );
	RenderImages[DEMO_HUD_IMAGE2]->Set_Hidden( false );
	RenderImages[DEMO_HUD_IMAGE2]->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	RenderImages[DEMO_HUD_IMAGE2]->Add_Quad( RectClass( rect.Right+float(-256-8),rect.Bottom+float(-128+8),rect.Right+float(-8),rect.Bottom+float(8)), RectClass(0.0f,0.5f,1.0f,1.0f),0xffffffff);
#endif

	if (render_available) {
		_HUDEnabled = true;

		SniperHUDClass::Init();
		Powerup_Init();
		Weapon_Init();
		Weapon_Chart_Init();
		Info_Init();
		Damage_Init();
		Target_Init();
		//Score_Init();
		Objective_Init();

		HUD_Help_Text_Init();

		_HUDInited = true;
	}
}

/*
**
*/
void 	HUDClass::Shutdown()
{
	if (_HUDInited) {

		Objective_Shutdown();
		//Score_Shutdown();
		Target_Shutdown();
		Damage_Shutdown();
		Info_Shutdown();
		Weapon_Chart_Shutdown();
		Weapon_Shutdown();
		Powerup_Shutdown();
		HUD_Help_Text_Shutdown();
		SniperHUDClass::Shutdown();

		for( int i = 0; i < NUM_RENDER_IMAGES; i++ ) {
			if ( RenderImages[i] ) {
				delete RenderImages[i];
				RenderImages[i] = NULL;
			}
		}

		_HUDInited = false;
	}
}

void 	HUDClass::Reset( void )
{
	Powerup_Reset();
	Damage_Reset();
	Weapon_Reset();
}

void 	HUDClass::Render()
{
	WWPROFILE( "HUD Render" );

#ifdef ATI_DEMO_HACK
	RenderImages[DEMO_HUD_IMAGE]->Render();
	RenderImages[DEMO_HUD_IMAGE2]->Render();
#else
	if ( COMBAT_CAMERA && COMBAT_CAMERA->Draw_Sniper() ) {
		SniperHUDClass::Render();
	}

	// Only render if Combat is active, and Menu is not,  and we have a star who is not sniping
	if ( Is_HUD_Displayed() ) {

		Powerup_Render();
		Weapon_Render();
		Weapon_Chart_Render();
		Info_Render();
		Damage_Render();
		Target_Render();
		//Score_Render();
		HUD_Help_Text_Render();
		Objective_Render();
		RadarManager::Render();

		for( int i = 0; i < NUM_RENDER_IMAGES; i++ ) {
			if ( RenderImages[i] ) {
				RenderImages[i]->Render();
			}
		}
	}
#endif
}


static bool	Is_HUD_Displayed( void )
{
	return (	_HUDEnabled &&
				COMBAT_STAR &&
				!COMBAT_STAR->Is_Dead() &&
				!COMBAT_STAR->Is_Destroyed() );
}

/*
**	called each time through the main loop
*/
void 	HUDClass::Think()
{
	WWPROFILE( "HUD Think" );

#ifndef ATI_DEMO_HACK
	if ( COMBAT_CAMERA && COMBAT_CAMERA->Draw_Sniper() ) {
		SniperHUDClass::Update();
	}

	if ( !Is_HUD_Displayed() ) {
		return;
	}

	if ( HUDInfo::Display_Action_Status_Bar() ) {

		RenderImages[ACTION_STATUSBAR_RENDERER]->Set_Hidden( false );
		RenderImages[ACTION_STATUSBAR_RENDERER]->Reset();
		RenderImages[ACTION_STATUSBAR_RENDERER]->Enable_Texturing( false );

		//
		//	Plot the rectangle
		//
		const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution();
		RectClass status_bar_rect (0.4F, 0.95F, 0.6F, 0.98F);
		status_bar_rect.Left		= int(status_bar_rect.Left * screen_rect.Width());
		status_bar_rect.Right	= int(status_bar_rect.Right * screen_rect.Width());
		status_bar_rect.Top		= int(status_bar_rect.Top * screen_rect.Height());
		status_bar_rect.Bottom	= int(status_bar_rect.Bottom  * screen_rect.Height());
		RenderImages[ACTION_STATUSBAR_RENDERER]->Add_Line( status_bar_rect.Upper_Left (), status_bar_rect.Upper_Right (), 1, 0xFFFFFFFF );
		RenderImages[ACTION_STATUSBAR_RENDERER]->Add_Line( status_bar_rect.Upper_Right (), status_bar_rect.Lower_Right (), 1, 0xFFFFFFFF );
		RenderImages[ACTION_STATUSBAR_RENDERER]->Add_Line( status_bar_rect.Lower_Right (), status_bar_rect.Lower_Left (), 1, 0xFFFFFFFF );
		RenderImages[ACTION_STATUSBAR_RENDERER]->Add_Line( status_bar_rect.Lower_Left (), status_bar_rect.Upper_Left (), 1, 0xFFFFFFFF );

		//
		//	Deflate the rectangle
		//
		status_bar_rect.Right -= 1.0F;
		status_bar_rect.Bottom -= 1.0F;

		//
		//	Plot the status rectangle
		//
		float width					= status_bar_rect.Width() * HUDInfo::Get_Action_Status_Value();
		status_bar_rect.Right	= status_bar_rect.Left + int(width);
		DWORD color					= VRGB_TO_INT32( Vector3( 1.0F, 0.25F, 0 ) );
		RenderImages[ACTION_STATUSBAR_RENDERER]->Add_Quad( status_bar_rect, color );

	} else {
		RenderImages[ACTION_STATUSBAR_RENDERER]->Set_Hidden( true );
	}

	SmartGameObj * star = COMBAT_STAR;
	if ( COMBAT_STAR->Get_Vehicle() ) {
		star = COMBAT_STAR->Get_Vehicle();
	}

	Info_Update();
	Powerup_Update();
	Weapon_Update();
	Weapon_Chart_Update();
	Damage_Update();
	Target_Update();
	//Score_Update();
	Objective_Update();

	// Radar
	Matrix3D tm;
	star->Get_Transform().Get_Inverse( tm );
	if ( star->As_VehicleGameObj() && COMBAT_CAMERA && !COMBAT_CAMERA->Is_2D_Targeting() ) {
		// Make a tm facing target
		Matrix3D gun;
		gun.Obj_Look_At( star->Get_Transform().Get_Translation(), star->Get_Targeting_Pos(), 0 );
		gun.Get_Inverse( tm );
	}

	PhysicalGameObj * obj = NULL;
	if ( HUDInfo::Get_Info_Object() ) {
		obj = HUDInfo::Get_Info_Object()->As_PhysicalGameObj();
	}
	RadarManager::Set_Bracket_Object( obj );
	Vector2	radar_center = InfoBase + RADAR_CENTER_OFFSET;
	RadarManager::Update( tm, radar_center );

	unsigned long reticle_color = HUDGlobalSettingsDef::Get_Instance()->Get_No_Relation_Color().Convert_To_ARGB();

	if ( HUDInfo::Get_Weapon_Target_Object() != NULL ) {
		reticle_color = HUDGlobalSettingsDef::Get_Instance()->Get_Friendly_Color().Convert_To_ARGB();
		PhysicalGameObj * pgo = HUDInfo::Get_Weapon_Target_Object()->As_PhysicalGameObj();
		if ( pgo && pgo->Is_Enemy( star ) ) {
			reticle_color = HUDGlobalSettingsDef::Get_Instance()->Get_Enemy_Color().Convert_To_ARGB();
		}
	}

	WeaponClass * weapon = star->Get_Weapon();
	if ( weapon ) {
		if ( weapon->Is_Reloading() || weapon->Is_Switching() || !weapon->Is_Loaded() ) {
			reticle_color = 0xFFFFFF00; // yellow
		}
	}

	Vector2 reticle_offset = COMBAT_CAMERA->Get_Camera_Target_2D_Offset();
	RenderImages[RETICLE]->Reset();
	RenderImages[RETICLE]->Add_Quad( RectClass( reticle_offset.X - RETICLE_WIDTH/2, reticle_offset.Y - RETICLE_HEIGHT/2, reticle_offset.X + RETICLE_WIDTH/2, reticle_offset.Y + RETICLE_HEIGHT/2 ), reticle_color);

	//RenderImages[RETICLE]->Set_Hidden( false );
	//TSS092401
	if ( CombatManager::Is_Gameplay_Permitted() ) {
		RenderImages[RETICLE]->Set_Hidden( false );
	} else {
		RenderImages[RETICLE]->Set_Hidden( true );
	}

	//TSS092401 if ( weapon ) {
	if ( CombatManager::Is_Gameplay_Permitted() && (weapon != NULL) && CombatManager::Is_Hit_Reticle_Enabled() ) {
		WWPROFILE( "Reticle" );
		Vector3 pos3d = HUDInfo::Get_Weapon_Target_Position();
		Vector3 reticle_hit_offset;
		COMBAT_CAMERA->Project( reticle_hit_offset, pos3d );
//		weapon_hitting = HUDInfo::Get_Weapon_Target_Object() != NULL;
		RenderImages[RETICLE_HIT]->Reset();
		RenderImages[RETICLE_HIT]->Add_Quad( RectClass( reticle_hit_offset.X - RETICLE_WIDTH/2, reticle_hit_offset.Y - RETICLE_HEIGHT/2, reticle_hit_offset.X + RETICLE_WIDTH/2, reticle_hit_offset.Y + RETICLE_HEIGHT/2 ), reticle_color);
		RenderImages[RETICLE_HIT]->Set_Hidden( false );
	} else {
		RenderImages[RETICLE_HIT]->Set_Hidden( true );
	}


#if 0
	// Display points
	if ( _HUDTimer > 0 ) {
		WWPROFILE( "Points" );
		_HUDTimer -= TimeManager::Get_Frame_Seconds();

		StringClass	pointstring;
		int points = _HUDPoints;
		if ( points ) {
			Vector2 position = Render2DClass::Get_Screen_Resolution().Lower_Right();
			position /= 2;
			float scale = WWMath::Clamp( 2 * _HUDTimer / HUD_POINTS_TIME, 0, 1 );
			position.Y *= 0.3f + (scale/2);
			pointstring.Format( "%d", points );
			((Render2DTextClass *)RenderImages[TEXT_RENDERER])->Set_Location( position );

			Font3DInstanceClass * font = ((Render2DTextClass *)RenderImages[TEXT_RENDERER])->Peek_Font();
			if ( font ) {
				font->Set_Scale( 2 + (scale * 3) );
				position.X -= font->String_Width( pointstring ) / 2;
				position.Y -= font->Char_Height() / 2;
				((Render2DTextClass *)RenderImages[TEXT_RENDERER])->Set_Location( position );
				((Render2DTextClass *)RenderImages[TEXT_RENDERER])->Draw_Text( pointstring, COLOR( scale ) );
				font->Set_Scale( 1 );
			}
		}
	}
#endif
#endif // ATI_DEMO_HACK
}

void	HUDClass::Toggle_Hide_Points( void )
{
//	_HUDHidePoints = !_HUDHidePoints;
}

void	HUDClass::Display_Points( float points )
{
/*	if ( !_HUDHidePoints ) {
		_HUDPoints = points;
		_HUDTimer = HUD_POINTS_TIME;
	}*/
}

bool	HUDClass::Is_Enabled( void )
{
	return _HUDEnabled;
}

void	HUDClass::Enable( bool enable )
{
	_HUDEnabled = enable;
}

#define	SNIPER_AMMO_UV		3, 139,79,191
#define	SNIPER_AMMO_OFFSET	-10,-4
#define	NUKE_UV				193,187,244,246
#define	NUKE_OFFSET			10,-4
#define	ION_UV				101,196,164,246
#define	ION_OFFSET			10,-4

void	HUDClass::Add_Powerup_Weapon( int id, int rounds )
{
	const WeaponDefinitionClass	* def = WeaponManager::Find_Weapon_Definition( id );
	if ( def ) {
		if ( !def->IconTextureName.Is_Empty() ) {
			Powerup_Add( TranslateDBClass::Get_String( def->IconNameID ), rounds,
				def->IconTextureName, def->IconTextureUV, def->IconOffset );
		}
	}
}

void	HUDClass::Add_Powerup_Ammo( int id, int rounds )
{
	Add_Powerup_Weapon( id, rounds );
#if 0
	const WeaponDefinitionClass	* wdef = WeaponManager::Find_Weapon_Definition( id );
	if ( wdef ) {
		const AmmoDefinitionClass	* def = WeaponManager::Find_Ammo_Definition( wdef->PrimaryAmmoDefID );
		if ( def ) {
			if ( !def->IconTextureName.Is_Empty() ) {
				Powerup_Add( TranslateDBClass::Get_String( def->IconNameID ), rounds,
						def->IconTextureName, def->IconTextureUV, def->IconOffset );
			}
		}
	}
#endif
}

void	HUDClass::Add_Shield_Grant( float strength )
{
	const char * texture_name = "hud_armor3.tga";
	if ( strength > 75 ) {
		texture_name = "hud_armor1.tga";
	} else if ( strength > 30 ) {
		texture_name = "hud_armor2.tga";
	}
	Powerup_Add( TranslateDBClass::Get_String(IDS_Power_up_Armor_00),  (int)strength, texture_name, RectClass( 0,0,64,64 ),	Vector2( 10,40 ) );
}

void	HUDClass::Add_Health_Grant( float amount )
{
	const char * texture_name = "hud_health1.tga";
	if ( amount > 75 ) {
		texture_name = "hud_health3.tga";
	} else if ( amount > 30 ) {
		texture_name = "hud_health2.tga";
	}
	Powerup_Add( TranslateDBClass::Get_String(IDS_Power_up_Health_00),  (int)amount, texture_name, RectClass( 0,0,64,64 ),	Vector2( 10,40 ) );
}

void	HUDClass::Add_Shield_Upgrade_Grant( float strength )
{
	Powerup_Add( TranslateDBClass::Get_String(IDS_Power_up_Armor_Upgrade),  (int)strength, "hud_armedal.tga", RectClass( 0,0,64,64 ),	Vector2( 10,40 ), false );
}

void	HUDClass::Add_Health_Upgrade_Grant( float amount )
{
	Powerup_Add( TranslateDBClass::Get_String(IDS_Power_up_Health_Upgrade),  (int)amount, "hud_hemedal.tga", RectClass( 0,0,64,64 ),	Vector2( 10,40 ), false );
}

void	HUDClass::Add_Key_Grant( int key )
{
	const char * texture_name = "hud_keycard_green.tga";
	if ( key == 3 ) {
		texture_name = "hud_keycard_red.tga";
	} else if ( key == 2 ) {
		texture_name = "hud_keycard_yellow.tga";
	}
	Powerup_Add( TranslateDBClass::Get_String(IDS_Power_up_SecurityCard),  0, texture_name, RectClass( 0,0,64,64 ),	Vector2( 10,40 ), false );
}

void	HUDClass::Add_Objective( int type )
{
	if ( type == ObjectiveManager::TYPE_PRIMARY ) {
		Powerup_Add( TranslateDBClass::Get_String(IDS_Enc_Obj_Priority_0_Primary),  0, "p_eva1.tga", RectClass( 0,0,64,64 ),	Vector2( 10,40 ), false );
	} else if ( type == ObjectiveManager::TYPE_SECONDARY ) {
		Powerup_Add( TranslateDBClass::Get_String(IDS_Enc_Obj_Priority_0_Secondary),  0, "p_eva2.tga", RectClass( 0,0,64,64 ),	Vector2( 10,40 ), false );
	}
}

void	HUDClass::Add_Data_Link( void )
{
	int cur = TimeManager::Get_Total_Seconds() * 2.0f;
	static int last = 0;
	// Don't accept too fast;
	if ( cur == last ) {
		return;
	}
	last = cur;
	Powerup_Add( TranslateDBClass::Get_String(IDS_Power_up_DataDisc_01),  0, "hud_cd_rom.tga", RectClass( 0,0,64,64 ),	Vector2( 10,40 ), false );
}

void	HUDClass::Add_Map_Reveal( void )
{
	Add_Data_Link();
//	Powerup_Add( TranslateDBClass::Get_String(IDS_Power_up_DataDisc_01),  0, "hud_cd_rom.tga", RectClass( 0,0,64,64 ),	Vector2( 10,40 ), false );
//	Powerup_Add( TranslateDBClass::Get_String(IDS_Enc_Pow_Bonus_Map_Name),  0, "hud_cd_rom.tga", RectClass( 0,0,64,64 ),	Vector2( 10,40 ), false );
}

/*
**
*/
enum	{
	CHUNKID_VARIABLES			=	1117011622,
	CHUNKID_MARKER_ENTRY,

	MICROCHUNKID_ENABLED		=	1,
};

/*
**
*/
bool	HUDClass::Save( ChunkSaveClass &csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_ENABLED,	_HUDEnabled );
	csave.End_Chunk();

	return true;
}

bool	HUDClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_ENABLED,	_HUDEnabled );
						default:
							Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}
	return true;
}
