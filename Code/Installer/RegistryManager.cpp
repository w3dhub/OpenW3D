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
 *                     $Archive:: /Commando/Code/Installer/RegistryManager.cpp  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/25/02 5:09p                 $* 
 *                                                                                             * 
 *                    $Revision:: 11                    $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "Installer.h"
#include "ErrorHandler.h"
#include "RegistryManager.h"
#include "resource.h"
#include "Translator.h"
#include "Utilities.h"
#include "Win.h"
#include <mmsystem.h>
#include	<winreg.h>


// Defines.
#define SHELL_FOLDERS_KEY				  "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
#define PROGRAMS_PATH_KEY				  "Programs"
#define DESKTOP_PATH_KEY				  "Desktop"		

#define SOFTWARE_KEY						  "Software"	
#define WESTWOOD_KEY						  "Westwood"

#define NAME_KEY							  "Name"
#define DESKTOP_SHORTCUT_PATHNAME_KEY "Desktop"			// Pathname of shortcut file to launch game.
#define FOLDER_PATH_KEY					  "FolderPath"					
#define INSTALL_PATHNAME_KEY			  "InstallPath"	// NOTE: This is a misnomer. Actually refers to product pathname.
#define LANGUAGE_KEY						  "Language"	
#define SERIAL_KEY						  "Serial"
#define SKU_KEY							  "SKU"
#define VERSION_KEY						  "Version"
#define WOLSETTINGS_KEY					  "WOLSettings"	
#define URL_KEY							  "URL"	

#define WOLAPI_KEY						  "WOLAPI"
#define WOLREGISTER_KEY					  "Register"
#define WOLBROWSER_KEY					  "WOLBrowser"
#define USAGE_KEY							  "Usage"	

#define MICROSOFT_KEY					  "Microsoft"
#define WINDOWS_KEY						  "Windows"
#define CURRENT_VERSION_KEY			  "CurrentVersion"
#define APP_PATHS_KEY					  "App Paths"
#define PATH_KEY							  "Path"	
#define UNINSTALL_KEY					  "Uninstall"	
#define DISPLAY_NAME_KEY				  "DisplayName"
#define UNINSTALLER_FILENAME_KEY		  "UninstallString"	
#define EXTENSIONS_KEY					  "Extensions"	
#define DOCUMENT_KEY						  "doc"

#define WESTWOOD_CHAT_KEY				  "Wchat"	
#define NICKNAME_INDEX_KEY				  "Nick%u"
#define NICKNAME_KEY						  "Nick"
#define PASSWORD_KEY						  "Pass"
#define PREFERRED_LOGIN_KEY			  "AutoLogin"
#define OPTIONS_KEY						  "Options"	

// Internet Game Room flags.
#define IGR_NO_AUTO_LOGIN  				0x01
#define IGR_NEVER_STORE_NICKS 			0x02
#define IGR_NEVER_RUN_REG_APP				0x04
#define IGR_ALL								(IGR_NO_AUTO_LOGIN | IGR_NEVER_STORE_NICKS | IGR_NEVER_RUN_REG_APP)
#define IGR_NONE								0x00


// Singleton.
RegistryManagerClass _RegistryManager;


