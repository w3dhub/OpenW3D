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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/launcher/Protect.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Tom_s $
*
* VERSION INFO
*     $Modtime: 3/04/02 5:55p $
*     $Revision: 3 $
*
******************************************************************************/

#include "Protect.h"

#ifdef COPY_PROTECT

#include "BFish.h"
#include <Support\UString.h>
#include <Support\RefPtr.h>
#include <Storage\File.h>
#include <windows.h>
#include <memory>
#include <assert.h>
#include <Debug\DebugPrint.h>

#include "SafeDisk\CdaPfn.h"

#include "..\combat\specialbuilds.h"

// This GUID should be unique for each product. (CHANGE IT WHEN DOING THE
// NEXT PRODUCT) Note that the game will need to agree on this GUID also, so
// the game will have to be modified also.
const char* const LAUNCHER_GUID = "48BC11BD-C4D7-466b-8A31-C6ABBAD47B3E";

#ifdef OLDWAY
/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

Protect::Protect()
	: mLauncherMutex(NULL),
	  mMappedFile(NULL)
	{
	// Secure launcher mutex
	mLauncherMutex = CreateMutex(NULL, FALSE, LAUNCHER_GUID);

	if ((mLauncherMutex == NULL) ||
			((mLauncherMutex != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)))
		{
		return;
		}

	// Create memory mapped file to mirror protected file
	File file("Conquer.dat", Rights_ReadOnly);

	if (!file.IsAvailable())
		{
		return;
		}

	UInt32 fileSize = file.GetLength();

	SECURITY_ATTRIBUTES security;
	security.nLength = sizeof(security);
	security.lpSecurityDescriptor = NULL;
	security.bInheritHandle = TRUE;

	mMappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, &security,
		PAGE_READWRITE, 0, fileSize, NULL);

	if ((mMappedFile == NULL) ||
			((mMappedFile != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)))
		{
		PrintWin32Error("***** CreateFileMapping() Failed!");
	  CloseHandle(mMappedFile);
		mMappedFile = NULL;
		return;
		}

	// Map file to programs address space
	LPVOID mapAddress = MapViewOfFileEx(mMappedFile, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

	if (mapAddress == NULL)
		{
		PrintWin32Error("***** MapViewOfFileEx() Failed!");
		return;
		}

	// Decrypt protected file contents to mapping file
	void* buffer = NULL;
	UInt32 bufferSize = 0;
	file.Load(buffer, bufferSize);

	if ((buffer != NULL) && (bufferSize > 0))
		{
	 	// Retrieve protection key
		RefPtr<UString> passKey = GetPassKey();
		Char key[64];
		passKey->ConvertToANSI(key, sizeof(key));
		DebugPrint("Retrieved PassKey: %s\n", key);

		// Decrypt data
		BlowfishEngine blowfish;
		blowfish.Submit_Key(key, strlen(key));
		blowfish.Decrypt(buffer, bufferSize, mapAddress);

		DebugPrint("Decrypted data: %s\n", mapAddress);

		free(buffer);
		}

	UnmapViewOfFile(mapAddress);
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

Protect::~Protect()
	{
	if (mMappedFile != NULL)
		{
		CloseHandle(mMappedFile);
		mMappedFile = NULL;
		}

	if (mLauncherMutex != NULL)
		{
		CloseHandle(mLauncherMutex);
		mLauncherMutex = NULL;
		}
	}


/******************************************************************************
*
* NAME
*     Protect::SendMappedFileHandle()
*
* DESCRIPTION
*     Send the handle to the memory mapped file to the application.
*
* INPUTS
*     Process  - Process of the application to send handle to.
*     ThreadID - Thread of the applicatin to send handle to.
*
* RESULT
*     NONE
*
******************************************************************************/

void Protect::SendMappedFileHandle(HANDLE process, DWORD threadID) const
	{
	DebugPrint("SendMappedFileHandle()\n");

	DebugPrint("Creating running notification event.\n");
	const char* const protectGUID = "D6E7FC97-64F9-4d28-B52C-754EDF721C6F";
	HANDLE event = CreateEvent(NULL, FALSE, FALSE, protectGUID);

	if ((event == NULL) || ((event != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)))
		{
		PrintWin32Error("CreateEvent() Failed!");
		return;
		}

	DebugPrint("Waiting for game (timeout in %.02f seconds)...\n", ((float)((5 * 60) * 1000) * 0.001));

	#ifdef _DEBUG
	unsigned long start = timeGetTime();
	#endif

	HANDLE handles[2];
	handles[0] = event;
	handles[1] = process;
	DWORD waitResult = WaitForMultipleObjects(2, &handles[0], FALSE, ((5 * 60) * 1000));

	#ifdef _DEBUG
	unsigned long stop = timeGetTime();
	#endif

	DebugPrint("WaitResult = %ld (WAIT_OBJECT_0 = %ld)\n", waitResult, WAIT_OBJECT_0);

	if (waitResult == WAIT_OBJECT_0)
		{
		if (mMappedFile != NULL)
			{
			DebugPrint("Sending game the beef. (%lx)\n", mMappedFile);
			BOOL sent = PostThreadMessage(threadID, 0xBEEF, 0, (LPARAM)mMappedFile);
			assert(sent == TRUE);
			}
		}
	else
		{
		DebugPrint("***** Timeout!\n");
		}

	#ifdef _DEBUG
	DebugPrint("Waited %.02f seconds\n", ((float)(stop - start) * 0.001));
	#endif

	CloseHandle(event);
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<UString> Protect::GetPassKey(void) const
	{
	RefPtr<UString> passKey = new UString;

	if (passKey.IsValid())
		{
		unsigned char installPath[MAX_PATH];
		installPath[0] = '\0';

		// Get game information
		HKEY hKey;
		
		/*
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
			"Software\\Westwood\\Red Alert 2", 0, KEY_READ, &hKey);
		*/

#if	defined(FREEDEDICATEDSERVER)
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\RenegadeFDS", 0, KEY_READ, &hKey);
#elif defined(MULTIPLAYERDEMO)
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\RenegadeMPDemo", 0, KEY_READ, &hKey);
#elif defined(BETACLIENT)
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\RenegadeBeta", 0, KEY_READ, &hKey);
#else
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\Renegade", 0, KEY_READ, &hKey);
#endif

		if (result != ERROR_SUCCESS)
			{
			DebugPrint("***** Failed to open game registry key\n");
			assert(result == ERROR_SUCCESS);
			}

		if (result == ERROR_SUCCESS)
			{
			// Retrieve install path
			DWORD type;
			DWORD sizeOfBuffer = sizeof(installPath);
			result = RegQueryValueEx(hKey, "InstallPath", NULL, &type, installPath, &sizeOfBuffer);

			if (result != ERROR_SUCCESS)
				{
				DebugPrint("***** Failed to obtain game install path!\n");
				assert(result == ERROR_SUCCESS);
				}

			DebugPrint("Game install path: %s\n", installPath);

			if (strlen((const char*)installPath) == 0)
				{
				DebugPrint("***** Game install path invalid!\n");
				}

			// Retrieve Hard drive S/N
			char drive[8];
			_splitpath((const char*)installPath, drive, NULL, NULL, NULL);
			strcat(drive, "\\");

			DWORD volumeSerialNumber = 0;
			DWORD maxComponentLength;
			DWORD fileSystemFlags;
			BOOL volInfoSuccess = GetVolumeInformation((const char*)drive, NULL, 0,
				&volumeSerialNumber, &maxComponentLength, &fileSystemFlags, NULL, 0);

			if (volInfoSuccess == FALSE)
				{
				PrintWin32Error("GetVolumeInformation() Failed!");
				}

			DebugPrint("Drive Serial Number: %lx\n", volumeSerialNumber);

			// Add hard drive serial number portion
			char volumeSN[16];
			sprintf(volumeSN, "%lx-", volumeSerialNumber);
			*passKey += volumeSN;

			// Retrieve game serial #
			unsigned char gameSerialNumber[64];
			gameSerialNumber[0] = '\0';
			sizeOfBuffer = sizeof(gameSerialNumber);
			result = RegQueryValueEx(hKey, "Serial", NULL, &type, gameSerialNumber, &sizeOfBuffer);

			if (result != ERROR_SUCCESS)
				{
				DebugPrint("***** Failed to obtain windows serial number!\n");
				assert(result == ERROR_SUCCESS);
				}

			DebugPrint("Game serial number: %s\n", gameSerialNumber);

			if (strlen((const char*)gameSerialNumber) == 0)
				{
				DebugPrint("***** Game serial number invalid!\n");
				}

			RegCloseKey(hKey);

			// Add game serial number portion
			*passKey += (char*)gameSerialNumber;
			}

		// Obtain windows product ID
		result = RegOpenKeyEx(HKEY_CURRENT_USER,
			"Software\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hKey);

		if (result != ERROR_SUCCESS)
			{
			DebugPrint("***** Failed to open windows registry key!\n");
			assert(result == ERROR_SUCCESS);
			}

		if (result == ERROR_SUCCESS)
			{
			// Retrieve Windows Product ID
			unsigned char winProductID[64];
			winProductID[0] = '\0';

			DWORD type;
			DWORD sizeOfBuffer = sizeof(winProductID);
			result = RegQueryValueEx(hKey, "ProductID", NULL, &type, winProductID, &sizeOfBuffer);

			if (result != ERROR_SUCCESS)
				{
				DebugPrint("***** Failed to obtain windows product ID!\n");
				assert(result == ERROR_SUCCESS);
				}

			DebugPrint("Windows Product ID: %s\n", winProductID);

			if (strlen((const char*)winProductID) == 0)
				{
				DebugPrint("***** Invalid windows product ID\n");
				}

			RegCloseKey(hKey);

			// Add windows product ID portion
			*passKey += "-";
			*passKey += (char*)winProductID;
			}
		}

	return passKey;
	}


#else

HANDLE mLauncherMutex = NULL;
HANDLE mMappedFile = NULL;

void InitializeProtect(void)
	{
	ShutdownProtect();

	DebugPrint("Initializing protection\n");

	mLauncherMutex = NULL;
	mMappedFile = NULL;

	// Secure launcher mutex
	mLauncherMutex = CreateMutex(NULL, FALSE, LAUNCHER_GUID);

	if ((mLauncherMutex == NULL) || (mLauncherMutex && (GetLastError() == ERROR_ALREADY_EXISTS)))
		{
		DebugPrint("***** Failed to create launcher mutex\n");
		return;
		}

	// Create memory mapped file to mirror protected file
	File file("Conquer.dat", Rights_ReadOnly);

	if (!file.IsAvailable())
		{
		DebugPrint("***** Unable to find Conquer.dat\n");
		return;
		}

	UInt32 fileSize = file.GetLength();

	SECURITY_ATTRIBUTES security;
	security.nLength = sizeof(security);
	security.lpSecurityDescriptor = NULL;
	security.bInheritHandle = TRUE;

	mMappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, &security, PAGE_READWRITE, 0, fileSize, NULL);

	if ((mMappedFile == NULL) || (mMappedFile && (GetLastError() == ERROR_ALREADY_EXISTS)))
		{
		PrintWin32Error("***** CreateFileMapping() Failed!");
	  CloseHandle(mMappedFile);
		mMappedFile = NULL;
		return;
		}
	}



CDAPFN_DECLARE_GLOBAL(SendProtectMessage, CDAPFN_OVERHEAD_L5, CDAPFN_CONSTRAINT_NONE);

void SendProtectMessage(HANDLE process, DWORD threadID)
	{
	// Decrypt protected file contents to mapping file
	File file("Conquer.dat", Rights_ReadOnly);

	if (!file.IsAvailable())
		{
		DebugPrint("***** Unable to find Conquer.dat\n");
		return;
		}

	// Map file to programs address space
	LPVOID mapAddress = MapViewOfFileEx(mMappedFile, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

	if (mapAddress == NULL)
		{
		PrintWin32Error("***** MapViewOfFileEx() Failed!");
		return;
		}

	void* buffer = NULL;
	UInt32 bufferSize = 0;
	file.Load(buffer, bufferSize);

	if (buffer && (bufferSize > 0))
		{
		DebugPrint("Generating PassKey\n");

		// Generate passkey
		char passKey[64];
		passKey[0] = '\0';

		// Get game information
		HKEY hKey;

/*
#ifdef FREEDEDICATEDSERVER
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\RenegadeFDS", 0, KEY_READ, &hKey);
#else  //FREEDEDICATEDSERVER
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\Renegade", 0, KEY_READ, &hKey);
#endif //FREEDEDICATEDSERVER
*/

#if	defined(FREEDEDICATEDSERVER)
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\RenegadeFDS", 0, KEY_READ, &hKey);
#elif defined(MULTIPLAYERDEMO)
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\RenegadeMPDemo", 0, KEY_READ, &hKey);
#elif defined(BETACLIENT)
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\RenegadeBeta", 0, KEY_READ, &hKey);
#else
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Westwood\\Renegade", 0, KEY_READ, &hKey);
#endif

		assert((result == ERROR_SUCCESS) && "Failed to open game registry key");

		if (result == ERROR_SUCCESS)
			{
			// Retrieve install path
			unsigned char installPath[MAX_PATH];
			DWORD type;
			DWORD sizeOfBuffer = sizeof(installPath);
			result = RegQueryValueEx(hKey, "InstallPath", NULL, &type, installPath, &sizeOfBuffer);

			assert((result == ERROR_SUCCESS) && "Failed to obtain game install path!");
			assert((strlen((const char*)installPath) > 0) && "Game install path invalid!");
			DebugPrint("Game install path: %s\n", installPath);

			// Retrieve Hard drive S/N
			char drive[8];
			_splitpath((const char*)installPath, drive, NULL, NULL, NULL);
			strcat(drive, "\\");

			DWORD volumeSerialNumber = 0;
			DWORD maxComponentLength;
			DWORD fileSystemFlags;
			BOOL volInfoSuccess = GetVolumeInformation((const char*)drive, NULL, 0,
					&volumeSerialNumber, &maxComponentLength, &fileSystemFlags, NULL, 0);

			if (volInfoSuccess == FALSE)
				{
				PrintWin32Error("***** GetVolumeInformation() Failed!");
				}

			DebugPrint("Drive Serial Number: %lx\n", volumeSerialNumber);

			// Add hard drive serial number portion
			char volumeSN[16];
			sprintf(volumeSN, "%lx-", volumeSerialNumber);
			strcat(passKey, volumeSN);

			// Retrieve game serial #
			unsigned char gameSerialNumber[64];
			gameSerialNumber[0] = '\0';
			sizeOfBuffer = sizeof(gameSerialNumber);
			result = RegQueryValueEx(hKey, "Serial", NULL, &type, gameSerialNumber, &sizeOfBuffer);

			assert((result == ERROR_SUCCESS) && "Failed to obtain windows serial number!");
			assert((strlen((const char*)gameSerialNumber) > 0) && "Game serial number invalid!");
			DebugPrint("Game serial number: %s\n", gameSerialNumber);

			RegCloseKey(hKey);

			// Add game serial number portion
			strcat(passKey, (char*)gameSerialNumber);
			}

		// Obtain windows product ID
		result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hKey);
		assert((result == ERROR_SUCCESS) && "Failed to open windows registry key!");

		if (result == ERROR_SUCCESS)
			{
			// Retrieve Windows Product ID
			unsigned char winProductID[64];
			winProductID[0] = '\0';

			DWORD type;
			DWORD sizeOfBuffer = sizeof(winProductID);
			result = RegQueryValueEx(hKey, "ProductID", NULL, &type, winProductID, &sizeOfBuffer);

			assert((result == ERROR_SUCCESS) && "Failed to obtain windows product ID!");
			assert((strlen((const char*)winProductID) > 0) && "Invalid windows product ID");

			DebugPrint("Windows Product ID: %s\n", winProductID);

			RegCloseKey(hKey);

			// Add windows product ID portion
			strcat(passKey, "-");
			strcat(passKey, (char*)winProductID);
			}

		DebugPrint("Retrieved PassKey: %s\n", passKey);

		// Decrypt protected data into the memory mapped file
		BlowfishEngine blowfish;
		blowfish.Submit_Key(passKey, strlen(passKey));
		blowfish.Decrypt(buffer, bufferSize, mapAddress);

		DebugPrint("Decrypted data: %s\n", mapAddress);

		free(buffer);
		}

	UnmapViewOfFile(mapAddress);

	//---------------------------------------------------------------------------
	// Send protection message
	//---------------------------------------------------------------------------
	DebugPrint("Sending protect message\n");

	DebugPrint("Creating running notification event.\n");
	const char* const protectGUID = "D6E7FC97-64F9-4d28-B52C-754EDF721C6F";
	HANDLE event = CreateEvent(NULL, FALSE, FALSE, protectGUID);

	if ((event == NULL) || (event && (GetLastError() == ERROR_ALREADY_EXISTS)))
		{
		PrintWin32Error("***** CreateEvent() Failed!");
		return;
		}

	DebugPrint("Waiting for game (timeout in %.02f seconds)...\n", ((float)((5 * 60) * 1000) * 0.001));

	#ifdef _DEBUG
	unsigned long start = timeGetTime();
	#endif

	HANDLE handles[2];
	handles[0] = event;
	handles[1] = process;
	DWORD waitResult = WaitForMultipleObjects(2, &handles[0], FALSE, ((5 * 60) * 1000));

	#ifdef _DEBUG
	unsigned long stop = timeGetTime();
	#endif

	DebugPrint("WaitResult = %ld (WAIT_OBJECT_0 = %ld)\n", waitResult, WAIT_OBJECT_0);

	if (waitResult == WAIT_OBJECT_0)
		{
		if (mMappedFile != NULL)
			{
			DebugPrint("Sending game the beef. (%lx)\n", mMappedFile);
			BOOL sent = PostThreadMessage(threadID, 0xBEEF, 0, (LPARAM)mMappedFile);
			assert(sent == TRUE);
			}
		}
	else
		{
		DebugPrint("***** Timeout!\n");
		}

	#ifdef _DEBUG
	DebugPrint("Waited %.02f seconds\n", ((float)(stop - start) * 0.001));
	#endif

	CloseHandle(event);
	CDAPFN_ENDMARK(SendProtectMessage);
	}


void ShutdownProtect(void)
	{
	if (mMappedFile)
		{
		CloseHandle(mMappedFile);
		mMappedFile = NULL;
		}

	if (mLauncherMutex)
		{
		CloseHandle(mLauncherMutex);
		mLauncherMutex = NULL;
		}
	}

#endif

#endif // COPY_PROTECT