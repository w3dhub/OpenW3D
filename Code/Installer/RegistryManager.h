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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Installer                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Installer/RegistryManager.h  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/25/01 2:35p                $* 
 *                                                                                             * 
 *                    $Revision:: 6                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _REGISTRY_MANAGER_H
#define _REGISTRY_MANAGER_H

// Includes
#include "registry.h"


// Forward declarations.
class StringClass;


class RegistryManagerClass {

	public:

		enum WOLComponentEnum {

			COMPONENT_FIRST			= 0,
			WOLAPI_COMPONENT			= COMPONENT_FIRST,
			WOLREGISTER_COMPONENT,
			WOLBROWSER_COMPONENT,
			COMPONENT_COUNT
		};

		RegistryManagerClass();

		bool Get_Folder_Path (WideStringClass &path);
		bool Get_Desktop_Path (WideStringClass &path);
		bool Get_Target_Game_Pathname (WideStringClass &path);
		bool Get_Target_Game_Path (WideStringClass &path);
		bool Get_Target_Game_Folder (WideStringClass &folder);
      bool Get_Target_WOL_Pathname (WOLComponentEnum wolcomponent, WideStringClass &pathname);
		bool Get_Target_WOL_Path (WOLComponentEnum wolcomponent, WideStringClass &path);
		bool Get_Target_WOL_Folder (WOLComponentEnum wolcomponent, WideStringClass &folder);
		bool Get_Target_WOL_Version (WOLComponentEnum wolcomponent, DWORD &version);
		bool Use_IGR_Settings (bool &useigrsettings);
		
		void Register_Game (const WideStringClass &name,
								  const WideStringClass &installpathname,
								  const WideStringClass &folderpath,
								  const WideStringClass &desktopshortcutpathname,
								  DWORD languageid,
								  const StringClass &serialnumber,
								  DWORD sku,
								  DWORD version);

		void Register_WOLAPI (const WideStringClass &folderpath,
			  						 const WideStringClass &installpathname,
			  						 const WideStringClass &name,
			  						 DWORD sku,
			  						 DWORD version,
									 bool	 useigrsettings);

		void Update_WOLAPI();

		void Register_WOLRegister (const WideStringClass &folderpath,
			  								const WideStringClass &installpathname,
			  								const WideStringClass &name,
			  								DWORD sku,
											DWORD version);

		void Register_WOLBrowser (const WideStringClass &installpathname,
			  							  const WideStringClass &name,
			  							  DWORD version);

		bool Get_WOL_Account (unsigned index, WideStringClass &name, WideStringClass &password);
		void Set_Preferred_WOL_Account (const WideStringClass &accountname);
		bool Get_Document_Application_Pathname (WideStringClass &pathname);

	protected:
		
		bool __cdecl Get_String (HKEY rootkey, WideStringClass *string, ...);
		bool __cdecl Get_Value (HKEY rootkey, DWORD *value, ...);
		bool __cdecl Set_String (HKEY rootkey, const WCHAR *string, ...);
		bool __cdecl Set_Value (HKEY rootkey, DWORD value, ...);
		bool __cdecl Get_Key (HKEY rootkey, DWORD keyindex, StringClass *keyname, ...);

		char *WOLKeys [COMPONENT_COUNT];
};


// Singleton.
extern RegistryManagerClass _RegistryManager;


#endif // REGISTRY_MANAGER_H