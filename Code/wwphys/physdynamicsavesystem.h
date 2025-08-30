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
 *                     $Archive:: /Commando/Code/wwphys/physdynamicsavesystem.h               $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/07/00 9:49a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYSDYNAMICSAVESYSTEM_H
#define PHYSDYNAMICSAVESYSTEM_H

#include "always.h"
#include "saveloadsubsystem.h"


/******************************************************************************************
**
** PhysDynamicSaveSystemClass
** This SaveLoadSubSystem handles saving the dynamic data from the physics system.
**
******************************************************************************************/
class PhysDynamicSaveSystemClass : public SaveLoadSubSystemClass
{
public:
	
	virtual uint32				Chunk_ID (void) const override;

protected:

	virtual bool				Contains_Data(void) const override;
	virtual bool				Save (ChunkSaveClass &csave) override;
	virtual bool				Load (ChunkLoadClass &cload) override;
	virtual const char*		Name() const override { return "PhysDynamicSaveSystemClass"; }
	virtual void				On_Post_Load(void) override;

	/*
	** internal chunk id's
	*/
	enum 
	{
		PDSSC_CHUNKID_SCENE			= 0x00007001,
		PDSSC_CHUNKID_CONSTANTS,
		PDSSC_CHUNKID_PATHMGR
	};
};

/*
** _WWPhysDynamicSaveSystem - global instance of the sub-system for dynamic data
*/
extern PhysDynamicSaveSystemClass _PhysDynamicSaveSystem;

#endif
