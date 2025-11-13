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
// Filename:     msgstat.h
// Project:      
// Author:       Tom Spencer-Smith
// Date:         
// Description:  Send and receive stats for a single type of message.
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef MSGSTAT_H
#define MSGSTAT_H

#include "bittype.h"

#ifndef NULL
#define NULL 0L
#endif

//-----------------------------------------------------------------------------
class cMsgStat
{
	public:
		cMsgStat(void);
		~cMsgStat(void);

		void		Increment_Num_Msg_Sent(		int increment = 1);
		void		Increment_Num_Byte_Sent(	int increment);
		void		Increment_Num_Msg_Recd(		int increment = 1);
		void		Increment_Num_Byte_Recd(	int increment);

		unsigned int		Get_Num_Msg_Sent(void)	const	{return NumMsgSent;}
		unsigned int		Get_Num_Byte_Sent(void)	const	{return NumByteSent;}
		unsigned int		Get_Num_Msg_Recd(void)	const	{return NumMsgRecd;}
		unsigned int		Get_Num_Byte_Recd(void)	const	{return NumByteRecd;}

		unsigned int		Compute_Avg_Num_Byte_Sent(void) const;
		unsigned int		Compute_Avg_Num_Byte_Recd(void) const;

		void		Set_Name(const char *name);
		const char *Get_Name(void) const				{return Name;}

	private:
      cMsgStat(const cMsgStat& source);					// disallow
      cMsgStat& operator=(const cMsgStat& source);	// disallow

		unsigned int		NumMsgSent;
		unsigned int		NumByteSent;
		unsigned int		NumMsgRecd;
		unsigned int		NumByteRecd;

		char		Name[30];
};

//-----------------------------------------------------------------------------

#endif // MSGSTAT_H


