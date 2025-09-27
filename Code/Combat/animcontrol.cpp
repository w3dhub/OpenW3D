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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/animcontrol.cpp                       $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/04/02 10:26a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 53                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "animcontrol.h"
#include "assets.h"
#include "debug.h"
#include "rendobj.h"
#include "chunkio.h"
#include "saveload.h"
#include "htree.h"

/*
**
*/
enum	{
	XXXCHUNKID_LEGS							=	910991512,
	XXXCHUNKID_TORSO,
	CHUNKID_CHANNEL,
	CHUNKID_VARIABLES,
	CHUNKID_OLD,
	CHUNKID_NEW,
	CHUNKID_PARENT,
	CHUNKID_CHANNEL1,
	CHUNKID_CHANNEL2,

	MICROCHUNKID_BLEND_TIMER			= 1,	
	MICROCHUNKID_BLEND_TOTAL,
	MICROCHUNKID_FRAME,
	XXXMICROCHUNKID_WEIGHT,
	MICROCHUNKID_MODE,
	MICROCHUNKID_ANIMATION_NAME,
	MICROCHUNKID_MODEL_PTR,
	MICROCHUNKID_CHANNEL2_RATIO,
	MICROCHUNKID_TARGET_FRAME,
	MICROCHUNKID_SKELETON,
};

/*
** AnimChannelClass
*/
AnimChannelClass::AnimChannelClass( void ) :
	Animation( NULL ),
	Frame( 0.0f ),
	NumFrames( 1 ),
	TargetFrame( 0.0f ),
	Mode( ANIM_MODE_ONCE )
{
}

AnimChannelClass::~AnimChannelClass( void )
{
	if ( Animation ) {
		Animation->Release_Ref();
		Animation = NULL;
	}
}

AnimChannelClass & AnimChannelClass::operator = (const AnimChannelClass & src)
{
	if (Animation != NULL) {
		Animation->Release_Ref();
	}
	Animation = src.Animation;
	if (Animation != NULL) {
		Animation->Add_Ref();
	}
	Frame			= src.Frame;
	NumFrames	= src.NumFrames;
	Mode			= src.Mode;
	TargetFrame	= src.TargetFrame;
	return *this;
}

bool 	AnimChannelClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		// save the anim first, because the load will stomp the frame & weight
		if ( Animation ) {
			csave.Begin_Micro_Chunk( MICROCHUNKID_ANIMATION_NAME );
			const char * anim_name = Animation->Get_Name();
			csave.Write( anim_name, strlen( anim_name ) + 1);
			csave.End_Micro_Chunk();
		}
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FRAME, Frame );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MODE, Mode );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TARGET_FRAME, TargetFrame );
	csave.End_Chunk();

	return true;
}

bool	AnimChannelClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_FRAME, Frame );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MODE, Mode );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TARGET_FRAME, TargetFrame );

						case MICROCHUNKID_ANIMATION_NAME:
						{
							char anim_name[80];
							cload.Read( anim_name, cload.Cur_Micro_Chunk_Length() );
							Set_Animation( anim_name );
							break;
						}
						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

void AnimChannelClass::Set_Animation( const char *name )
{
	// If this is our current anim, bail
	if ( ( Animation != NULL ) && ( name != NULL ) ) {
		if ( stricmp( Animation->Get_Name(), name ) == 0 ) {
			return;
		}
	}

	if ( ( Animation == NULL ) && ( name == NULL ) ) {
		return;
	}

	// Release the old anim
	if ( Animation ) {
		Animation->Release_Ref();
		Animation = NULL;
	}

	// we need to switch anims
	if (( name != NULL ) && ( name[0] != 0 ) ) {
		Animation = WW3DAssetManager::Get_Instance()->Get_HAnim( name );
	}

	if ( Animation ) {
		SET_REF_OWNER( Animation );
		NumFrames = Animation->Get_Num_Frames();
		Mode = ANIM_MODE_ONCE;
		Frame = 0;
		TargetFrame = 0;
	}
}

