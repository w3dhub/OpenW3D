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

//
// Filename:     nicenum.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1999
// Description:  
//

#include "nicenum.h"

#include "network-typedefs.h"
#include <stdio.h>

#include "wwdebug.h"
#include "netutil.h"
#include "useroptions.h"
#include "GameSpy_QnR.h"

//
// Class statics
//
unsigned int		cNicEnum::NicList[];
unsigned short	cNicEnum::NumNics			= 0;
unsigned int		cNicEnum::GSNicList[];	
unsigned short	cNicEnum::NumGSNics		= 0;

//----------------------------------------------------------------------------------
void 
cNicEnum::Init
(
	void
)
{
	WWDEBUG_SAY(("cNicEnum::Init\n"));

	WSADATA wsa_data;
	int startup_rc = ::WSAStartup(MAKEWORD(1, 1), &wsa_data);
	if (startup_rc != 0) 
	{
		WWDEBUG_SAY(("  WSAStartup failed!\n"));
		return;
	}

	//
	// Retrieve list of nic's
	//
	unsigned int local_addresses[MAX_NICS];
	unsigned int num_addresses = Enumerate_Nics(local_addresses, MAX_NICS);
	WWASSERT(num_addresses <= MAX_NICS);

	WWDEBUG_SAY(("  Found %d NIC(s)\n", num_addresses));

	unsigned short index	= 0;
	unsigned short class_1	= 0;
	unsigned short class_2	= 0;

	//
	// First, extract the non-internet addressable nicks, ordered on general usage.
	//
	unsigned int lan_addresses[MAX_NICS];
	unsigned int num_lan_addresses = 0;

	//
	// First, scan for 10.*.*.* addresses
	//
	for (index = 0; index < num_addresses; index++)
	{
		class_1 = (::ntohl(local_addresses[index]) & 0xff000000) >> 24;

		if (class_1 == 10)
		{
			lan_addresses[num_lan_addresses++] = local_addresses[index];
			local_addresses[index] = 0;
		}
	}

	//
	// Next, scan for 192.168.*.* addresses
	//
	for (index = 0; index < num_addresses; index++) 
	{
		class_1 = (::ntohl(local_addresses[index]) & 0xff000000) >> 24;
		class_2 = (::ntohl(local_addresses[index]) & 0x00ff0000) >> 16;
		
		if (class_1 == 192 && class_2 == 168)
		{
			lan_addresses[num_lan_addresses++] = local_addresses[index];
			local_addresses[index] = 0;
		}
	}

	//
	// Next, scan for 172.16-31.*.* addresses
	//
	for (index = 0; index < num_addresses; index++) 
	{
		class_1 = (::ntohl(local_addresses[index]) & 0xff000000) >> 24;
		class_2 = (::ntohl(local_addresses[index]) & 0x00ff0000) >> 16;
		
		if (class_1 == 172 && class_2 >= 16 && class_2 <= 31)
		{
			lan_addresses[num_lan_addresses++] = local_addresses[index];
			local_addresses[index] = 0;
		}
	}

	//
	// Finally, scan for 169.254.*.* addresses (IP autoconfiguration)
	//
	for (index = 0; index < num_addresses; index++) 
	{
		class_1 = (::ntohl(local_addresses[index]) & 0xff000000) >> 24;
		class_2 = (::ntohl(local_addresses[index]) & 0x00ff0000) >> 16;
		
		if (class_1 == 169 && class_2 == 254)
		{
			lan_addresses[num_lan_addresses++] = local_addresses[index];
			local_addresses[index] = 0;
		}
	}

	WWDEBUG_SAY(("  Of which %d are non-internet addressable.\n", num_lan_addresses));

	//
	// Next, copy the Internet addressable addresses. Weed out localhost and multicast 
	// addresses.
	//

	unsigned int internet_addresses[MAX_NICS];
	unsigned int num_internet_addresses = 0;

	for (index = 0; index < num_addresses; index++)
	{
		if (local_addresses[index] != 0)
		{
			class_1 = (::ntohl(local_addresses[index]) & 0xff000000) >> 24;
			
			if (class_1 != 127 && class_1 != 224)
			{
				internet_addresses[num_internet_addresses++] = local_addresses[index];
				local_addresses[index] = 0;
			}
		}
	}

	//
	// Now build the LAN and GameSpy NIC lists. They contain the same entries.
	// The only difference is that the LAN list puts the non-internet addressable 
	// NIC's first, the GameSpy list puts them last.
	//
	NumNics			= 0;
	NumGSNics 		= 0;

	for (index = 0; index < num_lan_addresses; index++)
	{
		NicList[NumNics++] = lan_addresses[index];
	}

	for (index = 0; index < num_internet_addresses; index++)
	{
		NicList[NumNics++] = internet_addresses[index];
	}

	for (index = 0; index < num_internet_addresses; index++)
	{
		GSNicList[NumGSNics++] = internet_addresses[index];
	}

	for (index = 0; index < num_lan_addresses; index++)
	{
		GSNicList[NumGSNics++] = lan_addresses[index];
	}

	WWASSERT(NumNics == NumGSNics);

	//
	// Initialize or update PreferredLanNic if required.
	//
	bool is_nic_valid = false;
	for (index = 0; index < NumNics; index++) 
	{
		if ((unsigned int) cUserOptions::PreferredLanNic.Get() == NicList[index])
		{
			is_nic_valid = true;
			break;
		}
	}

	if (!is_nic_valid)
	{
		if (NumNics > 0) 
		{
			cUserOptions::PreferredLanNic.Set(NicList[0]);
		}
		else
		{
			cUserOptions::PreferredLanNic.Set(0);
		}
	}

	//
	// Initialize or update PreferredGameSpyNic if required.
	//
	is_nic_valid = false;
	for (index = 0; index < NumGSNics; index++) 
	{
		if ((unsigned int) cUserOptions::PreferredGameSpyNic.Get() == GSNicList[index])
		{
			is_nic_valid = true;
			break;
		}
	}

	if (!is_nic_valid)
	{
		if (NumGSNics > 0) 
		{
			cUserOptions::PreferredGameSpyNic.Set(GSNicList[0]);
		}
		else
		{
			cUserOptions::PreferredGameSpyNic.Set(0);
		}
	}

	WWDEBUG_SAY(("  PreferredLanNic is %u (%s)\n", 
		(unsigned int) cUserOptions::PreferredLanNic.Get(), 
		cNetUtil::Address_To_String(cUserOptions::PreferredLanNic.Get())));

	WWDEBUG_SAY(("  PreferredGameSpyNic is %u (%s)\n", 
		(unsigned int) cUserOptions::PreferredGameSpyNic.Get(), 
		cNetUtil::Address_To_String(cUserOptions::PreferredGameSpyNic.Get())));

	[[maybe_unused]] int cleanup_rc = ::WSACleanup();
	WWASSERT(cleanup_rc != SOCKET_ERROR);
}

