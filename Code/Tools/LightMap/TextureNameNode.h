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
 *                     $Modtime:: 11/13/99 10:26p     $*
 *                                                                                             *
 *                    $Revision:: 2                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _TEXTURENAMENODE_H
#define _TEXTURENAMENODE_H


class TextureNameNode
{
	public:
		TextureNameNode() {TextureName = NULL; Next = NULL;}
		TextureNameNode (const char *texturename)
		{
			ASSERT (texturename != NULL);
			TextureName = new char [strlen (texturename) + 1];
			ASSERT (TextureName != NULL);
			strcpy (TextureName, texturename);
			Next = NULL;
		}

		~TextureNameNode()
		{
			if (TextureName != NULL) delete [] TextureName;
		}

		// Public data.
		char				 *TextureName;
		TextureNameNode *Next;
};


#endif // _TEXTURENAMENODE_H
