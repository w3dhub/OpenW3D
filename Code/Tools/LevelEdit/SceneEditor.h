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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/SceneEditor.h                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/08/01 10:48a                                              $*
 *                                                                                             *
 *                    $Revision:: 58                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SCENEEDITOR_H
#define __SCENEEDITOR_H


#include "pscene.h"
#include "listtypes.h"
#include "undomgr.h"
#include "vector.h"
#include "vislog.h"
#include "viswindowdialog.h"


//////////////////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class Vector3;
class Matrix3D;
class NodeBaseClass;
class NodeMgrClass;
class SelectionMgrClass;
class AudibleSoundClass;
class DecorationPhysClass;
class Phys3Class;
class PresetClass;
class SkyClass;
class LightNodeClass;


//////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SceneEditorClass
//////////////////////////////////////////////////////////////////////////////////////////////////
class SceneEditorClass : public PhysicsSceneClass
{
	public:

		//////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////
		SceneEditorClass (void);
		virtual ~SceneEditorClass (void);

		//////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////

		//
		//	Object creation/destruction
		//
		virtual void						Add_Node (NodeClass *node);
		virtual NodeClass *				Create_Node (PresetClass *preset, Matrix3D *transform = NULL, DWORD obj_id = 0, bool add_to_scene = true);
		virtual NodeClass *				Clone_Node (NodeClass *node);
		virtual bool						Delete_Node (NodeClass *node, bool allow_undo = true);
		virtual void						Delete_Nodes (void);
		virtual bool						Delete_Nodes (PresetClass *preset);
		virtual void						Update_File_Mgr (NodeClass *node, bool badd = true);

		//
		//	Cut/copy/paste/undo methods
		//
		virtual bool						Cut_Objects (void);
		virtual bool						Cut_Object (NodeClass * /* pnode */) { return false; }
		virtual bool						Copy_Objects (void);
		virtual bool						Copy_Object (NodeClass * /* pnode */) { return false; }
		virtual bool						Paste_Objects (void);
		virtual bool						Paste_Object (NodeClass * /* pnode */) { return false; }
		virtual bool						Can_Paste (void);
		virtual bool						Clone_Objects (void);
		virtual bool						Clone_Object (NodeClass * /* pnode */) { return false; }
		virtual bool						Undo (void);
		virtual void						Begin_Operation (OPERATION_TYPE type, NodeClass *node);
		virtual void						Begin_Operation (OPERATION_TYPE type, NODE_LIST *affected_list = NULL);
		virtual void						End_Operation (void);		

		//
		//	Object transformation methods
		//
		virtual void						Lock_Nodes (bool lock);
		virtual Vector3					Build_Node_List (NODE_LIST &node_list);		

		//
		//	Inline accessors
		//
		virtual SelectionMgrClass &	Get_Selection_Mgr (void) { return *m_SelectionMgr; }
		virtual UndoMgrClass &			Get_Undo_Mgr (void) { return m_UndoMgr; }
		virtual GROUP_LIST &				Get_Group_List (void) { return m_GroupsList; }		

		//
		//	Group methods
		//
		virtual GroupMgrClass *			Add_Global_Group (const CString &name, NODE_LIST *initial_list = NULL);
		virtual void						Remove_Global_Group (GroupMgrClass *pgroup);
		virtual void						Add_Group_To_Toolbar (GroupMgrClass *pgroup);
		virtual void						Remove_Group_From_Toolbar (GroupMgrClass *pgroup);
		virtual void						Reset_Global_Groups_List (void);

		//
		//	Initialization methods
		//
		virtual void						Initialize (void);
		virtual void						Cleanup_Resources (void);

		//
		//	Selection methods
		//
		void									Set_Selection (NodeClass *node);
		void									Toggle_Selection (NodeClass *node);
		void									Update_Selection_Boxes (void);
		void									Replace_Selection (void);

		//
		//	Hit test methods
		//
		bool									Execute_Function_At_Point (CPoint point);
		NodeClass *							Find_Node_At_Point (CPoint point, Vector3 *intersect_pt = NULL);
		void									Select_Node_At_Point (CPoint point)					{ Set_Selection (Find_Node_At_Point (point)); }
		void									Toggle_Node_Selection_At_Point (CPoint point)	{ Toggle_Selection (Find_Node_At_Point (point)); }

		//
		//	Vis generation methods
		//
		void									Record_Vis_Info (const Matrix3D & view_transform, const Vector3 &store_loc);
		void									Display_Vis_Points (bool onoff);
		bool									Are_Vis_Points_Displayed (void) const	{ return m_bVisPointsDisplayed; }
		void									Create_Vis_Point (const Matrix3D &transform);
		void									Remove_All_Vis_Points (void);
		virtual void						Reset_Vis (bool doitnow = false);
		virtual void						Internal_Vis_Reset (void);
		void									Get_Vis_Camera_FOV (double &hov, double &vfov);
		void									Get_Vis_Camera_Clip_Planes (float &znear, float &zfar);
		VisLogClass &						Get_Vis_Log (void) { return m_VisLog; }
		void									Set_Total_Vis_Points (int total)		{ m_TotalVisPoints = total; }
		int									Get_Total_Vis_Points (void) const	{ return m_TotalVisPoints; }
		CameraClass *						Get_Render_Camera (void);
		void									Export_VIS (LPCTSTR filename);
		void									Discard_Vis (void);
		void									Reset_Dynamic_Object_Visibility_Status (void);
		void									Reset_Vis_For_Node (NodeClass *node);

