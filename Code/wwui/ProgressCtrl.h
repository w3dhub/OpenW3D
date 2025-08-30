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
*     $Archive: /Commando/Code/wwui/ProgressCtrl.h $
*
* DESCRIPTION
*     Progress bar control
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 2 $
*     $Modtime: 10/18/01 9:22a $
*
******************************************************************************/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PROGRESS_CTRL_H
#define __PROGRESS_CTRL_H

#include "dialogcontrol.h"
#include "render2d.h"

//#include "vector3.h"
//#include "bittype.h"

class ProgressCtrlClass :
		public DialogControlClass
	{
	public:
		ProgressCtrlClass(void);
		virtual ~ProgressCtrlClass();

		// RTTI
		ProgressCtrlClass* As_ProgressCtrlClass(void)
			{return this;}

		void Render(void) override;

		// Sets the minimum and maximum values for the progress bar and redraws
		// the bar to reflect the new range. 
		void Set_Range(unsigned int min, unsigned int max);

		// Get the minimun and maximum values for the progress bar.
		void Get_Range(unsigned int& min, unsigned int& max);

		// Set the current position for the progress bar.
		void Set_Position(unsigned int position);

		// Advance the position of the progress bar by a specified increment.
		void Delta_Position(int delta);

		// Get the current position of the progress bar.
		unsigned int Get_Position(void) const;

		// Specify the step increment for the progress bar. This is the amount
		// the progress bar increases its position whenever Step_Position()
		// is called.
		void Set_Step(unsigned int step);

		// Advance the position for the progress bar by the step increment.
		void Step_Position(void);

	protected:
		void Create_Control_Renderers(void);
		float Calculate_Bar_Width(unsigned int position);
		void Update_Client_Rect(void) override;

	protected:
		Render2DClass mControlRenderer;

		RectClass mBarRect;
		
		unsigned int mMinLimit;
		unsigned int mMaxLimit;
		unsigned int mPosition;
		unsigned int mStep;
	};


#endif //__PROGRESS_CTRL_H