void AnimChannelClass::Set_Animation( const HAnimClass *anim )
{
	// If this is our current anim, bail
	if ( Animation == anim ) {
		return;
	}

	// Release the old anim
	if ( Animation ) {
		Animation->Release_Ref();
		Animation = NULL;
	}

	// we need to switch anims
	Animation = (HAnimClass *)anim;

	if ( Animation ) {
		Animation->Add_Ref();
		NumFrames = Animation->Get_Num_Frames();
		Mode = ANIM_MODE_ONCE;
		Frame = 0;
		TargetFrame = 0;
	}
}

void	AnimChannelClass::Set_Mode( AnimMode mode, float frame )
{ 
	Mode = mode; 
	if ( frame >= 0 ) {
		Frame = frame; 
	}
}

void AnimChannelClass::Update( float dtime )
{
	if ( Mode == ANIM_MODE_STOP ) {
		return;
	}

	if ( Animation != NULL ) {

		switch ( Mode )
		{		
			case ANIM_MODE_LOOP:
				
				//
				//	Increment the frame based on the current timeslice
				//
				Frame += dtime * Animation->Get_Frame_Rate();
				
				//
				//	Handle wrapping
				//
				if ( Frame >= NumFrames-1 ) {
					Frame -= NumFrames-1;
				}

				if ( Frame >= NumFrames ) {
					Frame = 0;
				}
				break;

			case ANIM_MODE_TARGET:
			
				//
				//	Which direction are we animating?
				//
				if ( Frame < TargetFrame ) {
					Frame += dtime * Animation->Get_Frame_Rate();
					
					//
					// If we overshoot targetframe, snap to targetframe
					//
					if (Frame >= TargetFrame) {								
						Frame = TargetFrame;
					}

				} else if ( Frame > TargetFrame ) {
					Frame -= dtime * Animation->Get_Frame_Rate();
					
					//
					// If we overshoot targetframe, snap to targetframe
					//
					if ( Frame <= TargetFrame ) {
						Frame = TargetFrame;
					}
				}
				break;

			case ANIM_MODE_ONCE:

				//
				//	Increment the frame based on the current timeslice
				//
				Frame += dtime * Animation->Get_Frame_Rate();

				//
				//	Make sure we don't go past the end
				//
				if ( Frame > NumFrames-1 ) {
					Frame = NumFrames-1;
				}
				break;
		}

#if 0
if ( dtime != 0 ) {
	Debug_Say(( "Anim %s frame %1.3f\n", Animation->Get_Name(), Frame ));
}
#endif

	}

}

void	AnimChannelClass::Get_Animation_Data( AnimationDataList & list, float weight )
{
	if ( Animation != NULL && weight > 0 ) {
		AnimationDataRecord * record = list.Uninitialized_Add();
		WWASSERT( record != NULL );
		record->Animation = Animation;
		record->Frame = Frame;
		record->Weight = weight;
	}
}

void	AnimChannelClass::Update_Model( RenderObjClass	*anim_model )
{
	if ( Animation ) {
		anim_model->Set_Animation( Animation, Frame );
	} else {
		anim_model->Set_Animation();
	}
}


/*
** BlendableAnimChannelClass
*/
BlendableAnimChannelClass::BlendableAnimChannelClass( void ) :
	BlendTimer( 0 ),
	BlendTotal( 0 )
{
}

bool 	BlendableAnimChannelClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_NEW );
	NewChannel.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_OLD );
	OldChannel.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_BLEND_TIMER, BlendTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_BLEND_TOTAL, BlendTotal );
	csave.End_Chunk();

	return true;
}

