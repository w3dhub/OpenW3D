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
 *                     $Archive:: /Commando/Code/Commando/messages.cpp                      $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/22/02 7:15p                                               $*
 *                                                                                             *
 *                    $Revision:: 229                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "cnetwork.h"

#include <stdio.h>

#include "playermanager.h"
#include "gameobjmanager.h"
#include "textdisplay.h"
#include "input.h"
#include "combatgmode.h"
#include "WWAudio.h"
#include "systimer.h"
#include "multihud.h"
#include "useroptions.h"
#include "devoptions.h"
#include "god.h"
#include "playertype.h"
#include "translatedb.h"
#include "string_ids.h"
#include "wolgmode.h"
#include "bitpackids.h"
#include "msgstatlistgroup.h"
#include "vistable.h"
#include "pscene.h"
#include "staticanimphys.h"
#include "combat.h"
#include "gametype.h"
#include "colors.h"
#include "networkobject.h"
#include "building.h"
#include "vendor.h"
#include "networkobjectfactorymgr.h"
#include "networkobjectfactory.h"
#include "networkobjectmgr.h"
#include "cstextobj.h"
#include "loadingevent.h"
#include "clientcontrol.h"
#include "wwprofile.h"
#include "changeteamevent.h"
#include "DlgMPTeamSelect.h"
#include "DlgMessageBox.h"
#include "apppacketstats.h"
#include "clientpingmanager.h"
#include "priority.h"
#include "crandom.h"
#include "wwmath.h"
#include "clienthintmanager.h"
#include "packetmgr.h"
#include "specialbuilds.h"
#include "gameinitmgr.h"
#include "dlgcncwinscreen.h"
#include "ConsoleMode.h"
#include "CDKeyAuth.h"

static int LastSortedSecond;

static const float max_update_rate = 140.0f;						// Priority 1 update rate
static const float min_update_rate = 5000.0f;					// Priority 0.001 update rate
static const float unseen_update_rate = 10000.0f;				// Priority 0 update rate.
static const unsigned short infinity_update_rate = 0xffff;	// Lowest update rate - no updates at all.



