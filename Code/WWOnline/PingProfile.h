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
*     $Archive: /Commando/Code/WWOnline/PingProfile.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 3 $
*     $Modtime: 11/09/01 6:12p $
*
******************************************************************************/

#ifndef __PINGPROFILE_H__
#define __PINGPROFILE_H__

#include <WWOnline/WaitCondition.h>

namespace WWOnline
{
class RawPing;
class Session;
}

typedef struct {unsigned char Pings[8];} PingProfile;

const PingProfile& GetLocalPingProfile(void);
int ComparePingProfile(const PingProfile& ping1, const PingProfile& ping2);
int EncodePingProfile(const PingProfile& pings, char* buffer);
void DecodePingProfile(const char* buffer, PingProfile& pings);

class PingProfileWait :
		public SingleWait,
		protected Observer<WWOnline::RawPing>
	{
	public:
		static RefPtr<PingProfileWait> Create(void);

		void WaitBeginning(void) override;

	protected:
		PingProfileWait();
		virtual ~PingProfileWait();

		PingProfileWait(const PingProfileWait&);
		const PingProfileWait& operator=(const PingProfileWait&);

		void HandleNotification(WWOnline::RawPing& ping) override;

	private:
		RefPtr<WWOnline::Session> mWOLSession;
		unsigned int mCount;
	};

#endif // __PINGPROFILE_H__
