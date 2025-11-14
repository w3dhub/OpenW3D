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
// Filename:     wwpacket.cpp
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:  Adding and extracting data from byte streams
//
//------------------------------------------------------------------------------------
#include "wwpacket.h"

#include "win.h"
#include "systimer.h"
#include "crc.h"
#include "quat.h"
#include "fromaddress.h"
#include "packettype.h"
#include "bitpackids.h"

#define	PACKET_ID_BITS		28		// Enough for about 100 packets per second for about a month.
#define	PACKET_TYPE_BITS	4		// Enough for 16 packet types (we currently have 7 at 9/20/2001 11:33PM)

DEFINE_AUTO_POOL(cPacket, 256)

//
// Class statics
//
#ifdef WRAPPER_CRC
const USHORT	cPacket::PACKET_HEADER_SIZE	= 7;
#else //WRAPPER_CRC
const int		cPacket::CRC_PLACEHOLDER		= 99999;
const USHORT	cPacket::PACKET_HEADER_SIZE	= 11;
#endif //WRAPPER_CRC
int				cPacket::RefCount					= 0;
bool				cPacket::EncoderInit				= true;
const unsigned int cPacket::DefSendTime		= 0xffffffff;

//------------------------------------------------------------------------------------
cPacket::cPacket() :
   Type(UNDEFINED_TYPE),
   Id(UNDEFINED_ID),
   SenderId(UNDEFINED_ID),
	SendTime(DefSendTime),
	FirstSendTime(DefSendTime),
   ResendCount(-1),			// so that first send doesn't count as a resend
	NumSends(1)
{
	//cEncoderList::Set_Compression_Enabled(false);

	RefCount++;

	if (EncoderInit) {
		Init_Encoder();
	}
}

//---------------- --------------------------------------------------------------------
cPacket::~cPacket()
{
	RefCount--;
}

//------------------------------------------------------------------------------------
//
// Assignment operator
//
cPacket& cPacket::operator=(const cPacket& source)
{
	PFromAddressWrapper	= source.PFromAddressWrapper;
   Type						= source.Type;
   Id							= source.Id;
	SenderId					= source.SenderId;
	SendTime					= source.SendTime;
	FirstSendTime			= source.FirstSendTime;
   ResendCount				= source.ResendCount;
#ifndef WRAPPER_CRC
   IsCrcCorrect			= source.IsCrcCorrect;
#endif //WRAPPER_CRC

	BitStreamClass::operator=(source);

	NumSends					= source.NumSends;

   return * this;
}

//------------------------------------------------------------------------------------
void cPacket::Add_Vector3(Vector3 & v)
{
	WWASSERT(v.Is_Valid());

	Add(v.X);
	Add(v.Y);
	Add(v.Z);
}

//------------------------------------------------------------------------------------
void cPacket::Get_Vector3(Vector3 & v)
{
	Get(v.X);
	Get(v.Y);
	Get(v.Z);

	WWASSERT(v.Is_Valid());
}

//------------------------------------------------------------------------------------
void cPacket::Add_Quaternion(Quaternion & q)
{
	WWASSERT(q.Is_Valid());

	Add(q.X);
	Add(q.Y);
	Add(q.Z);
	Add(q.W);
}

//------------------------------------------------------------------------------------
void cPacket::Get_Quaternion(Quaternion & q)
{
	Get(q.X);
	Get(q.Y);
	Get(q.Z);
	Get(q.W);

	WWASSERT(q.Is_Valid());
}

//------------------------------------------------------------------------------------
void cPacket::Set_Type(BYTE type)
{
   WWASSERT(Type == UNDEFINED_TYPE || Type == type);

   WWASSERT(type != UNDEFINED_TYPE);
   Type = type;
}

//------------------------------------------------------------------------------------
void cPacket::Set_Id(int id)
{
   WWASSERT(id != UNDEFINED_ID);
   Id = id;
}

//------------------------------------------------------------------------------------
void cPacket::Set_Sender_Id(int sender_id)
{
   SenderId = sender_id;
}

//------------------------------------------------------------------------------------
void cPacket::Set_Send_Time()
{
	unsigned int time = TIMEGETTIME();
	if (SendTime == DefSendTime) {
		FirstSendTime = time;
	}
	SendTime = time;
}

//------------------------------------------------------------------------------------
void cPacket::Set_Num_Sends(int num_sends)
{
	WWASSERT(num_sends > 0);
	NumSends = num_sends;
}

/*
//------------------------------------------------------------------------------------
BYTE cPacket::Peek_Message_Type() const
{
	//
	// Note that using Get is the only valid way to read data from a packet!
	//

	cPacket temp_packet;
	temp_packet = *this;
	BYTE message_type;
	temp_packet.Get(message_type);
	temp_packet.Flush();

	return message_type;
}
*/


void cPacket::Init_Encoder(void)
{
	EncoderInit = false;
	cEncoderList::Set_Precision(BITPACK_PACKET_ID, PACKET_ID_BITS);
	cEncoderList::Set_Precision(BITPACK_PACKET_TYPE, PACKET_TYPE_BITS);
}


