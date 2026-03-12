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
 *                     $Archive:: /Commando/Code/Tools/ChunkView/ChunkFileImage.cpp           $*
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


#include "stdafx.h"
#include <assert.h>
#include "ChunkFileImage.h"
#include "rawfile.h"
#include "chunkio.h"



ChunkFileImageClass::ChunkFileImageClass(void) :
	RootNode(NULL)
{
}

ChunkFileImageClass::~ChunkFileImageClass(void)
{
	Reset();
}


void ChunkFileImageClass::Reset(void)
{
	// have to handle the fact that there isn't one root node,
	ChunkImageClass * node_to_delete = RootNode;
	while (node_to_delete) {
		ChunkImageClass * sibling = node_to_delete->Sibling;
		delete node_to_delete;
		node_to_delete = sibling;
	}
}

void ChunkFileImageClass::Load(const char * filename)
{
	RawFileClass file(filename);
	file.Open();
	ChunkLoadClass cload(&file);

	cload.Open_Chunk();
	RootNode = new ChunkImageClass;
	RootNode->Load(cload);
	cload.Close_Chunk();

	// while we have more top-level chunks, load them and link them
	// to the root through its sibling pointer
	while (cload.Open_Chunk()) {
		ChunkImageClass * root_sibling = new ChunkImageClass;
		root_sibling->Load(cload);
		RootNode->Add_Sibling(root_sibling);
		cload.Close_Chunk();
	}

	file.Close();
}


ChunkImageClass::ChunkImageClass(void) :
	Child(NULL),
	Sibling(NULL),
	Data(NULL),
	ID(0),
	Length(0)
{
}

ChunkImageClass::~ChunkImageClass(void)
{
	// Delete our children
	ChunkImageClass * child = Child;
	while (child) {
		ChunkImageClass * child_sibling = child->Sibling;
		delete child;
		child = child_sibling;
	}

	// Delete our data
	if (Data != NULL) {
		delete Data;
	}
}


void ChunkImageClass::Load(ChunkLoadClass & cload)
{
	ID = cload.Cur_Chunk_ID();
	Length = cload.Cur_Chunk_Length();

	if (cload.Contains_Chunks()) {

		while (cload.Open_Chunk()) {
			ChunkImageClass * new_child = new ChunkImageClass;
			new_child->Load(cload);
			Add_Child(new_child);
			cload.Close_Chunk();
		}

	} else {

		if (Length > 0) {
			Data = new uint8[Length];
			cload.Read(Data,Length);
		}

	}
}


void ChunkImageClass::Add_Child(ChunkImageClass * new_child)
{
	assert(new_child != NULL);
	assert(new_child->Sibling == NULL);

	// need to add to the tail...
	if (Child == NULL) {
		Child = new_child;
	} else {
		Child->Add_Sibling(new_child);
	}
}

void ChunkImageClass::Add_Sibling(ChunkImageClass * new_sibling)
{
	// Need to add to the tail so we display chunks in
	// the same order they appear in the file, so we
	// search for the tail.
	ChunkImageClass * tail = this;
	while (tail->Sibling != NULL) {
		tail = tail->Sibling;
	}
	tail->Sibling = new_sibling;
	assert(new_sibling->Sibling == NULL);
}


int ChunkImageClass::Get_Child_Count(void) const
{
	int count = 0;

	ChunkImageClass * child = Child;
	while (child != NULL) {
		count++;
		child = child->Sibling;
	}
	return count;
}

const ChunkImageClass * ChunkImageClass::Get_Child(int i) const
{
	int count = 0;

	ChunkImageClass * child = Child;
	while (child != NULL && count < i) {
		count++;
		child = child->Sibling;
	}

	assert(count == i);
	return child;
}

int ChunkImageClass::Get_Sibling_Count(void) const
{
	// NOTE: counts 'this' as a sibling.
	int count = 0;

	const ChunkImageClass * sibling = this;
	while (sibling != NULL) {
		count++;
		sibling = sibling->Sibling;
	}
	return count;
}

const ChunkImageClass * ChunkImageClass::Get_Sibling(int i) const
{
	// NOTE: sibling(0) is 'this' (just makes some other code simpler...)
	int count = 0;

	const ChunkImageClass * sibling = this;
	while (sibling != NULL && count < i) {
		count++;
		sibling = sibling->Sibling;
	}

	assert(count == i);
	return sibling;
}

