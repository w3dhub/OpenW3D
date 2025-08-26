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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditScript.h                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/02/00 3:57p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EDIT_SCRIPT_H
#define __EDIT_SCRIPT_H

#include "../../Scripts/scriptevents.h"
#include "vector.h"
#include "Utils.h"
#include "wwstring.h"


// Forward declarations
class ChunkSaveClass;
class ChunkLoadClass;


//////////////////////////////////////////////////////////////////////////
//
//	EditScriptClass
//
//////////////////////////////////////////////////////////////////////////
class EditScriptClass
{
public:
	
	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	EditScriptClass (void);
	EditScriptClass (const EditScriptClass &src);
	virtual ~EditScriptClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const EditScriptClass &	operator= (const EditScriptClass &src);
	bool							operator== (const EditScriptClass &src);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	EditScriptClass *	Clone (void)											{ return new EditScriptClass (*this); }
	ScriptClass *		Create_Script (void);
	
	//
	//	Name/string methods
	//
	LPCTSTR				Get_Name (void) const;
	void					Set_Name (LPCTSTR name);
	void					Set_Param_Desc (LPCTSTR param_desc);
	void					Lookup_Param_Description (void);
	CString				Get_Composite_String (void)						{ return Build_Composite_String (); }
	void					Set_Composite_Values (LPCTSTR values);

	//
	//	Param methods
	//
	int					Get_Param_Count (void);
	LPCTSTR				Get_Param_Value (int index);
	LPCTSTR				Get_Param_Name (int index);
	PARAM_TYPES			Get_Param_Type (int index);
	void					Set_Param_Value (int index, LPCTSTR value);

	//
	//	Persistance methods
	//
	bool					Save (ChunkSaveClass &csave);
	bool					Load (ChunkLoadClass &cload);

protected:

	//////////////////////////////////////////////////////////////
	//	Protected data types
	//////////////////////////////////////////////////////////////
	typedef struct _PARAM_VALUE
	{						
		CString			name;
		CString			value;
		PARAM_TYPES		type;

		bool operator== (const _PARAM_VALUE &src) { return (type == src.type) && (value == src.value) && (name == src.name); }
		bool operator!= (const _PARAM_VALUE &src) { return !(*this == src); }
	} PARAM_VALUE;


	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	bool						Load_Variables (ChunkLoadClass &cload);
	CString					Build_Composite_String (void) const;	
	void						Update_Data (void);
	PARAM_TYPES				String_To_Type (LPCTSTR type_name);
	bool						Valid_Index (int index) const { return (index >= 0) && (index < m_ParamValues.Count ()); }
	
private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	StringClass									m_Name;
	DynamicVectorClass<PARAM_VALUE>		m_ParamValues;
	StringClass									m_ParamDescription;
};


//////////////////////////////////////////////////////////////
//	Set_Param_Desc
//////////////////////////////////////////////////////////////
inline void
EditScriptClass::Set_Param_Desc (LPCTSTR param_desc)
{
	m_ParamDescription = param_desc;
	Update_Data ();
	return ;
}


//////////////////////////////////////////////////////////////
//	Get_Param_Count
//////////////////////////////////////////////////////////////
inline int
EditScriptClass::Get_Param_Count (void)
{	
	return m_ParamValues.Count ();;
}


//////////////////////////////////////////////////////////////
//	Get_Param_Value
//////////////////////////////////////////////////////////////
inline LPCTSTR
EditScriptClass::Get_Param_Value (int index)
{
	LPCTSTR value = NULL;

	if (Valid_Index (index)) {
		value = m_ParamValues[index].value;
	}

	return value;
}


//////////////////////////////////////////////////////////////
//	Get_Param_Name
//////////////////////////////////////////////////////////////
inline LPCTSTR
EditScriptClass::Get_Param_Name (int index)
{
	LPCTSTR name = NULL;

	if (Valid_Index (index)) {
		name = m_ParamValues[index].name;
	}

	return name;
}


//////////////////////////////////////////////////////////////
//	Get_Param_Type
//////////////////////////////////////////////////////////////
inline PARAM_TYPES
EditScriptClass::Get_Param_Type (int index)
{
	PARAM_TYPES type = PARAM_TYPE_INT;

	if (Valid_Index (index)) {		
		type = m_ParamValues[index].type;
	}

	return type;
}


//////////////////////////////////////////////////////////////
//	Set_Param_Value
//////////////////////////////////////////////////////////////
inline void
EditScriptClass::Set_Param_Value (int index, LPCTSTR value)
{
	if (Valid_Index (index)) {
		m_ParamValues[index].value = value;
	}

	return ;
}

INLINE_ACCESSOR_CONST (LPCTSTR, EditScriptClass, Name);

#endif //__EDIT_SCRIPT_H
