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
 *                     $Archive:: /Commando/Code/Installer/CopyThread.h $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/10/02 6:34p                 $* 
 *                                                                                             * 
 *                    $Revision:: 7                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _COPY_THREAD_H
#define _COPY_THREAD_H

// Includes.
#include "Thread.h"
#include "Win.h"
#include	"wwstring.h"
#include "widestring.h"


// Class to implement a worker thread that will execute the file copying process.
// NOTE: Thread usage will ensure that copying does not stall the copy dialog.
class CopyThreadClass : public ThreadClass 
{
	public:

		enum StatusEnum {
		
			STATUS_OK,
			STATUS_RETRY = STATUS_OK,
			STATUS_ERROR,
			STATUS_QUIT,
			STATUS_SUCCESS,
			STATUS_ABORTED,
			STATUS_FAILURE
		};

		 CopyThreadClass (int64_t bytestocopy);
		~CopyThreadClass();

		void					 Thread_Function();
		bool					 Can_Abort (bool lock);
		void					 Set_Abort (bool abort);
		bool					 Get_Abort (bool canabort);	
		bool					 Is_Aborting() {return (IsAborting);}
		void					 Add_Bytes_Copied (unsigned bytecount);
		float					 Get_Fraction_Complete();
		void					 Set_Target_Path (const WideStringClass &targetpath);
		wchar_t					*Get_Target_Path (WideStringClass &targetpath);
		void					 Set_Status_Message (const WideStringClass &targetpath);
		wchar_t					*Get_Status_Message (WideStringClass &targetpath);
		void					 Set_Error_Message (const WideStringClass &errormessage);
		wchar_t					*Get_Error_Message (WideStringClass &errormessage);
		void					 Set_Status (StatusEnum status);
		StatusEnum			 Get_Status();
		bool					 Retry();

		DynamicVectorClass <StringClass> &Get_Subdirectory_Log() {return (SubdirectoryLog);}
		DynamicVectorClass <StringClass> &Get_Filename_Log()		{return (FilenameLog);}

		static bool Replace_File (const FILETIME &sourcefiletime, DWORD sourcefilesize, const WideStringClass &targetpathname);
		static bool Replace_File (const WideStringClass &sourcepathname, const WideStringClass &targetpathname);

		static CopyThreadClass *_ActiveCopyThread;
		
	protected:
		
		void Copy_Directory (const WideStringClass &sourcepath, const WideStringClass &targetpath);
		void Copy_File (const WideStringClass &sourcepathname, const WideStringClass &targetpathname);

		int64_t										 BytesToCopy;
		bool											 IsAborting;

		// NOTE: Do not access the following variables directly. Instead use the Set/Get()
		//			functions because they are protected by critical sections.
		bool										    Abort;
		bool											 CanAbort;	
		int64_t										 BytesCopied;		// Bytes copied so far.
		WideStringClass							 TargetPath;
		WideStringClass							 StatusMessage;
		WideStringClass							 ErrorMessage;
		StatusEnum									 Status;

		FastCriticalSectionClass::LockClass *AbortLock;
		FastCriticalSectionClass				 SectionAbort;
		FastCriticalSectionClass				 SectionBytesCopied;
		FastCriticalSectionClass				 SectionTargetPath;
		FastCriticalSectionClass				 SectionStatusMessage;
		FastCriticalSectionClass				 SectionErrorMessage;
		FastCriticalSectionClass				 SectionStatus;

		DynamicVectorClass <StringClass>		 SubdirectoryLog;
		DynamicVectorClass <StringClass>		 FilenameLog;
};


#endif // COPY_THREAD_H