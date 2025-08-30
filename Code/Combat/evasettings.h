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
 *                     $Archive:: /Commando/Code/Combat/evasettings.h         $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/01/01 10:49a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 5                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	EVASETTINGS_H
#define	EVASETTINGS_H

#include "always.h"
#include "definition.h"
#include "rect.h"
#include "render2d.h"


///////////////////////////////////////////////////////////////////////////////////////////
//
//	EvaSettingsDefClass
//
///////////////////////////////////////////////////////////////////////////////////////////
class EvaSettingsDefClass : public DefinitionClass
{
public:
	
	//////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////
	EvaSettingsDefClass (void);
	~EvaSettingsDefClass (void);

	//////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////

	//
	//	From DefinitionClass
	//
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual const PersistFactoryClass &	Get_Factory (void) const override;

	static EvaSettingsDefClass *			Get_Instance (void)	{ return EvaSettings; }	

	//
	//	Accessors
	//
	RectClass						Get_Objectives_Screen_Rect (void) const;
	RectClass						Get_Objectives_Text_Rect (void) const;
	const RectClass &				Get_Objectives_Endcap_Rect (void) const		{ return ObjectivesEndcapUVRect; }
	const RectClass &				Get_Objectives_Fadeout_Rect (void) const		{ return ObjectivesFadeoutUVRect; }
	const RectClass &				Get_Objectives_Background_Rect (void) const	{ return ObjectivesBackgroundUVRect; }
	const Vector2 &				Get_Objectives_Texture_Size (void) const		{ return ObjectivesTextureSize;; }

	RectClass						Get_Messages_Screen_Rect (void) const;
	RectClass						Get_Messages_Text_Rect (void) const;
	const RectClass &				Get_Messages_Endcap_Rect (void) const			{ return MessagesEndcapUVRect; }
	const RectClass &				Get_Messages_Fadeout_Rect (void) const			{ return MessagesFadeoutUVRect; }
	const RectClass &				Get_Messages_Background_Rect (void) const		{ return MessagesBackgroundUVRect; }
	const Vector2 &				Get_Messages_Texture_Size (void) const			{ return MessagesTextureSize;; }
	Vector2							Get_Messages_Icon_Position (void) const;
	
	//
	//	Editable support
	//
	DECLARE_EDITABLE (EvaSettingsDefClass, DefinitionClass);

protected:
	
	//////////////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////////////

	RectClass									ObjectivesScreenRect;
	RectClass									ObjectivesTextRect;
	RectClass									ObjectivesEndcapUVRect;
	RectClass									ObjectivesFadeoutUVRect;
	RectClass									ObjectivesBackgroundUVRect;
	Vector2										ObjectivesTextureSize;

	RectClass									MessagesScreenRect;
	RectClass									MessagesTextRect;
	RectClass									MessagesEndcapUVRect;
	RectClass									MessagesFadeoutUVRect;
	RectClass									MessagesBackgroundUVRect;
	Vector2										MessagesTextureSize;
	Vector2										MessagesIconPos;
	
	static EvaSettingsDefClass *			EvaSettings;
};


//////////////////////////////////////////////////////////////////////////
//	Get_Objectives_Screen_Rect
//////////////////////////////////////////////////////////////////////////
inline RectClass
EvaSettingsDefClass::Get_Objectives_Screen_Rect (void) const
{
	RectClass adjusted_rect = ObjectivesScreenRect;

	//
	//	Convert the normalized screen coords to pixels
	//
	const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution ();
	adjusted_rect.Scale (Vector2 (screen_rect.Width (), screen_rect.Height ()));
	adjusted_rect.Left	= float (int(adjusted_rect.Left + 0.5F));
	adjusted_rect.Top		= float (int(adjusted_rect.Top + 0.5F));
	adjusted_rect.Right	= float (int(adjusted_rect.Right + 0.5F));
	adjusted_rect.Bottom	= float (int(adjusted_rect.Bottom + 0.5F));
	return adjusted_rect;
}


//////////////////////////////////////////////////////////////////////////
//	Get_Objectives_Text_Rect
//////////////////////////////////////////////////////////////////////////
inline RectClass
EvaSettingsDefClass::Get_Objectives_Text_Rect (void) const
{
	RectClass adjusted_rect = ObjectivesTextRect;

	//
	//	Convert the normalized screen coords to pixels
	//
	const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution ();
	adjusted_rect.Scale (Vector2 (screen_rect.Width (), screen_rect.Height ()));
	adjusted_rect.Left	= float (int(adjusted_rect.Left + 0.5F));
	adjusted_rect.Top		= float (int(adjusted_rect.Top + 0.5F));
	adjusted_rect.Right	= float (int(adjusted_rect.Right + 0.5F));
	adjusted_rect.Bottom	= float (int(adjusted_rect.Bottom + 0.5F));
	return adjusted_rect;
}


//////////////////////////////////////////////////////////////////////////
//	Get_Messages_Screen_Rect
//////////////////////////////////////////////////////////////////////////
inline RectClass
EvaSettingsDefClass::Get_Messages_Screen_Rect (void) const
{
	RectClass adjusted_rect = MessagesScreenRect;

	//
	//	Convert the normalized screen coords to pixels
	//
	const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution ();
	adjusted_rect.Scale (Vector2 (screen_rect.Width (), screen_rect.Height ()));
	adjusted_rect.Left	= float (int(adjusted_rect.Left + 0.5F));
	adjusted_rect.Top		= float (int(adjusted_rect.Top + 0.5F));
	adjusted_rect.Right	= float (int(adjusted_rect.Right + 0.5F));
	adjusted_rect.Bottom	= float (int(adjusted_rect.Bottom + 0.5F));
	return adjusted_rect;
}


//////////////////////////////////////////////////////////////////////////
//	Get_Messages_Text_Rect
//////////////////////////////////////////////////////////////////////////
inline RectClass
EvaSettingsDefClass::Get_Messages_Text_Rect (void) const
{
	RectClass adjusted_rect = MessagesTextRect;

	//
	//	Convert the normalized screen coords to pixels
	//
	const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution ();
	adjusted_rect.Scale (Vector2 (screen_rect.Width (), screen_rect.Height ()));
	adjusted_rect.Left	= float (int(adjusted_rect.Left + 0.5F));
	adjusted_rect.Top		= float (int(adjusted_rect.Top + 0.5F));
	adjusted_rect.Right	= float (int(adjusted_rect.Right + 0.5F));
	adjusted_rect.Bottom	= float (int(adjusted_rect.Bottom + 0.5F));
	return adjusted_rect;
}


//////////////////////////////////////////////////////////////////////////
//	Get_Messages_Icon_Position
//////////////////////////////////////////////////////////////////////////
inline Vector2
EvaSettingsDefClass::Get_Messages_Icon_Position (void) const
{
	Vector2 adjusted_point = MessagesIconPos;

	//
	//	Convert the normalized screen coords to pixels
	//
	adjusted_point.X *= Render2DClass::Get_Screen_Resolution ().Width ();
	adjusted_point.Y *= Render2DClass::Get_Screen_Resolution ().Height ();
	return adjusted_point;
}


#endif	//	EVASETTINGS_H