//---------------------------------------------------------------------------
unsigned int 
cNicEnum::Enumerate_Nics
(
	unsigned int *	addresses, 
	unsigned int		max_nics
)
{
	WWASSERT(addresses != NULL);
	WWASSERT(max_nics > 0);

	WWDEBUG_SAY(("cNicEnum::Enumerate_Nics\n"));

	//
	// Get the local hostname
	//
	char local_host_name[300];
	[[maybe_unused]] int gethostname_rc = ::gethostname(local_host_name, sizeof(local_host_name));
	WWASSERT(gethostname_rc != SOCKET_ERROR);

	//
	// Resolve hostname for local adapter addresses. 
	// This does a DNS lookup (name resolution)
	//
	LPHOSTENT p_hostent = ::gethostbyname(local_host_name);
	if (p_hostent == NULL) 
	{
		DIE;
	}

	unsigned int num_addresses = 0;
	while (num_addresses < max_nics && p_hostent->h_addr_list[num_addresses] != NULL) 
	{
		IN_ADDR in_addr;
		::memcpy(&in_addr, p_hostent->h_addr_list[num_addresses], sizeof(in_addr));
		addresses[num_addresses] = in_addr.s_addr;
		WWDEBUG_SAY(("  NIC: %s\n", cNetUtil::Address_To_String(addresses[num_addresses])));
		num_addresses++;
	}

	return num_addresses;
}






























