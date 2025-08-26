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
 *                     $Archive:: /Commando/Code/Combat/radar.cpp                             $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/23/02 5:48p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 68                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "radar.h"
#include "assets.h"
#include "soldier.h"
#include "gameobjmanager.h"
#include "debug.h"
#include "combat.h"
#include "render2d.h"
#include "texture.h"
#include "playertype.h"
#include "globalsettings.h"
#include "objectives.h"
#include "globalsettings.h"
#include "wwprofile.h"
#include "WWAudio.h"
#include "render2dsentence.h"
#include "stylemgr.h"
#include "translatedb.h"
#include "string_ids.h"


DynamicVectorClass<RadarMarkerClass>	RadarManager::Markers;
Render2DClass						*	RadarManager::Renderer;
unsigned long							RadarManager::BlipColors[ NUM_BLIP_COLOR_TYPES ];
const PhysicalGameObj				*	RadarManager::BracketObj = NULL;
RectClass								RadarManager::BlipUV[ NUM_BLIP_TYPES ];
bool										RadarManager::IsHidden = false;
float										RadarManager::HiddenTimer = 0.0f;
RadarModeEnum							RadarManager::RadarMode = RADAR_ALL;
Render2DSentenceClass *				RadarManager::CompassRenderers[8];
int										RadarManager::CurrentCompassRendererIndex;

static		Matrix3D					RadarTM;
#if 0
static		float						RadarSweep					= 0;
static		float						RadarSweepMove;
#endif

DynamicVectorClass<Render2DClass *>		Blips;

const char * RadarManager::Get_Blip_Shape_Type_Name( int index )
{
	static const char * names[ NUM_BLIP_SHAPE_TYPES ] = {
		"None",
		"Human",
		"Vehicle",
		"Stationary",
		"Objective",
	};

	return names[index];
}

/*
**
*/
static	const RectClass &	Scale_UV( const RectClass & uv, float texture_size )
{
	static RectClass new_uv;
	new_uv = uv;
	new_uv.Scale( 1/texture_size );
	return new_uv;
}


void	RadarManager::Set_Hidden( bool onoff )	
{ 
	if ( IsHidden != onoff ) {

		int sound_id = 0;
		if ( onoff ) {
			sound_id = HUDGlobalSettingsDef::Get_Instance()->RadarOnSoundID;
		} else { 
			sound_id = HUDGlobalSettingsDef::Get_Instance()->RadarOffSoundID;
		}

		if (sound_id != 0) {
			WWAudioClass::Get_Instance ()->Create_Instant_Sound (sound_id, Matrix3D (1));
		}

		IsHidden = onoff; 
	}
}

/*
**
*/

#define		RADAR_TEXTURE		"HUD_MAIN.TGA"

extern Vector2 INFO_UV_SCALE;
extern Vector2 RADAR_RINGS_UV_UL;
extern Vector2 RADAR_RINGS_UV_LR;
extern Vector2 RADAR_RINGS_L_OFFSET;
extern Vector2 RADAR_RINGS_R_OFFSET;
extern Vector2 RADAR_STAR_UV_UL;
extern Vector2 RADAR_STAR_UV_LR;
extern Vector2 RADAR_SQUARE_UV_UL;
extern Vector2 RADAR_SQUARE_UV_LR;
extern Vector2 RADAR_TRIANGLE_UV_UL;
extern Vector2 RADAR_TRIANGLE_UV_LR;
extern Vector2 RADAR_CIRCLE_UV_UL;
extern Vector2 RADAR_CIRCLE_UV_LR;
extern Vector2 RADAR_BRACKET_UV_UL;
extern Vector2 RADAR_BRACKET_UV_LR;
extern Vector2 RADAR_SWEEP_UV_UL;
extern Vector2 RADAR_SWEEP_UV_LR;


