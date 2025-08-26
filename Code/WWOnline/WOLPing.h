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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/WWOnline/WOLPing.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 3 $
*     $Modtime: 8/28/01 2:25p $
*
******************************************************************************/

#ifndef __WOLPING_H__
#define __WOLPING_H__

#include <wwlib/wwstring.h>

namespace WWOnline {

class RawPing
	{
	public:
		RawPing(const char* host, unsigned long ip, int time) :
				mHostAddress(host),
				mIP(ip),
				mTime(time),
				mHandle(-1)
			{}

		inline const char* GetHostAddress(void) const
			{return mHostAddress.Peek_Buffer();}

		inline void SetIPAddress(unsigned long ip)
			{mIP = ip;}

		inline unsigned long GetIPAddress(void) const
			{return mIP;}

		inline void SetTime(int time)
			{mTime = time;}

		inline int GetTime(void) const
			{return mTime;}

		inline void SetHandle(int handle)
			{mHandle = handle;}

		inline int GetHandle(void) const
			{return mHandle;}

	private:
		StringClass mHostAddress;
		unsigned long mIP;
		int mTime;
		int mHandle;
	};

}

#endif // __WOLPING_H__