/*
void 
cNicEnum::Init
(
	void
)
{
	WWDEBUG_SAY(("cNicEnum::Init\n"));

	WSADATA wsa_data;
	int startup_rc = ::WSAStartup(MAKEWORD(1, 1), &wsa_data);
	if (startup_rc != 0) 
	{
		WWDEBUG_SAY(("  WSAStartup failed!\n"));
		return;
	}

	//
	// Retrieve list of nic's
	//
	unsigned int local_addresses[MAX_NICS];
	unsigned int num_addresses = Enumerate_Nics(local_addresses, MAX_NICS);
	WWASSERT(num_addresses <= MAX_NICS);

	WWDEBUG_SAY(("  Found %d NIC(s)\n", num_addresses));

	//
	// We are going to discard anything that is internet addressable.
	// The non-internet-addressable NIC's are:
	//   10.*.*.*
	//   192.168.*.*
	//   172.16-31.*.*
	// We will order these as above to promote the more common choice.
	//

	unsigned short index	= 0;
	unsigned short class_1	= 0;
	unsigned short class_2	= 0;
	NumNics			= 0;
	NumGSNics 		= 0;

	//
	// Create a seperate list of GameSpy compatible NIC's that includes
	// local and internet addressable
	//
	// Don't add the following interfaces to the list.
	//   127.* (localhost)
	//   224.* (multicast)
	//
	for (index = 0; index < num_addresses; index++)
	{
		class_1 = (::ntohl(local_addresses[index]) & 0xff000000) >> 24;
		
		if (class_1 != 127 && class_1 != 224)
		{
			GSNicList[NumGSNics++] = local_addresses[index];
		}
	}

	//
	// First, scan for 10.*.*.* addresses
	//
	for (index = 0; index < num_addresses; index++)
	{
		class_1 = (::ntohl(local_addresses[index]) & 0xff000000) >> 24;

		if (class_1 == 10)
		{
			NicList[NumNics++] = local_addresses[index];
		}
	}

	//
	// Next, scan for 192.168.*.* addresses
	//
	for (index = 0; index < num_addresses; index++) 
	{
		class_1 = (::ntohl(local_addresses[index]) & 0xff000000) >> 24;
		class_2 = (::ntohl(local_addresses[index]) & 0x00ff0000) >> 16;
		
		if (class_1 == 192 && class_2 == 168)
		{
			NicList[NumNics++] = local_addresses[index];
		}
	}

	//
	// Finally, scan for 172.16-31.*.* addresses
	//
	for (index = 0; index < num_addresses; index++) 
	{
		class_1 = (::ntohl(local_addresses[index]) & 0xff000000) >> 24;
		class_2 = (::ntohl(local_addresses[index]) & 0x00ff0000) >> 16;
		
		if (class_1 == 172 && class_2 >= 16 && class_2 <= 31)
		{
			NicList[NumNics++] = local_addresses[index];
		}
	}

	WWDEBUG_SAY(("  Of which %d are non-internet addressable.\n", NumNics));

	//
	// Initialize or update PreferredLanNic if required.
	//
	bool is_nic_valid = false;
	for (index = 0; index < NumNics; index++) 
	{
		if ((unsigned int) cUserOptions::PreferredLanNic.Get() == NicList[index])
		{
			is_nic_valid = true;
			break;
		}
	}

	if (!is_nic_valid)
	{
		if (NumNics > 0) 
		{
			cUserOptions::PreferredLanNic.Set(NicList[0]);
		}
		else
		{
			cUserOptions::PreferredLanNic.Set(0);
		}
	}

	is_nic_valid = false;
	for (index = 0; index < NumGSNics; index++) 
	{
		if ((unsigned int) cUserOptions::PreferredGameSpyNic.Get() == GSNicList[index])
		{
			is_nic_valid = true;
			break;
		}
	}

	if (!is_nic_valid)
	{
		if (NumGSNics > 0) 
		{
			cUserOptions::PreferredGameSpyNic.Set(GSNicList[0]);
		}
		else
		{
			cUserOptions::PreferredGameSpyNic.Set(0);
		}
	}

	WWDEBUG_SAY(("  PreferredLanNic is %u (%s)\n", 
		(unsigned int) cUserOptions::PreferredLanNic.Get(), 
		cNetUtil::Address_To_String(cUserOptions::PreferredLanNic.Get())));

	WWDEBUG_SAY(("  PreferredGameSpyNic is %u (%s)\n", 
		(unsigned int) cUserOptions::PreferredGameSpyNic.Get(), 
		cNetUtil::Address_To_String(cUserOptions::PreferredGameSpyNic.Get())));

	int cleanup_rc = ::WSACleanup();
	WWASSERT(cleanup_rc != SOCKET_ERROR);
}
*/

