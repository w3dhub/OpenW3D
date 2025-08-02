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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/LightDefinition.cpp    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/20/00 2:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "lightdefinition.h"
#include "simpledefinitionfactory.h"
#include "definitionclassids.h"
#include "definitionmgr.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "lightnode.h"


//////////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x00000100,
	CHUNKID_BASE_CLASS		= 0x00000200,
};

enum
{
	VARID_CASTS_SHADOWS		= 0x01,
	VARID_AMBIENT_COLOR,
	VARID_DIFFUSE_COLOR,
	VARID_SPECULAR_COLOR,
	VARID_INTENSITY,
	VARID_INNER_RADIUS,
	VARID_OUTER_RADIUS,
	VARID_LIGHT_TYPE,
	VARID_LIGHT_SPOT_ANGLE,
	VARID_LIGHT_SPOT_EXPONENT,
	VARID_LIGHT_SPOT_DIRECTION,
};


//////////////////////////////////////////////////////////////////////////////////
//
//	Static factories
//
//////////////////////////////////////////////////////////////////////////////////
DECLARE_DEFINITION_FACTORY(LightDefinitionClass, CLASSID_LIGHT, "Light")	_LightDefFactory;
SimplePersistFactoryClass<LightDefinitionClass, CHUNKID_LIGHT_DEF>			_LightPersistFactory;


