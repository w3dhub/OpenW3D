/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D Developers
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
* DESCRIPTION
*     IME candidate list provided by SDL3 (SDL_EVENT_TEXT_EDITING_CANDIDATES).
*
*     SDL only provides the candidates the IME is currently showing, so the
*     whole list is treated as a single page. SDL has no way for the
*     application to change the page or select a candidate.
*
******************************************************************************/

#include "IMECandidate.h"
#include "wwuiplatform.h"
#include "wwdebug.h"

namespace IME {

IMECandidate::IMECandidate() :
		mIndex(-1),
		mStartFrom1(true),
		mSelection(0),
		mPageStart(0)
{
}


IMECandidate::~IMECandidate()
{
}


void IMECandidate::Open(int index, bool startFrom1)
{
	mIndex = index;
	mStartFrom1 = startFrom1;
}


void IMECandidate::Set(const char* const* candidates, int count, int selection)
{
	mCandidates.clear();

	for (int index = 0; index < count; ++index)
	{
		mCandidates.push_back(WWUIPlatform::UTF8_To_Unichar(candidates[index]));
	}

	// No selection is reported to the candidate control as -1
	mSelection = (selection >= 0 && selection < count) ? (unsigned int)selection : (unsigned int)-1;
	mPageStart = 0;

	WWDEBUG_SAY(("IMECandidate: Index %d, Selection %d, Count %d\n", mIndex, selection, count));
}


void IMECandidate::Close(void)
{
	mIndex = -1;
	mStartFrom1 = true;
	mCandidates.clear();
	mSelection = 0;
	mPageStart = 0;
}


bool IMECandidate::IsValid(void) const
{
	return (-1 != mIndex);
}


int IMECandidate::GetIndex(void) const
{
	return mIndex;
}


unsigned int IMECandidate::GetStyle(void) const
{
	return 0;
}


unsigned int IMECandidate::GetPageStart(void) const
{
	return mPageStart;
}


void IMECandidate::SetPageStart(unsigned int start)
{
	// SDL does not let the application page through the IME candidates.
	WWDEBUG_SAY(("IMECandidate: SetPageStart(%u) not supported by SDL\n", start));
}


unsigned int IMECandidate::GetPageSize(void) const
{
	return (unsigned int)mCandidates.size();
}


unsigned int IMECandidate::GetCount(void) const
{
	return (unsigned int)mCandidates.size();
}


unsigned int IMECandidate::GetSelection(void) const
{
	return mSelection;
}


bool IMECandidate::IsStartFrom1(void) const
{
	return mStartFrom1;
}


const unichar_t* IMECandidate::GetCandidate(unsigned int index)
{
	if (index < GetCount())
	{
		return mCandidates[index].c_str();
	}

	return nullptr;
}


void IMECandidate::SelectCandidate(unsigned int selection)
{
	// SDL does not let the application select an IME candidate.
	WWDEBUG_SAY(("IMECandidate: SelectCandidate(%u) not supported by SDL\n", selection));
}


void IMECandidate::SetView(unsigned int /* topIndex */, unsigned int /* bottomIndex */)
{
}

} // namespace IME