//-----------------------------------------------------------------------------
//
// This is the most crucial place for server filtering
//
void cNetwork::Tell_Client_About_Dynamic_Objects
(
	int			client_id,
	Vector3 &	dest_pos
)
{
#ifndef BETACLIENT


if (cDevOptions::UseNewTCADO.Is_False()) {

	WWPROFILE("TCADO");

	WWASSERT(client_id >= 0);
   WWASSERT(cNetwork::I_Am_Server());
	if (Get_Server_Rhost(client_id) == NULL) {
		return;
	}

	if (cNetwork::I_Am_Client() && client_id == cNetwork::Get_My_Id())
	{
		//
		// Server does not send to his own client.
		//
		return;
	}

	int i;
	VisTableClass *pvs = NULL;
	int count = 0;

	{
 		WWPROFILE("GetVis");
		pvs = COMBAT_SCENE->Get_Vis_Table (dest_pos);
		count = NetworkObjectMgrClass::Get_Object_Count();
	}

	/*
	** List of objects requiring frequent updates.
	*/
	static DynamicVectorClass<NetworkObjectClass *> object_list;

	/*
	** List of objects requiring guaranteed updates. We can't schedule these.
	*/
	static DynamicVectorClass<NetworkObjectClass *> g_object_list;

	object_list.Clear();
	g_object_list.Clear();

	SoldierGameObj * player_ptr = GameObjManager::Find_Soldier_Of_Client_ID(client_id);

	{
		WWPROFILE("ListBuild");
		/*
		** Go through the object list once and figure out all the priorities.
		*/
		for (int index = 0; index < count; index ++) {

			NetworkObjectClass * p_object = NetworkObjectMgrClass::Get_Object(index);

			if (p_object == NULL) {
				continue;
			}

			float priority = 0.0f;


			if (p_object->Get_App_Packet_Type() == APPPACKETTYPE_SERVERFPS) {
				priority = 0.05f;
				p_object->Set_Cached_Priority(priority);
				object_list.Add(p_object);
			} else {

				/*
				** Get the base priority. This is just distance from where the server thinks the client is.
				*/
				priority = cPriority::Compute_Object_Priority(client_id, dest_pos, p_object);
				p_object->Set_Cached_Priority(priority);

				/*
				** If we can't see it at all, ignore it unless the priority is really high. A really high priority might indicate
				** an object update request from the client.
				*/
				if (priority < 1.0f) {
					int vis_id = p_object->Get_Vis_ID ();
					if (pvs && vis_id != -1 && !pvs->Get_Bit(vis_id)) {
						priority = 0.0f;
					}
				}


				if (p_object->Get_Object_Dirty_Bit(client_id, NetworkObjectClass::BIT_CREATION) ||
					p_object->Get_Object_Dirty_Bit(client_id, NetworkObjectClass::BIT_RARE) ||
					p_object->Get_Object_Dirty_Bit(client_id, NetworkObjectClass::BIT_OCCASIONAL)) {
					g_object_list.Add(p_object);
				} else {

					if (p_object == player_ptr) {
						priority = 0.8f;
					}
					p_object->Set_Cached_Priority(priority);

					if (priority > 0.001f) {

						/*
						** Work out the export size if we don't know it already.
						*/
						if (p_object->Get_Frequent_Update_Export_Size() == 0) {
							cPacket packet;
							int bits_before = packet.Get_Bit_Write_Position();
							packet.Add(p_object->Get_Network_ID());
							packet.Add(p_object->Get_Object_Dirty_Bits(client_id));
							packet.Add(p_object->Is_Delete_Pending());
							int bits_now = packet.Get_Bit_Write_Position();
							p_object->Export_Frequent (packet);
							int bits_after = packet.Get_Bit_Write_Position();
							int packet_size = 0;
							if (bits_now < bits_after) {
								packet_size = (bits_after - bits_before) / 8;

								/*
								** Add the packet header.
								*/
								packet_size += cPacket::Get_Packet_Header_Size();
								p_object->Set_Frequent_Update_Export_Size(packet_size);
							} else {
								/*
								** For some reason, some objects have a frequent bit set but there is no frequent update export.
								*/
								p_object->Set_Frequent_Update_Export_Size(0);
							}
						}
						object_list.Add(p_object);
					}
				}
			}
		}
	}

	/*
	** Now we have two lists. One list of non-guaranteed objects that we would like to send and another list of guaranteed
	** objects we 'must' send.
	*/

	/*
	** Sort the non-guaranteed list.
	** Bubble sort for now, quick sort later.
	*/
	NetworkObjectClass *temp_obj;

	/*
	** Get our budget in bits per second for this guy.
	*/
	cRemoteHost *r_host = cNetwork::Get_Server_Rhost(client_id);
	int bits_per_second = r_host->Get_Target_Bps();

	/*
	** Convert to bytes.
	*/
	int bytes_per_second = bits_per_second >> 3;

	/*
	** We have NetUpdateRate updates per second.
	*/
	int net_update_rate = cUserOptions::NetUpdateRate.Get();

	/*
	** Work out how many bytes we can send per update.
	*/
	int avail_bytes_per_update = bytes_per_second / net_update_rate;

	/*
	** Factor in the bandwidth multiplier.
	*/
	float mult = Get_Server_Rhost(client_id)->Get_Bandwidth_Multiplier();
	if (r_host->Get_Flood()) {
		if (r_host->Get_Target_Bps() < 14400) {
			mult = 0.7f;
		} else {
			mult = 1.0f;
		}
	}
	avail_bytes_per_update = (int) (mult * (float)avail_bytes_per_update);

	unsigned int time = TIMEGETTIME();

	/*
	** Don't use more than 50% of the available bytes per update for guaranteed packets.
	*/
	unsigned int bytes_out = 0;
	unsigned int max_bytes = avail_bytes_per_update;	//(avail_bytes_per_update >> 1);	//* 3) / 4;

	/*
	** Figure a compression ratio of 2:1
	*/
	max_bytes += bytes_out << 1;

	{
		WWPROFILE("SendG");
		/*
		** Send the guaranteed stuff first. This has to be done anyway so let's see how much of it there is.
		*/
		bool full = false;
		for (i=0 ; i<g_object_list.Count() ; i++) {
			temp_obj = g_object_list[i];
			if (!full || temp_obj->Get_App_Packet_Type() == APPPACKETTYPE_CLIENTBBOEVENT) {
				bytes_out += (Send_Object_Update(temp_obj, client_id) >> 3);
				temp_obj->Set_Last_Update_Time(client_id, time);
			}

			/*
			** See if we are using too much bandwidth and don't send more if we are.
			*/
			if (!full && bytes_out > max_bytes) {
	#ifdef WWDEBUG
				if (cDevOptions::ExtraNetDebug.Is_True()) {
					WWDEBUG_SAY(("*** WARNING: Tell_Client_About_Dynamic_Objects - Insufficient bandwidth to send all guaranteed packets ***\n"));
					WWDEBUG_SAY(("*** After %d objects, bytes_out = %d, max_bytes = %d\n", i, bytes_out, max_bytes));
				}
	#endif //WWDEBUG
				avail_bytes_per_update >>= 1;
				full = true;
				//break;
			}
		}
	}

	/*
	** Work out the average priority. We will need this later when balancing bandwidth budgets between clients.
	*/
	float average_priority = 0.0f;
	int num_priorities = 0;

	{
		WWPROFILE("CalcRate");
		for (i=0 ; i<object_list.Count() ; i++) {
			temp_obj = object_list[i];
			if (temp_obj->Get_App_Packet_Type() != APPPACKETTYPE_SERVERFPS) {
				float pri = temp_obj->Get_Cached_Priority();
				/*
				** Don't count unseen objects or objects we are getting hints for.
				*/
				if (pri > 0.001f && pri < 1.0f) {
					average_priority += pri;
					num_priorities++;
				}
			}
		}

		if (num_priorities) {
			average_priority = average_priority / (float)num_priorities;
			Get_Server_Rhost(client_id)->Set_Average_Priority(average_priority);

			/*
			** Remaining objects get updated based on priority.
			** Just set the rate based on priority then we will go back in a second pass and adjust it according to remaining bandwidth.
			*/
			float ms_low = max_update_rate;
			float ms_high = min_update_rate;
			float spread = ms_high - ms_low;
			int total_bps = 0;

			for (i=0 ; i<object_list.Count() ; i++) {
				temp_obj = object_list[i];
				float pri = temp_obj->Get_Cached_Priority();
				unsigned int update_rate = infinity_update_rate;	//0;
				if (pri > 0.025f) {	//01f) {
					update_rate = (unsigned int)(((1.0f - pri) * spread) + ms_low);
				}
				temp_obj->Set_Update_Rate(client_id, (unsigned short) update_rate);
				if (update_rate != infinity_update_rate) {
					int bps = (1000.0f / update_rate) * temp_obj->Get_Frequent_Update_Export_Size();
					total_bps += bps;
				}
			}

			/*
			** Make that number a per update figure.
			*/
			total_bps = total_bps / net_update_rate;

			/*
			** Now, scale the update rate based on available bandwidth.
			*/
			float factor = 1.0;
			if (total_bps) {
				factor = avail_bytes_per_update / total_bps;
				if (factor < 0.00001f) {
					factor = 0.00001f;
				}
			}

			unsigned short rate;
			float float_rate;
			for (i=0 ; i<object_list.Count() ; i++) {
				temp_obj = object_list[i];
				float obj_upd_rate = (float)temp_obj->Get_Update_Rate(client_id);
				if (obj_upd_rate != infinity_update_rate) {
					float_rate = obj_upd_rate / factor;
					rate = (unsigned short) float_rate;
					temp_obj->Set_Update_Rate(client_id, rate);
				}
			}
		} else {
			Get_Server_Rhost(client_id)->Set_Average_Priority(0.0f);
		}
	}

	{
		WWPROFILE("SendN");
		/*
		** Send those packets whos time has come.
		*/
		for (i=0 ; i<object_list.Count() ; i++) {
			temp_obj = object_list[i];
			unsigned int rate =  (unsigned int)temp_obj->Get_Update_Rate(client_id);
			if (rate != (unsigned int)infinity_update_rate) {
				if (time - temp_obj->Get_Last_Update_Time(client_id) > rate) {
					Send_Object_Update(temp_obj, client_id);
					temp_obj->Set_Last_Update_Time(client_id, time);
				}
			}
		}
	}

	REF_PTR_RELEASE(pvs);

} else {











	/*
	**
	**    Optimized version of TCADO
	**
	**
	**
	**
	*/


	const unsigned char dirty_check = (NetworkObjectClass::BIT_FREQUENT ^ 0xffffffff) & (NetworkObjectClass::BIT_CREATION | NetworkObjectClass::BIT_RARE | NetworkObjectClass::BIT_OCCASIONAL);

	WWPROFILE("TCADO");

	WWASSERT(client_id >= 0);
   WWASSERT(cNetwork::I_Am_Server());
	if (Get_Server_Rhost(client_id) == NULL) {
		return;
	}

	if (cNetwork::I_Am_Client() && client_id == cNetwork::Get_My_Id())
	{
		//
		// Server does not send to his own client.
		//
		return;
	}

	cRemoteHost *r_host = cNetwork::Get_Server_Rhost(client_id);

	bool update_priorities = (r_host->Get_Priority_Update_Counter() == 0) ? true : false;
	r_host->Increment_Priority_Count();

	int i;
	VisTableClass *pvs = NULL;
	int count = 0;
	bool global_packet_allowance_full = false;
	NetworkObjectClass *temp_obj;
	unsigned int time = TIMEGETTIME();
	int global_count = 0;

	/*
	** Get our budget in bits per second for this guy.
	*/
	int bits_per_second = r_host->Get_Target_Bps();

	/*
	** Adjust the cut off threshold for vis hidden objects further out if more bandwidth is available.
	*/
	float min_vis_distance = 15.0f;
	if (bits_per_second > 60000) {
		min_vis_distance = 50.0f;
	}

	/*
	** Convert to bytes.
	*/
	int bytes_per_second = bits_per_second >> 3;

	/*
	** We have NetUpdateRate updates per second.
	*/
	int net_update_rate = cUserOptions::NetUpdateRate.Get();

	/*
	** Work out how many bytes we can send per update.
	*/
	int avail_bytes_per_update = bytes_per_second / net_update_rate;

	/*
	** Don't use more than 50% of the available bytes per update for guaranteed packets.
	*/
	unsigned int bytes_out = 0;
	unsigned int max_bytes = avail_bytes_per_update;	//(avail_bytes_per_update >> 1);	//* 3) / 4;

	/*
	** Figure a compression ratio of 2:1 for guaranteed packets.
	*/
	//max_bytes <<= 1;

	{
		WWPROFILE("GetVis");
		if (update_priorities) {
			pvs = COMBAT_SCENE->Get_Vis_Table(dest_pos);
		}
	}
	count = NetworkObjectMgrClass::Get_Object_Count();

	/*
	** List of objects requiring frequent updates.
	*/
	static DynamicVectorClass<NetworkObjectClass *> object_list(500);
	object_list.Reset_Active();
	object_list.Set_Growth_Step(100);

	SoldierGameObj * player_ptr = GameObjManager::Find_Soldier_Of_Client_ID(client_id);

	{
		WWPROFILE("ListBuild");
		/*
		** Go through the object list once and figure out all the priorities.
		*/
		for (int index = 0; index < count; index ++) {

			NetworkObjectClass * p_object = NetworkObjectMgrClass::Get_Object(index);

			if (p_object == NULL) {
				continue;
			}

			float priority = 0.0f;

			/*
			** SERVERFPS events are low priority but must have some kind of priority.
			*/
			if (p_object->Get_App_Packet_Type() == APPPACKETTYPE_SERVERFPS) {
				priority = 0.05f;
				p_object->Set_Cached_Priority_2(client_id, priority);
				object_list.Add(p_object);
			} else {

				unsigned char dirty = p_object->Get_Object_Dirty_Bits(client_id);

				if (dirty & dirty_check) {
					/*
					** This is a guaranteed packet update. Just send it right away.
					*/
					if (!global_packet_allowance_full || p_object->Get_App_Packet_Type() == APPPACKETTYPE_CLIENTBBOEVENT) {
						bytes_out += (Send_Object_Update(p_object, client_id) >> 3);
						p_object->Set_Last_Update_Time(client_id, time);
					}
					global_count++;

					/*
					** See if we are using too much bandwidth and don't send any more guaranteed packets if we are.
					*/
					if (!global_packet_allowance_full && bytes_out > max_bytes) {
#ifdef WWDEBUG
						if (cDevOptions::ExtraNetDebug.Is_True()) {
							WWDEBUG_SAY(("*** WARNING: Tell_Client_About_Dynamic_Objects - Insufficient bandwidth to send all guaranteed packets ***\n"));
							WWDEBUG_SAY(("*** After %d objects, bytes_out = %d, max_bytes = %d\n", global_count, bytes_out, max_bytes));
						}
#endif //WWDEBUG
						global_packet_allowance_full = true;
					}


				} else {

					/*
					** Get the base priority. This is how important we think this object is to this client.
					*/
					if ((dirty & NetworkObjectClass::BIT_FREQUENT) != 0) {
						priority = p_object->Get_Cached_Priority_2(client_id);

						if (p_object == player_ptr) {
							if (player_ptr->Is_In_Vehicle()) {
								priority = 0.1f;
							} else {
								priority = 0.8f;
							}
						} else {

							if (p_object->Get_Client_Hint_Count(client_id) > 0) {
								priority = 1.0f;
								p_object->Reset_Client_Hint_Count(client_id);
							} else {

								/*
								** If we can't see it at all, ignore it unless the priority is really high. A really high priority might indicate
								** an object update request from the client.
								*/
								/*
								** Even if the client can't see the object, it's probably a good idea to update it if it's really
								** close to the client. Say 15 meters.
								*/
								if (update_priorities) {
									int vis_id = p_object->Get_Vis_ID();
									bool hidden = false;
									if (pvs && vis_id != -1 && !pvs->Get_Bit(vis_id)) {
										hidden = true;
									}
									if (hidden) {
										int distance = cPriority::Get_Object_Distance_2(dest_pos, p_object);
										if (distance > min_vis_distance) {
											/*
											** Allow some very infrequent updates for distant, hidden objects on broadband connections.
											*/
											if (bits_per_second > 100000 && distance < 150.0f) {
												priority = 0.01f;
											} else {
												priority = 0.0f;
											}
										} else {
											/*
											** It's hidden, but close (maybe as close at 15 meters), so it's probably somewhat important to us.
											*/
											priority = 0.2f;
										}
									} else {

										/*
										** Figure out the priority if it's time or if we couldn't see the object last frame but we can now.
										*/
										//if (update_priorities || priority == 0.0f) {
											priority = cPriority::Compute_Object_Priority_2(client_id, dest_pos, p_object, false, player_ptr);
											p_object->Set_Cached_Priority_2(client_id, priority);
										//}
									}
								}
							}
						}

						p_object->Set_Cached_Priority_2(client_id, priority);

						if (priority > 0.001f) {

							/*
							** Work out the export size if we don't know it already.
							*/
							if (p_object->Get_Frequent_Update_Export_Size() == 0) {
								cPacket packet;
								int bits_before = packet.Get_Bit_Write_Position();
								packet.Add(p_object->Get_Network_ID());
								packet.Add(p_object->Get_Object_Dirty_Bits_2(client_id));
								packet.Add(p_object->Is_Delete_Pending());
								int bits_now = packet.Get_Bit_Write_Position();
								p_object->Export_Frequent (packet);
								int bits_after = packet.Get_Bit_Write_Position();
								int packet_size = 0;
								if (bits_now < bits_after) {
									packet_size = (bits_after - bits_before) / 8;

									/*
									** Add the packet header.
									*/
									packet_size += cPacket::Get_Packet_Header_Size();
									p_object->Set_Frequent_Update_Export_Size(packet_size);
								} else {
									/*
									** For some reason, some objects have a frequent bit set but there is no frequent update export.
									*/
									p_object->Set_Frequent_Update_Export_Size(0xff);
								}
							}

							/*
							** Add this object to our update list.
							*/
							if (p_object->Get_Frequent_Update_Export_Size() > 0 && p_object->Get_Frequent_Update_Export_Size() < 0xff) {
								object_list.Add(p_object);
							}
						}
					}
				}
			}
		}
	}



	/*
	** Now we have a list of non-guaranteed objects that we would like to send.
	*/


	/*
	** Factor in the bandwidth multiplier.
	*/
	float mult = Get_Server_Rhost(client_id)->Get_Bandwidth_Multiplier();
	if (r_host->Get_Flood()) {
		if (r_host->Get_Target_Bps() < 14400) {
			mult = 0.7f;
		} else {
			mult = 1.0f;
		}
	}
	avail_bytes_per_update = (int) (mult * (float)avail_bytes_per_update);


	/*
	** If we used tons of bandwidth on guaranteed packets then throttle back on non-guaranteed stuff.
	*/
	if (global_packet_allowance_full) {
		avail_bytes_per_update >>= 1;
	}

	/*
	** Work out the average priority. We will need this later when balancing bandwidth budgets between clients.
	*/
	float average_priority = 0.0f;
	int num_priorities = 0;

	{
		WWPROFILE("CalcRate");
		for (i=0 ; i<object_list.Count() ; i++) {
			temp_obj = object_list[i];
			if (temp_obj->Get_App_Packet_Type() != APPPACKETTYPE_SERVERFPS) {
				float pri = temp_obj->Get_Cached_Priority_2(client_id);
				/*
				** Don't count unseen objects or objects we are getting hints for.
				*/
				if (pri > 0.001f && pri < 1.0f) {
					average_priority += pri;
					num_priorities++;
				}
			}
		}

		if (num_priorities) {
			average_priority = average_priority / (float)num_priorities;
			Get_Server_Rhost(client_id)->Set_Average_Priority(average_priority);

			/*
			** Remaining objects get updated based on priority.
			** Just set the rate based on priority then we will go back in a second pass and adjust it according to remaining bandwidth.
			*/
			float ms_low = max_update_rate;
			float ms_high = min_update_rate;
			float spread = ms_high - ms_low;
			int total_bps = 0;

			for (i=0 ; i<object_list.Count() ; i++) {
				temp_obj = object_list[i];
				float pri = temp_obj->Get_Cached_Priority_2(client_id);
				unsigned int update_rate = infinity_update_rate;	//0;
				if (pri > 0.025f) {	//01f) {
					update_rate = (unsigned int)(((1.0f - pri) * spread) + ms_low);
				} else {
					if (pri > 0.009f) {
						update_rate = min_update_rate;
					}
				}
				temp_obj->Set_Update_Rate(client_id, (unsigned short) update_rate);
				if (update_rate != infinity_update_rate) {
					int bps = (1000.0f / update_rate) * temp_obj->Get_Frequent_Update_Export_Size();
					total_bps += bps;
				}
			}

			/*
			** Make that number a per update figure.
			*/
			total_bps = total_bps / net_update_rate;

			/*
			** Now, scale the update rate based on available bandwidth.
			*/
			float factor = 1.0;
			if (total_bps) {
				factor = avail_bytes_per_update / total_bps;
				if (factor < 0.00001f) {
					factor = 0.00001f;
				}
			}

			unsigned short rate;
			float float_rate;
			for (i=0 ; i<object_list.Count() ; i++) {
				temp_obj = object_list[i];
				float obj_upd_rate = (float)temp_obj->Get_Update_Rate(client_id);
				if (obj_upd_rate != infinity_update_rate && obj_upd_rate < (min_update_rate + WWMATH_EPSILON)) {
					float_rate = obj_upd_rate / factor;
					rate = (unsigned short) float_rate;
					temp_obj->Set_Update_Rate(client_id, rate);
				}
			}
		} else {
			Get_Server_Rhost(client_id)->Set_Average_Priority(0.0f);
		}
	}

	{
		WWPROFILE("SendN");
		/*
		** Send those packets whos time has come.
		*/
		for (i=0 ; i<object_list.Count() ; i++) {
			temp_obj = object_list[i];
			unsigned int rate =  (unsigned int)temp_obj->Get_Update_Rate(client_id);
			if (rate != (unsigned int)infinity_update_rate) {
				if (time - temp_obj->Get_Last_Update_Time(client_id) > rate) {
					Send_Object_Update(temp_obj, client_id);
					temp_obj->Set_Last_Update_Time(client_id, time);
				}
			}
		}
	}

	if (pvs) {
		REF_PTR_RELEASE(pvs);
	}


}

#endif // not BETACLIENT
}

