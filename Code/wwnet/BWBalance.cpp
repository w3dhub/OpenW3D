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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwnet/BWBalance.cpp                          $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/13/02 9:40p                                              $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <always.h>
#include "BWBalance.h"
#include "connect.h"
#include "packetmgr.h"
#include "systimer.h"

BandwidthBalancerClass BandwidthBalancer;

/*
** 8000 bits per second is about as low as we want to go.
*/
#define MIN_ACCEPTABLE_BANDWIDTH	8000



/***********************************************************************************************
 * BandwidthBalancerClass::BandwidthBalancerClass -- Class constructor                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/21/2001 8:47PM ST : Created                                                            *
 *=============================================================================================*/
BandwidthBalancerClass::BandwidthBalancerClass(void)
{
	NumClientStructs = 0;
	ClientInfo = NULL;

	Allocate_Client_Structs(8);

	IsEnabled = true;
}



/***********************************************************************************************
 * BandwidthBalancerClass::~BandwidthBalancerClass -- Class destructor                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/21/2001 8:47PM ST : Created                                                            *
 *=============================================================================================*/
BandwidthBalancerClass::~BandwidthBalancerClass(void)
{
	if (ClientInfo) {
		delete [] ClientInfo;
	}
	ClientInfo = NULL;
}



/***********************************************************************************************
 * BandwidthBalancerClass::Allocate_Client_Structs -- Make room to store per client info       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Likely number of clients                                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/21/2001 8:47PM ST : Created                                                            *
 *=============================================================================================*/
void BandwidthBalancerClass::Allocate_Client_Structs(int num_structs)
{
	if (num_structs > NumClientStructs) {
		num_structs += 15;
		num_structs &= 0xfffffff0;

		if (ClientInfo) {
			delete [] ClientInfo;
		}

		ClientInfo = new ClientInfoStruct[num_structs];
		NumClientStructs = num_structs;
	}

}




/***********************************************************************************************
 * BandwidthBalancerClass::Adjust -- Adjust bandwidth to each client                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/21/2001 8:48PM ST : Created                                                            *
 *=============================================================================================*/
void BandwidthBalancerClass::Adjust(cConnection *connection, bool is_dedicated)
{
	static unsigned long _last_adjustment = 0;	//TIMEGETTIME();

	/*
	**
	** Get average average priority.
	**
	** Do a pass and assign bandwidth to clients based on priority. Boost bandwidth to players currently loading. Clip bandwidth to
	** clients max bps (and +- 50%) and keep a total of spare bandwidth.
	**
	** Repeat until no bandwidth left.
	**
	*/
	if (connection != NULL) {

		bool need_update = false;
		for (int i=connection->Get_Min_RHost() + (is_dedicated ? 0 : 1) ; i <= connection->Get_Max_RHost() ; i++) {
			cRemoteHost *client = connection->Get_Remote_Host(i);
			if (client && client->Get_Target_Bps() == 0) {
				need_update = true;
				break;
			}
		}


		if (need_update || TIMEGETTIME() - _last_adjustment > 100) {
			_last_adjustment = TIMEGETTIME();

			Adjust_Connection_Budget(connection);

			int num_remote_hosts = connection->Get_Num_RHosts();
			if (!is_dedicated) {
				num_remote_hosts--;
			}

			if (num_remote_hosts > NumClientStructs) {
				Allocate_Client_Structs(num_remote_hosts);
			}
			WWASSERT(NumClientStructs >= num_remote_hosts);
			NumClients = 0;

			/*
			** Copy all the info we need into a single place.
			*/
			float average_priority = 0;
			for (int i=connection->Get_Min_RHost() + (is_dedicated ? 0 : 1) ; i <= connection->Get_Max_RHost() ; i++) {

				cRemoteHost *client = connection->Get_Remote_Host(i);

				if (client != NULL) {

					ClientInfo[NumClients].AveragePriority = client->Get_Average_Priority();
					ClientInfo[NumClients].MaxBpsDown = client->Get_Maximum_Bps();
					ClientInfo[NumClients].AllocatedBBO = client->Get_Target_Bps();
					ClientInfo[NumClients].ID = client->Get_Id();
					ClientInfo[NumClients].IsLoading = client->Get_Is_Loading();
					ClientInfo[NumClients].IsDone = false;

					/*
					** Track the overally average priority.
					*/
					average_priority += ClientInfo[NumClients].AveragePriority;
					NumClients++;
				}
			}

			WWASSERT(NumClients == num_remote_hosts);

			if (NumClients) {
				/*
				** Get the average object priority among all players.
				*/
				average_priority = average_priority / NumClients;
				int bw_adjust = 100;

				unsigned long total_bbo_allocated = Allocate_Bandwidth(average_priority, bw_adjust, connection->Get_Bandwidth_Budget_Out());

				/*
				** Ok, we have a rough bandwidth allocation. If we allocated too much or too little then we will need to do
				** some fine tuning.
				*/
				unsigned long total_bbo = connection->Get_Bandwidth_Budget_Out();
				int diff = total_bbo - total_bbo_allocated;
				int percent_diff = (100 * abs(diff)) / total_bbo;
				diff = total_bbo - total_bbo_allocated;
				bw_adjust = 100 + ((100 * diff) / (((int)total_bbo * 9) / 10));

				if (bw_adjust) {

					/*
					** If we allocated more or less than we have then keep adjusting until it comes out at about 95%.
					*/
					int tries = 5;
					while ((diff < 0 || percent_diff > 10) && tries >= 0) {
						//WWDEBUG_SAY(("Allocated too much or too little bandwidth. total_bbo = %d, total_bbo_allocated = %d, bw_adjust = %d\n", total_bbo, total_bbo_allocated, bw_adjust));
						total_bbo_allocated = Allocate_Bandwidth(average_priority, bw_adjust, connection->Get_Bandwidth_Budget_Out());
						diff = total_bbo - total_bbo_allocated;
						percent_diff = (100 * abs(diff)) / total_bbo;
						bw_adjust = bw_adjust * (100 + ((100 * diff) / (((int)total_bbo * 9) / 10))) / 100;
						if (bw_adjust <= 0) {
							break;
						}
						tries--;
					}

					/*
					** If we get to this point, there should still be bandwidth left.
					*/
					if (diff < 0 && percent_diff > 3) {
						WWDEBUG_SAY(("***** WARNING - BandwidthBalancer: Insufficient bandwidth for the number of clients in the game *********\n"));
						WWDEBUG_SAY(("Allocated too much bandwidth. total_bbo = %d, total_bbo_allocated = %d, bw_adjust = %d\n", total_bbo, total_bbo_allocated, bw_adjust));
					}

					/*
					** Oh well, let's assume that's right. Set the bandwidth budgets back into the rhosts.
					*/
					int index = 0;
					for (int i=connection->Get_Min_RHost() + (is_dedicated ? 0 : 1) ; i <= connection->Get_Max_RHost() ; i++) {
						cRemoteHost *client = connection->Get_Remote_Host(i);
						if (client != NULL) {
							client->Set_Target_Bps(ClientInfo[index++].AllocatedBBO);
						}
					}
				}
			}
		}
	}
}



