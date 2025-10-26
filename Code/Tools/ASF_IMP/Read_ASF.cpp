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

#include <Max.h>
#include <istdplug.h>

#include "asf_data.h"
#include "read_asf.h"
#include "exception.h"

//----------------------------------------------------------------------------
// ASF_Lexer::ASF_Lexer
//----------------------------------------------------------------------------

ASF_Lexer::ASF_Lexer
(
	const char * file_name
):
	Input_file ( file_name, "r" )
{
	Current_char = tolower ( fgetc ( Input_file.fp ) );
	advance ();
}

//----------------------------------------------------------------------------
// ASF_Lexer::skip_whitespace
//----------------------------------------------------------------------------
// Skip whitespace and comments, leaving the Current_char pointing to the
// first non-whitespace character in the input stream.
//----------------------------------------------------------------------------

void ASF_Lexer::skip_whitespace ()
{
	while (1)
	{
		if ( Current_char == '#' ) // Strip out comments
		{
			while ( Current_char != '\n' && Current_char != EOF )
				Current_char = tolower ( fgetc ( Input_file.fp ) );
		}
		else if ( (isspace ( Current_char ) && Current_char != '\n') ||
			Current_char == ',' ||
			Current_char == '(' ||
			Current_char == ')' )
		{
			Current_char = tolower ( fgetc ( Input_file.fp ) );
		}
		else
		{
			break;
		}
	}
}

//----------------------------------------------------------------------------
// ASF_Lexer::advance
//----------------------------------------------------------------------------
// Advance to the next token in the input stream.
//----------------------------------------------------------------------------

void ASF_Lexer::advance ()
{
	skip_whitespace ();

	if ( Current_char == EOF )
	{
		Current_type = EOF_MARKER;
		strcpy ( Current_text, "<end of file>" );
		return;
	}

	if ( Current_char == '\n' )
	{
		Current_type = NEWLINE;
		strcpy ( Current_text, "<newline>" );
		Current_char = tolower ( fgetc ( Input_file.fp ) );
		return;
	}

	Current_type = TOKEN;

	// Read characters into the token text buffer.

	int i = 0;

	while ( ! isspace (Current_char) &&
		Current_char != '#' &&
		Current_char != ',' &&
		Current_char != '(' &&
		Current_char != ')' &&
		Current_char != EOF )
	{
		if ( i >= (MAX_TOKEN_LENGTH - 1) )
		{
			// Abort -- maximum token length exceeded.

			throw Parse_Error ( "Maximum token length exceeded." );
		}

		Current_text [i] = Current_char;
		++ i;

		Current_char = tolower ( fgetc ( Input_file.fp ) );
	}

	Current_text [i] = '\0';
}

//----------------------------------------------------------------------------
// Skeleton_Class::Skeleton_Class
//----------------------------------------------------------------------------

Skeleton_Class::Skeleton_Class
(
	const char *	file_name,
	ImpInterface *	iface,
	Interface *		gi
) :
	Angle_multiplier (DEGREES_TO_RADIANS),
	Length_multiplier (1.0f),
	First_bone (NULL),
	Import_interface (iface),
	Max_interface (gi)
{
	ASF_Lexer lexer ( file_name );

	parse_units_block		( lexer );
	parse_root_block		( lexer );
	parse_bonedata_block	( lexer );
	parse_hierarchy_block	( lexer );
}

//----------------------------------------------------------------------------
// Skeleton_Class::~Skeleton_Class
//----------------------------------------------------------------------------

Skeleton_Class::~Skeleton_Class ()
{
	Bone_Class * p = First_bone;

	while ( p != NULL )
	{
		Bone_Class * delete_p = p;
		p = p->next_bone ();

		delete delete_p;
	}
}

//----------------------------------------------------------------------------
// Skeleton_Class::parse_units_block
//----------------------------------------------------------------------------

