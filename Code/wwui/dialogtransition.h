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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/dialogtransition.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/27/01 9:56a                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DIALOG_TRANSITION_H
#define __DIALOG_TRANSITION_H


#include "refcount.h"


///////////////////////////////////////////////////////////////////////////////
//
//	DialogTransitionClass
//
///////////////////////////////////////////////////////////////////////////////
class DialogTransitionClass : public RefCountClass
{
public:

	////////////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////////////
	enum TYPE
	{
		SCREEN_OUT	= 0,
		SCREEN_IN
	};

	////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////////
	DialogTransitionClass (TYPE type = SCREEN_OUT);
	virtual ~DialogTransitionClass (void);

	////////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////////

	//
	//	Configuration
	//
	virtual void	Set_Type (TYPE type)		{ Type = type; }
	TYPE				Get_Type (void) const	{ return Type; }

	//
	//	Frame updates
	//
	virtual void	Render (void)				{}
	virtual void	On_Frame_Update (void)	{}

	//
	//	Flow control
	//
	virtual bool	Is_Complete (void)		{ return true; }

protected:

	////////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////////
	//virtual void	Update_Renderer (void);

	////////////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////////////
	//Render2DClass	Renderer;
	//Render2DClass	SwipeRenderer;
	TYPE				Type;
	//int				MaxTime;
	//int				Timer;
};


#endif //__DIALOG_TRANSITION_H