void 	RadarManager::Init()
{
	int i;

	BracketObj = NULL;

	Renderer = new Render2DClass();
	Renderer->Set_Texture( RADAR_TEXTURE );
	Renderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() ); 

	// Init Colors
	for ( i = 0; i < NUM_BLIP_COLOR_TYPES; i++ ) {
		BlipColors[ i ] = 0xFFFFFFFF;
	}
	if ( HUDGlobalSettingsDef::Get_Instance() != NULL ) {
		BlipColors[ BLIP_COLOR_TYPE_NOD ] =						HUDGlobalSettingsDef::Get_Instance()->Get_Nod_Color().Convert_To_ARGB(); 
		BlipColors[ BLIP_COLOR_TYPE_GDI ] =						HUDGlobalSettingsDef::Get_Instance()->Get_GDI_Color().Convert_To_ARGB(); 
		BlipColors[ BLIP_COLOR_TYPE_NEUTRAL ] =					HUDGlobalSettingsDef::Get_Instance()->Get_Neutral_Color().Convert_To_ARGB(); 
		BlipColors[ BLIP_COLOR_TYPE_MUTANT ] =					HUDGlobalSettingsDef::Get_Instance()->Get_Mutant_Color().Convert_To_ARGB(); 
		BlipColors[ BLIP_COLOR_TYPE_RENEGADE ] =				HUDGlobalSettingsDef::Get_Instance()->Get_Renegade_Color().Convert_To_ARGB(); 
		BlipColors[ BLIP_COLOR_TYPE_PRIMARY_OBJECTIVE ] =		HUDGlobalSettingsDef::Get_Instance()->Get_Primary_Objective_Color().Convert_To_ARGB(); 
		BlipColors[ BLIP_COLOR_TYPE_SECONDARY_OBJECTIVE ] =		HUDGlobalSettingsDef::Get_Instance()->Get_Secondary_Objective_Color().Convert_To_ARGB(); 
		BlipColors[ BLIP_COLOR_TYPE_TERTIARY_OBJECTIVE ] =		HUDGlobalSettingsDef::Get_Instance()->Get_Tertiary_Objective_Color().Convert_To_ARGB(); 
	}

#if 0
	const HUDGlobalSettingsDef * settings = HUDGlobalSettingsDef::Get_Instance();
	float radar_texture_size = settings->RadarTextureSize;

	BlipUV[ BLIP_SHAPE_TYPE_NONE ] =			RectClass( 0,0,0,0 );
	BlipUV[ BLIP_SHAPE_TYPE_HUMAN ] =		Scale_UV( settings->RadarHumanBlipUV, radar_texture_size );
	BlipUV[ BLIP_SHAPE_TYPE_VEHICLE ] =		Scale_UV( settings->RadarVehicleBlipUV, radar_texture_size );
	BlipUV[ BLIP_SHAPE_TYPE_STATIONARY ] =	Scale_UV( settings->RadarStationaryBlipUV, radar_texture_size );
	BlipUV[ BLIP_SHAPE_TYPE_OBJECTIVE ] =	Scale_UV( settings->RadarObjectiveBlipUV, radar_texture_size );
	BlipUV[ BLIP_BRACKET ] =					Scale_UV( settings->RadarBlipBracketUV, radar_texture_size );
	BlipUV[ BLIP_SWEEP ] =						Scale_UV( settings->RadarSweepUV, radar_texture_size );
