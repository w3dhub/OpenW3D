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
*     $Archive: /Commando/Code/wwui/IMECandidate.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 1/11/02 6:44p $
*
******************************************************************************/

#ifndef __IMECANDIDATE_H__
#define __IMECANDIDATE_H__

#include "Notify.h"
#include "win.h"
#include "unichar.h"
#include <imm.h>
#include <vector>

namespace IME {

class IMECandidate
	{
	public:
		IMECandidate();
		~IMECandidate();

		void Open(int index, HWND hwnd, UINT codepage, bool unicode, bool startFrom1);
		void Read(void);
		void Close(void);

		bool IsValid(void) const;

		int GetIndex(void) const;

		unsigned int GetStyle(void) const;

		// Get the index of the first candidate in the page
		unsigned int GetPageStart(void) const;

		// Set the page to start with the specified candidate index
		void SetPageStart(unsigned int);

		// Get the number of candidates per page
		unsigned int GetPageSize(void) const;

		// Get the total number of candidates in the list.
		unsigned int GetCount(void) const;

		// Get the index of the current candidate selection
		unsigned int GetSelection(void) const;

		// Get the specified candidate string
		const unichar_t* GetCandidate(unsigned int index);

		// Select a candidate from the list.
		void SelectCandidate(unsigned int index);

		// Set the candidate page view
		void SetView(unsigned int topIndex, unsigned int bottomIndex);

		// Check if the candidates should be displayed starting from 1 or 0
		bool IsStartFrom1(void) const;

	private:
		int mIndex;
		HWND mHWND;
		UINT mCodePage;
		bool mUseUnicode;
		bool mStartFrom1;

		unsigned int mCandidateSize;
		CANDIDATELIST* mCandidates;

		// Multibyte -> Unicode string conversion buffer
		wchar_t mTempString[80];
	};


typedef enum
	{
	CANDIDATE_OPEN = 1,
	CANDIDATE_CHANGE,
	CANDIDATE_CLOSE
	} CandidateAction;

typedef TypedActionPtr<CandidateAction, IMECandidate> CandidateEvent;

typedef std::vector<IMECandidate> IMECandidateCollection;

} // namespace IME

#endif // __IMECANDIDATE_H__

