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
*     $Archive: /Commando/Code/WWOnline/WOLNetUtilObserver.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 25 $
*     $Modtime: 2/21/02 5:36p $
*
******************************************************************************/

#include <windows.h>
#include "atlbase_compat.h"
#include "WOLNetUtilObserver.h"
#include "WOLSession.h"
#include "WOLLadder.h"
#include "WOLServer.h"
#include "WOLErrorUtil.h"

namespace WWOnline {

/******************************************************************************
*
* NAME
*     NetUtilObserver::NetUtilObserver
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

NetUtilObserver::NetUtilObserver() :
		mRefCount(1),
		mOuter(NULL)
	{
	WWDEBUG_SAY(("WOL: NetUtilObserver Instantiated\n"));
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::~NetUtilObserver
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

NetUtilObserver::~NetUtilObserver()
	{
	WWDEBUG_SAY(("WOL: NetUtilObserver Destroyed\n"));
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::Init
*
* DESCRIPTION
*     Initialize Net utility observer
*
* INPUTS
*     WOLSession - Outer session
*
* RESULT
*     NONE
*
******************************************************************************/

void NetUtilObserver::Init(Session& outer)
	{
	mOuter = &outer;
	}


/****************************************************************************
*
* NAME
*     IUnknown::QueryInterface
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
****************************************************************************/

STDMETHODIMP NetUtilObserver::QueryInterface(const IID& iid, void** ppv)
	{
	if ((iid == IID_IUnknown) || (iid == WOL::IID_INetUtilEvent))
		{
		*ppv = static_cast<WOL::INetUtilEvent*>(this);
		}
	else
		{
		*ppv = NULL;
		return E_NOINTERFACE;
		}

	static_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
	}


/****************************************************************************
*
* NAME
*     IUnknown::AddRef
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
****************************************************************************/

ULONG STDMETHODCALLTYPE NetUtilObserver::AddRef(void)
	{
	InterlockedIncrement((LPLONG)&mRefCount);
	return mRefCount;
	}


/****************************************************************************
*
* NAME
*     IUnknown::Release
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
****************************************************************************/

ULONG STDMETHODCALLTYPE NetUtilObserver::Release(void)
	{
	InterlockedDecrement((LPLONG)&mRefCount);

	if (mRefCount == 0)
		{
		delete this;
		return 0;
		}

	return mRefCount;
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::OnPing
*
* DESCRIPTION
*     Handle ping result response to a ping request
*
* INPUTS
*     Result - Error / Status code
*     Time   - Ping time
*     IP     - IP Address
*     Handle - Ping request handle
*
* RESULT
*
******************************************************************************/

STDMETHODIMP NetUtilObserver::OnPing(HRESULT result, int time, unsigned int ip, int handle)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPing '%s'\n", GetNetUtilErrorString(result)));
		return S_OK;
		}

	// Find the ping request
	int pingIndex = -1;

	for (unsigned int index = 0; index < mOuter->mPingRequests.size(); index++)
		{
		if (mOuter->mPingRequests[index].GetHandle() == handle)
			{
			pingIndex = index;
			break;
			}
		}

