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
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwlib/trim.cpp                               $* 
 *                                                                                             * 
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             * 
 *                     $Modtime:: 11/08/01 11:35a                                             $*
 *                                                                                             * 
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"trim.h"
#include	<string.h>

#ifdef _UNIX
#include	<wctype.h>
#endif // _UNIX

/*********************************************************************************************** 
 * strtrim -- Trim leading and trailing white space off of string.                             * 
 *                                                                                             * 
 *    This routine will remove the leading and trailing whitespace from the string specifed.   * 
 *    The string is modified in place.                                                         * 
 *                                                                                             * 
 * INPUT:   buffer   -- Pointer to the string to be trimmed.                                   * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
char* strtrim(char* buffer)
{
	if (buffer) {
		/* Strip leading white space from the string. */
		char* source = buffer;

		while ((*source != 0) && ((unsigned char)*source <= 32)) {
			++source;
		}

		if (source != buffer) {
			strcpy(buffer, source);
		}

		/* Clip trailing white space from the string. */
		for (size_t index = ::strlen(buffer); index > 0; ) {
			const size_t char_index = index - 1;
			if ((*source != 0) && ((unsigned char)buffer[char_index] <= 32)) {
				buffer[char_index] = '\0';
				--index;
			} else {
				break;
			}
		}
	}

	return buffer;
}


unichar_t* u_strtrim(unichar_t* buffer)
{
	if (buffer) {
		/* Strip leading white space from the string. */
		unichar_t* source = buffer;

		while ((*source != 0) && ((unsigned int)*source <= 32)) {
			++source;
		}
		
		if (source != buffer) {
			u_strcpy(buffer, source);
		}

		/* Clip trailing white space from the string. */
		for (size_t index = ::u_strlen(buffer); index > 0; ) {
			const size_t char_index = index - 1;
			if ((*source != 0) && ((unsigned int)buffer[char_index] <= 32)) {
				buffer[char_index] = U_CHAR('\0');
				--index;
			} else {
				break;
			}
		}
	}

	return buffer;
}
