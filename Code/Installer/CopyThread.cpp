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
 *                     $Archive:: /Commando/Code/Installer/CopyThread.cpp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/13/02 5:00p                                                                $* 
 *                                                                                             * 
 *                    $Revision:: 10                    $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "CopyThread.h"
#include "ErrorHandler.h"
#include "FDI.h"
#include "Installer.h"
#include "RegistryManager.h"
#include "resource.h"
#include "Translator.h"
#include "Verchk.h"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>


// Defines.
#define COPY_MESSAGE_FORMAT_STRING L"%s %s"


// Static data.
CopyThreadClass *CopyThreadClass::_ActiveCopyThread = NULL;


/***************************************************************************
 * Memory allocation function
 * HUGE * FAR DIAMONDAPI fdi_mem_alloc( ULONG cb )
 *=========================================================================*/
FNALLOC( fdi_mem_alloc )
{
	return (malloc (cb));
}


/***************************************************************************
 * Memory free function
 * FAR DIAMONDAPI fdi_mem_free( void HUGE *pv )
 *=========================================================================*/
FNFREE( fdi_mem_free )
{
	free (pv);
}


/***************************************************************************
 * int FAR DIAMONDAPI file_open( char FAR *pszFile, int oflag, int pmode )
 *=========================================================================*/
FNOPEN( file_open )
{
	HANDLE handle;

	WWASSERT ((oflag & (_O_APPEND | _O_TEMPORARY)) == 0x0);
	
	DWORD desiredaccess, creationdisposition, flagsandattributes;

	if (oflag & _O_WRONLY) {
		desiredaccess = GENERIC_WRITE;
	} else {
		if (oflag & _O_RDWR) {
			desiredaccess = GENERIC_READ | GENERIC_WRITE;
		} else {
			desiredaccess = GENERIC_READ;
		}
	}

	if (oflag & (_O_CREAT | _O_TRUNC)) {
		creationdisposition = OPEN_ALWAYS;
	} else {
		if (oflag & _O_CREAT) {
			creationdisposition = CREATE_NEW;
		} else {
			if (oflag & _O_TRUNC) {
				creationdisposition = TRUNCATE_EXISTING;
			} else {
				creationdisposition = OPEN_EXISTING;
			}
		}
	}

	if (pmode & _S_IREAD) {
		flagsandattributes = FILE_ATTRIBUTE_READONLY;
	} else {
		flagsandattributes = FILE_ATTRIBUTE_NORMAL;
	}

	while ((handle = CreateFile (pszFile, desiredaccess, FILE_SHARE_READ, NULL, creationdisposition, flagsandattributes | FILE_FLAG_WRITE_THROUGH, NULL)) == INVALID_HANDLE_VALUE) {
		if (!CopyThreadClass::_ActiveCopyThread->Retry()) break;
	}	
	return ((int) handle);
}


/***************************************************************************
 * UINT FAR DIAMONDAPI file_read( int hf, void FAR *pv, UINT cb )
 *=========================================================================*/
FNREAD( file_read )
{
	// Abort the copying process?
	if (!CopyThreadClass::_ActiveCopyThread->Get_Abort (true)) {

		unsigned int bytecount;
	  
		while (!ReadFile ((void*) hf, pv, cb, &bytecount, NULL)) {
			if (!CopyThreadClass::_ActiveCopyThread->Retry()) break;
		}
		return (bytecount);

	} else {
		
		// NOTE: Returning -1 will cause the calling cabinet process to close all
		// files, release its resources, and return an error code to the caller of
		// FDICopy().
		return (-1);
	}
}


/***************************************************************************
 * UINT FAR DIAMONDAPI file_write( int hf, void FAR *pv, UINT cb )
 *=========================================================================*/
