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
 *                 Project Name : ChunkView                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/ChunkView/ChunkFileImage.h             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/28/99 9:48a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef CHUNKFILEIMAGE_H
#define CHUNKFILEIMAGE_H


#include "bittype.h"

class ChunkLoadClass;
class ChunkImageClass;



class ChunkFileImageClass
{
public:

	ChunkFileImageClass(void);
	~ChunkFileImageClass(void);

	void									Reset(void);
	void									Load(const char * filename);
	const ChunkImageClass *			Get_Root(void) const								{ return RootNode; }

protected:

	ChunkImageClass *					RootNode;

};


class ChunkImageClass
{
public:

	ChunkImageClass(void);
	~ChunkImageClass(void);

	void									Load(ChunkLoadClass & cload);
	void									Add_Child(ChunkImageClass * child);	

	uint32								Get_ID(void) const								{ return ID; }
	uint32								Get_Length(void) const							{ return Length; }
	const uint8 *						Get_Data(void)	const								{ return Data; }

	int									Get_Child_Count(void) const;
	const ChunkImageClass *			Get_Child(int i) const;

	int									Get_Sibling_Count(void) const;
	const ChunkImageClass *			Get_Sibling(int i) const;

protected:

	void									Add_Sibling(ChunkImageClass * sibling);

	ChunkImageClass *					Child;
	ChunkImageClass *					Sibling;
	unsigned char *					Data;
	unsigned int						ID;
	unsigned int						Length;

	friend class ChunkFileImageClass;
};

#endif