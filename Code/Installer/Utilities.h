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
 *                     $Archive:: /Commando/Code/Installer/Utilities.h  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/11/02 10:18a                $* 
 *                                                                                             * 
 *                    $Revision:: 8                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _UTILITIES_H
#define _UTILITIES_H

// Includes.
#include "Vector.h"
#include "Win.h"


// Defines.
#define MAX_NUMBER_STRING_LENGTH		33		// Maximum length of string returned by ltoa().


// Forward declarations.
class StringClass;
class WideStringClass;


// Public prototypes.
bool	  Prompt_Install_CD (const WideStringClass &volumelabal, char &sourcedriveletter);
bool	  Validate_Install_CD (const WideStringClass &sourcepath, const WideStringClass &installvolumename);
bool	  Get_Disk_Space_Available (const WideStringClass &path, int64_t &diskspace);
int64_t Cluster_Padding (unsigned filecount);
bool	  Validate_Path (const WideStringClass &path, int &errorcode);
bool	  Valid_Install_Drive (const char *drive);
bool	  Create_Directory (const WideStringClass &path, DynamicVectorClass <StringClass> *log = NULL);
bool	  Is_Same_Path (const WideStringClass &path0, const WideStringClass &path1, bool standardize = true);
bool	  Is_Sub_Path (const WideStringClass &path0, const WideStringClass &path1, bool standardize = true);
wchar_t  *Remove_Trailing_Name (WideStringClass &path);
wchar_t  *Extract_Suffix_Root (WideStringClass &path, const WideStringClass &prefixpath);
wchar_t  *Extract_Trailing_Name (WideStringClass &path);
bool	  Directory_Exists (const WideStringClass &path);
bool	  Is_System_Directory (const WideStringClass &path);
void	  Get_Current_Directory (WideStringClass &path);
bool	  Generate_Temporary_Pathname (const WideStringClass &path, StringClass &multibytetemporarypathname);
void	  Message_Box (const WideStringClass &header, const WideStringClass &errormessage);

#endif // _UTILITIES_H