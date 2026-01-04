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
*     $Archive: /Commando/Code/Commando/WOLBuddyMgr.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 14 $
*     $Modtime: 1/31/02 2:52p $
*
******************************************************************************/

#ifndef __WOLBUDDYMGR_H__
#define __WOLBUDDYMGR_H__

#include <WWOnline/RefPtr.h>
#include <WWOnline/WOLSession.h>
#include <WWOnline/WOLPageMsg.h>
#include <wwlib/refcount.h>
#include <wwlib/Notify.h>
#include <wwlib/Signaler.h>
#include <wwlib/widestring.h>
#include <vector>

class DlgPasswordPrompt;
class WOLBuddyMgr;

typedef enum
	{
	BUDDYLIST_CHANGED = 1,
	IGNORELIST_CHANGED,
	BUDDYINFO_CHANGED
	} WOLBuddyMgrAction;

typedef TypedActionPtr<WOLBuddyMgrAction, WOLBuddyMgr> WOLBuddyMgrEvent;

typedef enum
	{
	PAGE_ERROR = 0,
	PAGE_NOT_THERE = 1,
	PAGE_TURNED_OFF,
	PAGE_SENT,
	PAGE_RECEIVED,
	INVITATION_RECEIVED,
	INVITATION_DECLINED
	} WOLPagedAction;

typedef TypedActionPtr<WOLPagedAction, WWOnline::PageMessage> WOLPagedEvent;

class WOLBuddyMgr :
		public RefCountClass,
		public Notifier<WOLBuddyMgrEvent>,
		public Notifier<WOLPagedEvent>,
		protected Observer<WWOnline::BuddyEvent>,
		protected Observer<WWOnline::UserEvent>,
		protected Observer<WWOnline::PageMessage>,
		protected Observer<WWOnline::PageSendStatus>,
		protected Signaler<DlgPasswordPrompt>
	{
	public:
		typedef std::vector< WideStringClass > IgnoreList;
		typedef WWOnline::PageMessageList PageList;

		typedef enum
			{
			DECLINE_MIN = 0,
			DECLINE_BYUSER = 1,
			DECLINE_NOTBUDDY,
			DECLINE_NOTAPPLICABLE,
			DECLINE_BUSY,
			DECLINE_MAX
			} DECLINE_REASON;

		static WOLBuddyMgr* GetInstance(bool createOK);

		// Get a description of a users location.
		static void GetLocationDescription(const RefPtr<WWOnline::UserData>& user, WideStringClass& description);

		// Request the buddy list anew.
		void RefreshBuddyList(void);
		
		// Get list of buddies
		const WWOnline::UserList& GetBuddyList(void) const
			{return mWOLSession->GetBuddyList();}
		
		// Get a user from our buddy list
		const RefPtr<WWOnline::UserData> FindBuddy(const unichar_t* name) const;

		// Add a user to our buddy list
		void AddBuddy(const unichar_t* name);

		// Remove a user from our buddy list
		void RemoveBuddy(const unichar_t* name);

		// Check if a user is in our buddy list
		bool IsBuddy(const unichar_t* name) const;

		// Request update to the location of our buddies.
		void RefreshBuddyInfo(void);

		// Add a user to the ignore list
		void AddIgnore(const unichar_t* name);

		// Remove a user from the ignore list.
		void RemoveIgnore(const unichar_t* name);

		// Check if a user is in the ignore list.
		bool IsIgnored(const unichar_t* name) const;

		// Get a list of usernames we are ignoring
		const IgnoreList& GetIngoreList(void) const
			{return mIgnoreList;}
	
		const PageList& GetPageList(void) const
			{return mPageList;}

		// Enable the display of a dialog when paged.
		// This function sets an internal counter that determines whether a dialog
		// should be displayed when a page message is received.
		void ShowPagedDialog(void);

		// Disable the display of a dialog when paged.
		// This function sets an internal counter that determines whether a dialog
		// should be displayed when a page message is received.
		void HidePagedDialog(void);

		// Clear all the queued pages.
		void ClearPageList(void);

		// Get the name of the last user who we received a page from.
		const unichar_t* GetLastPagersName(void) const
			{return mLastPagersName;}

		// Send a page message to user
		void PageUser(const unichar_t* name, const unichar_t* message);

		// Join a user at his location
		void JoinUser(const RefPtr<WWOnline::UserData>& user);

		// Check if we are in a position to invite users.
		bool CanInviteUsers(void) const;

		// Invite a user to our location
		void InviteUser(const unichar_t* username, const unichar_t* message);	

		// Decline an invitation to join a user.
		void DeclineInvitation(const unichar_t* username, DECLINE_REASON reason = DECLINE_BYUSER);

	protected:
		WOLBuddyMgr();
		~WOLBuddyMgr();

		bool FinalizeCreate(void);

		void LoadIgnoreList(void);
		void SaveIgnoreList(void);

		bool IsCommand(const unichar_t* message);
		
		void ProcessPendingJoin(void);
		void GotoPendingJoinLocation(const unichar_t* password);
		void ReceiveSignal(DlgPasswordPrompt&) override;

		bool IsInvitation(const unichar_t* message);
		void InvitationReceived(WWOnline::PageMessage& page);
		void DisplayInvitation(const RefPtr<WWOnline::UserData>& user, const unichar_t* message);

		bool IsInvitationDeclined(const unichar_t* message);
		void InvitationDeclined(const unichar_t* username, DECLINE_REASON reaon);

		void HandleNotification(WWOnline::BuddyEvent&) override;
		void HandleNotification(WWOnline::UserEvent&) override;
		void HandleNotification(WWOnline::PageMessage&) override;
		void HandleNotification(WWOnline::PageSendStatus&) override;

		DECLARE_NOTIFIER(WOLBuddyMgrEvent)
		DECLARE_NOTIFIER(WOLPagedEvent)

	private:
		static WOLBuddyMgr* _mInstance;
		RefPtr<WWOnline::Session> mWOLSession;

		IgnoreList mIgnoreList;

		PageList mPageList;
		PageList mInvitations;
		WideStringClass mLastPagersName;
		unsigned int mHidePagedDialog;

		RefPtr<WWOnline::UserData> mPendingJoin;
		};

#endif //__WOLBUDDYMGR_H__
