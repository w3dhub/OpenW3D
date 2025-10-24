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
 *                     $Archive:: /Commando/Code/wwnet/packetmgr.cpp                          $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/08/02 3:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 36                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "packetmgr.h"

#include <always.h>
#include <memory.h>
//#include <winsock2.h>
#include "systimer.h"
#include <malloc.h>
#include <thread>

#include "netutil.h"
#include "wwmemlog.h"
#include "crc.h"
#include "wwprofile.h"
#include "connect.h"
#include <algorithm>
#include "socket_wrapper.h"
#include <algorithm>

/*
** Single instance of PacketManagerClass.
*/
PacketManagerClass PacketManager;

/***********************************************************************************************
 * PacketManagerClass::Add_Bit -- Add a bit to a delta compressed packet stream                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Bit to add as a bool - true or false                                              *
 *           Ptr to bitstream                                                                  *
 *           Bit position at pointer                                                           *
 *                                                                                             *
 * OUTPUT:   New bit position                                                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2001 1:34PM ST : Created                                                             *
 *=============================================================================================*/
inline int PacketManagerClass::Add_Bit(bool bit, unsigned char * &bitstream, int &position)
{
	pm_assert(position < 8);

	unsigned char whole = bit ? 1 : 0;
	whole <<= position;
	*bitstream |= whole;

	position++;

	if (position == 8) {
		bitstream++;
		*bitstream = 0;
		position = 0;
	}

	return(position);
}


/***********************************************************************************************
 * PacketManagerClass::Get_Bit -- Get a bit from a delta compressed bitstream                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to bitstream                                                                  *
 *           Bit position                                                                      *
 *                                                                                             *
 * OUTPUT:   0 or 1                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2001 1:35PM ST : Created                                                             *
 *=============================================================================================*/
inline unsigned char PacketManagerClass::Get_Bit(unsigned char * &bitstream, int &position)
{
	pm_assert(position < 8);

	unsigned char whole = *bitstream;
	whole >>= position;
	whole &= 1;

	position++;

	if (position == 8) {
		bitstream++;
		position = 0;
	}

	return(whole);
}




/***********************************************************************************************
 * PacketManagerClass::PacketManagerClass -- Class constructor.                                *
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
 *   9/24/2001 1:36PM ST : Created                                                             *
 *=============================================================================================*/
PacketManagerClass::PacketManagerClass(void)
{
	BandwidthList.Set_Growth_Step(128);

	//memset(PacketLengths, 0, sizeof(PacketLengths));
	NextPacket = 0;
	NumPackets = 0;
	NumReceivePackets = 0;
	CurrentPacket = 0;
	LastSendTime = 0;
	FlushFrequency = 1000 / 10;		// Default = 10 times per second.
	AllowDeltas = true;
	AllowCombos = true;
	StatsFrequency = 10 * 1000;
	LastStatsUpdate = TIMEGETTIME();
	ErrorState = STATE_OK;
	ResetStatsIn = true;
	ResetStatsOut = true;

	NumSendBuffers = PACKET_MANAGER_BUFFERS;
	SendBuffers = new SendBufferClass[NumSendBuffers];
	NumReceiveBuffers = PACKET_MANAGER_RECEIVE_BUFFERS;
	ReceiveBuffers = new ReceiveBufferClass[NumReceiveBuffers];
}


/***********************************************************************************************
 * PacketManagerClass::~PacketManagerClass -- Class destructor                                 *
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
 *   10/15/2001 11:24AM ST : Created                                                           *
 *=============================================================================================*/
PacketManagerClass::~PacketManagerClass(void)
{
	if (SendBuffers) {
		delete [] SendBuffers;
		SendBuffers = NULL;
	}
	if (ReceiveBuffers) {
		delete [] ReceiveBuffers;
		ReceiveBuffers = NULL;
	}
}


/***********************************************************************************************
 * PacketManagerClass::Set_Is_Server -- Set whether to operate in server mode or not.          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    is_server - true if we are to be a server                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/15/2001 11:29AM ST : Created                                                           *
 *=============================================================================================*/
void PacketManagerClass::Set_Is_Server(bool is_server)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	WWMEMLOG(MEM_NETWORK);
	bool reset = false;

	if (is_server && NumSendBuffers != PACKET_MANAGER_BUFFERS_WHEN_SERVER) {
		NumSendBuffers = PACKET_MANAGER_BUFFERS_WHEN_SERVER;
		NumReceiveBuffers = PACKET_MANAGER_RECEIVE_BUFFERS_AS_SERVER;
		reset = true;
	} else {
		if (!is_server && NumSendBuffers != PACKET_MANAGER_BUFFERS) {
			NumSendBuffers = PACKET_MANAGER_BUFFERS;
			NumReceiveBuffers = PACKET_MANAGER_RECEIVE_BUFFERS;
			reset = true;
		}
	}

	if (reset) {
		NextPacket = 0;
		NumPackets = 0;
		NumReceivePackets = 0;
		CurrentPacket = 0;

		if (SendBuffers) {
			delete [] SendBuffers;
		}
		SendBuffers = new SendBufferClass[NumSendBuffers];

		if (ReceiveBuffers) {
			delete [] ReceiveBuffers;
		}
		ReceiveBuffers = new ReceiveBufferClass[NumReceiveBuffers];
		Reset_Stats();
	}
}



/***********************************************************************************************
 * PacketManagerClass::Build_Delta_Packet_Patch -- Calc a delta between two packets            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to base packet                                                                *
 *           Ptr to packet to compare with base packet                                         *
 *           Ptr to delta patch that describes differences between base and add packets        *
 *           Size of base packet (packet sizes must match)                                     *
 *           Size of add packet (must match base packets size)                                 *
 *                                                                                             *
 * OUTPUT:   Size of delta patch                                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2001 1:36PM ST : Created                                                             *
 *=============================================================================================*/
