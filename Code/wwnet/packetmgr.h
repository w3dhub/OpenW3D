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
 *                     $Archive:: /Commando/Code/wwnet/packetmgr.h                            $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/18/02 10:48p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef _PACKETMGR_H
#define _PACKETMGR_H

#include "mutex.h"
#include "wwdebug.h"
#include "vector.h"
#include "socket_wrapper.h"

#ifdef WWASSERT
#ifndef pm_assert
#define pm_assert WWASSERT
#endif //pm_assert
#else //WWASSERT
#define pm_assert assert
#endif //WWASSERT

#pragma pack(push)
#pragma pack(1)

#define WRAPPER_CRC


/*
** Header used at the beginning of a packet to identify the number of packets packed in this packet IYSWIM.
*/
struct PacketPackHeaderStruct {
	/*
	** Number of same length packets in this block.
	*/
	unsigned short NumPackets : 5;

	/*
	** Length of the packets.
	*/
	unsigned short PacketSize : 10;

	/*
	** More packets of a different length after these ones?
	*/
	unsigned short MorePackets : 1;
};


/*
** Header used at the beginning of every delta packet.
*/
struct PacketDeltaHeaderStruct {
	/*
	** Chunk pack bits are present.
	*/
	unsigned char ChunkPack : 1;

	/*
	** Byte pack bits are present.
	*/
	unsigned char BytePack : 1;
};


#pragma pack(pop)

/*
** Minimum MTU allowable on the internet is 576. IP Header is 20 bytes. UDP header is 8 bytes
** So our max packet size is 576 - 28 = 548
*/
#ifdef WRAPPER_CRC
#define PACKET_MANAGER_MTU 540
#else
#define PACKET_MANAGER_MTU 544
#endif //WRAPPER_CRC
#define PACKET_MANAGER_BUFFERS 256
#define PACKET_MANAGER_BUFFERS_WHEN_SERVER (32 * 32)
#define PACKET_MANAGER_RECEIVE_BUFFERS 128
#define PACKET_MANAGER_RECEIVE_BUFFERS_AS_SERVER (64 * 32)
#define PACKET_MANAGER_MAX_PACKETS 31
#define UDP_HEADER_SIZE 28


/*
** This class intercepts packets at the lowest level and applies delta based compression and packet combining to reduce
** low level bandwidth while being transparent to higher levels in the application.
**
** All packets are prefixed by a PacketHeaderStruct which is 2 bytes long. So sending a single packet incurs an extra
** overhead of two bytes. Normally, however, multiple small packets can be combined into a single large one which saves
** the IP and UDP overhead invlolved in sending the subsequent packets.
**
*/
class PacketManagerClass;
class PacketManagerClass
{
	public:
		/*
		** Constructor.
		*/
		PacketManagerClass(void);
		~PacketManagerClass(void);

		/*
		** Application interface.
		*/
		bool Take_Packet(unsigned char *packet, int packet_len, unsigned char *dest_ip, unsigned short dest_port, SOCKET socket);
		int Get_Packet(SOCKET socket, unsigned char *packet_buffer, int packet_buffer_size, unsigned char *ip_address, unsigned short &port);
		void Flush(bool forced = false);
		void Set_Is_Server(bool is_server);

		/*
		** Bandwidth Management.
		*/
		void Reset_Stats(void);
		void Update_Stats(bool forced = false);
		unsigned long Get_Total_Raw_Bandwidth_In(void);
		unsigned long Get_Total_Raw_Bandwidth_Out(void);
		unsigned long Get_Total_Compressed_Bandwidth_In(void);
		unsigned long Get_Total_Compressed_Bandwidth_Out(void);

		unsigned long Get_Raw_Bandwidth_In(struct sockaddr_in *address);
		unsigned long Get_Raw_Bandwidth_Out(struct sockaddr_in *address);
		unsigned long Get_Compressed_Bandwidth_In(struct sockaddr_in *address);
		unsigned long Get_Compressed_Bandwidth_Out(struct sockaddr_in *address);

		unsigned long Get_Raw_Bytes_Out(struct sockaddr_in *address);

		void Set_Stats_Sampling_Frequency_Delay(unsigned long time_ms);
		unsigned long Get_Stats_Sampling_Frequency_Delay(void) {return(StatsFrequency);};


		/*
		** Class configuration.
		*/
		void Set_Flush_Frequency(unsigned long freq) {FlushFrequency = freq;};
		bool Toggle_Allow_Deltas(void) {
			AllowDeltas = AllowDeltas ? false : true;
			return(AllowDeltas);
		};

		bool Toggle_Allow_Combos(void) {
			AllowCombos = AllowCombos ? false : true;
			return(AllowCombos);
		};

		//
		// TSS added 09/25/01
		//
		unsigned long Get_Flush_Frequency(void)	{return FlushFrequency;}
		bool Get_Allow_Deltas(void)					{return AllowDeltas;}
		bool Get_Allow_Combos(void)					{return AllowCombos;}
		void Disable_Optimizations(void);

		enum ErrorStateEnum {
			STATE_OK,
			STATE_WS_BUFFERS_FULL,
		};
		ErrorStateEnum Get_Error_State(void);


	private:

