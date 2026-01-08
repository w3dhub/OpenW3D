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
 *                     $Archive:: /Commando/Code/Commando/multihud.cpp                        $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/11/02 6:06p                                               $*
 *                                                                                             *
 *                    $Revision:: 139                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "multihud.h"

#include <stdio.h>

#include "soldier.h"
#include "assets.h"
#include "wwprofile.h"
#include "playermanager.h"
#include "teammanager.h"
#include "cnetwork.h"
#include "timemgr.h"
#include "miscutil.h"
#include "gamedata.h"
#include "combat.h"
#include "ccamera.h"
#include "gameobjmanager.h"
#include "smartgameobj.h"
#include "vehicle.h"
#include "physcoltest.h"
#include "pscene.h"
#include "phys.h"
#include "damage.h"
#include "movephys.h"
#include "humanphys.h"
#include "weapons.h"
#include "useroptions.h"
#include "devoptions.h"
#include "wwdebug.h"
#include "overlay.h"
#include "spawn.h"
#include "string_ids.h"
#include "translatedb.h"
#include "texture.h"
#include "render2d.h"
#include "render2dsentence.h"
#include "networkobjectmgr.h"
#include "networkobject.h"
#include "building.h"
#include "powerup.h"
#include "staticnetworkobject.h"
#include "widestring.h"
#include "font3d.h"
#include "apppackettypes.h"
#include "bandwidthgraph.h"
#include "priority.h"
#include "ConsoleMode.h"
#include "stylemgr.h"
#include "demosupport.h"

//
// MultiHUDClass statics
//
const float					MultiHUDClass::MAX_OVERLAY_DISTANCE_M		= 50;
const float					MultiHUDClass::Y_INCREMENT_FACTOR			= 1.2f;

#ifdef WWDEBUG
Render2DTextClass	*		MultiHUDClass::PTextRenderer					= NULL;
Font3DInstanceClass *	MultiHUDClass::PFont								= NULL;
#endif

Render2DSentenceClass* MultiHUDClass::NameRenderer					= NULL;
float							MultiHUDClass::BottomTextYPos					= 0;
//bool							MultiHUDClass::VerboseLists					= false;
PlayerlistFormatEnum		MultiHUDClass::PlayerlistFormat				= PLAYERLIST_FORMAT_TINY;
bool							MultiHUDClass::IsOn								= false;