bool	BlendableAnimChannelClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_NEW:
				NewChannel.Load( cload );
				break;
								
			case CHUNKID_OLD:
				OldChannel.Load( cload );
				break;
								
			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_BLEND_TIMER, BlendTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_BLEND_TOTAL, BlendTotal );
						default:
							Debug_Say(( "Unrecognized BlendableAnimChannel Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized BlendableAnimChannel chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

void	BlendableAnimChannelClass::Set_Animation( const char *name, float blendtime, float start_frame ) 
{
	// if setting to our current anim, bail
	if ( ( NewChannel.Peek_Animation() == NULL ) && ( name == NULL ) ) {
		return;
	}

	if ( ( NewChannel.Peek_Animation() != NULL ) && ( name != NULL ) ) {
		if ( stricmp( NewChannel.Peek_Animation()->Get_Name(), name ) == 0 ) {
			return;
		}
	}

	// if no current channel, or no blend, or no new name, don't blend
	if ( (NewChannel.Peek_Animation() == NULL) || (blendtime == 0) || (name == NULL) ) {
		BlendTotal = 0.0f;
		BlendTimer = 0.0f;
	} else if ( BlendTotal == 0.0 ) {						//if not currently blending
		OldChannel = NewChannel;	
		BlendTimer = 0.0f;
		BlendTotal = blendtime;
#if 0
	} else if ( OldChannel.Peek_Animation() == new_anim.Peek_Animation() ) {	// if old anim is the new one
		OldChannel.Copy_From( NewChannel );	
		BlendTimer = (1.0f - (BlendTimer / BlendTotal)) * blendtime;
		BlendTotal = blendtime;
#endif
	} else if ( (BlendTimer / BlendTotal) > 0.5 ) {	// if more than halfway through the old blend
		OldChannel = NewChannel;	
		BlendTimer = (1.0f - (BlendTimer / BlendTotal)) * blendtime;
		BlendTotal = blendtime;
	} else {
		BlendTimer = (BlendTimer / BlendTotal) * blendtime;
		BlendTotal = blendtime;
	}
	NewChannel.Set_Animation( name );
	if ( NewChannel.Peek_Animation() != NULL ) {
		NewChannel.Set_Frame( start_frame );
	}
	if ( name == NULL ) {
		OldChannel.Set_Animation( (const char *)NULL );
	}
}

void	BlendableAnimChannelClass::Set_Animation( const HAnimClass * anim, float blendtime, float start_frame )
{
	// if setting to our current anim, bail
	if ( ( NewChannel.Peek_Animation() == NULL ) && ( anim == NULL ) ) {
		return;
	}

	if ( ( NewChannel.Peek_Animation() != NULL ) && ( anim != NULL ) ) {
		if ( NewChannel.Peek_Animation() == anim ) {
			return;
		}
	}

	// if no current channel, or no blend, or no new name, don't blend
	if ( (NewChannel.Peek_Animation() == NULL) || (blendtime == 0) || (anim == NULL) ) {
		BlendTotal = 0.0f;
		BlendTimer = 0.0f;
	} else if ( BlendTotal == 0.0 ) {						//if not currently blending
		OldChannel = NewChannel;	
		BlendTimer = 0.0f;
		BlendTotal = blendtime;
#if 0
	} else if ( OldChannel.Peek_Animation() == new_anim.Peek_Animation() ) {	// if old anim is the new one
		OldChannel.Copy_From( NewChannel );	
		BlendTimer = (1.0f - (BlendTimer / BlendTotal)) * blendtime;
		BlendTotal = blendtime;
#endif
	} else if ( (BlendTimer / BlendTotal) > 0.5 ) {	// if more than halfway through the old blend
		OldChannel = NewChannel;	
		BlendTimer = (1.0f - (BlendTimer / BlendTotal)) * blendtime;
		BlendTotal = blendtime;
	} else {
		BlendTimer = (BlendTimer / BlendTotal) * blendtime;
		BlendTotal = blendtime;
	}
	NewChannel.Set_Animation( anim );
	if ( NewChannel.Peek_Animation() != NULL ) {
		NewChannel.Set_Frame( start_frame );
	}
	if ( anim == NULL ) {
		OldChannel.Set_Animation( (const HAnimClass *)NULL );
	}
}


void	BlendableAnimChannelClass::Update( float dtime ) 
{
	if ( BlendTotal != 0.0f )	{			// if blending between two animations
		BlendTimer += dtime;	// Bump blend timer forward
		if ( BlendTimer >= BlendTotal )	// blend complete, remove oldanim
		{
			BlendTotal = 0.0f;
			BlendTimer = 0.0f;
			OldChannel.Set_Animation( (const char *)NULL );
		}
	}

	// Calculate which frame we are on in each of the animations
	NewChannel.Update( dtime );
	OldChannel.Update( dtime );
}

void	BlendableAnimChannelClass::Get_Animation_Data( AnimationDataList & list, float weight )
{
	float blend_ratio = 1.0f;				// assume no blending
	if ( BlendTotal != 0.0f )	{			// if blending between two animations
		// Calculate the blend percentage between the two animations.
		// This starts at 0.0 (all OldAnimation) and proceeds to 1.0 (all Animation)
		blend_ratio = WWMath::Clamp( BlendTimer / BlendTotal, 0, 1 );
	}

	NewChannel.Get_Animation_Data( list, weight * blend_ratio );
	OldChannel.Get_Animation_Data( list, weight * ( 1 - blend_ratio ) );
}


void	BlendableAnimChannelClass::Update_Model( RenderObjClass	*anim_model )
{
	float blend_ratio = 1.0f;				// assume no blending
	if ( BlendTotal != 0.0f )	{			// if blending between two animations
		// Calculate the blend percentage between the two animations.
		// This starts at 0.0 (all OldAnimation) and proceeds to 1.0 (all Animation)
		blend_ratio = WWMath::Clamp( BlendTimer / BlendTotal, 0, 1 );
	}

	if ( OldChannel.Peek_Animation() ) {
		anim_model->Set_Animation(	OldChannel.Peek_Animation(),	OldChannel.Get_Frame(),
										NewChannel.Peek_Animation(),	NewChannel.Get_Frame(),			
										blend_ratio );
	} else if ( NewChannel.Peek_Animation() ) {
		anim_model->Set_Animation( NewChannel.Peek_Animation(), NewChannel.Get_Frame() );
	} else {
		anim_model->Set_Animation();
	}
}

/*
** AnimControlClass
*/
AnimControlClass::AnimControlClass( void )  :	
	Model( NULL ) 
{ 
}

AnimControlClass::~AnimControlClass( void ) 	
{ 
	REF_PTR_RELEASE( Model ); 
}

bool 	AnimControlClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MODEL_PTR, Model );
	csave.End_Chunk();

	return true;
}