/***********************************************************************************************
 * RegistryManagerClass::RegistryManagerClass --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
RegistryManagerClass::RegistryManagerClass()
{
	WOLKeys [WOLAPI_COMPONENT]		  = WOLAPI_KEY;
	WOLKeys [WOLREGISTER_COMPONENT] = WOLREGISTER_KEY;
	WOLKeys [WOLBROWSER_COMPONENT]  = WOLBROWSER_KEY;
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Folder_Path --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Folder_Path (WideStringClass &path)
{
	return (Get_String (HKEY_CURRENT_USER, &path, SHELL_FOLDERS_KEY, PROGRAMS_PATH_KEY, NULL));
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Desktop_Path --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Desktop_Path (WideStringClass &path)
{
	return (Get_String (HKEY_CURRENT_USER, &path, SHELL_FOLDERS_KEY, DESKTOP_PATH_KEY, NULL));
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Target_Game_Pathname --														  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Target_Game_Pathname (WideStringClass &pathname)
{
	return (Get_String (HKEY_CURRENT_USER, &pathname, SOFTWARE_KEY, WESTWOOD_KEY, RxStringClass (IDS_GAME_PRODUCT_KEY), INSTALL_PATHNAME_KEY, NULL));
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Target_Game_Path --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Target_Game_Path (WideStringClass &path)
{
	bool				 success = false;	
	WideStringClass pathname;

	if (Get_Target_Game_Pathname (pathname)) {

		// Remove filename.
		path = pathname;
		Remove_Trailing_Name (path);
		success = true;
	}

	return (success);
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Target_Game_Folder --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Target_Game_Folder (WideStringClass &folder)
{
	bool				  success = false;
	WideStringClass  path, folderpath;
	wchar_t				 *suffixroot;

	if (Get_String (HKEY_CURRENT_USER, &path, SOFTWARE_KEY, WESTWOOD_KEY, RxStringClass (IDS_GAME_PRODUCT_KEY), FOLDER_PATH_KEY, NULL)) {
		Get_Folder_Path (folderpath);
		suffixroot = Extract_Suffix_Root (path, folderpath);
		if (suffixroot != NULL) {
			folder = path;	
			success = true;
		}	
	}

	return (success);
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Target_WOL_Pathname --														  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Target_WOL_Pathname (WOLComponentEnum wolcomponent, WideStringClass &pathname)
{
	return (Get_String (HKEY_CURRENT_USER, &pathname, SOFTWARE_KEY, WESTWOOD_KEY, WOLKeys [wolcomponent], INSTALL_PATHNAME_KEY, NULL));
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Target_WOL_Path --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Target_WOL_Path (WOLComponentEnum wolcomponent, WideStringClass &path)
{
	bool				 success = false;
	WideStringClass pathname;

	if (Get_Target_WOL_Pathname (wolcomponent, pathname)) {

		// Remove filename.
		path = pathname;
		Remove_Trailing_Name (path);
		success = true;
	}
	
	return (success);
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Target_WOL_Folder --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Target_WOL_Folder (WOLComponentEnum wolcomponent, WideStringClass &folder)
{
	bool				  success = false;
	WideStringClass  path, folderpath;
	wchar_t				 *suffixroot;

	if (Get_String (HKEY_CURRENT_USER, &path, SOFTWARE_KEY, WESTWOOD_KEY, WOLKeys [wolcomponent], FOLDER_PATH_KEY, NULL)) {
		Get_Folder_Path (folderpath);
		suffixroot = Extract_Suffix_Root (path, folderpath);
		if (suffixroot != NULL) {
			folder = path;	
			success = true;
		}	
	}

	return (success);
}


/***********************************************************************************************
 * RegistryManagerClass::Get_WOL_Version --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Target_WOL_Version (WOLComponentEnum wolcomponent, DWORD &version)
{
	return (Get_Value (HKEY_CURRENT_USER, &version, SOFTWARE_KEY, WESTWOOD_KEY, WOLKeys [wolcomponent], VERSION_KEY, NULL));
}


/***********************************************************************************************
 * RegistryManagerClass::Use_IGR_Settings --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Use_IGR_Settings (bool &useigrsettings)
{
	bool	success;	
	DWORD igrsettings;

	success = Get_Value (HKEY_CURRENT_USER, &igrsettings, SOFTWARE_KEY, WESTWOOD_KEY, WOLAPI_KEY, OPTIONS_KEY, NULL);
	if (success) {
		useigrsettings = (igrsettings == IGR_ALL);
	}
	return (success);
}


/***********************************************************************************************
 * RegistryManagerClass::Register_Game --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void RegistryManagerClass::Register_Game (const WideStringClass &name,
														const WideStringClass &installpathname,
														const WideStringClass &folderpath,
														const WideStringClass &desktopshortcutpathname,
														DWORD languageid,
														const StringClass &serialnumber,
														DWORD	sku,
														DWORD version)
{
	DWORD				 maskedsku;			
	RxStringClass	 gameproductkey (IDS_GAME_PRODUCT_KEY);
	WideStringClass wideserialnumber (serialnumber);
	WideStringClass s, url;

	// The input SKU also contains the languageid in the LSB, so mask this off.
	maskedsku = sku & 0xffffff00;

	Set_String (HKEY_CURRENT_USER, name,						   SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, NAME_KEY,							   NULL);
	Set_String (HKEY_CURRENT_USER, installpathname,			SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, INSTALL_PATHNAME_KEY,				NULL);
	Set_String (HKEY_CURRENT_USER, folderpath,				   SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, FOLDER_PATH_KEY,					NULL);
	Set_String (HKEY_CURRENT_USER, desktopshortcutpathname, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, DESKTOP_SHORTCUT_PATHNAME_KEY, NULL);
	Set_Value  (HKEY_CURRENT_USER, languageid,					SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, LANGUAGE_KEY,						NULL);
	Set_String (HKEY_CURRENT_USER, wideserialnumber,			SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, SERIAL_KEY,							NULL);
	Set_Value  (HKEY_CURRENT_USER, sku,							SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, SKU_KEY,								NULL);
	Set_Value  (HKEY_CURRENT_USER, version,						SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, VERSION_KEY,							NULL);

	s = L"http://renchat2.westwood.com/cgi-bin/cgiclient?ren_clan_manager&request=expand_template&Template=index.html&SKU=%d&LANGCODE=%d&embedded=1";
	url.Format (s, maskedsku, languageid);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "BattleClans", NULL);

	s = L"http://renchat2.westwood.com/cgi-bin/cgiclient?ren_clan_manager&request=expand_template&Template=index.html&SKU=%d&LANGCODE=%d";
	url.Format (s, maskedsku, languageid);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "BattleClansX", NULL);

	s = L"http://renchat2.westwood.com/renegade_embedded/index.html";
	url.Format (s);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "Ladder", NULL);

	s = L"http://renchat2.westwood.com/renegade/index.html";
	url.Format (s);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "LadderX", NULL);

	s = L"http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_netstatus&LANGCODE=%d&SKU=%d&embedded=1";
	url.Format (s, languageid, maskedsku);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "NetStatus", NULL);

	s = L"http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_netstatus&LANGCODE=%d&SKU=%d";
	url.Format (s, languageid, maskedsku);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "NetStatusX", NULL);

	s = L"http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_news&LANGCODE=%d&SKU=%d&embedded=1";
	url.Format (s, languageid, maskedsku);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "News", NULL);

	s = L"http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_news&LANGCODE=%d&SKU=%d";
	url.Format (s, languageid, maskedsku);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "NewsX", NULL);

	s = L"http://renchat2.westwood.com/cgi-bin/cgiclient?live_ren_register&request=expand_template&Template=newreg_menu.html&LANGCODE=%d&embedded=1&SKU=%d";
	url.Format (s, languageid, maskedsku);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "Signup", NULL);

	s = L"http://renchat2.westwood.com/cgi-bin/cgiclient?live_ren_register&request=expand_template&Template=newreg_menu.html&LANGCODE=%d&SKU=%d";
	url.Format (s, languageid, maskedsku);
	Set_String (HKEY_CURRENT_USER, url, SOFTWARE_KEY, WESTWOOD_KEY, gameproductkey, WOLSETTINGS_KEY, URL_KEY, "SignupX", NULL);

	// Register the executable with the OS.
	s = installpathname;
	Remove_Trailing_Name (s);
	Set_String (HKEY_CURRENT_USER, s,					 SOFTWARE_KEY, MICROSOFT_KEY, WINDOWS_KEY, CURRENT_VERSION_KEY, APP_PATHS_KEY, RxStringClass (IDS_GAME_EXECUTABLE_FILENAME), PATH_KEY, NULL);
	Set_String (HKEY_CURRENT_USER, installpathname, SOFTWARE_KEY, MICROSOFT_KEY, WINDOWS_KEY, CURRENT_VERSION_KEY, APP_PATHS_KEY, RxStringClass (IDS_GAME_EXECUTABLE_FILENAME), "",		   NULL);

	// Register the uninstall program with the OS.
	Set_String (HKEY_CURRENT_USER, TxWideStringClass (IDS_VERBOSE_GAME_PRODUCT_NAME), SOFTWARE_KEY, MICROSOFT_KEY, WINDOWS_KEY, CURRENT_VERSION_KEY, UNINSTALL_KEY, RxStringClass (IDS_GAME_PRODUCT_KEY), DISPLAY_NAME_KEY, NULL);
	s += L"\\";
	s += RxWideStringClass (IDS_GAME_UNINSTALL_FILENAME); 
	Set_String (HKEY_CURRENT_USER, s, SOFTWARE_KEY, MICROSOFT_KEY, WINDOWS_KEY, CURRENT_VERSION_KEY, UNINSTALL_KEY, RxStringClass (IDS_GAME_PRODUCT_KEY), UNINSTALLER_FILENAME_KEY, NULL);
}	


/***********************************************************************************************
 * RegistryManagerClass::Register_WOLAPI --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void RegistryManagerClass::Register_WOLAPI (const WideStringClass &folderpath,
	  													  const WideStringClass &installpathname,
									  					  const WideStringClass &name,
									  					  DWORD sku,
	  													  DWORD version,
														  bool  useigrsettings)
{
	const DWORD igrsettings [] = {IGR_NONE, IGR_ALL};

	WideStringClass s;

	Set_String (HKEY_CURRENT_USER, folderpath,		 SOFTWARE_KEY, WESTWOOD_KEY, WOLAPI_KEY, FOLDER_PATH_KEY,		NULL);
	Set_String (HKEY_CURRENT_USER, installpathname, SOFTWARE_KEY, WESTWOOD_KEY, WOLAPI_KEY, INSTALL_PATHNAME_KEY, NULL);
	Set_String (HKEY_CURRENT_USER, name,				 SOFTWARE_KEY, WESTWOOD_KEY, WOLAPI_KEY, NAME_KEY,				   NULL);
	Set_Value  (HKEY_CURRENT_USER, sku,				 SOFTWARE_KEY, WESTWOOD_KEY, WOLAPI_KEY, SKU_KEY,				   NULL);
	Set_Value  (HKEY_CURRENT_USER, version,			 SOFTWARE_KEY, WESTWOOD_KEY, WOLAPI_KEY, VERSION_KEY,			   NULL);

	// Register the uninstall program with the OS.
	Set_String (HKEY_CURRENT_USER, TxWideStringClass (IDS_VERBOSE_WOL_PRODUCT_NAME), SOFTWARE_KEY, MICROSOFT_KEY, WINDOWS_KEY, CURRENT_VERSION_KEY, UNINSTALL_KEY, WOLAPI_KEY, DISPLAY_NAME_KEY, NULL);
	s  = installpathname;
	Remove_Trailing_Name (s);
	s += L"\\";
	s += RxWideStringClass (IDS_WOL_UNINSTALL_FILENAME);
	Set_String (HKEY_CURRENT_USER, s, SOFTWARE_KEY, MICROSOFT_KEY, WINDOWS_KEY, CURRENT_VERSION_KEY, UNINSTALL_KEY, WOLAPI_KEY, UNINSTALLER_FILENAME_KEY, NULL);

	// Write the game room settings.
	Set_Value (HKEY_CURRENT_USER, igrsettings [useigrsettings], SOFTWARE_KEY, WESTWOOD_KEY, WOLAPI_KEY, OPTIONS_KEY, NULL);
}


/***********************************************************************************************
 * RegistryManagerClass::Register_WOLRegister --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void RegistryManagerClass::Register_WOLRegister (const WideStringClass &folderpath,
	  															 const WideStringClass &installpathname,
	  															 const WideStringClass &name,
	  															 DWORD sku,
																 DWORD version)
{
	Set_String (HKEY_CURRENT_USER, folderpath,		 SOFTWARE_KEY, WESTWOOD_KEY, WOLREGISTER_KEY, FOLDER_PATH_KEY,		  NULL);
	Set_String (HKEY_CURRENT_USER, installpathname, SOFTWARE_KEY, WESTWOOD_KEY, WOLREGISTER_KEY, INSTALL_PATHNAME_KEY, NULL);
	Set_String (HKEY_CURRENT_USER, name,				 SOFTWARE_KEY, WESTWOOD_KEY, WOLREGISTER_KEY, NAME_KEY,				  NULL);
	Set_Value  (HKEY_CURRENT_USER, sku,				 SOFTWARE_KEY, WESTWOOD_KEY, WOLREGISTER_KEY, SKU_KEY,				  NULL);
	Set_Value  (HKEY_CURRENT_USER, version,			 SOFTWARE_KEY, WESTWOOD_KEY, WOLREGISTER_KEY, VERSION_KEY,			  NULL);
}


/***********************************************************************************************
 * RegistryManagerClass::Register_WOLBrowser --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void RegistryManagerClass::Register_WOLBrowser (const WideStringClass &installpathname,
	  															const WideStringClass &name,
	  															DWORD version)
{
	Set_String (HKEY_CURRENT_USER, installpathname, SOFTWARE_KEY, WESTWOOD_KEY, WOLBROWSER_KEY, INSTALL_PATHNAME_KEY, NULL);
	Set_String (HKEY_CURRENT_USER, name,				 SOFTWARE_KEY, WESTWOOD_KEY, WOLBROWSER_KEY, NAME_KEY,				 NULL);
	Set_Value  (HKEY_CURRENT_USER, version,			 SOFTWARE_KEY, WESTWOOD_KEY, WOLBROWSER_KEY, VERSION_KEY,			 NULL);
}


/***********************************************************************************************
 * RegistryManagerClass::Update_WOLAPI --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void RegistryManagerClass::Update_WOLAPI()
{
	DWORD			keyindex, usagecount;
	StringClass keyname;

	keyindex	  = 0;
	usagecount = 0;
	while (Get_Key (HKEY_CURRENT_USER, keyindex, &keyname, SOFTWARE_KEY, WESTWOOD_KEY,	NULL)) {

		DWORD				 sku;
		WideStringClass serialnumber;

		// If a SKU and a serial number exist...
		if (Get_Value (HKEY_CURRENT_USER, &sku, SOFTWARE_KEY, WESTWOOD_KEY, keyname, SKU_KEY, NULL)) {
			if (Get_String (HKEY_CURRENT_USER, &serialnumber, SOFTWARE_KEY, WESTWOOD_KEY, keyname, SERIAL_KEY, NULL)) {
				
				WideStringClass widekeyname;
				char				 skuname [MAX_NUMBER_STRING_LENGTH];	

				// Create a key for the key name under the WOLAPI key.
				ltoa (sku, skuname, 10);
				widekeyname = keyname;
				Set_String (HKEY_CURRENT_USER, widekeyname, SOFTWARE_KEY, WESTWOOD_KEY, WOLAPI_KEY, skuname, NAME_KEY, NULL);
				usagecount++;
			}
		}
		keyindex++;
	}

	// Update the WOLAPI usage count.
	Set_Value (HKEY_CURRENT_USER, usagecount, SOFTWARE_KEY, WESTWOOD_KEY, WOLAPI_KEY, USAGE_KEY, NULL);
}


/***********************************************************************************************
 * RegistryManagerClass::Get_WOL_Account --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_WOL_Account (unsigned index, WideStringClass &name, WideStringClass &password)
{
	unsigned nonemptyindex, i;

	// Find the n'th key with a non-empty name string.
	// NOTE: Conversion from zero-based to one-based index.
	nonemptyindex = 0;
	i = 1;
	while (true) {

		StringClass key;

		key.Format (NICKNAME_INDEX_KEY, i);
		if (Get_String (HKEY_CLASSES_ROOT, &name, WESTWOOD_CHAT_KEY, key, NICKNAME_KEY, "", NULL)) {
			if (name != L"") {
				if (index == nonemptyindex) {
					return (Get_String (HKEY_CLASSES_ROOT, &password, WESTWOOD_CHAT_KEY, key, PASSWORD_KEY, "", NULL));
				}
				nonemptyindex++;
			}
		} else {
			break;
		}
		i++;
	}

	// Key not found.
	return (false);
}


/***********************************************************************************************
 * RegistryManagerClass::Set_Preferred_WOL_Account --														  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void RegistryManagerClass::Set_Preferred_WOL_Account (const WideStringClass &accountname)
{
	Set_String (HKEY_CURRENT_USER, accountname, SOFTWARE_KEY, WESTWOOD_KEY, RxStringClass (IDS_GAME_PRODUCT_KEY), WOLSETTINGS_KEY, PREFERRED_LOGIN_KEY, NULL);
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Document_Application_Pathname -- 											  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Document_Application_Pathname (WideStringClass &pathname)
{
	return (Get_String (HKEY_CURRENT_USER, &pathname, SOFTWARE_KEY, MICROSOFT_KEY, WINDOWS_KEY, CURRENT_VERSION_KEY, EXTENSIONS_KEY, DOCUMENT_KEY, NULL));
}


/***********************************************************************************************
 * RegistryManagerClass::Get_String --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_String (HKEY rootkey, WideStringClass *string, ...)
{
	bool			 success = false;
	va_list		 marker;
	char			*keyname, *nextkeyname;	
	StringClass  keystring;
	HKEY			 key;

	va_start (marker, string);

  	keyname = va_arg (marker, char*);
	if (keyname != NULL) {

		// Concatenate all but the last key name.
		nextkeyname = va_arg (marker, char*);
		while (nextkeyname != NULL) {

			if (keystring.Get_Length() > 0) keystring += "\\";
			keystring  += keyname;
			keyname     = nextkeyname;
			nextkeyname = va_arg (marker, char*);
		}

		if (RegOpenKeyEx (rootkey, keystring, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {

			DWORD type;
			DWORD size;

			if (RegQueryValueEx (key, keyname, NULL, &type, NULL, &size) == ERROR_SUCCESS) {
				if (type == REG_SZ) {
					
					StringClass multibytestring;

					if (RegQueryValueEx (key, keyname, NULL, &type, (BYTE*) multibytestring.Get_Buffer (size), &size) == ERROR_SUCCESS) {
						*string = multibytestring;
						success = true;
					}
				}
			}
			RegCloseKey (key);
		}
	}

	va_end (marker);

	return (success);
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Value --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool RegistryManagerClass::Get_Value (HKEY rootkey, DWORD *value, ...)
{
	bool			 success = false;
	va_list		 marker;
	char			*keyname, *nextkeyname;	
	StringClass  keystring;
	HKEY			 key;

	va_start (marker, value);

  	keyname = va_arg (marker, char*);
	if (keyname != NULL) {

		// Concatenate all but the last key name.
		nextkeyname = va_arg (marker, char*);
		while (nextkeyname != NULL) {

			if (keystring.Get_Length() > 0) {
				keystring += "\\";
			}
			keystring  += keyname;
			keyname     = nextkeyname;
			nextkeyname = va_arg (marker, char*);
		}

		if (RegOpenKeyEx (rootkey, keystring, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {

			DWORD type;
			DWORD size;

			if (RegQueryValueEx (key, keyname, NULL, &type, NULL, &size) == ERROR_SUCCESS) {
				if (type == REG_DWORD) {
					if (RegQueryValueEx (key, keyname, NULL, &type, (BYTE*) value, &size) == ERROR_SUCCESS) {
						success = true;
					}
				}
			}
			RegCloseKey (key);
		}
	}

	va_end (marker);

	return (success);
}


/***********************************************************************************************
 * RegistryManagerClass::Set_String --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool __cdecl RegistryManagerClass::Set_String (HKEY rootkey, const wchar_t *string, ...)
{
	HKEY	   key;
	DWORD	   disposition;
	bool	   success = false;
	va_list  marker;
	char	  *keyname, *nextkeyname;	

	va_start (marker, string);

	// Create subkeys (if necessary).
	key = rootkey;
  	keyname = va_arg (marker, char*);
	if (keyname != NULL) {
	
		nextkeyname = va_arg (marker, char*);
		while (true) {

			if (nextkeyname == NULL) {
				
				StringClass multibytestring (string);
				
				if (RegSetValueEx (key, keyname, 0, REG_SZ, (BYTE*) multibytestring.Peek_Buffer(), multibytestring.Get_Length() + 1) == ERROR_SUCCESS) {
					success = true;
				}
				break;
			}
		
			if (RegCreateKeyEx (key, keyname, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disposition) != ERROR_SUCCESS) {
				break;
			}
			keyname = nextkeyname;
			nextkeyname = va_arg (marker, char*);
		}
	}

	va_end (marker);

	return (success);
}


/***********************************************************************************************
 * RegistryManagerClass::Set_Value --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool __cdecl RegistryManagerClass::Set_Value (HKEY rootkey, DWORD value, ...)
{
	HKEY	   key;
	DWORD	   disposition;
	bool	   success = false;
	va_list  marker;
	char	  *keyname, *nextkeyname;	

	va_start (marker, value);

	// Create subkeys (if necessary).
	key = rootkey;
  	keyname = va_arg (marker, char*);
	if (keyname != NULL) {
	
		nextkeyname = va_arg (marker, char*);
		while (true) {

			if (nextkeyname == NULL) {
				if (RegSetValueEx (key, keyname, 0, REG_DWORD, (BYTE*) &value, sizeof (value)) == ERROR_SUCCESS) {
					success = true;
				}
				break;
			}
		
			if (RegCreateKeyEx (key, keyname, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disposition) != ERROR_SUCCESS) {
				break;
			}
			keyname = nextkeyname;
			nextkeyname = va_arg (marker, char*);
		}
	}

	va_end (marker);

	return (success);
}


/***********************************************************************************************
 * RegistryManagerClass::Get_Key --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool __cdecl RegistryManagerClass::Get_Key (HKEY rootkey, DWORD keyindex, StringClass *keyname, ...)
{
	bool			 success = false;
	va_list		 marker;
	char			*ckeyname;
	StringClass  keystring;
	HKEY			 key;

	va_start (marker, keyname);

   while ((ckeyname = va_arg (marker, char*)) != NULL) {
		if (keystring.Get_Length() > 0) {
			keystring += "\\";
		}
		keystring += ckeyname;
	}

	if (RegOpenKeyEx (rootkey, keystring, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {

		FILETIME	lastwritetime;
		DWORD		size;

		if (RegEnumKeyEx (key, keyindex, keyname->Get_Buffer (256), &size, 0, NULL, NULL, &lastwritetime) == ERROR_SUCCESS) {
			success = true;
		}

		RegCloseKey (key);
	}

	va_end (marker);

	return (success);
}