void Skeleton_Class::parse_units_block ( ASF_Lexer & lexer )
{
	skip_unrecognized_blocks ( lexer );

	match_token ( lexer, ":units" );
	match_newline ( lexer );

	BOOL mass_defined   = false;
	BOOL length_defined = false;
	BOOL angle_defined  = false;

	while (1)
	{
		verify_token ( lexer );

		if ( strcmp ( lexer.text (), "mass" ) == 0 )
		{
			if ( mass_defined )
				throw Parse_Error ( "Multiple mass definitions." );

			lexer.advance ();
			skip_token ( lexer ); // Ignore the mass definition.
			match_newline ( lexer );

			mass_defined = true;
		}
		else if ( strcmp ( lexer.text (), "length" ) == 0 )
		{
			if ( length_defined )
				throw Parse_Error ( "Multiple length definitions." );

			lexer.advance ();
			verify_token ( lexer );
			Length_multiplier = 1.0f / float_token ( lexer );
			match_newline ( lexer );

			length_defined = true;
		}
		else if ( strcmp ( lexer.text (), "angle" ) == 0 )
		{
			if ( angle_defined )
				throw Parse_Error ( "Multiple angle definitions." );

			lexer.advance ();
			verify_token ( lexer );
			if ( strcmp ( lexer.text (), "deg" ) == 0 )
				Angle_multiplier = DEGREES_TO_RADIANS;
			else if ( strcmp ( lexer.text (), "rad" ) == 0 )
				Angle_multiplier = 1.0f;
			else
				throw Parse_Error
					( "\"deg\" or \"rad\" expected after angle keyword." );

			lexer.advance ();
			match_newline ( lexer );

			angle_defined = true;
		}
		else
		{
			break;
		}
	}
}

//----------------------------------------------------------------------------
// Skeleton_Class::parse_root_block
//----------------------------------------------------------------------------

void Skeleton_Class::parse_root_block ( ASF_Lexer & lexer )
{
	skip_unrecognized_blocks ( lexer );

	match_token ( lexer, ":root" );
	match_newline ( lexer );

	// Create a root bone.

	Bone_Class * new_bone = new Bone_Class;
	new_bone->set_name ( "root" );

	// The rotation order for orientation offset.

	match_token ( lexer, "axis" );
	skip_token ( lexer ); // &&& get rotation order for orientation offset.
	match_newline ( lexer );

	// The order of transformations for root.

	match_token ( lexer, "order" );
	for ( int i = 0; i < 6; ++ i )
	{
		verify_token ( lexer );

		if ( strcmp ( lexer.text (), "rx" ) == 0 )
			new_bone->add_axis ( ROTATE_X );
		else if ( strcmp ( lexer.text (), "ry" ) == 0 )
			new_bone->add_axis ( ROTATE_Y );
		else if ( strcmp ( lexer.text (), "rz" ) == 0 )
			new_bone->add_axis ( ROTATE_Z );
		else if ( strcmp ( lexer.text (), "tx" ) == 0 )
			new_bone->add_axis ( TRANSLATE_X );
		else if ( strcmp ( lexer.text (), "ty" ) == 0 )
			new_bone->add_axis ( TRANSLATE_Y );
		else if ( strcmp ( lexer.text (), "tz" ) == 0 )
			new_bone->add_axis ( TRANSLATE_Z );
		else
			throw Parse_Error ( "Unrecognized order token in :root." );

		lexer.advance ();
	}
	match_newline ( lexer );

	// Translation data for root node.

	match_token ( lexer, "position" );
	skip_token ( lexer ); // &&&
	skip_token ( lexer ); // &&&
	skip_token ( lexer ); // &&&
	match_newline ( lexer );

	// Rotation data to orient the skeleton.

	match_token ( lexer, "orientation" );
	float rot0 = float_token ( lexer ) * Angle_multiplier;
	float rot1 = float_token ( lexer ) * Angle_multiplier;
	float rot2 = float_token ( lexer ) * Angle_multiplier;
	match_newline ( lexer );

	Matrix3 axis_tm;
	axis_tm.IdentityMatrix ();
	axis_tm.RotateX ( rot0 );
	axis_tm.RotateY ( rot1 );
	axis_tm.RotateZ ( rot2 );
	new_bone->set_axis_tm ( axis_tm );

	new_bone->set_direction ( Point3 (0,0,0) );
	new_bone->set_length ( 0.0f );

	add_bone_to_list ( new_bone );
	new_bone->create_node ( Import_interface, Max_interface );

	// Add an AppData chunk to the root node to indicate how much to scale
	// its position keys.
	Position_Key_Scale_Chunk * data_p = (Position_Key_Scale_Chunk *)
		malloc ( sizeof (Position_Key_Scale_Chunk) );

	data_p->Position_Key_Scale = Length_multiplier;

	new_bone->add_app_data ( 2, data_p, sizeof (Position_Key_Scale_Chunk) );
}

//----------------------------------------------------------------------------
// Skeleton_Class::parse_bonedata_block
//----------------------------------------------------------------------------

