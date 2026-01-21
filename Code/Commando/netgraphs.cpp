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
 *                     $Archive:: /Commando/Code/Commando/netgraphs.cpp                          $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/11/01 1:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 40                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef WWDEBUG

#include "cnetwork.h"

#include <stdio.h>
#include "systimer.h"

#include "font3d.h"
#include "miscutil.h"
#include "gamemode.h"
#include "wolgmode.h"
#include "player.h"
#include "devoptions.h"
#include "msgstatlistgroup.h"
#include "render2d.h"
#include "networkobjectmgr.h"
#include "apppackettypes.h"

#define	COLOR_BLACK			0xFF000000
#define	COLOR_WHITE			0xFFFFFFFF
#define	COLOR_RED			0xFFFF0000
#define	COLOR_BLUE			0xFF6060FF
#define	COLOR_GREEN			0xFF48FF48
#define	COLOR_DARK_GREEN	0xFF00C800
#define	COLOR_YELLOW		0xFFFFFF00
#define	COLOR_DARK_RED		0xFF990000
#define	COLOR_RED_DIM		0xFF800000
#define	COLOR_CYAN			0xFF00FFFF
#define	COLOR_GRAY			0xFF808080

//-----------------------------------------------------------------------------
void cNetwork::Packet_Graph(Render2DTextClass * renderer, cMsgStatList * p_stat_list,
	int watch_type, LPCSTR label)
{
	WWASSERT(renderer		!= NULL);
	WWASSERT(p_stat_list	!= NULL);
	WWASSERT(label	!= NULL);

	GraphingY = cMathUtil::Round(GraphingY);

	//
	// Draw scaling bar
	//
	const float bar_length			= 200.0f;
	const float bar_height			= cMathUtil::Round(renderer->Peek_Font()->Char_Height());
	const float y_increment			= cMathUtil::Round(bar_height * 1.25f);

	renderer->Draw_Block(RectClass(0, GraphingY, bar_length, GraphingY + bar_height), COLOR_BLACK);

	const int max_samples			= 500;

	char full_label[300];
	::sprintf(full_label, "%s / %d", label, max_samples);

	renderer->Set_Location(Vector2(2, GraphingY));
	renderer->Draw_Text(full_label);

	GraphingY += y_increment;

	for (int message_type = 0; message_type <= p_stat_list->Get_Num_Stats(); message_type++) {

		int count = 0;

		switch (watch_type) {
			case 0:
				count = p_stat_list->Get_Num_Msg_Sent(message_type);
				break;

			case 1:
				count = p_stat_list->Get_Num_Byte_Sent(message_type);
				break;

			case 2:
				count = p_stat_list->Get_Num_Msg_Recd(message_type);
				break;

			case 3:
				count = p_stat_list->Get_Num_Byte_Recd(message_type);
				break;

			case 4:
				count = p_stat_list->Compute_Avg_Num_Byte_Sent(message_type);
				break;

			case 5:
				count = p_stat_list->Compute_Avg_Num_Byte_Recd(message_type);
				break;

			default:
				DIE;
				break;
		}

		float fraction = (count % (max_samples + 1)) / (float) max_samples;
		renderer->Draw_Block(RectClass(0, GraphingY, fraction * bar_length, GraphingY + bar_height), COLOR_RED_DIM);

		char text[200];
		strcpy(text, p_stat_list->Get_Name(message_type));
		renderer->Set_Location(Vector2(2, GraphingY));
		renderer->Draw_Text(text);

		sprintf(text, "%d", count);
		renderer->Set_Location(Vector2(bar_length, GraphingY));
		renderer->Draw_Text(text);

		GraphingY += y_increment;
	}
}

