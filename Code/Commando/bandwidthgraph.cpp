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
 *                     $Archive:: /Commando/Code/Commando/bandwidthgraph.cpp                       $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/02/02 2:29a                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "bandwidthgraph.h"

#include "assets.h"
#include "font3d.h"
#include "render2d.h"
#include "cnetwork.h"
#include "devoptions.h"
#include "packetmgr.h"
#include "ServerSettings.h"
#include "ConsoleMode.h"


//
// Class statics
//
Render2DTextClass	*		cBandwidthGraph::PTextRenderer		= NULL;
Font3DInstanceClass *	cBandwidthGraph::PFont					= NULL;
int							cBandwidthGraph::BandwidthScaler		= 50000;
float							cBandwidthGraph::YPosition				= 0;
float							cBandwidthGraph::BarHeight				= 0;
float							cBandwidthGraph::BarWidth				= 0;
float							cBandwidthGraph::YIncrement			= 0;

#define	COLOR_BLACK		0xFF000000
#define	COLOR_WHITE		0xFFFFFFFF
#define	COLOR_RED		0xFFFF0000
#define	COLOR_GREEN		0xFF48FF48

//-----------------------------------------------------------------------------
void
cBandwidthGraph::Onetime_Init
(
	void
)
{
	WWDEBUG_SAY(("cBandwidthGraph::Onetime_Init\n"));

	bool can_render = ConsoleBox.Is_Exclusive() ? false : true;
	if (can_render) {
		WWASSERT(WW3DAssetManager::Get_Instance() != NULL);
   	PFont = WW3DAssetManager::Get_Instance()->Get_Font3DInstance("FONT6x8.TGA");
   	WWASSERT(PFont != NULL);
		PFont->Set_Mono_Spaced();
		SET_REF_OWNER(PFont);

		PTextRenderer = new Render2DTextClass(PFont);
		PTextRenderer->Set_Coordinate_Range(Render2DClass::Get_Screen_Resolution());

		BarWidth		= Render2DClass::Get_Screen_Resolution().Width() / 10.0f;
		BarHeight	= PTextRenderer->Peek_Font()->Char_Height();
		YIncrement	= BarHeight * 1.25f;
	}
	// This needs to be done somewhere else - we need a good value whether we have the graph or not.
	//PacketManager.Set_Stats_Sampling_Frequency_Delay(1000);
	//PacketManager.Set_Stats_Sampling_Frequency_Delay(250);
}

//-----------------------------------------------------------------------------
void
cBandwidthGraph::Onetime_Shutdown
(
	void
)
{
	WWDEBUG_SAY(("cBandwidthGraph::Onetime_Close\n"));
	if (PTextRenderer != NULL)
	{
		delete PTextRenderer;
		PTextRenderer = NULL;
	}

	if (PFont != NULL)
	{
		PFont->Release_Ref();
		PFont = NULL;
	}
}

//-----------------------------------------------------------------------------
void
cBandwidthGraph::Bandwidth_Graph
(
	StringClass &	label,
	int				bps,
	int				target_bps,
	float				bandwidth_multiplier,
	float				average_priority,
	bool				is_loading
)
{
	if (PTextRenderer == NULL) {
		return;
	}
	WWASSERT(PTextRenderer != NULL);

	float x1 = 0;
	float x2 = bps / (float) BandwidthScaler * BarWidth;
	float y1 = YPosition;
	float y2 = y1 + BarHeight;

	//
	// Bandwidth used (graph)
	//
	PTextRenderer->Draw_Block(RectClass(x1, y1, x2, y2), COLOR_RED);
	PTextRenderer->Set_Location(Vector2(2, YPosition));
	PTextRenderer->Draw_Text(label);

	//
	// Bandwidth used (text)
	//
	StringClass text;
   text.Format("%d", bps);
	PTextRenderer->Set_Location(Vector2(200, YPosition));
	PTextRenderer->Draw_Text(text);

	//
	// Bandwidth budgeted (text)
	//
	if (target_bps >= 0)
	{
		StringClass text;
   	text.Format("%d", target_bps);
		PTextRenderer->Set_Location(Vector2(245, YPosition));
		PTextRenderer->Draw_Text(text);
		float target_x = target_bps / (float) BandwidthScaler * BarWidth;
		PTextRenderer->Draw_Block(RectClass(target_x - 1, y1, target_x + 1, y2));
	}

	//
	// Bandwidth multiplier and average object priority
	//
	if (bandwidth_multiplier >= 0.0f)
	{
		//
		// Graph mark
		//
		//float tp = threshold_priority * Render2DClass::Get_Screen_Resolution().Width();
		//PTextRenderer->Draw_Block(RectClass(tp - 1, y1, tp + 1, y2), COLOR_GREEN);

		//
		// Text
		//
		// Bandwidth multiplier
		if (is_loading) {
			text.Format("BM:%-8.6f Ld", bandwidth_multiplier);
		} else {
			text.Format("BM:%-8.6f", bandwidth_multiplier);
		}
		PTextRenderer->Set_Location(Vector2(400, YPosition));
		PTextRenderer->Draw_Text(text);

		// Average object priority on this client
		text.Format("AP:%-8.6f", average_priority);
		PTextRenderer->Set_Location(Vector2(500, YPosition));
		PTextRenderer->Draw_Text(text);

	}

	YPosition += YIncrement;
}

