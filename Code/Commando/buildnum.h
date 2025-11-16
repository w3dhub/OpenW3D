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
 *                     $Archive:: /Commando/Code/Commando/buildnum.h                          $*
 *                                                                                             *
 *                       Author:: Steve Tall                                                   *
 *                                                                                             *
 *                     $Modtime:: 10/29/01 9:58p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
**
** This is just a placeholder for the build number.
** A post build step will stamp the build number into here.
**
*/
class BuildInfoClass
{

	public:

		/*
		** Build types.
		*/
		typedef enum {
			BUILD_RELEASE,
			BUILD_DEBUG,
			BUILD_PROFILE
		} BuildType;

		/*
		** Gets the 32 bit build number.
		*/
		static unsigned int Get_Build_Number(void);

		/*
		** Gets the 32 bit build number as a human readable string.
		*/
		static const char *Get_Build_Number_String(void);

		/*
		** Gets the name of the person who built this executable.
		*/
		static const char *Get_Builder_Name(void);

		/*
		** Gets the date this executable was built on.
		*/
		static const char *Get_Build_Date_String(void);

		/*
		** Gets the initials of the person who built this executable.
		*/
		static const char *Get_Builder_Initials(void);

		/*
		** Get a composite build number string with the works.
		*/
		static const char *Get_Build_Version_String(void);

		/*
		** Get build type.
		*/
		static BuildType Get_Build_Type(void);
		static const char *Get_Build_Type_String(void);

		/*
		** Dump build info to the log file
		*/
		static void Log_Build_Info(void);

		/*
		** Lots of build info together.
		*/
		static const char *Composite_Build_Info(void);




	private:
		static char BuildNumber [64];
		static char BuildDate   [64];
};



