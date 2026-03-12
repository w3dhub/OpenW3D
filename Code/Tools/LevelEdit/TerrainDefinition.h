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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TerrainDefinition.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/29/00 2:53p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __TERRAIN_DEFINITION_H
#define __TERRAIN_DEFINITION_H

#include "definition.h"
#include "definitionclassids.h"
#include "vector.h"
#include "wwstring.h"

//////////////////////////////////////////////////////////////////////////////////
//
//	TerrainDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
class TerrainDefinitionClass : public DefinitionClass
{

public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(TerrainDefinitionClass, DefinitionClass);

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	TerrainDefinitionClass (void);
	virtual ~TerrainDefinitionClass (void);

	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const { return CLASSID_TERRAIN; }

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	virtual bool								Save (ChunkSaveClass &csave);
	virtual bool								Load (ChunkLoadClass &cload);
	virtual PersistClass *					Create (void) const;

	// Terrain definition specific
	virtual const char *						Get_Model_Name (void) const;
	virtual void								Set_Model_Name (const char *filename);

	virtual const char *						Get_Light_Filename (void) const					{ return m_LightFilename; }
	virtual void								Set_Light_Filename (const char *filename)		{ m_LightFilename = filename; }

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	bool											Save_Variables (ChunkSaveClass &csave);
	bool											Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////
	DynamicVectorClass<unsigned int>	m_DistanceList;
	StringClass								m_ModelName;
	StringClass								m_LightFilename;


};


/////////////////////////////////////////////////////////////////////
//	Get_Model_Name
/////////////////////////////////////////////////////////////////////
inline const char *
TerrainDefinitionClass::Get_Model_Name (void) const
{
	return m_ModelName;
}


/////////////////////////////////////////////////////////////////////
//	Set_Model_Name
/////////////////////////////////////////////////////////////////////
inline void
TerrainDefinitionClass::Set_Model_Name (const char *filename)
{
	m_ModelName = filename;
	return ;
}


#endif //__TERRAIN_DEFINITION_H

