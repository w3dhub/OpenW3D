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
 *                 Project Name : wwtranslatedb																  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwtranslatedb/stringtwiddler.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/01/01 3:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __STRING_TWIDDLER_H
#define __STRING_TWIDDLER_H

#include "translateobj.h"
#include "translatedb.h"
#include "vector.h"


//////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//	StringTwiddlerClass
//
//////////////////////////////////////////////////////////////////////////
class StringTwiddlerClass : public TDBObjClass
{
public:
	
	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	StringTwiddlerClass (void);
	StringTwiddlerClass (const StringTwiddlerClass &src);
	virtual ~StringTwiddlerClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const StringTwiddlerClass &	operator= (const StringTwiddlerClass &src);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	//
	//	RTTI
	//
	StringTwiddlerClass *		As_StringTwiddlerClass (void) override		{ return this; }
	
	//
	// From PersistClass
	//
	const PersistFactoryClass &Get_Factory (void) const override;
	bool								Save (ChunkSaveClass &csave) override;
	bool								Load (ChunkLoadClass &cload) override;

	//
	// Copy methods
	//
	TDBObjClass *					Clone (void) const override					{ return new StringTwiddlerClass (*this); }

	//
	// Inherited
	//
	const WideStringClass &		Get_String (uint32 lang_id) override;
	const StringClass &			Get_English_String (void) override		{ Randomize (); return TDBObjClass::Get_English_String (); }
	const StringClass &			Get_ID_Desc (void) override				{ Randomize (); return TDBObjClass::Get_ID_Desc (); }
	uint32							Get_Sound_ID (void) override				{ Randomize (); return TDBObjClass::Get_Sound_ID (); }
	const StringClass &			Get_Animation_Name (void) override		{ Randomize (); return TDBObjClass::Get_Animation_Name (); }
	uint32							Get_Category_ID (void) override			{ Randomize (); return TDBObjClass::Get_Category_ID (); }

	//
	//	String list access
	//	
	void								Add_String (int string_id)			{ StringList.Add (string_id); }
	void								Reset_String_List (void)			{ StringList.Delete_All (); }
	int								Get_String_Count (void) const		{ return StringList.Count (); }
	int								Get_String_Index (int index) const		{ return StringList[index]; }
	TDBObjClass *					Lookup_String (int index);
	
protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	void								Save_Variables (ChunkSaveClass &csave);
	void								Load_Variables (ChunkLoadClass &cload);

	void								Randomize (int lang_id = TranslateDBClass::LANGID_ENGLISH);

private:

	//////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////
	DynamicVectorClass<int>		StringList;
};


#endif //__STRING_TWIDDLER_H

