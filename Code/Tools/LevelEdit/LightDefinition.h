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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/LightDefinition.h     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/26/00 5:33p                                                $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __LIGHT_DEFINITION_H
#define __LIGHT_DEFINITION_H

#include "definition.h"
#include "definitionclassids.h"
#include "vector.h"
#include "wwstring.h"
#include "light.h"


//////////////////////////////////////////////////////////////////////////////////
//
//	LightDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
class LightDefinitionClass : public DefinitionClass
{

public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(LightDefinitionClass, DefinitionClass);

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	LightDefinitionClass (void);
	virtual ~LightDefinitionClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const LightDefinitionClass &	operator= (const LightDefinitionClass &src);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const { return CLASSID_LIGHT; }

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	virtual bool								Save (ChunkSaveClass &csave);
	virtual bool								Load (ChunkLoadClass &cload);
	virtual PersistClass *					Create (void) const;

	// Light definition specific
	virtual bool								Does_Cast_Shadows (void) const;
	virtual void								Casts_Shadows (bool onoff);

	virtual const Vector3 &					Get_Ambient_Color (void) const;
	virtual void								Set_Ambient_Color (const Vector3 &color);

	virtual const Vector3 &					Get_Diffuse_Color (void) const;
	virtual void								Set_Diffuse_Color (const Vector3 &color);

	virtual const Vector3 &					Get_Specular_Color (void) const;
	virtual void								Set_Specular_Color (const Vector3 &color);

	virtual float								Get_Intensity (void) const;
	virtual void								Set_Intensity (float intensity);

	virtual float								Get_Inner_Radius (void) const;
	virtual void								Set_Inner_Radius (float intensity);

	virtual float								Get_Outer_Radius (void) const;
	virtual void								Set_Outer_Radius (float intensity);

	virtual LightClass::LightType			Get_Light_Type (void) const;
	virtual void								Set_Light_Type (LightClass::LightType type);

	virtual const Vector3 &					Get_Spot_Direction (void) const;
	virtual void								Set_Spot_Direction (const Vector3 &dir);

	virtual float								Get_Spot_Angle (void) const;
	virtual void								Set_Spot_Angle (float angle);

	virtual float								Get_Spot_Exponent (void) const;
	virtual void								Set_Spot_Exponent (float exp);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	bool											Save_Variables (ChunkSaveClass &csave);
	bool											Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////
	Vector3						m_AmbientColor;
	Vector3						m_DiffuseColor;
	Vector3						m_SpecularColor;
	Vector3						m_SpotDir;
	float							m_SpotAngle;
	float							m_SpotExp;
	float							m_Intensity;
	float							m_FarRadiusInner;
	float							m_FarRadiusOuter;
	bool							m_CastsShadows;
	int							m_LightType;
};


/////////////////////////////////////////////////////////////////////
//	Get_Ambient_Color
/////////////////////////////////////////////////////////////////////
inline const Vector3 &
LightDefinitionClass::Get_Ambient_Color (void) const
{
	return m_AmbientColor;
}

/////////////////////////////////////////////////////////////////////
//	Set_Ambient_Color
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Ambient_Color (const Vector3 &color)
{
	m_AmbientColor = color;
	return ;
}

/////////////////////////////////////////////////////////////////////
//	Get_Diffuse_Color
/////////////////////////////////////////////////////////////////////
inline const Vector3 &
LightDefinitionClass::Get_Diffuse_Color (void) const
{
	return m_DiffuseColor;
}

/////////////////////////////////////////////////////////////////////
//	Set_Diffuse_Color
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Diffuse_Color (const Vector3 &color)
{
	m_DiffuseColor = color;
	return ;
}

/////////////////////////////////////////////////////////////////////
//	Get_Specular_Color
/////////////////////////////////////////////////////////////////////
inline const Vector3 &
LightDefinitionClass::Get_Specular_Color (void) const
{
	return m_SpecularColor;
}

/////////////////////////////////////////////////////////////////////
//	Set_Specular_Color
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Specular_Color (const Vector3 &color)
{
	m_SpecularColor = color;
	return ;
}

/////////////////////////////////////////////////////////////////////
//	Get_Intensity
/////////////////////////////////////////////////////////////////////
inline float
LightDefinitionClass::Get_Intensity (void) const
{
	return m_Intensity;
}

/////////////////////////////////////////////////////////////////////
//	Set_Intensity
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Intensity (float intensity)
{
	m_Intensity = intensity;
	return ;
}

/////////////////////////////////////////////////////////////////////
//	Get_Inner_Radius
/////////////////////////////////////////////////////////////////////
inline float
LightDefinitionClass::Get_Inner_Radius (void) const
{
	return m_FarRadiusInner;
}

/////////////////////////////////////////////////////////////////////
//	Set_Inner_Radius
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Inner_Radius (float radius)
{
	m_FarRadiusInner = radius;
	return ;
}

/////////////////////////////////////////////////////////////////////
//	Get_Outer_Radius
/////////////////////////////////////////////////////////////////////
inline float
LightDefinitionClass::Get_Outer_Radius (void) const
{
	return m_FarRadiusOuter;
}

/////////////////////////////////////////////////////////////////////
//	Set_Outer_Radius
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Outer_Radius (float radius)
{
	m_FarRadiusOuter = radius;
	return ;
}

/////////////////////////////////////////////////////////////////////
//	Does_Cast_Shadows
/////////////////////////////////////////////////////////////////////
inline bool
LightDefinitionClass::Does_Cast_Shadows (void) const
{
	return m_CastsShadows;
}

/////////////////////////////////////////////////////////////////////
//	Casts_Shadows
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Casts_Shadows (bool onoff)
{
	m_CastsShadows = onoff;
	return ;
}

/////////////////////////////////////////////////////////////////////
//	Get_Light_Type
/////////////////////////////////////////////////////////////////////
inline LightClass::LightType
LightDefinitionClass::Get_Light_Type (void) const
{
	return (LightClass::LightType)m_LightType;
}

/////////////////////////////////////////////////////////////////////
//	Set_Light_Type
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Light_Type (LightClass::LightType type)
{
	m_LightType = (int)type;
	return ;
}


/////////////////////////////////////////////////////////////////////
//	Get_Spot_Direction
/////////////////////////////////////////////////////////////////////
inline const Vector3 &
LightDefinitionClass::Get_Spot_Direction (void) const
{
	return m_SpotDir;
}


/////////////////////////////////////////////////////////////////////
//	Set_Spot_Direction
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Spot_Direction (const Vector3 &dir)
{
	m_SpotDir = dir;
	return ;
}


/////////////////////////////////////////////////////////////////////
//	Get_Spot_Angle
/////////////////////////////////////////////////////////////////////
inline float
LightDefinitionClass::Get_Spot_Angle (void) const
{
	return m_SpotAngle;
}


/////////////////////////////////////////////////////////////////////
//	Set_Spot_Angle
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Spot_Angle (float angle)
{
	m_SpotAngle = angle;
	return ;
}


/////////////////////////////////////////////////////////////////////
//	Get_Spot_Exponent
/////////////////////////////////////////////////////////////////////
inline float
LightDefinitionClass::Get_Spot_Exponent (void) const
{
	return m_SpotExp;
}


/////////////////////////////////////////////////////////////////////
//	Set_Spot_Exponent
/////////////////////////////////////////////////////////////////////
inline void
LightDefinitionClass::Set_Spot_Exponent (float exp)
{
	m_SpotExp = exp;
	return ;
}


#endif //__LIGHT_DEFINITION_H

