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
 *                     $Archive:: /Commando/Code/wwui/imagectrl.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/30/01 9:51a                                                $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __IMAGECTRL_H
#define __IMAGECTRL_H


#include "render2d.h"
#include "dialogcontrol.h"


//////////////////////////////////////////////////////////////////////
//
//	ImageCtrlClass
//
//////////////////////////////////////////////////////////////////////
class ImageCtrlClass : public DialogControlClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	ImageCtrlClass  (void);
	virtual ~ImageCtrlClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	void				Render (void) override;

	//
	//	Configuration
	//
	void				Set_Texture (const char *texture_name);
	
protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	void				Create_Control_Renderer (void);
	void				Create_Texture_Renderer (void);
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	Render2DClass			ControlRenderer;
	Render2DClass			TextureRenderer;
};


#endif //__IMAGECTRL_H
