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
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/hudinfo.h                             $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/03/02 6:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef HUDINFO_H
#define HUDINFO_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef PHYSICALGAMEOBJ_H
	#include "physicalgameobj.h"
#endif

#ifndef VECTOR3_H
	#include "vector3.h"
#endif

/*
**
*/
class HUDInfo 
{
public:
	// Where is the camera looking?
	static	void	Set_Camera_Target_Position( const Vector3 & pos )	{ CameraTargetPosition = pos; }
	static	const Vector3 & Get_Camera_Target_Position( void )			{ return CameraTargetPosition; }

	// Where is the Star's weapon pointing?
	static	void	Set_Weapon_Target_Position( const Vector3 & pos )	{ WeaponTargetPosition = pos; }
	static	const Vector3 & Get_Weapon_Target_Position( void )			{ return WeaponTargetPosition; }
	static	void	Set_Weapon_Target_Object( DamageableGameObj * obj );
	static	DamageableGameObj *	Get_Weapon_Target_Object( void )		{ return (DamageableGameObj*)WeaponTargetObject.Get_Ptr(); }

	// What object and building should we be displaying information for?
	static	void	Set_Info_Object( DamageableGameObj * obj, bool is_mct = false );
	static	DamageableGameObj *	Get_Info_Object( void )					{ return (DamageableGameObj*)InfoObject.Get_Ptr(); }
	static	bool					Get_Info_Object_Is_MCT( void )			{ return IsMCT; }

	static	void	Clear_Info_Object( void )									{ InfoObject = NULL; }
	static	void	Update_Info_Object( void );

	// Should we display the action statusbar, and what is it currently at?
	static	void	Display_Action_Status_Bar( bool onoff )				{ DisplayActionBar = onoff; }
	static	bool	Display_Action_Status_Bar( void )						{ return DisplayActionBar; }
	static	float	Get_Action_Status_Value( void )							{ return ActionStatusValue; }
	static	void	Set_Action_Status_Value( float value )					{ ActionStatusValue = value; }

	static	void	Set_HUD_Help_Text( const unichar_t *string, const Vector3 &color = Vector3 (1.0F, 1.0F, 1.0F)) { HUDHelpText = string; HUDHelpTextColor = color; IsHUDHelpTextDirty = true; }
	static	void	Set_Is_HUD_Help_Text_Dirty( bool onoff )				{ IsHUDHelpTextDirty = onoff; }
	static	bool	Is_HUD_Help_Text_Dirty( void )							{ return IsHUDHelpTextDirty; }
	static	const WideStringClass &Get_HUD_Help_Text( void )			{ return HUDHelpText; }
	static	const Vector3 &	Get_HUD_Help_Text_Color( void )			{ return HUDHelpTextColor; }

private:

	static	Vector3				CameraTargetPosition;

	static	GameObjReference	WeaponTargetObject;
	static	Vector3				WeaponTargetPosition;

	static	GameObjReference	InfoObject;
	static	float					InfoObjectTimer;

	static	bool					DisplayActionBar;
	static	float					ActionStatusValue;

	static	WideStringClass	HUDHelpText;
	static	bool					IsHUDHelpTextDirty;
	static	Vector3				HUDHelpTextColor;
	static	bool					IsMCT;
};

#endif		// HUDINFO_H
