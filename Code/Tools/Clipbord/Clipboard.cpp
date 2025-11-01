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

#include "clipboard.h"
#include <istdplug.h>
#include "utilapi.h"

//----------------------------------------------------------------------------
// Node_Key
//----------------------------------------------------------------------------

class Node_Key
{
public:

	Node_Key ( int nr_frames, const char * name, Node_Key * next );
	~Node_Key ();

	void set_position ( int frame, Point3 pos )
	{
		if ( frame >= 0 && frame < nr_frames_ )
			position_ [frame] = pos;
	}

	void set_orientation ( int frame, Quat facing )
	{
		if ( frame >= 0 && frame < nr_frames_ )
			orientation_ [frame] = facing;
	}

	const Point3 & position ( int frame )
	{
		if ( frame < 0 || frame >= nr_frames_ )
			frame = 0;

		return position_ [frame];
	}

	const Quat & orientation ( int frame )
	{
		if ( frame < 0 || frame >= nr_frames_ )
			frame = 0;

		return orientation_ [frame];
	}

	const char * name () { return name_; }
	int nr_frames () { return nr_frames_; }
	Node_Key * next () { return next_; }

private:

	char * name_;
	int nr_frames_;
	Point3 * position_;
	Quat * orientation_;
	Node_Key * next_;
};

//----------------------------------------------------------------------------
// Node_Key::Node_Key
//----------------------------------------------------------------------------

Node_Key::Node_Key
(
	int nr_frames,
	const char * name,
	Node_Key * next
):
	nr_frames_ (nr_frames),
	next_ (next)
{
	position_ = new Point3 [ nr_frames ];
	orientation_ = new Quat [ nr_frames ];
	name_ = new char [ strlen (name) + 1 ];
	strcpy ( name_, name );
}

//----------------------------------------------------------------------------
// Node_Key::~Node_Key
//----------------------------------------------------------------------------

Node_Key::~Node_Key ()
{
	delete [] position_;
	delete [] orientation_;
	delete [] name_;
}

//----------------------------------------------------------------------------
// Pose
//----------------------------------------------------------------------------

const MAX_NAME = 64;

class Pose
{
public:

	Pose ( Interface * ip, const char * new_name, int first_frame, int last_frame );

	~Pose ()
	{
		delete_keys ();
	}

	const char * name () const { return name_; }

	void Add_Objects_To_List ( HWND hWnd );

	void Paste_Keys ( Interface * ip );

	Pose * next;

private:

	void delete_keys ();

	Node_Key * first_key;

	char name_ [ MAX_NAME ];

	int ticks_per_frame_;
};

//----------------------------------------------------------------------------
// Clipboard_Class
//----------------------------------------------------------------------------

class Clipboard_Class : public UtilityObj
{
public:

	Clipboard_Class ();
	~Clipboard_Class ();

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void SelectionSetChanged ( Interface * ip, IUtil * iu );
	void DeleteThis() {}

	void Init(HWND hWnd);
	void Destroy(HWND hWnd);
	void Close () { iu->CloseUtility (); }

	void Create ( HWND );
	void Paste  ( HWND );
	void Delete ( HWND );

	BOOL Is_Empty () const { return first_pose == NULL; }

	void Update_Object_List ( HWND hWnd );

private:

	void Update_Pose_List ( HWND hWnd );

	IUtil * iu;
	Interface *ip;
	HWND hPanel;

	Pose * first_pose;
};

//----------------------------------------------------------------------------
// the_clipboard
//----------------------------------------------------------------------------

static Clipboard_Class the_clipboard;

//----------------------------------------------------------------------------
// Static data used for communicating to/from the name dialog.
//----------------------------------------------------------------------------

static char pose_name [ MAX_NAME ];
static int  first_frame;
static int  last_frame;
static int  current_frame;

//----------------------------------------------------------------------------
// Clipboard_Desc_Class
//----------------------------------------------------------------------------

