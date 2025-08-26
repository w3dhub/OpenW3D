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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ModelUtils.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/10/00 11:57a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "modelutils.h"
#include "rendobj.h"
#include "Utils.h"


/////////////////////////////////////////////////////////////////////////
//
//	Set_Model_Collision_Type
//
/////////////////////////////////////////////////////////////////////////
void
Set_Model_Collision_Type (RenderObjClass *model, int collision_type)
{
	if (model == NULL) {
		return ;
	}
		
	//
	//	Loop over all the sub-objects
	//
	int count = model->Get_Num_Sub_Objects ();
	for (int index = 0; index < count; index ++) {
		
		//
		// Get a pointer to this subobject
		//
		RenderObjClass *sub_object = model->Get_Sub_Object (index);
		if (sub_object != NULL) {
			
			//
			//	Set the collision bits for this sub-object
			//
			::Set_Model_Collision_Type (sub_object, collision_type);
			MEMBER_RELEASE (sub_object);
		}
	}	

	//
	//	Set the collision type for this model
	//
	if (	model->Is_Not_Hidden_At_All () &&
			model->Class_ID () != RenderObjClass::CLASSID_AABOX &&
			model->Class_ID () != RenderObjClass::CLASSID_OBBOX)
	{
		model->Set_Collision_Type (collision_type);
	}

	return ;
}
