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
 *                     $Archive:: /Commando/Code/Combat/effectrecycler.cpp                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/11/01 5:15p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   EffectRecyclerClass::EffectRecyclerClass -- constructor                                   *
 *   EffectRecyclerClass::~EffectRecyclerClass -- destructor                                   *
 *   EffectRecyclerClass::Reset -- releases all resources                                      *
 *   EffectRecyclerClass::Spawn_Effect -- Spawn a timed effect into the combat scene           *
 *   EffectRecyclerClass::Spawn_Effect -- spawn an effect into the combat scene                *
 *   EffectRecyclerClass::Object_Removed_From_Scene -- physobserver interface, reclaim our obj *
 *   EffectRecyclerClass::internal_get_tdeco -- internally recycle or allocate a tdeco         *
 *   EffectRecyclerClass::internal_get_model -- recycle or allocate a render object            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "effectrecycler.h"
#include "timeddecophys.h"
#include "rendobj.h"
#include "combat.h"
#include "assetmgr.h"
#include "htree.h"
#include "hanim.h"
#include "part_emt.h"


/***********************************************************************************************
 * EffectRecyclerClass::EffectRecyclerClass -- constructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
EffectRecyclerClass::EffectRecyclerClass(void)
{
}


/***********************************************************************************************
 * EffectRecyclerClass::~EffectRecyclerClass -- destructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
EffectRecyclerClass::~EffectRecyclerClass(void)
{
	Reset();
}


/***********************************************************************************************
 * EffectRecyclerClass::Reset -- releases all resources                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/8/2001   gth : Created.                                                                 *
 *   7/11/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void EffectRecyclerClass::Reset(void)
{
	ModelRecycler.Reset();
	InactiveTDecos.Reset_List();
}


/***********************************************************************************************
 * EffectRecyclerClass::Spawn_Effect -- Spawn a timed effect into the combat scene             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/8/2001   gth : Created.                                                                 *
 *=============================================================================================*/
void EffectRecyclerClass::Spawn_Effect(TimedDecorationPhysDefClass * def,const Matrix3D & tm)
{
	Spawn_Effect(def->Get_Model_Name(),tm,def->Get_Lifetime());
}


/***********************************************************************************************
 * EffectRecyclerClass::Spawn_Effect -- spawn an effect into the combat scene                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/8/2001   gth : Created.                                                                 *
 *=============================================================================================*/
void EffectRecyclerClass::Spawn_Effect(const char * robj_name,const Matrix3D & tm,float time)
{
	// allocate/recycle the render object
	RenderObjClass * model = internal_get_model(robj_name,tm);

	if (model != NULL) {

		// allocate/recycle a TDeco
		TimedDecorationPhysClass * tdeco = internal_get_tdeco();

		if ((time == -1.0f) && (model->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER)) {
			ParticleEmitterClass * emitter = (ParticleEmitterClass *)model;
			time = emitter->Get_Lifetime();
		}

		// configure and add to the scene
		tdeco->Set_Model(model);
		tdeco->Set_Lifetime(time);
		tdeco->Enable_Dont_Save(true);
		tdeco->Enable_Is_Pre_Lit(true);
		tdeco->Set_Collision_Group(UNCOLLIDEABLE_GROUP);
		tdeco->Set_Observer(this);

		COMBAT_SCENE->Add_Dynamic_Object(tdeco);
		tdeco->Set_Transform(tm);

		REF_PTR_RELEASE(model);
		REF_PTR_RELEASE(tdeco);
	}
}


/***********************************************************************************************
 * EffectRecyclerClass::Object_Removed_From_Scene -- physobserver interface, reclaim our objec *
 *                                                                                             *
 *    Reclaims the model and tdeco when they expire.                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/8/2001   gth : Created.                                                                 *
 *=============================================================================================*/
void EffectRecyclerClass::Object_Removed_From_Scene(PhysClass * observed_obj)
{
	WWASSERT(observed_obj != NULL);
	WWASSERT(observed_obj->As_TimedDecorationPhysClass() != NULL);
	WWASSERT(observed_obj->Peek_Model() != NULL);

	TimedDecorationPhysClass * tdeco = observed_obj->As_TimedDecorationPhysClass();
	RenderObjClass * model = tdeco->Peek_Model();
	ModelRecycler.Return_Render_Object(model);
	tdeco->Set_Model(NULL);

	InactiveTDecos.Add(tdeco);
}


/***********************************************************************************************
 * EffectRecyclerClass::internal_get_tdeco -- internally recycle or allocate a tdeco           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/8/2001   gth : Created.                                                                 *
 *=============================================================================================*/
TimedDecorationPhysClass * EffectRecyclerClass::internal_get_tdeco(void)
{
	if (InactiveTDecos.Is_Empty()) {
		return NEW_REF(TimedDecorationPhysClass,());
	} else {
		return InactiveTDecos.Remove_Head();
	}
}


/***********************************************************************************************
 * EffectRecyclerClass::internal_get_model -- recycle or allocate a render object              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/8/2001   gth : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * EffectRecyclerClass::internal_get_model(const char * robj_name,const Matrix3D & tm)
{
	RenderObjClass * model = ModelRecycler.Get_Render_Object(robj_name,tm);

	if ((model != NULL) && (model->Get_HTree() != NULL)) {
		/*
		** Auto play an anim if we find it
		*/
		StringClass	anim_name;
		anim_name.Format( "%s.%s",
			model->Get_HTree()->Get_Name(),
			model->Get_HTree()->Get_Name() );
		WWASSERT(WW3DAssetManager::Get_Instance() != NULL);
		HAnimClass * anim = WW3DAssetManager::Get_Instance()->Get_HAnim( anim_name );
		if ( anim != NULL ) {
			model->Set_Animation( anim, 0, RenderObjClass::ANIM_MODE_ONCE );
			anim->Release_Ref();
		}
	}

	return model;
}