#else
	BlipUV[ BLIP_SHAPE_TYPE_NONE ] =		RectClass( 0,0,0,0 );
	BlipUV[ BLIP_SHAPE_TYPE_HUMAN ] =		RectClass( RADAR_CIRCLE_UV_UL, RADAR_CIRCLE_UV_LR );
	BlipUV[ BLIP_SHAPE_TYPE_VEHICLE ] =		RectClass( RADAR_TRIANGLE_UV_UL, RADAR_TRIANGLE_UV_LR );
	BlipUV[ BLIP_SHAPE_TYPE_STATIONARY ] =	RectClass( RADAR_SQUARE_UV_UL, RADAR_SQUARE_UV_LR );
	BlipUV[ BLIP_SHAPE_TYPE_OBJECTIVE ] =	RectClass( RADAR_STAR_UV_UL, RADAR_STAR_UV_LR );
	BlipUV[ BLIP_BRACKET ] =				RectClass( RADAR_BRACKET_UV_UL, RADAR_BRACKET_UV_LR );
	BlipUV[ BLIP_SWEEP ] =					RectClass( RADAR_SWEEP_UV_UL, RADAR_SWEEP_UV_LR );
	BlipUV[ BLIP_SHAPE_TYPE_HUMAN ].Scale( INFO_UV_SCALE );
	BlipUV[ BLIP_SHAPE_TYPE_VEHICLE ].Scale( INFO_UV_SCALE );
	BlipUV[ BLIP_SHAPE_TYPE_STATIONARY ].Scale( INFO_UV_SCALE );
	BlipUV[ BLIP_SHAPE_TYPE_OBJECTIVE ].Scale( INFO_UV_SCALE );
	BlipUV[ BLIP_BRACKET ].Scale( INFO_UV_SCALE );
	BlipUV[ BLIP_SWEEP ].Scale( INFO_UV_SCALE );
#endif

	// Clear radar renderer pointers, these are initialized on demand
	for (i=0;i<8;++i) {
		CompassRenderers[i] = NULL;
	}

	// Don't reset the markers here, Some may have already been added when loading the level (Created)

	HiddenTimer = 0;
}

/*
**
*/
void 	RadarManager::Shutdown()
{
	if ( Renderer ) {
		delete Renderer;
		Renderer = NULL;
	}

	while ( Blips.Count() > 0 ) {
		int index = Blips.Count() - 1;
		delete Blips[ index ];
		Blips.Delete( index );
	}

	for (int i=0;i<8;++i) {
		delete CompassRenderers[i];
		CompassRenderers[i] = NULL;
	}


	Markers.Delete_All();

	IsHidden = false;		// Do this here rather than init, because init is called after load
}

/*
**
*/
enum	{
	CHUNKID_MANAGER_VARIABLES			=	630001357,
	CHUNKID_MARKER_ENTRY,

	MICROCHUNKID_IS_HIDDEN			=	1,
	MICROCHUNKID_HIDDEN_TIMER,
};

/*
**
*/
bool	RadarManager::Save( ChunkSaveClass &csave )
{
	csave.Begin_Chunk( CHUNKID_MANAGER_VARIABLES );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_IS_HIDDEN,	IsHidden );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HIDDEN_TIMER,	HiddenTimer );
	csave.End_Chunk();

	for ( int i = 0; i < Markers.Count(); i++ ) {
		csave.Begin_Chunk( CHUNKID_MARKER_ENTRY );
			Markers[i].Save( csave );
		csave.End_Chunk();
	}

	return true;
}

bool	RadarManager::Load( ChunkLoadClass &cload )
{
	WWASSERT( Markers.Count() == 0 );

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_MANAGER_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_IS_HIDDEN,	IsHidden );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HIDDEN_TIMER,	HiddenTimer );
						default:
							Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_MARKER_ENTRY:
			{
				RadarMarkerClass marker;
				marker.Load( cload );
				Add_Marker( marker );
				break;
			}

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}
	return true;
}

/*
**
*/
#define	RADAR_RADIUS			84
#define	BLIP_SIZE				4
#define	RADAR_FADE_STOP			(RADAR_RADIUS * 0.60f)
#define	RADAR_FADE_START		(RADAR_RADIUS * 0.50f)
#define	RADAR_CENTER_TWEAK	Vector2( 1, 2 )

Vector2	OldRadarCenter(0.0f,0.0f);
Vector2	RadarCenter(0.0f,0.0f);
float	RadarIntensity;
long	RadarColor;

