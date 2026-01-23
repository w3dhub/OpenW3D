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
 *                     $Archive:: /Commando/Code/Combat/dynamicspeechanim.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/16/02 7:15p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dynamicspeechanim.h"
#include "assetmgr.h"
#include "crandom.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
VisemeManager	DynamicSpeechAnimClass::VisemeLookupMgr;

////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
enum
{
	CHANNEL0		= 0,
	CHANNEL1,
	CHANNEL_MAX
};

static const int EYEBROW_POSES[] =
{
	1, 3, 4, 5, 6, 7
};

static const int EYEBROW_POSE_COUNT	= sizeof (EYEBROW_POSES) / sizeof (int);

typedef enum
{
	NORMAL					= 0,
	NORMAL_EYELIDS_HALF,
	NORMAL_EYELIDS_DOWN,
	EYEBROWS_DOWN,
	SPOCK_EYEBROW_L,
	SPOCK_EYEBROW_R,
	EYEBROWS_UP,
	EYEBROWS_DOWN_EYELIDS_HALF,
	EYEBROWS_DOWN_EYELIDS_DOWN

}	EYEBROW_POSE_INDICES;


////////////////////////////////////////////////////////////////
//
//	DynamicSpeechAnimClass
//
////////////////////////////////////////////////////////////////
DynamicSpeechAnimClass::DynamicSpeechAnimClass (const char *skeleton_name)
{
	StringClass anim0_name;
	StringClass anim1_name;

	//
	//	Determine which animations to load
	//
	StringClass upper_skel_name = skeleton_name;
    upper_skel_name.To_Upper();
	if (::strstr (upper_skel_name, "S_A_") != NULL) {
		anim0_name.Format ("%s.S_A_MOUTH",			skeleton_name);
		anim1_name.Format ("%s.S_A_EXPRESSION",	skeleton_name);
	} else {
		anim0_name.Format ("%s.S_B_MOUTH",			skeleton_name);
		anim1_name.Format ("%s.S_B_EXPRESSION",	skeleton_name);
	}

	//
	//	Load the animations for each channel
	//
	HAnimClass *animations[CHANNEL_MAX] = { 0 };
	animations[CHANNEL0] = WW3DAssetManager::Get_Instance ()->Get_HAnim (anim0_name);
	animations[CHANNEL1] = WW3DAssetManager::Get_Instance ()->Get_HAnim (anim1_name);

	//
	//	Did we successfully load all the animations?
	//
	bool is_valid = true;
	for (int index = 0; index < CHANNEL_MAX; index ++) {
		if (animations[index] == NULL) {
			is_valid = false;
			break;
		}
	}

	//
	//	If valid, create the morph animation from the channel information
	//
	if (is_valid) {
		Create_New_Morph (CHANNEL_MAX, animations);
	} else {

		//
		//	Release our hold on the animations
		//
		for (int index = 0; index < CHANNEL_MAX; index ++) {
			REF_PTR_RELEASE (animations[index]);
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DynamicSpeechAnimClass
//
////////////////////////////////////////////////////////////////
DynamicSpeechAnimClass::~DynamicSpeechAnimClass (void)
{
	Free_Morph ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Generate_Animation
//
////////////////////////////////////////////////////////////////
bool
DynamicSpeechAnimClass::Generate_Animation (const char *text, float duration)
{
	if (ChannelCount <= 0) {
		return false;
	}

	//
	//	Start fresh
	//
	Release_Keys ();

	//
	// Get the list of mouth poses based on the phrase through the viseme manager	
	//
	bool retval = false;
	int pose_table[256] = { 0 };
	int pose_count = VisemeLookupMgr.Get_Visemes (text, pose_table);
	if (pose_count > 0) {
		retval = true;

		//
		//	Determine how much time each pose should take
		//
		float time_per_pose	= duration / float(pose_count + 1);
		float frame_inc		= time_per_pose * Get_Frame_Rate ();
		float curr_frame		= 0;

		//
		// Always start the animation with the closed mouth
		//
		Insert_Morph_Key (CHANNEL0, 0, 0);	
		curr_frame += frame_inc;

		//
		//	Now add all the poses to the animation
		//
		for (int index = 0; index < pose_count; index ++) {
			Insert_Morph_Key (CHANNEL0, uint32(curr_frame + 0.5F), pose_table[index] + 1);
			curr_frame += frame_inc;
		}

		//
		// Always end the mouth sentence with a closed mouth
		//
		Insert_Morph_Key (CHANNEL0, uint32(curr_frame + (frame_inc * 2) + 0.5F), 0);

		//
		//	Add in some eyebrow animation
		//
		Generate_Eyebrows (duration);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Generate_Eyebrows
//
////////////////////////////////////////////////////////////////
void
DynamicSpeechAnimClass::Generate_Eyebrows (float duration, float /* frequency */)
{
	if (ChannelCount <= 0) {
		return ;
	}

	//
	//	Start with the eyebrows in the "normal" position
	//
	Insert_Morph_Key (CHANNEL1, 0, 0);

	//
	//	Calculate how many frames are in the animation
	//
	int max_frames = int(duration * Get_Frame_Rate ());

	int min_blink_delay = int(0.5F * Get_Frame_Rate ());
	int max_blink_delay = int(3.0F * Get_Frame_Rate ());

	int current_frame	= 0;
	int current_pose	= NORMAL;
	while (current_frame < max_frames) {
		
		//
		//	Wait a random amount of time before we blink
		//
		current_frame += FreeRandom.Get_Int (min_blink_delay, max_blink_delay);

		//
		//	Blink
		//
		if (current_frame < max_frames) {
			current_frame = Insert_Blink (current_frame, EYEBROW_POSES[current_pose]);
		}

		//
		//	Randomly change eyebrow position
		//
		if (current_frame < max_frames && FreeRandom.Get_Int (6) == 2) {

			//
			//	Lock the last pose at this frame
			//			
			Insert_Morph_Key (CHANNEL1, current_frame, EYEBROW_POSES[current_pose]);

			//
			//	Wait a small random amount of time
			//
			float delay = WWMath::Random_Float (0.2F, 0.4F);
			current_frame += int(delay * Get_Frame_Rate ());


			//
			//	Choose a random eyebrow pose
			//			
			current_pose = FreeRandom.Get_Int (EYEBROW_POSE_COUNT);
			Insert_Morph_Key (CHANNEL1, current_frame, EYEBROW_POSES[current_pose]);
		}
	}

	//
	//	End with the eyebrows in the "normal" position
	//
	Insert_Morph_Key (CHANNEL1, max_frames, 0);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Generate_Idle_Animation
//
////////////////////////////////////////////////////////////////
void
DynamicSpeechAnimClass::Generate_Idle_Animation (float duration, float frequency)
{
	if (ChannelCount <= 0) {
		return ;
	}

	//
	//	Start fresh
	//
	Release_Keys ();

	//
	// Make the mouth closed
	//
	Insert_Morph_Key (CHANNEL0, 0, 0);	

	//
	//	Generate 20 seconds worth of animation data for the eyebrows
	//
	Generate_Eyebrows (duration, frequency);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Insert_Blink
//
////////////////////////////////////////////////////////////////
int
DynamicSpeechAnimClass::Insert_Blink (int current_frame, int current_pose)
{
	//
	//	Insert a placeholder key so we don't "slow-mo" blink
	//
	Insert_Morph_Key (CHANNEL1, current_frame, current_pose);
	
	//
	//	Wait a small amount of time
	//
	current_frame += int(0.05F * Get_Frame_Rate ());

	//
	//	Determine which pose to use for "eyes-closed"
	//
	int closed_pose = EYEBROWS_DOWN_EYELIDS_DOWN;
	if (	current_pose == NORMAL ||
			current_pose == NORMAL_EYELIDS_HALF ||
			current_pose == EYEBROWS_UP)
	{
		closed_pose = NORMAL_EYELIDS_DOWN;
	}

	//
	//	Close the eyes
	//
	Insert_Morph_Key (CHANNEL1, current_frame, closed_pose);

	//
	//	Wait a small random amount of time
	//
	float blink_delay = WWMath::Random_Float (0.1F, 0.3F);
	current_frame += int(blink_delay * Get_Frame_Rate ());

	//
	//	Restore the last pose (open-eyed)
	//
	Insert_Morph_Key (CHANNEL1, current_frame, current_pose);
	return current_frame;
}
