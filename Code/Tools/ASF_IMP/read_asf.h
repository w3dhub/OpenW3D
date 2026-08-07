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

//----------------------------------------------------------------------------
// File_Class
//----------------------------------------------------------------------------

class File_Class
{
public:

	FILE * fp;

	File_Class ( const char * filename, const char * mode )
	{
		fp = fopen ( filename, mode );
	}

	~File_Class ()
	{
		if ( fp != nullptr )
			fclose ( fp );
		fp = nullptr;
	}
};

//----------------------------------------------------------------------------
// ASF_Lexeme_Type
//----------------------------------------------------------------------------

enum ASF_Lexeme_Type
{
	EOF_MARKER,
	TOKEN,
	NEWLINE
};

//----------------------------------------------------------------------------
// ASF_Lexer
//----------------------------------------------------------------------------

#define MAX_TOKEN_LENGTH 512

class ASF_Lexer
{
public:

	ASF_Lexer ( const char * file_name );

	ASF_Lexeme_Type	type () const { return Current_type; }
	const char *	text () const { return Current_text; }

	void			advance ();

protected:

	void			skip_whitespace ();

	File_Class		Input_file;
	int				Current_char;

	ASF_Lexeme_Type	Current_type;
	char 			Current_text [ MAX_TOKEN_LENGTH ];
};

//----------------------------------------------------------------------------
// Bone_Class
//----------------------------------------------------------------------------

class Bone_Class
{
public:

	Bone_Class ():
		Name (""),
		Direction (Point3 (0,0,0)),
		Parent_bone (nullptr),
		Next_bone (nullptr),
		Length (0.0f)
	{
		Axis_tm.IdentityMatrix ();
	}

	~Bone_Class () { delete [] Name; }

	void set_name ( const char * name )
	{
		Name = new char [ strlen (name) + 1 ];
		strcpy ( Name, name );
	}

	void set_next_bone ( Bone_Class * next_bone )
	{
		Next_bone = next_bone;
	}

	void set_length ( float length ) { Length = length; }
	void set_direction ( Point3 direction ) { Direction = direction; }
	void set_axis_tm ( const Matrix3 & axis_tm ) { Axis_tm = axis_tm; }

	void add_axis ( Axis_Names axis ) { Active_axes.add_axis ( axis ); }

	void set_parent ( Bone_Class * parent_bone );

	const char * name () const { return Name; }
	Bone_Class * next_bone () const { return Next_bone; }

	void create_node
	(
		ImpInterface *	import_interface,
		Interface *		max_interface
	);

	void add_app_data ( int chunk_id, void * data, int data_size );

private:

	char *			Name;

	Point3			Direction;
	Matrix3			Axis_tm;

	Bone_Class *	Parent_bone;
	Bone_Class *	Next_bone;

	ImpNode *		Node;

	float			Length;

	ASF_Data_Chunk	Active_axes;
};

//----------------------------------------------------------------------------
// Skeleton_Class
//----------------------------------------------------------------------------

#define DEGREES_TO_RADIANS 1.7453293e-2f

class Skeleton_Class
{
public:

	Skeleton_Class
	(
		const char *	file_name,
		ImpInterface *,
		Interface *
	);

	~Skeleton_Class ();

protected:

	void add_bone_to_list ( Bone_Class * new_bone );

	Bone_Class * find_bone ( const char * name );

	void				parse_units_block ( ASF_Lexer & );
	void				parse_root_block ( ASF_Lexer & );
	void				parse_bonedata_block ( ASF_Lexer & );
	void				parse_hierarchy_block ( ASF_Lexer & );
	void				parse_hierarchy_line ( ASF_Lexer & );
	void				parse_bone ( ASF_Lexer & );
	void				skip_unrecognized_blocks ( ASF_Lexer & );
	void				skip_to_next_line ( ASF_Lexer & );
	void				match_newline ( ASF_Lexer & );
	void				match_token ( ASF_Lexer &, const char * );
	void				skip_token ( ASF_Lexer & );
	void				verify_token ( ASF_Lexer & );
	float				float_token ( ASF_Lexer & );

	float				Angle_multiplier; // Converts angle to radians.
	float				Length_multiplier;

	ImpInterface *		Import_interface;
	Interface *			Max_interface;

	Bone_Class *		First_bone;
};
