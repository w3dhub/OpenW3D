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

// SkeletonHack.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"



class HTreeHackClass
{
public:
	HTreeHackClass(void);
	~HTreeHackClass(void);

	void	Load_W3D(ChunkLoadClass & cload);
	void	Save_W3D(ChunkSaveClass & csave);

	int	Num_Pivots(void) { return Header.NumPivots; }

	// Skeleton Modification: moves a bone to the end of the array.  Use to make
	// skeletons compatible with existing anims by moving all new bones to the end of the array.
	void	Move_Bone_To_End(int bone_index);

protected:

	bool	read_pivots(ChunkLoadClass & cload);
	bool	read_pivot_fixups(ChunkLoadClass & cload);
	bool	write_pivots(ChunkSaveClass & csave);
	bool	write_pivot_fixups(ChunkSaveClass & csave);

	void	move_single_bone_to_end(unsigned int bone_index);
	int	find_first_bad_bone(void);

	W3dHierarchyStruct		Header;
	W3dPivotStruct *			Pivots;
	W3dPivotFixupStruct *	PivotFixups;
};



void Print_Usage(void)
{
	printf("USAGE: SkeletonHack <W3D Filename> <BoneIndex>\r\n");
	printf("WARNING: This program only works on w3d files that contain a skeleton *ONLY*\r\n");
	printf("WARNING: This program modifies the input file.\r\n");
}


int main(int argc, char* argv[])
{
	// See if we got the proper number of arguments
	if (argc != 3) {
		Print_Usage();
		return 0;
	}

	// Try to open the file,
	char * filename = argv[1];
	int bone_index = atoi(argv[2]);

	RawFileClass in_file(filename);
	if (!in_file.Is_Available()) {
		printf("Unable to open file: %s\r\n",filename);
		return 0;
	}


	// Open the file, read the first chunk, and verify that it is an HTree
	in_file.Open();
	ChunkLoadClass cload(&in_file);
	cload.Open_Chunk();
	if (cload.Cur_Chunk_ID() != W3D_CHUNK_HIERARCHY) {
		printf("Other chunks encountered in file: %s\r\n",filename);
		cload.Close_Chunk();
		in_file.Close();
		return 0;
	}


	// Read the hierarchy tree into memory and close the file.
	HTreeHackClass htree;
	htree.Load_W3D(cload);
	cload.Close_Chunk();
	in_file.Close();

	// Validate the bone_index parameter
	if ((bone_index < 0) || (bone_index >= htree.Num_Pivots())) {
		printf("Invalid bone index requested: %d\r\n",bone_index);
		return 0;
	}

	// Edit the HTree, moving the specified bone to the end of the tree
	htree.Move_Bone_To_End(bone_index);

	// Write the hierarchy tree out again.
	RawFileClass out_file(filename);
	if (out_file.Open(FileClass::WRITE) == false) {
		printf("Failed to open %s for writing.  Is the file read-only?\r\n",filename);
		return 0;
	}

	ChunkSaveClass csave(&out_file);
	csave.Begin_Chunk(W3D_CHUNK_HIERARCHY);
	htree.Save_W3D(csave);
	csave.End_Chunk();
	out_file.Close();

	printf("Moved bone %d and all of its children to the end of the bone array.\r\n",bone_index);
	return 0;
}



HTreeHackClass::HTreeHackClass(void) :
	Pivots(NULL),
	PivotFixups(NULL)
{
}

HTreeHackClass::~HTreeHackClass(void)
{
	delete[] Pivots;
	delete[] PivotFixups;
}