bool	AnimControlClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:

				WWASSERT( Model == NULL );
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MODEL_PTR, Model );

						default:
							Debug_Say(( "Unrecognized AnimControl Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}

				if ( Model != NULL ) {
					REQUEST_REF_COUNTED_POINTER_REMAP( (RefCountClass **)&Model );
				} else {
					Debug_Say(( "Loaded NULL model\n" ));
				}

				break;

			default:
				Debug_Say(( "Unrecognized AnimControl chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

void	AnimControlClass::Set_Model( RenderObjClass	*anim_model ) 
{ 
	REF_PTR_SET( Model, anim_model ); 
}

/*
** SimpleAnimControlClass
*/
SimpleAnimControlClass::SimpleAnimControlClass( void ) 
{
}

SimpleAnimControlClass::~SimpleAnimControlClass( void )
{
}

bool 	SimpleAnimControlClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
	AnimControlClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_CHANNEL );
	Channel.Save( csave );
	csave.End_Chunk();
	return true;
}

bool	SimpleAnimControlClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				AnimControlClass::Load( cload );
				break;
								
			case CHUNKID_CHANNEL:
				Channel.Load( cload );
				break;
								
			default:
				Debug_Say(( "Unrecognized HumanAnimControl chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

void	SimpleAnimControlClass::Set_Animation( const char *name, float blendtime, float start_frame )
{
	Channel.Set_Animation( name, blendtime, start_frame );
}

void	SimpleAnimControlClass::Set_Animation( const HAnimClass * anim, float blendtime, float start_frame )
{
	Channel.Set_Animation( anim, blendtime, start_frame );
}

void	SimpleAnimControlClass::Update( float dtime )
{
	Channel.Update( dtime );

	// Setup the model for the current frame(s)
	assert( Model != NULL );

	Channel.Update_Model( Model );
}

/*
** HumanAnimControlClass
*/
HumanAnimControlClass::HumanAnimControlClass( void ) :
	AnimCombo( 2 ),
	Channel2Ratio( 0 ),
	Skeleton( 'A' ),
	AnimSpeedScale( 1 )
{
}

HumanAnimControlClass::~HumanAnimControlClass( void )
{
}

bool 	HumanAnimControlClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
	AnimControlClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CHANNEL2_RATIO, Channel2Ratio );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SKELETON, Skeleton );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_CHANNEL1 );
	Channel1.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_CHANNEL2 );
	Channel2.Save( csave );
	csave.End_Chunk();

