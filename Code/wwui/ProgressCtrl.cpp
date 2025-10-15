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
*     $Archive: /Commando/Code/wwui/ProgressCtrl.cpp $
*
* DESCRIPTION
*     Progress bar control
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 1/12/02 9:44p $
*
******************************************************************************/

#include "ProgressCtrl.h"
#include "stylemgr.h"
#include <algorithm>

#define BAR_INSET 3

/******************************************************************************
*
* NAME
*     ProgressCtrlClass::ProgressCtrlClass
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

ProgressCtrlClass::ProgressCtrlClass(void) :
		mBarRect(0, 0, 0, 0),
		mMinLimit(0),
		mMaxLimit(100),
		mPosition(0),
		mStep(10)
	{
	StyleMgrClass::Configure_Renderer(&mControlRenderer);
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::~ProgressCtrlClass
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

ProgressCtrlClass::~ProgressCtrlClass()
	{
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Create_Control_Renderers
*
* DESCRIPTION
*     Create the renderers for the control
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void ProgressCtrlClass::Create_Control_Renderers(void)
	{
	Render2DClass& renderer = mControlRenderer;

	// Configure this renderer
	renderer.Reset();
	renderer.Enable_Texturing(false);

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

	// Draw the bar
	if (mPosition > mMinLimit)
		{
		int barColor = StyleMgrClass::Get_Text_Color();

		if (IsEnabled == false)
			{
			barColor = StyleMgrClass::Get_Disabled_Text_Color();
			}

		renderer.Add_Quad(mBarRect, barColor);
		}
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Calculate_Bar_Width
*
* DESCRIPTION
*     Calculate the new bar width.
*
* INPUTS
*     Position - Position to calculate bar width for.
*
* RESULT
*     NONE
*
******************************************************************************/

float ProgressCtrlClass::Calculate_Bar_Width(unsigned int position)
	{
	// Adjust the bar to reflect the new position
	float maxBarWidth = (Rect.Width() - (float)(2 * BAR_INSET));
	float scaler = (maxBarWidth / (float)(mMaxLimit - mMinLimit));
	float barWidth = (scaler * (float)(position - mMinLimit));

	return barWidth;
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Update_Client_Rect
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

void ProgressCtrlClass::Update_Client_Rect(void)
	{
	// Calculate the new bar rectangle
	mBarRect.Left	= Rect.Left + BAR_INSET;
	mBarRect.Top = Rect.Top + BAR_INSET;
	mBarRect.Bottom	= Rect.Bottom - BAR_INSET;
	mBarRect.Right = (mBarRect.Left + Calculate_Bar_Width(mPosition));
	
	Set_Dirty();
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Render
*
* DESCRIPTION
*     Render the control
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void ProgressCtrlClass::Render(void)
	{
	// Recreate the renderers (if necessary)
	if (IsDirty)
		{
		Create_Control_Renderers();
		}

	// Render the progress bar's current state
	mControlRenderer.Render();
	DialogControlClass::Render();
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Set_Range
*
* DESCRIPTION
*     Sets the minimum and maximum values for the progress bar and redraws the
*     bar to reflect the new range. 
*
* INPUTS
*     Min - Minimum range value
*     Max - Maximum range value
*
* RESULT
*     NONE
*
******************************************************************************/

void ProgressCtrlClass::Set_Range(unsigned int min, unsigned int max)
	{
	WWASSERT(min < max);

	// Scale current position to new range
	if (mPosition > mMinLimit)
		{
		float oldDelta = (mMaxLimit - mMinLimit);
		float newDelta = (max - min);
		float scaler = (oldDelta / newDelta);

		unsigned int position = (unsigned int)((float)mPosition * scaler);
		Set_Position(position);
		}

	mMinLimit = min;
	mMaxLimit = max;

	Set_Dirty();
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Get_Range
*
* DESCRIPTION
*     Get the minimun and maximum values for the progress bar.
*
* INPUTS
*     Min - On return; Minimum range value
*     Max - On return; Maximum range value
*
* RESULT
*     NONE
*
******************************************************************************/

void ProgressCtrlClass::Get_Range(unsigned int& min, unsigned int& max)
	{
	min = mMinLimit;
	max = mMaxLimit;
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Set_Position
*
* DESCRIPTION
*     Set the current position for the progress bar.
*
* INPUTS
*     Position - New position
*
* RESULT
*     NONE
*
******************************************************************************/

void ProgressCtrlClass::Set_Position(unsigned int position)
	{
	unsigned int oldPosition = mPosition;
	mPosition = std::min<unsigned int>(mMaxLimit, position);
	mPosition = std::max<unsigned int>(mMinLimit, mPosition);

	if (oldPosition != mPosition)
		{
		mBarRect.Right = (mBarRect.Left + Calculate_Bar_Width(mPosition));
		Set_Dirty();
		}
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Delta_Position
*
* DESCRIPTION
*     Advance the position of the progress bar by a specified increment.
*
* INPUTS
*     Delta - Amount to advance the position by.
*
* RESULT
*     NONE
*
******************************************************************************/

void ProgressCtrlClass::Delta_Position(int delta)
	{
	Set_Position(mPosition + delta);
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Get_Position
*
* DESCRIPTION
*     Get the current position of the progress bar.
*
* INPUTS
*     NONE
*
* RESULT
*     Position - Progress position
*
******************************************************************************/

unsigned int ProgressCtrlClass::Get_Position(void) const
	{
	return mPosition;
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Set_Step
*
* DESCRIPTION
*     Specify the step increment for the progress bar. This is the amount
*     the progress bar increases its current position whenever Step_Position()
*     is called.
*
* INPUTS
*     Step - New step increment.
*
* RESULT
*     NONE
*
******************************************************************************/

void ProgressCtrlClass::Set_Step(unsigned int step)
	{
	mStep = std::min<unsigned int>((mMaxLimit - mMinLimit), step);
	mStep = std::max<unsigned int>(1, mStep);
	}


/******************************************************************************
*
* NAME
*     ProgressCtrlClass::Step_Position
*
* DESCRIPTION
*     Advance the position for the progress bar by the step increment.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void ProgressCtrlClass::Step_Position(void)
	{
	float stepping = ((float)(mMaxLimit - mMinLimit) / (float)mStep);
	float stepPos = (floor((float)mPosition / stepping) * stepping);
	Set_Position(stepPos + mStep);
	}