//-----------------------------------------------------------------------------
void cNetwork::Tell_Server_About_Dynamic_Objects
(
	void
)
{
#ifndef FREEDEDICATEDSERVER

   WWASSERT (cNetwork::I_Am_Client());

	//
	//	Loop over each network object
	//
	int count = NetworkObjectMgrClass::Get_Object_Count();

	//int debug_count = 0;

	for (int index = 0; index < count; index ++) {

		NetworkObjectClass * object = NetworkObjectMgrClass::Get_Object(index);

		//
		//	Should we send update information for this object?
		//
		if (object != NULL && object->Is_Client_Dirty(0))
		{
			//
			//	Transmit the object's data to the server
			//
			Send_Object_Update(object, 0);
			//debug_count++;
		}
	}
	//unsigned int time = TIMEGETTIME() / 1000;
	//WWDEBUG_SAY(("Updated %d objects at %d\n", debug_count, time));

#endif // !FREEDEDICATEDSERVER
}

//-----------------------------------------------------------------------------
void cNetwork::Tell_Client_About_Delete_Notifications(int client_id)
{
#ifndef BETACLIENT

	WWASSERT (client_id >= 0);
   WWASSERT (cNetwork::I_Am_Server ());

	if (Get_Server_Rhost (client_id) == NULL) {
		return;
	}

	//
	//	Loop over each network object
	//
	int count = NetworkObjectMgrClass::Get_Object_Count ();
	for (int index = 0; index < count; index ++) {
		NetworkObjectClass *object = NetworkObjectMgrClass::Get_Object (index);

		//
		//	Is this object going to be deleted soon?
		//
		if (object != NULL && object->Is_Delete_Pending ()) {

			//
			//	Does the local client need this information?
			//
			bool send = true;
			if (cNetwork::I_Am_Client() &&
				 client_id == cNetwork::Get_My_Id ())
			{
				send = false;
			}

			if (send)
			{
			   //
				//	Transmit the object's data to the client
				//
				Send_Object_Update (object, client_id);
			}
		}
	}

	return ;

#endif // not BETACLIENT
}

