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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/materialeffect.cpp                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/07/01 2:43p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "materialeffect.h"
#include "materialeffectlist.h"
#include "rinfo.h"
#include "matpass.h"



static NonRefMaterialEffectListClass _AllocatedEffects;



/*************************************************************************************************
**
** MaterialEffectClass Implementation
**
*************************************************************************************************/
MaterialEffectClass::MaterialEffectClass(void) :
	AutoRemoveEnabled(false),
	SuppressShadows(false)
{
	_AllocatedEffects.Add(this);
}

MaterialEffectClass::~MaterialEffectClass(void)
{
	_AllocatedEffects.Remove(this);
}

void MaterialEffectClass::Timestep_All_Effects(float dt)
{
	NonRefMaterialEffectListIterator it(&_AllocatedEffects);
	while (!it.Is_Done()) {
		it.Peek_Obj()->Timestep(dt);
		it.Next();
	}
}


/*************************************************************************************************
**
** SimpleEffectClass Implementation
**
*************************************************************************************************/

DEFINE_AUTO_POOL(SimpleEffectClass, SIMPLE_EFFECT_GROWTH_STEP);

SimpleEffectClass::SimpleEffectClass(MaterialPassClass * matpass) :
	MatPass(NULL)
{
	REF_PTR_SET(MatPass,matpass);
}

SimpleEffectClass::~SimpleEffectClass(void)
{
	REF_PTR_RELEASE(MatPass);
}

void SimpleEffectClass::Render_Push(RenderInfoClass & rinfo,PhysClass *)
{
	rinfo.Push_Material_Pass(MatPass);
}

void SimpleEffectClass::Render_Pop(RenderInfoClass & rinfo)
{
	rinfo.Pop_Material_Pass();
}




