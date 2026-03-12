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
 *                 Project Name : Combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/screenfademanager.cpp                 $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/17/02 12:09p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "screenfademanager.h"
#include "render2d.h"
#include "vector3.h"
#include "timemgr.h"
#include "chunkio.h"
#include "debug.h"

/**
** FloatInterpolatorClass
** This class can be used to manage a single floating point number which linearly
** interpolates towards target values that you give it.
*/
class FloatInterpolatorClass
{
public:
	FloatInterpolatorClass(float init) { Value = init; TargetValue = init; Rate = 0.0f; }

	bool			Save( ChunkSaveClass &csave );
	bool			Load( ChunkLoadClass &cload );

	float			Get_Value(void) const { return Value; }
	void			Set_Target_Value(float new_val,float time);
	void			Update(float dt);

protected:
	float			Value;
	float			TargetValue;
	float			Rate;
};

/*
**
*/
enum	{
	CHUNKID_VARIABLES			=	117021207,

	MICROCHUNKID_VALUE		=	1,
	MICROCHUNKID_TARGET_VALUE,
	MICROCHUNKID_RATE,
};

/*
**
*/
bool	FloatInterpolatorClass::Save( ChunkSaveClass &csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_VALUE,			Value );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_TARGET_VALUE,	TargetValue );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_RATE,			Rate );
	csave.End_Chunk();

	return true;
}

bool	FloatInterpolatorClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_VALUE,			Value );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_TARGET_VALUE,	TargetValue );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_RATE,			Rate );
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

void FloatInterpolatorClass::Set_Target_Value(float new_val,float time)
{
	TargetValue = new_val;
	if (time < WWMATH_EPSILON) {
		Value = new_val;
		Rate = 0.0f;
	} else {
		Rate = (new_val - Value)/time;
	}
}

void FloatInterpolatorClass::Update(float dt)
{
	float step = Rate * dt;
	if (WWMath::Fabs(step) > WWMath::Fabs(TargetValue - Value)) {
		Value = TargetValue;
	} else {
		Value += step;
	}
}


/*
** Static variables
*/
static FloatInterpolatorClass		_LetterboxFraction(0.0f);
static FloatInterpolatorClass		_OverlayOpacity(0.0f);
static FloatInterpolatorClass		_OverlayRed(0.0f);
static FloatInterpolatorClass		_OverlayGreen(0.0f);
static FloatInterpolatorClass		_OverlayBlue(0.0f);

static Render2DClass *				_Renderer = NULL;


/*
** Constants
*/
const float								LETTERBOX_SIZE = 0.125f;
const Vector3							BLACK(0,0,0);
const Vector3							WHITE(0,0,0);


/*
** ScreenFadeManager implementation
*/
void	ScreenFadeManager::Init()
{
	_Renderer = new Render2DClass();
	_Renderer->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );
	_Renderer->Enable_Alpha( true );
	_Renderer->Set_Coordinate_Range(RectClass(0,0,1,1));
}

void ScreenFadeManager::Shutdown()
{
	delete _Renderer;
	_Renderer = NULL;
}

void ScreenFadeManager::Think()
{
	/*
	** Update the parameters
	*/
	float seconds = TimeManager::Get_Frame_Seconds();

	_LetterboxFraction.Update(seconds);
	_OverlayOpacity.Update(seconds);
	_OverlayRed.Update(seconds);
	_OverlayGreen.Update(seconds);
	_OverlayBlue.Update(seconds);

	/*
	** Build the letterbox polys
	*/
	_Renderer->Reset();
	if (_OverlayOpacity.Get_Value() > 0.0f) {
		float r = _OverlayRed.Get_Value();
		float g = _OverlayGreen.Get_Value();
		float b = _OverlayBlue.Get_Value();
		float a = _OverlayOpacity.Get_Value();
		_Renderer->Add_Quad(RectClass(-1.0f,-1.0f,1.0f,1.0f),FRGBA_TO_INT32(r,g,b,a));
	}

	if (_LetterboxFraction.Get_Value() > 0.0f) {
		float lsize = _LetterboxFraction.Get_Value() * LETTERBOX_SIZE;
		_Renderer->Add_Quad(RectClass(0.0f,0.0f,1.0f,lsize),RGB_TO_INT32(0,0,0));
		_Renderer->Add_Quad(RectClass(0.0f,1.0f - lsize,1.0f,1.0f),RGB_TO_INT32(0,0,0));
	}

}

void ScreenFadeManager::Render()
{
	_Renderer->Render();
}

void ScreenFadeManager::Enable_Letterbox(bool onoff, float time)
{
	float new_val = (onoff ? 1.0f : 0.0f);
	_LetterboxFraction.Set_Target_Value(new_val,time);
}

void ScreenFadeManager::Set_Screen_Overlay_Color(const Vector3 & color,float time)
{
	_OverlayRed.Set_Target_Value(WWMath::Clamp(color.X),time);
	_OverlayGreen.Set_Target_Value(WWMath::Clamp(color.Y),time);
	_OverlayBlue.Set_Target_Value(WWMath::Clamp(color.Z),time);
}

void ScreenFadeManager::Set_Screen_Overlay_Color(float r,float g, float b,float time)
{
	_OverlayRed.Set_Target_Value(WWMath::Clamp(r),time);
	_OverlayGreen.Set_Target_Value(WWMath::Clamp(g),time);
	_OverlayBlue.Set_Target_Value(WWMath::Clamp(b),time);
}

void ScreenFadeManager::Set_Screen_Overlay_Opacity(float opacity,float time)
{
	_OverlayOpacity.Set_Target_Value(opacity,time);
}


/*
**
*/
enum	{
	CHUNKID_LETTERBOX_FRACTION			=	117021200,
	CHUNKID_OVERLAY_OPACITY,
	CHUNKID_OVERLAY_RED,
	CHUNKID_OVERLAY_GREEN,
	CHUNKID_OVERLAY_BLUE,
};

/*
**
*/
bool	ScreenFadeManager::Save( ChunkSaveClass &csave )
{
	csave.Begin_Chunk( CHUNKID_LETTERBOX_FRACTION );
		_LetterboxFraction.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_OVERLAY_OPACITY );
		_OverlayOpacity.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_OVERLAY_RED );
		_OverlayRed.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_OVERLAY_GREEN );
		_OverlayGreen.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_OVERLAY_BLUE );
		_OverlayBlue.Save( csave );
	csave.End_Chunk();

	return true;
}

bool	ScreenFadeManager::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_LETTERBOX_FRACTION:
				_LetterboxFraction.Load( cload );
				break;

			case 	CHUNKID_OVERLAY_OPACITY:
				_OverlayOpacity.Load( cload );
				break;

			case 	CHUNKID_OVERLAY_RED:
				_OverlayRed.Load( cload );
				break;

			case 	CHUNKID_OVERLAY_GREEN:
				_OverlayGreen.Load( cload );
				break;

			case 	CHUNKID_OVERLAY_BLUE:
				_OverlayBlue.Load( cload );
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}
	return true;
}