//////////////////////////////////////////////////////////////////////////////////
//
//	LightDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
LightDefinitionClass::LightDefinitionClass (void)
	:	m_CastsShadows (false),
		m_AmbientColor (0.7F, 0.7F, 0.7F),
		m_DiffuseColor (0.7F, 0.7F, 0.7F),
		m_SpecularColor (0.7F, 0.7F, 0.7F),
		m_Intensity (1.0F),
		m_FarRadiusInner (10.0F),
		m_FarRadiusOuter (20.0F),
		m_SpotDir (0, 0, 0),
		m_SpotAngle (0),
		m_SpotExp (0),
		m_LightType (LightClass::POINT),
		DefinitionClass ()				
{
	NAMED_EDITABLE_PARAM (LightDefinitionClass, ParameterClass::TYPE_BOOL, m_CastsShadows, "Casts Shadows");
	NAMED_EDITABLE_PARAM (LightDefinitionClass, ParameterClass::TYPE_COLOR, m_AmbientColor, "Ambient");
	NAMED_EDITABLE_PARAM (LightDefinitionClass, ParameterClass::TYPE_COLOR, m_DiffuseColor, "Diffuse");
	NAMED_EDITABLE_PARAM (LightDefinitionClass, ParameterClass::TYPE_COLOR, m_SpecularColor, "Specular");
	NAMED_EDITABLE_PARAM (LightDefinitionClass, ParameterClass::TYPE_FLOAT, m_FarRadiusInner, "Inner Radius");
	NAMED_EDITABLE_PARAM (LightDefinitionClass, ParameterClass::TYPE_FLOAT, m_FarRadiusOuter, "Outer Radius");
	FLOAT_EDITABLE_PARAM (LightDefinitionClass, m_Intensity, 0, 1.0F);
	ENUM_PARAM (LightDefinitionClass, m_LightType, ("Point", LightClass::POINT, "Directional", LightClass::DIRECTIONAL, "Spotlight", LightClass::SPOT, NULL));

	NAMED_EDITABLE_PARAM (LightDefinitionClass, ParameterClass::TYPE_ANGLE, m_SpotAngle, "Spot Angle");
	NAMED_EDITABLE_PARAM (LightDefinitionClass, ParameterClass::TYPE_FLOAT, m_SpotExp, "Spot Exponent");
	NAMED_EDITABLE_PARAM (LightDefinitionClass, ParameterClass::TYPE_VECTOR3, m_SpotDir, "Spot Direction");
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~LightDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
LightDefinitionClass::~LightDefinitionClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	operator=
//
//////////////////////////////////////////////////////////////////////////////////
const LightDefinitionClass &
LightDefinitionClass::operator= (const LightDefinitionClass &src)
{
	m_CastsShadows		= src.m_CastsShadows;
	m_AmbientColor		= src.m_AmbientColor;
	m_DiffuseColor		= src.m_DiffuseColor;
	m_SpecularColor	= src.m_SpecularColor;
	m_Intensity			= src.m_Intensity;
	m_FarRadiusInner	= src.m_FarRadiusInner;
	m_FarRadiusOuter	= src.m_FarRadiusOuter;
	m_LightType			= src.m_LightType;
	return *this;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
//////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
LightDefinitionClass::Get_Factory (void) const
{
	return _LightPersistFactory;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
LightDefinitionClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;

	csave.Begin_Chunk (CHUNKID_VARIABLES);
	retval &= Save_Variables (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
	retval &= DefinitionClass::Save (csave);
	csave.End_Chunk ();

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
LightDefinitionClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;

			case CHUNKID_BASE_CLASS:
				retval &= DefinitionClass::Load (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
LightDefinitionClass::Save_Variables (ChunkSaveClass &csave)
{
	bool retval = true;

	WRITE_MICRO_CHUNK (csave, VARID_CASTS_SHADOWS, m_CastsShadows)	
	WRITE_MICRO_CHUNK (csave, VARID_AMBIENT_COLOR, m_AmbientColor)
	WRITE_MICRO_CHUNK (csave, VARID_DIFFUSE_COLOR, m_DiffuseColor)
	WRITE_MICRO_CHUNK (csave, VARID_SPECULAR_COLOR, m_SpecularColor)
	WRITE_MICRO_CHUNK (csave, VARID_INTENSITY, m_Intensity)
	WRITE_MICRO_CHUNK (csave, VARID_INNER_RADIUS, m_FarRadiusInner)
	WRITE_MICRO_CHUNK (csave, VARID_OUTER_RADIUS, m_FarRadiusOuter)
	WRITE_MICRO_CHUNK (csave, VARID_LIGHT_TYPE, m_LightType)	
	WRITE_MICRO_CHUNK (csave, VARID_LIGHT_SPOT_ANGLE,	m_SpotAngle)
	WRITE_MICRO_CHUNK (csave, VARID_LIGHT_SPOT_EXPONENT, m_SpotExp)
	WRITE_MICRO_CHUNK (csave, VARID_LIGHT_SPOT_DIRECTION, m_SpotDir)
	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
LightDefinitionClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_CASTS_SHADOWS, m_CastsShadows)
			READ_MICRO_CHUNK (cload, VARID_AMBIENT_COLOR, m_AmbientColor)
			READ_MICRO_CHUNK (cload, VARID_DIFFUSE_COLOR, m_DiffuseColor)
			READ_MICRO_CHUNK (cload, VARID_SPECULAR_COLOR, m_SpecularColor)
			READ_MICRO_CHUNK (cload, VARID_INTENSITY, m_Intensity)
			READ_MICRO_CHUNK (cload, VARID_INNER_RADIUS, m_FarRadiusInner)
			READ_MICRO_CHUNK (cload, VARID_OUTER_RADIUS, m_FarRadiusOuter)
			READ_MICRO_CHUNK (cload, VARID_LIGHT_TYPE, m_LightType)	
			READ_MICRO_CHUNK (cload, VARID_LIGHT_SPOT_ANGLE,	m_SpotAngle)
			READ_MICRO_CHUNK (cload, VARID_LIGHT_SPOT_EXPONENT, m_SpotExp)
			READ_MICRO_CHUNK (cload, VARID_LIGHT_SPOT_DIRECTION, m_SpotDir)
		}

		cload.Close_Micro_Chunk ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////////////////
PersistClass *
LightDefinitionClass::Create (void) const
{
	return new LightNodeClass ();
}

