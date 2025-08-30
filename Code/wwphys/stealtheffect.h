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
 *                     $Archive:: /Commando/Code/wwphys/stealtheffect.h                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/27/01 11:43a                                             $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef STEALTHEFFECT_H
#define STEALTHEFFECT_H


#include "always.h"
#include "materialeffect.h"
#include "vector2.h"
#include "matrixmapper.h"

class ChunkSaveClass;
class ChunkLoadClass;

/**
** StealthEffectClass
** This material effect can manage and display "stealth" mode for an object.
** It has a requirement that the user "timestep" it each frame so that its internal
** state gets updated.  
*/
class StealthEffectClass : public MaterialEffectClass
{
public:
	StealthEffectClass(void);
	~StealthEffectClass(void);
	
	virtual void		Timestep(float dt) override;
	virtual void		Render_Push(RenderInfoClass & rinfo,PhysClass * obj) override;
	virtual void		Render_Pop(RenderInfoClass & rinfo) override;

	void					Enable_Stealth(bool onoff);
	void					Set_Friendly(bool onoff);
	void					Set_Broken(bool onoff);

	bool					Is_Stealth_Enabled(void)					{ return IsStealthEnabled; }
	bool					Is_Friendly(void)								{ return IsFriendly; }
	bool					Is_Broken(void)								{ return IsBroken; }
	bool					Is_Stealthed(void) const					{ return CurrentFraction > 0.5f; }
	
	void					Set_Fade_Distance(float d)					{ FadeDistance = d; }
	float					Get_Fade_Distance(void) const				{ return FadeDistance; }

	void					Set_Current_State( float percent )		{ CurrentFraction = percent; }
	void					Set_Target_State( float percent )		{ TargetFraction = percent; }

	void					Damage_Occured(void);

	/*
	** Save-Load support
	*/
	bool					Save(ChunkSaveClass & csave);
	bool					Load(ChunkLoadClass & cload);

protected:
	
	void					Update_Target_Fraction(void);

	bool					IsStealthEnabled;		// is stealth supposed to be turned on?
	bool					IsFriendly;				// is this object a friend of the player?
	bool					IsBroken;				// is this object's cloaking unit broken?
	float					FadeDistance;			// distance at which the effect fades completely out

	float					CurrentFraction;
	float					TargetFraction;
	
	Vector2				UVRate;

	// Render State
	bool					RenderBaseMaterial;
	bool					RenderStealthMaterial;
	float					IntensityScale;
	Vector2				UVOffset;

	MatrixMapperClass *		Mapper;
	MaterialPassClass *		MaterialPass;
};


#endif //STEALTHEFFECT_H