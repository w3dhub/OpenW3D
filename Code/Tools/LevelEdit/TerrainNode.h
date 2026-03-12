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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TerrainNode.h                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/12/02 1:44p                                               $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TERRAIN_NODE_H
#define __TERRAIN_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "tilenode.h"
#include "terrainsectionpersist.h"
#include "proxy.h"

////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class PresetClass;
class CollectionClass;


////////////////////////////////////////////////////////////////////////////
//
//	TerrainSectionNodeClass
//
////////////////////////////////////////////////////////////////////////////
class TerrainSectionNodeClass : public TileNodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	TerrainSectionNodeClass (void)	:
		Terrain (NULL)						{}
	~TerrainSectionNodeClass (void)	{}

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	//
	// From NodeClass
	//
	NodeClass *	Clone (void)											{ return NULL; }
	void			Initialize (void)										{}
	NODE_TYPE	Get_Type (void) const								{ return NODE_TYPE_TERRAIN_SECTION; }
	int			Get_Icon_Index (void) const						{ return TERRAIN_ICON; }
	void			Set_Transform (const Matrix3D &tm);
	void			Special_Set_Transform (const Matrix3D &tm)	{ NodeClass::Set_Transform (tm); }
	void			Pre_Export (void)		{}
	void			Post_Export (void)	{}

	//
	// Terrain Section specific
	//
	void						Create (RenderObjClass *render_obj);
	TerrainNodeClass *	Peek_Terrain (void) const						{ return Terrain; }
	void						Set_Terrain (TerrainNodeClass *terrain)	{ Terrain = terrain; }

private:

	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	TerrainNodeClass	*	Terrain;
};


////////////////////////////////////////////////////////////////////////////
//
//	TerrainNodeClass
//
////////////////////////////////////////////////////////////////////////////
class TerrainNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	TerrainNodeClass (PresetClass *preset = NULL);
	~TerrainNodeClass (void);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	void											On_Post_Load (void);

	//
	//	RTTI
	//
	TerrainNodeClass *As_TerrainNodeClass (void)					{ return this; }

	//
	// From NodeClass
	//
	NodeClass *	Clone (void)						{ return NULL; }
	void			Initialize (void);
	NODE_TYPE	Get_Type (void) const			{ return NODE_TYPE_TERRAIN; }
	int			Get_Icon_Index (void) const	{ return TERRAIN_ICON; }
	void			Add_To_Scene (void);
	void			Remove_From_Scene (void);
	void			Set_Transform (const Matrix3D &tm);
	Matrix3D		Get_Transform (void) { return Transform; }
	PhysClass *	Peek_Physics_Obj (void)	const	{ return NULL; }
	bool			Is_Static (void) const			{ return true; }
	void			Add_Vis_Points (VisPointGeneratorClass &generator, RenderObjClass *render_obj = NULL);
	void			Hide (bool hide);
	bool			Is_Hidden (void) const;
	void			Update_Cached_Vis_IDs (void);
	void			Reload (void);
	void			Pre_Export (void);
	void			Post_Export (void);
	bool			Is_A_Child_Node (NodeClass *node) const;


	// Terrain specific
	int			Get_Sub_Node_Count (void) const	{ return m_Sections.Count (); }
	NodeClass *	Get_Sub_Node (int index)			{ return m_Sections[index]; }

	void			Special_Set_Transform (const Matrix3D &tm);

	//
	//	From PersistClass
	//
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	void			Free_Sections (void);
	void			Assign_Section_IDs (void);
	void			Build_Section_ID_List (void);
	bool			Load_Variables (ChunkLoadClass &cload);
	void			Create_Proxies (DynamicVectorClass<ProxyClass> &proxy_list);
	void			Create_Lights (void);
	void			Free_Section_Data (void);
	PresetClass *Find_Proxy_Preset (const char *preset_name);


	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DynamicVectorClass<NodeClass *>	m_Sections;
	TerrainSectionPersistListClass	m_TerrainSectionInfo;

	friend TerrainSectionPersistClass;
	friend TerrainSectionPersistListClass;

	Matrix3D Transform;
	Matrix3D LoadedTransform;
};


/////////////////////////////////////////////////////////////
//	Get_Transform
/////////////////////////////////////////////////////////////
/*inline Matrix3D
TerrainNodeClass::Get_Transform (void)
{
	return Matrix3D (1);
}*/



#endif //__TERRAIN_NODE_H

