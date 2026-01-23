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
 *                     $Archive:: /Commando/Code/Combat/damageablegameobj.h                   $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 2/19/02 1:45p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 21                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	DAMAGEABLEGAMEOBJ_H
#define	DAMAGEABLEGAMEOBJ_H

#include "always.h"
#include "scriptablegameobj.h"
#include "damage.h"
#include "encyclopediamgr.h"


/*
** DamageableGameObjDef - Defintion class for a DamageableGameObj
*/
class DamageableGameObjDef : public ScriptableGameObjDef
{
public:
	DamageableGameObjDef( void );

	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;

	DECLARE_EDITABLE( DamageableGameObjDef, ScriptableGameObjDef );

	int											Get_Name_ID (void) const	{ return TranslatedNameID; }

	//
	//	Encyclopedia information
	//
	EncyclopediaMgrClass::TYPE				Get_Encyclopedia_Type (void) const	{ return EncyclopediaType; }
	int											Get_Encyclopedia_ID (void) const		{ return EncyclopediaID; }

	//
	//	Icon information
	//
	const StringClass &						Get_Icon_Filename (void) const		{ return InfoIconTextureFilename; }
	int											Get_Translated_Name_ID (void) const	{ return TranslatedNameID; }

	const DefenseObjectDefClass &			Get_DefenseObjectDef( void ) const	{ return DefenseObjectDef; }

	//
	// Accessors (added as needed)
	//
	int											Get_Default_Player_Type(void) const { return DefaultPlayerType; }

protected:
	DefenseObjectDefClass						DefenseObjectDef;
	StringClass									InfoIconTextureFilename;
	int											TranslatedNameID;	
	EncyclopediaMgrClass::TYPE					EncyclopediaType;
	int											EncyclopediaID;
	bool										NotTargetable;
	int											DefaultPlayerType;

	friend	class								DamageableGameObj;
};

/*
**
*/
class DamageableGameObj : public ScriptableGameObj
{

public:
	//	Constructor and Destructor
	DamageableGameObj( void );
	virtual	~DamageableGameObj( void );

	// Definitions
	void	Init( const DamageableGameObjDef & definition );
	void	Copy_Settings( const DamageableGameObjDef & definition );
	void	Re_Init( const DamageableGameObjDef & definition );
	const DamageableGameObjDef & Get_Definition( void ) const ;

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;

	DefenseObjectClass *	Get_Defense_Object( void )						{ return &DefenseObject; }
	virtual	void	Apply_Damage( const OffenseObjectClass & damager, float scale = 1.0f, int alternate_skin = -1 );
	virtual	void	Completely_Damaged( const OffenseObjectClass & /* damager */ ) {};

	// Information settings
	const StringClass & 	Get_Info_Icon_Texture_Filename( void )		{ return Get_Definition().InfoIconTextureFilename; }
	int						Get_Translated_Name_ID( void ) const		{ return Get_Definition().TranslatedNameID; }

	virtual	bool			Is_Targetable( void ) const					{ return !Get_Definition().NotTargetable; }

	virtual	bool			Is_Health_Bar_Displayed( void ) const		{ return	IsHealthBarDisplayed; }
	virtual	void			Set_Is_Health_Bar_Displayed( bool state ) { IsHealthBarDisplayed = state; }

	// Type identification
	virtual	DamageableGameObj	*As_DamageableGameObj( void ) override	{ return this; }


	// Teams / Playertypes
	virtual int Get_Player_Type(void) const {return PlayerType;}
	virtual void Set_Player_Type(int type);
	bool		Is_Team_Player(void);
	Vector3		Get_Team_Color(void);
	bool		Is_Teammate(DamageableGameObj * p_obj);
	bool		Is_Enemy(DamageableGameObj * p_obj);
	

	//
	// Network support
	//
	virtual	void								Import_Occasional( BitStreamClass &packet ) override;
	virtual	void								Export_Occasional( BitStreamClass &packet ) override;

protected:

	DefenseObjectClass	DefenseObject;

	int					PlayerType;
	bool					IsHealthBarDisplayed;
};

#endif	//	DAMAGEABLEGAMEOBJ_H