class Clipboard_Desc_Class:public ClassDesc
{
public:
	int 			IsPublic()     {return 1;}
	void *			Create(BOOL)   {return &the_clipboard;}
	const TCHAR *	ClassName()    {return _T("Key Clipboard");}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID()      {return Class_ID(0x5eb13907, 0x1d931bb4);}
	const TCHAR* 	Category()     {return _T("Westwood Tools");}
};

//----------------------------------------------------------------------------
// clipboard_desc
//----------------------------------------------------------------------------

static Clipboard_Desc_Class clipboard_desc;

//----------------------------------------------------------------------------
// ClipboardDesc
//----------------------------------------------------------------------------

ClassDesc* ClipboardDesc() {return &clipboard_desc;}

//----------------------------------------------------------------------------
// ClipboardDlgProc
//----------------------------------------------------------------------------

static BOOL CALLBACK ClipboardDlgProc
(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		the_clipboard.Init(hWnd);
		break;
	
	case WM_DESTROY:
		the_clipboard.Destroy(hWnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CLOSE:
			the_clipboard.Close ();
			break;

		case ID_CREATE:
			the_clipboard.Create ( hWnd );
			break;

		case ID_PASTE:
			the_clipboard.Paste ( hWnd );
			break;

		case ID_DELETE:
			the_clipboard.Delete ( hWnd );
			break;

		case IDC_POSE_LIST:
			if ( HIWORD(wParam) == CBN_SELCHANGE )
				the_clipboard.Update_Object_List ( hWnd );
			break;
		}
		break;

	default:
		return false;
	}
	return true; 
}

//----------------------------------------------------------------------------
// Pose_Name_Message_Handler
//----------------------------------------------------------------------------

static BOOL CALLBACK Pose_Name_Message_Handler
(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	static ISpinnerControl * first_frame_spin;
	static ISpinnerControl * last_frame_spin;

	switch (msg)
	{
	case WM_INITDIALOG:
		CenterWindow ( hWnd, GetParent (hWnd) );
		SetFocus ( GetDlgItem (hWnd, IDC_NAME) );

		first_frame_spin = SetupIntSpinner
		(
			hWnd,
			IDC_FIRST_FRAME_SPIN,
			IDC_FIRST_FRAME_EDIT,
			first_frame,
			last_frame,
			current_frame
		);

		first_frame_spin->SetResetValue ( current_frame );

		last_frame_spin = SetupIntSpinner
		(
			hWnd,
			IDC_LAST_FRAME_SPIN,
			IDC_LAST_FRAME_EDIT,
			first_frame,
			last_frame,
			current_frame
		);

		last_frame_spin->SetResetValue ( current_frame );

		return true;
	
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if ( SendDlgItemMessage ( hWnd, IDC_NAME, WM_GETTEXTLENGTH, 0, 0 ) == 0 )
				EndDialog ( hWnd, 0 );

			SendDlgItemMessage ( hWnd, IDC_NAME, WM_GETTEXT, MAX_NAME,
				(LPARAM) pose_name );

			first_frame = first_frame_spin->GetIVal ();
			last_frame  = last_frame_spin->GetIVal ();
			EndDialog ( hWnd, 1 );
			break;

		case IDCANCEL:
			EndDialog ( hWnd, 0 );
			break;
		}
		return true;

	case CC_SPINNER_CHANGE:

		// Do checking on the range spinners to make sure the low value never 
		// exceeds the high value.

		switch ( LOWORD(wParam) )
		{
		case IDC_FIRST_FRAME_SPIN:
			if ( first_frame_spin->GetIVal () > last_frame_spin->GetIVal () )
			{
				last_frame_spin->SetValue ( first_frame_spin->GetIVal (),
					false );
			}
			break;

		case IDC_LAST_FRAME_SPIN:
			if ( last_frame_spin->GetIVal () < first_frame_spin->GetIVal () )
			{
				first_frame_spin->SetValue ( last_frame_spin->GetIVal (),
					false );
			}
			break;
		}

		return true;
	}

	return false; 
}

//----------------------------------------------------------------------------
// Clipboard_Class::Clipboard_Class
//----------------------------------------------------------------------------