		void									Generate_Uniform_Sampled_Vis (float granularity, float sample_height, bool ignore_bias, bool selection_only, bool farm_mode = false,int farm_cpu_index = 1,int farm_cpu_total = 1);
		void									Generate_Edge_Sampled_Vis (float granularity, bool ignore_bias,bool farm_mode = false,int farm_cpu_index = 1,int farm_cpu_total = 1);
		void									Generate_Light_Vis(bool farm_mode = false, int farm_cpu_index = 1, int farm_cpu_total = 1);
		void									Generate_Manual_Vis(bool farm_mode = false, int farm_cpu_index = 1, int farm_cpu_total = 1);

		bool									Are_Manual_Vis_Points_Visible (void);
		void									Show_Manual_Vis_Points (bool show_points);

		bool									Is_Vis_Window_Visible (void);
		void									Show_Vis_Window (bool onoff);

		//
		//	Update methods
		//
		void									On_Frame (void);

		//
		//	Sphere methods
		//
		bool									Are_Sound_Spheres_Displayed (void) const { return m_DisplaySoundSpheres; }
		void									Display_Sound_Spheres (bool onoff);

		bool									Are_Light_Spheres_Displayed (void) const { return m_DisplayLightSpheres; }
		void									Display_Light_Spheres (bool onoff);


		//
		//	Light methods
		//
		bool									Are_Lights_On (void) const { return m_bLightsOn; }
		void									Turn_Lights_On (bool onoff);
		void									Reload_Lightmap_Models (void);
		void									Export_Lights (LPCTSTR filename);
		void									Import_Lights (DynamicVectorClass<StringClass> &filename_list, DynamicVectorClass<LightNodeClass *> *node_list = NULL);
		void									Import_Sunlight (LPCTSTR filename);
		void									Build_Light_List (ChunkLoadClass &cload, DynamicVectorClass<LightClass *> &light_list, uintptr_t group_id = 0);
		void									Filter_Lights (DynamicVectorClass<LightClass *> &light_list);
		bool									Compare_Lights (LightClass *light1, LightClass *light2);
		void									Update_Lighting (void);
		void									Set_Sun_Light_Orientation (float yaw, float pitch);

		//
		//	Background sound methods
		//
		void									Set_Background_Music (LPCTSTR filename);
		LPCTSTR								Get_Background_Music_Filename (void) const { return m_BackgroundSoundFilename; }

		//
		//	Pathfinding methods
		//
		void									Generate_Pathfind_Portals (void);
		void									Pathfind_Floodfill (Phys3Class &char_sim, const Vector3 &start_pos);
		void									DoObjectGoto (NodeClass *node1, NodeClass *node2);
		
		//
		//	Aggregate methods
		//
		void									View_Aggregate_Children (bool view = true);
		bool									Are_Aggregate_Children_Visible (void) const	{ return m_bAggregateChildrenVisible; }

		//
		//	Culling system accessors
		//
		void									Re_Partition_Dynamic_Objects (DynamicVectorClass<AABoxClass> & virtual_occludees) { Re_Partition_Dynamic_Culling_System (virtual_occludees); }
		void									Re_Partition_Audio_System (void);

		//
		//	Miscellaneous
		//
		bool									Are_Static_Anim_Phys_Displayed (void) const	{ return m_ShowStaticAnimPhys; }
		void									Display_Static_Anim_Phys (bool onoff);

		bool									Are_Editor_Objects_Displayed (void) const		{ return m_ShowEditorObjects; }
		void									Display_Editor_Objects (bool onoff);

		void									Enable_Proxy_Creation (bool onoff)				{ m_CreateProxies = onoff; }
		bool									Is_Proxy_Creation_Enabled (void) const			{ return m_CreateProxies; }

		//
		//	Building methods
		//
		void									Enable_Building_Power (bool onoff);
		bool									Is_Building_Power_Enabled (void) const	{ return m_BuildingPowerEnabled; }

	protected:

		//////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////
		virtual void						Add_Groups_To_List (NodeClass &node, GROUP_LIST &group_list);
		virtual void						Add_Nodes_To_List (NodeClass &node, NODE_LIST &node_list);
		virtual void						Build_Group_List (GROUP_LIST &group_list, NodeClass *node = NULL);
		virtual void						Build_Node_List (NODE_LIST &node_list, NodeClass *node);		
		virtual void						Update_Toolbars (void);
		virtual void						Empty_Local_Clipboard (void);		

		void									Move_Selected_Nodes (void);

		//
		//	From base class
		//
		virtual void						On_Vis_Occluders_Rendered(VisRenderContextClass & context,VisSampleClass & sample);
		
	private:

		//////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////
		//SkyClass *							m_Sky;
		SelectionMgrClass *				m_SelectionMgr;
		UndoMgrClass						m_UndoMgr;
		UINT									m_uiClipboardFormat;
		GROUP_LIST							m_GroupsList;
		DynamicVectorClass<NodeClass *>	m_LocalClipboard;
		bool									m_bLightsOn;
		bool									m_bAggregateChildrenVisible;
		bool									m_DisplaySoundSpheres;
		bool									m_DisplayLightSpheres;
		CString								m_BackgroundSoundFilename;
		CString								m_SkyTexture;
		bool									m_bVisPointsDisplayed;
		DynamicVectorClass<DecorationPhysClass *>	m_VisPoints;
		VisLogClass							m_VisLog;
		int									m_TotalVisPoints;
		bool									m_ManualVisPointsVisible;
		bool									m_ShowStaticAnimPhys;
		bool									m_ShowEditorObjects;
		bool									m_BuildingPowerEnabled;
		bool									m_CreateProxies;
		VisWindowDialogClass				m_VisWindow;

		NodeClass *							m_MovingObject;		
};


#endif //__SCENEEDITOR_H
