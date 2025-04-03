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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/lightsolvesavesystem.cpp     $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/04/02 11:41a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "stdafx.h"
#include "lightsolvesavesystem.h"
#include "sceneeditor.h"
#include "utils.h"
#include "chunkio.h"
#include "saveload.h"
#include "editorchunkids.h"
#include "staticphys.h"
#include "pscene.h"
#include "dx8renderer.h"


/*
** Instantiate the save system
*/
LightSolveSaveSystemClass _TheLightSolveSaveSystem;


/************************************************************************************************
**
**  LightSolveSaveSystemClass Implementation
**
************************************************************************************************/


uint32 LightSolveSaveSystemClass::Chunk_ID(void) const
{
	return CHUNKID_EDITOR_LIGHT_SOLVE_SAVELOAD;
}


bool LightSolveSaveSystemClass::Contains_Data(void) const
{
	// TODO: could check if we have anything to save...
	return true;
}


bool LightSolveSaveSystemClass::Save(ChunkSaveClass &csave)
{	
	SceneEditorClass * scene = ::Get_Scene_Editor();
	WWASSERT(scene != NULL);

	RefPhysListIterator it = scene->Get_Static_Object_Iterator();
	while (!it.Is_Done()) {
		StaticPhysClass * pobj = it.Peek_Obj()->As_StaticPhysClass();
		if (	(pobj != NULL) && 
				(pobj->Peek_Model() != NULL) && 
				(pobj->Peek_Model()->Has_User_Lighting()) ) 
		{
			csave.Begin_Chunk(LSS_CHUNKID_OBJECT_LIGHT_SOLVE);
			Save_Lighting_For_Object(csave,pobj);
			csave.End_Chunk();
		}

		it.Next();
	}
	return true;
}


bool LightSolveSaveSystemClass::Save_Lighting_For_Object(ChunkSaveClass & csave,StaticPhysClass * pobj)
{	
	RenderObjClass * model = pobj->Peek_Model();
	uint32 id = pobj->Get_ID();
	uint32 classid = (uint32)model->Class_ID();
	uint32 subobjcount = (uint32)model->Get_Num_Sub_Objects();	

	csave.Begin_Chunk(LSS_CHUNKID_OBJECT_VARIABLES);
	WRITE_MICRO_CHUNK(csave,LSS_VARIABLE_OBJECT_ID,id);
	WRITE_MICRO_CHUNK(csave,LSS_VARIABLE_OBJECT_CLASSID,classid);
	WRITE_MICRO_CHUNK(csave,LSS_VARIABLE_OBJECT_SUBOBJCOUNT,subobjcount);
	csave.End_Chunk();

	csave.Begin_Chunk(LSS_CHUNKID_OBJECT_LIGHTING);
	model->Save_User_Lighting(csave);
	csave.End_Chunk();

	return true;
}


bool LightSolveSaveSystemClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case LSS_CHUNKID_OBJECT_LIGHT_SOLVE:
				Load_Lighting_For_Object(cload);
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		
		cload.Close_Chunk();
	}
	return true;
}


bool LightSolveSaveSystemClass::Load_Lighting_For_Object(ChunkLoadClass & cload)
{	
	uint32 id = 0;
	uint32 classid = 0;
	uint32 subobjcount = 0;	

	/*
	** Read in the variables, return if we do not detect the variables chunk first
	*/
	cload.Open_Chunk();
	if (cload.Cur_Chunk_ID() != LSS_CHUNKID_OBJECT_VARIABLES) {
		cload.Close_Chunk();
		return false;
	}

	if (cload.Cur_Chunk_ID() == LSS_CHUNKID_OBJECT_VARIABLES) {
		while (cload.Open_Micro_Chunk()) {
			switch(cload.Cur_Micro_Chunk_ID()) {
				READ_MICRO_CHUNK(cload,LSS_VARIABLE_OBJECT_ID,id);
				READ_MICRO_CHUNK(cload,LSS_VARIABLE_OBJECT_CLASSID,classid);
				READ_MICRO_CHUNK(cload,LSS_VARIABLE_OBJECT_SUBOBJCOUNT,subobjcount);
			}
			cload.Close_Micro_Chunk();	
		}
	}
	cload.Close_Chunk();

	/*
	** Look up the object 
	*/
	StaticPhysClass * obj = PhysicsSceneClass::Get_Instance()->Get_Static_Object_By_ID(id);
	
	if (obj != NULL) {
	
		RenderObjClass * model = obj->Peek_Model();
		if (	(model != NULL) &&
				(obj->Get_ID() == id) &&
				(model->Class_ID() == (int)classid) &&
				(model->Get_Num_Sub_Objects() == (int)subobjcount) )
		{
			/*
			** If we find the object, load its lighting data
			*/
			if ((cload.Open_Chunk() == true) && (cload.Cur_Chunk_ID() == LSS_CHUNKID_OBJECT_LIGHTING)) {
				model->Load_User_Lighting(cload);
				cload.Close_Chunk();
			}
		}
	}
	return true;
}


void LightSolveSaveSystemClass::On_Post_Load(void)
{
	/*
	** The dx8 renderer needs to be reset so that the light solves get installed
	*/
	TheDX8MeshRenderer.Invalidate();
}




