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

/****************************************************************************\
*        C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S         *
******************************************************************************
Project Name: Generic Game Results Server
File Name   : wencrypt.h
Author      : Joe Howes
Start Date  : Jul 9, 1999
Last Update :
------------------------------------------------------------------------------
 
A simple encryption system for game results packets.  Specifically designed
to work only with unsigned char buffers of least 15 bytes long.
 
\****************************************************************************/


#ifndef __WENCRYPT_H__
#define __WENCRYPT_H__


/*------------------------------------------------------------------------------.
| BUILD TYPE									|
| In the interest of smaller code and distributing as little of our		|
| encryption system as possible with game CDs, this preprocessor directive	|
| should be uncommented when building a client.  When uncommented, only those	|
| functions relevant to encrypting will be compiled, and those relevant only	|
| to decrypting will not.							|
`------------------------------------------------------------------------------*/
#define __CLIENT__


/*------------------------------------------------------------------------------.
| PROTOTYPES									|
`------------------------------------------------------------------------------*/

#ifndef __CLIENT__
unsigned char*	DecryptPacket(unsigned char* encbuf, int* size);
#endif

// These are for munching the data
unsigned char* PrepareEncryptedPacket(unsigned char* packet, int* size);
void 		RLEncode(unsigned char* src, unsigned char* dst, int* size);
float 		ran3(int* idnum);
void 		GenerateKey(unsigned char* key, int len, int seed);
unsigned char* 	SimpleEncrypt(const unsigned char* src, int* len);
#ifndef __CLIENT__
unsigned char* 	RLDecode(unsigned char* src, int* size);
unsigned char*	SimpleDecrypt(unsigned char* buf, int* len);
#endif


/*------------------------------------------------------------------------------.
| MACROS									|
`------------------------------------------------------------------------------*/
// For key generation
#define MBIG    1000000000
#define MSEED   161803398
#define MZ      0
#define FAC     (1.0/MBIG)
 
// Errors
#define ERR_SPURIOUS_ESCAPE     -1      // A single escape was found at the end of a compressed
                                        // buffer.  Should never happen as escapes in the
                                        // stream are stuffed and should otherwise indicate
                                        // the start of a token.
 
#define ERR_BUFFER_TOO_SMALL    -2      // The buffer being passed to Simple Encrypt is
                                        // smaller than 15 bytes, which is too small to work
                                        // with it's seed-stuffing.  A gameres packet should
                                        // never be as small as 15 bytes anyway.

#define ERR_BAD_SEED		-3      // The seed value being passed is zero.  Not a valid
					// seed.



#endif

