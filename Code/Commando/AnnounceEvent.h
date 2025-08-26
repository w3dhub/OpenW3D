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
*     $Archive: /Commando/Code/commando/AnnounceEvent.h $
*
* DESCRIPTION
*     Client announcement
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Patrick $
*
* VERSION INFO
*     $Revision: 5 $
*     $Modtime: 11/28/01 4:22p $
*
******************************************************************************/

#ifndef __ANNOUNCEEVENT_H__
#define __ANNOUNCEEVENT_H__

#include "netevent.h"
#include "netclassids.h"

enum AnnouncementEnum
	{
	ANNOUNCEMENT_PUBLIC = 0,
	ANNOUNCEMENT_TEAM,
	ANNOUNCEMENT_PRIVATE,
	};

class	CSAnnouncement :
		public cNetEvent
	{
	public:
		CSAnnouncement();
		virtual ~CSAnnouncement();

		void Init(int to_id, int announcementID, AnnouncementEnum type, int radio_cmd_id = -1);

		virtual void Export_Creation(BitStreamClass& packet);
		virtual void Import_Creation(BitStreamClass& packet);

		virtual uint32 Get_Network_Class_ID(void) const
			{return NETCLASSID_CSANNOUNCEMENT;}

	protected:
		CSAnnouncement(const CSAnnouncement&);
		const CSAnnouncement& operator=(const CSAnnouncement&);

		virtual void Act(void);

		int mToID;
		int mFromID;
		int mAnnouncementID;
		int mRadioCmdID;
		AnnouncementEnum mType;
	};


class	SCAnnouncement :
		public cNetEvent
	{
	public:
		SCAnnouncement();
		virtual ~SCAnnouncement();

		void Init(int to_id, int from_id, int announcementID, AnnouncementEnum type, int radio_cmd_id = -1);

		virtual void Export_Creation(BitStreamClass& packet);
		virtual void Import_Creation(BitStreamClass& packet);

		virtual uint32 Get_Network_Class_ID(void) const
			{return NETCLASSID_SCANNOUNCEMENT;}

	protected:
		SCAnnouncement(const SCAnnouncement&);
		const SCAnnouncement& operator=(const SCAnnouncement&);

		virtual void Act(void);
		void Set_Dirty_Bit_For_Team(DIRTY_BIT bit, int team);

		int mToID;
		int mFromID;
		int mAnnouncementID;
		int mRadioCmdID;
		AnnouncementEnum mType;
	};

#endif // __ANNOUNCEEVENT_H__
