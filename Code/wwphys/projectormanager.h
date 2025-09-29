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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/projectormanager.h                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/15/00 9:57a                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PROJECTORMANAGER_H
#define PROJECTORMANAGER_H

#include "always.h"
#include "bittype.h"
#include "wwstring.h"


class RenderObjClass;
class TexProjectClass;
class ChunkLoadClass;
class ChunkSaveClass;

class ProjectorManagerDefClass;

/**
** ProjectorManagerClass
** This class is meant to be embedded in a phyisics object and simply manages the 
** chore of updating the transforms for texture projectors which are attached
** to bones in a model
*/
class ProjectorManagerClass
{
public:
	ProjectorManagerClass(void);
	~ProjectorManagerClass(void);
	
	enum 
	{
		IS_ANIMATED = 0x00000001,
	};

	void											Init(const ProjectorManagerDefClass & def,RenderObjClass * model);
	void											Update_From_Model(RenderObjClass * model);

	void											Set_Flag(int flag,bool onoff)		{ (onoff ? Flags |= flag : Flags &= ~flag); }
	bool											Get_Flag(int flag)					{ return ((Flags & flag) == flag); }

protected:
	
	void											Free(void);

	uint16										Flags;
	uint16										ProjectorBoneIndex;
	TexProjectClass *							Projector;

};


/**
** ProjectorManagerDefClass
** This class is meant to be a component of a definition class for a physics object
** which has a ProjectorManager.  The member variables of this class are public since
** various physics def classes make them directly editable.
*/
class ProjectorManagerDefClass
{
public:

	ProjectorManagerDefClass(void);
	~ProjectorManagerDefClass(void);

	void											Validate_Parameters(void);

	bool											Save(ChunkSaveClass &csave);
	bool											Load(ChunkLoadClass &cload);

public:

	bool											IsEnabled;						// should this object create a projector
	bool											IsPerspective;					// is this a perspective projection
	bool											IsAdditive;						// is this an additive projection
	bool											IsAnimated;						// is this projector animated (attached to a bone that animates?)
	float											OrthoWidth;						// width of the orthographic projection
	float											OrthoHeight;					// height of the orthographic projection
	float											HorizontalFOV;					// horizontal field of view
	float											VerticalFOV;					// vertical field of view
	float											NearZ;							// near clip plane
	float											FarZ;								// far clip plane
	float											Intensity;						// intensity of the projector
	StringClass									TextureName;					// name of texture to project
	StringClass									BoneName;						// name of the bone which should control the projector

};


/*
** Use this macro to make all of the member variables in a ProjectorManagerDefClass editable.
** The first parameter to the macro is the type-name of your class (e.g. StaticAnimPhysDefClass) and
** the second parameter is the name of the member variable which is a ProjectorManagerDefClass.
*/
#define	PROJECTORMANAGERDEF_EDITABLE_PARAMS( class_name , member_name )																						\
   PARAM_SEPARATOR( class_name, "Texture Projector Settings");																										\
	NAMED_EDITABLE_PARAM( class_name , ParameterClass::TYPE_BOOL,		(member_name).IsEnabled, "EnableProjector");								\
	NAMED_EDITABLE_PARAM( class_name , ParameterClass::TYPE_BOOL,		(member_name).IsPerspective, "Perspective Projection" );				\
	NAMED_EDITABLE_PARAM( class_name , ParameterClass::TYPE_BOOL,		(member_name).IsAdditive, "Projector Is Additive" );					\
	NAMED_EDITABLE_PARAM( class_name , ParameterClass::TYPE_BOOL,		(member_name).IsAnimated, "Projector Is Animated" );					\
	NAMED_FLOAT_UNITS_PARAM( class_name , (member_name).OrthoWidth , 0.01f , 1000.0f, "meters","Ortho Width" );									\
	NAMED_FLOAT_UNITS_PARAM( class_name , (member_name).OrthoHeight , 0.01f , 1000.0f, "meters","Ortho Height" );								\
	NAMED_ANGLE_EDITABLE_PARAM( class_name , (member_name).HorizontalFOV, DEG_TO_RADF(0.01f), DEG_TO_RADF(89.99f), "Horizontal FOV" );	\
	NAMED_ANGLE_EDITABLE_PARAM( class_name , (member_name).VerticalFOV, DEG_TO_RADF(0.01f), DEG_TO_RADF(89.99f), "Vertical FOV" );			\
	NAMED_FLOAT_UNITS_PARAM( class_name , (member_name).NearZ , 0.01f , 1000.0f, "meters","NearZ" );													\
	NAMED_FLOAT_UNITS_PARAM( class_name , (member_name).FarZ , 0.01f , 1000.0f, "meters","FarZ" );													\
	NAMED_FLOAT_UNITS_PARAM( class_name , (member_name).Intensity , 0.01f , 1.0f, "","Intensity" );													\
	FILENAME_PARAM ( class_name , (member_name).TextureName, "Texture files", ".tga");																	\
	NAMED_EDITABLE_PARAM( class_name , ParameterClass::TYPE_STRING,	(member_name).BoneName, "BoneName" );


#endif