int PacketManagerClass::Build_Delta_Packet_Patch(unsigned char *base_packet, unsigned char *add_packet, unsigned char *delta_packet, int base_packet_size, int add_packet_size)
{

	/*
	** Here's how this works.
	**
	** The source and delta packets are compared 8 bytes at a time and am equality bit is set in the output stream
	** for each block of 8. Then, for each non-matching block, a per byte bit is set in the output stream to indicate
	** whether a byte can be used from the base packet or if a patch byte is needed. Finally, patch bytes are added to
	** the stream at the end starting at a byte boundry.
	**
	*/


	/*
	** Locals.
	*/
	int write_bit_pos = 0;
	int read_bit_pos = 0;
	int num_diff_bytes = 0;
	unsigned char diff_bytes[1024];

	/*
	** Parameter asserts.
	*/
	pm_assert(base_packet != NULL);
	pm_assert(add_packet != NULL);
	pm_assert(delta_packet != NULL);
	pm_assert(base_packet_size == add_packet_size);
	pm_assert(base_packet_size < sizeof(diff_bytes));

	/*
	** dereference pointers to the packet header and the packet build pointer.
	*/
	PacketDeltaHeaderStruct *header = (PacketDeltaHeaderStruct*) delta_packet;
	unsigned char *build_delta_ptr = delta_packet + sizeof(PacketDeltaHeaderStruct);
	unsigned char *chunk_ptr = build_delta_ptr;
	pm_assert(sizeof(PacketDeltaHeaderStruct) == 1);
	*build_delta_ptr = 0;

	header->BytePack = 0;

	if (base_packet_size == add_packet_size) {

		/*
		** Break it up into 8 byte chunks and see whether any of the chunks are the same.
		*/
		bool chunks = false;
		bool this_match = false;
		int i;

		for (i=0 ; i<base_packet_size - 7 ; i+=8) {

			this_match = false;

			if (*((long*)&base_packet[i]) == *((long*)&add_packet[i])) {
				if (*((long*)&base_packet[i+4]) == *((long*)&add_packet[i+4])) {
					chunks = true;
					this_match = true;
				}
			}

			write_bit_pos = Add_Bit(this_match, build_delta_ptr, write_bit_pos);
		}

		/*
		** Mark the tail end of the buffer as different.
		*/
		if ((base_packet_size & 7) != 0) {
			if (chunks) {
				write_bit_pos = Add_Bit(false, build_delta_ptr, write_bit_pos);
			}
		}

		/*
		** If none of the chunks were the same then this is a pointless endevour. Mark in the header that we don't have
		** chunk info and move on to trying byte for byte.
		*/
		header->ChunkPack = chunks ? 1 : 0;

		/*
		** Lost the useless chunk info if none of the chunks matched anyway.
		*/
		if (!chunks) {
			build_delta_ptr = chunk_ptr;
			*build_delta_ptr = 0;
			write_bit_pos = 0;
		}


		/*
		** Try a byte for byte check in the chunks that don't match. For any bytes that are different, save the changed value
		** so we can add it to the end of the delta packet later.
		*/
		read_bit_pos = 0;
		num_diff_bytes = 0;
		for (i=0 ; i<base_packet_size ; i+= 8) {

			/*
			** If we have chunk info, then use it to skip matching chunks.
			*/
			if (chunks) {
				if (Get_Bit(chunk_ptr, read_bit_pos)) {
					continue;
				}
			}

			/*
			** If this chunk didn't match then go through byte by byte and note differences.
			*/
			for (int j=i ; j<i+8 && j<base_packet_size ; j++) {
				header->BytePack = 1;
				this_match = (base_packet[j] == add_packet[j]) ? true : false;
				write_bit_pos = Add_Bit(this_match, build_delta_ptr, write_bit_pos);
				if (!this_match) {
					diff_bytes[num_diff_bytes++] = add_packet[j];
				}
			}
		}

		/*
		** OK, now we have bitfields that mark the differences. A 1 in the bitfield means that the corresponding chunk or byte
		** in the add packet is the same as in the base packet.
		** Throw the non-matching bytes onto the end of the bitfield and we have a complete delta patch.
		*/
		unsigned char *delta_bytes_ptr = build_delta_ptr;
		if (write_bit_pos != 0) {
			delta_bytes_ptr++;
		}
		memcpy(delta_bytes_ptr, diff_bytes, num_diff_bytes);

		/*
		** Calculate the size of the whole delta patch.
		*/
		int patch_size = (delta_bytes_ptr - delta_packet) + num_diff_bytes;

		return(patch_size);
	}

	return(-1);
}




/***********************************************************************************************
 * PacketManagerClass::Reconstruct_From_Delta -- Rebuild a packet from a delta stream          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Base packet to copy same bytes from                                               *
 *           Buffer to store reconstructed packet into                                         *
 *           Ptr to delta packet stream.                                                       *
 *           Size of base packet                                                               *
 *           (out) bytes extracted from delta stream                                           *
 *                                                                                             *
 * OUTPUT:   Size of restored packet (should match size of base packet)                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/26/2001 2:14PM ST : Created                                                             *
 *=============================================================================================*/
int PacketManagerClass::Reconstruct_From_Delta(unsigned char *base_packet, unsigned char *reconstructed_packet, unsigned char *delta_packet, int base_packet_size, int &delta_size)
{
	if (base_packet == NULL) {
		WWDEBUG_SAY(("*** WARNING: MALFORMED PACKET - PacketManagerClass::Reconstruct_From_Delta -- Bad base packet\n"));
		return(0);
	}

	if (reconstructed_packet == NULL) {
		WWDEBUG_SAY(("*** WARNING: MALFORMED PACKET - PacketManagerClass::Reconstruct_From_Delta -- Bad reconstructed packet\n"));
		return(0);
	}

	if (delta_packet == NULL) {
		WWDEBUG_SAY(("*** WARNING: MALFORMED PACKET - PacketManagerClass::Reconstruct_From_Delta -- Bad delta packet\n"));
		return(0);
	}

	if (base_packet_size > 500) {
		WWDEBUG_SAY(("*** WARNING: MALFORMED PACKET - PacketManagerClass::Reconstruct_From_Delta -- Bad base packet size\n"));
		return(0);
	}


	pm_assert(base_packet != NULL);
	pm_assert(reconstructed_packet != NULL);
	pm_assert(delta_packet != NULL);
	int patch_list[1024];
	int num_patches = 0;
	int read_bit_pos = 0;
	int read_chunk_pos = 0;
	int restored_bytes = 0;

	PacketDeltaHeaderStruct *header = (PacketDeltaHeaderStruct*) delta_packet;
	unsigned char *read_delta_ptr = delta_packet + sizeof(PacketDeltaHeaderStruct);
	pm_assert(sizeof(PacketDeltaHeaderStruct) == 1);
	unsigned char *chunk_ptr = read_delta_ptr;

	/*
	** If there is chunk info then extract that first. Use it to copy the approprate parts of the base packet into the add packet.
	*/
	if (header->ChunkPack) {
		for (int i=0 ; i<base_packet_size - 7 ; i+=8) {
			if (Get_Bit(read_delta_ptr, read_bit_pos)) {
				memcpy(&reconstructed_packet[i], &base_packet[i], 8);
				restored_bytes += 8;
			}
		}

		/*
		** If the packet isn't an even number of chunks long then pull out an extra bit and throw it away. It should always be 0.
		*/
		if ((base_packet_size & 7) != 0) {
//#ifdef WWDEBUG
			unsigned char bitty =
//#endif //WWDEBUG
			Get_Bit(read_delta_ptr, read_bit_pos);
			if (bitty != 0) {
				WWDEBUG_SAY(("*** WARNING: MALFORMED PACKET - PacketManagerClass::Reconstruct_From_Delta -- Bad bitty\n"));
				return(0);
			}
			pm_assert(bitty == 0);
		}
	}


	/*
	** Now use the per byte bitmask to restore the rest of the data.
	*/
	for (int i=0 ; i<base_packet_size ; i+= 8) {

		/*
		** If we have chunk info, then use it to skip matching chunks.
		*/
		if (header->ChunkPack) {
			if (Get_Bit(chunk_ptr, read_chunk_pos)) {
				continue;
			}
		}

		/*
		** If this chunk didn't match then go through byte by byte and note differences.
		*/
		for (int j=i ; j<i+8 && j<base_packet_size ; j++) {
			if (Get_Bit(read_delta_ptr, read_bit_pos)) {
				reconstructed_packet[j] = base_packet[j];
				restored_bytes++;
			} else {
				patch_list[num_patches++] = j;
			}
		}
	}

	/*
	** Well, the patch bytes must be here somewhere.
	*/
	unsigned char *patch_bytes = read_delta_ptr;
	if (read_bit_pos != 0) {
		patch_bytes++;
	}

	/*
	** Work out how many delta bytes we processed - it's needed for meta packet decoding.
	*/
	delta_size = (patch_bytes - delta_packet) + num_patches;

	/*
	** Copy the patch bytes into the correct positions.
	*/
	for (int p=0 ; p<num_patches ; p++) {
		reconstructed_packet[patch_list[p]] = *patch_bytes++;
	}

	restored_bytes += num_patches;
	//pm_assert(restored_bytes == base_packet_size);

	return(restored_bytes);
}


