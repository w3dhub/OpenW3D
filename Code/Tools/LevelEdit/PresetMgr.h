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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PresetMgr.h                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/23/01 6:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PRESET_MGR_H
#define __PRESET_MGR_H

#include "saveloadsubsystem.h"
#include "vector.h"
#include "bittype.h"
#include "listtypes.h"
#include "ntree.h"



//////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////
class PresetClass;

//////////////////////////////////////////////////////////////////////////
// Typedefs
//////////////////////////////////////////////////////////////////////////
typedef SortedNTreeClass<PresetClass *>		PRESET_TREE;
typedef SortedNTreeLeafClass<PresetClass *>	PRESET_TREE_LEAF;

typedef SortedNTreeClass<uint32>					FACTORY_TREE;
typedef SortedNTreeLeafClass<uint32>			FACTORY_TREE_LEAF;


//////////////////////////////////////////////////////////////////////////
// Singleton instance
//////////////////////////////////////////////////////////////////////////
extern class PresetMgrClass _ThePresetMgr;

//////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////
const DWORD TEMP_DEF_ID_START			= 1000000000;


//////////////////////////////////////////////////////////////////////////
//
//	PresetMgrClass
//
//////////////////////////////////////////////////////////////////////////
class PresetMgrClass : public SaveLoadSubSystemClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	PresetMgrClass (void);
	virtual ~PresetMgrClass (void);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	// From SaveLoadSubSystemClass
	virtual uint32				Chunk_ID (void) const;
	virtual const char *		Name (void) const			{ return "PresetMgrClass"; }

	// Initialization
	static void					Initialize (void);
	static void					Shutdown (void);

	// List management
	static void					Add_Preset (PresetClass *preset);
	static void					Remove_Preset (PresetClass *preset);
	static void					Free_Presets (void);
	static void					Free_All_Embedded_Nodes (void);

	static bool					Check_Out_Database (uint32 class_id, bool *should_undo_on_err);
	static bool					Check_Out_Database (LPCTSTR path, bool *should_undo_on_err);
	static bool					Check_In_Database (LPCTSTR path);
	static bool					Is_Database_Checked_Out (LPCTSTR full_path);
	static bool					Undo_Database_Check_Out (uint32 class_id);

	// Enumeration
	typedef enum
	{
		ID_CLASS				= 1,
		ID_SUPERCLASS,
	} ID_TYPE;

	static PresetClass *		Find_Typed_Preset (uint32 class_id, LPCTSTR name);
	static PresetClass *		Find_Preset (uint32 id);
	static PresetClass *		Find_Preset (LPCTSTR name);
	static PresetClass *		Get_First (void);
	static PresetClass *		Get_First (uint32 id, ID_TYPE type, bool include_twiddlers = false);
	static PresetClass *		Get_Next (PresetClass *current);
	static PresetClass *		Get_Next (PresetClass *current, uint32 class_id, ID_TYPE type, bool include_twiddlers = false);

	//
	// Tree building methods
	//
	static void					Build_Preset_Tree (uint32 class_id, PRESET_TREE &tree, bool include_twiddlers = true);
	static void					Build_Factory_Tree (uint32 class_id, FACTORY_TREE &tree);

	//
	//	Content control
	//
	static void					Put_Presets_Back (PRESET_LIST &preset_list);
	static void					Remove_Non_Matching_Presets (uint32 class_id, bool class_id_matters, bool is_temp, PRESET_LIST &removed_preset_list);

	//
	//	Preset creation
	//
	static PresetClass *		Create_Preset (uint32 class_id, const char * name, bool is_temp);

	//
	//	Dirty preset methods
	//
	static bool					Are_Presets_Dirty (void)						{ return _PresetsDirty; }
	static void					Set_Presets_Dirty (void)						{ _PresetsDirty = true; }
	static void					Add_Dirty_Preset (int preset_id);
	static void					Check_In_Presets (void);
	static void					Discard_Preset_Changes (void);
	static bool					Get_Immediate_Check_In_Mode (void)			{ return _ImmediateCheckInMode; }
	static void					Set_Immediate_Check_In_Mode (bool onoff)	{ _ImmediateCheckInMode = onoff; }
	static void					Add_Dirty_Preset_Files_To_VSS (void);

	//
	//	Export methods
	//
	static void					Export_Settings (uint32 class_id, const char *filename);

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////

	// From SaveLoadSubSystemClass
	virtual bool				Contains_Data(void) const;
	virtual bool				Save (ChunkSaveClass &csave);
	virtual bool				Load (ChunkLoadClass &cload);

	// Save/load methods
	bool							Save_Presets (ChunkSaveClass &csave);
	bool							Load_Presets (ChunkLoadClass &cload);

	bool							Save_Embedded_Node_Data (ChunkSaveClass &csave);
	bool							Load_Embedded_Node_Data (ChunkLoadClass &csave);

	// Version methods
	static void					Validate_Version (void);

	// Tree building methods
	static void					Add_Children_To_Tree (uint32 parent_id, PRESET_TREE_LEAF *leaf, bool include_twiddlers);

	// Type checking
	static bool					Is_One_Of (uint32 id_to_find, ID_TYPE type, bool include_twiddlers, PresetClass *preset);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	static void					Link_Preset (PresetClass *preset);
	static void					Unlink_Preset (PresetClass *preset);

	/////////////////////////////////////////////////////////////////////
	//	Static member data
	/////////////////////////////////////////////////////////////////////
	static PresetClass *					_PresetListHead;
	static DynamicVectorClass<int>	_DirtyPresetList;
	static bool								_PresetsDirty;
	static bool								_ImmediateCheckInMode;
};


/////////////////////////////////////////////////////////////////////
//	Get_First
/////////////////////////////////////////////////////////////////////
inline PresetClass *
PresetMgrClass::Get_First (void)
{
	return _PresetListHead;
}


//////////////////////////////////////////////////////////////////////////
//
//	PresetListNode
//
//////////////////////////////////////////////////////////////////////////
class PresetListNode
{

public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	PresetListNode (void)
		:	m_Preset (NULL)				{ }

	virtual ~PresetListNode (void)	{ Reset (); }

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	PresetClass *			Get_Preset (void)		{ return m_Preset; }
	PRESET_NODE_LIST &	Get_Children (void)	{ return m_Children; }

	void						Set_Preset (PresetClass *preset) {	m_Preset = preset; }
	void						Add_Child (PresetClass *preset, bool sort = true);
	void						Reset (void);

protected:

	//////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////
	PRESET_NODE_LIST	m_Children;
	PresetClass *		m_Preset;
};



#endif //__PRESET_MGR_H