//	Don't need to save the animcombo

	return true;
}

bool	HumanAnimControlClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				AnimControlClass::Load( cload );
				break;
								
			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CHANNEL2_RATIO, Channel2Ratio );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SKELETON, Skeleton );

						default:
							Debug_Say(( "Unrecognized HumanAnimControl Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_CHANNEL1:
				Channel1.Load( cload );
				break;
								
			case CHUNKID_CHANNEL2:
				Channel2.Load( cload );
				break;
								
			default:
				Debug_Say(( "Unrecognized HumanAnimControl chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

void HumanAnimControlClass::Build_Skeleton_Anim_Name( StringClass& new_name, const char * name )
{
	if (name == NULL) return;
	if (*name == NULL) {
		Debug_Say(( "No name in Build_Skeleton_Anim_Name\n" ));
		return;
	}
	new_name = name;

	if ( Skeleton == 'V' ) {	// Special case for visceroids
		return;
	}

	// If the anim doesn't start with "S_A_HUMAN.", add it
    if ( strnicmp( name, "S_", 2 ) != 0 ) {
		new_name.Format( "S_%c_HUMAN.%s", Skeleton, name );
	}

	// If the anim name is "S_A_HUMAN.H_A_*", and the Skeleton is not 'A', use
	// the other skeleton anim, if found
    if ( new_name.Get_Length() > 14 && Skeleton != 'A' && strnicmp( new_name, "S_A_HUMAN.H_A_", 14 ) == 0 ) {
		StringClass	mod_name(new_name,true);
		mod_name[2] = Skeleton;
		mod_name[12] = Skeleton;

		// can we find the anim name?
		HAnimClass * anim = WW3DAssetManager::Get_Instance()->Get_HAnim( mod_name );
		if ( anim != NULL ) {
			anim->Release_Ref();
			new_name=mod_name;
		}
	}
}

void	HumanAnimControlClass::Set_Animation( const char *name, float	blendtime, float start_frame )
{
	StringClass new_name(0,true);
	Build_Skeleton_Anim_Name( new_name, name );

	Channel1.Set_Animation( new_name, blendtime, start_frame );
	Channel2.Set_Animation( (const char *)NULL );
	Channel2Ratio = 0;
}

void	HumanAnimControlClass::Set_Animation( const HAnimClass * anim, float blendtime, float start_frame )
{
	if ( anim != NULL ) {
		Set_Animation( anim->Get_Name(), blendtime, start_frame );
	} else {
		Set_Animation( (const char *)NULL, blendtime, start_frame );
	}
}

void	HumanAnimControlClass::Set_Animation( const char *name1, const char * name2, float ratio, float blendtime )
{
	StringClass new_name1(0,true);
	Build_Skeleton_Anim_Name( new_name1, name1 );
	StringClass new_name2(0,true);
	Build_Skeleton_Anim_Name( new_name2, name2 );

	if ( ratio == 0 ) {
		Set_Animation( new_name1, blendtime );
		return;
	}
	if ( Channel2Ratio == 0 ) {
		Channel2 = Channel1;
	}
	Channel1.Set_Animation( new_name1, blendtime );
	Channel2.Set_Animation( new_name2, blendtime );
	Channel2Ratio = ratio;
}

void	HumanAnimControlClass::Set_Mode( AnimMode mode, float frame )
{
	Channel1.Set_Mode( mode, frame );
	Channel2.Set_Mode( mode, frame );
}

void	HumanAnimControlClass::Set_Model( RenderObjClass	*anim_model )
{
	AnimControlClass::Set_Model( anim_model );

	// Update the skeleton
	if( anim_model != NULL ) {
		const HTreeClass * tree = anim_model->Get_HTree();
		if ( tree != NULL ) {
			const char * name = tree->Get_Name();
			Skeleton = name[2];
		}
	}
}


/*
**
*/
StringClass	_MonitorAnimDescription;
HumanAnimControlClass * _Monitor = NULL;

void	HumanAnimControlClass::Update( float dtime )
{
	// Update channels
	Channel1.Update( dtime * AnimSpeedScale );
	Channel2.Update( dtime * AnimSpeedScale );

	if ( Model != NULL ) {

		// Get Animation data
		DataList.Reset_Active();
		Channel1.Get_Animation_Data( DataList, (1 - Channel2Ratio) );
		Channel2.Get_Animation_Data( DataList, Channel2Ratio );

		// Use the cheapest anim method possible
		int total_animations = DataList.Count();

		if ( total_animations == 0 ) {

//			Debug_Say(( "No animations to display!\n" ));
			Model->Set_Animation();

		} else if ( total_animations == 1 ) {

//			Debug_Say(( "1 animation to display!\n" ));
			Model->Set_Animation( DataList[0].Animation, DataList[0].Frame );

		} else if ( total_animations == 2 ) {

//			Debug_Say(( "2 animation to display!\n" ));
			float percent = DataList[1].Weight / (DataList[0].Weight + DataList[1].Weight);
			Model->Set_Animation(	DataList[0].Animation, DataList[0].Frame,
											DataList[1].Animation, DataList[1].Frame, percent );

		} else {

//			Debug_Say(( ">2 animation to display!\n" ));

			// set up anim combo
			AnimCombo.Reset();
			for ( int i = 0; i < total_animations; i++ ) {
				HAnimComboDataClass * anim_data = new HAnimComboDataClass();
				anim_data->Set_HAnim( DataList[i].Animation );
				anim_data->Set_Frame( DataList[i].Frame );
				anim_data->Set_Weight( DataList[i].Weight );
				AnimCombo.Append_Anim_Combo_Data( anim_data );
			}

			// Setup the model for the current frame(s)
			Model->Set_Animation( &AnimCombo );
		}

		if ( _Monitor == this ) {
			for ( int i = 0; i < total_animations; i++ ) {
				if ( i == 0 ) {
					_MonitorAnimDescription = "";
				}
				StringClass a(0,true);
				a.Format( "%s %1.0f%% %1.0f\n", DataList[i].Animation->Get_Name()+10, DataList[i].Weight*100, DataList[i].Frame );
				_MonitorAnimDescription += a;
			}
		}

	}
}

/*
**
*/
void	HumanAnimControlClass::Get_Information( StringClass & string )
{
	_Monitor = this;
	string += _MonitorAnimDescription;
}

