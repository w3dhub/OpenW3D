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
 *                     $Archive:: /Commando/Code/Commando/nataddr.h                           $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 9/17/01 4:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#pragma once

#ifndef NATADDR_H
#define NATADDR_H

#include "always.h"

#ifdef WWASSERT
#ifndef fw_assert
#define fw_assert WWASSERT
#endif //fw_assert
#else //WWASSERT
#define fw_assert assert
#endif //WWASSERT

/*
** Class to manage IP addresses for the NAT stuff only.
*/
class IPAddressClass
{
	/*
	** Public Interface
	*/
	public:
		/*
		**	Constructors:
		*/
		IPAddressClass(void);
		IPAddressClass(unsigned char *address, unsigned short port = 0);
		IPAddressClass(unsigned int address, unsigned short port = 0);

		/*
		**	Set the address from explicit variables.
		*/
		void Set_Address(unsigned char *address, unsigned short port = 0);
		void Set_Address(unsigned int address, unsigned short port = 0);
		void Set_Port(unsigned short port) {Port = port;};

		/*
		**	Get the address values explicitly.
		*/
		void Get_Address(unsigned char *address, unsigned short *port = 0);
		unsigned int Get_Address(void);
		unsigned short Get_Port(void);

		/*
		**	Tells if this address is a broadcast address
		*/
		bool Is_Broadcast(void);

		/*
		** Is this address valid?
		*/
		bool Is_Valid(void) {return(IsValid);};

		/*
		** Convert address to human readable string
		*/
		char *As_String(void);

		/*
		** Compare IPs only, not ports.
		*/
		bool Is_IP_Equal(IPAddressClass &address);

		/*
		**	Overloaded operators:
		*/
		bool operator =  (const IPAddressClass &address);
		bool operator == (IPAddressClass &address);
		bool operator != (IPAddressClass &address);
		bool operator == (IPAddressClass address);
		bool operator != (IPAddressClass address);



	/*
	**	Private Interface
	*/
	private:
		/*
		** Actual xxx.xxx.xxx.xxx IP address.
		*/
		union  {
			unsigned int WholeAddress;
			unsigned char Address[4];
		};

		/*
		** Port number associated with this address.
		*/
		unsigned short Port;

		/*
		** Are the contents of the address array valid?
		*/
		bool IsValid;
};



#endif