//-----------------------------------------------------------------------------
int
cNetwork::Send_Object_Update(NetworkObjectClass *object, int client_id)
{
	WWPROFILE( "send obj upd" );
	WWASSERT(client_id >= 0);

   Debug_Network_Prolific((
      "Sending update for object %d to %s",
      object->Get_Network_ID (), Get_Client_String(client_id)));

	WWASSERT(object->Get_Network_ID() > 0);

	// Track number of bits sent.
	int bits_sent = 0;

	//
	//	Build a packet that will contain enough information about
	// the object so the client will be able to import the data
	//
	cPacket packet;
	packet.Add(object->Get_Network_ID());
	packet.Add(object->Get_Object_Dirty_Bits(client_id));
	packet.Add(object->Is_Delete_Pending());
	//packet.Add(object->Get_App_Packet_Type());

	BYTE type = object->Get_App_Packet_Type();
	int bits_start = packet.Get_Bit_Write_Position();

	//
	// Send mode is unreliable unless creation/rare/occasional data is present.
	//
	int mode = SEND_UNRELIABLE;

	//
	// TSS101101
	//
	if (object->Is_Delete_Pending()) {
		mode = SEND_RELIABLE;
	}

	//
	//	Add creation information to the packet (if necessary)
	//
	if (object->Get_Object_Dirty_Bit (client_id, NetworkObjectClass::BIT_CREATION)) {

		int bits_before = packet.Get_Bit_Write_Position();

		//
		//	Add the class id of the object so it can be created on the client.
		//
		uint32 net_classid = object->Get_Network_Class_ID ();
		packet.Add (net_classid);

		//
		//	Lookup the factory for this object
		//
		NetworkObjectFactoryClass *factory = NetworkObjectFactoryMgrClass::Find_Factory (net_classid);
		WWASSERT (factory != NULL);

		//
		//	Store any data in the packet that's needed in order
		// to create the object on the client
		//
		factory->Prep_Packet (object, packet);

		//
		//	Send object-specific data to the client
		//
		object->Export_Creation (packet);

		int bits_after = packet.Get_Bit_Write_Position();
		cAppPacketStats::Increment_Bits_Sent_Tier(type, PACKET_TIER_CREATION, bits_after - bits_before);

		mode = SEND_RELIABLE;
	}

	//
	//	Add rare information to the packet (if necessary)
	//
	if (object->Get_Object_Dirty_Bit (client_id, NetworkObjectClass::BIT_RARE)) {
		int bits_before = packet.Get_Bit_Write_Position();
		object->Export_Rare (packet);
		int bits_after = packet.Get_Bit_Write_Position();
		cAppPacketStats::Increment_Bits_Sent_Tier(type, PACKET_TIER_RARE, bits_after - bits_before);
		mode = SEND_RELIABLE;
	}

	//
	//	Add occasional information to the packet (if necessary)
	//
	if (object->Get_Object_Dirty_Bit (client_id, NetworkObjectClass::BIT_OCCASIONAL)) {
		int bits_before = packet.Get_Bit_Write_Position();
		object->Export_Occasional (packet);
		int bits_after = packet.Get_Bit_Write_Position();
		cAppPacketStats::Increment_Bits_Sent_Tier(type, PACKET_TIER_OCCASIONAL, bits_after - bits_before);
		mode = SEND_RELIABLE;
	}

	//
	//	Add frequent information to the packet (if necessary)
	//
	if (object->Get_Object_Dirty_Bit (client_id, NetworkObjectClass::BIT_FREQUENT)) {
		int bits_before = packet.Get_Bit_Write_Position();
		object->Export_Frequent (packet);
		int bits_after = packet.Get_Bit_Write_Position();
		cAppPacketStats::Increment_Bits_Sent_Tier(type, PACKET_TIER_FREQUENT, bits_after - bits_before);
	}

	int bits_end = packet.Get_Bit_Write_Position();

	if (mode == SEND_RELIABLE && object->Get_Unreliable_Override() == true) {
		//
		// The normal rules would make us send this packet reliably, but this
		// has been explicitly overridden.
		//
		mode = SEND_UNRELIABLE;
	}

	//
	// Trying just not sending the packet if there is nothing in it.
	//
	if (object->Is_Delete_Pending() || (bits_end > bits_start)) {

		bits_sent = packet.Get_Bit_Write_Position();

		//
		//	Send the packet to the client or server
		//
		if (client_id > 0) {
			//WWDEBUG_SAY(("Sending update for object %d\n", object->Get_Network_ID()));
			Server_Send_Packet(packet, mode, client_id);
		} else {
			Client_Send_Packet(packet, mode);
		}
	}

#ifdef WWDEBUG
	if (client_id > 0) {
		//
		// It is useful to be able to generate a bunch of
		// extra bandwidth
		//
		for (int i = 0; i < cDevOptions::SpamCount.Get (); i ++) {
			WWDEBUG_SAY(("Sending spam\n"));
			Server_Send_Packet(packet, mode, client_id);
		}
	}
#endif // WWDEBUG

	//
	//	Reset the dirty bits for this client
	//
	object->Set_Object_Dirty_Bit (client_id, NetworkObjectClass::BIT_CREATION, false);
	object->Set_Object_Dirty_Bit (client_id, NetworkObjectClass::BIT_RARE, false);
	object->Set_Object_Dirty_Bit (client_id, NetworkObjectClass::BIT_OCCASIONAL, false);

	//if (client_id > 0)
	{
		//
		// This is a server send. Record stats.
		//

		/*
		//if (type == APPPACKETTYPE_UNKNOWN)
		if (type == APPPACKETTYPE_SIMPLE)
		{
			WWDEBUG_SAY(("WTF is this?\n"));
		}
		/**/

		//
		// Record stats
		//
		cAppPacketStats::Increment_Packets_Sent(type);
		cAppPacketStats::Increment_Bits_Sent(type, bits_end - bits_start);
	}
	return(bits_sent);
}


