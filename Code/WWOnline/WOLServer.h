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
*     $Archive: /Commando/Code/WWOnline/WOLServer.h $
*
* DESCRIPTION
*     These classes encapsulate a Westwood Online Server.
*
*     This is a base class. Derived classes include (but not necessarily limited to) 
*     ChatServer, GameResultsServer, LadderServer, and WDTServer.
*
*     Server primarily repackages the WOL Server struct
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 11 $
*     $Modtime: 1/11/02 5:40p $
*
******************************************************************************/

#ifndef __WOLSERVER_H__
#define __WOLSERVER_H__

#include "RefCounted.h"
#include "RefPtr.h"
#include "WOLString.h"
#include <wwlib/wwstring.h>

namespace WOL
{
#include <wolapi/WOLAPI.h>
}

namespace WWOnline {

class ServerData :
		public RefCounted
	{
	public:
		inline WOL::Server& GetData(void)
			{return mData;}

		inline const int GetTimeZone(void) const
			{return mData.timezone;}

		inline const float GetLongitude(void) const
			{return mData.longitude;}

		inline const float GetLattitude(void) const
			{return mData.lattitude;}

	protected:
		ServerData(const WOL::Server&);
		virtual ~ServerData();

		WOL::Server mData;
	};


// base class for chat and game servers
class IRCServerData :
		public ServerData
	{
	public:
		static RefPtr<IRCServerData> Create(const WOL::Server&);

		inline const char* GetName(void) const
			{return mServerName;}

		inline bool IsGameServer(void) const
			{return mIsGameServer;}

		inline bool HasLanguageCode(void) const
			{return mHasLanguageCode;}

		inline bool MatchesLanguageCode(void) const
			{return mMatchesLanguageCode;}

	protected:
		IRCServerData(const WOL::Server&);
		virtual ~IRCServerData()
			{}

	private:
		StringClass mServerName;
		bool mIsGameServer;
		bool mMatchesLanguageCode;
		bool mHasLanguageCode;
	};


// base class for non-chat/non-game servers
class HostPortServerData :
		public ServerData
	{
	public:
		inline const char* GetName(void) const
			{return (const char*)mData.name;}

		inline const char* GetHostAddress(void) const
			{return mHostAddress;}

		inline unsigned int GetPort(void) const
			{return mHostPort;}

	protected:
		HostPortServerData(const WOL::Server&);
		virtual ~HostPortServerData()
			{}

	private:
		StringClass mHostAddress;
		unsigned int mHostPort;
	};


class LadderServerData :
		public HostPortServerData
	{
	public:
		static RefPtr<LadderServerData> Create(const WOL::Server&);

	private:
		LadderServerData(const WOL::Server&);
		virtual ~LadderServerData()
			{}

		LadderServerData(const LadderServerData&);
		const LadderServerData& operator=(const LadderServerData&);
	};


class GameResultsServerData :
		public HostPortServerData
	{
	public:
		static RefPtr<GameResultsServerData> Create(const WOL::Server&);

	private:
		GameResultsServerData(const WOL::Server&);
		virtual ~GameResultsServerData()
			{}

		GameResultsServerData(const GameResultsServerData&);
		const GameResultsServerData& operator=(const GameResultsServerData&);
	};


class WDTServerData :
		public HostPortServerData
	{
	public:
		static RefPtr<WDTServerData> Create(const WOL::Server&);

	private:
		WDTServerData(const WOL::Server&);
		virtual ~WDTServerData()
			{}

		WDTServerData(const WDTServerData&);
		const WDTServerData& operator=(const WDTServerData&);
	};


class MGLServerData :
		public HostPortServerData
	{
	public:
		static RefPtr<MGLServerData> Create(const WOL::Server&);

	private:
		MGLServerData(const WOL::Server&);
		virtual ~MGLServerData()
			{}

		MGLServerData(const MGLServerData&);
		const MGLServerData& operator=(const MGLServerData&);
	};


class PingServerData :
		public HostPortServerData
	{
	public:
		static RefPtr<PingServerData> Create(const WOL::Server&);

		void SetPingTime(int time);

		inline int GetPingTime(void) const
			{return mPingTime;}

	protected:
		PingServerData(const WOL::Server&);
		virtual ~PingServerData()
			{}

		// Prevent copy and assignment
		PingServerData(const PingServerData&);
		const PingServerData& operator=(const PingServerData&);

	private:
		int mPingTime;
	};


class ServerError
	{
	public:
		ServerError(int code, const char* description, unsigned long data = 0) :
				mCode(code),
				mDescription(description),
				mData(data)
			{}

		inline int GetErrorCode(void)
			{return mCode;}

		inline const wchar_t* GetDescription(void) const
	 		{return WOLSTRING(mDescription);}

		inline unsigned int GetData(void) const
			{return mData;}

	private:
		ServerError(const ServerError&);
		const ServerError& operator=(const ServerError&);

		int mCode;
		StringClass mDescription;
		unsigned long mData;
	};

} // namespace WWOnline

#endif // __WOLSERVER_H__
