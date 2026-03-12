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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditorAssetMgr.h             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/29/02 10:33a                                              $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __LEVELEDITASSETMGR_H
#define __LEVELEDITASSETMGR_H

#include "AssetMgr.H"
#include "FFactory.H"

/////////////////////////////////////////////////////////////////////////////
//
//	EditorFileFactoryClass
//
/////////////////////////////////////////////////////////////////////////////
class	EditorFileFactoryClass : public FileFactoryClass
{
public:

	///////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////
	virtual FileClass *	Get_File (char const *filename);
	virtual void			Return_File (FileClass *file);

	//
	//	Search path support
	//
	static void				Add_Search_Path (const char *path)	{ SearchPathList.Add (path); }
	static DynamicVectorClass<StringClass>	&	Get_Search_Path (void)	{ return SearchPathList; }

private:

	///////////////////////////////////////////////////
	//	Private data
	///////////////////////////////////////////////////
	static DynamicVectorClass<StringClass>	SearchPathList;
};


/////////////////////////////////////////////////////////////////////////////
//
// EditorAssetMgrClass
//
/////////////////////////////////////////////////////////////////////////////
class EditorAssetMgrClass : public WW3DAssetManager
{
	public:

		///////////////////////////////////////////////////
		//	Public constructors/destructors
		///////////////////////////////////////////////////
		EditorAssetMgrClass (void);
		virtual ~EditorAssetMgrClass (void) {}

		///////////////////////////////////////////////////
		//	Public methods
		///////////////////////////////////////////////////

		//
		// Base class overrides
		//
		virtual RenderObjClass *		Create_Render_Obj (const char * name);
		virtual HAnimClass *				Get_HAnim (const char * name);
		virtual HTreeClass *				Get_HTree (const char * name);
		virtual TextureClass *			Get_Texture (const char *filename, TextureClass::MipCountType mip_level_count=TextureClass::MIP_LEVELS_ALL,WW3DFormat texture_format=WW3D_FORMAT_UNKNOWN, bool allow_compression=true);


		//
		//	INI methods
		//
		virtual class EditorINIClass *Get_INI (const char *filename);

		//
		//	Asset managment
		//
		virtual void						Reload_File (const char *filename);
		virtual void						Load_Resource_Texture (const char *filename);

		//
		//	Directory methods
		//
		void									Set_Current_Directory (LPCTSTR path)	{ m_CurrentDir = path; ::SetCurrentDirectory (m_CurrentDir); }
		const char *						Get_Current_Directory (void)				{ return m_CurrentDir; }

		//
		// Load data from any type of w3d file
		//
		virtual bool						Load_3D_Assets (const char * filename);

		//
		//	Texture caching overrides
		//
		virtual void						Open_Texture_File_Cache(const char * /*prefix*/);
		virtual void						Close_Texture_File_Cache();

	protected:

		///////////////////////////////////////////////////
		//	Protected methods
		///////////////////////////////////////////////////
		virtual bool						Determine_Real_Location (LPCTSTR filename, CString &real_location);
		virtual bool						Is_File_Here (LPCTSTR full_path, bool get_from_vss);

	private:

		///////////////////////////////////////////////////
		//
		//	Private member data
		//
		CString								m_CurrentDir;
};


#endif //__LEVELEDITASSETMGR_H