float	RadarManager::Add_Blip( const Vector3 & pos, int shape_type, int color_type, float intensity, bool bracket, bool altitude_fade ) 
{
	if ( shape_type != BLIP_SHAPE_TYPE_NONE ) {

		Vector3	screen = RadarTM * pos;
		screen *= RADAR_RADIUS;
		screen *= 0.01f;	//			screen *= ZoomFactor;
		screen.Z = 0;

		float dist = screen.Length();

		if ( shape_type == BLIP_SHAPE_TYPE_OBJECTIVE ) {
			if ( dist >= RADAR_FADE_STOP ) {
				screen.Normalize();
				screen *= RADAR_FADE_STOP;
				dist = RADAR_FADE_STOP;
			}
		}

		if ( dist <= RADAR_FADE_STOP ) 
		{
#if 0
			// Find blip bearing
			float bearing = WWMath::Atan2( screen.Y, screen.X );
			// find the bering relative to sweep
			bearing = WWMath::Wrap( bearing - RadarSweep, -DEG_TO_RADF( 360 ), 0 );
			if ( bearing > -RadarSweepMove*2 ) {
				intensity = 1.0;		// Ping
			}
#else
			intensity = 1.0;		// Always Ping
#endif

			float alpha = 1.0f - ((dist - RADAR_FADE_START) / (RADAR_FADE_STOP - RADAR_FADE_START) );
			alpha = WWMath::Clamp( alpha, 0, 1 );
			if ( shape_type == BLIP_SHAPE_TYPE_OBJECTIVE ) {
				alpha = 1;
			}
			float color_alpha = alpha * intensity;
			if ( altitude_fade ) {
				color_alpha *= 0.66f;
			}


			unsigned int color = BlipColors[ color_type ];
			color &= 0x00FFFFFF;
			color |= (unsigned int)(RadarIntensity * color_alpha * 255) << 24;
			if ( Renderer ) {
				RectClass	blip( -BLIP_SIZE, -BLIP_SIZE, BLIP_SIZE, BLIP_SIZE );
				if ( altitude_fade ) {
					blip.Scale_Relative_Center( 0.66f );
				}
   				blip += Vector2( -screen.X, screen.Y );
   				blip += RadarCenter + RADAR_CENTER_TWEAK;
				RectClass uv = BlipUV[ shape_type ];
				Renderer->Add_Quad( blip, uv, color );

				if ( bracket ) {
					color = 0x0000FF00;	// Make Green
					color |= (unsigned int)(RadarIntensity * alpha * 255) << 24;
					RectClass uv = BlipUV[ BLIP_BRACKET ];
					Renderer->Add_Quad( blip, uv, color );
				}
			}
		}
	}

	return intensity;
}

#define	COMPASS_OFFSET			Vector2( 2, -73 )
#define	RADAR_Z_RANGE			3

