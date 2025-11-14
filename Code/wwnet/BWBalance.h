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
 *                     $Archive:: /Commando/Code/wwnet/BWBalance.h                            $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/24/01 1:58p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once
#ifndef _BWBALANCE_H

class cConnection;

/*
** This class balances bandwidth between clients.
**
**
*/
class BandwidthBalancerClass
{
	public:

		/*
		** Constructor, desturctor.
		*/
		BandwidthBalancerClass(void);
		~BandwidthBalancerClass(void);

		/*
		** Public function.
		*/
		void Adjust(cConnection *connection, bool is_dedicated);

		bool IsEnabled;

	private:

		unsigned int Allocate_Bandwidth(float average_priority, int bw_adjust, unsigned int total_server_bbo);
		void Allocate_Client_Structs(int num_structs);
		void Adjust_Connection_Budget(cConnection *connection);


		/*
		** Struct to keep info about each client.
		*/
		struct ClientInfoStruct {
			float AveragePriority;
			unsigned int MaxBpsDown;
			unsigned int AllocatedBBO;
			bool IsLoading;
			unsigned int ID;
			bool IsDone;
		};

		/*
		** Pointer to array of client info structs.
		*/
		ClientInfoStruct *ClientInfo;

		/*
		** Number of client info structs.
		*/
		int NumClientStructs;

		/*
		** Number of clients to balance between.
		*/
		int NumClients;
};



extern BandwidthBalancerClass BandwidthBalancer;



#endif //_BWBALANCE_H