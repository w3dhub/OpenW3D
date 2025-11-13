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
File Name   : wencrypt.cpp
Author      : Joe Howes
Start Date  : Jul 9, 1999
Last Update : Jul 20, 1999
------------------------------------------------------------------------------
 
A simple encryption system for game results packets.  Specifically designed
to work only with unsigned char buffers of least 15 bytes long.
 
\****************************************************************************/
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <stdlib.h>

#include <malloc.h>
#include <winsock2.h>
#include "wencrypt.h"
#include "wnet/packet.h"

/*extern "C" {
	extern unsigned char* PrepareEncryptedPacket(unsigned char* packet, int* size);
}*/
/*------------------------------------------------------------------------------.
| FUNCTION:  PrepareEncryptedPacket()											|
| If an error occurs, a NULL will be returned and the size value will be a		|
| negative integer corresponding to an error in wencrypt.h.						|
| JUL 20/99:  In the Windows WOLAPI, the function RequestGameresSend requires	|
| a buffer that can be passed to the constructor of PacketClass.  Thus we need	|
| to wrap our encrypted buffer in a packet with one big data field and pass		|
| it's Comms Packet output back.												|
`------------------------------------------------------------------------------*/
unsigned char* PrepareEncryptedPacket(unsigned char* packet, int* size)
{
	unsigned char*	rlebuf = NULL;
	unsigned char*	encbuf = NULL;
	PacketClass		wrap;
	static char		field[5] = { "CNTL" };


	rlebuf = (unsigned char *) malloc((*size)*2);


	// Run length encode
	RLEncode(packet, rlebuf, size);                         // Does not allocate any memory


	// XOR encrypt
	encbuf = SimpleEncrypt(rlebuf, size);
	if( rlebuf != NULL )  free(rlebuf);
	if( encbuf == NULL )  return NULL;                     // SimpleEncrypt fills the error in
	
 
    // Now we have to wrap the encrypted data inside a valid PacketClass blob
	wrap.Add_Field((char*)"CNTL", (void*)encbuf, *size);
	free(encbuf);


	return (unsigned char*)(wrap.Create_Comms_Packet(*size));
}


/*------------------------------------------------------------------------------.
| FUNCTION:  DecryptPacket() 													|
| If an error occurs, a NULL will be returned and the size value will be a      |
| negative integer corresponding to an error in wencrypt.h                      |
`------------------------------------------------------------------------------*/
#ifndef __CLIENT__
unsigned char*  DecryptPacket(unsigned char* encbuf, int* size)
{
	unsigned char* rlebuf;
	unsigned char* packet;


        rlebuf = SimpleDecrypt(encbuf, size);
	if( rlebuf == NULL )
		return NULL;					// SimpleDecrypt fills the error in
 
 
        packet = RLDecode(rlebuf, size); 		        // Allocates memory!!
	if( packet == NULL )
		return NULL;					// RLDecode fills the error in


	return packet;
}
#endif


/*------------------------------------------------------------------------------.
| FUNCTION:  RLEncode()															|
| -  src is a buffer conatining the uncompressed data.  						|
| -  dst is a buffer which should be twice the size of the src buffer.			|
| -  size indicates the size of the source buffer at the start of the function,	|
|    and is filled with the length of the dest buffer on return.				|
|																				|
| NOTE:  As is the nature of RLE, it is possible the "compressed" buffer		|
| will be bigger than the uncompressed one by one byte.  Actually in the		|
| absolute worst case it can be bigger than that if the escape code occurs		|
| often enough in the buffer and needs to be stuffed several times.  I *think*	|
| it's impossible for the compressed buffer to ever get twice as big as the		|
| original, being that we choose the lowest frequency value as the escape, but	|
| there will never be a memory overrun this way, and gameres packets are tiny.	|
`------------------------------------------------------------------------------*/
void RLEncode(unsigned char* src, unsigned char* dst, int* size)
{
	int 		i, j, k;
	int		frequencies[256];
	int		lowest;
	unsigned char	escape;
	unsigned char 	currtally = 1;
	unsigned char	curr;


	// Determine what we should use for the escape code.  Ideally it's a value
	// that doesn't occur in the buffer, but we'll settle for the value that
	// appears the least number of times.
	memset(frequencies, 0, 256 * sizeof(int));
	for(i = 0; i < *size; frequencies[src[i++]]++);
	lowest = frequencies[0];
	escape = 0;
	for(i = 0; i < 256; i++)
	{
		if( frequencies[i] == 0 )
		{
			escape = (unsigned char)i;
			break;
		}
		if( frequencies[i] < lowest )
		{
			lowest = frequencies[i];
			escape = (unsigned char)i;
		}
	}
	dst[0] = escape;


	// Build the dest buffer
	curr = src[0];
	for(i = 1, j = 1; i <= *size; i++)
	{
		if( src[i] == curr && i < *size )
			currtally++;
		else
		{
			if( currtally > 3 )
			{
				dst[j++] = escape;
				dst[j++] = currtally;
				dst[j++] = curr;
			}
			else
			{
				// Stuff the escape character
				for(k = 0; k < currtally; k++)
				{
					if( curr == escape )
						dst[j++] = curr;
					dst[j++] = curr;
				}
			}
			curr = src[i];
			currtally = 1;
		}
	}
	*size = j;
}