//-----------------------------------------------------------------------------
void cNetwork::Intermission_Over_Processing(void)
{
	WWDEBUG_SAY(("cNetwork::Intermission_Over_Processing\n"));

	WWASSERT(cNetwork::I_Am_Server());

	//
	//	Terminate the win screen dialog
	//
	CNCWinScreenMenuClass::Close_Dialog ();

   //
   // Destroy any commando's, let them be recreated afresh
   //
   for (
		SLNode<cPlayer> * objnode = cPlayerManager::Get_Player_Object_List()->Head();
		objnode; objnode =
		objnode->Next()) {

		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);
      Delete_Player_Objects(p_player->Get_Id());

		if (p_player->Is_Human()) {
			//
			// Set their ingame flag to false.
			// They will each notify us when they have finished loading.
			//
			p_player->Set_Is_In_Game(false);
			if (cNetwork::PServerConnection) {
				cNetwork::PServerConnection->Set_Rhost_Is_In_Game(p_player->Get_Id(), false);
			}

		}
   }

	if (cGameData::Is_Manual_Exit()) {
		GameInitMgrClass::Set_Needs_Game_Exit_All(true);
	} else {

		//
		// If we have to quit out now then don't do a core restart.
		//
		if (The_Game () != NULL && The_Game ()->Is_Map_Cycle_Over () == false) {
			extern bool g_b_core_restart;
			g_b_core_restart = true;
			GameModeClass *game_mode = GameModeManager::Find("WOL");
			if (game_mode && game_mode->Is_Active()) {
				WolGameModeClass* wol_game = (WolGameModeClass*) game_mode;
				if (wol_game->Post_Game_Check()) {
					g_b_core_restart = false;
				}
			}
		} else {
			GameInitMgrClass::Set_Needs_Game_Exit (true);
		}
	}

	//
	// TSS092601
	//
	WWASSERT(PTheGameData != NULL);
	The_Game()->IsIntermission.Set(false);
}

