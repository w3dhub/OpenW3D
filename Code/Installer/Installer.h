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
 *                     $Archive:: /Commando/Code/Installer/Installer.h  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/17/02 5:01p                 $* 
 *                                                                                             * 
 *                    $Revision:: 13                    $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _INSTALLER_H
#define _INSTALLER_H

// Includes.
#include "RegistryManager.h"
#include "Utilities.h"
#include "Wwuiinput.h"


// Forward declarations.
class	AudibleSoundClass;
class CardSelectionDialogClass;
class CopyDialogClass;
class DialogBaseClass;
class FinalDialogClass;
class GameDirectoryDialogClass;
class GameFolderDialogClass;
class INIClass;
class NDADialogClass;
class LicenseDialogClass;
class MixFileFactoryClass;
class	ReviewDialogClass;
class SerialDialogClass;
class WelcomeDialogClass;
class WhatToInstallDialogClass;
class WOL1DialogClass;
class WOL2DialogClass;
class WOLDirectoryDialogClass;
class WOLFolderDialogClass;
class WWAudioClass;
class	WW3DAssetManager;


// Class definitions.
class InstallerUIInputClass : public WWUIInputClass
{
	public:

		InstallerUIInputClass()	: WWUIInputClass(), MousePosition (Vector3 (0.0f, 0.0f, 0.0f)) {}

		void Add_Mouse_Wheel (short delta)
		{
			MousePosition.Z += delta;
		}	

	private:

		const Vector3 &Get_Mouse_Pos (void) const
		{
			POINT cursorposition;

			GetCursorPos (&cursorposition);
			ScreenToClient (MainWindow, &cursorposition);
			MousePosition.X = cursorposition.x;
			MousePosition.Y = cursorposition.y;
			return (MousePosition);
		}
		
		void Set_Mouse_Pos (const Vector3 &position)
		{	
			POINT p;

			if (GameInFocus) {
				p.x = position.X;
				p.y = position.Y;
				ClientToScreen (MainWindow, &p);
				SetCursorPos (p.x, p.y);
				MousePosition.X = p.x;
				MousePosition.Y = p.y;
			}
		}

		bool Is_Button_Down (int vk_mouse_button_id)
		{
			bool retval = false;
			switch (vk_mouse_button_id)
			{
				case VK_LBUTTON:
				case VK_MBUTTON:
				case VK_RBUTTON:
					retval = (GetKeyState (vk_mouse_button_id) & 0x80000000) != 0x0;
					break;
			}

			return retval;
		}

		mutable Vector3 MousePosition;
};


class FileLogClass {

	public:

		// Equality operator.
		bool operator == (const FileLogClass &f) {
			return ((RelativePathname == f.RelativePathname) && (Size == f.Size));
		}

		// Inequality operator.
		bool operator != (const FileLogClass &f) {
			return (!(*this == f));
		}
	
		WideStringClass RelativePathname;		// Relative pathname of file.
		int64_t			 Size;						// Size of file (in bytes).
};


class InstallerClass
{
	public:

		 InstallerClass();
		~InstallerClass(); 
		
		InstallerUIInputClass *Get_Input() {return (UIInput);}

		void Cancel_Introduction()			  {CancelIntroduction = true;}

		void Install (MixFileFactoryClass *mixfilefactory);
		void Dialog_Callback (DialogBaseClass *dialog, int ctrl_id, int message_id, DWORD param);
		void On_Prog_End();

  		// Main interface.
		const WCHAR			*Get_Source_Game_Path (WideStringClass &path)		{path = SourceGamePath; return (path);}
		const WCHAR			*Get_Source_WOL_Path (WideStringClass &path) 		{path = SourceWOLPath; return (path);}
		bool					 Install_Game();
		bool					 Install_WOL();
		bool					 Install_Game_Shortcut();
		int64_t				 Get_Game_Size (bool ondisk);
		int64_t				 Get_WOL_Size (bool ondisk);
		int64_t				 Get_Total_Size (bool ondisk);
		bool					 Get_Game_Space_Available (const WideStringClass &path, int64_t &diskspace) {return (Get_Disk_Space_Available (path, diskspace));}
		bool					 Get_WOL_Space_Available (const WideStringClass &path, int64_t &diskspace);
		const char			*Get_Serial_Number (StringClass &serialnumber);
		const WCHAR			*Get_Target_Game_Path (WideStringClass &path);
		bool					 Get_Target_Sub_Path (unsigned index, WideStringClass &subdirectoryname);
		const WCHAR			*Get_Target_WOL_Path (WideStringClass &path);
		const WCHAR			*Get_Target_Game_Folder (WideStringClass &folder);
		const WCHAR			*Get_Target_WOL_Folder (WideStringClass &folder);
		bool					 Is_Fresh_Game_Install()									{return (FreshGameInstall);}				
		bool					 Is_Fresh_WOL_Install()										{return (FreshWOLInstall);}
		bool					 Is_Target_WOL_Older_Than_Source()						{return (TargetWOLOlder);}
		WW3DAssetManager	*Get_Asset_Manager()											{return (AssetManager);}
		bool					 Can_Use_IGR_Settings();
		bool					 Use_IGR_Settings();
		bool					 Beta_Test();
		