/***********************************************************************************************
 * PacketManagerClass::Get_Next_Free_Buffer_Index -- Get next buffer to use for taking packets *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Buffer index (-1 if none free)                                                    *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/26/2001 2:17PM ST : Created                                                             *
 *=============================================================================================*/
int PacketManagerClass::Get_Next_Free_Buffer_Index(void)
{
	int return_index = -1;
	for (int i=0 ; i<NumSendBuffers ; i++) {
		NextPacket++;
		if (NextPacket >= NumSendBuffers) {
			NextPacket = 0;
		}
		if (SendBuffers[NextPacket].PacketLength == 0) {
			return_index = NextPacket;
			break;
		}
	}
	return(return_index);
}



/***********************************************************************************************
 * PacketManagerClass::Take_Packet -- Intercept a packet before it's sent                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to packet                                                                     *
 *           Length of packet                                                                  *
 *           IP of packet recipient                                                            *
 *           Port of packet recipient                                                          *
 *                                                                                             *
 * OUTPUT:   True if packet was accepted                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/18/2001 4:24PM ST : Created                                                             *
 *=============================================================================================*/
bool PacketManagerClass::Take_Packet(unsigned char *packet, int packet_len, unsigned char *dest_ip, unsigned short dest_port, SOCKET source_socket)
{
	CriticalSectionClass::LockClass lock(CriticalSection);

	if (NumPackets < NumSendBuffers) {
		if (packet_len > 0 && packet_len < (PACKET_MANAGER_MTU - sizeof(PacketPackHeaderStruct))) {
			int index = Get_Next_Free_Buffer_Index();
			if (index != -1) {
				memcpy(SendBuffers[index].PacketBuffer, packet, packet_len);
				SendBuffers[index].PacketLength = packet_len;
				SendBuffers[index].Port = dest_port;
				memcpy(&SendBuffers[index].IPAddress[0], dest_ip, 4);
				SendBuffers[index].PacketSendSocket = source_socket;
				NumPackets++;
				//WWDEBUG_SAY(("NumPackets = %d (added packet at index %d)\n", NumPackets, index));
				if (NumPackets > NumSendBuffers - 4) {
					WWDEBUG_SAY(("***WARNING*** Outgoing packet buffer full - NumPackets = %d\n", NumPackets, index));
					Flush(true);
					WWDEBUG_SAY(("NumPackets = %d after flush\n", NumPackets, index));
				}
				Register_Packet_Out(dest_ip, dest_port, 0, packet_len + UDP_HEADER_SIZE);
				return(true);
			}
		}
	}

	return(false);
}





/***********************************************************************************************
 * PacketManagerClass::Flush -- Coalesce and send any pending packets                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Forced - set to true if packets should be sent even if it's not time              *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/26/2001 2:19PM ST : Created                                                             *
 *=============================================================================================*/
