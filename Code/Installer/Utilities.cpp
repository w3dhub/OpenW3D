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
 *                     $Archive:: /Commando/Code/Installer/Utilities.cpp  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/11/02 10:27a                $* 
 *                                                                                             * 
 *                    $Revision:: 10                    $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "Utilities.h"
#include "ErrorHandler.h"
#include "resource.h"
#include "Translator.h"
#include <shlwapi.h>
#include <stdlib.h>
#include	<winbase.h>
#include <shlobj.h>


// Private prototypes.
static bool	  Create_Directory (const WCHAR *drive, const WCHAR *directory, WCHAR *subdirectory, bool remove, DynamicVectorClass <StringClass> *log);
static WCHAR *Standardize_Path (const WideStringClass &path, WideStringClass &standardpath);
static bool	  Validate_Install_CD (const char driveletter, const WideStringClass &installvolumename);


/***********************************************************************************************
 * Prompt_Install_CD -- Is the install CD in one of the CD drives? If not prompt for it.		  *
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
bool Prompt_Install_CD (const WideStringClass &installvolumename, char &sourcedriveletter)
{
	bool success = false;

	while (!success) {
	
		for (char r = 'A'; r <= 'Z'; r++) {
			
			char rootpathname [] = "?:\\";

			rootpathname [0] = r;

			// If the drive is a CD-ROM...
			if (GetDriveType (rootpathname) == DRIVE_CDROM) {
				if (Validate_Install_CD (r, installvolumename)) {
					sourcedriveletter = r;
					success = true;
					break;
				}
			}
		}

		if (!success) {

			bool cancel;

			// No CD-ROM with the correct label found. Ask user to insert the CD-ROM.
			ShowCursor (true);
			cancel = MessageBoxExW (NULL, TxWideStringClass (IDS_INSERT_GAME_CD_ROM), TxWideStringClass (IDS_APPLICATION_NAME), MB_RETRYCANCEL | MB_APPLMODAL | MB_SETFOREGROUND, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT)) == IDCANCEL;
			ShowCursor (false);
			if (cancel) break;
		}
	}

	return (success);
}


/***********************************************************************************************
 * Validate_Install_CD -- Is the install CD in the designated CD drive?								  *
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
bool Validate_Install_CD (const WideStringClass &sourcepath, const WideStringClass &installvolumename)
{
	StringClass multibytesourcepath (sourcepath);
	return (Validate_Install_CD (multibytesourcepath [0], installvolumename));
}


/***********************************************************************************************
 * Validate_Install_CD -- Is the install CD in the designated CD drive?								  *
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
bool Validate_Install_CD (const char driveletter, const WideStringClass &installvolumename)
{
	char	multibytevolumename [_MAX_PATH];
	char	filesystemname [_MAX_PATH];
	DWORD	maxfilenamelength;
	DWORD	flags;
	char	rootpathname [] = "?:\\";

	rootpathname [0] = driveletter;
	if (GetVolumeInformation (rootpathname, multibytevolumename, _MAX_PATH, NULL, &maxfilenamelength, &flags, filesystemname, _MAX_PATH)) {
				 
		WideStringClass volumename (multibytevolumename);
		
		// Windows '95 appears to have a volume name limit of 11 characters. I cannot find a Win32
		// call that will return the maximum volume name length so the value '11' is hard-coded here
		// and the assumption made that all OS's have this length or better.
		if (wcsncmp (volumename, installvolumename, MAX (11, volumename.Get_Length())) == 0)	{
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * Get_Disk_Space_Available --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool Get_Disk_Space_Available (const WideStringClass &path, int64_t &diskspace)
{
	StringClass    multibytepath (path);
	char				drive [_MAX_DRIVE];
	ULARGE_INTEGER freebytecount;		// Free bytes on disk available to caller (caller may not have access to entire disk).
	ULARGE_INTEGER totalbytecount;	// Total bytes on disk.
	StringClass		kernelpathname;

	int (__stdcall *getfreediskspaceex) (LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);

	// Extract the drive.
	// NOTE 0: Even though GetDiskFreeSpaceEx() will accept a full path, it will err if the path is not valid.
	// NOTE 1: _splitpath() automatically handles multi-byte character strings.
	_splitpath (multibytepath, drive, NULL, NULL, NULL);

	GetSystemDirectory (kernelpathname.Get_Buffer (_MAX_PATH), _MAX_PATH);
	kernelpathname += "\\";
	kernelpathname += "Kernel32.dll";
	getfreediskspaceex = (int (_stdcall*) (LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER)) GetProcAddress (GetModuleHandle (kernelpathname.Peek_Buffer()), "GetDiskFreeSpaceExA");
	if (getfreediskspaceex != NULL) {

		// NOTE: This function uses GetDiskFreeSpaceEx() and therefore assumes Win '95 OSR2 or greater.
		if (!getfreediskspaceex (drive, &freebytecount, &totalbytecount, NULL)) return (false);
	
		// Convert to a 64-bit integer.
		diskspace = freebytecount.QuadPart;
	
	} else {

		DWORD sectorspercluster, bytespersector, freeclustercount, totalclustercount;
		
		// The Ex version is not available. Use the Win'95 version.
		// QUESTION: SDK docs say that values returned by this function are erroneous if partition > 2Gb.
		//				 Does that mean that the partition is guaranteed to be <= 2Gb if Ex is not available?
		if (!GetDiskFreeSpace (drive, &sectorspercluster, &bytespersector, &freeclustercount, &totalclustercount)) return (false); 
		diskspace = sectorspercluster * bytespersector * freeclustercount;
	}

	return (true);
}


/***********************************************************************************************
 * Validate_Path --																									  *
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
bool Validate_Path (const WideStringClass &path, int &errorcode)
{
	bool valid = false;

	// Check that path has syntax <drive>:\<directory>\<directory>...
	if (path.Get_Length() > _MAX_DRIVE) {
		if (((path [0] >= L'A') && (path [0] <= L'Z')) || ((path [0] >= L'a') && (path [0] <= L'z'))) {
			if ((path [1] == L':') && ((path [2] == L'\\') || (path [2] == L'/'))) {

				const WCHAR *c = ((WideStringClass) path).Peek_Buffer() + _MAX_DRIVE;
	
				while	(*c != L'\0') {
					if ((*c == L'\\') || (*c == L'/')) {

						// Cannot have two adjacent slashes.
						if ((*(c - 1) == L'\\') || (*(c - 1) == L'/')) break;
					}
					c++;
				}
	
				// Parsed entire string?
				if (*c == L'\0') {
			
					// Check that path does not contain any illegal characters.
					if (wcscspn (((WideStringClass) path).Peek_Buffer(), L"*?<>|") == (size_t) path.Get_Length()) {

						WCHAR				 drive [_MAX_DRIVE + 1];
						WideStringClass directory;

						wcsncpy (drive, path, _MAX_DRIVE);
						drive [_MAX_DRIVE] = L'\0';
						directory = ((WideStringClass) path).Peek_Buffer() + _MAX_DRIVE;

						// Attempt to create (and then remove) the path.
						if (Create_Directory (drive, directory.Peek_Buffer(), directory.Peek_Buffer(), true, NULL)) {
							valid = true;	
						} else {
							errorcode = IDS_CANNOT_CREATE_DIRECTORY;
						}

					} else {
						errorcode = IDS_INVALID_PATH;
					}
				
				} else {
					errorcode = IDS_INVALID_PATH;
				}

			} else {
				errorcode = IDS_INVALID_PATH;
			}

		} else {
			errorcode = IDS_INVALID_PATH;
		}

	} else {
		errorcode = IDS_INVALID_PATH;
	}

	return (valid);
}


/***********************************************************************************************
 * Create_Directory --																								  *
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
bool Create_Directory (const WideStringClass &path, DynamicVectorClass <StringClass> *log)
{
	WCHAR				 drive [_MAX_DRIVE + 1];
	WideStringClass directory;

	// NOTE: Assumes that path is a full path. If not, an error may be thrown.
	wcsncpy (drive, path, _MAX_DRIVE);
	drive [_MAX_DRIVE] = L'\0';
	directory = ((WideStringClass) path).Peek_Buffer() + _MAX_DRIVE;

	return (Create_Directory (drive, directory.Peek_Buffer(), directory.Peek_Buffer(), false, log));
}
	

/***********************************************************************************************
 * Create_Directory --																								  *
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
bool Create_Directory (const WCHAR *drive, const WCHAR *directory, WCHAR *subdirectory, bool remove, DynamicVectorClass <StringClass> *log)
{
	bool				  valid;
	WCHAR				 *c, replacement;
	WideStringClass  path;
	StringClass		  multibytepath;
	DWORD				  errorcode;

	// If subdirectory is empty, we're done.
	c = subdirectory;
	if (*c != L'\0') {

		// Step down the subdirectory to the next slash or null.
		while ((*c != L'\\') && (*c != L'/') && (*c != L'\0')) c++;
		replacement = *c;
		*c = L'\0';

		path  = drive;
		path += directory;

		path.Convert_To (multibytepath);
		if (CreateDirectory (multibytepath, NULL)) {
			errorcode = 0;
			if (log != NULL) log->Add (multibytepath);
		} else {
			errorcode = GetLastError();
		}
		
		// If no error or directory already exists...
		if ((errorcode == 0) || (errorcode == ERROR_ALREADY_EXISTS)) {

			// Recurse if not at end of string.
			if (replacement != L'\0') {
				*c = replacement;
				valid = Create_Directory (drive, directory, c + 1, remove, log);
			} else {
				valid = true;
			}

			// If caller wants it removed (and it didn't already exist) then remove it (ie. just verify that it can be created).
			if (remove && (errorcode == 0)) {
				if (!RemoveDirectory (multibytepath)) FATAL_SYSTEM_ERROR;
			}
		
		} else {
			valid = false;
		}

	} else {
		valid = true;
	}
		
	return (valid);
}


/***********************************************************************************************
 * Cluster_Padding --																								  *
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
int64_t Cluster_Padding (unsigned filecount)
{
	// NOTE: Currently, it is not known how to obtain (or if it is possible to obtain) the cluster size
	//			of a hard disk. According to MS documentation, the largest possible cluster size of FAT16,
	//			FAT32 and NTFS file systems is 64K (and is more typically 4-16K). Therefore, conservatively
	//			estimate the cluster size as 64K.

	const unsigned clustersize = 0x10000;

	int64_t padding = clustersize * filecount;
	return (padding);
}


/***********************************************************************************************
 * Is_Same_Path --																										  *
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
bool Is_Same_Path (const WideStringClass &path0, const WideStringClass &path1, bool standardize)
{
	if (standardize) {

		WideStringClass path0copy, path1copy;
		
		Standardize_Path (path0, path0copy);
		Standardize_Path (path1, path1copy);
		return (path0copy.Compare (path1copy) == 0);
	} else {
		return (path0.Compare (path1) == 0);
	}
}


/***********************************************************************************************
 * Is_Sub_Path --																										  *
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
bool Is_Sub_Path (const WideStringClass &path0, const WideStringClass &path1, bool standardize)
{
	WideStringClass path0copy, path1copy;

	if (standardize) {
		Standardize_Path (path0, path0copy);
		Standardize_Path (path1, path1copy);
	} else {
		path0copy = path0;
		path1copy = path1;
	}

	if	(wcsstr (path0copy, path1copy) != NULL) {

		if (path0copy.Get_Length() == path1copy.Get_Length()) {
		
			return (true);
		
		} else {
	
			WCHAR c;

			c = path0copy [path1copy.Get_Length()];
			if ((c == L'\\') || (c == L'/')) return (true);
		}
	}

	return (false);
}


/***********************************************************************************************
 * Standardize_Path --																								  *
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
WCHAR *Standardize_Path (const WideStringClass &path, WideStringClass &standardpath)
{
	WideStringClass t (path);
	int				 i, c;

	// Remove leading white space.
	i = 0;
	while (t [i] == L' ') {
		i++;
	}
	if (i > 0) t.Erase (0, i);

	// Remove trailing white space and any trailing slashes.
	c = 0;
	i = t.Get_Length() - 1;
	while ((i >= 0) && ((t [i] == L' ') || (t [i] == L'\\') || (t [i] == L'/'))) {
		i--;
		c++;
	}
	if (c > 0) t.Erase (i + 1, c);

	// Replace back slashes with forward slashes and change to upper case.
	for (i = 0; i < t.Get_Length(); i++) {
		if (t [i] == L'/') t [i] = L'\\';
		t [i] = towupper (t [i]);
	}

	standardpath = t;
	return (standardpath.Peek_Buffer());
}


/***********************************************************************************************
 * Remove_Trailing_Name --																							  *
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
WCHAR *Remove_Trailing_Name (WideStringClass &path)
{
	int i, c;

	for (i = path.Get_Length() - 1, c = 1; i >= 0; i--, c++) {
		if ((path [i] == L'\\') || (path [i] == L'/')) {
			path.Erase (i, c);
			break;
		}
	}
	return (path.Peek_Buffer());
}


/***********************************************************************************************
 * Extract_Suffix_Root --																							  *
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
WCHAR *Extract_Suffix_Root (WideStringClass &path, const WideStringClass &prefixpath)
{
	if (wcsstr (path, prefixpath) == path) {

		int s, e, c;
		
		s = prefixpath.Get_Length();
		if ((path [s] == L'\\') || (path [s] == L'/')) s++;
		if (s > 0) path.Erase (0, s);
		e = 0;
		c = path.Get_Length();
		for (e = 0, c = path.Get_Length(); c > 0; e++, c--) { 
			if ((path [e] == L'\\') || (path [e] == L'/')) {
				path.Erase (e, c);
				break;
			}
		}
		return (path.Peek_Buffer());
	}
	return (NULL);
}


/***********************************************************************************************
 * Extract_Trailing_Name --																						  *
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
WCHAR *Extract_Trailing_Name (WideStringClass &path)
{
	int i;

	for (i = path.Get_Length() - 1; i >= 0; i--) {
		if ((path [i] == L'\\') || (path [i] == L'/')) {
			path.Erase (0, i + 1);
			break;
		}
	}
	return (path.Peek_Buffer());
}


/***********************************************************************************************
 * Directory_Exists --																								  *
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
bool Directory_Exists (const WideStringClass &path)
{
	StringClass multibytepath (path);
	DWORD			errorcode;

	if (CreateDirectory (multibytepath, NULL)) {
		errorcode = 0; 
	} else {
		errorcode = GetLastError();
	}
	
	if (errorcode == 0) {
		if (!RemoveDirectory (multibytepath)) FATAL_SYSTEM_ERROR;
		return (false);
	} else {
		return (errorcode == ERROR_ALREADY_EXISTS);
	}
}


/***********************************************************************************************
 * Is_System_Directory --																							  *
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
bool Is_System_Directory (const WideStringClass &path)
{
	const int DirectoryTypes [] = {

		CSIDL_DESKTOP,							// 0x0000 <user name>\desktop
		CSIDL_PROGRAMS,						// 0x0002 <user name>\Start Menu\Programs
		CSIDL_PERSONAL,						// 0x0005 <user name>\Start Menu\My Documents
		CSIDL_FAVORITES,						// 0x0006 <user name>Favorites
		CSIDL_STARTUP,							// 0x0007 <user name>\Start Menu\Programs\Startup
		CSIDL_RECENT,							// 0x0008 <user name>\Recent
		CSIDL_SENDTO,							// 0x0009 <user name>\SendTo
		CSIDL_STARTMENU,						// 0x000b <user name>\Start Menu
		CSIDL_DESKTOPDIRECTORY,				// 0x0010 <user name>\Desktop
		CSIDL_NETHOOD,							// 0x0013 <user name>\nethood
		CSIDL_FONTS,							// 0x0014 windows\fonts
		CSIDL_TEMPLATES,						// 0x0015 <user name>\Templates
		CSIDL_COMMON_STARTMENU,				// 0x0016 All Users\Start Menu
		CSIDL_COMMON_PROGRAMS,				// 0X0017 All Users\Start Menu\Programs
		CSIDL_COMMON_STARTUP,				// 0x0018 All Users\Start Menu\Programs\Startup
		CSIDL_COMMON_DESKTOPDIRECTORY,	// 0x0019 All Users\Desktop
		CSIDL_APPDATA,							// 0x001a <user name>\Application Data
		CSIDL_PRINTHOOD,						// 0x001b <user name>\PrintHood
		CSIDL_COMMON_FAVORITES,				// 0x001f All Users\Favorites
		CSIDL_INTERNET_CACHE,				// 0x0020 <user name>\Local Settings\Temporary Internet Files
		CSIDL_COOKIES,							// 0x0021 <user name>\Cookies
		CSIDL_HISTORY							// 0x0022 <user name>\Local Settings\History
	};

	char				 multibytesystempath [_MAX_PATH];
	WideStringClass systempath;
	bool				 result;
	HINSTANCE		 hinstLib; 

	// Test for Windows directory.
	if (!GetWindowsDirectory (multibytesystempath, _MAX_PATH)) FATAL_SYSTEM_ERROR;
	systempath = multibytesystempath;
	if (Is_Sub_Path (path, systempath)) return (true);

	//	 Test for System directory.
	if (!GetSystemDirectory (multibytesystempath, _MAX_PATH)) FATAL_SYSTEM_ERROR;
	systempath = multibytesystempath;
	if (Is_Sub_Path (path, systempath)) return (true);

   // Get a handle to the DLL module.
   result = false;
	hinstLib = LoadLibrary ("shfolder.dll"); 
	if (hinstLib != NULL) { 

		HRESULT (__stdcall * PFNSHGETFOLDERPATHA)(HWND, int, HANDLE, DWORD, LPSTR);
		PFNSHGETFOLDERPATHA = ( HRESULT ( __stdcall * )(HWND, int, HANDLE, DWORD, LPSTR)) GetProcAddress( hinstLib, "SHGetFolderPathA" );

		if (PFNSHGETFOLDERPATHA != NULL) {

			HRESULT hr = 0;

			for (int i = 0; i < sizeof (DirectoryTypes) / sizeof(int); i++) {			

				// Get each of the known directories above and compare to the desired directory.
				hr = (*PFNSHGETFOLDERPATHA)( NULL, DirectoryTypes [i], NULL, 0, multibytesystempath);
				if (hr == S_OK) {
					systempath = multibytesystempath;
					if (Is_Sub_Path (path, systempath)) {
						result = true;
						break;
					}
				}
			}
    	}

		// Free the DLL module.
		FreeLibrary (hinstLib); 
	}
	
	return (result);
}


/***********************************************************************************************
 * Get_Current_Directory --																						  *
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
void Get_Current_Directory (WideStringClass &path)
{
	StringClass multibytepath;

	if (!GetCurrentDirectory (_MAX_PATH, multibytepath.Get_Buffer (MAX_PATH))) FATAL_SYSTEM_ERROR;
	path = multibytepath;
}


/***********************************************************************************************
 * Generate_Temporary_Pathname --																				  *
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
bool Generate_Temporary_Pathname (const WideStringClass &path, StringClass &multibytetemporarypathname)
{
	StringClass multibytepath;

	// Create temporary file.
	multibytepath = path;
	if (!GetTempFileName (multibytepath, "XXX", 0, multibytetemporarypathname.Get_Buffer (_MAX_PATH))) return (false);
	return (true);
}


/***********************************************************************************************
 * Message_Box --																										  *
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
void Message_Box (const WideStringClass &header, const WideStringClass &errormessage)
{
	ShowCursor (true);
	MessageBoxExW (NULL, ((WideStringClass) errormessage).Peek_Buffer(), ((WideStringClass) header).Peek_Buffer(), MB_OK | MB_APPLMODAL | MB_SETFOREGROUND, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT));
	ShowCursor (false);
}


