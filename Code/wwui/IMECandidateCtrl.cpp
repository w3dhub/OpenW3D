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

#include "IMECandidateCtrl.h"
#include "IMECandidate.h"
#include "dialogbase.h"
#include "stylemgr.h"
#include "mousemgr.h"
#include <algorithm>

#define BORDER_WIDTH 2
#define BORDER_HEIGHT 2

/******************************************************************************
*
* NAME
*     IMECandidateCtrl::IMECandidateCtrl
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

IMECandidateCtrl::IMECandidateCtrl() :
		mFullRect(0,0,0,0),
		mCellSize(0,0),
		mCurrSel(-1),
		mScrollPos(0),
		mCellsPerPage(0),
		mCandidate(NULL)
	{
	StyleMgrClass::Assign_Font(&mTextRenderer, StyleMgrClass::FONT_LISTS);

	// Candidate control never wants to take focus
	Set_Wants_Focus(false);

	// Setup child scrollbar
	mScrollBarCtrl.Set_Wants_Focus(false);
	mScrollBarCtrl.Set_Small_BMP_Mode(true);
	mScrollBarCtrl.Set_Advise_Sink(this);
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::~IMECandidateCtrl
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

IMECandidateCtrl::~IMECandidateCtrl()
	{
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::Init
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::Init(IME::IMECandidate* candidate)
	{
	WWDEBUG_SAY(("IMECandidateCtrl: Init\n"));

	Reset();
	mCandidate = candidate;
	Changed(candidate);
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::Changed
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::Changed(IME::IMECandidate* candidate)
	{
	WWDEBUG_SAY(("IMECandidateCtrl: Changed\n"));
	WWASSERT(mCandidate == candidate);

	if (candidate)
		{
		mScrollPos = candidate->GetPageStart();

		unsigned int candSel = candidate->GetSelection();

		if (candSel != (unsigned int)mCurrSel)
			{
			mCurrSel = candSel;
			UpdateScrollPos();
			}
		}

	Set_Dirty();
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::Reset
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::Reset(void)
	{
	mCellSize.Set(0,0);
	mCurrSel = -1;
	mScrollPos = 0;
	mCellsPerPage = 0;
	mScrollBarCtrl.Show(false);
	mCandidate = NULL;
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::CreateControlRenderer
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::CreateControlRenderer(void)
	{
	Render2DClass& renderer = mControlRenderer;

	// Configure this renderer
	renderer.Reset();
	renderer.Enable_Texturing(false);
	renderer.Set_Coordinate_Range(Render2DClass::Get_Screen_Resolution());

	renderer.Add_Quad(mFullRect, RGBA_TO_INT32(0, 0, 0, 170));

	// Determine which color to draw the outline in
	int color = StyleMgrClass::Get_Line_Color();
	int bkColor = StyleMgrClass::Get_Bk_Color();

	if (IsEnabled == false)
		{
		color = StyleMgrClass::Get_Disabled_Line_Color();
		bkColor = StyleMgrClass::Get_Disabled_Bk_Color();
		}

	// Draw the control outline
	RectClass rect = Rect;
	renderer.Add_Outline(rect, 1.0F, color);

	// Now draw the background
	rect.Right -= 1;
	rect.Bottom -= 1;
	renderer.Add_Quad(rect, bkColor);
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::CreateTextRenderer
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::CreateTextRenderer(void)
	{
	mHilightRenderer.Reset();
	mHilightRenderer.Set_Coordinate_Range(Render2DClass::Get_Screen_Resolution());
	StyleMgrClass::Configure_Hilighter(&mHilightRenderer);

	mTextRenderer.Reset();

	if (mCandidate)
		{
		// Add each candidate to the list
		const unsigned int selIndexBias = (mCandidate->IsStartFrom1() ? 1 : 0);

		float currYPos = ClientRect.Top;
		const unsigned int candidateCount = mCandidate->GetCount();

		for (unsigned int index = mScrollPos; index < candidateCount; ++index)
			{
			// Build the rectangle we will draw the text into
			RectClass textRect;
			textRect.Left = ClientRect.Left;
			textRect.Top = currYPos;
			textRect.Right = (textRect.Left + mCellSize.X);
			textRect.Bottom = (textRect.Top + mCellSize.Y);

			// If this cell extends past the end of the control then stop
			if (textRect.Bottom > ClientRect.Bottom)
				{
				break;
				}

			// Draw the text
			const unichar_t* text = mCandidate->GetCandidate(index);

			WideStringClass entry(0, true);
			entry.Format(U_CHAR("%d. %s"), ((index - mScrollPos) + selIndexBias), text);
			
			StyleMgrClass::Render_Text(entry, &mTextRenderer, textRect, true, true);

			//	Hilight this entry (if its the currently selected one)
			if ((int)index == mCurrSel)
				{
				StyleMgrClass::Render_Hilight(&mHilightRenderer, textRect);
				}

			currYPos += mCellSize.Y;
			}
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::Render
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::Render(void)
	{
	// Recreate the renderers (if necessary)
	if (IsDirty)
		{
		CreateControlRenderer();
		CreateTextRenderer();
		}

	mControlRenderer.Render();
	mTextRenderer.Render();
	mHilightRenderer.Render();

	DialogControlClass::Render();
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::SetCurrSel
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::SetCurrSel(int index)
	{
	if (mCandidate)
		{
		if ((index != mCurrSel) && (index == -1) || ((index >= 0) && (unsigned int)index < mCandidate->GetCount()))
			{
			mCurrSel = index;
			Set_Dirty();
			}
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::EntryFromPos
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

int IMECandidateCtrl::EntryFromPos(const Vector2& mousePos)
	{
	if (mCandidate)
		{
		// Loop over all the entries in our current view
		float currYPos = ClientRect.Top;

		const unsigned int candidateCount = mCandidate->GetCount();

		for (unsigned int index = mScrollPos; index < candidateCount; ++index)
			{
			// Is ths mouse over this entry?
			if ((mousePos.Y >= currYPos && mousePos.Y <= (currYPos + mCellSize.Y))
					|| mousePos.Y > ClientRect.Bottom)
				{
				return index;
				}

			currYPos += mCellSize.Y;
			}
		}
	
	return -1;
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::UpdateScrollPos
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::UpdateScrollPos(void)
	{
	if (mCurrSel != -1)
		{
		unsigned int scrollPos = mScrollPos;

		if (mCurrSel < (int)scrollPos)
			{
			// Scroll up so the current selection is in view
			scrollPos = mCurrSel;
			Set_Dirty();
			}
		else if (mCurrSel >= (int)(scrollPos + mCellsPerPage))
			{
			//	Scroll down so the current selection is in view
			scrollPos = std::max<unsigned int>((unsigned int)mCurrSel - (mCellsPerPage - 1), 0);
			Set_Dirty();
			}

		if (scrollPos != mScrollPos)
			{
			// Update the scrollbar
			mScrollBarCtrl.Set_Pos(mScrollPos, false);

			if (mCandidate)
				{
				mCandidate->SetPageStart(mScrollPos);
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::Update_Client_Rect
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::Update_Client_Rect(void)
	{
	WWDEBUG_SAY(("IMECandidateCtrl: Update_Client_Rect()\n"));

	Vector2 pageSize;
	CalculateCandidatePageExtent(pageSize, mCellSize);

	mCellsPerPage = (unsigned int)(pageSize.Y / mCellSize.Y);

	Rect.Right = (Rect.Left + (pageSize.X + (BORDER_WIDTH * 2.0f)));
	Rect.Bottom = (Rect.Top + (pageSize.Y + (BORDER_HEIGHT * 2.0f)));
	
	mFullRect = Rect;

	ClientRect = Rect;
	ClientRect.Inflate(Vector2(-BORDER_WIDTH, -BORDER_HEIGHT));

	if (mCandidate)
		{
		WWASSERT(mCandidate->GetPageSize() <= mCellsPerPage);

		//	Do we need to show a scroll bar?
		const unsigned int candidateCount = mCandidate->GetCount();

		if ((unsigned int)mCellsPerPage < candidateCount)
			{
			// Position the scrollbar to the right of the list
			RectClass scrollRect;
			scrollRect.Left = mFullRect.Right;
			scrollRect.Top = mFullRect.Top;
			scrollRect.Right = -1;
			scrollRect.Bottom = mFullRect.Bottom;

			//	Size the scroll bar
			mScrollBarCtrl.Set_Window_Rect(scrollRect);
			mScrollBarCtrl.Set_Page_Size(mCellsPerPage - 1);
			mScrollBarCtrl.Set_Range(0, (candidateCount - mCellsPerPage));
			mScrollBarCtrl.Show(true);

			const RectClass& rect = mScrollBarCtrl.Get_Window_Rect();
			mFullRect.Right = rect.Right;
			}
		}

	Set_Dirty();
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::CalculateCandidatePageExtent(Vector2& outExtent, Vector2& outCellSize)
	{
	if (mCandidate)
		{
		// Get the size of the widest candidate string.
		float maxCandWidth = 0.0f;
		const unsigned int candidateCount = mCandidate->GetCount();

		for (unsigned int index = 0; index < candidateCount; ++index)
			{
			// Get the extent of the current entry
			const unichar_t* text = mCandidate->GetCandidate(index);
			Vector2 textExtent = mTextRenderer.Get_Text_Extents(text);

			if (textExtent.X > maxCandWidth)
				{
				maxCandWidth = textExtent.X;
				}
			}

		Vector2 charSize = mTextRenderer.Get_Text_Extents(U_CHAR("W"));

		// The cell size is the maximum candidate size plus some spacing.
		outCellSize.X = (maxCandWidth + (charSize.X * 3.0f));
		outCellSize.Y = (charSize.Y * 1.25f);

		outExtent.X = outCellSize.X;
		outExtent.Y = (mCellSize.Y * (float)mCandidate->GetPageSize());
		}
	else
		{
		outExtent.X = 0.0f;
		outExtent.Y = 0.0f;

		outCellSize.X = 0.0f;
		outCellSize.Y = 0.0f;
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::On_Set_Cursor
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::On_Set_Cursor(const Vector2& mousePos)
	{
	//	Change the mouse cursor if necessary
	if (ClientRect.Contains(mousePos))
		{
		MouseMgrClass::Set_Cursor(MouseMgrClass::CURSOR_ACTION);
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::On_LButton_Down
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::On_LButton_Down(const Vector2& mousePos)
	{
	SetCurrSel(EntryFromPos(mousePos));
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::On_LButton_Up
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::On_LButton_Up(const Vector2& mousePos)
	{
	int sel = EntryFromPos(mousePos);
	SetCurrSel(sel);

	// Process candidate selection here
	if (mCandidate && (sel >= 0))
		{
		[[maybe_unused]] const unichar_t* string = mCandidate->GetCandidate(sel);
		WWDEBUG_SAY(("*** Selected Candidate: %d %04x\n", sel, *string));
		mCandidate->SelectCandidate((unsigned int)sel);
		}
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::On_Add_To_Dialog
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::On_Add_To_Dialog(void)
	{
	Parent->Add_Control(&mScrollBarCtrl);
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::On_Remove_From_Dialog
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::On_Remove_From_Dialog(void)
	{
	Parent->Remove_Control(&mScrollBarCtrl);
	}


/******************************************************************************
*
* NAME
*     IMECandidateCtrl::On_VScroll
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void IMECandidateCtrl::On_VScroll(ScrollBarCtrlClass*, int , int newPos)
	{
	mScrollPos = newPos;

	if (mCandidate)
		{
		mCandidate->SetPageStart(mScrollPos);
		}

	Set_Dirty();
	}
