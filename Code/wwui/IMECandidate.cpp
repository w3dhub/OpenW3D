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
*     $Archive: /Commando/Code/wwui/IMECandidate.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 1/12/02 9:43p $
*
******************************************************************************/

#include "IMECandidate.h"
#include "wwdebug.h"

namespace IME {

/******************************************************************************
*
* NAME
*     IMECandidate::IMECandidate
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

IMECandidate::IMECandidate() :
		mCandidateSize(0),
		mCandidates(NULL)
	{
	Close();
	}


/******************************************************************************
*
* NAME
*     IMECandidate::~IMECandidate
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

IMECandidate::~IMECandidate()
	{
	if (mCandidates)
		{
		delete[] mCandidates;
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidate::Open
*
* DESCRIPTION
*
* INPUTS
*     Index      - Candidate Index ID
*     HWND       - Window IME is associated with
*     CodePage   - Code page for current input mode.
*     Unicode    - If true, process IME as unicode; if false, process as MBCS
*     StartFrom1 - If true, candidates should be displayed starting from 1
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidate::Open(int id, HWND hwnd, UINT codepage, bool unicode, bool startFrom1)
	{
	mIndex = id;
	mHWND = hwnd;
	mCodePage = codepage;
	mUseUnicode = unicode;
	mStartFrom1 = startFrom1;
	}


/******************************************************************************
*
* NAME
*     IMECandidate::Read
*
* DESCRIPTION
*     Read the candidate list from IMM
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidate::Read(void)
	{
	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		DWORD size = 0;

		// Get the size of the candidate list
		if (mUseUnicode)
			{
			size = ImmGetCandidateListW(imc, mIndex, NULL, 0);
			}
		else
			{
			size = ImmGetCandidateList(imc, mIndex, NULL, 0);
			}

		// Allocate space to hold candidates
		if ((mCandidates == NULL) || (size > mCandidateSize))
			{
			// Free the existing candidate buffer
			if (mCandidates != NULL)
				{
				delete[] mCandidates;
				}

			// Allocate a new buffer to hold the candidates
			mCandidates = (CANDIDATELIST*)(new unsigned char[size]);
			mCandidateSize = size;
			}

		if (mCandidates != NULL)
			{
			if (mUseUnicode)
				{
				ImmGetCandidateListW(imc, mIndex, mCandidates, mCandidateSize);
				}
			else
				{
				ImmGetCandidateList(imc, mIndex, mCandidates, mCandidateSize);
				}

			WWDEBUG_SAY(("IMECandidate: Index %d, Style %08lX, Selection %d, PageStart %d, PageSize %d, Count %d\n",
				mIndex, GetStyle(), GetSelection(), GetPageStart(), GetPageSize(), GetCount()));
			}

		ImmReleaseContext(mHWND, imc);
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidate::Close
*
* DESCRIPTION
*     Close the candidate.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidate::Close(void)
	{
	mIndex = -1;
	mHWND = NULL;
	mCodePage = CP_ACP;
	mUseUnicode = true;
	mStartFrom1 = true;

	if (mCandidates)
		{
		memset(mCandidates, 0, sizeof(CANDIDATELIST));
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidate::IsValid
*
* DESCRIPTION
*     Check if the candidate data is valid.
*
* INPUTS
*     NONE
*
* RESULT
*     True if valid
*
******************************************************************************/

bool IMECandidate::IsValid(void) const
	{
	return ((-1 != mIndex) && (mCandidates != NULL));
	}


/******************************************************************************
*
* NAME
*     IMECandidate::GetIndex
*
* DESCRIPTION
*     Get the candidate index ID
*
* INPUTS
*     NONE
*
* RESULT
*     Candidate index ID
*
******************************************************************************/

int IMECandidate::GetIndex(void) const
	{
	return mIndex;
	}


/******************************************************************************
*
* NAME
*     IMECandidate::GetStyle
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

unsigned int IMECandidate::GetStyle(void) const
	{
	WWASSERT(mCandidates != NULL);
	return mCandidates->dwStyle;
	}


/******************************************************************************
*
* NAME
*     IMECandidate::GetPageStart
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

unsigned int IMECandidate::GetPageStart(void) const
	{
	WWASSERT(mCandidates != NULL);
	return mCandidates->dwPageStart;
	}


/******************************************************************************
*
* NAME
*     IMECandidate::SetPageStart
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidate::SetPageStart(unsigned int start)
	{
	WWASSERT((start >=0) && (start < GetCount()));

	if ((start >=0) && (start < GetCount()))
		{
		HIMC imc = ImmGetContext(mHWND);

		if (imc)
			{
			// The IMM documentation says that the candidate list index must
			// be in the range of 0 - 3 for NI_SETCANDIDATE_PAGESTART
			WWASSERT((mIndex >= 0 && mIndex <= 3) && "IMM parameter error");

			ImmNotifyIME(imc, NI_SETCANDIDATE_PAGESTART, mIndex, start);
			ImmReleaseContext(mHWND, imc);
			}
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidate::GetPageSize
*
* DESCRIPTION
*     Get the number of candidates per page.
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

unsigned int IMECandidate::GetPageSize(void) const
	{
	WWASSERT(mCandidates != NULL);
	return mCandidates->dwPageSize;
	}


/******************************************************************************
*
* NAME
*     IMECandidate::GetCount
*
* DESCRIPTION
*     Get the total number of candidates.
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

// Get the number of candidates in the list.
unsigned int IMECandidate::GetCount(void) const
	{
	if (mCandidates)
		{
		return mCandidates->dwCount;
		}

	return 0;
	}


/******************************************************************************
*
* NAME
*     IMECandidate::GetSelection
*
* DESCRIPTION
*     Get the index of the currently selected candidate.
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

unsigned int IMECandidate::GetSelection(void) const
	{
	WWASSERT(mCandidates != NULL);
	return mCandidates->dwSelection;
	}


/******************************************************************************
*
* NAME
*     IMECandidate::IsStartFrom1
*
* DESCRIPTION
*     Check if the candidates should be listed as starting from 1 or 0
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

bool IMECandidate::IsStartFrom1(void) const
	{
	return mStartFrom1;
	}


/******************************************************************************
*
* NAME
*     IMECandidate::GetCandidate
*
* DESCRIPTION
*     Get a candidate string.
*
* INPUTS
*     Index of the candidate string to get.
*
* RESULT
*     String
*
******************************************************************************/

const unichar_t* IMECandidate::GetCandidate(unsigned int index)
	{
	if (index < GetCount())
		{
		// For the IME_CAND_CODE style, the candidate list has a special structure
		// depending on the value of the dwCount member. If dwCount is 1, the dwOffset
		// member contains a single DBCS character rather than an offset, and no
		// candidate string is provided. If the dwCount member is greater than 1,
		// the dwOffset member contains valid offsets, and the candidate strings are
		// text representations of individual DBCS character values in hexadecimal notation.
		if ((IME_CAND_CODE == GetStyle()) && (1 == GetCount()))
			{
			unsigned int dbcs = mCandidates->dwOffset[0];

			// If this char has a lead byte then it is double byte. Swap the bytes
			// for generate string order
			if (dbcs & 0xFF00)
				{
				dbcs = (((dbcs & 0xFF) << 8) | (dbcs >> 8));
				}

			// Convert char to unicode
			MultiByteToWideChar(mCodePage, 0, (const char*)&dbcs, -1, mTempString, 1);
			mTempString[1] = 0;

			return reinterpret_cast<unichar_t *>(mTempString);
			}

		DWORD offset = mCandidates->dwOffset[index];
		const char* candString = ((const char*)mCandidates + offset);

		if (mUseUnicode)
			{
			return ((const unichar_t*)candString);
			}

		MultiByteToWideChar(mCodePage, 0, candString, -1, mTempString, (sizeof(mTempString) / sizeof(unichar_t)));
		mTempString[(sizeof(mTempString) / sizeof(unichar_t)) - 1] = 0;
		return reinterpret_cast<unichar_t *>(mTempString);;
		}
	
	return NULL;
	}


/******************************************************************************
*
* NAME
*     IMECandidate::SelectCandidate
*
* DESCRIPTION
*     Select a string in the candidate list.
*
* INPUTS
*     Index of candidate string to select.
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidate::SelectCandidate(unsigned int selection)
	{
	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		ImmNotifyIME(imc, NI_SELECTCANDIDATESTR, mIndex, selection);
		ImmNotifyIME(imc, NI_CLOSECANDIDATE, mIndex, 0);
		ImmReleaseContext(mHWND, imc);
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidate::SetView
*
* DESCRIPTION
*
* INPUTS
*     Top - Index of top candidate.
*     Bottom - Index of bottom candidate.
*
* RESULT
*
******************************************************************************/

void IMECandidate::SetView(unsigned int topIndex, unsigned int bottomIndex)
	{
	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		ImmNotifyIME(imc, NI_SETCANDIDATE_PAGESIZE, mIndex, (bottomIndex - topIndex) + 1);
		ImmNotifyIME(imc, NI_SETCANDIDATE_PAGESTART, mIndex, topIndex);
		ImmReleaseContext(mHWND, imc);
		}
	}
	
} // namespace IME
