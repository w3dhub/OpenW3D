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
 *                     $Archive:: /Commando/Code/wwphys/materialeffect.h                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/07/01 2:43p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MATERIALEFFECT_H
#define MATERIALEFFECT_H

#include "always.h"
#include "multilist.h"
#include "refcount.h"


class PhysClass;
class RenderInfoClass;
class	MaterialPassClass;



/**
** MaterialEffectClass
** This class is an abstract base class for any kind of "extra pass" material effect.  Phys objects
** support a list of these effects being added to them and will push and pop the effects during
** rendering.  For "transient" effects like shadows, the auto-remove flag can be enabled which
** will cause the physics object to un-link the effect after rendering is complete.
**
**  NOTES:
**
**  I need to abstract the differences between the stealth effect and things like texture projections
**  and shadows.  By clumping all of the stealth effect stuff into an external class, we'll be able
**  to apply it both to the character and any other models like his first person hands, etc.  By
**  unifying texture projectors and these new types of effects, we can just upgrade the list
**  that is already present in all PhysClass's.
**
**  This means that when shadows are linked onto render objects, they will need to be wrapped by
**  a class derived from this new abstract base class.  We'll probably want to recycle the wrappers
**  since shadows are re-attached every frame.  This also means that the wrapper needs an auto-remove
**  flag so the phys object can discard it after rendering is complete if necessary.
**
**  If we want to also use the stealth object for game logic, we need to ensure that its internal
**  variables update even when it doesn't get rendered.  The PhysClass could timestep all of the 
**  currently attached material effects.  The only ones attached during timestep would be the 
**  "persistant" ones...  
**
*/

class MaterialEffectClass : public MultiListObjectClass, public RefCountClass
{
public:

	MaterialEffectClass(void);
	virtual ~MaterialEffectClass(void);

	void					Enable_Auto_Remove(bool onoff)			{ AutoRemoveEnabled = onoff; }
	bool					Is_Auto_Remove_Enabled(void)				{ return AutoRemoveEnabled; }

	void					Enable_Suppress_Shadows(bool onoff)		{ SuppressShadows = onoff; }
	bool					Are_Shadows_Suppressed(void)				{ return SuppressShadows; }

	virtual void		Timestep(float /* dt */)							{ }

	virtual void		Render_Push(RenderInfoClass & rinfo,PhysClass * obj)		= 0;
	virtual void		Render_Pop(RenderInfoClass & rinfo)								= 0;

	static void			Timestep_All_Effects(float dt);

private:

	bool					AutoRemoveEnabled;
	bool					SuppressShadows;
};


/**
** SimpleEffectClass
** This is a material effect object which simply causes a material pass to be added.  The texture
** projection system uses this in "Auto Remove" mode to simply get the shadow pass applied to 
** each object that the shadow falls on.  These objects are Auto-Pooled for fast allocation and
** de-allocation so the user cannot derive a class from SimpleEffectClass.  
*/

#define SIMPLE_EFFECT_GROWTH_STEP 256

class SimpleEffectClass : public MaterialEffectClass , public AutoPoolClass<SimpleEffectClass,SIMPLE_EFFECT_GROWTH_STEP>
{
public:
	SimpleEffectClass(MaterialPassClass * matpass);
	~SimpleEffectClass(void);

	virtual void		Render_Push(RenderInfoClass & rinfo,PhysClass * obj) override;
	virtual void		Render_Pop(RenderInfoClass & rinfo) override;

private:

	MaterialPassClass *	MatPass;

};




#endif //MATERIALEFFECT_H