//-----------------------------------------------------------------------------
void cNetwork::Watch_Packets(Render2DTextClass * renderer)
{
	WWASSERT(renderer		!= NULL);

	char					text[100]		= "";
	cMsgStatList *		p_stat_list		= NULL;
	int					watch_type		= -1;

	if (cDevOptions::PacketsSentServer.Is_True() && PServerStatListGroup != NULL) {
		sprintf(text, "PacketsSentServer");
		p_stat_list = PServerStatListGroup->Get_Stat_List(cMsgStatListGroup::ALL_LISTS);
		watch_type = 0;
	}

	if (cDevOptions::PacketsSentClient.Is_True()) {
		sprintf(text, "PacketsSentClient");
		p_stat_list = PClientStatList;
		watch_type = 0;
	}

	if (cDevOptions::PacketsRecdServer.Is_True() && PServerStatListGroup != NULL) {
		sprintf(text, "PacketsRecdServer");
		p_stat_list = PServerStatListGroup->Get_Stat_List(cMsgStatListGroup::ALL_LISTS);
		watch_type = 2;
	}

	if (cDevOptions::PacketsRecdClient.Is_True()) {
		sprintf(text, "PacketsRecdClient");
		p_stat_list = PClientStatList;
		watch_type = 2;
	}

	if (cDevOptions::AvgSizePacketsSentServer.Is_True() && PServerStatListGroup != NULL) {
		sprintf(text, "AvgSizePacketsSentServer");
		p_stat_list = PServerStatListGroup->Get_Stat_List(cMsgStatListGroup::ALL_LISTS);
		watch_type = 4;
	}

	if (cDevOptions::AvgSizePacketsSentClient.Is_True()) {
		sprintf(text, "AvgSizePacketsSentClient");
		p_stat_list = PClientStatList;
		watch_type = 4;
	}

	if (cDevOptions::AvgSizePacketsRecdServer.Is_True() && PServerStatListGroup != NULL) {
		sprintf(text, "AvgSizePacketsRecdServer");
		p_stat_list = PServerStatListGroup->Get_Stat_List(cMsgStatListGroup::ALL_LISTS);
		watch_type = 5;
	}

	if (cDevOptions::AvgSizePacketsRecdClient.Is_True()) {
		sprintf(text, "AvgSizePacketsRecdClient");
		p_stat_list = PClientStatList;
		watch_type = 5;
	}

	if (cDevOptions::BytesSentServer.Is_True() && PServerStatListGroup != NULL) {
		sprintf(text, "BytesSentServer");
		p_stat_list = PServerStatListGroup->Get_Stat_List(cMsgStatListGroup::ALL_LISTS);
		watch_type = 1;
	}

	if (cDevOptions::BytesSentClient.Is_True()) {
		sprintf(text, "BytesSentClient");
		p_stat_list = PClientStatList;
		watch_type = 1;
	}

	if (cDevOptions::BytesRecdServer.Is_True() && PServerStatListGroup != NULL) {
		sprintf(text, "BytesRecdServer");
		p_stat_list = PServerStatListGroup->Get_Stat_List(cMsgStatListGroup::ALL_LISTS);
		watch_type = 3;
	}

	if (cDevOptions::BytesRecdClient.Is_True()) {
		sprintf(text, "BytesRecdClient");
		p_stat_list = PClientStatList;
		watch_type = 3;
	}




	if (cDevOptions::WwnetPacketsSentServer.Is_True() && PServerConnection != NULL) {
		sprintf(text, "WwnetPacketsSentServer");
		p_stat_list = PServerConnection->Get_Stat_List();
		watch_type = 0;
	}

	if (cDevOptions::WwnetPacketsSentClient.Is_True() && PClientConnection != NULL) {
		sprintf(text, "WwnetPacketsSentClient");
		p_stat_list = PClientConnection->Get_Stat_List();
		watch_type = 0;
	}

	if (cDevOptions::WwnetPacketsRecdServer.Is_True() && PServerConnection != NULL) {
		sprintf(text, "WwnetPacketsRecdServer");
		p_stat_list = PServerConnection->Get_Stat_List();
		watch_type = 2;
	}

	if (cDevOptions::WwnetPacketsRecdClient.Is_True() && PClientConnection != NULL) {
		sprintf(text, "WwnetPacketsRecdClient");
		p_stat_list = PClientConnection->Get_Stat_List();
		watch_type = 2;
	}

	if (cDevOptions::WwnetAvgSizePacketsSentServer.Is_True() && PServerConnection != NULL) {
		sprintf(text, "WwnetAvgSizePacketsSentServer");
		p_stat_list = PServerConnection->Get_Stat_List();
		watch_type = 4;
	}

	if (cDevOptions::WwnetAvgSizePacketsSentClient.Is_True() && PClientConnection != NULL) {
		sprintf(text, "WwnetAvgSizePacketsSentClient");
		p_stat_list = PClientConnection->Get_Stat_List();
		watch_type = 4;
	}

	if (cDevOptions::WwnetAvgSizePacketsRecdServer.Is_True() && PServerConnection != NULL) {
		sprintf(text, "WwnetAvgSizePacketsRecdServer");
		p_stat_list = PServerConnection->Get_Stat_List();
		watch_type = 5;
	}

	if (cDevOptions::WwnetAvgSizePacketsRecdClient.Is_True() && PClientConnection != NULL) {
		sprintf(text, "WwnetAvgSizePacketsRecdClient");
		p_stat_list = PClientConnection->Get_Stat_List();
		watch_type = 5;
	}

	if (cDevOptions::WwnetBytesSentServer.Is_True() && PServerConnection != NULL) {
		sprintf(text, "WwnetBytesSentServer");
		p_stat_list = PServerConnection->Get_Stat_List();
		watch_type = 1;
	}

	if (cDevOptions::WwnetBytesSentClient.Is_True() && PClientConnection != NULL) {
		sprintf(text, "WwnetBytesSentClient");
		p_stat_list = PClientConnection->Get_Stat_List();
		watch_type = 1;
	}

	if (cDevOptions::WwnetBytesRecdServer.Is_True() && PServerConnection != NULL) {
		sprintf(text, "WwnetBytesRecdServer");
		p_stat_list = PServerConnection->Get_Stat_List();
		watch_type = 3;
	}

	if (cDevOptions::WwnetBytesRecdClient.Is_True() && PClientConnection != NULL) {
		sprintf(text, "WwnetBytesRecdClient");
		p_stat_list = PClientConnection->Get_Stat_List();
		watch_type = 3;
	}



	if (p_stat_list != NULL)
	{
		WWASSERT(watch_type != -1);

		Packet_Graph(renderer, p_stat_list, watch_type, text);
	}
}

