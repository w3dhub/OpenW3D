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
 *                     $Archive:: /Commando/Code/wwphys/vissectorstats.cpp                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/06/01 5:45p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Count_Textures -- counts textures used by the given object                                *
 *   VisSectorStatsClass::VisSectorStatsClass -- Constructor                                   *
 *   VisSectorStatsClass::~VisSectorStatsClass -- Destructor                                   *
 *   VisSectorStatsClass::Compute_Stats -- Initializes the statistics for the given object     *
 *	  VisSectorStatsClass::operator= -- Safely copies the member data from the source object.	  *
 *	  VisSectorStatsClass::Get_Name -- Returns the name of the model this vis sector contains	  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "vissectorstats.h"
#include "pscene.h"
#include "vistable.h"
#include "staticphys.h"
#include "statistics.h"
#include "matinfo.h"



/***********************************************************************************************
 * Count_Textures -- counts textures used by the given object                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/6/2000   gth : Created.                                                                 *
 *=============================================================================================*/
static void Count_Textures(RenderObjClass * obj)
{
	if (obj == NULL) {
		return;
	}

	/*
	** Count the textures for the sub objects
	*/
	for (int i=0; i<obj->Get_Num_Sub_Objects(); i++) {
		RenderObjClass * subobj = obj->Get_Sub_Object(i);
		Count_Textures(subobj);
		REF_PTR_RELEASE(subobj);
	}
	
	/*
	** Count the textures for this object
	*/
	MaterialInfoClass * matinfo = obj->Get_Material_Info();
	if (matinfo != NULL) {
		for (int ti=0; ti<matinfo->Texture_Count(); ti++) {
			Debug_Statistics::Record_Texture(matinfo->Peek_Texture(ti));
		}
	}
}


/*********************************************************************************************
**
** VisSectorStatsClass Implementation
**
*********************************************************************************************/			


/***********************************************************************************************
 * VisSectorStatsClass::VisSectorStatsClass -- Constructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/6/2000   gth : Created.                                                                 *
 *=============================================================================================*/
VisSectorStatsClass::VisSectorStatsClass(void) :
	VisId(-1),
	PolygonCount(0),
	TextureCount(0),
	TextureBytes(0),
	CenterPoint(0,0,0),
	PhysObj(NULL)
{
}


/***********************************************************************************************
 * VisSectorStatsClass::VisSectorStatsClass -- Constructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/6/2000   pds : Created.                                                                 *
 *=============================================================================================*/
VisSectorStatsClass::VisSectorStatsClass(const VisSectorStatsClass &src) :
	VisId(-1),
	PolygonCount(0),
	TextureCount(0),
	TextureBytes(0),
	CenterPoint(0,0,0),
	PhysObj(NULL)
{
	(*this) = src;
}


/***********************************************************************************************
 * VisSectorStatsClass::~VisSectorStatsClass -- Destructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
VisSectorStatsClass::~VisSectorStatsClass(void)
{
	REF_PTR_RELEASE(PhysObj);
}


/***********************************************************************************************
 * VisSectorStatsClass::Compute_Stats -- Initializes the statistics for the given object       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/6/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void VisSectorStatsClass::Compute_Stats(StaticPhysClass * obj,VisTableClass * vistable)
{
	/*
	** Save the vis id and a point above the center of the object
	*/
	VisId = obj->Get_Vis_Object_ID();
	CenterPoint = obj->Get_Cull_Box().Center + Vector3(0.0f,0.0f,2.0f);

	/*
	** Reset the texture statistics tracker
	*/
	PolygonCount = 0;
	TextureCount = 0;
	TextureBytes = 0;

	if (vistable != NULL) {
		Debug_Statistics::Begin_Statistics();
		Debug_Statistics::Record_Texture_Mode(Debug_Statistics::RECORD_TEXTURE_DETAILS);

		/*
		** Walk through every static object in the level, adding its resources
		** to our stats if it is visible from this sector
		*/
		RefPhysListIterator it = PhysicsSceneClass::Get_Instance()->Get_Static_Object_Iterator();
		for (it.First(); !it.Is_Done(); it.Next()) {
			
			StaticPhysClass * next_obj = it.Peek_Obj()->As_StaticPhysClass();
			if (next_obj && vistable->Get_Bit(next_obj->Get_Vis_Object_ID())) {

				PolygonCount += next_obj->Peek_Model()->Get_Num_Polys();

				Count_Textures(next_obj->Peek_Model());

			}
		}

		/*
		** Finish the texture statistics
		*/
		Debug_Statistics::Record_Texture_Mode(Debug_Statistics::RECORD_TEXTURE_NONE);
		Debug_Statistics::End_Statistics();

		TextureCount = Debug_Statistics::Get_Record_Texture_Count();
		TextureBytes = Debug_Statistics::Get_Record_Texture_Size();
	}

	/*
	** Save a reference to the physics object
	*/
	REF_PTR_SET (PhysObj, obj);
}


/***********************************************************************************************
 * VisSectorStatsClass::operator= -- Safely copies the member data from the source object.	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/6/2000   pds : Created.                                                                 *
 *=============================================================================================*/
const VisSectorStatsClass &VisSectorStatsClass::operator= (const VisSectorStatsClass &src)
{
	VisId				= src.VisId;
	PolygonCount	= src.PolygonCount;
	TextureCount	= src.TextureCount;
	TextureBytes	= src.TextureBytes;
	CenterPoint		= src.CenterPoint;
	REF_PTR_SET (PhysObj, src.PhysObj);
	return (*this);
}


/***********************************************************************************************
 * VisSectorStatsClass::Get_Name -- Returns the name of the model this vis sector contains	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/6/2000   pds : Created.                                                                 *
 *=============================================================================================*/
const char *VisSectorStatsClass::Get_Name(void)
{
	const char *name = NULL;

	if (PhysObj != NULL && PhysObj->Peek_Model () != NULL) {
		name = PhysObj->Peek_Model ()->Get_Name ();
	}

	return name;
}