//------------------------------------------------------------------------------------
void cPacket::Construct_Full_Packet(cPacket & full_packet, cPacket & src_packet)
{
   WWASSERT(
		src_packet.Get_Type() >= PACKETTYPE_FIRST &&
		src_packet.Get_Type() <= PACKETTYPE_LAST);
   WWASSERT(src_packet.Get_Id() != UNDEFINED_ID);

#ifndef WRAPPER_CRC
	full_packet.Add(CRC_PLACEHOLDER);
#endif //WRAPPER_CRC
	full_packet.Add(src_packet.Get_Type(), BITPACK_PACKET_TYPE);
   full_packet.Add(src_packet.Get_Id(), BITPACK_PACKET_ID);
   full_packet.Add((BYTE)src_packet.Get_Sender_Id());
	full_packet.Add((USHORT)src_packet.Get_Bit_Length());

	int header_bit_length = full_packet.Get_Bit_Length();
	WWASSERT(header_bit_length == PACKET_HEADER_SIZE * 8);

	memcpy(
		full_packet.Get_Data() + PACKET_HEADER_SIZE,
		src_packet.Get_Data(),
		src_packet.Get_Compressed_Size_Bytes());
	unsigned int whole_bit_length = header_bit_length + src_packet.Get_Bit_Length();
	full_packet.Set_Bit_Length(whole_bit_length);

	//
	// Compute a CRC for all the data following the CRC placeholder
	//
	//ULONG crc = CRC::Memory(
	//	(BYTE *) (full_packet.Get_Data() + sizeof(CRC_PLACEHOLDER)),
	//	full_packet.Get_Max_Size() - sizeof(CRC_PLACEHOLDER));

	// Only CRC the meaningful data in the buffer - not the other 1300ish bytes as well. ST - 9/19/2001 11:18PM
#ifndef WRAPPER_CRC
	ULONG crc = CRC::Memory((BYTE *) (full_packet.Get_Data() + sizeof(CRC_PLACEHOLDER)), (whole_bit_length / 8) - sizeof(CRC_PLACEHOLDER));

	//
	// Overwrite the crc placeholder with the computed crc.
	//
	cPacket temp_packet;
	temp_packet.Add(crc);

	memcpy(
		full_packet.Get_Data(),
		temp_packet.Get_Data(),
		temp_packet.Get_Compressed_Size_Bytes());
#endif //WRAPPER_CRC
}

//------------------------------------------------------------------------------------
void cPacket::Construct_App_Packet(cPacket & packet, cPacket & full_packet)
{
#ifndef WRAPPER_CRC
   int remote_crc;
#endif //WRAPPER_CRC
	BYTE type;
	int packet_id;
	char sender_id;
	USHORT bit_size;

#ifndef WRAPPER_CRC
	full_packet.Get(remote_crc);
#endif //WRAPPER_CRC
	full_packet.Get(type, BITPACK_PACKET_TYPE);
	full_packet.Get(packet_id, BITPACK_PACKET_ID);
	full_packet.Get(sender_id);
	full_packet.Get(bit_size);

#ifdef WRAPPER_CRC
	packet.Set_Type(type);
	packet.Set_Id(packet_id);
	packet.Set_Sender_Id(sender_id);
	packet.Set_Bit_Length(bit_size);
	packet.PFromAddressWrapper = full_packet.PFromAddressWrapper;
	memcpy(packet.Get_Data(), full_packet.Get_Data() + PACKET_HEADER_SIZE, packet.Get_Compressed_Size_Bytes());

#else //WRAPPER_CRC

	if (((bit_size / 8) + PACKET_HEADER_SIZE) < MAX_BUFFER_SIZE) {

		//
		// Only CRC the meaningful data in the buffer - not the other 1300ish bytes as well. ST - 9/19/2001 11:29PM
		//
		int local_crc = CRC::Memory((BYTE *) (full_packet.Get_Data() + sizeof(CRC_PLACEHOLDER)), ((bit_size / 8) + PACKET_HEADER_SIZE) - sizeof(CRC_PLACEHOLDER));

		if (local_crc == remote_crc) {
			packet.Set_Is_Crc_Correct(true);
			packet.Set_Type(type);
			packet.Set_Id(packet_id);
			packet.Set_Sender_Id(sender_id);
			packet.Set_Bit_Length(bit_size);
			packet.PFromAddressWrapper = full_packet.PFromAddressWrapper;

			memcpy(packet.Get_Data(), full_packet.Get_Data() + PACKET_HEADER_SIZE, packet.Get_Compressed_Size_Bytes());
		} else {
			packet.Set_Is_Crc_Correct(false);
		}
	} else {
		packet.Set_Is_Crc_Correct(false);
	}

#endif //WRAPPER_CRC
}













	//ExecuteTime(0),
	//ReturnCode(0),
   //ExecuteTime				= source.ExecuteTime;
   //ReturnCode				= source.ReturnCode;

//------------------------------------------------------------------------------------
/*
void cPacket::Set_Execute_Time(int execute_time)
{
   WWASSERT(execute_time > 0);
   ExecuteTime = execute_time;
}
*/


   /*
   if (!Is_Flushed()) {
		WWDEBUG_SAY(("*** cPacket::~cPacket: !Is_Flushed()\n"));
      DIE;
   }
	*/