void Skeleton_Class::parse_bonedata_block ( ASF_Lexer & lexer )
{
	skip_unrecognized_blocks ( lexer );

	match_token ( lexer, ":bonedata" );
	match_newline ( lexer );

	// Parse bone definition blocks.

	while (1)
	{
		verify_token ( lexer );
		if ( strcmp ( lexer.text (), "begin" ) == 0 )
		{
			parse_bone ( lexer );
		}
		else
		{
			break;
		}
	}
}

//----------------------------------------------------------------------------
// Skeleton_Class::parse_hierarchy_block
//----------------------------------------------------------------------------

void Skeleton_Class::parse_hierarchy_block ( ASF_Lexer & lexer )
{
	skip_unrecognized_blocks ( lexer );

	match_token ( lexer, ":hierarchy" );
	match_newline ( lexer );

	match_token ( lexer, "begin" );
	match_newline ( lexer );

	while (1)
	{
		verify_token ( lexer );
		if ( strcmp ( lexer.text (), "end" ) == 0 )
		{
			break;
		}
		else
		{
			parse_hierarchy_line ( lexer );
		}
	}

	lexer.advance ();
	match_newline ( lexer );
}

//----------------------------------------------------------------------------
// Skeleton_Class::parse_hierarchy_line
//----------------------------------------------------------------------------

void Skeleton_Class::parse_hierarchy_line ( ASF_Lexer & lexer )
{
	verify_token ( lexer );
	Bone_Class * parent_bone = find_bone ( lexer.text () );
	if ( parent_bone == NULL )
		throw Parse_Error ( "Undefined parent bone." );
	lexer.advance ();

	while ( lexer.type () == TOKEN )
	{
		Bone_Class * child_bone = find_bone ( lexer.text () );
		if ( child_bone == NULL )
			throw Parse_Error ( "Undefined child bone." );

		child_bone->set_parent ( parent_bone );

		lexer.advance ();
	}

	match_newline ( lexer );
}

//----------------------------------------------------------------------------
// Skeleton_Class::find_bone
//----------------------------------------------------------------------------

Bone_Class * Skeleton_Class::find_bone ( const char * name )
{
	Bone_Class * p = First_bone;

	while ( p != NULL )
	{
		if ( strcmp ( name, p->name () ) == 0 )
			break;

		p = p->next_bone ();
	}

	return p;
}

//----------------------------------------------------------------------------
// lh_to_rh
//----------------------------------------------------------------------------

static Point3 lh_to_rh ( Point3 point )
{
	Point3 new_point;

	new_point.x =  point.x;
	new_point.y = -point.z;
	new_point.z =  point.y;

	return  new_point;
}

//----------------------------------------------------------------------------
// Skeleton_Class::parse_bone
//----------------------------------------------------------------------------

