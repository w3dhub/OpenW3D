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
 *                 Project Name : Renegade                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/effectrecycler.h                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/11/01 3:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef EFFECTRECYCLER_H
#define EFFECTRECYCLER_H

#include "always.h"
#include "robjlist.h"
#include "multilist.h"
#include "combatphysobserver.h"
#include "renderobjectrecycler.h"

class Matrix3D;
class TimedDecorationPhysDefClass;
class TimedDecorationPhysClass;
class RenderObjClass;

/**
** EffectRecyclerClass
** This class can recycle any "fire-and-forget" eye-candy type rendering objects.  It
** will recycle both the render objects and timed-deco-phys wrappers internally.  This
** system will be used for things in the game like surface-effect particle emitters, 
** explosions, etc.
**
** This object will create/recycle the desired render object, insert it into the combat
** scene, and reclaim the objects when the animation/emitter finishes.
*/
class EffectRecyclerClass : public CombatPhysObserverClass
{
public:

	EffectRecyclerClass(void);
	~EffectRecyclerClass(void);

	void					Reset(void);
	void					Spawn_Effect(TimedDecorationPhysDefClass * definition,const Matrix3D & tm);
	void					Spawn_Effect(const char * render_obj_name,const Matrix3D & tm,float time = -1.0f);
	
	/*
	** PhysObserver Interface - whenever an effect expires, we can re-use the model and
	** physics object.  
	*/
	virtual void		Object_Removed_From_Scene(PhysClass * observed_obj) override;

private:

	TimedDecorationPhysClass *							internal_get_tdeco(void);
	RenderObjClass *										internal_get_model(const char * robj_name,const Matrix3D & tm);

	RenderObjectRecyclerClass							ModelRecycler;
	RefMultiListClass<TimedDecorationPhysClass>	InactiveTDecos;

};


#endif //EFFECTRECYCLER_H