	if ((pingIndex >= 0) && ((unsigned)pingIndex < mOuter->mPingRequests.size()))
		{
		RawPing* ping = &mOuter->mPingRequests[pingIndex];
		ping->SetTime(time);
		ping->SetIPAddress(ip);

		WWDEBUG_SAY(("WOL: OnPing %s = %d\n", ping->GetHostAddress(), time));

		// Automatically update ping time for ping servers.
		mOuter->UpdatePingServerTime(ping->GetHostAddress(), time);

		// Notify others about the ping result.
		mOuter->NotifyObservers(*ping);
	
		// Remove ping from request list.
		std::vector<RawPing>::iterator iter = mOuter->mPingRequests.begin();

		while (iter != mOuter->mPingRequests.end())
			{
			if (&(*iter) == ping)
				{
				mOuter->mPingRequests.erase(iter);
				mOuter->mPingsPending--;
				break;
				}

			iter++;
			}
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::OnLadderList
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP NetUtilObserver::OnLadderList(HRESULT result, WOL::Ladder* list,
			int /* rungCount */, int timeStamp, int keyRung)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result) || (list == NULL))
		{
		WWDEBUG_SAY(("WOLERROR: OnLadderList '%s'\n", GetNetUtilErrorString(result)));

		// Clear the ladder type we requested (in order)
		if (mOuter->mLadderPending & LadderType_Individual)
			{
			mOuter->mLadderPending &= ~LadderType_Individual;
			}
		else if (mOuter->mLadderPending & LadderType_Team)
			{
			mOuter->mLadderPending &= ~LadderType_Team;
			}
		else if (mOuter->mLadderPending & LadderType_Clan)
			{
			mOuter->mLadderPending &= ~LadderType_Clan;
			}
		else
			{
			mOuter->mLadderPending &= ~LADDERTYPE_MASK;
			}

		// If there are no pending ladders then remove the satisfied requests
		if ((mOuter->mLadderPending & LADDERTYPE_MASK) == 0)
			{
			for (unsigned int count = 0; count < mOuter->mLadderPending; count++)
				{
				mOuter->mLadderRequests.pop_front();
				}

			mOuter->mLadderPending = 0;
			}

		return S_OK;
		}

	//---------------------------------------------------------------------------
	// Handle ladder information for individual users.
	//---------------------------------------------------------------------------
	if (keyRung == -1)
		{
		ProcessLadderListResults(list, timeStamp);
		return S_OK;
		}

	//---------------------------------------------------------------------------
	// Handle ladder search results
	//---------------------------------------------------------------------------
	LadderList ladders;

	WOL::Ladder* wolLadder = list;

	while (wolLadder)
		{
		RefPtr<LadderData> data = LadderData::Create(*wolLadder, timeStamp);

		if (data.IsValid())
			{
			ladders.push_back(data);
			}

		wolLadder = wolLadder->next;
		}

	mOuter->NotifyObservers(ladders);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::ProcessLadderListResults
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void NetUtilObserver::ProcessLadderListResults(WOL::Ladder* list, int timeStamp)
	{
	if (mOuter->mLadderPending)
		{
		// Within a given request, ladder results are guaranteed to be in order.
		WOL::Ladder* wolLadder = list;

		// Extract the ladder request type from the hiword of the SKU
		LadderType type = (LadderType)(wolLadder->sku & LADDERTYPE_MASK);

		if (type == 0)
			{
			type = LadderType_Individual;
			}

		Session::LadderRequestList::iterator request = mOuter->mLadderRequests.begin();
		unsigned int ladderCount = 0;

		while (wolLadder)
			{
			// Get the name of the user we requested information for.
			const unichar_t* requestName = u_strchr(*request, U_CHAR(':'));
			WWASSERT(requestName != NULL && "Invalid ladder request");
			requestName++;

			unichar_t ladderName[64];
			u_mbtows(ladderName, (const char*)wolLadder->login_name, sizeof(wolLadder->login_name));

			WWDEBUG_SAY(("WOL: LadderInfo [%08lX] Requested '%S', Received '%S'\n", type, requestName, ladderName));

			// If the ladder name matches the requested name then there is ladder info available.
			bool hasLadderData = (u_strcasecmp(requestName, ladderName, U_COMPARE_CODE_POINT_ORDER) == 0);

			if (type == LadderType_Clan)
				{
				RefPtr<SquadData> squad = SquadData::FindByAbbr(requestName);

				if (squad.IsValid())
					{
					// Update / create ladder information for this clan
					RefPtr<LadderData> ladderData = squad->GetLadder();

					if (ladderData.IsValid())
						{
						ladderData->UpdateData(*wolLadder, timeStamp);
						}
					else if (hasLadderData)
						{
						ladderData = LadderData::Create(*wolLadder, timeStamp);
						WWASSERT(ladderData.IsValid());
						squad->SetLadder(ladderData);
						}

					// Notify others about updated clan ladder information
					NotifyClanLadderUpdate(mOuter->mUsers, squad);
					NotifyClanLadderUpdate(mOuter->mBuddies, squad);
					}
				}
			else
				{
				RefPtr<UserData> user = mOuter->GetUserOrBuddy(requestName);

				// Update users with new ladder information.
				if (user.IsValid())
					{
					RefPtr<LadderData> ladderData = user->GetLadderFromType(type);

					if (ladderData.IsValid())
						{
						ladderData->UpdateData(*wolLadder, timeStamp);
						}															 
					else if (hasLadderData)
						{
						ladderData = LadderData::Create(*wolLadder, timeStamp);
						WWASSERT(ladderData.IsValid());

						if (ladderData.IsValid())
							{
							user->SetLadderFromType(ladderData, type);
							}
						}

					// Notify others that the ladder information for this user has changed.
					UserEvent event(UserEvent::LadderInfo, user);
					mOuter->NotifyObservers(event);
					}
				}

			// Notify others about the raw ladder information
			LadderInfoEvent ladderEvent(ladderName, *wolLadder, timeStamp);
			mOuter->NotifyObservers(ladderEvent);

			request++;
			ladderCount++;
			wolLadder = wolLadder->next;
			}

		// Clear the ladder type we received
		mOuter->mLadderPending &= ~type;

		// If there are no pending ladders then remove the satisfied requests
		if ((mOuter->mLadderPending & LADDERTYPE_MASK) == 0)
			{
			WWASSERT(ladderCount == mOuter->mLadderPending && "LadderInfo received doesn't match number requested");

			for (unsigned int count = 0; count < mOuter->mLadderPending; ++count)
				{
				mOuter->mLadderRequests.pop_front();
				}

			mOuter->mLadderPending = 0;
			}
		}
	}