//-----------------------------------------------------------------------------
void cNetwork::End_Game_Test(void)
{
#ifndef BETACLIENT

   WWASSERT(cNetwork::I_Am_Server());

	if (IS_MISSION || !GameModeManager::Find("Combat")->Is_Active()) {
		return;
	}

	WWASSERT(PTheGameData != NULL);
	if (The_Game()->IsIntermission.Is_True()) {

      if (The_Game()->Get_Intermission_Time_Remaining() < WWMATH_EPSILON) {
         Intermission_Over_Processing();
      }

   } else {

		//
		// Sort teams and players once a second
		//
		bool is_game_over = The_Game()->Is_Game_Over();

		bool sort=is_game_over;
		int seconds=cMathUtil::Round(TimeManager::Get_Seconds());
		if (!sort) {
			if (seconds!=LastSortedSecond) {
				LastSortedSecond=seconds;
				sort=true;
			}
		}

		if (sort) {
			cTeamManager::Sort_Teams();
			cPlayerManager::Sort_Players(true);
		}

		if (is_game_over) {
			The_Game()->Game_Over_Processing();
		}
	}

#endif // not BETACLIENT
}

//-----------------------------------------------------------------------------
void cNetwork::Shared_Client_And_Server_Think(void)
{
	WWASSERT(I_Am_Client() || I_Am_Server());

	WWASSERT(PTheGameData != NULL);
	The_Game()->Think();

	Hibernation_Think();
}

