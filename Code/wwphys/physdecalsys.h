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
 *                     $Archive:: /Commando/Code/wwphys/physdecalsys.h                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/29/01 11:26a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef PHYSDECALSYS_H
#define PHYSDECALSYS_H

#include "always.h"
#include "vector.h"
#include "simplevec.h"
#include "multilist.h"
#include "decalsys.h"

class MeshClass;
class PhysicsSceneClass;
class CameraClass;
class PhysClass;
class DistAlphaVPClass;

/**
** PhysDecalSysClass
** This derived DecalSystemClass provides two pools of decals.  One pool is a 
** fixed size buffer of decals which are recycled when the buffer fills up.  The second
** pool is for "permanent" decals.
**
** Saving and loading of decals is currently not possible so the external user will have
** to track the parameters used to create any decals that he wants to survive across
** a save-load.  
**
** The decal ID used by this system is formatted in a way so that we 
*/
class PhysDecalSysClass : public DecalSystemClass
{
public:

	PhysDecalSysClass(PhysicsSceneClass * parent_scene);
	virtual ~PhysDecalSysClass(void);

	/*
	** Create_Decal - this is the interface typically used by PhysicsSceneClass to create new decals
	** Update_Decal_Fade_Distances - this updates the alpha fading parameters
	*/
	void										Update_Decal_Fade_Distances(const CameraClass & camera);
	int										Create_Decal(		const Matrix3D &	tm,
																		const char *		texture_name,
																		float					radius,
																		bool					is_permanent,
																		bool					apply_to_translucent_meshes,
																		PhysClass *			only_this_obj);
	
	bool										Remove_Decal(uint32 id);

	/*
	**	Create and release DecalGenerators.  Note that this is the point at which the 
	** decal system can track "logical" decals.  The generator will keep an internal list
	** of all of the render objects which generated decals which you should copy if you
	** want to track them (e.g. if you want to cap the maximum number of active decals and
	** kill the old ones...)
	*/
	virtual void							Unlock_Decal_Generator(DecalGeneratorClass * generator) override;

	/*
	** When a decal-mesh is destroyed, it must inform the DecalSystem.  Otherwise, systems 
	** which track decals can get dangling pointers.
	*/
	virtual void							Decal_Mesh_Destroyed(uint32 decal_id,DecalMeshClass * mesh) override;

	/*
	** Control over the size of the temporary decal array.  When this array is filled, the
	** oldest decals are removed as new decals are added.
	*/
	void										Set_Temporary_Decal_Pool_Size(int count);
	int										Get_Temporary_Decal_Pool_Size(void);

protected:

	virtual uint32							Generate_Decal_Id(void) override;
	bool										is_decal_id_permanent(uint32 id);
	bool										internal_remove_decal(uint32 id,MeshClass * mesh);

	void										allocate_resources(void);
	void										release_resources(void);


	/**
	** LogicalDecalClass
	** This class is used to track all of the meshes that were affected when a 
	** decal is generated.
	*/
	class LogicalDecalClass : public MultiListObjectClass
	{
	public:
		LogicalDecalClass(void);
		~LogicalDecalClass(void);

		bool										operator == (const LogicalDecalClass & ) { return false; }
		bool										operator != (const LogicalDecalClass & ) { return true; }

		void										Reset(void);
		void										Init(DecalGeneratorClass * gen);

		uint32									DecalID;
		SimpleDynVecClass<MeshClass *>	Meshes;
	};

	PhysicsSceneClass *						ParentScene;				// scene that this decal system works with
	bool											CreatePermanentDecals;	// internal setting, are we creating permanent or temporary decals

	uint32										NextTempDecalIndex;		// index of the next temporary decal
	VectorClass<LogicalDecalClass>		TempDecals;					// array of logical decals for 
	MultiListClass<LogicalDecalClass>	PermanentDecals;			// linked list of permanent decals

	VertexMaterialClass *					DecalMaterial;				// material used by all decals in WWPhys
	ShaderClass									DecalShader;				// shader used by all decals in WWPhys

	DistAlphaVPClass*							DecalDistAlphaVP;
};



#endif //PHYSDECALSYS_H

