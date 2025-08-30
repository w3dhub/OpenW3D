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
 *                     $Archive:: /Commando/Code/wwphys/physstaticsavesystem.h                $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/02/00 6:20p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYSSTATICSAVESYSTEM_H
#define PHYSSTATICSAVESYSTEM_H

#include "always.h"
#include "saveloadsubsystem.h"


/**
**
** PhysStaticDataSaveSystemClass
** This SaveLoadSubSystem handles saving the static "frame-work" of the physics scene.
** The "framework" for the scene is all of the pre-calculated static data like visibility
** tables and culling systems that the physics scene contains.  This data is saved through
** a separate sub-system in order to allow our Editor to save it without also saving the
** static objects which exist inside the "framework".  The editor normally doesn't save the
** objects in the physics scene since it re-creates them from their "definitions" each
** time a level is loaded into it.  However, this data (visibility, pathfinding...) is too 
** expensive to simply re-create each time so we put it into a separate sub-system that it
** can save.
**
*/
class PhysStaticDataSaveSystemClass : public SaveLoadSubSystemClass
{
public:
	
	virtual uint32				Chunk_ID (void) const override;

protected:

	virtual bool				Save (ChunkSaveClass &csave) override;
	virtual bool				Load (ChunkLoadClass &cload) override;
	virtual const char*		Name() const override { return "PhysStaticSaveSystemClass"; }
	virtual void				On_Post_Load(void) override;

	/*
	** internal chunk id's
	*/
	enum 
	{
		PSDSSC_CHUNKID_SCENE				= 0x04433220,
		PSDSSC_CHUNKID_PATHFIND
	};
};

/**
** global instance of the static data save sub-system
*/
extern PhysStaticDataSaveSystemClass _PhysStaticDataSaveSystem;


/**
**
** PhysStaticObjectsSaveSystemClass
** This SaveLoadSubSystem handles saving the static objects in the physics system.
** This sub-system is not called by the editor so data which needs to persist through
** an editor save-load needs to be placed in the PhysStaticFrameworkSaveSystem.  On
** the other hand, all of the objects that are re-created by the editor need to be 
** saved by this sub-system in order for them to persist when a level is exported to
** the game.  
**
*/
class PhysStaticObjectsSaveSystemClass : public SaveLoadSubSystemClass
{
public:
	
	virtual uint32				Chunk_ID (void) const override;

protected:

	virtual bool				Save (ChunkSaveClass &csave) override;
	virtual bool				Load (ChunkLoadClass &cload) override;
	virtual const char*		Name() const override { return "PhysStaticObjectsSaveSystemClass"; }
	virtual void				On_Post_Load (void) override;

	/*
	** internal chunk id's
	*/
	enum 
	{
		PSOSSC_CHUNKID_SCENE				= 0x06090609,
	};
};

/**
** global instance of the static objects save sub-system
*/
extern PhysStaticObjectsSaveSystemClass _PhysStaticObjectsSaveSystem;

#endif
