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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/buildnum.cpp                        $*
 *                                                                                             *
 *                       Author:: Steve Tall                                                   *
 *                                                                                             *
 *                     $Modtime:: 10/29/01 10:06p                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "buildnum.h"
#include "wwdebug.h"
#include "gitinfo.h"
#include <stdio.h>
#include <string.h>

/***********************************************************************************************
 * BuildInfoClass::Get_Build_Number -- Gets the 32 bit build number.                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Build number                                                                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 4:54PM ST : Created                                                            *
 *=============================================================================================*/
unsigned BuildInfoClass::Get_Build_Number(void)
{
	return (GitRevision);
}

/***********************************************************************************************
 * BuildInfoClass::Get_Build_Number_String -- Gets the build number as a human readable string.*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to build number string                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 4:55PM ST : Created                                                            *
 *=============================================================================================*/
const char *BuildInfoClass::Get_Build_Number_String(void)
{
	static char _buffer[16];
	sprintf (_buffer, "%u", GitRevision);
	return (_buffer);
}

/***********************************************************************************************
 * BuildInfoClass::Get_Builder_Name -- Gets the name of the person who built this executable.  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to builder name string                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 5:08PM ST : Created                                                            *
 *=============================================================================================*/
const char *BuildInfoClass::Get_Builder_Name(void)
{
	return BuildAuthorName;
}

/***********************************************************************************************
 * BuildInfoClass::Get_Build_Date_String -- Gets the date this executable was built on.        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to build date string                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 5:10PM ST : Created                                                            *
 *=============================================================================================*/
const char *BuildInfoClass::Get_Build_Date_String(void)
{
	return(GitCommitDate);
}

/***********************************************************************************************
 * BuildInfoClass::Get_Build_Version_String -- Get composite version as string                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Version string                                                                    *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 7:30PM ST : Created                                                            *
 *=============================================================================================*/
const char *BuildInfoClass::Get_Build_Version_String(void)
{
	static char _buffer[128];
	sprintf(_buffer, "%u.%u.%u.%u", GitMajor, GitMinor, GitPatch, GitRevision);
	return(_buffer);
}

const char *BuildInfoClass::Get_Build_Commit_String(bool short_str)
{
	if (short_str)
		return(GitShortSHA1);
	else
		return(GitSHA1);
}

bool BuildInfoClass::Is_Build_Dirty(void)
{
	return(GitUncommittedChanges);
}

/***********************************************************************************************
 * BuildInfoClass::Get_Build_Type -- Get the type of build this is                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Build type                                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 7:40PM ST : Created                                                            *
 *=============================================================================================*/
BuildInfoClass::BuildType BuildInfoClass::Get_Build_Type(void)
{
	#ifndef _DEBUG
		#ifdef WWDEBUG
			return(BUILD_PROFILE);
		#else //WWDEBUG
			return(BUILD_RELEASE);
		#endif //WWDEBUG
	#else //_DEBUG
		return(BUILD_DEBUG);
	#endif //_DEBUG
}

/***********************************************************************************************
 * BuildInfoClass::Get_Build_Type_String -- Get the build type as a string.                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 7:42PM ST : Created                                                            *
 *=============================================================================================*/
const char *BuildInfoClass::Get_Build_Type_String(void)
{
	switch (Get_Build_Type()) {
		case BUILD_DEBUG:
			return("DEBUG");

		case BUILD_RELEASE:
			return("RELEASE");

		case BUILD_PROFILE:
			return("PROFILE");

		default:
			WWASSERT(false);
			return("UNKNOWN");
	}
}

/***********************************************************************************************
 * BuildInfoClass::Composite_Build_Info -- Get lots of build info                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to info                                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 7:49PM ST : Created                                                            *
 *=============================================================================================*/
const char *BuildInfoClass::Composite_Build_Info(void)
{
	static char _buffer[256];
	[[maybe_unused]] int written = snprintf(_buffer, sizeof(_buffer), "%s %s Commit %s%s at %s",
			BuildAuthorName,
			Get_Build_Type_String(),
			Is_Build_Dirty() ? "~" : "",
			GitShortSHA1,
			GitCommitDate);
	WWASSERT(unsigned(written) < sizeof(_buffer));
	return(_buffer);
}

/***********************************************************************************************
 * BuildInfoClass::Log_Build_Info -- Dump build info to the logfile.                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 7:49PM ST : Created                                                            *
 *=============================================================================================*/
void BuildInfoClass::Log_Build_Info(void)
{
	WWDEBUG_SAY((Composite_Build_Info()));
	WWDEBUG_SAY(("\n"));
}
