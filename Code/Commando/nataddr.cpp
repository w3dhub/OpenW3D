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
 *                     $Archive:: /Commando/Code/Commando/nataddr.cpp                         $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 9/17/01 3:55p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * IPAddressClass::IPAddressClass -- Class constructor                                         *
 * IPAddressClass::IPAddressClass -- Class constructor                                         *
 * IPAddressClass::IPAddressClass -- Class constructor                                         *
 * IPAddressClass::Set_Address -- Sets a valid IP address into the class                       *
 * IPAddressClass::Set_Address -- Sets a valid IP address into the class                       *
 * IPAddressClass::Get_Address -- Get the address value of this class                          *
 * IPAddressClass::Get_Address -- Get the address value of this class                          *
 * IPAddressClass::Is_Broadcast -- Does this class represent a broadcast address?              *
 * IPAddressClass::As_String -- Get address in human readable form                             *
 * IPAddressClass::operator == - Equality operator for the class                               *
 * IPAddressClass::operator != - Non-equality operator for the class                           *
 * IPAddressClass::operator = Assignment operator                                              *
 * IPXAddressClass::As_String -- Get address in human readable form                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"assert.h"
#include	"nataddr.h"

#include	<stdio.h>
#include	<string.h>



/***********************************************************************************************
 * IPAddressClass::IPAddressClass -- Class constructor                                         *
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
 *   3/9/00 12:58PM ST : Created                                                               *
 *=============================================================================================*/
IPAddressClass::IPAddressClass(void)
{
	WholeAddress = 0x00000000;
	Port = 0;
	IsValid = false;
}



/***********************************************************************************************
 * IPAddressClass::IPAddressClass -- Class constructor                                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to valid IP address                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 12:59PM ST : Created                                                               *
 *=============================================================================================*/
IPAddressClass::IPAddressClass(unsigned char *address, unsigned short port)
{
	WholeAddress = *((unsigned int*)address);
	Port = port;
	IsValid = true;
}


/***********************************************************************************************
 * IPAddressClass::IPAddressClass -- Class constructor                                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    32 bit IP address                                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:00PM ST : Created                                                                *
 *=============================================================================================*/
IPAddressClass::IPAddressClass(unsigned int address, unsigned short port)
{
	WholeAddress = address;
	Port = port;
	IsValid = true;
}




/***********************************************************************************************
 * IPAddressClass::Set_Address -- Sets a valid IP address into the class                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to IP address                                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:01PM ST : Created                                                                *
 *=============================================================================================*/
void IPAddressClass::Set_Address(unsigned char *address, unsigned short port)
{
	WholeAddress = *((unsigned int*)address);
	Port = port;
	IsValid = true;
}



/***********************************************************************************************
 * IPAddressClass::Set_Address -- Sets a valid IP address into the class                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    32 bit IP address                                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:01PM ST : Created                                                                *
 *=============================================================================================*/
void IPAddressClass::Set_Address(unsigned int address, unsigned short port)
{
	WholeAddress = address;
	Port = port;
	IsValid = true;
}



/***********************************************************************************************
 * IPAddressClass::Get_Address -- Get the address value of this class                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to area to store the address into                                             *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:03PM ST : Created                                                                *
 *=============================================================================================*/
void IPAddressClass::Get_Address(unsigned char *address, unsigned short *port)
{
	fw_assert(IsValid);
	*((unsigned int*)address) = WholeAddress;
	if (port) {
		*port = Port;
	}
}




/***********************************************************************************************
 * IPAddressClass::Get_Address -- Get the address value of this class                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   32 bit IP address                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:03PM ST : Created                                                                *
 *=============================================================================================*/
unsigned int IPAddressClass::Get_Address(void)
{
	fw_assert(IsValid);
	return (WholeAddress);
}



/***********************************************************************************************
 * IPAddressClass::Get_Port -- Get the port number associated with this address                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Port number                                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/24/00 11:51AM ST : Created                                                             *
 *=============================================================================================*/