void Skeleton_Class::parse_bone ( ASF_Lexer & lexer )
{
	match_token ( lexer, "begin" );
	match_newline ( lexer );

	// Create a new bone object.

	Bone_Class * new_bone = new Bone_Class;

	// Optional ID number.

	verify_token ( lexer );
	if ( strcmp ( lexer.text (), "id" ) == 0 )
	{
		lexer.advance ();
		skip_token ( lexer ); // Ignore the bone ID number for now.
		match_newline ( lexer );
	}

	// Name.

	match_token ( lexer, "name" );
	verify_token ( lexer );
	new_bone->set_name ( lexer.text () );
	lexer.advance ();
	match_newline ( lexer );

	// Direction vector.

	match_token ( lexer, "direction" );
	Point3 direction;
	direction.x = float_token ( lexer );
	direction.y = float_token ( lexer );
	direction.z = float_token ( lexer );
	new_bone->set_direction ( direction );
	match_newline ( lexer );

	// Length.

	match_token ( lexer, "length" );
	new_bone->set_length ( float_token ( lexer ) * Length_multiplier / (float)GetMasterScale(UNITS_INCHES));
	match_newline ( lexer );

	// Rotation axis in world coordinates, with order of rotations.

	match_token ( lexer, "axis" );
	float rot0 = float_token ( lexer ) * Angle_multiplier;
	float rot1 = float_token ( lexer ) * Angle_multiplier;
	float rot2 = float_token ( lexer ) * Angle_multiplier;

	// &&& Ultimately this should handle any order of rotations.
	match_token ( lexer, "xyz" );

	match_newline ( lexer );

	Matrix3 axis_tm;
	axis_tm.IdentityMatrix ();
	axis_tm.RotateX ( rot0 );
	axis_tm.RotateY ( rot1 );
	axis_tm.RotateZ ( rot2 );
	new_bone->set_axis_tm ( axis_tm );

	// Optional mass of skinbody associated with this bone.

	verify_token ( lexer );
	if ( strcmp ( lexer.text (), "bodymass" ) == 0 )
	{
		lexer.advance ();
		skip_token ( lexer ); // Ignore the bodymass.
		match_newline ( lexer );
	}

	// Optional position of center of mass along the bone.

	verify_token ( lexer );
	if ( strcmp ( lexer.text (), "cofmass" ) == 0 )
	{
		lexer.advance ();
		skip_token ( lexer ); // Ignore the center of mass position.
		match_newline ( lexer );
	}

	// Optional degrees of freedom.

	verify_token ( lexer );
	if ( strcmp ( lexer.text (), "dof" ) == 0 )
	{
		lexer.advance ();

		if ( lexer.type () == TOKEN && strcmp ( lexer.text (), "rx" ) == 0 )
		{
			new_bone->add_axis ( ROTATE_X );
			lexer.advance ();
		}

		if ( lexer.type () == TOKEN && strcmp ( lexer.text (), "ry" ) == 0 )
		{
			new_bone->add_axis ( ROTATE_Y );
			lexer.advance ();
		}

		if ( lexer.type () == TOKEN && strcmp ( lexer.text (), "rz" ) == 0 )
		{
			new_bone->add_axis ( ROTATE_Z );
			lexer.advance ();
		}

		if ( lexer.type () == TOKEN && strcmp ( lexer.text (), "l" ) == 0 )
		{
			new_bone->add_axis ( TRANSLATE_LENGTH );
			lexer.advance ();
		}

		match_newline ( lexer );

		// Limits for the given degrees of freedom.

		match_token ( lexer, "limits" );

		// &&&

		while ( strcmp ( lexer.text (), "end" ) != 0 )
			lexer.advance ();
	}

	match_token ( lexer, "end" );
	match_newline ( lexer );

	// Add the bone to the list of bones.

	add_bone_to_list ( new_bone );

	new_bone->create_node ( Import_interface, Max_interface );
}

//----------------------------------------------------------------------------
// Skeleton_Class::skip_unrecognized_blocks
//----------------------------------------------------------------------------
// It is assumed that the current lexeme is the first on a line.
//----------------------------------------------------------------------------

void Skeleton_Class::skip_unrecognized_blocks
(
	ASF_Lexer &		lexer
)
{
	while (1)
	{
		if ( lexer.type () == EOF_MARKER )
			break;
		if ( strcmp ( lexer.text (), ":units" ) == 0 )
			break;
		if ( strcmp ( lexer.text (), ":root" ) == 0 )
			break;
		if ( strcmp ( lexer.text (), ":bonedata" ) == 0 )
			break;
		if ( strcmp ( lexer.text (), ":hierarchy" ) == 0 )
			break;

		skip_to_next_line ( lexer );
	}
}

//----------------------------------------------------------------------------
// Skeleton_Class::skip_to_next_line
//----------------------------------------------------------------------------

void Skeleton_Class::skip_to_next_line
(
	ASF_Lexer &		lexer
)
{
	while ( lexer.type () != NEWLINE &&
		lexer.type () != EOF_MARKER )
		lexer.advance ();

	if ( lexer.type () == NEWLINE )
		lexer.advance ();
}

//----------------------------------------------------------------------------
// Skeleton_Class::match_newline
//----------------------------------------------------------------------------

void Skeleton_Class::match_newline
(
	ASF_Lexer &		lexer
)
{
	char message_buffer [ 512 ];

	if ( lexer.type () != NEWLINE )
	{
		sprintf ( message_buffer, "Expected newline; found \"%s\" instead.",
			lexer.text () );
		throw Parse_Error ( message_buffer );
	}

	lexer.advance ();
}

//----------------------------------------------------------------------------
// Skeleton_Class::verify_token
//----------------------------------------------------------------------------

void Skeleton_Class::verify_token
(
	ASF_Lexer &		lexer
)
{
	char message_buffer [ 512 ];

	if ( lexer.type () != TOKEN )
	{
		sprintf ( message_buffer, "Expected token; found \"%s\" instead.",
			lexer.text () );
		throw Parse_Error ( message_buffer );
	}
}

