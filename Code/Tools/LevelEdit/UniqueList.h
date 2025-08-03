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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/UniqueList.h                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/14/98 11:09a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __UNIQUELIST_H
#define __UNIQUELIST_H

#include "vector.h"

/////////////////////////////////////////////////////////////////////
//
//	UniqueListClass
//
template<class T>
class UniqueListClass : public DynamicVectorClass<T>
{
	public:
		UniqueListClass (void)
			: DynamicVectorClass<T> () {}
		virtual ~UniqueListClass (void) {}

		UniqueListClass<T> &			operator += (const UniqueListClass<T> &reference);
		bool								Add_Unique (T const & object);
		void								Remove (T const & object);
		bool								Is_Item_In_List (T const & object);
};


/////////////////////////////////////////////////////////////////////
//
//	Is_Item_In_List
//
template<class T>
bool
UniqueListClass<T>::Is_Item_In_List (T const & object)
{
	// Assume failure
	bool bfound = false;

	// Loop through all the objects in this list and see
	// if any of them match the provided object
	for (int index = 0; (index < Count ()) && !bfound; index ++) {
		if (object == Vector[index]) {
			bfound = true;
		}
	}

	// Return the true/false result code
	return bfound;
}


/////////////////////////////////////////////////////////////////////
//
//	Add_Unique
//
template<class T>
UniqueListClass<T> &
UniqueListClass<T>::operator += (const UniqueListClass<T> &reference)
{
	for (int index = 0; index < reference.Count (); index ++) {
		Add_Unique (reference[index]);
	}

	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	Add_Unique
//
template<class T>
bool
UniqueListClass<T>::Add_Unique (T const & object)
{
	// Assume sucess
	bool retval = true;

	// If the object isn't already in the list, then add it
	if (!Is_Item_In_List (object)) {
		retval = Add (object);
	}

	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Remove
//
template<class T>
void
UniqueListClass<T>::Remove (T const & object)
{
	// Loop through all the objects in this list and see
	// if any of them match the provided object
	bool bfound = false;
	for (int index = 0; (index < Count ()) && !bfound; index ++) {
		if (object == Vector[index]) {
			Delete (index);
			bfound = true;
		}
	}

	return ;
}



#endif //__UNIQUELIST_H
