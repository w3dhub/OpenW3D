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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Preset.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/26/01 9:53a                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PRESET_H
#define __PRESET_H

#include "refcount.h"
#include "persist.h"
#include "vector.h"
#include "listtypes.h"
#include "definition.h"
#include "Utils.h"
#include "transition.h"


///////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////
class DefinitionClass;
class NodeClass;
class ModelDefParameterClass;


///////////////////////////////////////////////////////////////////////
//
//	PresetClass
//
///////////////////////////////////////////////////////////////////////
class PresetClass : public PersistClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	PresetClass (void);
	virtual ~PresetClass (void);

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////

	//
	// From PersistClass
	//
	const PersistFactoryClass &	Get_Factory (void) const;
	virtual bool			Save (ChunkSaveClass &csave);
	virtual bool			Load (ChunkLoadClass &cload);
	virtual void			On_Post_Load (void);

	virtual bool			Show_Properties (bool read_only = false);
	virtual bool			Delete (void) { return true; }
	virtual void			Copy_Properties (const PresetClass &preset);

	virtual NodeClass *	Create (void);

	//
	//	Asset management
	//
	void						Load_All_Assets (void);
	void						Add_Files_To_VSS (void);

	//
	// Dependency management
	//
	void						Get_All_Dependencies (STRING_LIST &list) const;
	void						Get_Implicit_Dependencies (STRING_LIST &list) const;
	void						Get_Manual_Dependencies (STRING_LIST &list) const;	
	void						Set_Manual_Dependencies (STRING_LIST &list);
	
	//
	// Inline accessors
	//
	DefinitionClass *		Get_Definition (void) const							{ return m_Definition; }
	void						Set_Definition (DefinitionClass *definition);
	LPCTSTR					Get_Comments (void) const;
	void						Set_Comments (LPCTSTR comments);
	bool						Get_IsTemporary (void) const;
	void						Set_IsTemporary (bool is_temp);
	PresetClass *			Get_Parent (void) const						{ return m_Parent; }
	void						Set_Parent (PresetClass *parent);
	bool						Is_A_Parent (PresetClass *preset);
	bool						Is_A_Parent (LPCTSTR parent_name);	

	//
	//	Child access
	//
	int						Get_Child_Preset_Count (void)		{ return m_ChildIDList.Count (); }
	int						Get_Child_Preset_ID (int index)	{ return m_ChildIDList[index]; }
	PresetClass *			Get_Child_Preset (int index);
	void						Add_Child_Preset (int child_id);
	void						Remove_Child_Preset (int child_id);

	//
	//	Definition access
	//
	void						Collect_Definitions (DEFINITION_LIST &list);
	static void				Add_Definition_Dependencies (DefinitionClass *definition, STRING_LIST &list);

	//
	//	Display methods
	//
	int						Get_Icon_Index (void) const;
	
	//
	// Pass-thrus to the definition object
	//
	uint32					Get_ID (void) const;
	void						Set_ID (uint32 id);
	LPCTSTR					Get_Name (void) const;
	void						Set_Name (LPCTSTR name);
	uint32					Get_Class_ID (void) const;

	//
	// Misc helper methods
	//
	bool						Is_Valid_Sound_Preset (void);
	bool						Is_Soldier_Preset (void);

	//
	// Validity checks
	//
	bool						Is_Valid (void) const;

	//
	//	Node list access
	//
	void						Build_Node_List (NodeClass *parent_node);
	void						Create_Linked_Nodes (NodeClass *parent_node);
	NODE_LIST &				Get_Node_List (void)		{ return m_NodeList; }
	void						Free_Node_List (void);	

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	virtual bool			Save_Variables (ChunkSaveClass &csave);
	virtual bool			Load_Variables (ChunkLoadClass &cload);

	TRANSITION_DATA_LIST *			Get_Transition_List (void) const;
	void									Build_Zone_List (DefinitionClass *definition, ZONE_PARAM_LIST &zone_list) const;
	ModelDefParameterClass *		Get_Phys_Def_Param (void);
	ScriptListParameterClass *		Get_Script_List_Param (void);	

	////////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////////
	PresetClass *			m_NextPreset;
	PresetClass *			m_PrevPreset;

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	PresetClass *			m_Parent;
	DefinitionClass *		m_Definition;
	STRING_LIST				m_ManualDependencies;
	StringClass				m_Comments;
	bool						m_IsTemporary;
	bool						m_IsValid;
	uint32					m_ParentID;
	mutable uint32			m_DefinitionID;
	NODE_LIST				m_NodeList;

	DynamicVectorClass<int>	m_ChildIDList;

	////////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////////
	friend class PresetMgrClass;
};

//INLINE_ACCESSOR_CONST (DefinitionClass *, PresetClass, Definition);
INLINE_ACCESSOR_CONST (LPCTSTR, PresetClass, Comments);
INLINE_ACCESSOR_CONST (bool, PresetClass, IsTemporary);
//INLINE_ACCESSOR_CONST (PresetClass *, PresetClass, Parent);

////////////////////////////////////////////////////////////////////
//	Get_ID
////////////////////////////////////////////////////////////////////
inline uint32
PresetClass::Get_ID (void) const
{
	if (m_Definition != NULL) {
		m_DefinitionID = m_Definition->Get_ID ();
	}

	return m_DefinitionID;
}

////////////////////////////////////////////////////////////////////
//	Set_ID
////////////////////////////////////////////////////////////////////
inline void
PresetClass::Set_ID (uint32 id)
{
	if (m_Definition != NULL) {
		m_Definition->Set_ID (id);		
	}

	m_DefinitionID = id;
	return ;
}

////////////////////////////////////////////////////////////////////
//	Get_Name
////////////////////////////////////////////////////////////////////
inline LPCTSTR
PresetClass::Get_Name (void) const
{
	LPCTSTR name = NULL;
	if (m_Definition != NULL) {
		name = m_Definition->Get_Name ();
	}

	return name;
}

////////////////////////////////////////////////////////////////////
//	Set_Name
////////////////////////////////////////////////////////////////////
inline void
PresetClass::Set_Name (LPCTSTR name)
{
	if (m_Definition != NULL) {
		m_Definition->Set_Name (name);
	}

	return ;
}

////////////////////////////////////////////////////////////////////
//	Is_Valid
////////////////////////////////////////////////////////////////////
inline bool
PresetClass::Is_Valid (void) const
{
	return m_IsValid;
}

////////////////////////////////////////////////////////////////////
//	Get_Manual_Dependencies
////////////////////////////////////////////////////////////////////
inline void
PresetClass::Get_Manual_Dependencies (STRING_LIST &list) const
{
	list = m_ManualDependencies;
	return ;
}

////////////////////////////////////////////////////////////////////
//	Set_Manual_Dependencies
////////////////////////////////////////////////////////////////////
inline void
PresetClass::Set_Manual_Dependencies (STRING_LIST &list)
{
	m_ManualDependencies = list;
	return ;
}

////////////////////////////////////////////////////////////////////
//	Get_Class_ID
////////////////////////////////////////////////////////////////////
inline uint32
PresetClass::Get_Class_ID (void) const
{
	uint32 class_id = 0;
	if (m_Definition != NULL) {
		class_id = m_Definition->Get_Class_ID ();
	}

	return class_id;
}


#endif //__PRESET_H