/*------------------------------------------------------------------------------.
| FUNCTION:  RLDecode()                                                         |
| Nothing special...just remember byte 0 is the escape and not part of the 		|
| original buffer.  If there is an error, the returned pointer will be NULL		|
| and the size will contain a negative number indicating the error code.		|
`------------------------------------------------------------------------------*/
#ifndef __CLIENT__
unsigned char* RLDecode(unsigned char* src, int* size)
{
	int		i, j, k;
	int		dstsize = 0;
	unsigned char 	escape = src[0];
	unsigned char*	dst;


	// How big will the resultant buffer be?
	for(i = 1; i < *size; i++)
	{
		if( src[i] == escape && i < (*size-1) )
		{
			if( src[i+1] == escape )
			{
				dstsize++;			// Stuffed escape
				i++;
			}
			else
			{
				dstsize += (int)src[i+1];	// Add the occurences
				i += 2;				// Skip the token
			}
		}	
		else if( src[i] == escape && i == (*size-1) )
		{
			*size = ERR_SPURIOUS_ESCAPE;
			return NULL;
		}
		else
			dstsize++;
	}

	
	// Build the new buffer
	dst = malloc( dstsize );
	j = 0;
	for(i = 1; i < *size; i++)
	{
		if( src[i] == escape && i < (*size-1) )
		{
			if( src[i+1] == escape )
			{
				dst[j++] = src[i];		// Stuffed escape
				i++;
			}
			else
			{
				for(k = 0; k < (int)src[i+1]; k++)
					dst[j+k] = src[i+2];
				j += k;
				i += 2;				// Skip the token
			}
		}	
		else if( src[i] == escape && i == (*size-1) )
		{
			*size = ERR_SPURIOUS_ESCAPE;
			delete[] dst;
			return NULL;
		}
		else
			dst[j++] = src[i];
	}

	*size = dstsize;
	return dst;
}
#endif


/*--------------------------------------------------------------------------------------.
| FUNCTION:  ran3()                                                                     |
| From 'Numerical Recipies In C', Chapter 7, 'Portable Random Number Generators'.       |
`--------------------------------------------------------------------------------------*/
float ran3(int* idnum)
{
        static int      inext, inextp;
        static int     ma[66];
        static int      iff = 0;
        int            mj, mk;
        int             i, ii, k;
 
 
        if( *idnum < 0 || iff == 0 )
        {
                iff = 1;
                mj = MSEED - (*idnum < 0 ? -*idnum : *idnum);
                mj %= MBIG;
                ma[55] = mj;
                mk = 1;
                for(i = 1; i <= 54; i++)
                {
                        ii = (21*i) % 55;
                        ma[ii] = mk;
                        mk = mj - mk;
                        if( mk < MZ )  mk += MBIG;
                        mj = ma[ii];
                }
                for(k = 1; k <=4; k++)
                {
                        for(i = 1; i <= 55; i++)
                        {
                                ma[i] -= ma[1+(i+30) % 55];
                                if( ma[i] < MZ )  ma[i] += MBIG;
                        }
                }
                inext = 0;
                inextp = 31;
                *idnum = 1;
        }
        if( ++inext == 56 )  inext = 1;
        if( ++inextp == 56 )  inextp = 1;
        mj = ma[inext] - ma[inextp];
        if( mj < MZ )  mj += MBIG;
        ma[inext] = mj;
        return (float) (mj*FAC );
}


