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
 *                     $Archive:: /Commando/Code/Combat/simplegameobj.h                       $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 12/17/01 8:06p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 22                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SIMPLEGAMEOBJ_H
#define	SIMPLEGAMEOBJ_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef PHYSICALGAMEOBJ_H
	#include "physicalgameobj.h"
#endif

#include "playerterminal.h"


/*
** SimpleGameObjDef - Defintion class for a SimpleGameObj
*/
class SimpleGameObjDef : public PhysicalGameObjDef
{
public:
	SimpleGameObjDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( SimpleGameObjDef, PhysicalGameObjDef );

	// Accessors
	PlayerTerminalClass::TYPE				Get_Player_Terminal_Type( void ) const	{ return PlayerTerminalType; }
	bool											Get_Is_Editor_Object( void )	const		{ return IsEditorObject; }


protected:

	bool											IsEditorObject;
	bool											IsHiddenObject;	
	friend	class								SimpleGameObj;

	// See playerterminal.h
	PlayerTerminalClass::TYPE				PlayerTerminalType;
};


/*
**
*/
class SimpleGameObj : public PhysicalGameObj {

public:
	SimpleGameObj();
	virtual	~SimpleGameObj();

	// Definitions
	virtual	void	Init( void ) override;
	void	Init( const SimpleGameObjDef & definition );
	const SimpleGameObjDef & Get_Definition( void ) const ;

	//	RTTI
	virtual	SimpleGameObj	*As_SimpleGameObj( void ) override		{ return this; }

	// SimpleGameObj interface
	bool Is_Hidden_Object(void)									{ return Get_Definition().IsHiddenObject; }

	// Save / Load / Construction Factory
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;
	virtual	void	On_Post_Load( void ) override;

	// Network support - TSS091901
	virtual	bool	Is_Always_Dirty( void ) override						{ return false; }
};

#endif	//	SIMPLEGAMEOBJ_H
