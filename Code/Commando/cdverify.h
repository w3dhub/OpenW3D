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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/cdverify.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/19/01 9:22a                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __CDVERIFY_H
#define __CDVERIFY_H

#include "Notify.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class StringClass;
class CDVerifyClass;


//////////////////////////////////////////////////////////////////////
//
//	CDVerifyEvent
//
//////////////////////////////////////////////////////////////////////
class CDVerifyEvent : public TypedEventPtr<CDVerifyEvent, CDVerifyClass>
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constants
	///////////////////////////////////////////////////////////////////
	typedef enum
	{
		NOT_VERIFIED = 0,
		VERIFIED,
	} EventID;

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	CDVerifyEvent (EventID event, CDVerifyClass *object)	:
		TypedEventPtr<CDVerifyEvent, CDVerifyClass>(object),
		MyEventID (event)  {}

	~CDVerifyEvent (void) {}

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	inline EventID		Event (void) const	{ return MyEventID; }
	
private:
	
	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	EventID MyEventID;
};


//////////////////////////////////////////////////////////////////////
//
//	CDVerifyClass
//
//////////////////////////////////////////////////////////////////////
class CDVerifyClass : public Notifier<CDVerifyEvent>
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	CDVerifyClass  (void) {}
	~CDVerifyClass (void) {}

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	bool		Get_CD_Path (StringClass &drive_path);
	void		Display_UI (Observer<CDVerifyEvent> *observer);
	
private:
	
	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
};


#endif //__CDVERIFY_H
