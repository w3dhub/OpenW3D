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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : wwui                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/textmarqueectrl.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/18/01 3:56p                                                 $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TEXTMARQUEECTRL_H
#define __TEXTMARQUEECTRL_H

#include "dialogcontrol.h"
#include "render2dsentence.h"
#include "vector.h"


//////////////////////////////////////////////////////////////////////
//
//	TextMarqueeCtrlClass
//
//////////////////////////////////////////////////////////////////////
class TextMarqueeCtrlClass : public DialogControlClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	TextMarqueeCtrlClass  (void);
	~TextMarqueeCtrlClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	void				Render (void) override;
	void				Set_Text (const unichar_t *title) override;

	//
	//	Configuration
	//
	void				Set_Scroll_Rate (float rows_per_sec)		{ ScrollRate = rows_per_sec; }
	float				Get_Scroll_Rate (void)							{ return ScrollRate; }

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected classes
	///////////////////////////////////////////////////////////////////
	typedef struct _CREDIT_LINE
	{
		_CREDIT_LINE (void)	:
			FontIndex (0), Color (0), Height (0)	{}

		_CREDIT_LINE (const unichar_t *text, int font_index, int color)	:
			Text (text), FontIndex (font_index), Color (color)	{}

		bool operator== (const _CREDIT_LINE &/* src*/)	{ return false; }
		bool operator!= (const _CREDIT_LINE &/* src*/)	{ return true; }
			
		WideStringClass	Text;
		int					FontIndex;
		int					Color;
		float					Height;

	} CREDIT_LINE;
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	void				Update_Client_Rect (void) override;
	void				On_Frame_Update (void) override;

	void				Create_Control_Renderer (void);
	void				Create_Text_Renderer (void);
	void				Calculate_Row_Count (void);

	int				Read_Tag (const unichar_t *text, CREDIT_LINE &line);
	const unichar_t *	Read_Line (const unichar_t *text, CREDIT_LINE &line);
	void				Build_Credit_Lines (void);

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	Render2DClass				ControlRenderer;
	Render2DSentenceClass	TextRenderers[2];
	float							ScrollPos;
	int							PixelHeight;
	float							ScrollRate;
	
	DynamicVectorClass<CREDIT_LINE>	CreditLines;
};


#endif //__TEXTMARQUEECTRL_H