/***********************************************************************************************
 * BandwidthBalancerClass::Allocate_Bandwidth -- Split bandwidth between clients               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Average object priority for all clients                                           *
 *                                                                                             *
 *                                                                                             *
 * OUTPUT:   Percentage to apply to final bandwidth numbers                                    *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/21/2001 8:49PM ST : Created                                                            *
 *=============================================================================================*/
unsigned long BandwidthBalancerClass::Allocate_Bandwidth(float average_priority, int bw_adjust, unsigned long total_server_bbo)
{
	WWASSERT(bw_adjust != 0);
	WWASSERT(average_priority >= 0.0f);
	WWASSERT(average_priority <= 1.0f);

	/*
	** Get our total bandwidth budget out. This has to be split between all clients.
	*/
	unsigned long total_bbo = total_server_bbo;
	unsigned long bbo_per_client = total_bbo / NumClients;
	unsigned long total_bbo_allocated = 0;

	/*
	** Loop through and assign bandwidth based on average priority and other metrics.
	*/
	for (int i=0 ; i<NumClients ; i++) {

		ClientInfoStruct *client = &ClientInfo[i];
		float pri = client->AveragePriority;

		/*
		** Work out the bandwidth, initially based on average priority.
		*/
		int client_bbo_adjust = (pri - average_priority) * bbo_per_client;
		if (client_bbo_adjust > 0) {
			client_bbo_adjust = min(client_bbo_adjust, (int)bbo_per_client / 2);
		} else {
			client_bbo_adjust = max(client_bbo_adjust, -((int)bbo_per_client / 2));
		}
		int new_client_bbo = bbo_per_client + client_bbo_adjust;
		WWASSERT(new_client_bbo > 0);

		/*
		** Boost the bandwidth if the client is loading. He doesn't need it now but there will be a huge demand for
		** bandwidth as soon as the load is done so it's good to free some up now.
		*/
		if (client->IsLoading) {
			new_client_bbo = new_client_bbo << 1;
		}

		/*
		** Apply the overall percentage adjustment.
		*/
		double big_client_bbo = (double) new_client_bbo;
		double big_bw_adjust = (double) bw_adjust;
		big_client_bbo = (big_client_bbo * big_bw_adjust) / 100.0;
		big_client_bbo = min(big_client_bbo, (double) 0x7fffffff);
		//new_client_bbo = (new_client_bbo * bw_adjust) / 100;
		new_client_bbo = (int) big_client_bbo;

		/*
		** OK, we have the bandwidth adjusted according to priority.
		** Clamp it to the min allowed and the max the client can take.
		*/
		new_client_bbo = min(new_client_bbo, (int)client->MaxBpsDown);
		new_client_bbo = max(new_client_bbo, MIN_ACCEPTABLE_BANDWIDTH);

		client->AllocatedBBO = (unsigned long) new_client_bbo;
		total_bbo_allocated += (unsigned long) new_client_bbo;
	}

	return(total_bbo_allocated);
}





/***********************************************************************************************
 * BandwidthBalancerClass::Adjust_Connection_Budget -- Adjust BBO of server connection         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to server connection                                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/24/2001 1:56PM ST : Created                                                            *
 *=============================================================================================*/
void BandwidthBalancerClass::Adjust_Connection_Budget(cConnection *connection)
{
	static unsigned long _last_time = 0;
	unsigned long time = TIMEGETTIME();

	/*
	** Check every n seconds and see if we had any send errors that would be caused by sending too much data.
	*/
	if (PacketManager.Get_Error_State() == PacketManagerClass::STATE_WS_BUFFERS_FULL) {
		if (time - _last_time > 10000) {
			_last_time = time;
			ULONG bbo = connection->Get_Bandwidth_Budget_Out();
			ULONG new_bbo = (bbo * 9) / 10;
			connection->Set_Bandwidth_Budget_Out(new_bbo);
			WWDEBUG_SAY(("*** WARNING BandwidthBalancerClass - Adjusting Server connection BBO from %d to %d due to send overflow ***\n", bbo, new_bbo));
		}
	}
}