//----------------------------------------------------------------------------
// Skeleton_Class::skip_token
//----------------------------------------------------------------------------

void Skeleton_Class::skip_token
(
	ASF_Lexer &		lexer
)
{
	verify_token ( lexer );
	lexer.advance ();
}

//----------------------------------------------------------------------------
// Skeleton_Class::float_token
//----------------------------------------------------------------------------

float Skeleton_Class::float_token
(
	ASF_Lexer &		lexer
)
{
	verify_token ( lexer );
	float value = (float) strtod ( lexer.text (), NULL );
	lexer.advance ();
	return value;
}

//----------------------------------------------------------------------------
// Skeleton_Class::match_token
//----------------------------------------------------------------------------

void Skeleton_Class::match_token
(
	ASF_Lexer &		lexer,
	const char *	token_text
)
{
	char message_buffer [ 512 ];

	verify_token ( lexer );

	if ( strcmp ( lexer.text (), token_text ) != 0 )
	{
		sprintf ( message_buffer, "Expected \"%s\"; found \"%s\" instead.",
			token_text, lexer.text () );
		throw Parse_Error ( message_buffer );
	}

	lexer.advance ();
}

//----------------------------------------------------------------------------
// Skeleton_Class::add_bone_to_list
//----------------------------------------------------------------------------

void Skeleton_Class::add_bone_to_list
(
	Bone_Class * new_bone
)
{
	new_bone->set_next_bone ( First_bone );
	First_bone = new_bone;
}

//----------------------------------------------------------------------------
// Bone_Class::create_node
//----------------------------------------------------------------------------

void Bone_Class::create_node
(
	ImpInterface *	import_interface,
	Interface *		max_interface
)
{
	Node = import_interface->CreateNode ();
	Node->SetName ( Name );

	// Create a box object.

	GeomObject * obj = (GeomObject *) max_interface->CreateInstance
		( GEOMOBJECT_CLASS_ID, Class_ID (BOXOBJ_CLASS_ID, 0) );

	IParamArray *iBoxParams = obj->GetParamBlock();
	assert(iBoxParams);

	// Set the value of width, height and length.

	int width_index = obj->GetParamBlockIndex(BOXOBJ_WIDTH);
	assert(width_index >= 0);
	iBoxParams->SetValue(width_index,TimeValue(0),Length / 4);

	int height_index = obj->GetParamBlockIndex(BOXOBJ_HEIGHT);
	assert(height_index >= 0);
	iBoxParams->SetValue(height_index,TimeValue(0),Length);

	int length_index = obj->GetParamBlockIndex(BOXOBJ_LENGTH);
	assert(length_index >= 0);
	iBoxParams->SetValue(length_index,TimeValue(0),Length / 4);

	Node->Reference ( (Object *) obj );

	Node->GetINode ()->SetNodeTM ( 0, Axis_tm );

	// Add the node to the scene.

	import_interface->AddNodeToScene ( Node );

		// Add an AppData chunk to the inode to indicate which transformation
	// axes are active.

	ASF_Data_Chunk * data_p = (ASF_Data_Chunk *)
		malloc ( sizeof (ASF_Data_Chunk) );

	*data_p = Active_axes;

	add_app_data ( 1, data_p, sizeof (ASF_Data_Chunk) );
}

//----------------------------------------------------------------------------
// Bone_Class::set_parent
//----------------------------------------------------------------------------

void Bone_Class::set_parent ( Bone_Class * parent_bone )
{
	Parent_bone = parent_bone;
	parent_bone->Node->GetINode ()->AttachChild ( Node->GetINode () );

	// Build the child node's transform matrix.

	Matrix3 parent_tm = Node->GetINode ()->GetParentTM ( 0 );

	Matrix3 child_tm = Axis_tm;
	child_tm.Translate ( Normalize (parent_bone->Direction) *
		 parent_bone->Length );
	child_tm.Translate ( parent_tm.GetTrans () );

	Node->GetINode ()->SetNodeTM ( 0, child_tm );
}

//----------------------------------------------------------------------------
// Bone_Class::add_app_data
//----------------------------------------------------------------------------

void Bone_Class::add_app_data ( int chunk_id, void * data, int data_size )
{
	Node->GetINode ()->AddAppDataChunk
	(
		Class_ID(0x74975aa6, 0x1810323f),
		SCENE_IMPORT_CLASS_ID,
		chunk_id,
		data_size,
		data
	);
}

