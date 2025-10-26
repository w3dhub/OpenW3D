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
// amc_import.cpp
//
// Acclaim Motion Capture import module
//
// James McNeill
//
// Created October 1996
//
// Copyright (c) 1996 Westwood Studios
//----------------------------------------------------------------------------

#include <Max.h>
#include <istdplug.h>
#include <fstream.h>

#include "resource.h"
#include "exception.h"
#include "asf_data.h"

const float ANGLE_MULTIPLIER = 1.7453293e-2f; // degrees to radians

const first_frame_time         = 320;
const amc_ticks_per_frame      = 80;
static int max_ticks_per_frame = 160;

static HINSTANCE   hInstance;

static TCHAR *     GetString ( int id )
{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

static int MessageBox ( int s1, int s2, int option = MB_OK )
{
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
}

static int Alert ( int s1, int s2 = IDS_LIB_SHORT_DESC, int option = MB_OK )
{
	return MessageBox(s1, s2, option);
}

//----------------------------------------------------------------------------
// AMC_Import
//----------------------------------------------------------------------------

class AMC_Import : public SceneImport
{
public:

					AMC_Import();
					~AMC_Import();

	int				ExtCount();			// Number of extensions supported
	const TCHAR *	Ext(int n);			// Extension #n
	const TCHAR *	LongDesc();			// Long ASCII description
	const TCHAR *	ShortDesc();		// Short ASCII description
	const TCHAR *	AuthorName();		// ASCII Author name
	const TCHAR *	CopyrightMessage();	// ASCII Copyright message
	const TCHAR *	OtherMessage1();	// Other message #1
	const TCHAR *	OtherMessage2();	// Other message #2
	unsigned int	Version();			// Version number * 100
	void			ShowAbout(HWND);	// Show DLL's "About..." box

	int				DoImport
	(
		const TCHAR *  name,
		ImpInterface * i,
		Interface *    gi,
		BOOL				suppressPrompts
	);
};

//----------------------------------------------------------------------------
// DllMain
//----------------------------------------------------------------------------

static int         controlsInit = false;

BOOL WINAPI        DllMain
(
    HINSTANCE      hinstDLL,
	ULONG          fdwReason,
	LPVOID         lpvReserved
)
{
	hInstance = hinstDLL;

	if ( !controlsInit )
	{
		controlsInit = true;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH: break;
		case DLL_THREAD_ATTACH:  break;
		case DLL_THREAD_DETACH:  break;
		case DLL_PROCESS_DETACH: break;
	}

	return true;
}


//----------------------------------------------------------------------------
// AMC_ClassDesc
//----------------------------------------------------------------------------

class AMC_ClassDesc : public ClassDesc
{
public:
	int 			IsPublic()     { return 1; }
	void *			Create(BOOL loading = false) { return new AMC_Import; }
	const TCHAR *	ClassName()    { return GetString(IDS_SHORT_DESC); }
	SClass_ID		SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID		ClassID()      { return Class_ID(0x5be11422, 0x6e0177f0); }
	const TCHAR* 	Category()     { return GetString(IDS_CATEGORY);  }
};

static AMC_ClassDesc AMC_desc;

//----------------------------------------------------------------------------
// This is the interface to Jaguar:
//----------------------------------------------------------------------------

__declspec( dllexport ) const TCHAR *    LibDescription()
{
	return GetString(IDS_LIB_LONG_DESC);
}

__declspec( dllexport ) int    LibNumberClasses()
{
	return 1;
}

__declspec( dllexport ) ClassDesc *    LibClassDesc(int i)
{
	switch(i)
	{
	case  0: return & AMC_desc; break;
	default: return          0; break;
	}
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG    LibVersion()
{
	return VERSION_3DSMAX;
}

//
// ASF import module functions follow:
//

AMC_Import::AMC_Import() {}
AMC_Import::~AMC_Import() {}

int    AMC_Import::ExtCount()
{
	return 1;
}

// Extensions supported for import/export modules

const TCHAR *      AMC_Import::Ext(int n)
{
	return _T("AMC");
}

const TCHAR *      AMC_Import::LongDesc()
{
	return GetString(IDS_LONG_DESC);
}
	
const TCHAR *      AMC_Import::ShortDesc()
{
	return GetString(IDS_SHORT_DESC);
}

const TCHAR *      AMC_Import::AuthorName()
{
	return GetString(IDS_AUTHOR_NAME);
}

const TCHAR *      AMC_Import::CopyrightMessage()
{
	return GetString(IDS_COPYRIGHT);
}

const TCHAR *      AMC_Import::OtherMessage1()
{
	return _T("");
}

const TCHAR *      AMC_Import::OtherMessage2()
{
	return _T("");
}

unsigned int       AMC_Import::Version()
{
	return 100;
}

void               AMC_Import::ShowAbout(HWND hWnd)
{
}

//----------------------------------------------------------------------------
// Key_Class
//----------------------------------------------------------------------------

struct Key_Class
{
	TimeValue		Time;
	Quat			Orientation;
	Point3			Position;
};

//----------------------------------------------------------------------------
// Bone
//----------------------------------------------------------------------------

class Bone
{
public:

	Bone ( const char * name, Bone * next )
	{
		Name = new char [ strlen (name) + 1 ];
		strcpy ( Name, name );
		Next = next;
		Max_keys = 1;
		Number_of_keys = 0;
		Key = NULL;
	}

	~Bone ()
	{
		delete [] Name;
		if ( Key != NULL )
			delete [] Key;
	}

	void alloc_key_space ( Interface * gi )
	{
		if ( Key == NULL && Max_keys > 0 )
			Key = new Key_Class [ Max_keys ];

		// Find the bone's I-node.
		// TODO: check to ensure we got the inode!
		INode * inode = gi->GetINodeByName ( Name );

		if (inode) {
			// Add a key at frame zero to hold the original position.

			Quat rotation ( 0.0f, 0.0f, 0.0f, 1.0f );
	
			Matrix3 localTM = inode->GetNodeTM (0) *
				Inverse (inode->GetParentTM (0));
			rotation *= localTM;

			add_key ( 0, rotation, Point3 (0,0,0) );
		}
	}

	void inc_max_keys () { ++ Max_keys; }

	void add_key ( TimeValue time, const Quat & rot, const Point3 & pos )
	{
		if ( Key != NULL && Number_of_keys < Max_keys )
		{
			Key [Number_of_keys].Time		 = time;
			Key [Number_of_keys].Orientation = rot;
			Key [Number_of_keys].Position	 = pos;

			++ Number_of_keys;
		}
	}

	void put_keys_into_max ( ImpInterface * imp_i, Interface * i );

	Bone * next () const { return Next; }
	char * name () const { return Name; }

protected:

	char *			Name;
	Bone *			Next;
	unsigned		Max_keys;
	unsigned		Number_of_keys;
	Key_Class *		Key;
};

//----------------------------------------------------------------------------
// Bone::put_keys_into_max
//----------------------------------------------------------------------------

void Bone::put_keys_into_max ( ImpInterface * imp_i, Interface * i )
{
	unsigned k;

	// Find the bone's I-node.

	INode * inode = i->GetINodeByName ( Name );
	if ( inode == NULL )
		return;

	// Make each quaternion be on the same side of the hypersphere as its
	// predecessor.

	for ( k = 1; k < Number_of_keys; ++ k )
	{
		Key [k].Orientation.MakeClosest ( Key [k-1].Orientation );
	}

	// Make each rotation relative to the previous one.

	if ( Number_of_keys > 1 )
	{
		for ( unsigned k = Number_of_keys - 1; k > 0; -- k )
		{
			Key [k].Orientation = Key [k].Orientation / Key [k-1].Orientation;
		}
	}

	// Add the keys to the bone's rotation controller.

	// Replace the rotation controller if it is not the right type.

	Control * c = inode->GetTMController ()->GetRotationController ();

	if ( c && c->ClassID () != Class_ID (TCBINTERP_ROTATION_CLASS_ID, 0) )
	{
		Control *tcb = (Control*) i->CreateInstance ( CTRL_ROTATION_CLASS_ID,
			Class_ID (TCBINTERP_ROTATION_CLASS_ID,0) );
		if ( ! inode->GetTMController ()->SetRotationController(tcb) )
		{
			tcb->DeleteThis();
		}
	}

	c = inode->GetTMController ()->GetRotationController ();
//	c->SetORT ( ORT_LOOP, ORT_BEFORE );
//	c->SetORT ( ORT_LOOP, ORT_AFTER );

	IKeyControl * keys = GetKeyControlInterface ( c );

	keys->SetNumKeys ( Number_of_keys );

	// Create keys.

	for ( k = 0; k < Number_of_keys; ++ k )
	{
		ITCBRotKey key;
		key.tens = 0.0f;
		key.cont = 0.0f;
		key.bias = 0.0f;
		key.easeIn = 0.0f;
		key.easeOut = 0.0f;

		key.time = Key [k].Time;
		key.val  = (AngAxis) Key [k].Orientation;

		keys->SetKey ( k, & key );
	}

	keys->SortKeys ();

	// &&& This is a hack; add position keys for the root node.
	// Another hack: The positions are multiplied by a scale factor which
	// should be derived from the ASF file.
	float length_multiplier;

	if ( strcmp ( Name, "root" ) == 0 )
	{
		// Since this is the root node, check for an AppData chunk defining
		// the length multiplier.
		// This "length multiplier" will convert whatever numbers are in the file
		// into *inches*.  We later convert this to whatever units Max is using...
		AppDataChunk * chunk = inode->GetAppDataChunk
		(
			Class_ID(0x74975aa6, 0x1810323f),
			SCENE_IMPORT_CLASS_ID,
			2
		);

		if ( chunk == NULL )
		{
			// (gth) HACK HACK HACK!  For some time, I had removed this 'length_multiplier'
			// Many commando animations were created without the multiplier which was 1.0 / 0.0254
			// so, shove that multiplier in if one wasn't found...  This only comes into play
			// when loading a motion into a max file that use the old importer to create the
			// base pose.
			length_multiplier = 1.0f / 0.0254f;
		}
		else
		{
			Position_Key_Scale_Chunk * data_p = (Position_Key_Scale_Chunk *) chunk->data;
			length_multiplier = data_p->Position_Key_Scale;
		}

		// Account for the current units setting of Max (e.g. 1 unit = 1 Meter)
		length_multiplier /= (float)GetMasterScale(UNITS_INCHES);

		// Make sure we have the right kind of controller.

		c = inode->GetTMController ()->GetPositionController ();

		if ( c && c->ClassID () != Class_ID (TCBINTERP_POSITION_CLASS_ID, 0) )
		{
			Control *tcb = (Control*) i->CreateInstance ( CTRL_POSITION_CLASS_ID,
				Class_ID (TCBINTERP_POSITION_CLASS_ID,0) );
			if ( ! inode->GetTMController ()->SetPositionController(tcb) )
			{
				tcb->DeleteThis();
			}
		}

		c = inode->GetTMController ()->GetPositionController ();
//		c->SetORT ( ORT_LOOP, ORT_BEFORE );
//		c->SetORT ( ORT_LOOP, ORT_AFTER );

		keys = GetKeyControlInterface ( c );

		keys->SetNumKeys ( Number_of_keys );

		// Create keys.

		for ( k = 0; k < Number_of_keys; ++ k )
		{
			ITCBPoint3Key key;
			key.tens = 0.0f;
			key.cont = 0.0f;
			key.bias = 0.0f;
			key.easeIn = 0.0f;
			key.easeOut = 0.0f;

			key.time = Key [k].Time;
			key.val  = Key [k].Position  * length_multiplier;

			keys->SetKey ( k, & key );
		}

		keys->SortKeys ();
	}
}

//----------------------------------------------------------------------------
// Key_Manager
//----------------------------------------------------------------------------

class Key_Manager
{
public:

	Key_Manager () { Bones = NULL; }
	~Key_Manager ();

	// Call this first to increment the maximum number of keys for a given
	// bone.

	void inc_max_keys ( const char * bone_name );

	// After all the maximums are set properly, call this to allocate space
	// for holding the keys.

	void alloc_key_space ( Interface * gi );

	// After allocating space, add keys by calling this.

	void add_key
	(
		const char *	bone_name,
		TimeValue		time,
		const Quat &	rot,
		const Point3 &	pos
	);

	// Once all the keys have been added, call this to massage them and
	// put them into 3D Studio.

	void put_keys_into_max ( ImpInterface *, Interface * );

protected:

	Bone *			Bones;
};

//----------------------------------------------------------------------------
// Key_Manager::~Key_Manager
//----------------------------------------------------------------------------

Key_Manager::~Key_Manager ()
{
	Bone * p = Bones;

	while ( p != NULL )
	{
		Bone * delete_p = p;
		p = p->next ();

		delete delete_p;
	}
}

//----------------------------------------------------------------------------
// Key_Manager::inc_max_keys
//----------------------------------------------------------------------------

void Key_Manager::inc_max_keys ( const char * name )
{
	for ( Bone * p = Bones; p != NULL; p = p->next () )
	{
		if ( strcmp ( p->name (), name ) == 0 )
			break;
	}

	if ( p == NULL )
	{
		// This is a new bone; add it to the list.

		p = new Bone ( name, Bones );

		Bones = p;
	}

	p->inc_max_keys ();
}

//----------------------------------------------------------------------------
// Key_Manager::alloc_key_space
//----------------------------------------------------------------------------

void Key_Manager::alloc_key_space ( Interface * gi )
{
	for ( Bone * p = Bones; p != NULL; p = p->next () )
	{
		p->alloc_key_space ( gi );
	}
}

//----------------------------------------------------------------------------
// Key_Manager::add_key
//----------------------------------------------------------------------------

void Key_Manager::add_key
(
	const char *	name,
	TimeValue		time,
	const Quat &	rot,
	const Point3 &	pos
)
{
	for ( Bone * p = Bones; p != NULL; p = p->next () )
	{
		if ( strcmp ( p->name (), name ) == 0 )
			break;
	}

	if ( p != NULL )
	{
		p->add_key ( time, rot, pos );
	}
}

//----------------------------------------------------------------------------
// Key_Manager::put_keys_into_max
//----------------------------------------------------------------------------

void Key_Manager::put_keys_into_max ( ImpInterface * imp_i, Interface * i )
{
	for ( Bone * p = Bones; p != NULL; p = p->next () )
	{
		p->put_keys_into_max ( imp_i, i );
	}
}

//----------------------------------------------------------------------------
// get_name
//----------------------------------------------------------------------------

void				get_name
(
	char * &		line_p,
	char * &		name_p
)
{
	name_p = line_p;

	while ( ! isspace (*line_p) )
		++ line_p;

	*line_p = '\0';
	++ line_p;
}

//----------------------------------------------------------------------------
// read_frames
//----------------------------------------------------------------------------

static void			read_frames
(
	FILE *			file,
	ImpInterface *	iface,
	Interface *		gi
)
{
	char			line [ 512 ];
	Key_Manager		key_manager;

	// Start by identifying the bones in the system and the number of keys
	// for each.

	long start_pos = ftell ( file );

	while (1)
	{
		char * rv = fgets ( line, sizeof line, file );
		if ( rv == NULL )
			break;

		if ( ! isdigit (line [0]) )
		{
			char * line_p = line;
			char * name_p;

			get_name ( line_p, name_p );

			key_manager.inc_max_keys ( name_p );
		}
	}

	// Prepare to scan the file again.  Allocate space to hold the keys.

	fseek ( file, start_pos, SEEK_SET );
	key_manager.alloc_key_space ( gi );

	int frame_time = 0;

	while (1)
	{
		char * rv = fgets ( line, sizeof line, file );
		if ( rv == NULL )
			break;

		if ( isdigit (line [0]) )
		{
			// This line marks the start of a new frame.
			
			int frame_number = strtol ( line, NULL, 10 );
			frame_time = (frame_number - 1) * amc_ticks_per_frame +
				first_frame_time;
		}
		else
		{
			// This line contains a bone's data for the current frame.
			// Get the name of the bone whose key is being defined.

			char * line_p = line;
			char * name_p;
			get_name ( line_p, name_p );

			// Find the I-node with the name given in the input file.

			INode * inode = gi->GetINodeByName ( name_p );
#if 0
			if ( inode == NULL )
			{
				char message [ 256 ];
				sprintf ( message, "Can't find node named \"%s\".",
					name_p );
				MessageBox ( GetActiveWindow (), message, "Parse error",
					MB_OK );
				return;
			}
#else
			if ( inode == NULL) {
				continue;
			}
#endif

			// From the I-node, get the appdata chunk that defines the order
			// in which rotations should be applied.

			AppDataChunk * chunk = inode->GetAppDataChunk
			(
				Class_ID(0x74975aa6, 0x1810323f),
				SCENE_IMPORT_CLASS_ID,
				1
			);

			if ( chunk == NULL )
			{
				char message [ 256 ];
				sprintf ( message, "\"%s\" has no app data chunk.",
					name_p );
				MessageBox ( GetActiveWindow (), message, "Parse error",
					MB_OK );
				return;
			}

			ASF_Data_Chunk * data_p = (ASF_Data_Chunk *) chunk->data;

			// Read in angle settings and build a transform matrix from them.

			Quat rotation (0.0f,0.0f,0.0f,1.0f);
			Point3 translation ( 0, 0, 0 );

			for ( unsigned i = 0; i < data_p->Number_of_axes; ++ i )
			{
				float value;

				value = (float) strtod ( line_p, & line_p );

				switch ( data_p->Axis [i] )
				{
				case ROTATE_X:
					rotation *= (Quat) AngAxis ( Point3 (1, 0, 0),
						-value * ANGLE_MULTIPLIER );
					break;
				case ROTATE_Y:
					rotation *= (Quat) AngAxis ( Point3 (0, 1, 0),
						-value * ANGLE_MULTIPLIER );
					break;
				case ROTATE_Z:
					rotation *= (Quat) AngAxis ( Point3 (0, 0, 1),
						-value * ANGLE_MULTIPLIER );
					break;
				case TRANSLATE_X:
					translation.x = value;
					break;
				case TRANSLATE_Y:
					translation.y = value;
					break;
				case TRANSLATE_Z:
					translation.z = value;
					break;
				case TRANSLATE_LENGTH:
				default:
					break;
				}
			}

			Matrix3 localTM = inode->GetNodeTM (0) *
				Inverse (inode->GetParentTM (0));
			rotation *= localTM;

			if ( frame_time % max_ticks_per_frame == 0 )
				key_manager.add_key ( name_p, frame_time, rotation, translation );

		}
	}

	key_manager.put_keys_into_max ( iface, gi );

	gi->SetAnimRange ( Interval (0, frame_time) );
}

//----------------------------------------------------------------------------
// amc_load
//----------------------------------------------------------------------------

static int         amc_load
(
	const TCHAR *  filename,
	ImpInterface * iface,
	Interface *    gi
)
{
	char line [ 512 ];

	// Load the motion-capture data file.

	FILE * file = fopen ( filename, "r" );

	// Ignore the first line.

	fgets ( line, sizeof line, file );

	// Match the second line to ":FULLY-SPECIFIED".

	fgets ( line, sizeof line, file );
	if ( strcmp ( line, ":FULLY-SPECIFIED\n" ) != 0 )
	{
		MessageBox ( GetActiveWindow (), "First line.", "Parse error",
			MB_OK );
		return -1;
	}

	// Match the third line to ":DEGREES".

	fgets ( line, sizeof line, file );
	if ( strcmp ( line, ":DEGREES\n" ) != 0 )
	{
		MessageBox ( GetActiveWindow (), "Second line.", "Parse error",
			MB_OK );
		return -1;
	}

	// Build the data structures to hold the rotation keys.

	read_frames ( file, iface, gi );

	fclose ( file );

	return 1;
}

//----------------------------------------------------------------------------
// AMC_Import::DoImport
//----------------------------------------------------------------------------

int                AMC_Import::DoImport
(
	const TCHAR *  filename,
	ImpInterface * iface,
	Interface *    gi,
	BOOL
)
{
	max_ticks_per_frame = GetTicksPerFrame ();

	int status = amc_load ( filename, iface, gi );
	if ( status == 0 )
		status = IMPEXP_CANCEL;	

	return (status <= 0) ? IMPEXP_FAIL : status;
}
