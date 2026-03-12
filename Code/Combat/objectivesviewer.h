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
 *                     $Archive:: /Commando/Code/Combat/objectivesviewer.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/05/00 6:36p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __OBJECTIVES_VIEWER_H
#define __OBJECTIVES_VIEWER_H

#include "wwstring.h"
#include "vector.h"
#include "vector3.h"
#include "textwindow.h"

////////////////////////////////////////////////////////////////
//
//	ObjectivesViewerClass
//
////////////////////////////////////////////////////////////////
class ObjectivesViewerClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ObjectivesViewerClass (void);
	~ObjectivesViewerClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	void				Initialize (void);
	void				Shutdown (void);

	//
	//	Visibility methods
	//
	bool				Is_Displayed (void) const	{ return IsDisplayed; }
	void				Display (bool onoff);
	void				Render (void);

	//
	//	Display control
	//
	void				Page_Down (void);
	void				Update (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	static int __cdecl	fnCompareObjectivesCallback (const void *elem1, const void *elem2);

	////////////////////////////////////////////////////////////////
	//	Private data types
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		COL_DESC		= 0,
		COL_STATUS,
		COL_MAX
	} COLUMNS;

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	bool					IsDisplayed;
	TextWindowClass *	TextWindow;
};


#endif //__OBJECTIVES_VIEWER_H