void PacketManagerClass::Flush(bool forced)
{
{
WWPROFILE("PMgr Flush");

	CriticalSectionClass::LockClass lock(CriticalSection);

	int base_index = -1;
	int length = 0;
	unsigned char *base_packet = NULL;
	int new_length = 0;
	PacketPackHeaderStruct *header = (PacketPackHeaderStruct*) BuildPacket;
	unsigned char *next_packet_pos = NULL;
	int index = 0;
	int i;
	SOCKET socket = INVALID_SOCKET;

	/*
	** If it's not time to send packets yet then just return.
	*/
	unsigned long time = TIMEGETTIME();
	if (!forced && FlushFrequency != 0) {
		if (time - LastSendTime < FlushFrequency) {
			return;
		}
	}
	LastSendTime = time;

	//WWDEBUG_SAY(("NumPackets = %d\n", NumPackets));

	/*
	** Clear the array that indicates whether a packet is ready to be sent.
	*/
	for (i=0 ; i<NumSendBuffers ; i++) {
		SendBuffers[i].PacketReady = false;
	}

	/*
	** Loop until we process all packets.
	*/
	while (NumPackets) {

		/*
		** Get a packet to start with. Find the oldest packet first in each case.
		*/
		index = NextPacket + 1;
		if (index >= NumSendBuffers) {
			index = 0;
		}
		base_index = -1;
		for (i=0 ; i<NumSendBuffers ; i++) {
			if (SendBuffers[index].PacketLength != 0) {
				base_index = index;
				length = SendBuffers[index].PacketLength;
				base_packet = (unsigned char *) SendBuffers[index].PacketBuffer;
				//WWDEBUG_SAY(("Found base packet %d\n", base_index));

				socket = SendBuffers[index].PacketSendSocket;
				header->NumPackets = 1;
				header->PacketSize = length;
				header->MorePackets = 0;
				pm_assert(header->NumPackets > 0 && header->NumPackets <= PACKET_MANAGER_MAX_PACKETS);
				next_packet_pos = BuildPacket + sizeof(*header);
				memcpy(next_packet_pos, base_packet, length);
				next_packet_pos += length;
				new_length = length + sizeof(*header);
				//WWDEBUG_SAY(("Added base packet %d to metapacket - %d (+2) bytes\n", index, length));
				index++;
				if (index >= NumSendBuffers) {
					index = 0;
				}
				break;
			}

			index++;
			if (index >= NumSendBuffers) {
				index = 0;
			}
		}

		/*
		** If we found a packet, see if there are more packets of the same size that we can package up with it.
		*/
		if (base_index != -1 && NumPackets > 1) {
			if ((new_length + length + sizeof(PacketDeltaHeaderStruct)) < PACKET_MANAGER_MTU) {

				for (int j=i ; j<NumSendBuffers ; j++) {

					/*
					** If we can't fit any more packets in this packet then break out of the loop and send it as is.
					*/
					if (header->NumPackets == PACKET_MANAGER_MAX_PACKETS) {
						break;
					}

					/*
					** Is this packet the same length? If so, it might be suitable for coagulation.
					*/
					if (SendBuffers[index].PacketLength == length && SendBuffers[index].PacketSendSocket == socket) {
						pm_assert(index != base_index);

						/*
						** Is this packet for the same recipient?
						*/
						if (SendBuffers[base_index].Port == SendBuffers[index].Port && (*(unsigned long*)(&SendBuffers[index].IPAddress[0])) == (*(unsigned long*)(&SendBuffers[base_index].IPAddress[0]))) {

							//WWDEBUG_SAY(("Found secondary packet %d\n", index));

							/*
							** See if using a delta of the two packets would be smaller than including the whole packet.
							*/
							int bytes = Build_Delta_Packet_Patch(base_packet, SendBuffers[index].PacketBuffer->Buffer, DeltaPacket, length, length);
							if (bytes < length && AllowDeltas) {
								/*
								** Yes, the delta is smaller so use that.
								*/
								memcpy(next_packet_pos, DeltaPacket, bytes);
								next_packet_pos += bytes;
								new_length += bytes;
								//WWDEBUG_SAY(("Added delta packet %d to metapacket - %d bytes\n", index, bytes));
								header->NumPackets++;
								pm_assert(header->NumPackets > 0 && header->NumPackets <= PACKET_MANAGER_MAX_PACKETS);
							} else {
								/*
								** The delta was no better than the original. Stick a 0 byte in to say it's not a delta and then
								** Copy the whole thing.
								*/
								PacketDeltaHeaderStruct *delta_header = (PacketDeltaHeaderStruct*) next_packet_pos;
								delta_header->ChunkPack = 0;
								delta_header->BytePack = 0;
								next_packet_pos += sizeof(PacketDeltaHeaderStruct);
								new_length += sizeof(PacketDeltaHeaderStruct);

								memcpy(next_packet_pos, SendBuffers[index].PacketBuffer, length);
								next_packet_pos += length;
								new_length += length;
								//WWDEBUG_SAY(("Added secondary packet %d to metapacket - %d (+1) bytes\n", index, length));
								header->NumPackets++;
								pm_assert(header->NumPackets > 0 && header->NumPackets <= PACKET_MANAGER_MAX_PACKETS);
							}

							/*
							** Discard the packet buffer now that we have added this packet to the current staging buffer area.
							*/
							SendBuffers[index].PacketLength = 0;
							NumPackets--;
							//WWDEBUG_SAY(("NumPackets = %d (discarding secondary packet index %d)\n", NumPackets, index));

							pm_assert(header->NumPackets > 0 && header->NumPackets <= PACKET_MANAGER_MAX_PACKETS);

							/*
							** If there's no room for any more of these packets then bail out.
							*/
							if ((new_length + length + sizeof(PacketDeltaHeaderStruct)) >= PACKET_MANAGER_MTU) {
								break;
							}
						}
					}
					index++;
					if (index >= NumSendBuffers) {
						index = 0;
					}
				}
			}
		}

		pm_assert(header->NumPackets > 0 && header->NumPackets <= PACKET_MANAGER_MAX_PACKETS);

		/*
		** Allright, it's time to send this sucker.
		*/
		if (base_index != -1) {

			/*
			** Just copy it back into the base index for now and mark it as ready to go.
			*/
			memcpy(SendBuffers[base_index].PacketBuffer, BuildPacket, new_length);
			SendBuffers[base_index].PacketReady = true;
			SendBuffers[base_index].PacketSendLength = new_length;
			SendBuffers[base_index].PacketLength = 0;
			NumPackets--;
			//WWDEBUG_SAY(("NumPackets = %d (discarding base packet %d)\n", NumPackets, base_index));
			pm_assert(NumPackets >= 0);
		}
		pm_assert(header->NumPackets > 0 && header->NumPackets <= PACKET_MANAGER_MAX_PACKETS);
	}


	/*
	** Merge ready buffers where possible. This is the step that adds runs of different sizeed packets into the same packet.
	*/
	if (AllowCombos) {
		for (i=0 ; i<NumSendBuffers ; i++) {
			if (SendBuffers[i].PacketReady) {

				PacketPackHeaderStruct *current_header = (PacketPackHeaderStruct*) SendBuffers[i].PacketBuffer;
				int current_len = SendBuffers[i].PacketSendLength;
				socket = SendBuffers[i].PacketSendSocket;


				/*
				** See if we can find another packet for the same destination that will fit into the same buffer.
				*/
#ifdef WWDEBUG
				int num_sub_packets = 1;
#endif //WWDEBUG
				for (int j=i+1 ; j<NumSendBuffers ; j++) {
					if (SendBuffers[j].PacketReady && SendBuffers[j].PacketSendSocket == socket) {
						if (SendBuffers[i].PacketSendLength + SendBuffers[j].PacketSendLength < PACKET_MANAGER_MTU) {
							if (SendBuffers[i].Port == SendBuffers[j].Port && (*(unsigned long*)(&SendBuffers[i].IPAddress[0])) == (*(unsigned long*)(&SendBuffers[j].IPAddress[0]))) {
								unsigned char *dest_ptr = &SendBuffers[i].PacketBuffer->Buffer[current_len];
								memcpy(dest_ptr, SendBuffers[j].PacketBuffer, SendBuffers[j].PacketSendLength);
								current_header->MorePackets = 1;
								current_header = (PacketPackHeaderStruct*) dest_ptr;
								current_len += SendBuffers[j].PacketSendLength;
								SendBuffers[i].PacketSendLength = current_len;
								SendBuffers[j].PacketReady = false;
#ifdef WWDEBUG
								num_sub_packets++;
#endif //WWDEBUG
								//WWDEBUG_SAY(("Added packet %d with %d sub packets to packet %d\n", j, ((PacketPackHeaderStruct*)&PacketBuffers[j][0])->NumPackets, i));
								//WWDEBUG_SAY(("Packet %d has %d packets total\n", i, num_sub_packets));
							}
						}
					}
				}
			}
		}
	}


	/*
	** Send any packets marked as ready.
	*/
	for (i=0 ; i<NumSendBuffers ; i++) {
		if (SendBuffers[i].PacketReady) {
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = SendBuffers[i].Port;
			memcpy (&addr.sin_addr.s_addr, &SendBuffers[i].IPAddress[0], 4);
			socket = SendBuffers[i].PacketSendSocket;
#ifdef WWDEBUG
			int debug_num_packets = (int)(((PacketPackHeaderStruct*)SendBuffers[i].PacketBuffer)->NumPackets);
			int debug_packet_size = (int)(((PacketPackHeaderStruct*)SendBuffers[i].PacketBuffer)->PacketSize);
			pm_assert(debug_num_packets > 0);
			pm_assert(debug_packet_size < PACKET_MANAGER_MTU);
			pm_assert(SendBuffers[i].PacketSendLength < PACKET_MANAGER_MTU);
			//WWDEBUG_SAY(("Sending packet %d (%d bytes) to %s. Packet has %d packets of %d bytes each\n", i, PacketSendLength[i], Addr_As_String(&addr), debug_num_packets, debug_packet_size));
#endif //WWDEBUG


#ifdef WRAPPER_CRC

			unsigned long crc = CRC::Memory((unsigned char*)SendBuffers[i].PacketBuffer, SendBuffers[i].PacketSendLength);
#if (1)
			/*
			** Reverse byte order to prevent the demo from having the same CRC as the game.
			*/
			crc = _byteswap_ulong(crc);
#endif //(0)
			char *crc_and_buffer = (char*)_alloca(SendBuffers[i].PacketSendLength + sizeof(crc));
			*((unsigned long*) crc_and_buffer) = crc;
			memcpy(crc_and_buffer + sizeof(crc), (const char*)SendBuffers[i].PacketBuffer, SendBuffers[i].PacketSendLength);

			Register_Packet_Out(&SendBuffers[i].IPAddress[0], SendBuffers[i].Port, SendBuffers[i].PacketSendLength + UDP_HEADER_SIZE + sizeof(crc), 0);
			socklen_t addr_len = sizeof(struct sockaddr_in);
			int result = wwnet::SocketSendTo(socket, crc_and_buffer, SendBuffers[i].PacketSendLength + sizeof(crc), 0, (const sockaddr*)&addr, &addr_len);

#else //WRAPPER_CRC

			Register_Packet_Out(&SendBuffers[i].IPAddress[0], SendBuffers[i].Port, SendBuffers[i].PacketSendLength + UDP_HEADER_SIZE, 0);
			socklen_t addr_len = sizeof(struct sockaddr_in);
			int result = wwnet::SocketSendTo(socket, (const char*)SendBuffers[i].PacketBuffer, SendBuffers[i].PacketSendLength, 0, (const sockaddr*)&addr, &addr_len);
#endif //WRAPPER_CRC


			if (result == SOCKET_ERROR){
				int error_code = wwnet::SocketGetLastError();
				if (error_code != WSAEWOULDBLOCK) {
					WWDEBUG_SAY(("PacketManagerClass - sendto returned error code %d - %s\n", error_code, cNetUtil::Winsock_Error_Text(error_code)));
					Clear_Socket_Error(socket);
				} else {

					/*
					** No more room for outgoing packets. Unfortunately, this means we lose the lot.
					*/
					WWDEBUG_SAY(("PacketManagerClass - sendto returned WSAEWOULDBLOCK\n"));
					std::this_thread::yield();
					ErrorState = STATE_WS_BUFFERS_FULL;
				}
			}

			/*
			** Send some random garbage to see if we crash.
			*/
			//char garbage[640];
			//for (int i=0 ; i<540 ; i++) {
			//	garbage[i] = rand();
			//}
			//wwnet::SocketSendTo(socket, (const char*)garbage, 540, 0, (const sockaddr*) &addr, sizeof(struct sockaddr_in));

		}
	}
	Update_Stats();
}
}



