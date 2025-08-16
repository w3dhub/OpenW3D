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
 *                     $Archive:: /Commando/Code/Combat/scriptablegameobj.h                   $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 1/05/02 10:52a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 25                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SCRIPTABLEGAMEOBJ_H
#define	SCRIPTABLEGAMEOBJ_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	BASEGAMEOBJ_H
	#include "basegameobj.h"
#endif

#ifndef	GAMEOBJREF_H
	#include "gameobjref.h"
#endif

#ifndef	TIMEMGR_H
	#include "timemgr.h"
#endif

#ifndef GAMEOBJOBSERVER_H
	#include "gameobjobserver.h"
#endif

#ifndef SIMPLEVEC_H
	#include "simplevec.h"
#endif

#ifndef	VECTOR_H
	#include "vector.h"
#endif

#ifndef	__AUDIO_EVENTS_H
    #include "AudioEvents.h"
#endif


typedef	SimpleDynVecClass<GameObjObserverClass *>		GameObjObserverList;

class	GameObjObserverTimerClass;
class	GameObjCustomTimerClass;
class	DamageableGameObj;
class	BuildingGameObj;
class	SoldierGameObj;
class ScriptZoneGameObj;

/*
** ScriptableGameObjDef - Defintion class for a ScriptableGameObj
*/
class ScriptableGameObjDef : public BaseGameObjDef
{
public:
	ScriptableGameObjDef( void );

	virtual bool								Save( ChunkSaveClass &csave );
	virtual bool								Load( ChunkLoadClass &cload );

	DECLARE_EDITABLE( ScriptableGameObjDef, BaseGameObjDef );

protected:
	DynamicVectorClass<StringClass>		ScriptNameList;
	DynamicVectorClass<StringClass>		ScriptParameterList;

	friend	class								ScriptableGameObj;
};

/*
**
*/
class ScriptableGameObj : public BaseGameObj, public ReferenceableGameObj, public AudioCallbackClass
{

public:
	//	Constructor and Destructor
	ScriptableGameObj( void );
	virtual	~ScriptableGameObj( void );

	// Definitions
	void	Init( const ScriptableGameObjDef & definition );	
	void	Copy_Settings( const ScriptableGameObjDef & definition );
	void	Re_Init( const ScriptableGameObjDef & definition );
	virtual void	Post_Re_Init( void );
	const ScriptableGameObjDef & Get_Definition( void ) const ;
	virtual void	Set_Delete_Pending (void);

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave );
	virtual	bool	Load( ChunkLoadClass & cload );
	virtual	void	On_Post_Load( void );

	//	Thinking
	virtual	void	Think();
	virtual	void	Post_Think();

	virtual	void		Get_Position(Vector3 * set_pos) const		= 0;

	// Observers
	void Add_Observer( GameObjObserverClass * observer );
	void Remove_Observer( GameObjObserverClass * observer );
	void Remove_All_Observers(void);
	// Start_Observers will call created on all observers.  Should be used
	// in On_Post_Load (if first load), and after spawning / creating.  
	// Observers added in other cases will already have Created called.
	void Start_Observers( void );
	const GameObjObserverList & Get_Observers( void )	{ return Observers; }
	// This just adds to the list and calls attached, does not call Created
	void Insert_Observer( GameObjObserverClass * observer );

	// Timers
	void	Start_Observer_Timer( int observer_id, float duration, int timer_id );
	void	Start_Custom_Timer( ScriptableGameObj * from, float delay, int type, int param );

	// Type identification
	virtual	ScriptableGameObj	*As_ScriptableGameObj( void )	{ return this; };
	virtual	DamageableGameObj	*As_DamageableGameObj( void )	{ return NULL; };
	virtual	BuildingGameObj	*As_BuildingGameObj( void )	{ return NULL; };
	virtual	SoldierGameObj		*As_SoldierGameObj( void )		{ return NULL; };
	virtual	ScriptZoneGameObj	*As_ScriptZoneGameObj( void )	{ return NULL; };
	ReferenceableGameObj *	As_ReferenceableGameObj( void ) { return this; }

	virtual	void	Get_Information( StringClass & string );

	// From AudioCallbackClass
	virtual void	On_Sound_Ended( SoundSceneObjClass *sound_obj );

	// Network support
	virtual bool	Is_Always_Dirty( void )				{ return true; }
	//virtual bool	Exists_On_Client( void ) const	{ return true; }
	virtual void	Export_Creation( BitStreamClass &packet );
	virtual void	Import_Creation( BitStreamClass &packet );

protected:
	bool															ObserverCreatedPending;
	GameObjObserverList										Observers;
	DynamicVectorClass<GameObjObserverTimerClass *>	ObserverTimerList;
	DynamicVectorClass<GameObjCustomTimerClass *>	CustomTimerList;
};



#endif	//	SCRIPTABLEGAMEOBJ_H
