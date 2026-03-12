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
 *                 Project Name : LightMap                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tool $*
 *                                                                                             *
 *                      $Author:: Ian_l               $*
 *                                                                                             *
 *                     $Modtime:: 10/17/99 9:31p      $*
 *                                                                                             *
 *                    $Revision:: 4                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef STRINGBUILDER_H
#define STRINGBUILDER_H

class StringBuilder {

	public:
		 StringBuilder (unsigned buffersize);
		 StringBuilder (char *buffer, unsigned buffersize);
		~StringBuilder ();

		char *String() {return (Buffer);}
		char *Copy (const char *controlstring, ...);
		char *Concatenate (const char *controlstring, ...);

	private:

		StringBuilder ();

		char		*Buffer;
		unsigned  BufferSize;
		bool		 BufferAllocated;
};


#endif // STRINGBUILDER_H