/***********************************************************************************************
 * PacketManagerClass::Disable_Optimizations -- Disable all low level optimizations            *
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
 *   9/26/2001 2:24PM TSS : Created                                                            *
 *=============================================================================================*/
void PacketManagerClass::Disable_Optimizations(void)
{
	FlushFrequency = 0;
	AllowDeltas = false;
	AllowCombos = false;
}



/***********************************************************************************************
 * PacketManagerClass::Break_Packet -- Break up in incoming packet into it's original packets  *
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
 *   9/26/2001 2:25PM ST : Created                                                             *
 *=============================================================================================*/
bool PacketManagerClass::Break_Packet(unsigned char *packet, int original_packet_size, unsigned char *ip_address, unsigned short port)
{
	/*
	** Dereference a pointer to the packet header.
	*/
	PacketPackHeaderStruct *header = (PacketPackHeaderStruct*) packet;

	/*
	** Pull out the number of packets and the size of each.
	*/
	int num_packets = header->NumPackets;
	int packet_size = header->PacketSize;
	bool more_packets = header->MorePackets;

	if (num_packets < 1 || packet_size > PACKET_MANAGER_MTU) {
		WWDEBUG_SAY(("PacketManager - Packet decode error. num_packets = %d\n, packet_size = %d\n", num_packets, packet_size));
		return(false);
	}
	pm_assert(num_packets >= 1);
	pm_assert(packet_size <= PACKET_MANAGER_MTU);

	/*
	** Get the first packet. This is needed as a reference for other delta packets.
	*/
	unsigned char *packet_ptr = packet + sizeof(*header);
	memcpy(&ReceiveBuffers[NumReceivePackets].ReceiveHoldingBuffer[0], packet_ptr, packet_size);
	ReceiveBuffers[NumReceivePackets].ReceivePacketLength = packet_size;
	Register_Packet_In(ip_address, port, 0, packet_size + UDP_HEADER_SIZE);
	int delta_base_index = NumReceivePackets;
	memcpy(ReceiveIPAddress, ip_address, 4);
	ReceivePort = port;
	NumReceivePackets++;
	//WWDEBUG_SAY(("Extracted base packet from metapacket - %d bytes\n", packet_size));

	packet_ptr += packet_size;

	/*
	** Get the rest of the packets.
	*/
	for (int i=0 ; i<num_packets-1 ; i++) {
		PacketDeltaHeaderStruct * delta_header = (PacketDeltaHeaderStruct*) packet_ptr;

		/*
		** If this is a delta packet then we need to reconstruct it before copying it into the buffer.
		*/
		if (delta_header->ChunkPack || delta_header->BytePack) {
			int delta_size = 0;
//#ifdef WWDEBUG
			int bytes =
//#endif //WWDEBUG
				Reconstruct_From_Delta(&ReceiveBuffers[delta_base_index].ReceiveHoldingBuffer[0], &ReceiveBuffers[NumReceivePackets].ReceiveHoldingBuffer[0], (unsigned char *)delta_header, packet_size, delta_size);
			if (bytes != packet_size) {
				WWDEBUG_SAY(("*** WARNING: MALFORMED PACKET - PacketManagerClass::Break_Packet -- bytes !=  packet_size\n"));
				return(false);
			}
			if (delta_size <= 0) {
				WWDEBUG_SAY(("*** WARNING: MALFORMED PACKET - PacketManagerClass::Break_Packet -- delta_size <= 0\n"));
				return(false);
			}
			pm_assert(bytes == packet_size);
			pm_assert(delta_size > 0);
			ReceiveBuffers[NumReceivePackets].ReceivePacketLength = packet_size;
			NumReceivePackets++;
			packet_ptr += delta_size;
			Register_Packet_In(ip_address, port, 0, packet_size + UDP_HEADER_SIZE);
			//WWDEBUG_SAY(("Extracted delta packet from metapacket - %d bytes (delta size = %d)\n", bytes, delta_size));
		} else {
			/*
			** Not a delta, just copy the whole thing.
			*/
			packet_ptr += sizeof(*delta_header);
			memcpy(&ReceiveBuffers[NumReceivePackets].ReceiveHoldingBuffer[0], packet_ptr, packet_size);
			ReceiveBuffers[NumReceivePackets].ReceivePacketLength = packet_size;
			packet_ptr += packet_size;
			NumReceivePackets++;
			Register_Packet_In(ip_address, port, 0, packet_size + UDP_HEADER_SIZE);
			//WWDEBUG_SAY(("Extracted secondary packet from metapacket - %d (+1) bytes\n", PacketLength));
		}
		if (NumReceivePackets >= PACKET_MANAGER_RECEIVE_BUFFERS) {
			break;
		}
	}

	int bytes_pulled_from_packet = packet_ptr - packet;

	/*
	** More packets in this buffer?
	*/
	if (more_packets && NumReceivePackets < PACKET_MANAGER_RECEIVE_BUFFERS) {
		if (!Break_Packet(packet_ptr, original_packet_size - bytes_pulled_from_packet, ip_address, port)) {
			return(false);
		}
	}

	//if (NumReceivePackets > 20) {
	//	WWDEBUG_SAY(("Sub packet broken into %d packets\n", NumReceivePackets));
	//}

	return(true);
}


/***********************************************************************************************
 * PacketManagerClass::Clear_Socket_Error -- Clear an error condition on a socket              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Socket                                                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/2001 12:19PM ST : Created                                                           *
 *=============================================================================================*/