//-----------------------------------------------------------------------------
void
cBandwidthGraph::Think
(
	void
)
{
	bool bail = true;

#ifdef WWDEBUG
	if (cDevOptions::ShowBandwidth.Is_True())
	{
		bail = false;
	}
#endif // WWDEBUG

	if (bail)
	{
		return;
	}

	PacketManager.Update_Stats();

	if (PTextRenderer == NULL) {
		return;
	}

	WWASSERT(PTextRenderer != NULL);

	PTextRenderer->Reset();

	//
	// Draw the scale at the top
	//
	YPosition = 50;

	float x1 = 0;
	float x2 = 0;
	float y1 = YPosition;
	float y2 = y1 + BarHeight;
	int i		= 0;

	do {
		x2 = x1 + BarWidth;
		PTextRenderer->Draw_Block(RectClass(x1, y1, x2, y2), (i % 2) ? COLOR_WHITE : COLOR_BLACK);
		x1 = x2;
		i++;
	} while (x2 < Render2DClass::Get_Screen_Resolution().Width());

   //
	// Label the scale
	//
	StringClass text;
   text.Format("BW / %d", BandwidthScaler);
	PTextRenderer->Set_Location(Vector2(2, YPosition - 10));
	PTextRenderer->Draw_Text(text);

	YPosition += YIncrement;

	DWORD bps = 0;
	DWORD target_bps = 0;

	//
	// Client bandwidth
	//
	if (cNetwork::I_Am_Client() && cNetwork::PClientConnection->Is_Established())
	{
		text.Format("c%d->s", cNetwork::Get_My_Id());
		bps = PacketManager.Get_Compressed_Bandwidth_Out(&cNetwork::Get_Client_Rhost()->Get_Address());
		target_bps = cNetwork::Get_Client_Rhost()->Get_Target_Bps();
		Bandwidth_Graph(text, bps, target_bps, -1.0f, -1.0f, false);	//cNetwork::Get_Client_Rhost()->Get_Threshold_Priority());

		text.Format("c%d<-s", cNetwork::Get_My_Id());
		bps = PacketManager.Get_Compressed_Bandwidth_In(&cNetwork::Get_Client_Rhost()->Get_Address());
		Bandwidth_Graph(text, bps, -1, -1, -1.0f, false);
	}

	//
	// Server bandwidth
	//
	if (cNetwork::PServerConnection != NULL)
	{
		text.Format("s->c*");
		bps = PacketManager.Get_Total_Compressed_Bandwidth_Out();
		target_bps = cNetwork::PServerConnection->Get_Bandwidth_Budget_Out();
		Bandwidth_Graph(text, bps, target_bps, -1, -1.0f, false);

		text.Format("s<-c*");
		bps = PacketManager.Get_Total_Compressed_Bandwidth_In();
		Bandwidth_Graph(text, bps, -1, -1, -1.0f, false);

	   for (int i = cNetwork::PServerConnection->Get_Min_RHost(); i <= cNetwork::PServerConnection->Get_Max_RHost(); i++)
		{
			cRemoteHost * p_rhost = cNetwork::Get_Server_Rhost(i);

			if (p_rhost != NULL)
			{
				bool is_loading = p_rhost->Get_Flood();
				text.Format("s->c%d", i);
				bps = PacketManager.Get_Compressed_Bandwidth_Out(&p_rhost->Get_Address());
				target_bps = p_rhost->Get_Target_Bps();
				Bandwidth_Graph(text, bps, target_bps, p_rhost->Get_Bandwidth_Multiplier(), p_rhost->Get_Average_Priority(), is_loading);

				text.Format("s<-c%d", i);
				bps = PacketManager.Get_Compressed_Bandwidth_In(&p_rhost->Get_Address());
				Bandwidth_Graph(text, bps, -1, -1, -1.0f, false);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void
cBandwidthGraph::Render
(
	void
)
{
	bool bail = true;

#ifdef WWDEBUG
	if (cDevOptions::ShowBandwidth.Is_True())
	{
		bail = false;
	}
#endif // WWDEBUG

	if (bail)
	{
		return;
	}

	if (PTextRenderer == NULL) {
		return;
	}

	WWASSERT(PTextRenderer != NULL);
	PTextRenderer->Render();
}