//-----------------------------------------------------------------------------
bool cNetwork::Client_Think(void)
{
	bool ret_code = false;

#ifndef FREEDEDICATEDSERVER

	if (PClientConnection->Is_Destroy()) {
		Cleanup_Client();
		return(ret_code);
	}

   if (!PClientConnection->Have_Id()) {
		return(ret_code);
	}

	//
	// Client reload if necessary
	//
	WWASSERT(PTheGameData != NULL);
	if (I_Am_Only_Client() &&
		 The_Game()->IsIntermission.Is_True() &&
		 The_Game()->Get_Intermission_Time_Remaining() < WWMATH_EPSILON)
	{
		//
		//	Terminate the win screen dialog
		//
		CNCWinScreenMenuClass::Close_Dialog ();

		//
		// See if we have the next map in the cycle.
		//
		bool quit = false;
		if (The_Game()->Is_Map_Cycle_Over()) {
			quit = true;
		} else {
			if (!The_Game()->Is_Map_Valid()) {
				quit = true;
			}
		}

		if (quit) {

			//
			//	If the map looping is over, then quit the game
			//
			GameInitMgrClass::Set_Needs_Game_Exit (true);

		} else {
			extern bool g_b_core_restart;
			g_b_core_restart = true;
		}
	}

	cClientPingManager::Think();

	cClientHintManager::Think();

	WWASSERT(Receiver!= NULL);
	ret_code = Receiver->Client_Update_Dynamic_Objects();

	//
	// Pop up the team change dialog if it is appropriate and we have not already
	// done so.
	//
	if (!HaveDoneTeamChangeDialog && cChangeTeamEvent::Is_Change_Team_Possible()
			&& DlgMsgBox::Get_Current_Count() == 0) {

		HaveDoneTeamChangeDialog = true;
		DlgMPTeamSelect::DoDialog(*The_Game());
	}

	//
	// Pop up the MOTD dialog if appropriate.
	//
	if (!HaveDoneMotdDialog) {
		if (I_Am_Only_Client()) {
			if ((DlgMsgBox::Get_Current_Count() == 0)
					&& GameModeManager::Find("Combat") != NULL && GameModeManager::Find("Combat")->Is_Active()
					&& The_Game() != NULL && ::u_strlen(The_Game()->Get_Motd()) > 0) {

				DlgMsgBox::DoDialog(TRANSLATE (IDS_MENU_MOTD), The_Game()->Get_Motd());
				HaveDoneMotdDialog = true;
			}
		} else {
			HaveDoneMotdDialog = true;
		}
	}

#endif // !FREEDEDICATEDSERVER

	return(ret_code);
}

//-----------------------------------------------------------------------------
VisTableClass * cNetwork::Peek_Temp_Vis_Table(void)
{
	bool alloc_new_table = false;

	if (VisTable == NULL) {
		alloc_new_table = true;
	} else if (VisTable->Get_Bit_Count() != COMBAT_SCENE->Get_Vis_Table_Size()) {
		alloc_new_table = true;
	}

	if (alloc_new_table) {
		REF_PTR_RELEASE(VisTable);
		VisTable = new VisTableClass(COMBAT_SCENE->Get_Vis_Table_Size(), 0);
	}

	WWASSERT(VisTable != NULL);
	VisTable->Reset_All();
	return VisTable;
}

