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
 *                     $Archive:: /Commando/Code/Combat/animcontrol.h                         $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/04/02 10:33a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 36                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	ANIMCONTROL_H
#define	ANIMCONTROL_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	HANIM_H
	#include "hanim.h"
#endif

#ifndef	VECTOR_H
	#include "vector.h"
#endif

#include "wwstring.h"

/*
**
*/
class		RenderObjClass;

typedef	enum {
	ANIM_MODE_ONCE,
	ANIM_MODE_LOOP,
	ANIM_MODE_STOP,
	ANIM_MODE_TARGET
} AnimMode;

/*
** AnimationDataRecord
*/
struct AnimationDataRecord {
	HAnimClass	*Animation;
	float			Frame;
	float			Weight;

	bool operator == (AnimationDataRecord const & ) const	{ return false; }
	bool operator != (AnimationDataRecord const & ) const	{ return true; }
};

typedef	DynamicVectorClass<AnimationDataRecord>	AnimationDataList;

/*
** AnimChannelClass
*/
class	AnimChannelClass {

public:
	// Constructor and Destructor
	AnimChannelClass( void );
	AnimChannelClass(const AnimChannelClass &);
	AnimChannelClass & operator = (const AnimChannelClass &);
	~AnimChannelClass(void);

	virtual	bool 	Save( ChunkSaveClass & csave );
	virtual	bool	Load( ChunkLoadClass & cload );

	// Set Animation
	void			Set_Animation( const char *name );
	void			Set_Animation( const HAnimClass *anim );
	HAnimClass	*Peek_Animation( void )		{ return Animation; }

	// Set Mode
	void			Set_Mode( AnimMode mode, float frame = -1 );
	AnimMode		Get_Mode( void )					{ return Mode; }
	bool			Is_Complete( void )				{ return (Animation == NULL) || ((Mode == ANIM_MODE_ONCE) && (Frame == NumFrames-1)) || ((Mode == ANIM_MODE_TARGET) && (Frame == TargetFrame)); }
	const char *Get_Animation_Name( void )		{ return Animation ? Animation->Get_Name() : ""; }

	// Frame
	void			Set_Frame( float frame )	{ Frame = frame; }
	float			Get_Frame( void )				{ return Frame; }
	float			Get_Progress( void )			{ return NumFrames ? Frame / NumFrames : 0; }

	void			Set_Target_Frame( float frame )	{ TargetFrame = frame; }
	float			Get_Target_Frame( void )			{ return TargetFrame; }

	// Update the animation
	void			Update( float dtime );

	// Get Animation Data
	void			Get_Animation_Data( AnimationDataList & list, float weight = 1.0f  );

	// Update Model
	void			Update_Model( RenderObjClass	*anim_model );

private:
	HAnimClass *	Animation;
	float			Frame;
	float			NumFrames;
	float			TargetFrame;
	AnimMode		Mode;
};


/*
** BlendableAnimChannelClass
*/
class	BlendableAnimChannelClass {

public:
	// Constructor
	BlendableAnimChannelClass( void );

	virtual	bool 	Save( ChunkSaveClass & csave );
	virtual	bool	Load( ChunkLoadClass & cload );

	// Set Animation
	void			Set_Animation( const char *name, float blendtime = 0.0f, float start_frame = 0.0f );
	void			Set_Animation( const HAnimClass * anim, float blendtime = 0.0f, float start_frame = 0.0f  );
	void			Set_Mode( AnimMode mode, float frame = -1 )	{ NewChannel.Set_Mode( mode, frame ); }
	AnimMode		Get_Mode( void )					{ return NewChannel.Get_Mode(); }
	bool	Is_Complete( void )						{ return NewChannel.Is_Complete(); }
	const char *Get_Animation_Name( void )		{ return NewChannel.Get_Animation_Name(); }

	void			Set_Target_Frame( float frame )	{ NewChannel.Set_Target_Frame ( frame ); }
	float			Get_Target_Frame( void )			{ return NewChannel.Get_Target_Frame(); }

	HAnimClass	*Peek_Animation( void )			{ return NewChannel.Peek_Animation (); }

	// Update the animation
	void			Update( float dtime );

	// Get Animation Data
	void			Get_Animation_Data( AnimationDataList & list, float weight = 1.0f  );

	// Update Model
	void			Update_Model( RenderObjClass	*anim_model );

	// Data Access
	float			Get_Frame( void )					{ return NewChannel.Get_Frame(); }
	float			Get_Progress( void )				{ return NewChannel.Get_Progress(); }

private:
	AnimChannelClass	NewChannel;
	AnimChannelClass	OldChannel;
	float					BlendTimer;		// in seconds
	float					BlendTotal;		// in seconds
};


/*
** AnimControlClass
*/
class AnimControlClass {

public:
	// Constructor and Destructor
	AnimControlClass( void );
	virtual ~AnimControlClass( void );

	virtual	bool 	Save( ChunkSaveClass & csave );
	virtual	bool	Load( ChunkLoadClass & cload );