void	RadarManager::Update( const Matrix3D & player_tm, const Vector2 & center )
{
	WWPROFILE( "Radar Update" );

	OldRadarCenter=RadarCenter;
	RadarCenter = center;

	RadarTM = player_tm;
	RadarTM.Pre_Rotate_Z( DEG_TO_RAD( -90 ) );

#if 0
	RadarSweepMove = TimeManager::Get_Frame_Seconds() * 2;
	RadarSweep -= RadarSweepMove;
	if ( RadarSweep < 0 ) {
		RadarSweep += DEG_TO_RAD( 360 );
	}
#endif

	HiddenTimer += TimeManager::Get_Frame_Seconds() * (IsHidden ? 1 : -1);
	HiddenTimer = WWMath::Clamp( HiddenTimer, 0, 1 );

	RadarIntensity = 1-HiddenTimer;
	RadarColor = ((int)((1-HiddenTimer) * 255) << 24) | 0x00FFFFFF;

	Renderer->Reset();

	if ( RadarIntensity == 0 ) {
		return;
	}

	// Radar Rings
	RectClass uv;
	RectClass draw;
	uv.Set( RADAR_RINGS_UV_UL, RADAR_RINGS_UV_LR );
	draw = uv;
	draw.Right = draw.Left + uv.Height();
	draw.Bottom = draw.Top + uv.Width();
	uv.Scale( INFO_UV_SCALE );
	draw += center + RADAR_RINGS_L_OFFSET - draw.Upper_Left();
	Renderer->Add_Quad( draw.Lower_Left(), draw.Lower_Right(), draw.Upper_Left(), draw.Upper_Right(), uv, RadarColor );
	draw += center + RADAR_RINGS_R_OFFSET - draw.Upper_Left();
	Renderer->Add_Quad_Backfaced( draw.Lower_Right(), draw.Lower_Left(), draw.Upper_Right(), draw.Upper_Left(), uv, RadarColor );

#if 0
{WWPROFILE( "Sweep" );
	// Draw sweep Line
	Vector2 edge = Vector2( -WWMath::Cos( RadarSweep ), WWMath::Sin( RadarSweep ) ) * RADAR_FADE_STOP + center;
	float width = 20;
	Vector2	corner_offset = center - edge;  	// get line relative to edge
	float temp = corner_offset.X;					// Rotate 90
	corner_offset.X = corner_offset.Y;
	corner_offset.Y = -temp;
	corner_offset.Normalize();						// scale to length width/2
	corner_offset *= width;
	RectClass uv = BlipUV[ BLIP_SWEEP ];
	Renderer->Add_Tri( edge - corner_offset, center, edge,  
		uv.Lower_Left(),	uv.Upper_Right(), uv.Upper_Left(),	0xFF00FF00 & RadarColor );
}
#endif

#if 0
{WWPROFILE( "Compass" );
	// Draw the compass
	const HUDGlobalSettingsDef * settings = HUDGlobalSettingsDef::Get_Instance();
	float radar_texture_size = settings->RadarTextureSize;
	float	bering = WWMath::Wrap( (player_tm.Get_Z_Rotation() / DEG_TO_RAD( 360.0f )) + 0.25f, 0, 1 );
	int frame = (int)((bering * 8.0f) + 0.5f);
	RectClass compass( Vector2( 0, 0 ), settings->RadarCompassSize );
//	compass += Render2DClass::Get_Screen_Resolution().Lower_Left() + settings->RadarCompassOffset;
	compass += center + COMPASS_OFFSET;
	RectClass compass_uv = Scale_UV( settings->RadarCompassBaseUV, radar_texture_size );
	compass_uv += (settings->RadarCompassUVOffset/radar_texture_size) * (frame & 7);
	Renderer->Add_Quad( compass, compass_uv, RadarColor );
}

#else
	float	bering = WWMath::Wrap( (player_tm.Get_Z_Rotation() / DEG_TO_RAD( 360.0f )) + 0.25f, 0, 1 );
	CurrentCompassRendererIndex = (int)((bering * 8.0f) + 0.5f);
	CurrentCompassRendererIndex&=7;

	int dir[8] = { IDS_HUD_COMPASS_N, IDS_HUD_COMPASS_NE, IDS_HUD_COMPASS_E, IDS_HUD_COMPASS_SE, 
						IDS_HUD_COMPASS_S, IDS_HUD_COMPASS_SW, IDS_HUD_COMPASS_W, IDS_HUD_COMPASS_NW };

	// If the renderer object for this particular radar direction hasn't been created, create it now...
	if (!CompassRenderers[CurrentCompassRendererIndex]) {
		CompassRenderers[CurrentCompassRendererIndex]=new Render2DSentenceClass();
		FontCharsClass *font = StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_TXT );
		CompassRenderers[CurrentCompassRendererIndex]->Set_Font( font );
		CompassRenderers[CurrentCompassRendererIndex]->Reset();
		CompassRenderers[CurrentCompassRendererIndex]->Build_Sentence(TRANSLATE(dir[CurrentCompassRendererIndex]));
		Vector2 text_size=CompassRenderers[CurrentCompassRendererIndex]->Get_Text_Extents(TRANSLATE(dir[CurrentCompassRendererIndex]));

		Vector2 pos = center + COMPASS_OFFSET - (text_size * 0.5f);
		pos.X = (int)pos.X;
		pos.Y = (int)pos.Y;
		CompassRenderers[CurrentCompassRendererIndex]->Set_Location( pos );

		CompassRenderers[CurrentCompassRendererIndex]->Draw_Sentence();
	}
	else {
		// If the radar center has moved (which should never happen unless the screen size changes)
		if (RadarCenter!=OldRadarCenter) {	
			CompassRenderers[CurrentCompassRendererIndex]->Reset();
			CompassRenderers[CurrentCompassRendererIndex]->Build_Sentence( TRANSLATE(dir[CurrentCompassRendererIndex]));
			Vector2 text_size=CompassRenderers[CurrentCompassRendererIndex]->Get_Text_Extents(TRANSLATE(dir[CurrentCompassRendererIndex]));
			Vector2 pos = center + COMPASS_OFFSET - (text_size * 0.5f);
			pos.X = (int)pos.X;
			pos.Y = (int)pos.Y;
			CompassRenderers[CurrentCompassRendererIndex]->Set_Location( pos );
			CompassRenderers[CurrentCompassRendererIndex]->Draw_Sentence();
		}
	}