		/*
		** Delta compression.
		*/
		static int Build_Delta_Packet_Patch(unsigned char *base_packet, unsigned char *add_packet, unsigned char *delta_packet, int base_packet_size, int add_packet_size);
		static int Reconstruct_From_Delta(unsigned char *base_packet, unsigned char *reconstructed_packet, unsigned char *delta_packet, int base_packet_size, int &delta_size);
		bool Break_Packet(unsigned char *packet, int packet_len, unsigned char *ip_address, unsigned short port);

		/*
		** Bit packing.
		*/
		static inline int Add_Bit(bool bit, unsigned char * &bitstream, int &position);
		static inline unsigned char Get_Bit(unsigned char * &bitstream, int &position);

		/*
		** Buffer allocation.
		*/
		int Get_Next_Free_Buffer_Index(void);

		/*
		** Error handling.
		*/
		void Clear_Socket_Error(SOCKET socket);

		/*
		** Stats management.
		*/
		struct BandwidthStatsStruct {
			unsigned long	IPAddress;
			unsigned short	Port;
			unsigned long	UncompressedBytesIn;
			unsigned long	UncompressedBytesOut;
			unsigned long	CompressedBytesIn;
			unsigned long	CompressedBytesOut;
			unsigned long	UncompressedBandwidthIn;
			unsigned long	UncompressedBandwidthOut;
			unsigned long	CompressedBandwidthIn;
			unsigned long	CompressedBandwidthOut;

			bool operator == (BandwidthStatsStruct const &stats);
			bool operator != (BandwidthStatsStruct const &stats);
		};
		int Get_Stats_Index(unsigned long ip_address, unsigned short port, bool can_create = true);
		void Register_Packet_In(unsigned char *ip_address, unsigned short port, unsigned long compressed_size, unsigned long uncompressed_size);
		void Register_Packet_Out(unsigned char *ip_address, unsigned short port, unsigned long compressed_size, unsigned long uncompressed_size);

		/*
		** Send buffers.
		*/
		typedef struct tPacketBufferType {
			unsigned char Buffer [600];
		} PacketBufferType;

		class SendBufferClass {
			public:
				PacketBufferType 		*PacketBuffer;
				unsigned char        IPAddress[4];
				unsigned short			Port;
				int						PacketLength;
				bool						PacketReady;
				int						PacketSendLength;
				SOCKET					PacketSendSocket;

				SendBufferClass(void) {
					PacketBuffer = new PacketBufferType;
					IPAddress[0] = 0;
					IPAddress[1] = 0;
					IPAddress[2] = 0;
					IPAddress[3] = 0;
					Port = 0;
					PacketLength = 0;
					PacketReady = false;
					PacketSendLength = 0;
					PacketSendSocket = INVALID_SOCKET;
				};

				~SendBufferClass(void) {
					delete PacketBuffer;
					PacketBuffer = NULL;
				};

		};

		int NumSendBuffers;
		SendBufferClass *SendBuffers;

		//unsigned char *PacketBuffers;		//[PACKET_MANAGER_BUFFERS][600];
		//unsigned char *IPAddresses;		//[PACKET_MANAGER_BUFFERS][4];
		//unsigned short *Ports;				//[PACKET_MANAGER_BUFFERS];
		//int *PacketLengths					//[PACKET_MANAGER_BUFFERS];
		//bool *PacketReady;					//[PACKET_MANAGER_BUFFERS];
		//int *PacketSendLength;				//[PACKET_MANAGER_BUFFERS];
		//SOCKET *PacketSendSockets;			//[PACKET_MANAGER_BUFFERS];

		int NextPacket;
		int NumPackets;

		unsigned char BuildPacket[PACKET_MANAGER_MTU];
		unsigned char DeltaPacket[PACKET_MANAGER_MTU + 128];

		/*
		** Receive buffers. Don't need so many since we only process one packet at a time.
		*/
		class ReceiveBufferClass {
			public:
				unsigned char ReceiveHoldingBuffer[600];
				unsigned long ReceivePacketLength;
		};

		int NumReceiveBuffers;
		ReceiveBufferClass *ReceiveBuffers;

		//unsigned char ReceiveHoldingBuffers[PACKET_MANAGER_RECEIVE_BUFFERS][600];
		//unsigned long ReceivePacketLengths[PACKET_MANAGER_RECEIVE_BUFFERS];
		unsigned char ReceiveIPAddress[4];
		unsigned short ReceivePort;
		int NumReceivePackets;
		int CurrentPacket;
		SOCKET ReceiveSocket;

		/*
		** Send timing.
		*/
		unsigned long LastSendTime;
		unsigned long FlushFrequency;

		/*
		** Bandwidth measurement.
		*/
		DynamicVectorClass<BandwidthStatsStruct> BandwidthList;
		unsigned long TotalCompressedBandwidthIn;
		unsigned long TotalCompressedBandwidthOut;
		unsigned long TotalUncompressedBandwidthIn;
		unsigned long TotalUncompressedBandwidthOut;
		unsigned long StatsFrequency;
		unsigned long LastStatsUpdate;
		bool ResetStatsIn;
		bool ResetStatsOut;

		/*
		** Debug
		*/
		bool AllowDeltas;
		bool AllowCombos;

		/*
		** Winsock error handling.
		*/
		ErrorStateEnum ErrorState;

		/*
		** Thread safety
		*/
		CriticalSectionClass CriticalSection;

};


/*
** Single instance of the packet manager.
*/
extern PacketManagerClass PacketManager;

#endif //_PACKETMGR_H