void HTreeHackClass::Load_W3D(ChunkLoadClass & cload)
{
	/*
	**	Read the first chunk, it should be the hierarchy header
	*/
	cload.Open_Chunk();
	assert(cload.Cur_Chunk_ID() == W3D_CHUNK_HIERARCHY_HEADER);
	cload.Read(&Header,sizeof(W3dHierarchyStruct));
	cload.Close_Chunk();

	/*
	** Allocate the array of pivots
	*/
	if (Header.NumPivots > 0) {
		Pivots = new W3dPivotStruct[Header.NumPivots];
		PivotFixups = new W3dPivotFixupStruct[Header.NumPivots];
	}

	/*
	** Now, read in all of the other chunks for this hierarchy.
	*/
	while (cload.Open_Chunk()) {

		switch (cload.Cur_Chunk_ID()) {

			case W3D_CHUNK_PIVOTS:
				read_pivots(cload);
				break;
			case W3D_CHUNK_PIVOT_FIXUPS:
				read_pivot_fixups(cload);
				break;
			default:
				break;
		}
		cload.Close_Chunk();
	}
}

bool HTreeHackClass::read_pivots(ChunkLoadClass & cload)
{
	for (unsigned int pidx=0; pidx < Header.NumPivots; pidx++) {
		if (cload.Read(&Pivots[pidx],sizeof(W3dPivotStruct)) != sizeof(W3dPivotStruct)) {
			return false;
		}
	}
	return true;
}

bool HTreeHackClass::read_pivot_fixups(ChunkLoadClass & cload)
{
	for (unsigned int pidx=0; pidx < Header.NumPivots; pidx++) {
		if (cload.Read(&PivotFixups[pidx],sizeof(W3dPivotFixupStruct)) != sizeof(W3dPivotFixupStruct)) {
			return false;
		}
	}
	return true;
}




void HTreeHackClass::Save_W3D(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(W3D_CHUNK_HIERARCHY_HEADER);
	csave.Write(&Header,sizeof(W3dHierarchyStruct));
	csave.End_Chunk();

	csave.Begin_Chunk(W3D_CHUNK_PIVOTS);
	write_pivots(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(W3D_CHUNK_PIVOT_FIXUPS);
	write_pivot_fixups(csave);
	csave.End_Chunk();
}

bool HTreeHackClass::write_pivots(ChunkSaveClass & csave)
{
	for (unsigned int pidx=0; pidx < Header.NumPivots; pidx++) {
		if (csave.Write(&Pivots[pidx],sizeof(W3dPivotStruct)) != sizeof(W3dPivotStruct)) {
			return false;
		}
	}
	return true;
}

bool HTreeHackClass::write_pivot_fixups(ChunkSaveClass & csave)
{
	for (unsigned int pidx=0; pidx < Header.NumPivots; pidx++) {
		if (csave.Write(&PivotFixups[pidx],sizeof(W3dPivotFixupStruct)) != sizeof(W3dPivotFixupStruct)) {
			return false;
		}
	}
	return true;
}

void HTreeHackClass::Move_Bone_To_End(int bone_index)
{
	move_single_bone_to_end(bone_index);

	int bad_bone = find_first_bad_bone();
	while (bad_bone != -1) {
		move_single_bone_to_end(bad_bone);
		bad_bone = find_first_bad_bone();
	}
}

void HTreeHackClass::move_single_bone_to_end(unsigned int bone_index)
{
	if (bone_index >= Header.NumPivots) {
		return;
	}

	// move entries at 'bone_index' to the ends of their arrays
	W3dPivotStruct tmp_pivot = Pivots[bone_index];
	W3dPivotFixupStruct tmp_fixup = PivotFixups[bone_index];

	for (unsigned int i=bone_index; i<Header.NumPivots-1; i++) {
		Pivots[i] = Pivots[i+1];
		PivotFixups[i] = PivotFixups[i+1];
	}
	Pivots[Header.NumPivots-1] = tmp_pivot;
	PivotFixups[Header.NumPivots-1] = tmp_fixup;

	// adjust all parent indices to reflect the change
	for (i=bone_index; i<Header.NumPivots-1; i++) {
		if (Pivots[i].ParentIdx > bone_index) {
			Pivots[i].ParentIdx--;
		} else if (Pivots[i].ParentIdx == bone_index) {
			Pivots[i].ParentIdx = Header.NumPivots-1;
		}
	}
}

int HTreeHackClass::find_first_bad_bone(void)
{
	for (unsigned int i=1; i<Header.NumPivots; i++) {
		if (Pivots[i].ParentIdx > i) {
			return i;
		}
	}
	return -1;
}
