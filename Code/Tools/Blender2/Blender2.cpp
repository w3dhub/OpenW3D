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

#include "blender2.h"
#include <istdplug.h>
#include "utilapi.h"
#include "appdata.h"

//----------------------------------------------------------------------------
// Blender_Class
//----------------------------------------------------------------------------

class Blender_Class : public UtilityObj
{
public:

	Blender_Class();

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void SelectionSetChanged ( Interface * ip, IUtil * ui );
	void DeleteThis() {}

	void Init(HWND hWnd);
	void Destroy(HWND hWnd);
	void Close () { iu->CloseUtility (); }

	void Get_Active_Time_Range ();

	void Blend_Keys ();

	void Loop_Controllers ();

	void OutputObject(INode *node,TCHAR *fname);

private:

	BOOL Is_Root ( INode * node );
	float Heading_Delta_From_Quat ( Quat q );
	void Set_Data_Chunk ( INode * node, const Blender_Data_Chunk & new_data );
	void Remove_Data_Chunk ( INode * node );

	int first_frame;
	int first_match;
	int last_frame;
	int last_match;

	ISpinnerControl * first_frame_spin;
	ISpinnerControl * first_match_spin;
	ISpinnerControl * last_frame_spin;
	ISpinnerControl * last_match_spin;

	IUtil * iu;
	Interface *ip;
	HWND hPanel;
};

//----------------------------------------------------------------------------
// the_blender
//----------------------------------------------------------------------------

static Blender_Class the_blender;

//----------------------------------------------------------------------------
// Blender_Desc_Class
//----------------------------------------------------------------------------

class Blender_Desc_Class:public ClassDesc
{
public:
	int 			IsPublic()     {return 1;}
	void *			Create(BOOL)   {return &the_blender;}
	const TCHAR *	ClassName()    {return _T("Key Blender");}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID()      {return Blender_Class_ID;}
	const TCHAR* 	Category()     {return _T("Westwood Tools");}
};

//----------------------------------------------------------------------------
// blender_desc
//----------------------------------------------------------------------------

static Blender_Desc_Class blender_desc;

//----------------------------------------------------------------------------
// BlenderDesc
//----------------------------------------------------------------------------

ClassDesc* BlenderDesc() {return &blender_desc;}

//----------------------------------------------------------------------------
// BlenderDlgProc
//----------------------------------------------------------------------------

static BOOL CALLBACK BlenderDlgProc
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
		the_blender.Init(hWnd);			
		break;
	
	case WM_DESTROY:
		the_blender.Destroy(hWnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CLOSE:
			the_blender.Close ();
			break;

		case ID_GET_ACTIVE_TIME_RANGE:
			the_blender.Get_Active_Time_Range ();
			break;

		case ID_APPLY:
			the_blender.Blend_Keys ();
			break;
		}
		break;

	default:
		return false;
	}
	return true; 
}

//----------------------------------------------------------------------------
// Blender_Class::Blender_Class
//----------------------------------------------------------------------------

Blender_Class::Blender_Class()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;

	first_frame = 0;
	first_match = 0;
	last_frame  = 0;
	last_match  = 0;
}

//----------------------------------------------------------------------------
// Blender_Class::BeginEditParams
//----------------------------------------------------------------------------

void Blender_Class::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage
	(
		hInstance,
		MAKEINTRESOURCE(IDD_ASCIIOUT_PANEL),
		BlenderDlgProc,
		_T("Key Blender"),
		0
	);
}
	
//----------------------------------------------------------------------------
// Blender_Class::EndEditParams
//----------------------------------------------------------------------------

void Blender_Class::EndEditParams ( Interface *ip, IUtil *iu ) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

//----------------------------------------------------------------------------
// Blender_Class::SelectionSetChanged
//----------------------------------------------------------------------------

