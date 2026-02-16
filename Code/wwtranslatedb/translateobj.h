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
 *                     $Archive:: /Commando/Code/wwtranslatedb/translateobj.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/10/01 8:43a                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TRANSLATE_OBJ_H
#define __TRANSLATE_OBJ_H

#include "persist.h"
#include "bittype.h"
#include "wwstring.h"
#include "widestring.h"
#include "vector.h"


//////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;
class StringTwiddlerClass;


//////////////////////////////////////////////////////////////////////////
//
//	TDBObjClass
//
//////////////////////////////////////////////////////////////////////////
class TDBObjClass : public PersistClass
{
public:
	
	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	TDBObjClass (void);
	TDBObjClass (const TDBObjClass &src);
	virtual ~TDBObjClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const TDBObjClass &	operator= (const TDBObjClass &src);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	//
	//	RTTI
	//
	virtual StringTwiddlerClass *	As_StringTwiddlerClass (void)		{ return NULL; }
	
	//
	// From PersistClass
	//
	const PersistFactoryClass &	Get_Factory (void) const override;
	bool									Save (ChunkSaveClass &csave) override;
	bool									Load (ChunkLoadClass &cload) override;

	//
	// Copy methods
	//
	virtual TDBObjClass *				Clone (void) const					{ return new TDBObjClass (*this); }

	//
	// TranslateObj specific
	//
	virtual const WideStringClass &	Get_String (uint32 lang_id);
	virtual const WideStringClass &	Get_String (void);
	virtual const StringClass &		Get_English_String (void)			{ return EnglishString; }
	virtual uint32							Get_ID (void)							{ return ID; }
	virtual const StringClass &		Get_ID_Desc (void)					{ return IDDesc; }
	virtual uint32							Get_Sound_ID (void)					{ return SoundID; }
	virtual const StringClass &		Get_Animation_Name (void)			{ return AnimationName; }
	virtual uint32							Get_Category_ID (void)				{ return CategoryID; }
	
	virtual void							Set_String (uint32 lang_id, const unichar_t *string);
	virtual void							Set_English_String (const char *string);
	virtual void							Set_ID (uint32 id);
	virtual void							Set_ID_Desc (const char *desc);
	virtual void							Set_Animation_Name (const char *name)	{ AnimationName = name; }
	virtual void							Set_Sound_ID (uint32 id)					{ SoundID = id; }
	virtual void							Set_Category_ID (uint32 id)				{ CategoryID = id; }

	//
	//	Informational
	//
	virtual bool							Contains_Translation (uint32 lang_id);
	
protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	void								Save_Variables (ChunkSaveClass &csave);
	void								Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////
	StringClass									EnglishString;
	StringClass									IDDesc;
	uint32										ID;
	uint32										SoundID;
	uint32										CategoryID;
	StringClass									AnimationName;
	DynamicVectorClass<WideStringClass>	TranslatedStrings;
};


#endif //__TRANSLATE_OBJ_H

