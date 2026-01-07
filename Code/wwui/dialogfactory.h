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
 *                     $Archive:: /Commando/Code/wwui/dialogfactory.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/26/01 10:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DIALOG_FACTORY_H
#define __DIALOG_FACTORY_H

#include "dialogbase.h"


////////////////////////////////////////////////////////////////
//
//	DialogFactoryBaseClass
//
////////////////////////////////////////////////////////////////
class DialogFactoryBaseClass
{
public:
	virtual ~DialogFactoryBaseClass() {}
	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	virtual void	Do_Dialog (void)	= 0;
};


////////////////////////////////////////////////////////////////
//
//	DialogFactoryClass
//
////////////////////////////////////////////////////////////////
template<class  T>
class DialogFactoryClass : public DialogFactoryBaseClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void	Do_Dialog (void) override	{ T *dialog = new T; dialog->Start_Dialog (); REF_PTR_RELEASE (dialog); }
};


#endif //__DIALOG_FACTORY_H