#endif


	// Now build the blips
	float star_z = 0;
	if ( COMBAT_STAR ) {
		Vector3 p;
		COMBAT_STAR->Get_Position( &p );
		star_z = p.Z;
	}

{WWPROFILE( "Blips" );
	// for all physicalgameobjs
	SLNode<BaseGameObj> *objnode;
	for (	objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
		PhysicalGameObj *obj = objnode->Data()->As_PhysicalGameObj();
		if ( obj ) {
			if ( obj == COMBAT_STAR ) continue;	// Don't draw the star
			if ( obj->Peek_Physical_Object() == NULL ) continue;

			// Don't draw dead soldiers
			if (	obj->As_SoldierGameObj() && 
					obj->Get_Defense_Object() && 
					obj->Get_Defense_Object()->Get_Health() <= 0.0f ) {
				continue;
			}

			//
			// TSS: Filter blips based on RadarMode
			//
			if (COMBAT_STAR != NULL && RadarMode != RADAR_ALL) {
				if (RadarMode == RADAR_NOBODY) {
					continue;
				} else {
					WWASSERT(RadarMode == RADAR_TEAMMATES);
					int other_pt = obj->Get_Player_Type();
					int my_pt = COMBAT_STAR->Get_Player_Type();
					if (my_pt != other_pt || 
					   (my_pt != PLAYERTYPE_NOD && my_pt != PLAYERTYPE_GDI)) {
						continue;
					}
				}
			}

			//
			// gth: don't show radar blips for enemy stealthed units
			//
			SmartGameObj * smart_obj = obj->As_SmartGameObj();
			if ((smart_obj != NULL) && (smart_obj->Is_Stealthed())) {

				int obj_pt = smart_obj->Get_Player_Type();
				int my_pt = COMBAT_STAR->Get_Player_Type();
				if (my_pt != obj_pt) {
					continue;
				}
			}


			Vector3 objpos;
			obj->Get_Position( &objpos );

			bool altitude_fade = false;
			float z_diff = star_z - objpos.Z;
			if ( WWMath::Fabs( z_diff ) > RADAR_Z_RANGE) {
				altitude_fade = true;
			}

//			if ( (WWMath::Fabs( z_diff ) < RADAR_Z_RANGE) || (obj->Get_Radar_Blip_Shape_Type() == BLIP_SHAPE_TYPE_OBJECTIVE) ) {
				float intensity = obj->Get_Radar_Blip_Intensity();
				bool bracket = obj == BracketObj;
				intensity = Add_Blip( objpos, obj->Get_Radar_Blip_Shape_Type(), obj->Get_Radar_Blip_Color_Type(), intensity, bracket, altitude_fade );
	#if 0
				intensity -= RadarSweepMove / DEG_TO_RAD( 360 ) * 0.666f;				// Drop Blip intensity
	#endif
				obj->Set_Radar_Blip_Intensity( WWMath::Clamp( intensity, 0, 1 ) );
//			}
		}
	}
}

	int i;