	// Set Model
	virtual	void	Set_Model( RenderObjClass	*anim_model );
	virtual	RenderObjClass	*Peek_Model( void )	{ return Model; }

	// Set Animation
	virtual	void	Set_Animation( const char *name, float blendtime = 0.0f, float start_frame = 0.0f )	= 0;
	virtual	void	Set_Animation( const HAnimClass * anim, float blendtime = 0.0f, float start_frame = 0.0f  ) = 0;
	virtual	void	Set_Mode( AnimMode mode, float frame = -1 )						= 0;
	virtual	AnimMode	Get_Mode( void )														= 0;
	virtual	bool	Is_Complete( void )														= 0;
	virtual	const char *Get_Animation_Name( void )										= 0;

	virtual	void	Set_Target_Frame( float frame )										= 0;
	virtual	float	Get_Target_Frame( void )												= 0;
	virtual	float	Get_Current_Frame( void )												= 0;

	virtual HAnimClass *	Peek_Animation( void ) { return NULL; }

	// Update the animation
	virtual	void	Update( float dtime )													= 0;

protected:
	RenderObjClass	*Model;
};


/*
** SimpleAnimControlClass
*/
class SimpleAnimControlClass : public AnimControlClass {

public:
	// Constructor and Destructor
	SimpleAnimControlClass( void );
	~SimpleAnimControlClass( void );

	virtual	bool 	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;

	// Set Animation
	virtual	void	Set_Animation( const char *name, float blendtime = 0.0f, float start_frame = 0.0f  ) override;
	virtual	void	Set_Animation( const HAnimClass * anim, float blendtime = 0.0f, float start_frame = 0.0f  ) override;
	virtual	void	Set_Mode( AnimMode mode, float frame = -1 )	override { Channel.Set_Mode( mode, frame ); }
	virtual	AnimMode	Get_Mode( void ) override									{ return Channel.Get_Mode(); }
	virtual	bool	Is_Complete( void ) override									{ return Channel.Is_Complete(); }
	virtual	const char *Get_Animation_Name( void ) override					{ return Channel.Get_Animation_Name(); }

	virtual	void	Set_Target_Frame( float frame ) override					{ Channel.Set_Target_Frame( frame ); }
	virtual	float	Get_Target_Frame( void ) override							{ return Channel.Get_Target_Frame(); }
	virtual	float	Get_Current_Frame( void ) override							{ return Channel.Get_Frame(); }


	// Update the animation
	virtual	void	Update( float dtime ) override;

private:
	BlendableAnimChannelClass	Channel;

};


/*
** HumanAnimControlClass
*/
class HumanAnimControlClass : public AnimControlClass {

public:
	// Constructor and Destructor
	HumanAnimControlClass( void );
	~HumanAnimControlClass( void );

	virtual	bool 	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;

	virtual	void	Set_Model( RenderObjClass	*anim_model ) override;

	// Set Animation
	virtual	void	Set_Animation( const char *name, float	blendtime = 0.0f, float start_frame = 0.0f  ) override;
	virtual	void	Set_Animation( const HAnimClass * anim, float blendtime = 0.0f, float start_frame = 0.0f  ) override;
	virtual	void	Set_Animation( const char *name1, const char * name2, float ratio, float blendtime = 0.0f );
	virtual	void	Set_Mode( AnimMode mode, float frame = -1 ) override;
	virtual	AnimMode	Get_Mode( void ) override					{ return Channel1.Get_Mode(); }

	virtual	bool	Is_Complete( void )	override				{ return Channel1.Is_Complete(); }
	virtual	const char *Get_Animation_Name( void ) override	{ return Channel1.Get_Animation_Name(); }
	virtual	float	Get_Frame( void )						{ return Channel1.Get_Frame(); }
	virtual	float	Get_Progress( void )					{ return Channel1.Get_Progress(); }

	virtual	void	Set_Target_Frame( float frame )	override { Channel1.Set_Target_Frame( frame ); }
	virtual	float	Get_Target_Frame( void ) override			{ return Channel1.Get_Target_Frame(); }
	virtual	float	Get_Current_Frame( void ) override			{ return Channel1.Get_Frame(); }

	virtual	void	Set_Anim_Speed_Scale( float speed )		{ AnimSpeedScale = speed; }

	virtual	void	Update( float dtime ) override;

	virtual HAnimClass *	Peek_Animation( void ) override		{ return Channel1.Peek_Animation(); }

	void				Get_Information( StringClass & string );

	char				Get_Skeleton( void )					{ return Skeleton; }

private:

	BlendableAnimChannelClass	Channel1;
	BlendableAnimChannelClass	Channel2;
	float								Channel2Ratio;

	float								AnimSpeedScale;

	AnimationDataList				DataList;
	HAnimComboClass				AnimCombo;
	char								Skeleton;

	void								Build_Skeleton_Anim_Name( StringClass& new_name, const char * name );
};

#endif		// ANIMCONTROL_H


