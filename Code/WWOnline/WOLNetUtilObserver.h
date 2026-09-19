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
*     $Archive: /Commando/Code/WWOnline/WOLNetUtilObserver.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 1/22/02 5:53p $
*
******************************************************************************/

#ifndef __WOLNETUTILOBSERVER_H__
#define __WOLNETUTILOBSERVER_H__

#include <windows.h>
#include "WOLUser.h"

#include <wolapi/WOLAPI.h>

template<typename T> class RefPtr;

namespace WWOnline {

class Session;
class SquadData;

class NetUtilObserver :
		public WOL::INetUtilEvent
	{
	public:
		NetUtilObserver();

		void Init(Session& outer);

		//---------------------------------------------------------------------------
		// IUnknown methods
		//---------------------------------------------------------------------------
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv) override;
		virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
		virtual ULONG STDMETHODCALLTYPE Release(void) override;

		//---------------------------------------------------------------------------
		// INetUtilEvent Methods
		//---------------------------------------------------------------------------
		WOLAPI_STDMETHOD(OnPing)(WOL::WOLAPI_RESULT hr, int time, unsigned int ip, int handle) override;

		WOLAPI_STDMETHOD(OnLadderList)(WOL::WOLAPI_RESULT hr, WOL::Ladder* list, int count, int time, int keyRung) override;

		WOLAPI_STDMETHOD(OnGameresSent)(WOL::WOLAPI_RESULT hr) override;

		WOLAPI_STDMETHOD(OnNewNick)(WOL::WOLAPI_RESULT hr, LPCSTR message, LPCSTR nick, LPCSTR pass) override;

		WOLAPI_STDMETHOD(OnAgeCheck)(WOL::WOLAPI_RESULT hr, int years, int consent) override;

		WOLAPI_STDMETHOD(OnWDTState)(WOL::WOLAPI_RESULT hr, unsigned char* state, int length) override;

		WOLAPI_STDMETHOD(OnHighscore)(WOL::WOLAPI_RESULT hr, WOL::Highscore* list, int count, int time, int keyRung) override;

	protected:
		virtual ~NetUtilObserver();

		NetUtilObserver(const NetUtilObserver&);
		const NetUtilObserver& operator=(const NetUtilObserver&);

		void ProcessLadderListResults(WOL::Ladder* list, int timeStamp);
		void NotifyClanLadderUpdate(const UserList& users, const RefPtr<SquadData>& squad);

	private:
		ULONG mRefCount;
		Session* mOuter;
	};

}

#endif // _WOLNETUTILOBSERVER_H__