{WWPROFILE( "Objectives" );
	// for all objectives with a position
	int count = ObjectiveManager::Get_Objective_Count();
	for ( int i = 0; i < count; i++ ) {
		Objective * objective = ObjectiveManager::Get_Objective( i );
		if ( objective->DrawBlip && objective->Status == ObjectiveManager::STATUS_IS_PENDING ) {
			float intensity = objective->BlipIntensity;
			intensity = Add_Blip( objective->Position, BLIP_SHAPE_TYPE_OBJECTIVE, objective->Radar_Blip_Color_Type(), intensity, false );
#if 0
			intensity -= RadarSweepMove / DEG_TO_RAD( 360 ) * 0.666f;				// Drop Blip intensity
#endif
			objective->BlipIntensity = WWMath::Clamp( intensity, 0, 1 );
		}
	}
}

{WWPROFILE( "Markers" );
	// for all markers
	for ( i = 0; i < Markers.Count(); i++ ) {
		float intensity = Markers[i].Intensity;
		intensity = Add_Blip( Markers[i].Position, Markers[i].Type, Markers[i].Color, intensity, false );
#if 0
		intensity -= RadarSweepMove / DEG_TO_RAD( 360 ) * 0.666f;				// Drop Blip intensity
#endif
		Markers[i].Intensity = WWMath::Clamp( intensity, 0, 1 );
	}
}

}

/*
**
*/
void	RadarManager::Render( void )
{
	if ( (HiddenTimer < 1) && Renderer ) {
		Renderer->Render();

		if ( CompassRenderers[CurrentCompassRendererIndex] != NULL ) {
			CompassRenderers[CurrentCompassRendererIndex]->Render();
		}
	}
}

/*
**
*/
void	RadarManager::Clear_Marker( int id )
{
	for ( int i = 0; i < Markers.Count(); i++ ) {
		if ( Markers[i].ID == id ) {
			Markers.Delete( i );
			i--;
		}
	}
}

void	RadarManager::Change_Marker_Color( int id, int color )
{
	for ( int i = 0; i < Markers.Count(); i++ ) {
		if ( Markers[i].ID == id ) {
			Markers[i].Color = color;
		}
	}
}

/*
**
*/
RadarMarkerClass::RadarMarkerClass( void ) :
	ID( 0 ),
	Type( 0 ),
	Color( 0 ),
	Position( 0,0,0 ),
	Intensity( 0 )
{
}


enum	{
	CHUNKID_VARIABLES						=	630001403,
	XXXCHUNKID_GAME_OBJ_REF,

	MICROCHUNKID_ID						=	1,
	MICROCHUNKID_POSITION,
	XXXMICROCHUNKID_COLOR,
	XXXMICROCHUNKID_FLASH,
	XXXMICROCHUNKID_OBJECT_BASED,
	MICROCHUNKID_TYPE,
	MICROCHUNKID_COLOR,
};

bool	RadarMarkerClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_ID, ID );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_POSITION, Position );                    
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_TYPE, Type );                  
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_COLOR, Color );                  
	csave.End_Chunk();

	return true;
}

bool	RadarMarkerClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_ID, ID );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_POSITION, Position );                    
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_TYPE, Type );                  
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_COLOR, Color );                  

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