void Blender_Class::SelectionSetChanged ( Interface * ip, IUtil * iu )
{
	if ( ip->GetSelNodeCount () == 0 )
	{
		EnableWindow ( GetDlgItem ( hPanel, ID_APPLY ), false );
	}
	else
	{
		EnableWindow ( GetDlgItem ( hPanel, ID_APPLY ), true );
	}
}

//----------------------------------------------------------------------------
// Blender_Class::Init
//----------------------------------------------------------------------------

void Blender_Class::Init ( HWND hWnd )
{
	if ( ip->GetSelNodeCount () == 0 )
	{
		EnableWindow ( GetDlgItem ( hWnd, ID_APPLY ), false );
	}
	else
	{
		EnableWindow ( GetDlgItem ( hWnd, ID_APPLY ), true );
	}

	int ticks_per_frame = GetTicksPerFrame ();

	int start_frame   = ip->GetAnimRange ().Start () / ticks_per_frame;
	int end_frame     = ip->GetAnimRange ().End () / ticks_per_frame;

	first_frame = start_frame;
	first_match = start_frame;
	last_frame  = end_frame;
	last_match  = start_frame;

	first_frame_spin = SetupIntSpinner
	(
		hWnd,
		IDC_FIRST_SPIN,
		IDC_FIRST_EDIT,
		-65536,
		65535,
		first_frame
	);

	first_frame_spin->SetResetValue ( first_frame );

	first_match_spin = SetupIntSpinner
	(
		hWnd,
		IDC_FIRST_MATCH_SPIN,
		IDC_FIRST_MATCH_EDIT,
		-65536,
		65535,
		first_match
	);

	first_match_spin->SetResetValue ( first_match );

	last_frame_spin = SetupIntSpinner
	(
		hWnd,
		IDC_LAST_SPIN,
		IDC_LAST_EDIT,
		-65536,
		65535,
		last_frame
	);

	last_frame_spin->SetResetValue ( last_frame );

	last_match_spin = SetupIntSpinner
	(
		hWnd,
		IDC_LAST_MATCH_SPIN,
		IDC_LAST_MATCH_EDIT,
		-65536,
		65535,
		last_match
	);

	last_match_spin->SetResetValue ( last_match );
}

//----------------------------------------------------------------------------
// Blender_Class::Destroy
//----------------------------------------------------------------------------

void Blender_Class::Destroy(HWND hWnd)
{
}

//----------------------------------------------------------------------------
// Blender_Class::Get_Active_Time_Range
//----------------------------------------------------------------------------

void Blender_Class::Get_Active_Time_Range ()
{
	int ticks_per_frame = GetTicksPerFrame ();

	int start_frame   = ip->GetAnimRange ().Start () / ticks_per_frame;
	int end_frame     = ip->GetAnimRange ().End () / ticks_per_frame;

	first_frame_spin->SetValue ( start_frame, false );
	last_frame_spin->SetValue  ( end_frame,   false );
	first_match_spin->SetValue ( start_frame, false );
	last_match_spin->SetValue  ( start_frame, false );
}

//----------------------------------------------------------------------------
// Blender_Class::Blend_Keys
//----------------------------------------------------------------------------

static const Quat zero (0.0,0.0,0.0,1.0);