Clipboard_Class::Clipboard_Class()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
	first_pose = NULL;
}

//----------------------------------------------------------------------------
// Clipboard_Class::~Clipboard_Class
//----------------------------------------------------------------------------

Clipboard_Class::~Clipboard_Class ()
{
	while ( first_pose != NULL )
	{
		Pose * delete_p = first_pose;
		first_pose = first_pose->next;
		delete delete_p;
	}
}

//----------------------------------------------------------------------------
// Clipboard_Class::BeginEditParams
//----------------------------------------------------------------------------

void Clipboard_Class::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage
	(
		hInstance,
		MAKEINTRESOURCE(IDD_CLIPBOARD_PANEL),
		ClipboardDlgProc,
		_T("Key Clipboard"),
		0
	);
}
	
//----------------------------------------------------------------------------
// Clipboard_Class::EndEditParams
//----------------------------------------------------------------------------

void Clipboard_Class::EndEditParams ( Interface *ip, IUtil *iu ) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

//----------------------------------------------------------------------------
// Clipboard_Class::SelectionSetChanged
//----------------------------------------------------------------------------

void Clipboard_Class::SelectionSetChanged ( Interface * ip, IUtil * iu )
{
	if ( ip->GetSelNodeCount () == 0 )
	{
		EnableWindow ( GetDlgItem ( hPanel, ID_CREATE ), false );
	}
	else
	{
		EnableWindow ( GetDlgItem ( hPanel, ID_CREATE ), true );
	}
}

//----------------------------------------------------------------------------
// Clipboard_Class::Init
//----------------------------------------------------------------------------

void Clipboard_Class::Init ( HWND hWnd )
{
	Update_Pose_List ( hWnd );
	SendDlgItemMessage ( hWnd, IDC_POSE_LIST, CB_SETCURSEL, 0, 0 );
	Update_Object_List ( hWnd );

	if ( ip->GetSelNodeCount () == 0 )
	{
		EnableWindow ( GetDlgItem ( hWnd, ID_CREATE ), false );
	}
	else
	{
		EnableWindow ( GetDlgItem ( hWnd, ID_CREATE ), true );
	}

	if ( Is_Empty () )
	{
		EnableWindow ( GetDlgItem ( hWnd, ID_PASTE  ), false );
		EnableWindow ( GetDlgItem ( hWnd, ID_DELETE ), false );
	}
	else
	{
		EnableWindow ( GetDlgItem ( hWnd, ID_PASTE  ), true );
		EnableWindow ( GetDlgItem ( hWnd, ID_DELETE ), true );
	}
}

//----------------------------------------------------------------------------
// Clipboard_Class::Destroy
//----------------------------------------------------------------------------

void Clipboard_Class::Destroy(HWND hWnd) {}

//----------------------------------------------------------------------------
// Clipboard_Class::Create
//----------------------------------------------------------------------------

void Clipboard_Class::Create ( HWND hWnd )
{
	// Set up the data that will appear in the name dialog.

	const int ticks_per_frame = GetTicksPerFrame ();

	first_frame   = ip->GetAnimRange ().Start () / ticks_per_frame;
	last_frame    = ip->GetAnimRange ().End () / ticks_per_frame;
	current_frame = ip->GetTime () / ticks_per_frame;

	// Put up the name dialog.

	int rv = DialogBoxParam
	(
		hInstance,
		MAKEINTRESOURCE ( IDD_POSE_NAME ),
		hWnd,
		Pose_Name_Message_Handler,
		0
	);

	if ( rv == 0 )
		return;

	Pose * new_pose = new Pose ( ip, pose_name, first_frame, last_frame );

	new_pose->next = first_pose;
	first_pose = new_pose;

	Update_Pose_List ( hWnd );
	SendDlgItemMessage ( hWnd, IDC_POSE_LIST, CB_SETCURSEL, 0, 0 );
	Update_Object_List ( hWnd );

	EnableWindow ( GetDlgItem ( hWnd, ID_PASTE  ), true );
	EnableWindow ( GetDlgItem ( hWnd, ID_DELETE ), true );
}