FNWRITE( file_write )
{
	// Abort the copying process?
	if (!CopyThreadClass::_ActiveCopyThread->Get_Abort (true)) {

		unsigned int bytecount;
		
		while (!WriteFile ((void*) hf, pv, cb, &bytecount, NULL)) {
			if (!CopyThreadClass::_ActiveCopyThread->Retry()) break;
		}

		if (bytecount >= 0) {
			CopyThreadClass::_ActiveCopyThread->Add_Bytes_Copied ((unsigned)bytecount);
		}
		return (bytecount);

	} else {
		
		// NOTE: Returning -1 will cause the calling cabinet process to close all
		// files, release its resources, and return an error code to the caller of
		// FDICopy().
		return (-1);
	}
}	


/***************************************************************************
 * int FAR DIAMONDAPI file_close( int hf )
 *=========================================================================*/
FNCLOSE( file_close )
{
	int result;
	
	while (!(result = CloseHandle ((void*) hf))) {
		if (!CopyThreadClass::_ActiveCopyThread->Retry()) break;
	}
	return (result ? 0 : -1);
}


/***************************************************************************
 * int FAR DIAMONDAPI file_seek( int hf, int dist, int seektype )
 *=========================================================================*/
FNSEEK( file_seek )
{
	int	result;
	DWORD movemethod;

	// Map seek type to Win32.
	switch (seektype) {

		case SEEK_SET:
			movemethod = FILE_BEGIN;
			break;

		case SEEK_CUR:
			movemethod = FILE_CURRENT;
			break;

		case SEEK_END:
			movemethod = FILE_END;
			break;
	}

	while ((result = SetFilePointer ((void*) hf, dist, NULL, movemethod)) == 0xffffffff) {
		if (!CopyThreadClass::_ActiveCopyThread->Retry()) break;
	}
	return (result);
}


/******************************************************************************
 * int FAR DIAMONDAPI notification_function( FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin )
 *=============================================================================*/
