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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/DefinitionParameter.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/12/00 6:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DEFINITION_PARAMETER_H
#define __DEFINITION_PARAMETER_H

#include "refcount.h"
#include "Utils.h"


/////////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////////
class DefinitionClass;
class ParameterClass;


/////////////////////////////////////////////////////////////////////////////
//
// DefinitionParameterClass
//
/////////////////////////////////////////////////////////////////////////////
class DefinitionParameterClass : public RefCountClass
{
public:

	////////////////////////////////////////////////////////////
	//	Public constructors/destructor
	////////////////////////////////////////////////////////////
	DefinitionParameterClass (void)
		:	m_Definition (NULL),
			m_Parameter (NULL),
			m_Index (0),
			m_Parent (NULL) {}

	virtual ~DefinitionParameterClass (void) { MEMBER_RELEASE (m_Parent); }

	////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////
	
	//
	//	Parameter info
	//
	DefinitionClass *	Get_Definition (void) const				{ return m_Definition; }
	ParameterClass *	Get_Parameter (void) const					{ return m_Parameter; }
	int					Get_Index (void) const						{ return m_Index; }
	void					Set_Definition (DefinitionClass *def)	{ m_Definition = def; }
	void					Set_Parameter (ParameterClass *param)	{ m_Parameter = param; }
	void					Set_Index (int index)						{ m_Index = index; }

	//
	//	Parent info
	//
	DefinitionParameterClass *	Peek_Parent (void) const					{ return m_Parent; }
	void					Set_Parent (DefinitionParameterClass *	parent)	{ MEMBER_ADD (m_Parent, parent); }

private:
	
	////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////
	DefinitionClass *				m_Definition;
	ParameterClass *				m_Parameter;
	int								m_Index;

	DefinitionParameterClass *	m_Parent;
};


#endif //__DEFINITION_PARAMETER_H