//-----------------------------------------------------------------------------
void MultiHUDClass::Init(void)
{
	WWDEBUG_SAY(("MultiHUDClass::Init\n"));
	if (!ConsoleBox.Is_Exclusive()) {
#ifdef WWDEBUG
		WWASSERT(WW3DAssetManager::Get_Instance() != NULL);
   	PFont = WW3DAssetManager::Get_Instance()->Get_Font3DInstance("FONT6x8.TGA");
   	WWASSERT(PFont != NULL);
		PFont->Set_Mono_Spaced();
		SET_REF_OWNER(PFont);

		PTextRenderer = new Render2DTextClass(PFont);
		PTextRenderer->Set_Coordinate_Range(Render2DClass::Get_Screen_Resolution());
#endif

		NameRenderer = new Render2DSentenceClass;
		StyleMgrClass::Assign_Font(NameRenderer, StyleMgrClass::FONT_INGAME_TXT);
		NameRenderer->Set_Mono_Spaced(true);

		IsOn = true;
	}
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Shutdown(void)
{
	WWDEBUG_SAY(("MultiHUDClass::Shutdown\n"));

	if (NameRenderer) {
		delete NameRenderer;
		NameRenderer = NULL;
	}

#ifdef WWDEBUG
	if (PTextRenderer != NULL) {
		delete PTextRenderer;
		PTextRenderer = NULL;
	}

	if (PFont != NULL) {
		PFont->Release_Ref();
		PFont = NULL;
	}
#endif

	IsOn = false;
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Render_Text(WideStringClass & text, float x, float y, unsigned int color)
{
	if (NameRenderer) {
		//
		// Text comes out blurry with the new text system if the text position
		// is non-integral.
		//

		x = cMathUtil::Round(x);
		y = cMathUtil::Round(y);

		NameRenderer->Set_Location(Vector2(x, y));
		NameRenderer->Build_Sentence(text);
		NameRenderer->Draw_Sentence(color);
	}
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Toggle(void)
{
	if (IsOn) {
		Shutdown();
	} else {
		Init();
	}
}

//-----------------------------------------------------------------------------
bool MultiHUDClass::Is_On(void)
{
	return IsOn;
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Render(void)
{
#ifdef WWDEBUG
	if (PTextRenderer != NULL) {
		WWPROFILE("MultiHud Debug Render");
		PTextRenderer->Render();
	}
#endif

	if (NameRenderer != NULL) {
		WWPROFILE("MultiHud Render");
		NameRenderer->Render();
	}
}

//-----------------------------------------------------------------------------
//
// Simple but presumably hugely inefficient code follows
//
void MultiHUDClass::Show_Player_Names(void)
{
	if (NameRenderer == NULL) {
		return;
	}

	WWPROFILE("Show_Player_Name");
   if (GameModeManager::Find("Menu")->Is_Active() ||
		COMBAT_CAMERA == NULL ||
		!cNetwork::I_Am_Client() ||
      cUserOptions::ShowNamesOnSoldier.Is_False()) {

		return;
   }

   //
   // Project name of each player onto his commando
	//

   for (
		SLNode<SmartGameObj> * smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head();
		smart_objnode;
		smart_objnode = smart_objnode->Next()) {

		SmartGameObj * p_smart_obj = smart_objnode->Data();
      WWASSERT(p_smart_obj != NULL);

		//
		// We will eliminate candidates with sequential tests. Want to
		// avoid deep conditional nesting here
		//

		if (p_smart_obj->Is_Delete_Pending()) {
			continue;
      }

      //
		// We are only interested in soldiers
		//
		if (p_smart_obj->As_SoldierGameObj() == NULL) {
			continue;
      }

      //
		// ... living soldiers
		//
		WWASSERT(p_smart_obj->Get_Defense_Object() != NULL);
      float health = p_smart_obj->Get_Defense_Object()->Get_Health();
		if (health < WWMATH_EPSILON) {
			continue;
		}

		//

		// ... with player objects
		//
		cPlayer * p_player = NULL;
      if (p_smart_obj->Has_Player()) {
         p_player = cPlayerManager::Find_Player(p_smart_obj->Get_Control_Owner());
      }
		if (p_player == NULL) {
			continue;
		}

      //
		// Is this guy inside our frustrum?
		//
		Vector3 text_position_3d;
		p_smart_obj->Get_Position(&text_position_3d);
      text_position_3d.Z += 1.3f;

	   Vector3 text_position_2d;
      if (COMBAT_CAMERA->Project(text_position_2d, text_position_3d) != CameraClass::INSIDE_FRUSTUM) {
			continue;
		}

		//
		// Take stealthing into account.
		//
		bool is_stealthed = false;
		SoldierGameObj * p_soldier = p_smart_obj->As_SoldierGameObj();
		WWASSERT(p_soldier != NULL);
		VehicleGameObj * p_vehicle = NULL;
		if (p_soldier->Is_Stealthed()) {
			is_stealthed = true;
		} else {
			p_vehicle = p_soldier->Get_Vehicle();
			if (p_vehicle != NULL && p_vehicle->Is_Stealthed()) {
				is_stealthed = true;
			}
		}

		if (COMBAT_STAR != NULL && p_soldier->Is_Teammate(COMBAT_STAR)) {
			//
			// Don't skip for teammates...
			//
		} else if (is_stealthed) {
			continue;
		}

		//
		// adjust x,y from range -1 to 1
		//
		text_position_2d.X = (text_position_2d.X + 1) * Render2DClass::Get_Screen_Resolution().Width() / 2;
		text_position_2d.Y = (1 - text_position_2d.Y) * Render2DClass::Get_Screen_Resolution().Height() / 2;

      //
		// Is this guy nearby?
		//
		Vector3 ray_start = COMBAT_CAMERA->Get_Transform().Get_Translation();
		Vector3 delta = text_position_3d - ray_start;
		float apparent_distance = delta.Length();
		if (COMBAT_CAMERA->Is_Star_Sniping()) {
			//
			// adjust the apparent distance
			//
			float zoom = COMBAT_CAMERA->Get_Profile_Zoom();
			WWASSERT(zoom > 0);
			apparent_distance /= zoom;
		}
		if (apparent_distance > MAX_OVERLAY_DISTANCE_M) {
			continue;
		}

		if (CombatManager::Is_First_Person() && COMBAT_STAR != NULL) {
			//
			// Ignore collision between ray and own commando
			//
			COMBAT_STAR->Peek_Physical_Object()->Inc_Ignore_Counter();
		}

      //
      // Ok! This guy deserves a ray-cast for visibility...
      //
		Vector3 ray_end = ray_start + delta * 1.1f;
		LineSegClass ray(ray_start, ray_end);
		CastResultStruct res;
		PhysRayCollisionTestClass raytest(ray, &res,
			BULLET_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL);

		{
			WWPROFILE("Cast Ray");
			COMBAT_SCENE->Cast_Ray(raytest);
		}

		if (CombatManager::Is_First_Person() && COMBAT_STAR != NULL) {
			COMBAT_STAR->Peek_Physical_Object()->Dec_Ignore_Counter();
		}

		if (raytest.CollidedPhysObj == NULL) {
			//
			// I think this means that the ray collided with the terrain.
			//
			continue;
		}

		if (raytest.CollidedPhysObj->Get_Observer() == NULL) {
			continue;
		} else {
			PhysicalGameObj * p_blocker = ((CombatPhysObserverClass *)raytest.CollidedPhysObj->Get_Observer())->As_PhysicalGameObj();
	      //if (p_blocker != p_smart_obj) {
	      if ((p_blocker == NULL) || 
				 ((p_blocker != p_smart_obj) && (p_blocker != p_vehicle))) {
			   //
				// The ray failed to collide with the soldier in question
				//
	         continue;
		   }
		}

		//
		// OK, we should go ahead and draw the name
		//

      //
      // Show name text
      //
      if (cUserOptions::ShowNamesOnSoldier.Is_True()) {

			WideStringClass text(0,true);
			text += p_player->Get_Name();

#ifdef WWDEBUG
			if (p_player->Invulnerable.Is_True()) {
				text += U_CHAR("\n");
				text += TRANSLATE (IDS_MENU_GOD);
			}

			if (p_player->Get_Damage_Scale_Factor() < 100) {
				text += U_CHAR("\n");
				text += TRANSLATE (IDS_MENU_VIP);
			}
#endif // WWDEBUG

			//
			// MVP carries over into next game
			//
			WWASSERT(The_Game() != NULL);
			WideStringClass mvp_name = The_Game()->Get_Mvp_Name();
			if (!p_player->Get_Name().Compare_No_Case(mvp_name.Peek_Buffer())) {
				text += U_CHAR("\n");
				text += TRANSLATE(IDS_MP_MVP);
				if (The_Game()->Get_Mvp_Count() > 1) {
					WideStringClass consecutives_text;
					consecutives_text.Format(U_CHAR(" * %d"), The_Game()->Get_Mvp_Count());
					text += consecutives_text;
				}
			}

			//
			// WOL game newbies get a "recruit" tag
			//
			if (GameModeManager::Find("WOL")->Is_Active() &&
				 COMBAT_STAR != NULL &&
				 p_soldier->Is_Teammate(COMBAT_STAR) &&
				 p_player->Get_Num_Wol_Games() <= cPlayer::NUM_NEWBIE_GAMES) {

				text += U_CHAR("\n");
				text += TRANSLATE(IDS_MP_RECRUIT);
			}

			Vector2 textExtent = NameRenderer->Get_Text_Extents(text);
			float x = text_position_2d.X - textExtent.X / 2.0f;
			float y = text_position_2d.Y;

			float scale = (MAX_OVERLAY_DISTANCE_M - apparent_distance) / MAX_OVERLAY_DISTANCE_M;

			int color = (((int)(scale * 255) & 0xFF) << 24) |
				(VRGB_TO_INT32(p_player->Get_Color()) & 0x00FFFFFF);

			Render_Text(text, x, y, color);
      }
   }
}

#ifdef WWDEBUG
//-----------------------------------------------------------------------------
void MultiHUDClass::Render_Debug_Text(const char *text, float x, float y, unsigned int color)
{
	//
	// Text comes out blurry with the new text system if the text position
	// is non-integral.
	//

	WWASSERT(text != NULL);

	x = cMathUtil::Round(x);
	y = cMathUtil::Round(y);

	if (PTextRenderer != NULL) {
		WWASSERT(PTextRenderer != NULL);
		PTextRenderer->Set_Location(Vector2(x, y));
		PTextRenderer->Draw_Text(text, color);
	}
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Show_Import_State_Counts(NetworkObjectClass *object)
{
   WWASSERT(object != NULL);

	if (PTextRenderer == NULL) {
		return;
	}

	bool show = false;

	if (cDevOptions::ShowImportStates.Is_True())
	{
		show = true;
	}

	float z_offset = 0;
	if (cDevOptions::ShowImportStatesSV.Is_True() &&
		(object->Get_App_Packet_Type() == APPPACKETTYPE_SOLDIER ||
		 object->Get_App_Packet_Type() == APPPACKETTYPE_VEHICLE))
	{
		z_offset = 1.5f;
		show = true;
	}

   Vector3 text_position_3d;
	if (	show &&
			COMBAT_CAMERA != NULL &&
			object->Get_World_Position (text_position_3d))
	{
      text_position_3d.Z += z_offset;

      Vector3 text_position_2d;
      if (COMBAT_CAMERA->Project(text_position_2d, text_position_3d) == CameraClass::INSIDE_FRUSTUM) {

			text_position_2d.X = (text_position_2d.X + 1) * Render2DClass::Get_Screen_Resolution().Width() / 2;
			text_position_2d.Y = (1 - text_position_2d.Y) * Render2DClass::Get_Screen_Resolution().Height() / 2;

			StringClass text;
			StringClass sub_string;

         if (cNetwork::I_Am_Only_Client()) {
				sub_string.Format("ISC %d\n",  object->Get_Import_State_Count());
				text += sub_string;
         }

			/*
			if (cNetwork::I_Am_Server()) {
				sub_string.Format(" PRI %7.4f\n", object->Get_Cached_Priority ());
				text += sub_string;
			}
			*/

			float x = text_position_2d.X - PTextRenderer->Peek_Font()->String_Width("XXXX XXXXX") / 2.0f;
			float y = text_position_2d.Y;

			int color = 0xFFFFFFFF; // white
         if (object->Get_Import_State_Count() % 2 != 0) {
				color = 0xFF00FF00; // green
			}

			Render_Debug_Text(text, x, y, color);
      }
   }
}



void MultiHUDClass::Show_Distance_And_Priority(NetworkObjectClass *object)
{
   WWASSERT(object != NULL);

	if (PTextRenderer == NULL) {
		return;
	}

	bool show = false;

	if (cDevOptions::ShowPriorities.Is_True()) {
		show = true;
	}

   Vector3 text_position_3d;
	if (show && COMBAT_CAMERA != NULL && object->Get_World_Position (text_position_3d))
	{
      SmartGameObj * player_ptr = GameObjManager::Find_Soldier_Of_Client_ID(cNetwork::Get_My_Id());

		if (player_ptr) {
			Vector3 position;
			player_ptr->Get_Position(&position);
			float priority = cPriority::Compute_Object_Priority(cNetwork::Get_My_Id(), position, object, true);
			float distance = cPriority::Get_Object_Distance(position, object);

      	Vector3 text_position_2d;
      	if (COMBAT_CAMERA->Project(text_position_2d, text_position_3d) == CameraClass::INSIDE_FRUSTUM) {

				text_position_2d.X = (text_position_2d.X + 1) * Render2DClass::Get_Screen_Resolution().Width() / 2;
				text_position_2d.Y = (1 - text_position_2d.Y) * Render2DClass::Get_Screen_Resolution().Height() / 2;

				StringClass text;
				StringClass sub_string;

         	if (cNetwork::I_Am_Only_Client()) {
					sub_string.Format("DIS %.1f\n", distance);
					text += sub_string;
         	}

				StringClass text2;
				StringClass sub_string2;

         	if (cNetwork::I_Am_Only_Client()) {
					sub_string2.Format("PRI %.2f\n", priority);
					text2 += sub_string2;
         	}


				/*
				if (cNetwork::I_Am_Server()) {
					sub_string.Format(" PRI %7.4f\n", object->Get_Cached_Priority ());
					text += sub_string;
				}
				*/

				float x = text_position_2d.X - PTextRenderer->Peek_Font()->String_Width("XXXX XXXXX") / 2.0f;
				float y = text_position_2d.Y;

				int color = 0xFFFFFFFF; // white
         	if (distance < 300.0f) {
					color = 0xFF00FF00; // green
				}

				// Render down a bit so we don't overlap with ISC info.
				Render_Debug_Text(text, x, y+10, color);
				Render_Debug_Text(text2, x, y+20, color);
      	}
		}
   }
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Show_Player_Rhost_Data(SmartGameObj * smart_obj)
{
   WWASSERT(smart_obj != NULL);

	if (PTextRenderer == NULL) {
		return;
	}

	if (cDevOptions::ShowServerRhostData.Is_True() &&
		 cNetwork::I_Am_Server() &&
		 COMBAT_CAMERA != NULL &&
		 smart_obj->Is_Human_Controlled() &&
		 !smart_obj->Is_Delete_Pending()) {

		int controlling_client = smart_obj->Get_Control_Owner();
      cPlayer * p_player = cPlayerManager::Find_Player(controlling_client);
      WWASSERT(p_player != NULL);

		if (!The_Game() || The_Game()->IsDedicated.Is_False()) {
			Vector3 text_position_3d;
      	smart_obj->Get_Position(&text_position_3d);
      	text_position_3d.Z += 1.5f;

      	Vector3 text_position_2d;
      	if (COMBAT_CAMERA->Project(text_position_2d, text_position_3d) == CameraClass::INSIDE_FRUSTUM) {

				text_position_2d.X = (text_position_2d.X + 1) * Render2DClass::Get_Screen_Resolution().Width() / 2;
				text_position_2d.Y = (1 - text_position_2d.Y) * Render2DClass::Get_Screen_Resolution().Height() / 2;

				SoldierGameObj * p_soldier = smart_obj->As_SoldierGameObj();

				if (p_soldier != NULL) {

					cRemoteHost * p_rhost = cNetwork::Get_Server_Rhost(controlling_client);
					WWASSERT(p_rhost != NULL);

					StringClass text;
					StringClass sub_string;

					sub_string.Format( "Id:   %d\n", controlling_client);
					text += sub_string;

					sub_string.Format( "TP:   %-7.4f\n", p_rhost->Get_Threshold_Priority());
					text += sub_string;

					sub_string.Format( "TBps: %d\n", p_rhost->Get_Target_Bps());
					text += sub_string;

					sub_string.Format( "RTMs: %d\n", p_rhost->Get_Resend_Timeout_Ms());
					text += sub_string;

					float x = text_position_2d.X - PTextRenderer->Peek_Font()->String_Width("AAAA: AAAA") / 2.0f;
					float y = text_position_2d.Y;

					Render_Debug_Text(text, x, y);
				}
			}
		} else {

			cRemoteHost * p_rhost = cNetwork::Get_Server_Rhost(controlling_client);
			float x = 20;
			float y = 60 + controlling_client * 10;

			WideStringClass name(p_player->Get_Name(), true);
			StringClass short_name;
			name.Convert_To(short_name);
			int len = name.Get_Length();
			char temp_name[256];
			strcpy(temp_name, short_name.Peek_Buffer());
			strncat(temp_name, "                   ", 13-len);


			StringClass text(temp_name, true);
			StringClass sub_string;

			sub_string.Format( "Id: %03d    ", controlling_client);
			text += sub_string;

			sub_string.Format( "AP:%1.2f    ", p_rhost->Get_Average_Priority());
			text += sub_string;

			sub_string.Format( "BM:%05.2f    ", p_rhost->Get_Bandwidth_Multiplier());
			text += sub_string;

			sub_string.Format( "TBps:%08d  ", p_rhost->Get_Target_Bps());
			text += sub_string;

			sub_string.Format( "RTMs:%05d  ", p_rhost->Get_Resend_Timeout_Ms());
			text += sub_string;

			sub_string.Format( "%05d/%05d/%05d     ", p_rhost->Get_Min_Internal_Pingtime_Ms(), p_rhost->Get_Max_Internal_Pingtime_Ms(), p_rhost->Get_Average_Internal_Pingtime_Ms());
			text += sub_string;

			sub_string.Format( "Rs:%05d ", p_rhost->Get_Total_Resends());
			text += sub_string;

			unsigned int time = TIMEGETTIME() - p_rhost->Get_Creation_Time();
			time = time / 1000;
			sub_string.Format( " Con: %03d.%02d.%02d", time / (60*60), (time / 60) % 60, time % 60);
			text += sub_string;

			Render_Debug_Text(text, x, y);

			if (controlling_client == 1) {

				y = 50;

				StringClass text("Name         ", true);
				StringClass sub_string;

				sub_string.Format( "RHost ID   ");
				text += sub_string;

				sub_string.Format( "Ave Pri    ");
				text += sub_string;

				sub_string.Format( "Band Mult   ");
				text += sub_string;

				sub_string.Format( "Target BPS     ");
				text += sub_string;

				sub_string.Format( "Resend dly  ");
				text += sub_string;

				sub_string.Format( "Min/Max/Ave ping      ");
				text += sub_string;

				sub_string.Format( "Tot rsnds ");
				text += sub_string;

				sub_string.Format( "Duration");
				text += sub_string;

				Render_Debug_Text(text, x, y);
			}
		}
   }
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Show_Client_Rhost_Data(void)
{
	if (PTextRenderer == NULL) {
		return;
	}

   if (cDevOptions::ShowClientRhostData.Is_True() &&
		 cNetwork::I_Am_Client() &&
		 COMBAT_CAMERA != NULL) {

		cRemoteHost * p_rhost = cNetwork::Get_Client_Rhost();

		if (p_rhost != NULL) {

			StringClass text;
			StringClass sub_string;

			sub_string.Format("Id:   %d\n", 0);
			text += sub_string;

			sub_string.Format("TP:   %-7.4f\n", p_rhost->Get_Threshold_Priority());
			text += sub_string;

			sub_string.Format("TBps: %d\n", p_rhost->Get_Target_Bps());
			text += sub_string;

			sub_string.Format("RTMs: %d\n", p_rhost->Get_Resend_Timeout_Ms());
			text += sub_string;

			float x = 20;
			float y = 20;
			Render_Debug_Text(text, x, y);
		}
   }
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Show_Description(NetworkObjectClass * p_object, float height_offset, float max_distance)
{
   WWASSERT(p_object != NULL);
	WWASSERT(max_distance > 0);

	if (PTextRenderer == NULL) {
		return;
	}

   if (COMBAT_CAMERA != NULL &&
		 !p_object->Is_Delete_Pending()) {

      Vector3 text_position_3d;
      if (p_object->Get_World_Position(text_position_3d)) {
			text_position_3d.Z += height_offset;

			Vector3 ray_start = COMBAT_CAMERA->Get_Transform().Get_Translation();
			Vector3 delta = text_position_3d - ray_start;

			Vector3 text_position_2d;
			if (delta.Length() < max_distance &&
				COMBAT_CAMERA->Project(text_position_2d, text_position_3d) == CameraClass::INSIDE_FRUSTUM) {

				text_position_2d.X = (text_position_2d.X + 1) * Render2DClass::Get_Screen_Resolution().Width() / 2;
				text_position_2d.Y = (1 - text_position_2d.Y) * Render2DClass::Get_Screen_Resolution().Height() / 2;

				float x = text_position_2d.X;
				float y = text_position_2d.Y;

				StringClass description;
				p_object->Get_Description(description);

				if (!description.Is_Empty()) {
					float scale = (max_distance - delta.Length()) / max_distance;
					int color = (((int)(scale * 255) & 0xFF) << 24) | 0x00FFFFFF;

					Render_Debug_Text(description, x, y, color);
				}
			}
		}
   }
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Show_Spawner_Data(void)
{
	if (PTextRenderer == NULL) {
		return;
	}

	if (COMBAT_CAMERA != NULL && cDevOptions::ShowSpawnerData.Is_True()) {

		DynamicVectorClass<SpawnerClass*> spawner_list = SpawnManager::Get_Spawner_List();

		for (int i = 0; i < spawner_list.Count(); i++) {

			WWASSERT(spawner_list[i] != NULL);
			Vector3 text_position_3d;
			spawner_list[i]->Get_TM().Get_Translation(&text_position_3d);
			text_position_3d.Z += 1.0f;

			Vector3 text_position_2d;
			if (COMBAT_CAMERA->Project(text_position_2d, text_position_3d) == CameraClass::INSIDE_FRUSTUM) {

				char text[300];
				::strcpy(text, spawner_list[i]->Get_Definition().Get_Name());

				text_position_2d.X = (text_position_2d.X + 1) * Render2DClass::Get_Screen_Resolution().Width() / 2;
				text_position_2d.Y = (1 - text_position_2d.Y) * Render2DClass::Get_Screen_Resolution().Height() / 2;

				float x = text_position_2d.X - PTextRenderer->Peek_Font()->String_Width(text) / 2.0f;
				float y = text_position_2d.Y;

				Render_Debug_Text(text, x, y);
			}
		}
	}
}
#endif // WWDEBUG

//-----------------------------------------------------------------------------
void MultiHUDClass::Think(void)
{
#ifdef WWDEBUG
	if (PTextRenderer == NULL) {
		return;
	}
#endif

	if (NameRenderer == NULL) {
		return;
	}

	WWPROFILE("MultiHud Think");

#ifdef WWDEBUG
	PTextRenderer->Reset();
#endif

	NameRenderer->Reset();

	DEMO_SECURITY_CHECK;

	if (!GameModeManager::Find("Combat")->Is_Active() ||
		  GameModeManager::Find("Menu")->Is_Active()) {
		return;
	}

   Show_Player_Names();

//	BottomTextYPos = Render2DClass::Get_Screen_Resolution().Bottom - 15;

	//
	// From here down its all diagnostics
	//

#ifdef WWDEBUG

	//
	//	Loop over each network object
	//
	int count = NetworkObjectMgrClass::Get_Object_Count ();
	for (int index = 0; index < count; index ++) {
		NetworkObjectClass *object = NetworkObjectMgrClass::Get_Object (index);

		//
		//	Display the import state count and priority for each object
		//
		Show_Import_State_Counts (object);
		Show_Distance_And_Priority(object);

	}

	{
	int count = StaticNetworkObjectClass::Get_Static_Network_Object_Count ();
	for (int index = 0; index < count; index ++) {

		StaticNetworkObjectClass * p_object = (StaticNetworkObjectClass * ) StaticNetworkObjectClass::Get_Static_Network_Object (index);
		WWASSERT(p_object != NULL);

		if (
			(p_object->As_Door_Network_Object_Class() != NULL && cDevOptions::ShowDoorData.Is_True()) ||
			(p_object->As_Elevator_Network_Object_Class() != NULL && cDevOptions::ShowElevatorData.Is_True()) ||
			(p_object->As_DSAPO_Network_Object_Class() != NULL && cDevOptions::ShowDSAPOData.Is_True())) {

			Show_Description(p_object, 0, 20);
		}
	}
	}

	//
	// Traverse objects and show pertinent diagnostics
	//
	SLNode<BaseGameObj> * objnode;
	for (objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
		WWASSERT(objnode->Data() != NULL);
      PhysicalGameObj * p_phys_obj = objnode->Data()->As_PhysicalGameObj();

		if (p_phys_obj != NULL) {

			SmartGameObj * p_smart_obj = p_phys_obj->As_SmartGameObj();

			if (p_smart_obj != NULL) {
				Show_Player_Rhost_Data(p_smart_obj);

            if (cDevOptions::ShowSoldierData.Is_True()) {
					SoldierGameObj * p_soldier = p_smart_obj->As_SoldierGameObj();
					if (p_soldier != NULL) {
						Show_Description(p_soldier, 1.5f, 20);
					}
				}

            if (cDevOptions::ShowVehicleData.Is_True()) {
					VehicleGameObj * p_vehicle = p_smart_obj->As_VehicleGameObj();
					if (p_vehicle != NULL) {
						Show_Description(p_vehicle, 1.5f, 20);
					}
				}
			}

         if (cDevOptions::ShowPowerupData.Is_True()) {
				if (p_phys_obj->As_SimpleGameObj() != NULL) {
					PowerUpGameObj * p_powerup = p_phys_obj->As_SimpleGameObj()->As_PowerUpGameObj();
					if (p_powerup != NULL) {
						Show_Description(p_powerup, 0, 20);
					}
				}
			}
		}

      if (cDevOptions::ShowBuildingData.Is_True()) {
			if (objnode->Data()->As_ScriptableGameObj() != NULL) {
				BuildingGameObj * p_building = objnode->Data()->As_ScriptableGameObj()->As_BuildingGameObj();
				if (p_building != NULL) {
					Show_Description(p_building, 0, 100);
				}
			}
		}
	}

	Show_Spawner_Data();

	Show_Client_Rhost_Data();

//#endif // WWDEBUG

	//BottomTextYPos = Render2DClass::Get_Screen_Resolution().Bottom - 15;

//#ifdef WWDEBUG
   cNetwork::Set_Graphing_Y(64);
	//cNetwork::Watch_Bandwidth(PTextRenderer);
	cNetwork::Watch_Latency(PTextRenderer);
	cNetwork::Watch_Last_Contact(PTextRenderer);
	cNetwork::Watch_Packets(PTextRenderer);
	cNetwork::Watch_Size_Lists(PTextRenderer);
	cNetwork::Watch_Time_Lists(PTextRenderer);
	cNetwork::Watch_Packet_Size_Lists(PTextRenderer);
	cNetwork::Simulation_Warnings(PTextRenderer);

	cBandwidthGraph::Think();
#endif
}

//-----------------------------------------------------------------------------
void MultiHUDClass::Next_Playerlist_Format(void)
{
	switch (PlayerlistFormat) {
		case PLAYERLIST_FORMAT_TINY:
			PlayerlistFormat = PLAYERLIST_FORMAT_MEDIUM;
			break;

		case PLAYERLIST_FORMAT_MEDIUM:
			PlayerlistFormat = PLAYERLIST_FORMAT_FULL;
			break;

		case PLAYERLIST_FORMAT_FULL:
			PlayerlistFormat = PLAYERLIST_FORMAT_TINY;
			break;

		default:
			DIE;
			break;
	}
}