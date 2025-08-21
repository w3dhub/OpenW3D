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

/////////////////////////////////////////////////////////////////////////////
//
//  Utils.H
//
//  Module containing usefull misc. utility functions
//

#ifndef __UTILS_H
#define __UTILS_H


#include "refcount.h"
#include "shlobj.h"
#include "listtypes.h"
#include "nodetypes.h"
#include "assetmgr.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"


/////////////////////////////////////////////////////////////////////////////
//
// Macros
//
#define SAFE_DELETE(pobject)					\
			if (pobject) {							\
				delete pobject;					\
				pobject = NULL;					\
			}											\

#define SAFE_DELETE_ARRAY(pobject)			\
			if (pobject) {							\
				delete [] pobject;				\
				pobject = NULL;					\
			}											\

#define SAFE_ADD_REF(pobject)					\
			if (pobject) {							\
				pobject->Add_Ref ();				\
			}											\

#define SAFE_RELEASE_REF(pobject)			\
			if (pobject) {							\
				pobject->Release_Ref ();		\
			}											\
			
#define MEMBER_RELEASE(pmember)				\
			SAFE_RELEASE_REF(pmember);			\
			pmember = NULL;						\


#define MEMBER_ADD(pmember, pnew)			\
			MEMBER_RELEASE (pmember);			\
			pmember = pnew;						\
			SAFE_ADD_REF (pmember);				\


#define COM_RELEASE(pobject)					\
			if (pobject) {							\
				pobject->Release ();				\
			}											\
			pobject = NULL;						\

#define SAFE_CLOSE(handle)								\
			if (handle != INVALID_HANDLE_VALUE) {	\
				::CloseHandle (handle);					\
				handle = INVALID_HANDLE_VALUE;		\
			}													\

#define COPY_CHAR_ARRAY(dest, src)					\
			::lstrcpyn (dest, src, sizeof (dest));	\
			dest[sizeof (dest)-1] = 0;					\


#define SANITY_CHECK(expr)		\
			ASSERT (expr);			\
			if (!(expr))

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))