unsigned short IPAddressClass::Get_Port(void)
{
	fw_assert(IsValid);
	return(Port);
}



/***********************************************************************************************
 * IPAddressClass::Is_Broadcast -- Does this class represent a broadcast address?              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   True if broadcast address                                                         *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:06PM ST : Created                                                                *
 *=============================================================================================*/
bool IPAddressClass::Is_Broadcast(void)
{
	if (IsValid && WholeAddress == 0xffffffff) {
		return(true);
	}
	return(false);

}



/***********************************************************************************************
 * IPAddressClass::As_String -- Get address in human readable form                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to address                                                                    *
 *                                                                                             *
 * OUTPUT:   ptr to string                                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:07PM ST : Created                                                                *
 *=============================================================================================*/
char *IPAddressClass::As_String(void)
{
	static char _addr_str[128];
	sprintf (_addr_str, "%d.%d.%d.%d ; %d", Address[0], Address[1], Address[2], Address[3], (unsigned int)Port);
	return (_addr_str);
}



/***********************************************************************************************
 * IPAddressClass::Is_IP_Equal -- Compare the IP portion of the address only, ignore the port  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address to compare to                                                             *
 *                                                                                             *
 * OUTPUT:   True if equal                                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/25/00 2:23PM ST : Created                                                              *
 *=============================================================================================*/
bool IPAddressClass::Is_IP_Equal(IPAddressClass &address)
{
	if (IsValid && address.Is_Valid() && address.Get_Address() == WholeAddress) {
		return(true);
	}
	return(false);

}



/***********************************************************************************************
 * IPAddressClass::operator == - Equality operator for the class                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Class to compare to                                                               *
 *                                                                                             *
 * OUTPUT:   True if the same                                                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:10PM ST : Created                                                                *
 *=============================================================================================*/
bool IPAddressClass::operator == (IPAddressClass &address)
{
	if (IsValid && address.Is_Valid() && address.Get_Address() == WholeAddress && address.Get_Port() == Port) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * IPAddressClass::operator != - Non-equality operator for the class                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Class to compare to                                                               *
 *                                                                                             *
 * OUTPUT:   True if not the same                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:10PM ST : Created                                                                *
 *=============================================================================================*/
bool IPAddressClass::operator != (IPAddressClass &address)
{
	if (!IsValid || !address.Is_Valid() || address.Get_Address() != WholeAddress || address.Get_Port() != Port) {
		return(true);
	}
	return(false);
}

/***********************************************************************************************
 * IPAddressClass::operator == - Equality operator for the class                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Class to compare to                                                               *
 *                                                                                             *
 * OUTPUT:   True if the same                                                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:10PM ST : Created                                                                *
 *=============================================================================================*/
bool IPAddressClass::operator == (IPAddressClass address)
{
	if (IsValid && address.Is_Valid() && address.Get_Address() == WholeAddress && address.Get_Port() == Port) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * IPAddressClass::operator != - Non-equality operator for the class                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Class to compare to                                                               *
 *                                                                                             *
 * OUTPUT:   True if not the same                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:10PM ST : Created                                                                *
 *=============================================================================================*/
bool IPAddressClass::operator != (IPAddressClass address)
{
	if (!IsValid || !address.Is_Valid() || address.Get_Address() != WholeAddress || address.Get_Port() != Port) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * IPAddressClass::operator = Assignment operator                                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Class to store into this class                                                    *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/9/00 1:10PM ST : Created                                                                *
 *=============================================================================================*/
bool IPAddressClass::operator = (const IPAddressClass &address)
{
	IsValid = ((IPAddressClass &)address).Is_Valid();

	/*
	** Can't read an invalid address without causing an assert.
	*/
	if (IsValid) {
		WholeAddress = ((IPAddressClass &)address).Get_Address();
		Port = ((IPAddressClass &)address).Get_Port();
	} else {
		WholeAddress = 0xffffffff;
		Port = 0;
	}
	return(IsValid);
}