//-----------------------------------------------------------------------------
void cNetwork::Bandwidth_Graph(Render2DTextClass * renderer,
	LPCSTR label, int count, int target_bps,
	float packetloss_pc, const int bar_color, bool is_send,
	float threshold_priority)
{
	WWASSERT(renderer != NULL);
	WWASSERT(label != NULL);

	const int screen_width	= Render2DClass::Get_Screen_Resolution().Width();
	const int screen_height = Render2DClass::Get_Screen_Resolution().Height();

	WWASSERT(packetloss_pc > -MISCUTIL_EPSILON && packetloss_pc < 100 + MISCUTIL_EPSILON);

	const float bar_height		= cMathUtil::Round(renderer->Peek_Font()->Char_Height());
	const float y_increment		= cMathUtil::Round(bar_height * 1.25f);

	float end_x;
	float full_x;
	//float end_y;
   float ratio = (100 - packetloss_pc) / 100.0f;

   if (is_send) {
      full_x = count / (float) BandwidthScaler * BandwidthBarLength;
      end_x = full_x * ratio;
   } else {
      end_x = count / (float) BandwidthScaler * BandwidthBarLength;
      full_x = end_x / ratio;
   }

	//end_y = GraphingY + bar_height;

   if (packetloss_pc > 0) {
      //char text[200];
		//sprintf(text, "PL %7.4f%%", packetloss_pc);
		//p_text_drawer->Print(p_font, text, 0.3f, GraphingY);
		renderer->Draw_Block(RectClass( 0, GraphingY, full_x, GraphingY + bar_height), COLOR_DARK_RED);
   }

	renderer->Draw_Block(RectClass(0, GraphingY, end_x, GraphingY + bar_height), bar_color);

	renderer->Set_Location(Vector2(2, GraphingY));
	renderer->Draw_Text(label);

   /*
   char text[200];
	sprintf(text, "%d", count);
	p_text_drawer->Print(p_font, text, 0.3f, GraphingY);
   */

	if (target_bps != -1) {
		float target_x;

		//
		// Per second
		//
		target_x = target_bps / (float) BandwidthScaler * BandwidthBarLength;
		renderer->Draw_Block(RectClass(target_x - 1, GraphingY, target_x + 1, GraphingY + bar_height));

		/**/
		//
		// For this sample
		//
		target_bps = (int) (target_bps * cNetUtil::NETSTATS_SAMPLE_TIME_MS / 1000.0f);

		target_x = target_bps / (float) BandwidthScaler * BandwidthBarLength;
		renderer->Draw_Block(RectClass(target_x - 1, GraphingY, target_x + 1, GraphingY + bar_height));
		/**/
	}

	//
	// ThresholdPriority
	//
	if (threshold_priority > -1) {

		char tp_text[100];
		sprintf(tp_text, "TP:%-8.6f", threshold_priority);
		renderer->Set_Location(Vector2(500, GraphingY));
		renderer->Draw_Text(tp_text);

		/*
		const float TPLine_size = 1;
		float x1 = threshold_priority - TPLine_size;
		float x2 = x1 + 2 * TPLine_size;
		renderer->Draw_Block(RectClass(x1, GraphingY, x2, GraphingY + bar_height), COLOR_GREEN);
		*/

		float tp = threshold_priority * screen_width;

		//TSS2001d renderer->Draw_Block(RectClass(tp - 1, GraphingY, tp + 1, GraphingY + bar_height), COLOR_GREEN);
		renderer->Draw_Block(RectClass(tp - 1, GraphingY, tp + 1, screen_height), COLOR_GREEN);
	}

	GraphingY += y_increment;

	/*
	if (full_x > 1 || end_x > 1) {
		//BandwidthBarLength /= 2.0f;
      BandwidthScaler *= 2;
	}
	*/

	/*
	//
	// TSS101101
	//
	if (full_x > Render2DClass::Get_Screen_Resolution().Width() || end_x > Render2DClass::Get_Screen_Resolution().Width()) {
		//BandwidthBarLength /= 2.0f;
      BandwidthScaler *= 2;
	}
	*/


	//TSS2001d
	//
	// Render object prioritys so can compare against threshold priority
	//
	if (cNetwork::I_Am_Server())	{
		int top = (int)(0.2 * screen_height);
		int obj_count = NetworkObjectMgrClass::Get_Object_Count();

		int x_pos = screen_width - 75;

		renderer->Set_Location(Vector2(x_pos, 300));
		renderer->Draw_Text("UNKNOWN", COLOR_BLACK);

		renderer->Set_Location(Vector2(x_pos, 310));
		renderer->Draw_Text("SOLDIER", COLOR_RED);

		renderer->Set_Location(Vector2(x_pos, 320));
		renderer->Draw_Text("VEHICLE", COLOR_GREEN);

		renderer->Set_Location(Vector2(x_pos, 330));
		renderer->Draw_Text("TURRET", COLOR_DARK_GREEN);

		renderer->Set_Location(Vector2(x_pos, 340));
		renderer->Draw_Text("BUILDING", COLOR_BLUE);

		renderer->Set_Location(Vector2(x_pos, 350));
		renderer->Draw_Text("PLAYER", COLOR_YELLOW);

		renderer->Set_Location(Vector2(x_pos, 360));
		renderer->Draw_Text("POWERUP", COLOR_CYAN);

		renderer->Set_Location(Vector2(x_pos, 370));
		renderer->Draw_Text("OTHER", COLOR_WHITE);

		for (int index = 0; index < obj_count; index ++) {

			NetworkObjectClass * p_object = NetworkObjectMgrClass::Get_Object(index);
			WWASSERT(p_object != NULL);
			float priority = 0;//XXX p_object->Get_Cached_Priority();

			if (priority > 0) {

				RectClass rect;
				rect.Left	= (int)(priority * screen_width) - 3;
				rect.Right	= rect.Left + 6;
				rect.Top		= top;
				rect.Bottom	= rect.Top + 1;

				/*
				if (p_object->Is_Tagged())
				{
					renderer->Draw_Block(rect, COLOR_YELLOW);
				}
				else
				{
					renderer->Draw_Block(rect, COLOR_WHITE);
				}
				*/

				int color = COLOR_WHITE;
				switch (p_object->Get_App_Packet_Type())
				{
					case APPPACKETTYPE_UNKNOWN:						color = COLOR_BLACK;			break;
					case APPPACKETTYPE_SOLDIER:						color = COLOR_RED;			break;
					case APPPACKETTYPE_VEHICLE:						color = COLOR_GREEN;			break;
					case APPPACKETTYPE_TURRET:							color = COLOR_DARK_GREEN;	break;
					case APPPACKETTYPE_BUILDING:						color = COLOR_BLUE;			break;
					case APPPACKETTYPE_PLAYER:							color = COLOR_YELLOW;		break;
					case APPPACKETTYPE_POWERUP:						color = COLOR_CYAN;			break;

					case APPPACKETTYPE_ALL:								DIE;								break;

					default:													color = COLOR_WHITE;			break;
				}

				renderer->Draw_Block(rect, color);

				top += 3;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void cNetwork::Watch_Bandwidth(Render2DTextClass * renderer)
{
	WWASSERT(renderer != NULL);

	bool bail = true;

#ifdef WWDEBUG
	if (cDevOptions::ShowBandwidth.Is_True()) {
		bail = false;
	}
#endif // WWDEBUG

	if (bail) {
		return;
	}

	//
	// Draw the scale at the top
	//
	const int	screen_width	= Render2DClass::Get_Screen_Resolution().Width();
	const float	bar_height		= cMathUtil::Round(renderer->Peek_Font()->Char_Height());
	const float y_increment		= cMathUtil::Round(bar_height * 1.25f);

	//BandwidthBarLength = screen_width / 8.0f;
	BandwidthBarLength = screen_width / 10.0f;

	float x1 = 0;
	float x2 = 0;
	int i		= 0;

	do {
		x2 = x1 + BandwidthBarLength;
		renderer->Draw_Block(RectClass(x1, GraphingY, x2, GraphingY + bar_height), (i % 2) ? COLOR_WHITE : COLOR_BLACK);
		x1 = x2;
		i++;
	} while (x2 < screen_width);

   //
	// Label the scale
	//
	char label[100];
   sprintf(label, "BW / %d", BandwidthScaler);
	renderer->Set_Location(Vector2(2, GraphingY));
	renderer->Draw_Text(label);

	GraphingY += y_increment;

	char text[100];

	if (I_Am_Client() && PClientConnection->Is_Established()) {

		sprintf(text, "c%d->s", Get_My_Id());

		Bandwidth_Graph(
			renderer,
			text,
			//XXX Get_Client_Rhost()->Get_Stats().StatSnapshot[STAT_BitsSent],
			Get_Client_Rhost()->Get_Stats().StatMacroSnapshot[STAT_BitsSent],
			Get_Client_Rhost()->Get_Target_Bps(),
         Get_Client_Rhost()->Get_Stats().Get_Pc_Packetloss_Sent(),
			COLOR_RED,
         true,
			Get_Client_Rhost()->Get_Threshold_Priority());

		sprintf(text, "c%d<-s", Get_My_Id());
		Bandwidth_Graph(
			renderer,
			text,
			//XXX Get_Client_Rhost()->Get_Stats().StatSnapshot[STAT_BitsRcv],
			Get_Client_Rhost()->Get_Stats().StatMacroSnapshot[STAT_BitsRcv],
			-1,
         Get_Client_Rhost()->Get_Stats().Get_Pc_Packetloss_Received(),
         COLOR_RED,
         false,
			-1);

		/*
      char text[200];
		sprintf(text, "PACKETLOSS %7.4f",
			Get_Client_Rhost()->Get_Stats().Get_Pc_Packetloss_Received());
		p_text_drawer->Print(p_font, text, 0.5f, 0.2f);
      */
	}

	if (PServerConnection != NULL) {

		Bandwidth_Graph(
			renderer,
			"s->c*",
			//XXX PServerConnection->Get_Combined_Stats().StatSnapshot[STAT_BitsSent],
			PServerConnection->Get_Combined_Stats().StatMacroSnapshot[STAT_BitsSent],
			PServerConnection->Get_Bandwidth_Budget_Out(),
			0,
			COLOR_BLUE,
         true,
			-1);

		Bandwidth_Graph(
			renderer,
			"s<-c*",
			//XXX PServerConnection->Get_Combined_Stats().StatSnapshot[STAT_BitsRcv],
			PServerConnection->Get_Combined_Stats().StatMacroSnapshot[STAT_BitsRcv],
			-1,
			0,
			COLOR_BLUE,
         false,
			-1);

	   for (int i = PServerConnection->Get_Min_RHost(); i <= PServerConnection->Get_Max_RHost(); i++) {

			WWASSERT(
				i >= cNetwork::PServerConnection->Get_Min_RHost() &&
				i <= cNetwork::PServerConnection->Get_Max_RHost());

			cRemoteHost * p_rhost = Get_Server_Rhost(i);

			if (p_rhost != NULL) {

				sprintf(text, "s->c%d", i);
				Bandwidth_Graph(
					renderer,
					text,
					//XXX p_rhost->Get_Stats().StatSnapshot[STAT_BitsSent],
					p_rhost->Get_Stats().StatMacroSnapshot[STAT_BitsSent],
					p_rhost->Get_Target_Bps(),
               p_rhost->Get_Stats().Get_Pc_Packetloss_Sent(),
					COLOR_RED,
               true,
					p_rhost->Get_Threshold_Priority());

				sprintf(text, "s<-c%d", i);
				Bandwidth_Graph(
					renderer,
					text,
					//XXX p_rhost->Get_Stats().StatSnapshot[STAT_BitsRcv],
					p_rhost->Get_Stats().StatMacroSnapshot[STAT_BitsRcv],
               -1,
					p_rhost->Get_Stats().Get_Pc_Packetloss_Received(),
					COLOR_RED,
               false,
					-1);

				/*
            char text[200];
				sprintf(text, "PACKETLOSS %7.4f",
					p_rhost->Get_Stats().Get_Pc_Packetloss_Received());
				p_text_drawer->Print(p_font, text, 0.5f, 0.5 + 0.05 * i);
            */
			}
		}
	}
}

//-----------------------------------------------------------------------------
void cNetwork::Latency_Graph(Render2DTextClass * renderer, LPCSTR label,
	const float bar_length, cRemoteHost * p_rhost)
{
	WWASSERT(renderer != NULL);
	WWASSERT(label != NULL);
	WWASSERT(p_rhost != NULL);

	int count = p_rhost->Get_Average_Internal_Pingtime_Ms();

	const float bar_width = 8;
	const float latency_block = 100.0f;
	const float graph_separation = 7;

	float end_x;
	float end_y;

	end_x = count / latency_block * bar_length;
	end_y = GraphingY + bar_width;
	renderer->Draw_Block( RectClass( 0, GraphingY, end_x, end_y ), COLOR_RED );
	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( label, COLOR_YELLOW  );

	/*
	char remote_framerate[20];
	sprintf(remote_framerate, "%d fps", p_rhost->Get_Stats().Get_Remote_Service_Count());
	renderer->Set_Location( Vector2( 200, GraphingY+1 ) );
	renderer->Draw_Text( remote_framerate, COLOR_YELLOW  );
	*/

	/*
	char txt[200];
	sprintf(txt, "%s: %d/%d = %d", label, p_rhost->TotalInternalPingtimeMs,
		p_rhost->NumInternalPings, p_rhost->AverageInternalPingtimeMs);
	p_text_drawer->Print(p_font, txt, 0, GraphingY);
	*/
	GraphingY += bar_width + graph_separation;
}

//-----------------------------------------------------------------------------
void cNetwork::Watch_Latency(Render2DTextClass * renderer)
{
	WWASSERT(renderer != NULL);

	if (cDevOptions::ShowLatency.Is_False()) {
		return;
	}

	const float bar_length = 64;
	float x1 = 0;
	float x2;
	float end_y = GraphingY + 8;
	for (int i = 0; i < 10; i++) {
		x2 = x1 + bar_length;
		renderer->Draw_Block( RectClass( x1, GraphingY, x2, end_y ), (i%2)?COLOR_WHITE:COLOR_BLACK );
		x1 = x2;
	}
	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( "LAT/100ms", COLOR_YELLOW  );
	GraphingY += 10;

	char text[100];

	if (I_Am_Client() && PClientConnection->Is_Established()) {
		sprintf(text, "c%d->s", Get_My_Id());
		Latency_Graph(renderer, text, bar_length, Get_Client_Rhost());
	}

	if (I_Am_Server()) {
	   for (int i = PServerConnection->Get_Min_RHost(); i <= PServerConnection->Get_Max_RHost(); i++) {
			if (Get_Server_Rhost(i) != NULL) {
				sprintf(text, "s->c%d", i);
				Latency_Graph(renderer, text, bar_length, Get_Server_Rhost(i));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void cNetwork::Last_Contact_Graph(Render2DTextClass * renderer,
	LPCSTR label, const float bar_length,
	cRemoteHost * p_rhost)
{
	WWASSERT(renderer != NULL);
	WWASSERT(label != NULL);
	WWASSERT(p_rhost != NULL);

	int count = TIMEGETTIME() - p_rhost->Get_Last_Contact_Time();
	WWASSERT(count >= 0);

	const float bar_width = 0.017f;
	const float time_block = 1000.0f;
	const float graph_separation = 0.01f;

	float end_x;
	float end_y;

	end_x = count / time_block * bar_length;
	end_y = GraphingY + bar_width;
	renderer->Draw_Block( RectClass( 0, GraphingY, end_x, end_y ), COLOR_RED );

	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( label, COLOR_RED  );

	GraphingY = end_y + graph_separation;
}

//-----------------------------------------------------------------------------
void cNetwork::Watch_Last_Contact(Render2DTextClass * renderer)
{
	WWASSERT(renderer != NULL);

	if (cDevOptions::ShowLastContact.Is_False()) {
		return;
	}

	const float bar_length = 24;
	float x1 = 0;
	float x2;
	float end_y = GraphingY + 8;
	for (int i = 0; i < 20; i++) {
		x2 = x1 + bar_length;
		renderer->Draw_Block( RectClass( x1, GraphingY, x2, end_y ), (i%2)?COLOR_WHITE:COLOR_BLACK );
		x1 = x2;
	}
	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( "LAST/1s", COLOR_RED );
	GraphingY += 14;

	char text[100];

	if (I_Am_Client() && PClientConnection->Is_Established()) {
		sprintf(text, "c%d->s", Get_My_Id());
		Last_Contact_Graph(renderer, text, bar_length, Get_Client_Rhost());
	}

	if (I_Am_Server()) {
	   for (int i = PServerConnection->Get_Min_RHost(); i <= PServerConnection->Get_Max_RHost(); i++) {
			if (Get_Server_Rhost(i) != NULL) {
				sprintf(text, "s->c%d", i);
				Last_Contact_Graph(renderer, text, bar_length, Get_Server_Rhost(i));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void cNetwork::List_Size_Graph(Render2DTextClass * renderer,
	LPCSTR label, const float bar_length, int count)
{
	WWASSERT(renderer != NULL);
	WWASSERT(label != NULL);

	const float bar_width = 12;
	const float list_size_block = 10.0f;
	const float graph_separation = 5;

	float end_x = count / list_size_block * bar_length;
	float end_y = GraphingY + bar_width;

	renderer->Draw_Block( RectClass( 0, GraphingY, end_x, end_y ), COLOR_RED );
	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( label, COLOR_RED );

	GraphingY += bar_width + graph_separation;
}

//-----------------------------------------------------------------------------
void cNetwork::List_Time_Graph(Render2DTextClass * renderer,
	LPCSTR label, const float bar_length, int time_ms)
{
	WWASSERT(renderer != NULL);
	WWASSERT(label != NULL);

	const float bar_width = 12;
	const float list_block = 50.0f;
	const float graph_separation = 5;

	float end_x = time_ms / list_block * bar_length;
	float end_y = GraphingY + bar_width;
	renderer->Draw_Block( RectClass( 0, GraphingY, end_x, end_y ), COLOR_RED );
	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( label, COLOR_RED );

	GraphingY += bar_width + graph_separation;
}

//-----------------------------------------------------------------------------
void cNetwork::List_Packet_Size_Graph(Render2DTextClass * renderer,
	LPCSTR label, const float bar_length, int count)
{
	WWASSERT(renderer != NULL);
	WWASSERT(label != NULL);

	const float bar_width = 12;
	const float list_block = 200.0f;
	const float graph_separation = 5;

	float end_x = count / list_block * bar_length;
	float end_y = GraphingY + bar_width;

	renderer->Draw_Block( RectClass( 0, GraphingY, end_x, end_y ), COLOR_RED );
	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( label, COLOR_RED );

	GraphingY += bar_width + graph_separation;
}

//-----------------------------------------------------------------------------
void cNetwork::Watch_Size_Lists(Render2DTextClass * renderer)
{
	WWASSERT(renderer != NULL);

	if (cDevOptions::ShowListSizes.Is_False()) {
		return;
	}

	const float bar_length = 64;
	float x1 = 0;
	float x2;
	float end_y = GraphingY + 12;
	for (int i = 0; i < 10; i++) {
		x2 = x1 + bar_length;
		renderer->Draw_Block( RectClass( x1, GraphingY, x2, end_y ), (i%2)?COLOR_WHITE:COLOR_BLACK );
		x1 = x2;
	}
	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( "LIST/10", COLOR_RED );

	GraphingY += 18;

	char text[100];

	if (I_Am_Client() && PClientConnection->Is_Established()) {
		sprintf(text, "c rsnd");
		List_Size_Graph(renderer,  text, bar_length,
			Get_Client_Rhost()->Get_List_Max(RELIABLE_SEND_LIST));

		sprintf(text, "c rrcv");
		List_Size_Graph(renderer,  text, bar_length,
			Get_Client_Rhost()->Get_List_Max(RELIABLE_RCV_LIST));

		sprintf(text, "c usnd");
		List_Size_Graph(renderer,  text, bar_length,
			Get_Client_Rhost()->Get_List_Max(UNRELIABLE_SEND_LIST));

		sprintf(text, "c urcv");
		List_Size_Graph(renderer,  text, bar_length,
			Get_Client_Rhost()->Get_List_Max(UNRELIABLE_RCV_LIST));
   }

   if (I_Am_Server()) {
	   for (int i = PServerConnection->Get_Min_RHost(); i <= PServerConnection->Get_Max_RHost(); i++) {
			if (Get_Server_Rhost(i) != NULL) {

            sprintf(text, "s rsnd %d", i);
				List_Size_Graph(renderer,  text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(RELIABLE_SEND_LIST));

		      sprintf(text, "s rrcv %d", i);
				List_Size_Graph(renderer,  text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(RELIABLE_RCV_LIST));

		      sprintf(text, "s usnd %d", i);
				List_Size_Graph(renderer,  text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(UNRELIABLE_SEND_LIST));

		      sprintf(text, "s urcv %d", i);
				List_Size_Graph(renderer,  text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(UNRELIABLE_RCV_LIST));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void cNetwork::Watch_Time_Lists(Render2DTextClass * renderer)
{
	WWASSERT(renderer != NULL);

	if (cDevOptions::ShowListTimes.Is_False()) {
		return;
	}

	const float bar_length = 64;
	float x1 = 0;
	float x2;
	float end_y = GraphingY + 12;
	for (int i = 0; i < 10; i++) {
		x2 = x1 + bar_length;
		renderer->Draw_Block( RectClass( x1, GraphingY, x2, end_y ), (i%2)?COLOR_WHITE:COLOR_BLACK );
		x1 = x2;
	}
	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( "LISTTIME/50ms", COLOR_RED );
	GraphingY += 20;

	char text[100];

	if (I_Am_Client() && PClientConnection->Is_Established()) {
		/*
		sprintf(text, "c rsnd");
		List_Size_Graph(p_text_drawer, p_graph_drawer,
         text, bar_length,
			Get_Client_Rhost()->Get_List_Max(RELIABLE_SEND_LIST),
			Get_Client_Rhost()->Get_List_Processing_Time(RELIABLE_SEND_LIST));

		sprintf(text, "c rrcv");
		List_Size_Graph(p_text_drawer, p_graph_drawer,
         text, bar_length,
			Get_Client_Rhost()->Get_List_Max(RELIABLE_RCV_LIST),
			Get_Client_Rhost()->Get_List_Processing_Time(RELIABLE_RCV_LIST));

		sprintf(text, "c usnd");
		List_Size_Graph(p_text_drawer, p_graph_drawer,
         text, bar_length,
			Get_Client_Rhost()->Get_List_Max(UNRELIABLE_SEND_LIST),
			Get_Client_Rhost()->Get_List_Processing_Time(UNRELIABLE_SEND_LIST));
		*/

		sprintf(text, "c urcv");
		List_Time_Graph(renderer, text, bar_length,
			Get_Client_Rhost()->Get_List_Processing_Time(UNRELIABLE_RCV_LIST));
   }

   /*
	if (I_Am_Server()) {
		for (int i = 0; i < MAX_RHOSTS; i++) {
			if (Get_Server_Rhost(i) != NULL) {

            sprintf(text, "s rsnd %d", i);
		      List_Size_Graph(p_text_drawer, p_graph_drawer,
               text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(RELIABLE_SEND_LIST),
					Get_Server_Rhost(i)->Get_List_Processing_Time(RELIABLE_SEND_LIST));

		      sprintf(text, "s rrcv %d", i);
		      List_Size_Graph(p_text_drawer, p_graph_drawer,
               text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(RELIABLE_RCV_LIST),
					Get_Server_Rhost(i)->Get_List_Processing_Time(RELIABLE_RCV_LIST));

		      sprintf(text, "s usnd %d", i);
		      List_Size_Graph(p_text_drawer, p_graph_drawer,
               text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(UNRELIABLE_SEND_LIST),
					Get_Server_Rhost(i)->Get_List_Processing_Time(UNRELIABLE_SEND_LIST));

		      sprintf(text, "s urcv %d", i);
		      List_Size_Graph(p_text_drawer, p_graph_drawer,
               text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(UNRELIABLE_RCV_LIST),
					Get_Server_Rhost(i)->Get_List_Processing_Time(UNRELIABLE_RCV_LIST));
			}
		}
	}
	*/
}

//-----------------------------------------------------------------------------
void cNetwork::Watch_Packet_Size_Lists(Render2DTextClass * renderer)
{
	WWASSERT(renderer != NULL);

	if (cDevOptions::ShowListPacketSizes.Is_False()) {
		return;
	}

	const float bar_length = 64;
	float x1 = 0;
	float x2;
	float end_y = GraphingY + 12;
	for (int i = 0; i < 10; i++) {
		x2 = x1 + bar_length;
		renderer->Draw_Block( RectClass( x1, GraphingY, x2, end_y ), (i%2)?COLOR_WHITE:COLOR_BLACK );
		x1 = x2;
	}
	renderer->Set_Location( Vector2( 0, GraphingY+1 ) );
	renderer->Draw_Text( "LISTPSZ/200b", COLOR_RED );
	GraphingY += 20;

	/*
	char text[100];

	if (I_Am_Client() && PClientConnection->Is_Established()) {
		sprintf(text, "c rsnd");
		List_Size_Graph(p_text_drawer, p_graph_drawer,
         text, bar_length,
			Get_Client_Rhost()->Get_List_Max(RELIABLE_SEND_LIST),
			Get_Client_Rhost()->Get_List_Processing_Time(RELIABLE_SEND_LIST));

		sprintf(text, "c rrcv");
		List_Size_Graph(p_text_drawer, p_graph_drawer,
         text, bar_length,
			Get_Client_Rhost()->Get_List_Max(RELIABLE_RCV_LIST),
			Get_Client_Rhost()->Get_List_Processing_Time(RELIABLE_RCV_LIST));

		sprintf(text, "c usnd");
		List_Size_Graph(p_text_drawer, p_graph_drawer,
         text, bar_length,
			Get_Client_Rhost()->Get_List_Max(UNRELIABLE_SEND_LIST),
			Get_Client_Rhost()->Get_List_Processing_Time(UNRELIABLE_SEND_LIST));
		*/

		/* broken
		sprintf(text, "c urcv");
		List_Packet_Size_Graph(p_text_drawer, p_graph_drawer,
         text, bar_length,
			Get_Client_Rhost()->Get_List_Packet_Size(UNRELIABLE_RCV_LIST));
   }
	*/

   /*
	if (I_Am_Server()) {
		for (int i = 0; i < MAX_RHOSTS; i++) {
			if (Get_Server_Rhost(i) != NULL) {

            sprintf(text, "s rsnd %d", i);
		      List_Size_Graph(p_text_drawer, p_graph_drawer,
               text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(RELIABLE_SEND_LIST),
					Get_Server_Rhost(i)->Get_List_Processing_Time(RELIABLE_SEND_LIST));

		      sprintf(text, "s rrcv %d", i);
		      List_Size_Graph(p_text_drawer, p_graph_drawer,
               text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(RELIABLE_RCV_LIST),
					Get_Server_Rhost(i)->Get_List_Processing_Time(RELIABLE_RCV_LIST));

		      sprintf(text, "s usnd %d", i);
		      List_Size_Graph(p_text_drawer, p_graph_drawer,
               text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(UNRELIABLE_SEND_LIST),
					Get_Server_Rhost(i)->Get_List_Processing_Time(UNRELIABLE_SEND_LIST));

		      sprintf(text, "s urcv %d", i);
		      List_Size_Graph(p_text_drawer, p_graph_drawer,
               text, bar_length,
					Get_Server_Rhost(i)->Get_List_Max(UNRELIABLE_RCV_LIST),
					Get_Server_Rhost(i)->Get_List_Processing_Time(UNRELIABLE_RCV_LIST));
			}
		}
	}
	*/
}

//-----------------------------------------------------------------------------
void cNetwork::Simulation_Warnings(Render2DTextClass * renderer)
{
	WWASSERT(renderer != NULL);

   static int color_cycle = 0;
	int color = (++color_cycle & 64) ? COLOR_WHITE : COLOR_GREEN;

   char message[300];

#ifdef WWDEBUG
	renderer->Set_Location( Vector2( 3, 360 ) );
   if (cDevOptions::DesiredFrameSleepMs.Get() > 0) {
		sprintf(message, "SLEEP: %d ms\n", cDevOptions::DesiredFrameSleepMs.Get());
		renderer->Draw_Text( message, color );
	}

   if (cDevOptions::SimulatedPacketLossPc.Get() > MISCUTIL_EPSILON) {
		sprintf(message, "SIM. PACKETLOSS = %5.2f %%\n", (float) cDevOptions::SimulatedPacketLossPc.Get());
		renderer->Draw_Text( message, color );
   }

   if (cDevOptions::SimulatedPacketDuplicationPc.Get() > MISCUTIL_EPSILON) {
		sprintf(message, "SIM. PACKET DUP. = %5.2f %%\n", (float) cDevOptions::SimulatedPacketDuplicationPc.Get());
		renderer->Draw_Text( message, color );
   }

   if (cDevOptions::SpamCount.Get() > 0) {
		sprintf(message, "SPAM COUNT = %d\n", cDevOptions::SpamCount.Get());
		renderer->Draw_Text( message, color );
   }

	int minimum_latency_ms;
	int maximum_latency_ms;
	Get_Simulated_Latency_Range_Ms(minimum_latency_ms, maximum_latency_ms);
   if (minimum_latency_ms > 0 || maximum_latency_ms > 0) {
		sprintf(message, "SIM. LATENCY = (%d, %d)\n", minimum_latency_ms, maximum_latency_ms);
		renderer->Draw_Text( message, color );
   }
#endif //WWDEBUG
}

/*
//-----------------------------------------------------------------------------
void cNetwork::Priorities_Graph(Render2DTextClass * renderer
{
	WWASSERT(p_text_drawer != NULL);
	WWASSERT(p_graph_drawer != NULL);

	if (cDevOptions::ShowPriorities.Is_False()) {
		return;
	}

	cRemoteHost * p_rhost = NULL;
	int rhost_index;
	if (cNetwork::I_Am_Client_Server()) {
		p_rhost = cNetwork::Get_Server_Rhost(2);
		rhost_index = 2;
	} else if (cNetwork::I_Am_Only_Server()) {
		p_rhost = cNetwork::Get_Server_Rhost(1);
		rhost_index = 1;
	} else if (cNetwork::I_Am_Client()) {
		p_rhost = cNetwork::Get_Client_Rhost();
		rhost_index = 0;
	}

	if (p_rhost != NULL) {
		p_graph_drawer->Set_Shader(ShaderClass::_PresetOpaqueSolidShader);

		float priority;
		float x1;
		float x2;
		float y1;
		float y2;

		const float blob_size = 0.004f;
		const float blob_size_2 = blob_size / 2.0f;

		y1 = 0;

			x1 = priority - blob_size_2;
			x2 = x1 + blob_size;
			y2 = y1 + blob_size;

			p_graph_drawer->Set_Vertex_Color();
			p_graph_drawer->Quad(x1, y1, x2, y2);

			y1 += blob_size_2;
	}
}
*/

#endif // WWDEBUG

























	//const float	graph_separation	= 9;
/*
//-----------------------------------------------------------------------------
void cNetwork::Watch_Bandwidth_Budget_Out(Render2DTextClass * renderer)
{
	WWASSERT(renderer != NULL);

	return;

   if (cDevOptions::ShowBandwidthBudgetOut.Is_False() || !I_Am_Server()) {
		return;
	}

	char text[200];
   WWASSERT(PServerConnection != NULL);
	sprintf(text, "BBO: %d bps", PServerConnection->Get_Bandwidth_Budget_Out());
	renderer->Set_Location(Vector2(2, 25));
	renderer->Draw_Text(text);
}
*/