void Blender_Class::Blend_Keys ()
{
	Interval interval; // Used for returned validity intervals (which are not used).

	int ticks_per_frame = GetTicksPerFrame ();

	TimeValue start_time  = first_frame_spin->GetIVal () * ticks_per_frame;
	TimeValue end_time    = last_frame_spin->GetIVal  () * ticks_per_frame;
	TimeValue start_match = first_match_spin->GetIVal () * ticks_per_frame;
	TimeValue end_match   = last_match_spin->GetIVal  () * ticks_per_frame;

	float t_scale = 1.0f / (float) (end_time - start_time);

	BOOL bad_controller_found = false;

	theHold.Begin ();

	SetCursor ( LoadCursor (NULL, IDC_WAIT) );
	SuspendAnimate ();
	AnimateOn ();

	int number_of_nodes = ip->GetSelNodeCount ();

	for ( int i = 0; i < number_of_nodes; ++ i )
	{
		// Get the inode.

		INode * inode = ip->GetSelNode ( i );

		Control * tm_controller = inode->GetTMController ();
		if ( tm_controller == NULL )
			continue;

		Quat rot_start_delta = zero;
		Quat rot_end_delta   = zero;

		Blender_Data_Chunk data;
		data.position_delta = Point3 (0.0f, 0.0f, 0.0f);
		data.heading_delta  = 0.0f;

		Control * c;

		//--------------------------------------------------------------------
		// Get the rotation controller and change its keys.

		c = tm_controller->GetRotationController ();

		if ( c != NULL )
		{
			if ( c->ClassID () != Class_ID (TCBINTERP_ROTATION_CLASS_ID, 0) )
			{
#if 0
				char m [ 256 ];
				sprintf ( m, "Node %s has rotation controller ID %u.",
					inode->GetName (), c->ClassID ().PartA () );
				MessageBox ( GetActiveWindow (), m, "Debug", MB_OK );
#endif
				bad_controller_found = true;
			}
			else
			{
				Quat actual_start_orientation;
				Quat desired_start_orientation;
				Quat actual_end_orientation;
				Quat desired_end_orientation;

				c->GetValue ( start_time,  & actual_start_orientation,  interval );
				c->GetValue ( start_match, & desired_start_orientation, interval );
				c->GetValue ( end_time,    & actual_end_orientation,    interval );
				c->GetValue ( end_match,   & desired_end_orientation,   interval );

				// Ensure there are keys at the beginning and end of the blend interval.

				c->SetValue ( start_time,  & actual_start_orientation );
				c->SetValue ( end_time,    & actual_end_orientation   );

				actual_end_orientation.MakeClosest ( actual_start_orientation );
				desired_start_orientation.MakeClosest ( actual_start_orientation );
				desired_end_orientation.MakeClosest ( actual_end_orientation );

				rot_start_delta = desired_start_orientation / actual_start_orientation;
				rot_end_delta   = desired_end_orientation   / actual_end_orientation;

				data.heading_delta = Heading_Delta_From_Quat
					( actual_end_orientation / actual_start_orientation );

#if 1
				int number_of_keys = c->NumKeys ();

				for ( int j = 0; j < number_of_keys; ++ j )
				{
					TimeValue key_time = c->GetKeyTime ( j );

					if ( key_time >= start_time && key_time <= end_time )
					{
						Quat orientation;

						c->GetValue ( key_time, & orientation, interval );

						float t = (float) (key_time - start_time) * t_scale;

						Quat delta = Slerp ( rot_start_delta, zero, t ) *
							Slerp ( zero, rot_end_delta, t );

						orientation = orientation * delta;

						c->SetValue ( key_time, & orientation );
					}
				}
#else
				IKeyControl * keys = GetKeyControlInterface ( c );

				if ( keys != NULL )
				{
					int number_of_keys = keys->GetNumKeys ();

					Quat prev_key_absolute (0.0,0.0,0.0,1.0);
					Quat new_prev_key_absolute (0.0,0.0,0.0,1.0);

					for ( int j = 0; j < number_of_keys; ++ j )
					{
						ITCBRotKey key;

						keys->GetKey ( j, & key );

						Quat this_key_absolute = prev_key_absolute * (Quat) key.val;
						Quat new_key_absolute = this_key_absolute;

						if ( key.time >= start_time && key.time <= end_time )
						{
							float t = (float) (key.time - start_time) * t_scale;

							Quat delta = Slerp ( rot_start_delta, zero, t ) *
								Slerp ( zero, rot_end_delta, t );

							new_key_absolute = this_key_absolute * delta;

							key.val = new_key_absolute / new_prev_key_absolute;

							keys->SetKey ( j, & key );
						}

						prev_key_absolute = this_key_absolute;
						new_prev_key_absolute = new_key_absolute;
					}
				}
#endif
			}
		}

		//--------------------------------------------------------------------
		// Get the position controller and change its keys.

		c = tm_controller->GetPositionController ();

		if ( c != NULL )
		{
			if ( c->ClassID () != Class_ID (TCBINTERP_POSITION_CLASS_ID, 0) )
			{
				bad_controller_found = true;
			}
			else
			{
				Point3 actual_start_position;
				Point3 desired_start_position;
				Point3 actual_end_position;
				Point3 desired_end_position;

				c->GetValue ( start_time,  & actual_start_position,  interval );
				c->GetValue ( start_match, & desired_start_position, interval );
				c->GetValue ( end_time,    & actual_end_position,    interval );
				c->GetValue ( end_match,   & desired_end_position,   interval );

				// Ensure there are keys at the beginning and end of the blend interval.

				c->SetValue ( start_time,  & actual_start_position );
				c->SetValue ( end_time,    & actual_end_position   );

				data.position_delta = actual_end_position - actual_start_position;

#if 1
				int number_of_keys = c->NumKeys ();

				for ( int j = 0; j < number_of_keys; ++ j )
				{
					TimeValue key_time = c->GetKeyTime ( j );

					if ( key_time >= start_time && key_time <= end_time )
					{
						Point3 position;

						c->GetValue ( key_time, & position, interval );

						float t = (float) (key_time - start_time) * t_scale;

						// Calculate the position the node would be in if it
						// traveled in a straight line.

						Point3 actual_position = actual_start_position * (1.0f - t) +
							actual_end_position * t;

						// Find the delta between the straight-line position and
						// the real position.

						Point3 delta = position - actual_position;

						// Rotate the delta by the change in orientation
						// at this time.

						Quat rot_delta = Slerp ( rot_start_delta, zero, t ) *
							Slerp ( zero, rot_end_delta, t );
						Matrix3 rot_matrix;
						rot_delta.MakeMatrix ( rot_matrix );

						delta = delta * rot_matrix;

						// Add the delta to the straight-line position on the
						// desired line.

						Point3 desired_position = desired_start_position * (1.0f - t) +
							desired_end_position * t;

						position = desired_position + delta;

						// Store the new key value.

						c->SetValue ( key_time, & position );
					}
				}
#else
				IKeyControl * keys = GetKeyControlInterface ( c );

				if ( keys != NULL )
				{
					int number_of_keys = keys->GetNumKeys ();

					for ( int j = 0; j < number_of_keys; ++ j )
					{
						ITCBPoint3Key key;

						keys->GetKey ( j, & key );

						if ( key.time >= start_time && key.time <= end_time )
						{
							float t = (float) (key.time - start_time) * t_scale;

							// Calculate the position the node would be in if it
							// traveled in a straight line.

							Point3 actual_position = actual_start_position * (1.0f - t) +
								actual_end_position * t;

							// Find the delta between the straight-line position and
							// the real position.

							Point3 delta = key.val - actual_position;

							// Rotate the delta by the change in orientation
							// at this time.

							Quat rot_delta = Slerp ( rot_start_delta, zero, t ) *
								Slerp ( zero, rot_end_delta, t );
							Matrix3 rot_matrix;
							rot_delta.MakeMatrix ( rot_matrix );

							delta = delta * rot_matrix;

							// Add the delta to the straight-line position on the
							// desired line.

							Point3 desired_position = desired_start_position * (1.0f - t) +
								desired_end_position * t;

							key.val = desired_position + delta;

							// Store the new key value.

							keys->SetKey ( j, & key );
						}
					}
				}
#endif
			}
		}

		// If this node is a root node (its parent is not selected), attach
		// a data chunk that describes its motion during the blended section.

		if ( Is_Root ( inode ) )
			Set_Data_Chunk ( inode, data );
		else
			Remove_Data_Chunk ( inode );

		inode->InvalidateTM ();
	}

	ResumeAnimate ();

	TSTR undostr;
	undostr.printf ( "Blend Keys" );
	theHold.Accept ( undostr );
	SetCursor ( LoadCursor (NULL, IDC_ARROW) );

	ip->RedrawViews ( ip->GetTime () );

	if ( bad_controller_found )
	{
		MessageBox ( GetActiveWindow (),
			"Warning: Some controllers were not blended\n"
			"because they were not TCB controllers.", "Warning", MB_OK );
	}
}