#define INLINE_ACCESSOR_CONST(_type, _class, _name)	\
inline _type _class::Get_##_name (void) const			\
{	return m_##_name; }											\
inline void _class::Set_##_name (_type val)				\
{	m_##_name = val; }											\


#define INLINE_ACCESSOR(_type, _class, _name)	\
inline _type _class::Get_##_name (void)			\
{	return m_##_name; }									\
inline void _class::Set_##_name (_type val)		\
{	m_##_name = val; }									\


/////////////////////////////////////////////////////////////////////////////
//
// Inlines
//
__inline void Delimit_Path (LPTSTR path)
{
	if (path != NULL && path[0] != 0 && path[::lstrlen (path) - 1] != '\\') {
		::lstrcat (path, "\\");
	}
	return ;
}

__inline void Delimit_Path (CString &path)
{
	if (path.GetLength () > 0 && path[::lstrlen (path) - 1] != '\\') {
		path += CString ("\\");
	}
	return ;
}

__inline bool Is_Path (LPCTSTR string)
{
	bool retval = (::strchr (string, '\\') != NULL);
	retval |= (::strchr (string, '/') != NULL);
	return retval;
}

__inline bool Is_W3D_Filename (LPCTSTR filename)
{
	CString lower_case = filename;
	lower_case.MakeLower ();
	return (::strstr (lower_case, ".w3d") != NULL);
}

__inline bool Is_Texture_Filename (LPCTSTR filename)
{
	CString lower_case = filename;
	lower_case.MakeLower ();
	return (::strstr (lower_case, ".tga") != NULL);
}

__inline bool Is_Full_Path (LPCTSTR string)
{
	return (string[1] == ':') || (string[0] == '\\' && string[1] == '\\');
}

__inline CString Make_Path (LPCTSTR path, LPCTSTR filename)
{
	CString full_path;

	if (Is_Full_Path (filename)) {
		full_path = filename;
	} else {
		full_path = path;
		Delimit_Path (full_path);
		full_path += filename;
	}

	return full_path;
}

__inline void Create_Dir_If_Necessary (LPCTSTR path)
{
	if (::GetFileAttributes (path) == 0xFFFFFFFF) {
		::CreateDirectory (path, NULL);
	}

	return ;
}

__inline RenderObjClass *Create_Render_Obj (LPCTSTR name)
{
	return WW3DAssetManager::Get_Instance()->Create_Render_Obj (name);
}

__inline void Set_Current_Directory (LPCTSTR path)
{
	_pThe3DAssetManager->Set_Current_Directory (path);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
/////////////////////////////////////////////////////////////////////////////
class OverlapPageClass;
class InstancesPageClass;
class GlobalPresetsFormClass;
class CLevelEditDoc;
class CLevelEditView;
class CameraMgr;
class MouseMgrClass;
class FileMgrClass;
class SceneEditorClass;
class NodeMgrClass;
class SelectionMgrClass;
class GroupsPageClass;
class LightAmbientFormClass;
class PresetsFormClass;
class Matrix3D;
class AABoxClass;
class OBBoxClass;
class PersistClass;
class DefinitionClass;
class ConversationPageClass;


/////////////////////////////////////////////////////////////////////////////
//
// Prototypes
//
CLevelEditDoc *				Get_Current_Document (void);
CLevelEditView *				Get_Main_View (void);
CameraMgr *						Get_Camera_Mgr (void);
MouseMgrClass *				Get_Mouse_Mgr (void);
FileMgrClass *					Get_File_Mgr (void);
SceneEditorClass *			Get_Scene_Editor (void);
NodeMgrClass &					Get_Node_Mgr (void);
SelectionMgrClass &			Get_Selection_Mgr (void);
void								Refresh_Main_View (void);
void								General_Pump_Messages (void);
void								Pump_Messages (void);
GROUP_LIST &					Get_Global_Group_List (void);
CImageList *					Get_Global_Image_List (void);

//
//	Form/Property page management
//
ConversationPageClass *		Get_Conversation_Form (void);
OverlapPageClass *			Get_Overlap_Form (void);
InstancesPageClass *			Get_Instances_Form (void);
LightAmbientFormClass *		Get_Ambient_Light_Form (void);
PresetsFormClass *			Get_Presets_Form (void);

//
//	Misc routines
//
int								Get_Next_Temp_ID (void);
LPCTSTR							Get_Factory_Name (uint32 class_id);

//
//	'Safe' creation routines (provides error message, etc)
//
PersistClass *					Instance_Definition (DefinitionClass *definition);

//
//	Painting/subclassing/dialog control routines
//
LRESULT CALLBACK				CheckBoxSubclassProc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
void								Paint_Gradient (HWND hwnd, BYTE base_red, BYTE base_green, BYTE base_blue);
void								Constrain_Point_To_Aspect_Ratio (float &xpos, float &ypos);
void								Fill_Node_Instance_Combo (HWND hcombobox, NodeClass *pdefault = NULL);
void								Fill_Group_Combo (HWND hcombobox, GroupMgrClass *pdefault = NULL);
int								Type_To_Icon (NODE_TYPE type, bool btemp = false);

//
//	Vis routines
//
void								Perform_Job (LPCTSTR filename, bool delete_on_completion);

//
//	Directory routines
//
CString							Get_Startup_Directory (void);
CString							Get_Data_Directory (void);

//
//	String manipulation routines
//
CString							Get_Filename_From_Path (LPCTSTR path);
CString							Strip_Filename_From_Path (LPCTSTR path);
CString							Get_Subdir_From_Full_Path (LPCTSTR path);
CString							Get_Subdir_And_Filename_From_Path (LPCTSTR path);
int								Build_List_From_String (LPCTSTR buffer, LPCTSTR delimiter, CString **pstring_list);
CString							Build_String_From_List (const CString *pstring_list, int count, LPCTSTR delimiter);
CString							Asset_Name_From_Filename (LPCTSTR filename);
CString							Filename_From_Asset_Name (LPCTSTR asset_name);
CString							Up_One_Directory (LPCTSTR path);
void								Convert_Chars_To_Newline (CString &string);
void								Convert_Newline_To_Chars (CString &string);

//
//	Message box/status methods
//
UINT								Message_Box (HWND hparentwnd, UINT message_id, UINT title_id, UINT style = MB_ICONEXCLAMATION | MB_OK);
void								Update_Frame_Count (int frame, int max_frames);
void								Set_Modified (bool modified = true);
bool								Is_Silent_Mode (void);
void								Set_Silent_Mode (bool is_silent);

//
// Dialog routines
//
void								SetDlgItemFloat (HWND hdlg, UINT child_id, float value);
float								GetDlgItemFloat (HWND hdlg, UINT child_id, bool interpret = false);
float								GetWindowFloat (HWND hwnd, bool interpret = false);
void								SetWindowFloat (HWND hwnd, float value);
void								Make_Edit_Float_Ctrl (HWND edit_wnd);
void								Make_Edit_Int_Ctrl (HWND edit_wnd);
HWND								Show_VSS_Update_Dialog (HWND hparent_wnd);
void								Kill_VSS_Update_Dialog (HWND hdlg);

//
//	File routines
//
DWORD								Get_File_Size (LPCTSTR path);
bool								Get_File_Time (LPCTSTR path, LPFILETIME pcreation_time = NULL, LPFILETIME paccess_time = NULL, LPFILETIME pwrite_time = NULL);
bool								Is_Path_Relative (LPCTSTR path);
bool								Find_File (LPCTSTR filename, LPCTSTR start_dir, CString &full_path);
int								Quick_Compare_Files (LPCTSTR file1, LPCTSTR file2);
int								Get_LOD_File_Count (LPCTSTR first_lod_filename, CString *pbase_filename = NULL);
bool								Browse_For_Folder (CString &folder, HWND hparentwnd, LPCTSTR default_path, LPCTSTR title, UINT flags = BIF_RETURNONLYFSDIRS);
bool								Copy_File (LPCTSTR existing_filename, LPCTSTR new_filename, bool bforce_copy = false);

//
//	Version routines
//
bool								Check_Editor_Version (void);

//
//	Message/Debug routines
//
void								Output_Message (LPCTSTR message);

//
//	Math routines
//
void								Rotate_Matrix (const Matrix3D &input_mat, const Matrix3D &rotation_mat, const Matrix3D &coord_system, Matrix3D *poutput_mat);
bool								Get_Collision_Box (const RenderObjClass *model, AABoxClass &box);
bool								Get_Collision_Box (RenderObjClass *render_obj, AABoxClass &aabox, OBBoxClass &obbox);

//
//	Thread routines
//
typedef UINT (*MY_THREADPROC) (DWORD dwparam1, DWORD dwparam2, DWORD dwparam3, HRESULT *presult);
typedef UINT (*MY_UITHREADPROC) (DWORD_PTR dwparam1, DWORD_PTR dwparam2, DWORD_PTR dwparam3, HRESULT *presult, HWND *phmain_wnd);
void								Create_Worker_Thread (MY_THREADPROC fnthread_proc, DWORD dwparam1, DWORD dwparam2, DWORD dwparam3, HRESULT *presult);
void								Create_UI_Thread (MY_UITHREADPROC fnthread_proc, DWORD_PTR dwparam1, DWORD_PTR dwparam2, DWORD_PTR dwparam3, HRESULT *presult, HWND *phmain_wnd);


/////////////////////////////////////////////////////////////////////////////
//
// Helper classes
//
class FileAccessRightsClass
{
	public:
		typedef enum
		{
			WANTS_READONLY		= 1,
			WANTS_WRITEABLE
		} ACCESS_TYPE;

		FileAccessRightsClass (LPCTSTR filename, ACCESS_TYPE type = WANTS_WRITEABLE, bool should_restore = false);
		~FileAccessRightsClass (void);

	private:
		bool			m_bNeedsRestoring;
		LONG			m_FileAttrs;
		CString		m_Filename;
};


#endif //__UTILS_H
