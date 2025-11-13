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
// Filename:     wwpacket.h
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef WWPACKET_H
#define WWPACKET_H

#include "bittype.h"
#include "bitstream.h"
#include "fromaddress.h"
#include "packetmgr.h"
#include "mempool.h"

#ifndef NULL
#define NULL 0L
#endif

class Vector3;
class Quaternion;
class cFromAddress;

//-----------------------------------------------------------------------------
class cPacket : public BitStreamClass, public AutoPoolClass<cPacket, 256>
{
	public:
		cPacket();
		~cPacket();

      cPacket& operator=(const cPacket& source);

		enum {NO_ENCODER			= -1};
		enum {UNDEFINED_TYPE		= 15};
		enum {UNDEFINED_ID		= -1};

		int				Get_Max_Size() const					{return BitStreamClass::Get_Buffer_Size();}
      void				Set_Bit_Length(UINT bit_position){BitStreamClass::Set_Bit_Write_Position(bit_position);}
      UINT				Get_Bit_Length(void)					{return BitStreamClass::Get_Bit_Write_Position();}
		void				Add_Vector3(Vector3 & v);
		void				Get_Vector3(Vector3 & v);
      void				Add_Quaternion(Quaternion & q);
		void				Get_Quaternion(Quaternion & q);
		const cFromAddress *	Get_From_Address_Wrapper() const {return &PFromAddressWrapper;}
		cFromAddress *	Get_From_Address_Wrapper() {return &PFromAddressWrapper;}
      void				Set_Type(BYTE type);
      BYTE				Get_Type() const						{return Type;}
		void				Set_Id(int id);
      int				Get_Id() const							{WWASSERT(Id != UNDEFINED_ID); return Id;}//NEW
		void				Set_Sender_Id(int sender_id);
      int				Get_Sender_Id() const				{return SenderId;}
		void				Set_Send_Time(void);
		unsigned int	Get_Send_Time() const				{return SendTime;}
		void				Clear_Resend_Count()					{ResendCount = 0;}
		void				Increment_Resend_Count()			{ResendCount++;}
      int				Get_Resend_Count() const			{return ResendCount;}
#ifndef WRAPPER_CRC
		bool				Is_Crc_Correct() const				{return IsCrcCorrect;}
#endif //WRAPPER_CRC
		void				Set_Num_Sends(int num_sends);
		int				Get_Num_Sends() const				{return NumSends;}
		unsigned int	Get_First_Send_Time(void) const	{return FirstSendTime;}
		static void		Init_Encoder(void);
		static int		Get_Ref_Count()						{return RefCount;}
		static void		Construct_Full_Packet(cPacket & full_packet, cPacket & src_packet);
		static void		Construct_App_Packet(cPacket & packet, cPacket & full_packet);
		static USHORT	Get_Packet_Header_Size(void)		{return (PACKET_HEADER_SIZE);}
		//BYTE				Peek_Message_Type() const;
		static unsigned int Get_Default_Send_Time(void) {return(DefSendTime);}

	private:
      cPacket(const cPacket& source); // Disallow

#ifndef WRAPPER_CRC
		void				Set_Is_Crc_Correct(bool flag)		{IsCrcCorrect = flag;}

		static const int		CRC_PLACEHOLDER;
#endif //WRAPPER_CRC
		static const USHORT	PACKET_HEADER_SIZE;
		static const unsigned int DefSendTime;

		cFromAddress	PFromAddressWrapper;
		BYTE				Type;
      int				Id;
      int				SenderId;
      unsigned int	SendTime;
		unsigned int	FirstSendTime;
      int				ResendCount;
#ifndef WRAPPER_CRC
		bool				IsCrcCorrect;
#endif //WRAPPER_CRC
		int				NumSends;
		static int		RefCount;
		static bool		EncoderInit;
};



//-----------------------------------------------------------------------------

#endif // WWPACKET_H











      //int ExecuteTime;
      //int ReturnCode;



		//void Set_Execute_Time(int execute_time);
      //int Get_Execute_Time() const			{return ExecuteTime;}

		//void Set_Return_Code(int return_code) {ReturnCode = return_code;}
      //int Get_Return_Code() const			{return ReturnCode;}

      //BYTE Get_Type() const					{WWASSERT(Type != UNDEFINED_TYPE); return Type;}//NEW



//
// Maximum Transmission Unit (MTU) for LAN is 1500.
//
//const int		CRC_PLACEHOLDER							= 99999;
//const USHORT	PACKET_HEADER_SIZE						= 17;
//const USHORT	MAX_LAN_PACKET_APP_DATA_SIZE			= MAX_BUFFER_SIZE;
//const USHORT	MAX_INTERNET_PACKET_APP_DATA_SIZE	= 512;
//const USHORT	MAX_PACKET_APP_DATA_SIZE				= 512;