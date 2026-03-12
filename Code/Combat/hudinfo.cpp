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
 *                     $Archive:: /Commando/Code/Combat/hudinfo.cpp                           $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/03/02 8:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "hudinfo.h"
#include "combat.h"
#include "ccamera.h"
#include "gametype.h"
#include "debug.h"

/*
**
*/
GameObjReference	HUDInfo::WeaponTargetObject;
Vector3				HUDInfo::WeaponTargetPosition( 0,0,0 );
GameObjReference	HUDInfo::InfoObject;
float			 		HUDInfo::InfoObjectTimer;
bool			 		HUDInfo::DisplayActionBar		= false;
float			 		HUDInfo::ActionStatusValue		= 0;
bool					HUDInfo::IsHUDHelpTextDirty	= true;
WideStringClass	HUDInfo::HUDHelpText;
Vector3				HUDInfo::HUDHelpTextColor (1.0F, 1.0F, 1.0F);
bool					HUDInfo::IsMCT	 = false;


/*
**
*/
void	HUDInfo::Set_Info_Object( DamageableGameObj * obj, bool is_mct )
{
	static void * last_info_object = NULL;
	static float  last_info_health = 0;

//	Debug_Say(( "--- Info Object %s\n", obj ? obj->Get_Definition().Get_Name() : "NONE" ));

	IsMCT	= is_mct;

	if ( obj != NULL && obj->As_BuildingGameObj() != NULL ) {
		if ( last_info_object == obj ) {
			// Don't re-select buildings unless it has been damaged
			if ( last_info_health == obj->Get_Defense_Object()->Get_Health() ) {
				// Only in single player and only non mct
				if ( !is_mct && IS_MISSION ) {
					return;
				}
			}
		}
	}

	if ( obj != NULL ) {
		last_info_object = obj;
		last_info_health = obj->Get_Defense_Object()->Get_Health();
	}
	InfoObject = obj;
	InfoObjectTimer = 0;

//	Debug_Say(( "Set Info Object %s\n", obj ? obj->Get_Definition().Get_Name() : "NONE" ));

}

/*
**
*/
void	HUDInfo::Update_Info_Object( void )
{
	// Timeout info for buildings	only
	DamageableGameObj * info = Get_Info_Object();

	// Forget Building as soon as we are off them (in MP)
	if ( !IS_MISSION && info != NULL && info->As_BuildingGameObj() != NULL ) {
		if ( HUDInfo::InfoObjectTimer > 0 ) {
			InfoObject = NULL;
			info = NULL;
		}
	}

	// Forget Dead
	if ( info != NULL && info->Get_Defense_Object()->Get_Health() == 0 ) {
		InfoObject = NULL;
		info = NULL;
	}

//	if ( info != NULL && info->As_BuildingGameObj() == NULL ) {
	if ( info != NULL ) {

		HUDInfo::InfoObjectTimer += TimeManager::Get_Frame_Seconds();
		if ( HUDInfo::InfoObjectTimer > 5 ) {
			InfoObject = NULL;
			info = NULL;
		} else {
			// Forget him if we fall off target
#if 0
			if ( info->As_PhysicalGameObj() != NULL ) {
				Vector3 obj_pos = info->As_PhysicalGameObj()->Get_Bullseye_Position();
				Vector3 screen_pos;
				if (COMBAT_CAMERA->Project(screen_pos, obj_pos) == CameraClass::INSIDE_FRUSTUM) {
#else
			if ( info->As_PhysicalGameObj() != NULL && info->As_PhysicalGameObj()->Peek_Physical_Object() != NULL ) {
				AABoxClass bounds = info->As_PhysicalGameObj()->Peek_Physical_Object()->Get_Cull_Box();
				if ( !COMBAT_CAMERA->Cull_Box( bounds ) ) {
#endif

#if 0
					Vector2	reticle_offset = COMBAT_CAMERA->Get_Camera_Target_2D_Offset();
					screen_pos.X -= reticle_offset.X;
					screen_pos.Y -= reticle_offset.Y;
					screen_pos.Z = 0;
					if ( screen_pos.Length() > 0.25f ) {
						if ( HUDInfo::InfoObjectTimer > 0.5f ) {
							InfoObject = NULL;
						}
					}
#endif
				} else {
					InfoObject = NULL;
				}
			}
		}
	}
}


void	HUDInfo::Set_Weapon_Target_Object( DamageableGameObj * obj )
{
//	Debug_Say(( "Set Weapon Target %s\n", obj ? obj->Get_Definition().Get_Name() : "NONE" ));
	WeaponTargetObject = obj;
}
