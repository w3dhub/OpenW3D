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
 *                 Project Name : wwphys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/transitioneffect.h                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/06/01 6:59p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef TRANSITIONEFFECT_H
#define TRANSITIONEFFECT_H


#include "always.h"
#include "materialeffect.h"
#include "vector2.h"
#include "matrixmapper.h"

class ChunkSaveClass;
class ChunkLoadClass;
class TextureClass;

/**
** TransitionEffectClass
** This material effect can manage and display a "transition" effect for an object.
** It has a requirement that the user "timestep" it each frame so that its internal
** state gets updated.  The transition is controlled by a single parameter which
** when zero, the object is invisible and when 1, the object is fully visible.
*/
class TransitionEffectClass : public MaterialEffectClass
{
public:
	TransitionEffectClass(void);
	~TransitionEffectClass(void);
	
	virtual void		Timestep(float dt) override;
	virtual void		Render_Push(RenderInfoClass & rinfo,PhysClass * obj) override;
	virtual void		Render_Pop(RenderInfoClass & rinfo) override;

	/*
	** Initialization
	*/
	void					Set_Parameter(float param)							{ CurrentParameter = param; }
	void					Set_Target_Parameter(float param)				{ TargetParameter = param; }
	void					Set_Texture(TextureClass * texture);
	void					Set_Start_Delay(float seconds)					{ StartDelay = seconds; }
	void					Set_Transition_Time(float seconds)				{ if (seconds > 0.0f) ParameterVelocity = 1.0f / seconds; }
	void					Set_Min_UV_Velocity(const Vector2 & uv_vel)	{ MinUVRate = uv_vel; }
	void					Set_Max_UV_Velocity(const Vector2 & uv_vel)	{ MaxUVRate = uv_vel; }
	void					Set_Max_Intensity(float max_inten)				{ MaxIntensity = max_inten; }
	void					Enable_Remove_On_Complete(bool onoff)			{ RemoveOnComplete = onoff; }

	float					Get_Parameter(void) const							{ return CurrentParameter; }
	float					Get_Target_Parameter(void) const					{ return TargetParameter; }
	float					Get_Max_Intensity(void) const						{ return MaxIntensity; }
	float					Get_Start_Delay(void) const						{ return StartDelay; }
	bool					Is_Remove_On_Complete_Enabled(void) const		{ return RemoveOnComplete; }

	/*
	** Save-Load support
	*/
	bool					Save(ChunkSaveClass & csave);
	bool					Load(ChunkLoadClass & cload);

protected:
	
	/*
	** Current State
	*/
	float					CurrentParameter;
	float					TargetParameter;

	/*
	** Properties
	*/
	float					ParameterVelocity;
	Vector2				MinUVRate;
	Vector2				MaxUVRate;
	Vector2				UVRate;
	float					MaxIntensity;
	float					StartDelay;
	bool					RemoveOnComplete;

	/* 
	** Render State
	*/
	bool					RenderBaseMaterial;
	bool					RenderTransitionMaterial;
	float					IntensityScale;
	Vector2				UVOffset;

	MatrixMapperClass *		Mapper;
	MaterialPassClass *		MaterialPass;
};

#endif //TRANSITIONEFFECT_H

