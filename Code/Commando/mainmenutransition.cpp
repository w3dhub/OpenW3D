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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/mainmenutransition.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/20/01 7:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "mainmenutransition.h"
#include "camera.h"
#include "stylemgr.h"
#include "assetmgr.h"
#include "hanim.h"
#include "rendobj.h"
#include "ww3d.h"
#include "renegadedialog.h"
#include "render2d.h"
#include "dialogcontrol.h"
#include "dialogbase.h"
#include "WWAudio.h"
#include <algorithm>

////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const float IN_START_FRAME	= 27.0F;
static const float IN_END_FRAME		= 65.0F;
static const float OUT_START_FRAME	= 66.0F;
static const float OUT_END_FRAME		= 99.0F;

static const char *ANIM_NAME			= "IF_TITLETRANS.IF_TITLETRANS";


////////////////////////////////////////////////////////////////
//
//	MainMenuTransitionClass
//
////////////////////////////////////////////////////////////////
MainMenuTransitionClass::MainMenuTransitionClass (void)	:
	Model (NULL),
	Dialog (NULL),
	OtherDlg (NULL),
	Camera (NULL),
	CurrentFrame (0),
	TargetFrame (0),
	TransitionAnim (NULL)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~MainMenuTransitionClass
//
////////////////////////////////////////////////////////////////
MainMenuTransitionClass::~MainMenuTransitionClass (void)
{
	REF_PTR_RELEASE (Model);
	REF_PTR_RELEASE (TransitionAnim);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Dialogs
//
////////////////////////////////////////////////////////////////
void
MainMenuTransitionClass::Set_Dialogs (DialogBaseClass *dialog, DialogBaseClass *other_dlg)
{
	Dialog	= dialog;
	OtherDlg	= other_dlg;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Model
//
////////////////////////////////////////////////////////////////
void
MainMenuTransitionClass::Set_Model (RenderObjClass *model)
{
	REF_PTR_SET (Model, model);

	if (TransitionAnim == NULL) {
		TransitionAnim	= WW3DAssetManager::Get_Instance ()->Get_HAnim (ANIM_NAME);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Type
//
////////////////////////////////////////////////////////////////
void
MainMenuTransitionClass::Set_Type (TYPE type)
{
	DialogTransitionClass::Set_Type (type);

	if (type == SCREEN_IN) {
		CurrentFrame	= IN_START_FRAME;
		TargetFrame		= IN_END_FRAME;
		WWAudioClass::Get_Instance ()->Simple_Play_2D_Sound_Effect ("interface_mainmove.wav", 1.0F, 0.8F);
	} else if (type == SCREEN_OUT) {
		CurrentFrame	= OUT_START_FRAME;
		TargetFrame		= OUT_END_FRAME;
		WWAudioClass::Get_Instance ()->Simple_Play_2D_Sound_Effect ("interface_movezoom.wav", 1.0F, 0.8F);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
MainMenuTransitionClass::On_Frame_Update (void)
{
	if (Model != NULL && TransitionAnim != NULL) {
		
		//
		//	Do we need to keep animating?
		//
		if (CurrentFrame < TargetFrame) {
			
			//
			//	Get the frame rate
			//
			float frame_rate = TransitionAnim->Get_Frame_Rate ();

			//
			// Adjust the current frame counter
			//
			CurrentFrame += (frame_rate * (WW3D::Get_Frame_Time () / 1000.0F));
			CurrentFrame = std::min (CurrentFrame, TargetFrame);

			//
			//	Update the animation
			//
			Model->Set_Animation (TransitionAnim, CurrentFrame);

			//
			//	Move the controls with the animation
			//
			Update_Controls ();

			//
			//	Remove the model if we've finished animating
			//
			if (CurrentFrame >= TargetFrame && Type == SCREEN_OUT) {
				Model->Remove ();
			}
		}	
	}

	DialogTransitionClass::On_Frame_Update ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Controls
//
////////////////////////////////////////////////////////////////
void
MainMenuTransitionClass::Update_Controls (void)
{
	const int ENTRIES = 6;

	const int ControlIDArray[ENTRIES] = 
	{
		IDC_MENU_START_SP_GAME_BUTTON,
		//C_MENU_START_SKIRMISH_GAME_BUTTON,
		IDC_MENU_MP_INTERNET_GAME_BUTTON,
		IDC_MENU_MP_LAN_GAME_BUTTON,
		IDC_MENU_START_PRACTICE_GAME_BUTTON,
		IDC_MENU_OPTIONS_BUTTON,
		IDC_MENU_QUIT_BUTTON
	};

	//
	//	Get the half dimensions of the screen
	//
	const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution ();
	float half_width		= screen_rect.Width () / 2.0F;
	float half_height		= screen_rect.Height () / 2.0F;

	//
	//	Update the position of each control
	//
	for (int index = 0; index < ENTRIES; index ++) {

		//
		//	Get the current position of the bone
		//
		StringClass bone_name;
		bone_name.Format ("IF_MMTF%d", index + 1);
		const Matrix3D &tm = Model->Get_Bone_Transform (bone_name);		

		//
		//	Transform the position into screen space
		//
		Vector3 cam_space_pos (0, 0, 0);
		Camera->Transform_To_View_Space (cam_space_pos, tm.Get_Translation ());

		Vector3 new_pos (0, 0, 0);
		Camera->Project_Camera_Space_Point (new_pos, cam_space_pos);
		
		//
		//	Convert the normalized screen space position to pixel coordinates
		//
		new_pos.X = half_width * (new_pos.X + 1.0F);
		new_pos.Y = half_height * (1.0F - new_pos.Y);

		//
		//	Move each control to the left of the bone a little
		//
		new_pos.X -= StyleMgrClass::Get_X_Scale () * 100.0F;

		//
		//	Move the dialog control
		//
		DialogControlClass *control	= Dialog->Get_Dlg_Item (ControlIDArray[index]);
		const RectClass &control_rect	= control->Get_Window_Rect ();
		new_pos.Y							-= (control_rect.Height () / 2);
		control->Set_Window_Pos (Vector2 (new_pos.X, new_pos.Y));
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Valid
//
////////////////////////////////////////////////////////////////
bool
MainMenuTransitionClass::Is_Valid (void) const
{
	bool retval = false;

	//
	//	Check to see if the model and animation are valid
	//
	if (Model != NULL && TransitionAnim != NULL) {
		retval = true;
	}

	return retval;
}