FNFDINOTIFY( notification_function )
{
	static WideStringClass _targetpathname;
	static StringClass	  _multibytetemporarypathname;
	static FILETIME		  _sourcefiletime;
	
	switch (fdint) {

		case fdintCOPY_FILE:			
		{
			FILETIME localfiletime;
			
			// Build target pathname.
			// NOTE: If the relative path does not contain subdirectories it will not contain a backslash.
			CopyThreadClass::_ActiveCopyThread->Get_Target_Path (_targetpathname);
			if (pfdin->psz1 [0] != '\\') _targetpathname += L"\\";
			_targetpathname += WideStringClass (pfdin->psz1);

			_Installer.Log (_targetpathname, pfdin->cb);
			
			// If target should be overwritten...
			if (!DosDateTimeToFileTime (pfdin->date, pfdin->time, &localfiletime)) return (-1);
			if (!LocalFileTimeToFileTime (&localfiletime, &_sourcefiletime)) return (-1);
			
			if (CopyThreadClass::Replace_File (_sourcefiletime, pfdin->cb, _targetpathname)) {

				WideStringClass statusmessage;
				int				 handle;
				WideStringClass targetpath;
				WideStringClass filename;

				// Update the name of the current file being copied.
				filename = WideStringClass (pfdin->psz1);
				Extract_Trailing_Name (filename);
				statusmessage.Format (COPY_MESSAGE_FORMAT_STRING, TxWideStringClass (IDS_COPYING), filename);
				CopyThreadClass::_ActiveCopyThread->Set_Status_Message (statusmessage);

				// Create temporary filename.
				targetpath = _targetpathname;
				Remove_Trailing_Name (targetpath);
				if (!Generate_Temporary_Pathname (targetpath, _multibytetemporarypathname)) return (-1);

				// Open the temporary file for writing.
				handle = file_open (_multibytetemporarypathname.Peek_Buffer(), _O_BINARY | _O_TRUNC | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL, _S_IREAD | _S_IWRITE);

				// Add temporary filename to log.
				CopyThreadClass::_ActiveCopyThread->Get_Filename_Log().Add (_multibytetemporarypathname);
				
				return (handle);

			} else {
				
				// Do not copy file.
				CopyThreadClass::_ActiveCopyThread->Add_Bytes_Copied ((unsigned) pfdin->cb);
				return (0);
			}
		}

		case fdintCLOSE_FILE_INFO:	
		{
			StringClass		 multibytetargetpathname;	
			HANDLE			 targetfile;
			WIN32_FIND_DATA targetfiledata;
			int				 count;	

			// Close the temporary file.
			if (file_close (pfdin->hf) != 0) return (-1);

			// Delete the original target file (if it exists).
			multibytetargetpathname = _targetpathname;
			targetfile = FindFirstFile (multibytetargetpathname, &targetfiledata);
			if (targetfile != INVALID_HANDLE_VALUE) {
				if (!FindClose (targetfile)) return (-1);
				if (targetfiledata.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
					if (!SetFileAttributes (multibytetargetpathname, FILE_ATTRIBUTE_NORMAL)) return (-1);
				}
				if (!DeleteFile (multibytetargetpathname)) return (-1);
			}

			// Rename the temporary file the target file.
			if (!MoveFile (_multibytetemporarypathname, multibytetargetpathname)) return (-1);

			// Remove the temporary file from the file log (the last item added).
			count = CopyThreadClass::_ActiveCopyThread->Get_Filename_Log().Count();
			WWASSERT (count > 0);
			CopyThreadClass::_ActiveCopyThread->Get_Filename_Log().Delete (count - 1);

			// Add the target file to the file log.
			CopyThreadClass::_ActiveCopyThread->Get_Filename_Log().Add (multibytetargetpathname);

			// Stamp the target file with write time of source.
			targetfile = CreateFile (multibytetargetpathname, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (targetfile == INVALID_HANDLE_VALUE) return (-1);
			if (!SetFileTime (targetfile, NULL, NULL, &_sourcefiletime)) return (-1);
			if (!CloseHandle (targetfile)) return (-1);

			// Stamp the target file with attributes of source.
			if (!SetFileAttributes (multibytetargetpathname, pfdin->attribs & (~FILE_ATTRIBUTE_READONLY))) return (-1);

			// Success.
			return (1);
		}
		
		case fdintNEXT_CABINET:				
		{
			// Nothing to do. Assume that the next cabinet file is in the same directory as the current
			// cabinet file.
			return (1);
		}

		default:
			return (1);
	}
}


/***********************************************************************************************
 * CopyThreadClass::CopyThreadClass --																			  *
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
CopyThreadClass::CopyThreadClass (int64_t bytestocopy)
	: ThreadClass(),
	  Status (STATUS_OK),
	  Abort (false),
	  CanAbort (true),
	  AbortLock (NULL),
	  IsAborting (false),
	  BytesToCopy (bytestocopy),
	  BytesCopied (0)
{
	// Only one instance can be active.
	WWASSERT (_ActiveCopyThread == NULL);
	_ActiveCopyThread = this;
}


/***********************************************************************************************
 * CopyThreadClass::~CopyThreadClass --																		  *
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
CopyThreadClass::~CopyThreadClass()
{
	WWASSERT (_ActiveCopyThread != NULL);
	_ActiveCopyThread = NULL;
}


/***********************************************************************************************
 * CopyThreadClass::Thread_Function --																			  *
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
void CopyThreadClass::Thread_Function()
{
	try {

		unsigned			 i;
		WideStringClass sourcepath, targetpath;
		int				 gamesubdirectorycount, gamefilecount; 

		// Copy the game directory if user requested it.
		if (_Installer.Install_Game()) {
	
			// Create game subdirectories on target.
			// NOTE 0: This could have been done whilst copying, but is more efficient to do here,
			//			  considering that InstallerClass already has this information.
			// NOTE 1: Directory paths should have been validated, so any errors here are fatal.
			if (!Create_Directory (_Installer.Get_Target_Game_Path (targetpath), &Get_Subdirectory_Log())) FATAL_SYSTEM_ERROR;
			i = 0;
			while (_Installer.Get_Target_Sub_Path (i, targetpath)) {
				_Installer.Log (targetpath);
				if (!Create_Directory (targetpath, &Get_Subdirectory_Log())) FATAL_SYSTEM_ERROR;
				i++;
			}

			Copy_Directory (_Installer.Get_Source_Game_Path (sourcepath), _Installer.Get_Target_Game_Path (targetpath));
		}

		gamesubdirectorycount = Get_Subdirectory_Log().Count();
		gamefilecount			 = Get_Filename_Log().Count();

		if (Is_Aborting()) goto abort;

		// Copy the WOL directory if user requested it.
		if (_Installer.Install_WOL()) {
			Copy_Directory (_Installer.Get_Source_WOL_Path (sourcepath), _Installer.Get_Target_WOL_Path (targetpath));
		}

		// Find out if the user wants to abort. At the same time disable the user's ability to abort 
		// because the operations to follow are irreversible ie. this is the last chance for the
		// user to abort.
		if (Get_Abort (false)) goto abort;

		// Update registry and make start menu items.
		Set_Status_Message (TxWideStringClass (IDS_UPDATING_REGISTRY));
		_Installer.Update_Registry();

		// Make menu items and icons.
		Set_Status_Message (TxWideStringClass (IDS_MAKING_MENU_ITEMS));
		_Installer.Create_Links();

		// Create uninstall logs.
		Set_Status_Message (TxWideStringClass (IDS_CREATING_UNINSTALL_LOGS));
		_Installer.Create_Uninstall_Logs();

		// Indicate completion.
		Set_Status_Message (TxWideStringClass (IDS_INSTALLATION_COMPLETE));
		Status = STATUS_SUCCESS;
		return;

	abort:

		Set_Status_Message (WideStringClass (L""));
		Status = STATUS_ABORTED;

		// Remove installed components if game or WOL was freshly installed.
		if (_Installer.Is_Fresh_Game_Install() || _Installer.Is_Fresh_WOL_Install()) {

			int i;

			Set_Status_Message (TxWideStringClass (IDS_REMOVING_INSTALLED_COMPONENTS));

			// NOTE: Order of removal must be inverse of order of creation to ensure that leaf
			//			subdirectories are removed before parent directories.	To satisfy this
			//			requirement, remove in the following order:
			//			1. WOL files (in any order).
			//			2. Game files (in any order).
			//			3. WOL subdirectories (in reverse order).
			//			4. Game subdirectories (in reverse order).
			if (_Installer.Is_Fresh_WOL_Install()) {
				for (i = gamefilecount; i < Get_Filename_Log().Count(); i++) {
					DeleteFile (Get_Filename_Log() [i]);
				}
			}
			if (_Installer.Is_Fresh_Game_Install()) {
				for (i = 0; i < gamefilecount; i++) {
					DeleteFile (Get_Filename_Log() [i]);
				}
			}
			if (_Installer.Is_Fresh_WOL_Install()) {
				for (i = Get_Subdirectory_Log().Count() - 1; i >= gamesubdirectorycount; i--) {
					RemoveDirectory (Get_Subdirectory_Log() [i]);
				}
			}
			if (_Installer.Is_Fresh_Game_Install()) {
				for (i = gamesubdirectorycount - 1; i >= 0; i--) {
					RemoveDirectory (Get_Subdirectory_Log() [i]);
				}
			}

			// Give time for user to read status message.
			Sleep (5000);
		}
		
	} catch (const WideStringClass &errormessage) {

		// Copy off the errormessage so that it can be interrogated by the thread creator.
		Set_Status_Message (WideStringClass (L""));
		Set_Error_Message (errormessage);
		Status = STATUS_FAILURE;
	}
}


/***********************************************************************************************
 * CopyThreadClass::Copy_Directory --																			  *
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
void CopyThreadClass::Copy_Directory (const WideStringClass &sourcepath, const WideStringClass &targetpath)
{
	// WARNING: Do not call SetCurrentDirectory() from this thread because it will affect
	//				file access for the entire process.

	const wchar_t *wildcardname = L"*.*";

	HFDI				 hfdi;
	ERF				 erf;
	WideStringClass sourcepathname;
	StringClass		 multibytesourcepathname;
	WIN32_FIND_DATA finddata;	
	HANDLE			 handle;
	bool				 done = false;			

	hfdi = FDICreate (fdi_mem_alloc,	// Memory allocation function.
							fdi_mem_free, 	// Memory free function.
							file_open,		// File open function.
							file_read,		// File read function.
							file_write,		// File write function.
							file_close,		// File close function.
							file_seek,		// File seek function.
							cpu80386,		// Type of CPU.
							&erf);			// Pointer to error structure.

	WWASSERT (hfdi != NULL);

	// Create subdirectory (if it doesn't already exist).
	if (!Create_Directory (targetpath, &Get_Subdirectory_Log())) FATAL_SYSTEM_ERROR;

	// Iterate over files in source directory and copy to target directory.
	sourcepathname  = sourcepath;
	sourcepathname += L"\\";
	sourcepathname += wildcardname;
	multibytesourcepathname = sourcepathname;
	
	// Find a source file.
	// NOTE At least one file must be in the directory.
	while ((handle = FindFirstFile (multibytesourcepathname, &finddata)) == INVALID_HANDLE_VALUE) {
		if (!Retry()) FATAL_SYSTEM_ERROR;
	}

	while (!done && !Is_Aborting()) {

		WideStringClass filename (finddata.cFileName);

		// Filter out system files.
  		if (filename [0] != L'.') {

  			// Is it a subdirectory?
  			if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

  				WideStringClass subsourcepath (sourcepath);
  				WideStringClass subtargetpath (targetpath);

  				// Recurse.
  				subsourcepath += L"\\";
  				subsourcepath += filename;
  				subtargetpath += L"\\";
  				subtargetpath += filename;

				_Installer.Log (subtargetpath);
				
				Copy_Directory (subsourcepath, subtargetpath);

  			} else {
					
  				const wchar_t *cabextension = L".cab";

  				wchar_t extension [_MAX_EXT];

   			// Is it a CAB file?
  				_wsplitpath (filename, NULL, NULL, NULL, extension);
	  			if (_wcsicmp (cabextension, extension) == 0) {

					StringClass sourcepathbackslash (sourcepath);

					// NOTE: FDICopy() requires a trailing backslash on the source path.
					sourcepathbackslash += "\\";

  					Set_Target_Path (targetpath);
					
					// Copy the contents of the cabinet file to the target directory.
					// If the function fails then throw a 'cabinet error'.
					if (!FDICopy (hfdi,											// Handle to FDI context (created by FDICreate()).
									  finddata.cFileName,	  					// Name of cabinet file, excluding path information.
									  sourcepathbackslash.Peek_Buffer(),	// File path to cabinet file.
									  0,												// Flags to control extract operation.
									  notification_function,					// Ptr to a notification (status update) function.
									  NULL,											// Ptr to a decryption function.
									  NULL)) 
									  
									  // If failure was not due to user aborting then throw an error.
									  if (!Is_Aborting()) FATAL_CAB_ERROR (erf.erfOper);

  				} else {
		
  					// Update the name of the current file being copied.
					WideStringClass statusmessage;
					int64_t			 filesize;	

					statusmessage.Format (COPY_MESSAGE_FORMAT_STRING, TxWideStringClass (IDS_COPYING), filename);
					Set_Status_Message (statusmessage);

					// It's a regular file.
  					WideStringClass sourcepathname (sourcepath);
  					WideStringClass targetpathname (targetpath);

  					sourcepathname += L"\\";
  					sourcepathname += filename;
  					targetpathname += L"\\";
  					targetpathname +=	filename;
  				
					filesize = ((((int64_t)MAXDWORD) + 1) * ((int64_t) finddata.nFileSizeHigh)) + ((int64_t) finddata.nFileSizeLow);
					_Installer.Log (targetpathname, filesize);
					
					// If target should be overwitten...
					if (Replace_File (sourcepathname, targetpathname)) {
						Copy_File (sourcepathname, targetpathname);
					} else {
						BytesCopied += filesize;
					}
  				}
  			}
  		}
		
		if (Is_Aborting()) break;
		
		while (done = (FindNextFile (handle, &finddata) == 0)) { 
			if (GetLastError() == ERROR_NO_MORE_FILES) break;
			if (!Retry()) FATAL_SYSTEM_ERROR;
		}
  	}
  	
	while (!FindClose (handle)) {
		if (!Retry()) FATAL_SYSTEM_ERROR;
	}

	FDIDestroy (hfdi);
}


/***********************************************************************************************
 * CopyThreadClass::Copy_File --																					  *
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
void CopyThreadClass::Copy_File (const WideStringClass &sourcepathname, const WideStringClass &targetpathname)
{
	const unsigned buffersize = 32768;

	static char _buffer [buffersize]; 
	
	StringClass		 multibytesourcepathname, multibytetargetpathname;
	WideStringClass targetpath;
	StringClass		 multibytetargetpath, multibytetemporarypathname;
	HANDLE			 sourcefile, temporaryfile, targetfile; 
	DWORD				 sourcebytecount, bytecount; 
	WIN32_FIND_DATA sourcefiledata, targetfiledata;

	multibytesourcepathname = sourcepathname;
	multibytetargetpathname = targetpathname;

	// Open the read file. 
	while ((sourcefile = CreateFile (multibytesourcepathname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
		if (!Retry()) FATAL_SYSTEM_ERROR;
	}

	// Create temporary file.
	targetpath = targetpathname;
	Remove_Trailing_Name (targetpath);
	if (!Generate_Temporary_Pathname (targetpath, multibytetemporarypathname)) FATAL_SYSTEM_ERROR;
	
	// Open the temporary file for writing.
	temporaryfile = CreateFile (multibytetemporarypathname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
	if (temporaryfile == INVALID_HANDLE_VALUE) FATAL_SYSTEM_ERROR;
 
	do { 
		
		// Abort the copying process?
		if (Get_Abort (true)) break;

		while (!ReadFile (sourcefile, _buffer, buffersize, &sourcebytecount, NULL)) {
			if (!Retry()) FATAL_SYSTEM_ERROR;
		}
		if (!WriteFile (temporaryfile, _buffer, sourcebytecount, &bytecount, NULL)) FATAL_SYSTEM_ERROR;
		BytesCopied += sourcebytecount;  

	} while (sourcebytecount == buffersize); 

	// Close.
	while (!CloseHandle (sourcefile)) {
		if (!Retry()) FATAL_SYSTEM_ERROR;
	}
	while (!CloseHandle (temporaryfile)) {
		if (!Retry()) FATAL_SYSTEM_ERROR;
	}

	while ((sourcefile = FindFirstFile (multibytesourcepathname, &sourcefiledata)) == INVALID_HANDLE_VALUE) {
		if (!Retry()) FATAL_SYSTEM_ERROR;
	}
	while (!FindClose (sourcefile)) {
		if (!Retry()) FATAL_SYSTEM_ERROR;
	}	

	// Delete the original target file (if it exists).
	targetfile = FindFirstFile (multibytetargetpathname, &targetfiledata);
	if (targetfile != INVALID_HANDLE_VALUE) {
		if (!FindClose (targetfile)) FATAL_SYSTEM_ERROR;
		if (targetfiledata.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			if (!SetFileAttributes (multibytetargetpathname, FILE_ATTRIBUTE_NORMAL)) FATAL_SYSTEM_ERROR;
		}
		if (!DeleteFile (multibytetargetpathname)) FATAL_SYSTEM_ERROR;
	}

	// Rename the temporary file the original target file.
	if (!MoveFile (multibytetemporarypathname, multibytetargetpathname)) FATAL_SYSTEM_ERROR;

	// Add the name of the file to the file log.
	Get_Filename_Log().Add (multibytetargetpathname);

	// Stamp the target file with write time of source.
	targetfile = CreateFile (multibytetargetpathname, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (targetfile == INVALID_HANDLE_VALUE) FATAL_SYSTEM_ERROR;
	if (!SetFileTime (targetfile, NULL, NULL, &sourcefiledata.ftLastWriteTime)) FATAL_SYSTEM_ERROR;
	if (!CloseHandle (targetfile)) FATAL_SYSTEM_ERROR;

	// Stamp the target file with file attributes of source (but clear the read-only flag to ensure that
	// the file can be patched at a later date).
	if (!SetFileAttributes (multibytetargetpathname, sourcefiledata.dwFileAttributes & (~FILE_ATTRIBUTE_READONLY))) FATAL_SYSTEM_ERROR;
}


/***********************************************************************************************
 * Replace_File --  																									  *
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
bool CopyThreadClass::Replace_File (const FILETIME &sourcefiletime, DWORD sourcefilesize, const WideStringClass &targetpathname)
{
	StringClass		 multibytetargetpathname (targetpathname);	
	WIN32_FIND_DATA targetdata;
	HANDLE			 handle;
	int				 t;	

	// Does the target file exist?
	handle = FindFirstFile (multibytetargetpathname, &targetdata); 
	if (handle == INVALID_HANDLE_VALUE) return (true);

	// Test last write times.
	t = CompareFileTime (&sourcefiletime, &targetdata.ftLastWriteTime);
	if	(t > 0) return (true);
	if (t < 0) return (false);

	// So far files are deemed equal. To err on the side of caution, test file sizes and elect to replace the file if sizes differ.
	return ((sourcefilesize != targetdata.nFileSizeLow) || (targetdata.nFileSizeHigh != 0));
}


/***********************************************************************************************
 * Replace_File --																									  *
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
bool CopyThreadClass::Replace_File (const WideStringClass &sourcepathname, const WideStringClass &targetpathname)
{
	StringClass			multibytesourcepathname (sourcepathname);
	StringClass			multibytetargetpathname (targetpathname);
	WIN32_FIND_DATA	sourcedata, targetdata;
	HANDLE				handle;
	VS_FIXEDFILEINFO	sourceversion, targetversion;
	int					t;	

	// Find source file. It must exist.
	while ((handle = FindFirstFile (multibytesourcepathname, &sourcedata)) == INVALID_HANDLE_VALUE) {
		if (!_ActiveCopyThread->Retry()) FATAL_SYSTEM_ERROR;
	}

	// Does the target file exist?
	handle = FindFirstFile (multibytetargetpathname, &targetdata); 
	if (handle == INVALID_HANDLE_VALUE) return (true);

	// Test version numbers (if they exist).
	if (GetVersionInfo (multibytesourcepathname.Peek_Buffer(), &sourceversion) && GetVersionInfo (multibytetargetpathname.Peek_Buffer(), &targetversion)) {
		if (sourceversion.dwFileVersionMS > targetversion.dwFileVersionMS) return (true);
		if (sourceversion.dwFileVersionMS < targetversion.dwFileVersionMS) return (false);
		if (sourceversion.dwFileVersionLS > targetversion.dwFileVersionLS) return (true);
		if (sourceversion.dwFileVersionLS < targetversion.dwFileVersionLS) return (false);
	}

	// Test last write times.
	t = CompareFileTime (&sourcedata.ftLastWriteTime, &targetdata.ftLastWriteTime);
	if	(t > 0) return (true);
	if (t < 0) return (false);

	// So far files are deemed equal. To err on the side of caution, test file sizes and elect to replace the file if sizes differ.
	return ((sourcedata.nFileSizeLow != targetdata.nFileSizeLow) || (sourcedata.nFileSizeHigh != targetdata.nFileSizeHigh));
}


/***********************************************************************************************
 * CopyThreadClass::Can_Abort --																					  *
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
bool CopyThreadClass::Can_Abort (bool lock)
{
	if (lock) {

		if (AbortLock == NULL) {
			AbortLock = new FastCriticalSectionClass::LockClass (SectionAbort);
		}
		return (CanAbort);

	} else {

		FastCriticalSectionClass::LockClass cs (SectionAbort);

		return (CanAbort);
	}
}


/***********************************************************************************************
 * CopyThreadClass::Set_Abort --																					  *
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
void CopyThreadClass::Set_Abort (bool abort)
{
	Abort = abort;
	WWASSERT (AbortLock != NULL);
	delete AbortLock;
	AbortLock = NULL;
}


/***********************************************************************************************
 * CopyThreadClass::Get_Abort --																					  *
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
bool CopyThreadClass::Get_Abort (bool canabort)
{
	FastCriticalSectionClass::LockClass cs (SectionAbort);
		
	CanAbort = canabort;
	if (Abort) IsAborting = true;
	return (Abort);
}


/***********************************************************************************************
 * CopyThreadClass::Add_Bytes_Copied --																		  *
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
void CopyThreadClass::Add_Bytes_Copied (unsigned bytecount)
{
	FastCriticalSectionClass::LockClass cs (SectionBytesCopied);

	BytesCopied += bytecount;
}


/***********************************************************************************************
 * CopyThreadClass::Get_Fraction_Complete --																	  *
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
float CopyThreadClass::Get_Fraction_Complete()
{
	FastCriticalSectionClass::LockClass cs (SectionBytesCopied);

	float fraction;

	fraction = ((double) BytesCopied) / ((double) BytesToCopy);
 
	// Clamp to valid range.
	fraction = MAX (0.0f, MIN (1.0f, fraction));

	return (fraction);
}


/***********************************************************************************************
 * CopyThreadClass::Set\Get_Target_Path --																	  *	
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
void CopyThreadClass::Set_Target_Path (const WideStringClass &targetpath)
{
	FastCriticalSectionClass::LockClass cs (SectionTargetPath);
	
	TargetPath = targetpath;
}

wchar_t *CopyThreadClass::Get_Target_Path (WideStringClass &targetpath)
{
	FastCriticalSectionClass::LockClass cs (SectionTargetPath);
	
	targetpath = TargetPath;
	return (targetpath.Peek_Buffer());
}


/***********************************************************************************************
 * CopyThreadClass::Set\Get_Status_Message --																  *	
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
void CopyThreadClass::Set_Status_Message (const WideStringClass &statusmessage)
{
	FastCriticalSectionClass::LockClass cs (SectionStatusMessage);
	
	StatusMessage = statusmessage;
}

wchar_t *CopyThreadClass::Get_Status_Message (WideStringClass &statusmessage)
{
	FastCriticalSectionClass::LockClass cs (SectionStatusMessage);
	
	statusmessage = StatusMessage;
	return (statusmessage.Peek_Buffer());
}


/***********************************************************************************************
 * CopyThreadClass::Set\Get_Error_Message --																	  *	
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
void CopyThreadClass::Set_Error_Message (const WideStringClass &errormessage)
{
	FastCriticalSectionClass::LockClass cs (SectionErrorMessage);
	
	ErrorMessage = errormessage;
}

wchar_t *CopyThreadClass::Get_Error_Message (WideStringClass &errormessage)
{
	FastCriticalSectionClass::LockClass cs (SectionErrorMessage);
	
	errormessage = ErrorMessage;
	return (errormessage.Peek_Buffer());
}


/***********************************************************************************************
 * CopyThreadClass::Set\Get_Status --																			  *	
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
void CopyThreadClass::Set_Status (StatusEnum status)
{
	FastCriticalSectionClass::LockClass cs (SectionStatus);
	
	Status = status;
}

CopyThreadClass::StatusEnum CopyThreadClass::Get_Status()
{
	FastCriticalSectionClass::LockClass cs (SectionStatus);
	
	return (Status);
}


/***********************************************************************************************
 * CopyThreadClass::Retry --																						  *	
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
bool CopyThreadClass::Retry()
{
	CopyThreadClass::StatusEnum status;

	Set_Status (STATUS_ERROR);
	while ((status = Get_Status()) == STATUS_ERROR) {
		Sleep (50);
	}
	return (status == STATUS_RETRY);
}