void PacketManagerClass::Clear_Socket_Error(SOCKET socket)
{
	unsigned long error_code;
	socklen_t opt_len = static_cast<socklen_t>(sizeof(error_code));
	assert(socket != INVALID_SOCKET);

	if (socket != INVALID_SOCKET) {
		wwnet::SocketGetSockOpt(socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error_code), &opt_len);
		WWDEBUG_SAY(("Per socket error is %d - %s\n", error_code, cNetUtil::Winsock_Error_Text(error_code)));
	}
}



/***********************************************************************************************
 * PacketManagerClass::Get_Packet -- Return the next incoming packet to the app                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Socket to use                                                                     *
 *           Ptr to packet buffer                                                              *
 *           Size of packet buffer                                                             *
 *           (out) Ptr to ip_address                                                           *
 *           (out) Port number                                                                 *
 *                                                                                             *
 * OUTPUT:   Size of packet (0 = no packet, -1 = fatal socket error)                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/26/2001 2:28PM ST : Created                                                             *
 *=============================================================================================*/
int PacketManagerClass::Get_Packet(SOCKET socket, unsigned char *packet_buffer, int packet_buffer_size, unsigned char *ip_address, unsigned short &port)
{
{
WWPROFILE("Pmgr Get");
	CriticalSectionClass::LockClass lock(CriticalSection);

	if (NumReceivePackets == 0) {
		socklen_t address_size = sizeof(sockaddr_in);
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		pm_assert(packet_buffer_size >= PACKET_MANAGER_MTU);
		wwnet::SocketIoctlParam bytes_available;
		int result = wwnet::SocketIoctl(socket, FIONREAD, &bytes_available);
		if (result == 0 && bytes_available != 0) {

			int bytes = wwnet::SocketRecvFrom(socket, (char*)packet_buffer, packet_buffer_size, 0, (sockaddr*)&addr, &address_size);
			if (bytes > 0) {
#ifndef WRAPPER_CRC
				Register_Packet_In((unsigned char*) &addr.sin_addr.s_addr, addr.sin_port, bytes + UDP_HEADER_SIZE, 0);
#endif //WRAPPER_CRC

#ifdef WRAPPER_CRC
				unsigned long crc = CRC::Memory((unsigned char*)packet_buffer + 4, bytes - sizeof(crc));
#if (1)
				/*
				** Reverse byte order to prevent the demo from having the same CRC as the game.
				*/
				crc = _byteswap_ulong(crc);
#endif //(0)
				if (crc != *((unsigned long*)packet_buffer)) {
					WWDEBUG_SAY(("PMC::Get_Packet: Socket %d, received packet %d bytes long from %s\n", socket, bytes, Addr_As_String(&addr)));
					WWDEBUG_SAY(("PMC::Get_Packet: *** PACKET WRAPPER CRC ERROR ***"));
					NumReceivePackets = 0;
				} else {
					Register_Packet_In((unsigned char*) &addr.sin_addr.s_addr, addr.sin_port, bytes + UDP_HEADER_SIZE, 0);
					bytes -= sizeof(crc);
					memmove(packet_buffer, packet_buffer + sizeof(crc), bytes);
#endif //WRAPPER_CRC

					//WWDEBUG_SAY(("PMC::Get_Packet: Socket %d, received packet %d bytes long from %s\n", socket, bytes, Addr_As_String(&addr)));
					ReceiveSocket = socket;
					//WWDEBUG_SAY(("Breaking packet %d bytes long from %s\n", bytes, Addr_As_String(&addr)));
					bool broken = Break_Packet(packet_buffer, bytes, (unsigned char*) &addr.sin_addr.s_addr, addr.sin_port);
					if (!broken) {
						WWDEBUG_SAY(("Failed to break packet %d bytes long from %s\n", bytes, Addr_As_String(&addr)));
						WWDEBUG_SAY(("Discarding %d suspect packets due to decode failure\n", NumReceivePackets));
						NumReceivePackets = 0;
					} else {
						//WWDEBUG_SAY(("PMC::Get_Packet: Packet broken into %d packets\n", NumReceivePackets));
					}
					CurrentPacket = 0;
#ifdef WRAPPER_CRC
				}
#endif //WRAPPER_CRC
				} else {
					if (bytes == SOCKET_ERROR) {
						int error_code = wwnet::SocketGetLastError();
						if (error_code != WSAEWOULDBLOCK) {
							WWDEBUG_SAY(("PacketManagerClass - recvfrom failed with error %d - %s\n", error_code, cNetUtil::Winsock_Error_Text(error_code)));
							Clear_Socket_Error(socket);
							if (error_code == WSAECONNRESET) {
								WWDEBUG_SAY(("PacketManagerClass - WSAECONNRESET from address %s\n", Addr_As_String(&addr)));
								memcpy(ip_address, &addr.sin_addr.s_addr, 4);
								port = addr.sin_port;
								return(-1);
							}
						} else {
							WWDEBUG_SAY(("PacketManagerClass - recvfrom failed with error WSAEWOULDBLOCK\n"));
						}
					}
				}
		}
	}

	if (CurrentPacket < NumReceivePackets && socket == ReceiveSocket) {
		/*
		** Copy the current packet into the return buffer. We have to zero out the rest of the buffer or the CRC won't come out
		** right. Lordy. FIxed in CRC code now ST - 9/24/2001 3:36PM
		*/
		int size = (int)ReceiveBuffers[CurrentPacket].ReceivePacketLength;
		memcpy(packet_buffer, &ReceiveBuffers[CurrentPacket].ReceiveHoldingBuffer[0], std::min(size, packet_buffer_size));
		//if (size < packet_buffer_size) {
		//	memset(packet_buffer + size, 0, packet_buffer_size - size);
		//}
		memcpy(ip_address, ReceiveIPAddress, 4);
		port = ReceivePort;
		CurrentPacket++;

		if (CurrentPacket >= NumReceivePackets) {
			CurrentPacket = 0;
			NumReceivePackets = 0;
		}

		return(size);
	}

	return(0);
}
}




/***********************************************************************************************
 * PacketManager::Reset_Stats -- Reset bandwidth stats                                         *
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
 *   10/9/2001 8:54AM ST : Created                                                             *
 *=============================================================================================*/
void PacketManagerClass::Reset_Stats(void)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	WWDEBUG_SAY(("PacketManagerClass Resetting stats\n"));
	BandwidthList.Delete_All();
	LastStatsUpdate = TIMEGETTIME();
	ResetStatsIn = true;
	ResetStatsOut = true;
}




/***********************************************************************************************
 * PacketManager::Get_Stats_Index -- Get stats struct index for ip/port pair                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    IP                                                                                *
 *           Port                                                                              *
 *           Can create - true if allowed to create an entry if one doesn't exist              *
 *                                                                                             *
 * OUTPUT:   Index of stats struct for this ip/port, -1 for not found                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 8:54AM ST : Created                                                             *
 *=============================================================================================*/