//----------------------------------------------------------------------------
// Blender_Class::Is_Root
//----------------------------------------------------------------------------

BOOL Blender_Class::Is_Root ( INode * node )
{
	node = node->GetParentNode ();

	int number_of_nodes = ip->GetSelNodeCount ();

	for ( int i = 0; i < number_of_nodes; ++ i )
	{
		if ( ip->GetSelNode ( i ) == node )
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------
// Blender_Class::Heading_Delta_From_Quat
//----------------------------------------------------------------------------

float Blender_Class::Heading_Delta_From_Quat ( Quat q )
{
	Matrix3 rot_matrix;
	q.MakeMatrix ( rot_matrix );

	Point3 row = rot_matrix.GetRow ( 0 );

	return (float) atan2 ( row.y, row.x );
}

//----------------------------------------------------------------------------
// Blender_Class::Set_Data_Chunk
//----------------------------------------------------------------------------

void Blender_Class::Set_Data_Chunk
(
	INode * node,
	const Blender_Data_Chunk & new_data
)
{
	AppDataChunk * chunk = node->GetAppDataChunk
	(
		Blender_Class_ID,
		UTILITY_CLASS_ID,
		1
	);

	Blender_Data_Chunk * data;

	if ( chunk != NULL )
	{
		data = (Blender_Data_Chunk *) chunk->data;
	}
	else
	{
		data = (Blender_Data_Chunk *) malloc ( sizeof (Blender_Data_Chunk) );

		node->AddAppDataChunk
		(
			Blender_Class_ID,
			UTILITY_CLASS_ID,
			1,
			sizeof (Blender_Data_Chunk),
			data
		);
	}

	*data = new_data;

#if 0
	char m [ 256 ];
	sprintf ( m, "Setting data chunk for %s:\n"
		"Position delta = (%.1f, %.1f, %.1f)\n"
		"Heading delta = %.1f",
		node->GetName (),
		data->position_delta.x,
		data->position_delta.y,
		data->position_delta.z,
		data->heading_delta * 180 / PI );
	MessageBox ( GetActiveWindow (), m, "Debug", MB_OK );
#endif
}

//----------------------------------------------------------------------------
// Blender_Class::Remove_Data_Chunk
//----------------------------------------------------------------------------

void Blender_Class::Remove_Data_Chunk ( INode * node )
{
	node->RemoveAppDataChunk
	(
		Blender_Class_ID,
		UTILITY_CLASS_ID,
		1
	);
}

//----------------------------------------------------------------------------
// Blender_Class::Loop_Controllers
//----------------------------------------------------------------------------

void Blender_Class::Loop_Controllers ()
{
	int number_of_nodes = ip->GetSelNodeCount ();

	for ( int i = 0; i < number_of_nodes; ++ i )
	{
		// Get the inode.

		INode * inode = ip->GetSelNode ( i );

		Control * tm_controller = inode->GetTMController ();
		if ( tm_controller == NULL )
			continue;

		Control * c;

		c = tm_controller->GetRotationController ();
		if ( c )
		{
			c->SetORT ( ORT_LOOP, ORT_BEFORE );
			c->SetORT ( ORT_LOOP, ORT_AFTER );
		}

		c = tm_controller->GetPositionController ();
		if ( c )
		{
			c->SetORT ( ORT_LOOP, ORT_BEFORE );
			c->SetORT ( ORT_LOOP, ORT_AFTER );
		}

		c = tm_controller->GetScaleController ();
		if ( c )
		{
			c->SetORT ( ORT_LOOP, ORT_BEFORE );
			c->SetORT ( ORT_LOOP, ORT_AFTER );
		}
	}
}