//----------------------------------------------------------------------------
// Clipboard_Class::Paste
//----------------------------------------------------------------------------

void Clipboard_Class::Paste ( HWND hWnd )
{
	SetCursor ( LoadCursor (NULL, IDC_WAIT) );

	int sel = SendDlgItemMessage ( hWnd, IDC_POSE_LIST, CB_GETCURSEL, 0, 0 );

	if ( sel == CB_ERR )
		return;

	// Find the selected pose.

	Pose * p = first_pose;

	while ( sel > 0 && p != NULL )
	{
		p = p->next;
		-- sel;
	}

	if ( p == NULL )
		return;

	p->Paste_Keys ( ip );

	SetCursor ( LoadCursor (NULL, IDC_ARROW) );
}

//----------------------------------------------------------------------------
// Clipboard_Class::Delete
//----------------------------------------------------------------------------

void Clipboard_Class::Delete ( HWND hWnd )
{
	int sel = SendDlgItemMessage ( hWnd, IDC_POSE_LIST, CB_GETCURSEL, 0, 0 );

	if ( sel != CB_ERR )
	{
		// Delete the pose's name from the list box.

		SendDlgItemMessage ( hWnd, IDC_POSE_LIST, CB_DELETESTRING, sel, 0 );
		SendDlgItemMessage ( hWnd, IDC_POSE_LIST, CB_SETCURSEL, 0, 0 );

		// Find the selected pose and delete it.

		Pose ** pointer_to_p = & first_pose;
		Pose * p = first_pose;

		while ( sel > 0 && p != NULL )
		{
			pointer_to_p = & p->next;
			p = p->next;
			-- sel;
		}

		if ( p != NULL )
		{
			*pointer_to_p = p->next;

			delete p;
		}

		Update_Object_List ( hWnd );
	}

	if ( Is_Empty () )
	{
		EnableWindow ( GetDlgItem ( hWnd, ID_PASTE  ), false );
		EnableWindow ( GetDlgItem ( hWnd, ID_DELETE ), false );
	}
}

//----------------------------------------------------------------------------
// Clipboard_Class::Update_Pose_List
//----------------------------------------------------------------------------

void Clipboard_Class::Update_Pose_List ( HWND hWnd )
{
	// Delete any strings in the combo box.

	SendDlgItemMessage ( hWnd, IDC_POSE_LIST, CB_RESETCONTENT, 0, 0 );

	// Rebuild the combo box.

	for ( Pose * p = first_pose; p != NULL; p = p->next )
	{
		SendDlgItemMessage ( hWnd, IDC_POSE_LIST, CB_ADDSTRING, 0,
			(LPARAM) (LPCTSTR) p->name () );
	}
}

//----------------------------------------------------------------------------
// Clipboard_Class::Update_Object_List
//----------------------------------------------------------------------------

void Clipboard_Class::Update_Object_List ( HWND hWnd )
{
	// Delete any strings in the combo box.

	SendDlgItemMessage ( hWnd, IDC_OBJECT_LIST, LB_RESETCONTENT, 0, 0 );

	// Add strings from the objects in the currently selected pose.

	int sel = SendDlgItemMessage ( hWnd, IDC_POSE_LIST, CB_GETCURSEL, 0, 0 );

	if ( sel != CB_ERR )
	{
		Pose * p = first_pose;

		while ( sel > 0 && p != NULL )
		{
			p = p->next;
			-- sel;
		}

		if ( p != NULL )
		{
			p->Add_Objects_To_List ( hWnd );
		}
	}
}

//----------------------------------------------------------------------------
// Pose::Pose
//----------------------------------------------------------------------------