int PacketManagerClass::Get_Stats_Index(unsigned long ip_address, unsigned short port, bool can_create)
{
	/*
	** Find the stats struct entry for this ip/port.
	*/
	for (int i=0 ; i<BandwidthList.Count() ; i++) {
		if (ip_address == BandwidthList[i].IPAddress && port == BandwidthList[i].Port) {
			return(i);
		}
	}

	if (can_create) {
		/*
		** No entry found - create a new one.
		*/
		BandwidthStatsStruct stats;
		stats.IPAddress = ip_address;
		stats.Port = port;
		stats.UncompressedBytesIn = 0;
		stats.UncompressedBytesOut = 0;
		stats.CompressedBytesIn = 0;
		stats.CompressedBytesOut = 0;
		stats.UncompressedBandwidthIn = 0;
		stats.UncompressedBandwidthOut = 0;
		stats.CompressedBandwidthIn = 0;
		stats.CompressedBandwidthOut = 0;
		BandwidthList.Add(stats);
		return(BandwidthList.Count()-1);
	}
	return(-1);
}



/***********************************************************************************************
 * PacketManager::Register_Packet_In -- Register an incoming packet for bandwidth stats        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    IP address packet came from                                                       *
 *           Port packet came from                                                             *
 *           Compressed size of packet (can be 0)                                              *
 *           Uncompressed size of packet (can be 0)                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 8:56AM ST : Created                                                             *
 *=============================================================================================*/
void PacketManagerClass::Register_Packet_In(unsigned char *ip_address, unsigned short port, unsigned long compressed_size, unsigned long uncompressed_size)
{
	static unsigned long _last_ip = 0;
	static unsigned short _last_port = 0;
	static int _last_stats = -1;
	unsigned long long_ip = *((unsigned long*)ip_address);

	if (ResetStatsIn) {
		_last_ip = 0;
		_last_port = 0;
		_last_stats = -1;
		ResetStatsIn = false;
	}

	if (_last_stats != -1 && _last_ip == long_ip && _last_port == port && BandwidthList.Count()) {
		BandwidthList[_last_stats].UncompressedBytesIn += uncompressed_size;
		BandwidthList[_last_stats].CompressedBytesIn += compressed_size;
	} else {

		int stats_index = Get_Stats_Index(long_ip, port);
		assert(stats_index != -1);

		if (stats_index != -1) {
			BandwidthList[stats_index].UncompressedBytesIn += uncompressed_size;
			BandwidthList[stats_index].CompressedBytesIn += compressed_size;
			_last_stats = stats_index;
			_last_ip = long_ip;
			_last_port = port;
		}
	}
}



/***********************************************************************************************
 * PacketManager::Register_Packet_Out -- Register an outgoing packet for bandwidth stats       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    IP address packet is going to                                                     *
 *           Port the packet is going to                                                       *
 *           Compressed size of packet (can be 0)                                              *
 *           Uncompressed size of packet (can be 0)                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 8:56AM ST : Created                                                             *
 *=============================================================================================*/
void PacketManagerClass::Register_Packet_Out(unsigned char *ip_address, unsigned short port, unsigned long compressed_size, unsigned long uncompressed_size)
{
	static unsigned long _last_ip = 0;
	static unsigned short _last_port = 0;
	static int _last_stats = -1;
	unsigned long long_ip = *((unsigned long*)ip_address);

	if (ResetStatsOut) {
		_last_ip = 0;
		_last_port = 0;
		_last_stats = -1;
		ResetStatsOut = false;
	}

	if (_last_stats != -1 && _last_ip == long_ip && _last_port == port && BandwidthList.Count()) {
		BandwidthList[_last_stats].UncompressedBytesOut += uncompressed_size;
		BandwidthList[_last_stats].CompressedBytesOut += compressed_size;
	} else {

		int stats_index = Get_Stats_Index(long_ip, port);
		assert(stats_index != -1);

		if (stats_index != -1) {
			BandwidthList[stats_index].UncompressedBytesOut += uncompressed_size;
			BandwidthList[stats_index].CompressedBytesOut += compressed_size;
			_last_stats = stats_index;
			_last_ip = long_ip;
			_last_port = port;
		}
	}
}


/***********************************************************************************************
 * PacketManagerClass::Update_Stats -- Periodically update the bandwidth stats                 *
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
 *   10/9/2001 8:57AM ST : Created                                                             *
 *=============================================================================================*/
void PacketManagerClass::Update_Stats(bool forced)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	unsigned long time = TIMEGETTIME();

	/*
	** Handle timer resetting.
	*/
	if (time < LastStatsUpdate) {
		LastStatsUpdate = 0;
	}

	/*
	** Only update every 'n' seconds.
	*/
	if (time - LastStatsUpdate > StatsFrequency || forced) {

		LastStatsUpdate = time;

		/*
		** Reset the bandwidth totals.
		*/
		TotalCompressedBandwidthIn = 0;
		TotalCompressedBandwidthOut = 0;
		TotalUncompressedBandwidthIn = 0;
		TotalUncompressedBandwidthOut = 0;

		/*
		** Go through the entry for each ip/port and work out the averages.
		*/
		for (int i=0 ; i<BandwidthList.Count() ; i++) {
			BandwidthStatsStruct *stats = &BandwidthList[i];

			if (stats->CompressedBytesOut) {
				//stats->CompressedBandwidthOut = (stats->CompressedBytesOut * 8) / StatsFrequency;
				stats->CompressedBandwidthOut = (int)((1000 * stats->CompressedBytesOut * 8) / (float) StatsFrequency);
				stats->CompressedBytesOut = 0;
				TotalCompressedBandwidthOut += stats->CompressedBandwidthOut;
			}

			if (stats->CompressedBytesIn) {
				//stats->CompressedBandwidthIn = (stats->CompressedBytesIn * 8) / StatsFrequency;
				stats->CompressedBandwidthIn = (int)((1000 * stats->CompressedBytesIn * 8) / (float) StatsFrequency);
				stats->CompressedBytesIn = 0;
				TotalCompressedBandwidthIn += stats->CompressedBandwidthIn;
			}

			if (stats->UncompressedBytesOut) {
				//stats->UncompressedBandwidthOut = (stats->UncompressedBytesOut * 8) / StatsFrequency;
				stats->UncompressedBandwidthOut = (int)((1000 * stats->UncompressedBytesOut * 8) / (float) StatsFrequency);
				stats->UncompressedBytesOut = 0;
				TotalUncompressedBandwidthOut += stats->UncompressedBandwidthOut;
			}

			if (stats->UncompressedBytesIn) {
				//stats->UncompressedBandwidthIn = (stats->UncompressedBytesIn * 8) / StatsFrequency;
				stats->UncompressedBandwidthIn = (int)((1000 * stats->UncompressedBytesIn * 8) / (float) StatsFrequency);
				stats->UncompressedBytesIn = 0;
				TotalUncompressedBandwidthIn += stats->UncompressedBandwidthIn;
			}
		}

		/*
		** Just debug output.
		*/
		//WWDEBUG_SAY(("TotalUncompressedBandwidthOut = %d bits per second\n", TotalUncompressedBandwidthOut));
		//WWDEBUG_SAY(("TotalCompressedBandwidthOut = %d bits per second\n", TotalCompressedBandwidthOut));
		//WWDEBUG_SAY(("TotalUncompressedBandwidthIn = %d bits per second\n", TotalUncompressedBandwidthIn));
		//WWDEBUG_SAY(("TotalCompressedBandwidthIn = %d bits per second\n", TotalCompressedBandwidthIn));
		//unsigned long comp_out = 100 - ((100 * TotalCompressedBandwidthOut) / TotalUncompressedBandwidthOut);
		//unsigned long comp_in = 100 - ((100 * TotalCompressedBandwidthIn) / TotalUncompressedBandwidthIn);
		//WWDEBUG_SAY(("Compression out = %d percent\n", comp_out));
		//WWDEBUG_SAY(("Compression in = %d percent\n", comp_in));
	}
}




