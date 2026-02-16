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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditorAssetMgr.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/29/02 3:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 29                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"

#include "editorassetmgr.h"
#include "texture.h"
#include "proto.h"
#include "editorini.h"
#include "ffactory.h"
#include "ww3d.h"
#include "Utils.h"
#include "filemgr.h"
#include "pathutil.h"
#include "rawfile.h"
#include "rcfile.h"
#include "filelocations.h"
#include "editorbuild.h"
#include "assetdatabase.h"


////////////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////////////
DynamicVectorClass<StringClass>	EditorFileFactoryClass::SearchPathList;


////////////////////////////////////////////////////////////////////////
//
//	EditorAssetMgrClass
//
////////////////////////////////////////////////////////////////////////
EditorAssetMgrClass::EditorAssetMgrClass (void)
{
	Set_Activate_Fog_On_Load (true);
	m_CurrentDir = ::Get_Startup_Directory ();
	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Reload_File
//
void
EditorAssetMgrClass::Reload_File (const char *filename)
{
	// Param OK?
	ASSERT (filename != NULL);
	if (filename != NULL) {		
		CString extension = ::strrchr (filename, '.');

		// Is this a W3D file?
		if (extension.CompareNoCase (".W3D") == 0) {
			CString asset_name = ::Asset_Name_From_Filename (filename);
			Remove_Prototype (asset_name);
			Load_3D_Assets (filename);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Is_File_Here
//
////////////////////////////////////////////////////////////////////////
bool
EditorAssetMgrClass::Is_File_Here (LPCTSTR full_path, bool get_from_vss)
{
	//
	//	Output this file lookup to the message window
	//
	CString message;
	message.Format ("Attempting to load: %s\r\n", full_path);
	::Output_Message (message);

	//
	//	Check to see if the file exists in this location
	//
	return ::Get_File_Mgr ()->Does_File_Exist (full_path, get_from_vss);
}


////////////////////////////////////////////////////////////////////////
//
//	Determine_Real_Location
//
////////////////////////////////////////////////////////////////////////
bool
EditorAssetMgrClass::Determine_Real_Location
(
	LPCTSTR		path,
	CString &	real_location
)
{
	bool retval = false;
	TCHAR full_path[MAX_PATH];

	//
	// Did the caller supply a path?
	//
	if (Is_Full_Path (path)) {
		::lstrcpy (full_path, path);
	} else {		

		//
		//	Make a full path out of the filename
		//
		CString test_path = Make_Path (m_CurrentDir, path);

		//
		//	If the file doesn't exist here, then try the search paths
		//
		if (!::cPathUtil::PathExists (test_path)) {
		
			//
			//	Try to find the file in our search path list
			//
			DynamicVectorClass<StringClass> &path_list = EditorFileFactoryClass::Get_Search_Path ();
			for (int index = 0; index < path_list.Count (); index ++) {
				
				//
				//	Does the file exist in this directoy?
				//
				StringClass full_path = static_cast<const char *>(Make_Path (path_list[index], path));
				if (cPathUtil::PathExists (full_path)) {
					test_path = full_path;
					break;
				}
			}
		}

		::lstrcpy (full_path, test_path);
	}

	//
	// Find out if the file lives in the directory
	//
	if (Is_File_Here (full_path, true) == false) {			
		
		//
		// Hmmm... not here, try up a directory level
		//
		CString parent_dir	= ::Up_One_Directory (::Strip_Filename_From_Path (full_path));
		CString filename		= ::Get_Filename_From_Path (full_path);
		parent_dir				= ::Make_Path (parent_dir, filename);

		//
		//	Does the file live in the parent directory?
		//
		if (Is_File_Here (parent_dir, true) == false) {
			
			//
			//	Is this a texture file?
			//
			if (Is_Texture_Filename (full_path)) {

#ifndef PUBLIC_EDITOR_VER

				//
				//	Does the file live in the global texture directory?
				//
				CString texture_path = ::Make_Path (GLOBAL_TEXTURE_PATH, filename);
				if (Is_File_Here (texture_path, false)) {
					real_location = texture_path;
					retval = true;					
				}

#endif //!PUBLIC_EDITOR_VER

			} else if (::strnicmp ("h_a_", filename, 4) == 0) {
				
				//
				//	We are 'assuming' this is a human animation file, so
				// try to find it in the human animation directory.
				//
				CString anim_path  = ::Make_Path (CHAR_ANIMS_PATH, filename);
				if (Is_File_Here (anim_path, true)) {
					real_location = anim_path;
					retval = true;					
				}
			}

		} else {
			real_location = parent_dir;
			retval = true;
		}

	} else {
		real_location = full_path;
		retval = true;
	}

#ifdef PUBLIC_EDITOR_VER
	
	//
	//	If the file does not exist locally, then use the one from the mix file
	//
	if (retval == false && ::Get_File_Mgr ()->Is_File_In_VSS (full_path)) {
		real_location = full_path;
		retval = true;
	}

#endif //PUBLIC_EDITOR_VER

	return retval;
}


////////////////////////////////////////////////////////////////////////
//
//	Create_Render_Obj
//
RenderObjClass *
EditorAssetMgrClass::Create_Render_Obj (const char * name)
{
	// Let windows do some processing
	::Pump_Messages ();

	// Assume failure
	RenderObjClass *prenderobj = NULL;

	// Try to find a prototype
	PrototypeClass * proto = Find_Prototype(name);

	if (WW3D_Load_On_Demand && proto == NULL) {	// If we didn't find one, try to load on demand
		char filename[ MAX_PATH ];
		sprintf( filename, "%s.W3D", name);
		const char *mesh_name = ::strchr (name, '.');
		if (mesh_name != NULL) {
			::lstrcpyn (filename, name, mesh_name - name + 1);
			::lstrcat (filename, ".W3D");
		} else {
			sprintf( filename, "%s.W3D", name);
		}

		// See if we can determine where this file exists
		CString real_location;
		if (Determine_Real_Location (filename, real_location)) {
			::lstrcpy (filename, real_location);
		} else {
			TRACE ("Asset file not found!\n");
		}

		// Load the assets from this file into the manager
		Load_3D_Assets (filename);

		// Attempt to find the prototype loader now.
		proto = Find_Prototype (name);
	}

	// If we found a prototype for this object, then
	// use it to create a new instance of the requested asset.
	if (proto != NULL) {
		prenderobj = proto->Create ();
	}

	// Return a pointer to the render object
	return prenderobj;
}


////////////////////////////////////////////////////////////////////////
//
//	Get_HAnim
//
HAnimClass *
EditorAssetMgrClass::Get_HAnim (const char * name)
{
	// Let windows do some processing
	::Pump_Messages ();

	// Try to find the animation
	HAnimClass *panim = HAnimManager.Get_Anim(name);

	if (WW3D_Load_On_Demand && panim == NULL) {	// If we didn't find it, try to load on demand
		
		char filename[ MAX_PATH ];
		const char *animname = strchr( name, '.');
		if (animname != NULL) {
			sprintf( filename, "%s.W3D", animname+1);
		} else {
			TRACE ("Animation has no . in the name\n");
			return NULL;
		}

		// See if we can determine where this file exists
		CString real_location;
		if (Determine_Real_Location (filename, real_location)) {
			::lstrcpy (filename, real_location);
		} else {
			TRACE ("Asset file not found!\n");
		}

		// Load the assets from this file into the manager
		Load_3D_Assets (filename);

		// Attempt to find the prototype loader now.
		panim = HAnimManager.Get_Anim(name);
	}

	// Return a pointer to the animation
	return panim;
}


////////////////////////////////////////////////////////////////////////
//
//	Get_HTree
//
HTreeClass *
EditorAssetMgrClass::Get_HTree (const char * name)
{
	// Let windows do some processing
	::Pump_Messages ();

	// Try to find the htree
	HTreeClass *phtree = HTreeManager.Get_Tree(name);

	if (WW3D_Load_On_Demand && phtree == NULL) {	// If we didn't find it, try to load on demand
		
		char filename[ MAX_PATH ];
		sprintf( filename, "%s.W3D", name);

		// See if we can determine where this file exists
		CString real_location;
		if (Determine_Real_Location (filename, real_location)) {
			::lstrcpy (filename, real_location);
		} else {
			TRACE ("Asset file not found!\n");
		}

		// Load the assets from this file into the manager
		Load_3D_Assets (filename);

		// Attempt to find the object loader now.
		phtree = HTreeManager.Get_Tree(name);
	}

	// Return a pointer to the animation
	return phtree;
}


////////////////////////////////////////////////////////////////////////
//
//	Get_INI
//
EditorINIClass *
EditorAssetMgrClass::Get_INI (const char *filename)
{
	EditorINIClass *pini = NULL;

	FileClass * pini_file = _TheFileFactory->Get_File (filename);
	if (pini_file) {
		pini = new EditorINIClass (*pini_file);
		_TheFileFactory->Return_File (pini_file);
	}

	// Return a pointer to the INI file class
	return pini;
}


////////////////////////////////////////////////////////////////////////
//
//	Get_Texture
//
////////////////////////////////////////////////////////////////////////
TextureClass *
EditorAssetMgrClass::Get_Texture
(
	const char *					tga_filename,
	TextureClass::MipCountType	mip_level_count,
	WW3DFormat						texture_format,
	bool								allow_compression
)
{
	//
	// Let windows do some processing
	//
	::Pump_Messages ();

	/*
	** Bail if the user isn't really asking for anything
	*/
	if ((tga_filename == NULL) || (strlen(tga_filename) == 0)) {
		return NULL;
	}

	//
	//	Determine what the texture's "name" should be
	//
	CString texture_name;
	if (::strstr (tga_filename, "+\\") != NULL) {
		texture_name = ::Get_Subdir_And_Filename_From_Path (tga_filename);
	} else {
		texture_name = ::Get_Filename_From_Path (tga_filename);
	}
	texture_name.MakeLower ();

	/*
	** See if the texture has already been loaded.
	*/
	TextureClass* tex = TextureHash.Get ((const char *)texture_name);
	if (tex && texture_format!=WW3D_FORMAT_UNKNOWN) {
		WWASSERT_PRINT(tex->Get_Texture_Format()==texture_format,("Texture %s has already been loaded witt different format",(const char *)texture_name));
	}

	/*
	** Didn't have it so we have to create a new texture
	*/
	if (!tex) {

		//
		// See if we can determine where this file exists
		//
		CString real_location;
		if (Determine_Real_Location (tga_filename, real_location) == false) {
			CString message;
			message.Format ("Texture file not found: %s\r\n", (LPCTSTR)texture_name);
			::Output_Message (message);
			real_location = tga_filename;
		}

		/*
		** Didn't have it so we have to create a new texture
		*/
		tex = NEW_REF(TextureClass,(texture_name, real_location, mip_level_count, texture_format, allow_compression));
		TextureHash.Insert(tex->Get_Texture_Name(),tex);
	}

	tex->Add_Ref();
	return tex;
}


////////////////////////////////////////////////////////////////////////
//
//	Load_Resource_Texture
//
////////////////////////////////////////////////////////////////////////
void
EditorAssetMgrClass::Load_Resource_Texture (const char *filename)
{
	CString texture_path			= ::Get_File_Mgr ()->Get_Texture_Cache_Path ();
	CString texture_filename	= ::Make_Path (texture_path, filename);

	//
	//	Open the resource texture and create the
	// real file.
	//
	ResourceFileClass res_texture (NULL, filename);
	RawFileClass texture_file (texture_filename);
	if (texture_file.Create () == 1) {
		texture_file.Open (FileClass::WRITE);

		//
		//	Copy the contents of the resource (memory) texture
		// to a file.
		//
		BYTE buffer[2048];
		int size				= res_texture.Size ();
		int total_written	= 0;
		while (total_written < size) {
			int bytes_read		= res_texture.Read (buffer, sizeof (buffer));
			int bytes_written = texture_file.Write (buffer, bytes_read);
			total_written		+= bytes_written;
			if (bytes_written == 0) {
				break;
			}
		}
		texture_file.Close ();

		//
		//	Force the texture to be loaded into memory
		//
		TextureClass *texture = Get_Texture (texture_filename);
		REF_PTR_RELEASE (texture);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Load_3D_Assets
//
////////////////////////////////////////////////////////////////////////
bool
EditorAssetMgrClass::Load_3D_Assets (const char *path)
{	
	bool retval = true;	
	
	//
	//	Check to see if the asset is already in the asset manager
	//
	CString filename				= ::Get_Filename_From_Path (path); 
	CString asset_name			= ::Asset_Name_From_Filename (filename);
	PrototypeClass *prototype	= Find_Prototype (asset_name);

	//
	//	Load the asset fresh if necessary
	//
	if (prototype == NULL) {
		CString full_path	= ::Get_File_Mgr ()->Make_Full_Path (path);		
		retval = false;

#ifdef PUBLIC_EDITOR_VER
	
		//
		//	If the file exists locally, or its coming from the mix file
		// then load it.
		//
		if (	::Get_File_Mgr ()->Does_File_Exist (full_path) ||
				::Get_File_Mgr ()->Is_File_In_VSS (full_path))
		{
			retval = WW3DAssetManager::Load_3D_Assets (full_path);
		}
#else

		//
		//	If the file exists, then load it.
		//
		if (::Get_File_Mgr ()->Does_File_Exist (full_path, true)) {
			retval = WW3DAssetManager::Load_3D_Assets (full_path);
		}

#endif //PUBLIC_EDITOR_VER

	}

	return retval;
}


////////////////////////////////////////////////////////////////////////
//
//	Get_File
//
////////////////////////////////////////////////////////////////////////
FileClass *
EditorFileFactoryClass::Get_File (char const *filename)
{
	CString path;

	//
	//	If the filename contains a relative path, then
	// turn it into an absolute path.
	//
	if (::Is_Path (filename)) {		
		path = ::Get_File_Mgr ()->Make_Full_Path (filename);
	} else {

		//
		//	Make a full path out of the filename
		//
		const char *curr_dir	= _pThe3DAssetManager->Get_Current_Directory ();
		path						= Make_Path (curr_dir, filename);

		//
		//	If the file doesn't exist here, then try the search paths
		//
		if (!cPathUtil::PathExists (path)) {
		
			//
			//	Try to find the file in our search path list
			//
			for (int index = 0; index < SearchPathList.Count (); index ++) {
				
				//
				//	Does the file exist in this directoy?
				//
				StringClass full_path = static_cast<const char *>(Make_Path (SearchPathList[index], filename));
				if (cPathUtil::PathExists (full_path)) {
					path = full_path;
					break;
				}
			}
		}
	}

	//
	//	Get the file from the database
	//
	return ::Get_File_Mgr ()->Get_Database_Interface ().Get_File (path);
}


////////////////////////////////////////////////////////////////////////
//
//	Return_File
//
////////////////////////////////////////////////////////////////////////
void
EditorFileFactoryClass::Return_File (FileClass *file)
{
	if (file != NULL) {
		delete file;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Open_Texture_File_Cache
//
////////////////////////////////////////////////////////////////////////
void
EditorAssetMgrClass::Open_Texture_File_Cache (const char * /*prefix*/)
{
	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Close_Texture_File_Cache
//
////////////////////////////////////////////////////////////////////////
void
EditorAssetMgrClass::Close_Texture_File_Cache (void)
{
	return ;
}