Pose::Pose ( Interface * ip, const char * name, int first_frame, int last_frame ):
	first_key (NULL),
	next (NULL),
	ticks_per_frame_ (GetTicksPerFrame ())
{
	int number_of_nodes = ip->GetSelNodeCount ();
	int nr_frames = (last_frame - first_frame) + 1;
	TimeValue start_time = first_frame * ticks_per_frame_;

	char frame_count [ 16 ];
	sprintf ( frame_count, " (%d)", nr_frames );

	strncpy ( name_, name, MAX_NAME - 6 );
	strncat ( name_, frame_count, 5 );
	name_ [MAX_NAME - 1] = '\0';

	for ( int i = 0; i < number_of_nodes; ++ i )
	{
		// Get the inode.

		INode * inode = ip->GetSelNode ( i );

		Control * tm_controller = inode->GetTMController ();
		if ( tm_controller == NULL )
			continue;

		Node_Key * new_key = new Node_Key
		(
			nr_frames,
			inode->GetName (),
			first_key
		);

		first_key = new_key;

		// Store the position and rotation keys.

		Point3 node_position ( 0.0, 0.0, 0.0 );
		Quat node_orientation ( 0.0, 0.0, 0.0, 1.0 );

		for ( int frame = 0; frame < nr_frames; ++ frame )
		{
			TimeValue key_time = frame * ticks_per_frame_ + start_time;
			Control * c;

			// Copy rotation keys.

			c = tm_controller->GetRotationController ();

			if ( c != NULL )
				c->GetValue ( key_time, & node_orientation, FOREVER );
			else
				node_orientation = Quat (0.0, 0.0, 0.0, 1.0);

			// Copy position keys.

			c = tm_controller->GetPositionController ();

			if ( c != NULL )
				c->GetValue ( key_time, & node_position, FOREVER );
			else
				node_position = Point3 (0.0, 0.0, 0.0);

			new_key->set_position    ( frame, node_position    );
			new_key->set_orientation ( frame, node_orientation );
		}
	}
}

//----------------------------------------------------------------------------
// Pose::Paste_Keys
//----------------------------------------------------------------------------

void Pose::Paste_Keys ( Interface * ip )
{
	TimeValue current_time = ip->GetTime ();

	theHold.Begin ();

	SuspendAnimate ();
	AnimateOn ();

	Node_Key * p = first_key;

	while ( p != NULL )
	{
		INode * inode = ip->GetINodeByName ( p->name () );

		if ( inode != NULL )
		{
			Control * tm_controller = inode->GetTMController ();
			if ( tm_controller != NULL )
			{
				for ( int frame = 0; frame < p->nr_frames (); ++ frame )
				{
					TimeValue key_time = current_time + frame * ticks_per_frame_;
					Control * c;

					// Paste rotation keys.

					c = tm_controller->GetRotationController ();

					if ( c != NULL )
					{
						c->EnableORTs (false);
						Quat orientation = p->orientation (frame);
						c->SetValue ( key_time, & orientation );
						c->EnableORTs (true);
					}

					// Paste position keys.

					c = tm_controller->GetPositionController ();

					if ( c != NULL )
					{
						c->EnableORTs (false);
						Point3 position = p->position (frame);
						c->SetValue ( key_time, & position );
						c->EnableORTs (true);
					}
				}
			}
		}

		p = p->next ();
	}

	ResumeAnimate ();

	TSTR undostr;
	undostr.printf ( "Paste Keys" );
	theHold.Accept ( undostr );

	ip->RedrawViews ( current_time );
}

//----------------------------------------------------------------------------
// Pose::delete_keys
//----------------------------------------------------------------------------

void Pose::delete_keys ()
{
	while ( first_key != NULL )
	{
		Node_Key * delete_p = first_key;
		first_key = first_key->next ();
		delete delete_p;
	}
}

//----------------------------------------------------------------------------
// Pose::Add_Objects_To_List
//----------------------------------------------------------------------------

void Pose::Add_Objects_To_List ( HWND hWnd )
{
	for ( Node_Key * k = first_key; k != NULL; k = k->next () )
	{
		SendDlgItemMessage ( hWnd, IDC_OBJECT_LIST, LB_ADDSTRING, 0,
			(LPARAM) (LPCTSTR) k->name () );
	}
}