/***********************************************************************************************
 * PacketManager::Get_Total_Raw_Bandwidth_In -- Get total uncompressed bandwidth in            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Total bandwidth before compression, in bits per second                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 8:59AM ST : Created                                                             *
 *=============================================================================================*/
unsigned long PacketManagerClass::Get_Total_Raw_Bandwidth_In(void)
{
	return(TotalUncompressedBandwidthIn);
}


/***********************************************************************************************
 * PacketManager::Get_Total_Raw_Bandwidth_Out -- Get total uncompressed bandwidth out          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Total bandwidth before compression, in bits per second                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 8:59AM ST : Created                                                             *
 *=============================================================================================*/
unsigned long PacketManagerClass::Get_Total_Raw_Bandwidth_Out(void)
{
	return(TotalUncompressedBandwidthOut);

}


/***********************************************************************************************
 * PacketManager::Get_Total_Compressed_Bandwidth_In -- Get total compressed bandwidth in       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Total bandwidth after compression, in bits per second                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 8:59AM ST : Created                                                             *
 *=============================================================================================*/
unsigned long PacketManagerClass::Get_Total_Compressed_Bandwidth_In(void)
{
	return(TotalCompressedBandwidthIn);
}


/***********************************************************************************************
 * PacketManager::Get_Total_Compressed_Bandwidth_Out -- Get total compressed bandwidth out     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Total bandwidth after compression, in bits per second                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 8:59AM ST : Created                                                             *
 *=============================================================================================*/
unsigned long PacketManagerClass::Get_Total_Compressed_Bandwidth_Out(void)
{
	return(TotalCompressedBandwidthOut);
}


/***********************************************************************************************
 * PacketManager::Get_Raw_Bandwidth_In -- Get uncompressed bandwidth in from given address     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address                                                                           *
 *                                                                                             *
 * OUTPUT:   Uncompressed bandwidth from address, in bits per second                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 9:01AM ST : Created                                                             *
 *=============================================================================================*/
unsigned long PacketManagerClass::Get_Raw_Bandwidth_In(struct sockaddr_in *address)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	unsigned long ip = *((unsigned long*)&address->sin_addr.s_addr);
	unsigned short port = address->sin_port;
	int stats = Get_Stats_Index(ip, port, false);

	unsigned long bw = 0;
	if (stats != -1) {
		bw = BandwidthList[stats].UncompressedBandwidthIn;
	}
	return(bw);
}


/***********************************************************************************************
 * PacketManager::Get_Raw_Bandwidth_Out -- Get uncompressed bandwidth to given address         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address                                                                           *
 *                                                                                             *
 * OUTPUT:   Uncompressed bandwidth to address, in bits per second                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 9:01AM ST : Created                                                             *
 *=============================================================================================*/
unsigned long PacketManagerClass::Get_Raw_Bandwidth_Out(struct sockaddr_in *address)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	unsigned long ip = *((unsigned long*)&address->sin_addr.s_addr);
	unsigned short port = address->sin_port;
	int stats = Get_Stats_Index(ip, port, false);

	unsigned long bw = 0;
	if (stats != -1) {
		bw = BandwidthList[stats].UncompressedBandwidthOut;
	}
	return(bw);
}



/***********************************************************************************************
 * PacketManager::Get_Raw_Bytes_Out -- Get uncompressed bytes to given address                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address                                                                           *
 *                                                                                             *
 * OUTPUT:   Uncompressed bytes sent to address.                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 9:01AM ST : Created                                                             *
 *=============================================================================================*/
unsigned long PacketManagerClass::Get_Raw_Bytes_Out(struct sockaddr_in *address)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	unsigned long ip = *((unsigned long*)&address->sin_addr.s_addr);
	unsigned short port = address->sin_port;
	int stats = Get_Stats_Index(ip, port, false);

	unsigned long bytes = 0;
	if (stats != -1) {
		bytes = BandwidthList[stats].UncompressedBytesOut;
	}
	return(bytes);
}




/***********************************************************************************************
 * PacketManager::Get_Compressed_Bandwidth_In -- Get compressed bandwidth in from address      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address                                                                           *
 *                                                                                             *
 * OUTPUT:   Compressed bandwidth in from address, in bits per second                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 9:01AM ST : Created                                                             *
 *=============================================================================================*/
unsigned long PacketManagerClass::Get_Compressed_Bandwidth_In(struct sockaddr_in *address)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	unsigned long ip = *((unsigned long*)&address->sin_addr.s_addr);
	unsigned short port = address->sin_port;
	int stats = Get_Stats_Index(ip, port, false);

	unsigned long bw = 0;
	if (stats != -1) {
		bw = BandwidthList[stats].CompressedBandwidthIn;
	}
	return(bw);
}


/***********************************************************************************************
 * PacketManager::Get_Compressed_Bandwidth_Out -- Get compressed bandwidth to address          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address                                                                           *
 *                                                                                             *
 * OUTPUT:   Compressed bandwidth to address, in bits per second                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 9:01AM ST : Created                                                             *
 *=============================================================================================*/
unsigned long PacketManagerClass::Get_Compressed_Bandwidth_Out(struct sockaddr_in *address)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	unsigned long ip = *((unsigned long*)&address->sin_addr.s_addr);
	unsigned short port = address->sin_port;
	int stats = Get_Stats_Index(ip, port, false);

	unsigned long bw = 0;
	if (stats != -1) {
		bw = BandwidthList[stats].CompressedBandwidthOut;
	}
	return(bw);
}


/***********************************************************************************************
 * PacketManagerClass::Set_Stats_Sampling_Frequency -- Set sample freq for stats               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    New frequency (delay in ms)                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 9:58AM ST : Created                                                             *
 *=============================================================================================*/
void PacketManagerClass::Set_Stats_Sampling_Frequency_Delay(unsigned long time_ms)
{
	assert(time_ms > 0);
	StatsFrequency = time_ms;
	Reset_Stats();
}



/***********************************************************************************************
 * PacketManagerClass::Get_Error_State -- Report and clear winsock error state                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Class error state                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/24/2001 1:53PM ST : Created                                                            *
 *=============================================================================================*/
PacketManagerClass::ErrorStateEnum PacketManagerClass::Get_Error_State(void)
{
	ErrorStateEnum state = ErrorState;
	ErrorState = STATE_OK;
	return(state);
}











/*
** Operators required to allow us to add BandwidthStateStruct to a dynamic vector.
*/
bool PacketManagerClass::BandwidthStatsStruct::operator == (BandwidthStatsStruct const &stats) {
	return((memcmp(this, &stats, sizeof(*this)) == 0) ? true : false);
}

bool PacketManagerClass::BandwidthStatsStruct::operator != (BandwidthStatsStruct const &stats) {
	return((memcmp(this, &stats, sizeof(*this)) == 0) ? false : true);
}