void NetUtilObserver::NotifyClanLadderUpdate(const UserList& users, const RefPtr<SquadData>& squad)
	{
	const size_t userCount = users.size();

	for (size_t index = 0; index < userCount; ++index)
		{
		const RefPtr<UserData>& user = users[index];

		if (user.IsValid() && (user->GetSquadID() == squad->GetID()))
			{
			// Notify others that this user received squad information
			UserEvent event(UserEvent::LadderInfo, user);
			mOuter->NotifyObservers(event);
			}
		}
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::OnGameresSent
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP NetUtilObserver::OnGameresSent(HRESULT)
	{
	WWDEBUG_SAY(("WOLWARNING: OnGameresSent() not implemented\n"));
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::OnNewNick
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP NetUtilObserver::OnNewNick(HRESULT result, LPCSTR message, LPCSTR nickname, LPCSTR password)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnNewNick '%s'\n", GetNetUtilErrorString(result)));
		return S_OK;
		}

	RefPtr<LoginInfo> login;

	if (result == S_OK)
		{
		login = LoginInfo::Create(nickname, password, true);
		}
		
	NewLoginInfoEvent event(login, message);
	mOuter->NotifyObservers(event);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::OnAgeCheck
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP NetUtilObserver::OnAgeCheck(HRESULT result, int years, int consent)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnAgeCheck '%s'\n", GetNetUtilErrorString(result)));
		return S_OK;
		}

	AgeCheckEvent event(years, consent != 0);
	mOuter->NotifyObservers(event);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::OnWDTState
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP NetUtilObserver::OnWDTState(HRESULT /* result */, unsigned char* , int )
	{
	WWDEBUG_SAY(("WOLWARNING: OnWDTState not implemented\n"));
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     NetUtilObserver::OnHighscore
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP NetUtilObserver::OnHighscore(HRESULT /* result */, WOL::Highscore* /* list */,
		int /* count */, int /* time */, int /* keyRung */)
	{
	WWDEBUG_SAY(("WOLWARNING: OnHighscore not implemented\n"));
	return S_OK;
	}

} // namespace WWOnline