/*------------------------------------------------------------------------------.
| FUNCTION:  GenerateKey()														|
| Make a key the same length as the buffer.  Gameres packets are typically		|
| < 600 bytes so period isn't a concern.										|
`------------------------------------------------------------------------------*/
void GenerateKey(unsigned char* key, int len, int seed)
{
	int     i;
        float   running;
	int	num;
 
 
        if( seed > 0 )  seed *= -1;             // Initial val must be negative
        running = ran3(&seed);
 
        for(i = 0; i < len; i++)
        {
                num = (int)(running * (float)255.0);
                if( num < 0 )  num *= -1;
		key[i] = (unsigned char)num;
                running = ran3(&seed);
        }
}

/*--------------------------------------------------------------.
| FUNCTON:  SimpleEncrypt()										|
| Ok, not entirely simple but it's not bad.  We generate the	|
| key, then prepare a new buffer which will contain the 		|
| encrypted data.  Byte 6 of the buffer is a random offset into	|
| bytes 10 - 256 which will contain the four byte seed value	|
| used to construct the key.									|
| If there is an error, the returned pointer will be NULL and 	|
| the len value will be filled with a negative number 			|
| indicating the error code.									|
`--------------------------------------------------------------*/
unsigned char* SimpleEncrypt(const unsigned char* src, int* len)
{
	int		i, j;
	unsigned char*	key;
	unsigned char*	dst;
	unsigned char	offset = 0;
	int		offsetseed = 0;
	unsigned int	netseed;
	int		seed;
	float		limit;


	key = (unsigned char *) malloc( *len );

	dst = (unsigned char *) malloc( (*len)+5 );

	if( *len < 15 )
	{
		*len = -2;
		free( key );
		free( dst );
		return NULL;
	}
	
	// Generate the random stuff
	seed = (int)time(NULL);
	if( seed > 0 )  seed *= -1;			// Don't check for seed == 0 because a ligit
							// packet builder should never set a seed of
							// 0 and it's another mistake a cracker could 
							// possibly make.
	netseed = htonl((unsigned int)seed);
	GenerateKey(key, *len, seed);

	// We want the offset to be somewhere between byte 10 and byte (255-14), or
	// (*len - 4), whichever is smaller
	offsetseed = (int)time(NULL);
	if( offsetseed > 0 )  offsetseed *= -1;
	limit = ((*len) < 241) ? (float)((*len)-14) : (float)241.0;
	offset = (unsigned char)(ran3(&offsetseed) * limit);
	offset += (unsigned char)10;

	
	// Construct the buffer
	for(i = 0, j = 0; i < *len; i++)
	{  
		if( j == 5 )
			dst[j++] = offset;

		if( j == (int)offset )
		{
			memcpy(&dst[j], &netseed, 4);
			j += 4;
		}

		dst[j++]  = src[i] ^ key[i];
	}


	*len += 5;
	free( key );
	return dst;
}


/*--------------------------------------------------------------.
| FUNCTON:  SimpleDecrypt()										|
| We look for the offset, grab the seed, generate a key, then	|
| decrypt.														|
`--------------------------------------------------------------*/
#ifndef __CLIENT__
unsigned char* SimpleDecrypt(unsigned char* src, int* len)
{
	int             i, j;
        unsigned char*  key;
        unsigned char*  dst;
        unsigned char   offset = 0;
        unsigned int  	netseed;
        int   		seed;



	key = malloc( *len );

	dst = malloc((*len) - 5 );


	if( *len < 20 )
	{
		*len = -2;
		delete[] key;
		delete[] dst;
		return NULL;
	}
 
	
	// Generate the key
	offset = src[5];
	memcpy(&netseed, &src[offset], 4);
	seed = (int)ntohl(netseed);
	if( seed > 0 )
	{
		*len = -3;
		delete[] key;
		delete[] dst;
		return NULL;
	}
	GenerateKey(key, (*len)-5, seed);


        // Construct the buffer
        for(i = 0, j = 0; i < *len; i++)
        {
                if( i == 5 )
			continue;
                else if( i >= (int)offset && i <= (int)(offset+(unsigned char)3) )
			continue;
                else
		{
                        dst[j] = src[i] ^ key[j];
			j++;
		}
        }
 
 
	*len -= 5;
	delete[] key;
        return dst;
}
#endif