		// Registry interface.
		void Update_Registry();
		void Create_Links();
		bool Get_WOL_Account (unsigned index, WideStringClass &name, WideStringClass &password);
		void Set_Preferred_WOL_Account (const WideStringClass &accountname) {_RegistryManager.Set_Preferred_WOL_Account (accountname);}

		// File/subdirectory logging interface.
		void Log (const WideStringClass &pathname, int64_t size = -1);
		void Create_Uninstall_Logs();

	protected:

		enum ProgEndActionEnum {
			ACTION_NONE,
			ACTION_RUN_GAME,
			ACTION_DISPLAY_README
		};

		// Internal initialization/shutdown.
		void Initialize();
		void Shutdown();
		bool WW3D_Initialize (int cardselection = -1);
		void WW3D_Shutdown();

		// Support functions.
		bool Check_Source();
		void Count_Source_Files (const WideStringClass &sourcepath, unsigned &filecount, int64_t &filesize);
		void Check_Existing_Install();
		void Auto_Configure();
		bool Register_COM_Server (const WideStringClass &comdll);
		void Create_Encryption_File (const WideStringClass &pathname);
		void Encrypt (const char *number, const WideStringClass &pathname, StringClass &encryptednumber);
		bool Create_File_Link (const WideStringClass &linkpath, const WideStringClass &title, const WideStringClass &targetpathname, const WideStringClass &iconpathname, const WideStringClass *arguments = NULL);
		bool Create_URL_Link (const WideStringClass &linkpath, const WideStringClass &title, const WideStringClass &url);
		bool Has_WOL_Account();
		void Create_Game_Uninstall_Log();
		void Create_WOL_Uninstall_Log();
		void Run_Game();
		void Display_Readme();

		// Data.
		INIClass										 *SetupIni;
		INIClass										 *GameIni;
		INIClass										 *WOLIni;
		WideStringClass							  SourceGamePath;
		WideStringClass							  SourceWOLPath;
		MixFileFactoryClass						 *MixFileFactory;
		DynamicVectorClass <WideStringClass>  FontNames;
		WideStringClass							  TargetGamePath;
		bool											  FreshGameInstall;
		bool											  FreshWOLInstall;
		bool											  TargetWOLOlder;
		unsigned										  GameFileCount;
		int64_t										  GameSize;	
		unsigned										  WOLFileCount;
		int64_t										  WOLSize;	
		WW3DAssetManager							 *AssetManager;
		bool											  WwmathInitialized;
		bool											  DialogMgrInitialized;
		WWAudioClass								 *AudioSystem;
		AudibleSoundClass							 *TransitionMusic;
		AudibleSoundClass							 *InstallMusic;
		InstallerUIInputClass					 *UIInput;
		bool											  CancelIntroduction;
		bool											  CancelApplication;
		WelcomeDialogClass						 *WelcomeDialog;
		NDADialogClass								 *NDADialog;
		LicenseDialogClass						 *LicenseDialog;	
		SerialDialogClass							 *SerialDialog;
		WhatToInstallDialogClass				 *WhatToInstallDialog;
	   GameDirectoryDialogClass				 *GameDirectoryDialog;
	   GameFolderDialogClass					 *GameFolderDialog;
	   WOLDirectoryDialogClass					 *WOLDirectoryDialog;
	   WOLFolderDialogClass						 *WOLFolderDialog;
	   ReviewDialogClass							 *ReviewDialog;
		CopyDialogClass							 *CopyDialog;
		WOL1DialogClass							 *WOL1Dialog;
		WOL2DialogClass							 *WOL2Dialog;
		FinalDialogClass							 *FinalDialog;
		CardSelectionDialogClass				 *CardSelectionDialog;
		DynamicVectorClass <WideStringClass>  GameSubdirectories;
		DynamicVectorClass <FileLogClass>	  GameFiles;
		DynamicVectorClass <FileLogClass>	  WOLFiles;
		ProgEndActionEnum							  ProgEndAction;
};


// Singleton.
extern InstallerClass _Installer;


#endif // INSTALLER_H