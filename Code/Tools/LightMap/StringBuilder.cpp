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
 *                     $Modtime:: 2/09/00 1:12p       $*
 *                                                                                             *
 *                    $Revision:: 5                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "StringBuilder.h"
#include <stdio.h>
#include <stdarg.h>


/***********************************************************************************************
 * StringBuilder::StringBuilder --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/27/99    IML : Created.                                                                 *
 *=============================================================================================*/
StringBuilder::StringBuilder (unsigned buffersize)
{
	Buffer = new char [buffersize];
	ASSERT (Buffer != NULL);
	Buffer [0] = '\0';
	BufferAllocated = true;
	BufferSize = buffersize;
}


/***********************************************************************************************
 * StringBuilder::StringBuilder --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/27/99    IML : Created.                                                                 *
 *=============================================================================================*/
StringBuilder::StringBuilder (char *buffer, unsigned buffersize)
{
	Buffer			 = buffer;
	Buffer [0]		 = '\0';
	BufferAllocated = false;
	BufferSize		 = buffersize;
}


/***********************************************************************************************
 * StringBuilder::~StringBuilder --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/27/99    IML : Created.                                                                 *
 *=============================================================================================*/
StringBuilder::~StringBuilder()
{
	if (BufferAllocated) delete [] Buffer;
}


/***********************************************************************************************
 * StringBuilder::Copy --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/27/99    IML : Created.                                                                 *
 *=============================================================================================*/
char *StringBuilder::Copy (const char *controlstring, ...)
{
	unsigned  messagebufferlength;
	char		*messagebuffer;

	// Allocate a workspace buffer.
	messagebufferlength = BufferSize;
	messagebuffer = new char [messagebufferlength];
	ASSERT (messagebuffer != NULL);

	va_list args;

	va_start (args, controlstring);

	_vsnprintf (messagebuffer, messagebufferlength - 1, controlstring, args);
	messagebuffer [messagebufferlength - 1] = '\0';
	strncpy (Buffer, messagebuffer, BufferSize - 1);
	Buffer [BufferSize - 1] = '\0';

	va_end (args);

	// Clean-up.
	delete messagebuffer;

	return (Buffer);
}


/***********************************************************************************************
 * StringBuilder::Concatenate --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/27/99    IML : Created.                                                                 *
 *=============================================================================================*/
char *StringBuilder::Concatenate (const char *controlstring, ...)
{
	unsigned  messagebufferlength;
	char		*messagebuffer;

	// Allocate a workspace buffer.
	messagebufferlength = BufferSize;
	messagebuffer = new char [messagebufferlength];
	ASSERT (messagebuffer != NULL);

	va_list args;

	va_start (args, controlstring);

	_vsnprintf (messagebuffer, messagebufferlength - 1, controlstring, args);
	messagebuffer [messagebufferlength - 1] = '\0';
	strncat (Buffer, messagebuffer, BufferSize - 1 - strlen (Buffer));
	Buffer [BufferSize - 1] = '\0';

	va_end (args);

	// Clean-up.
	delete messagebuffer;

	return (Buffer);
}
