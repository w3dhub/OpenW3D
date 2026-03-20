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
 *                     $Archive:: /Commando/Code/Commando/cnetwork.h                         $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/19/02 4:49p                                               $*
 *                                                                                             *
 *                    $Revision:: 76                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef	CNETWORK_H
#define	CNETWORK_H

#include "always.h"
#include "vector3.h"
#include "w3d_file.h"
#include "teammanager.h"
#include "comnetrcvinst.h"
#include "comnetrcv.h"
#include "connect.h"
#include "nethandler.h"
#include "regbool.h"

#define MAX_TEXTMESSAGE_LENGTH 200

class cPlayer;
class cTeam;
class cPacket;
class ScriptableGameObj;
class SmartGameObj;
class ChunkSaveClass;
class ChunkLoadClass;
class cMsgStatList;
class cMsgStatListGroup;
class	Render2DTextClass;
class	VisTableClass;

//-----------------------------------------------------------------------------
class	cNetwork
{
public:
	enum {
		ALL			=		-1,	// for sending to everyone
	};

	static void Compute_Exe_Key(void);
   static void Onetime_Init(void);
   static void Onetime_Shutdown(void);
   static void	Update();

	static bool	Save(ChunkSaveClass & csave);
	static bool	Load(ChunkLoadClass & cload);

   static float Get_Distance_Priority(Vector3 & pos1, Vector3 & pos2);
	static void Update_Fps(void);
	static int Get_Fps(void) {return Fps;}
	static int Get_Think_Count(void) {return ThinkCount;}
	static void Shell_Command(LPCSTR command);

   static void Server_Packet_Handler(cPacket & packet, int rhost_id);
   static void Client_Packet_Handler(cPacket & packet);
	static void Server_Broken_Connection_Handler(int rhost_id);
	static void Client_Broken_Connection_Handler(void);
	static void Accept_Handler(void);
	static void Refusal_Handler(REFUSAL_CODE refusal_code);
   static void Connection_Handler(int new_rhost_id);
	static REFUSAL_CODE Application_Acceptance_Handler(cPacket & packet);
   static void Eviction_Handler(int evicted_rhost_id);

   static bool I_Am_Client(void)				{return PClientConnection != NULL;}
   static bool I_Am_Server(void)				{return PServerConnection != NULL;}
   static bool	I_Am_Only_Client(void)		{return PClientConnection != NULL && PServerConnection == NULL;}
   static bool	I_Am_Only_Server(void)		{return PClientConnection == NULL && PServerConnection != NULL;}
   static bool	I_Am_Client_Server(void)	{return PClientConnection != NULL && PServerConnection != NULL;}
	static bool I_Am_God(void);


	// Sending Simple Client Packets
   static int Send_Object_Update(NetworkObjectClass *object, int client_id);
   static void Tell_Client_About_Dynamic_Objects(int recipient_client_id, Vector3 & dest_pos);
	static void Tell_Client_About_Delete_Notifications(int recipient_client_id);
   static void Tell_Server_About_Dynamic_Objects(void);

   static void Server_Kill_Connection(int client_id);
	static void Set_Receiver(CombatNetworkReceiver *receiver )		{Receiver = receiver;}

   static void Process_Eviction_Sc(cPacket & packet);

   static void Cleanup_After_Client(int client_id);
   static void Delete_Player_Objects(int client_id);
   static void Remove_Player(int player_id);
	static void Test_For_Team_Defaulting(cPlayer * p_player);

   static void Packet_Graph(Render2DTextClass * renderer, cMsgStatList * p_stat_list, int watch_type, LPCSTR label);
	static void Watch_Packets(Render2DTextClass * renderer);
	static void Bandwidth_Graph(Render2DTextClass * renderer, LPCSTR label, int count, int target_bps, float packetloss_pc,
		const int bar_color, bool is_send, float threshold_priority);
	static void Watch_Bandwidth(Render2DTextClass * renderer);
	static void Latency_Graph(Render2DTextClass * renderer, LPCSTR label, const float bar_length, cRemoteHost * p_rhost);
	static void Last_Contact_Graph(Render2DTextClass * renderer, LPCSTR label, const float bar_length, cRemoteHost * p_rhost);
	static void List_Size_Graph(Render2DTextClass * renderer, LPCSTR label, const float bar_length, int count);
	static void List_Time_Graph(Render2DTextClass * renderer, LPCSTR label, const float bar_length, int time_ms);
	static void List_Packet_Size_Graph(Render2DTextClass * renderer, LPCSTR label, const float bar_length, int count);
	static void Watch_Latency(Render2DTextClass * renderer);
	static void Watch_Last_Contact(Render2DTextClass * renderer);
	static void Watch_Size_Lists(Render2DTextClass * renderer);
	static void Watch_Time_Lists(Render2DTextClass * renderer);
	static void Watch_Packet_Size_Lists(Render2DTextClass * renderer);
	static void Simulation_Warnings(Render2DTextClass * renderer);
	static void Connection_Status_Change_Feedback(void);