//-----------------------------------------------------------------------------
void cNetwork::Hibernation_Think(void)
{
	if (COMBAT_SCENE != NULL) {

		//
		// The server can't let any player go into hibernation or updates will cease to be sent for that player.
		//
		if (I_Am_Server() && !IS_MISSION) {
			for (
				SLNode<BaseGameObj> * p_objnode = GameObjManager::Get_Game_Obj_List()->Head();
				p_objnode;
				p_objnode = p_objnode->Next()) {

				PhysicalGameObj * p_phys_go = p_objnode->Data()->As_PhysicalGameObj();

				if (p_phys_go != NULL) {
					p_phys_go->Reset_Hibernating();
				}
			}
		} else {


			VisTableClass * p_vis_table = Peek_Temp_Vis_Table();
			WWASSERT(p_vis_table != NULL);

			//
			// Build a union of all players' PVS's
			//
			for (
				SLNode<SmartGameObj> * p_smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head();
				p_smart_objnode;
				p_smart_objnode = p_smart_objnode->Next()) {

				WWASSERT(p_smart_objnode->Data() != NULL);
				SoldierGameObj * p_soldier = p_smart_objnode->Data()->As_SoldierGameObj();

				if (p_soldier != NULL && p_soldier->Has_Player()) {

					Vector3 player_pos;
					p_soldier->Get_Position(&player_pos);
					player_pos.Z += 2; // Start near the player's head

					VisTableClass * p_player_pvs = COMBAT_SCENE->Get_Vis_Table(player_pos);

					if (p_player_pvs == NULL) {
						p_vis_table = NULL;
						break;
					} else {
						p_vis_table->Merge(*p_player_pvs);
						REF_PTR_RELEASE(p_player_pvs);
					}
				}
			}

			//
			// Reset hibernating on anything visible
			//
			Vector3	star_pos(0,0,0);
			if ( COMBAT_STAR ) {
				COMBAT_STAR->Get_Position( &star_pos );
			}

			for (
				SLNode<BaseGameObj> * p_objnode = GameObjManager::Get_Game_Obj_List()->Head();
				p_objnode;
				p_objnode = p_objnode->Next()) {

				PhysicalGameObj * p_phys_go = p_objnode->Data()->As_PhysicalGameObj();
				if (p_phys_go != NULL) {
					PhysClass * p_phys_obj = p_phys_go->Peek_Physical_Object();
					if (p_phys_obj != NULL &&
						(p_vis_table == NULL || p_vis_table->Get_Bit(p_phys_obj->Get_Vis_Object_ID()))) {

						// Only if within 300m of the star;
						#define		MIN_HIB_DISTANCE		300
						Vector3 pos;
						p_phys_go->Get_Position( &pos );
						pos -= star_pos;
						if ( pos.Length2() < MIN_HIB_DISTANCE * MIN_HIB_DISTANCE ) {
							p_phys_go->Reset_Hibernating();
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool cNetwork::Server_Think(void)
{
	WWASSERT(I_Am_Server());

	bool ret_code = false;
	{
		WWPROFILE( "God Think" );
		if (GameModeManager::Find( "Combat" )->Is_Active()) {
			cGod::Think();
		}
	}

	{
		WWPROFILE( "End_Game_Test" );
	   End_Game_Test();
	}

   //
	// Update clients about dynamic & static objects
	//
	WWASSERT(Receiver!= NULL);

	//
	// TSS121101
	// The following I_Am_Server test seems to be necessary. In v 85
	// we were getting occasional I_Am_Server asserts on the server
	// inside Server_Update_Dynamic_Objects.
	//
	if (I_Am_Server())
	{
		{
			WWPROFILE( "svrupd dyn" );
			ret_code = Receiver->Server_Update_Dynamic_Objects();
		}

		{
			WWPROFILE( "svrupd del" );
			Receiver->Server_Send_Delete_Notifications();
		}
	}

	return(ret_code);
}

//-----------------------------------------------------------------------------
void cNetwork::Server_Kill_Connection( int client_id )
{
	WWASSERT(client_id >= 0);
	WWASSERT(I_Am_Server());
   PServerConnection->Destroy_Connection( client_id );

   WWDEBUG_SAY(("cNetwork::Server_Kill_Connection for client %d\n", client_id));
}

//-----------------------------------------------------------------------------
void cNetwork::Delete_Player_Objects(int client_id)
{
	WWDEBUG_SAY(("cNetwork::Delete_Player_Objects %d\n", client_id));

   WWASSERT(I_Am_Server());

   //
   // Delete all objects controlled by this guy; inform all clients.
   //
	SLNode<BaseGameObj> * objnode;
	for (objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
		WWASSERT(objnode->Data() != NULL);
      PhysicalGameObj * p_phys_obj = objnode->Data()->As_PhysicalGameObj();
		if (p_phys_obj != NULL) {

		   SmartGameObj * p_smart_obj = p_phys_obj->As_SmartGameObj();

         if (p_smart_obj != NULL &&
            (p_smart_obj->Get_Control_Owner() == client_id))
			{
				//
				//	Destroy the object
				//
				p_smart_obj->Set_Delete_Pending ();
			}
      }
   }
}

//-----------------------------------------------------------------------------
//
// Call this after a client leaves or breaks connection.
//
//void cNetwork::Cleanup_After_Client(int client_id, bool is_departure_graceful)
void cNetwork::Cleanup_After_Client(int client_id)
{
	WWDEBUG_SAY(("cNetwork::Cleanup_After_Client %d\n", client_id));

   WWASSERT(I_Am_Server());

   Remove_Player(client_id);

   Delete_Player_Objects(client_id);

	NetworkObjectMgrClass::Delete_Client_Objects(client_id);

	//
	// We need to special-case these new delete notifications or delete_pending
	// will clear them first.
	//
	WWASSERT(Receiver!= NULL);
	Receiver->Server_Send_Delete_Notifications();

	NetworkObjectMgrClass::Restore_Dirty_Bits(client_id);

	CCDKeyAuth::DisconnectUser(client_id);
}

//-----------------------------------------------------------------------------
void cNetwork::Remove_Player(int player_id)
{
	WWDEBUG_SAY(("cNetwork::Remove_Player %d\n", player_id));

	WWASSERT(I_Am_Server());

   cPlayer * p_player = cPlayerManager::Find_Player(player_id);

	//
	// We can't remove the player structure because we wish to retain
	// it for scoring etc
	//
	if (p_player != NULL) {
		p_player->Increment_Total_Time();
		p_player->Set_Is_Active(false);

		StringClass str(128, true);
		p_player->Get_Name().Convert_To(str);
		ConsoleBox.Print_Maybe("Player %s left the game\n", str.Peek_Buffer());
	}

	if (p_player != NULL && IS_MULTIPLAY) {
		Test_For_Team_Defaulting(p_player);
	}
}

//-----------------------------------------------------------------------------
void cNetwork::Test_For_Team_Defaulting(cPlayer * p_player)
{
	//
	// If all players on the winning team bail, take punitive action:
	// reverse the team scores of Nod and GDI.
	//

	WWDEBUG_SAY(("cNetwork::Test_For_Team_Defaulting\n"));

	WWASSERT(p_player != NULL);
	WWASSERT(IS_MULTIPLAY);

	int count_nod = cPlayerManager::Tally_Team_Size(PLAYERTYPE_NOD);
	int count_gdi = cPlayerManager::Tally_Team_Size(PLAYERTYPE_GDI);

	cTeam * p_team_nod = cTeamManager::Find_Team(PLAYERTYPE_NOD);
	WWASSERT(p_team_nod != NULL);
	cTeam * p_team_gdi = cTeamManager::Find_Team(PLAYERTYPE_GDI);
	WWASSERT(p_team_gdi != NULL);

	float score_nod = p_team_nod->Get_Score();
	float score_gdi = p_team_gdi->Get_Score();

	if (p_player->Get_Player_Type() == PLAYERTYPE_NOD &&
		 count_nod == 0 &&
		 score_nod > score_gdi) {

		WWDEBUG_SAY(("Reversing Nod and GDI scores due to Nod defaulting.\n"));

		p_team_gdi->Set_Score(score_nod);
		p_team_nod->Set_Score(score_gdi);
	}
	else if (p_player->Get_Player_Type() == PLAYERTYPE_GDI &&
		 count_gdi == 0 &&
		 score_gdi > score_nod) {

		WWDEBUG_SAY(("Reversing Nod and GDI scores due to GDI defaulting.\n"));

		p_team_gdi->Set_Score(score_nod);
		p_team_nod->Set_Score(score_gdi);
	}
}