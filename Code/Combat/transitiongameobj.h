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
 *                     $Archive:: /Commando/Code/Combat/transitiongameobj.h                   $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 11/16/01 3:19p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 11                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	TRANSITIONGAMEOBJ_H
#define	TRANSITIONGAMEOBJ_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef BASEGAMEOBJ_H
	#include "basegameobj.h"
#endif

#ifndef TRANSITION_H
	#include "transition.h"
#endif

/*
** TransitionGameObjDef - Defintion class for a TransitionGameObj 
*/
class TransitionGameObjDef : public BaseGameObjDef
{
public:
	TransitionGameObjDef( void ) {};
	virtual ~TransitionGameObjDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	TRANSITION_DATA_LIST *					Get_Transition_List( void ) { return &Transitions; }
	const TRANSITION_DATA_LIST &			Get_Transition_List( void ) const { return Transitions; }
	void											Free_Transition_List( void );

protected:

	TRANSITION_DATA_LIST						Transitions;

	friend	class								TransitionGameObj;
};

/*
**
*/
class TransitionGameObj : public BaseGameObj {

public:
	TransitionGameObj();
	~TransitionGameObj();

	// Definitions
	virtual	void	Init( void ) override;
	void	Init( const TransitionGameObjDef & definition );
	const TransitionGameObjDef & Get_Definition( void ) const ;

	// Save / Load / Construction Factory
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;
	virtual	void	On_Post_Load( void ) override;

	// Physics
	void					Set_Transform(const Matrix3D & tm);
	const Matrix3D &	Get_Transform(void) const;

	// Transition access
	void		Create_Transitions( void );
	void		Destroy_Transitions( void );
	void		Update_Transitions( void );

	int								Get_Transition_Count( void ) const	{ return TransitionInstances.Count(); }
	TransitionInstanceClass *	Get_Transition( int index )			{ return TransitionInstances[index]; }

	//
	//	Ladder support
	//
	int		Get_Ladder_Index( void )					{ return LadderIndex; }
	void		Set_Ladder_Index( int ladder_index );

protected:

	DynamicVectorClass<TransitionInstanceClass *> TransitionInstances;
	Matrix3D						TM;
	int							LadderIndex;
};


#endif	//	TRANSITIONGAMEOBJ_H