	static void Client_Send_Packet(cPacket & packet, int mode);
	static void Server_Send_Packet(cPacket & packet, int mode, int recipient = ALL);
	static void Server_Send_Packet_To_All_Connected(cPacket & packet, int mode);

   static int Show_Welcome_Message(WideStringClass & name);

   static cPlayer * Get_My_Player_Object(void);
   static int Get_My_Team_Number(void);
   static Vector3 Get_My_Color(void);

   static LPCSTR Get_Client_String(int recipient);
   static LPCSTR Get_Client_Enumeration_String(void);

   static cRemoteHost * Get_Server_Rhost(int client_id);
   static cRemoteHost * Get_Client_Rhost(void);
   static float Get_Server_Rhost_Threshold_Priority(int client_id);
   static float Get_Client_Rhost_Threshold_Priority(void);

   static int	Get_My_Id(void);

	static uint32_t	Get_Exe_Key(void)						{ return ExeKey; }
	static uint32_t	Get_Exe_CRC(void)						{ return ExeCRC; }
	static uint32_t	Get_Strings_CRC(void)				{ return StringsCRC; }

   static void Set_Desired_Frame_Sleep_Ms(int b);
   static void Set_Simulated_Packet_Loss_Pc(int b);
   static void Set_Simulated_Packet_Duplication_Pc(int b);
   static void Set_Simulated_Latency_Range_Ms(int lower, int upper);
   static void Set_Spam_Count(int spam_count);

   static void Get_Simulated_Latency_Range_Ms(int & lower, int & upper);

	static void Set_Graphing_Y(float y)						{GraphingY = y;}

   static void Init_Client(unsigned short my_port = 0);
   static void Init_Server(void);

   static void Cleanup_Server(void);
   static void Cleanup_Client(void);

	static void Flush(void);//TSS2001e

	static void SwitchTeam(int newTeam);

	static void Enable_Waiting_Players(void);


	//
	// Hide...
	//
	static cConnection * PServerConnection;
   static cConnection * PClientConnection;

	// Temp - going away later. ST - 10/16/2001 9:33PM
	static bool SensibleUpdates;



private:

	static int	Get_Data_Files_CRC(void);

   static void Shared_Client_And_Server_Think(void);
   static bool Client_Think(void);
   static bool Server_Think(void);
	static void Hibernation_Think(void);

   static void End_Game_Test(void);
   static void Intermission_Over_Processing(void);

	static VisTableClass * Peek_Temp_Vis_Table(void);

   static CombatNetworkReceiver *Receiver;

   static CombatNetworkReceiverInstanceClass * NetworkReceiver;

   static char ClientString[300]; // used by Get_Client_String
   static char ClientEnumerationString[300]; // used by Get_Client_Enumeration_String

	static bool LastServerConnectionStateBad;

   //
   // Version validation data
   //
	static uint32_t ExeKey;
	static uint32_t ExeCRC;
	static uint32_t StringsCRC;

	//
   // Server-specific data
   //

   static char MessageToSend[MAX_TEXTMESSAGE_LENGTH];
   static char Command[MAX_TEXTMESSAGE_LENGTH];

	static float GraphingY;
	static float BandwidthBarLength;
   static int BandwidthScaler;

	static GameCombatNetworkHandlerClass NetHandler;

	static int Fps;
	static int ThinkCount;

	static cMsgStatList *		PClientStatList;
	static cMsgStatListGroup *	PServerStatListGroup;

	static bool						HaveDoneTeamChangeDialog;

	static bool						HaveDoneMotdDialog;

	static VisTableClass *		VisTable;
};

#endif





