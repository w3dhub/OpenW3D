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
 *                     $Archive:: /Commando/Code/Combat/textwindow.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/28/01 7:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TEXT_WINDOW_H
#define __TEXT_WINDOW_H

#include "wwstring.h"
#include "widestring.h"
#include "vector.h"
#include "vector3.h"
#include "bittype.h"
#include "rect.h"
#include "render2d.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class Render2DSentenceClass;
class RenderObjClass;
class TextColumnClass;
class SceneClass;

////////////////////////////////////////////////////////////////
//
//	TextWindowClass
//
////////////////////////////////////////////////////////////////
class TextWindowClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	TextWindowClass (void);
	~TextWindowClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void		Initialize (SceneClass *scene);
	static void		Shutdown (void);

	//
	//	Backdrop methods
	//
	void				Set_Backdrop (const char *texture_name, const RectClass &screen_rect, const Vector2 &texture_size, const RectClass &endcap_rect, const RectClass &fadeout_rect, const RectClass &textback_rect);
	void				Set_Text_Area (const RectClass &rect)	{ TextRect = rect; }
	
	//
	//	Font control
	//
	void				Set_Heading_Font (const char *font_name)	{ HeadingFontName = font_name; }
	void				Set_Text_Font (const char *font_name)		{ TextFontName = font_name; }

	//
	//	Column support
	//
	void				Add_Column (const unichar_t *column_name, float width, const Vector3 &color);
	bool				Remove_Column (int index);
	void				Delete_All_Columns (void);
	bool				Are_Columns_Displayed (void) const	{ return AreColumnsDisplayed; }
	void				Display_Columns (bool onoff)			{ AreColumnsDisplayed = onoff; IsViewDirty = true; IsWindowDirty = true; }

	//
	//	Content control
	//
	int				Insert_Item (int index, const unichar_t *text);	
	bool				Set_Item_Text (int index, int col_index, const unichar_t *text);
	bool				Set_Item_Color (int index, int col_index, const Vector3 &color);
	bool				Set_Item_Data (int index, uintptr_t user_data);
	uintptr_t			Get_Item_Data (int index);
	bool				Delete_Item (int index);
	void				Delete_All_Items (void);
	int				Get_Item_Count (void) const;

	//
	//	Scrolling support
	//
	void				Scroll_To_Top (void);
	void				Page_Down (void);
	void				Page_Up (void);

	//
	//	Visibility methods
	//
	bool				Is_Displayed (void) const			{ return IsDisplayed; }
	void				Display (bool onoff);
	int				Get_Display_Count (void);
	float				Get_Total_Display_Height (void);
	
	//
	//	Rendering methods
	//
	void				On_Frame_Update (void);
	void				Render (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void				Free_Backdrop (void);
	void				Free_Contents (void);
	void				Free_Renderers (void);
	void				Build_View (void);
	void				Update_View (float *total_height = NULL, bool info_only = false);

	void				Update_Row (int item_index, float y_pos, float *row_height);

	////////////////////////////////////////////////////////////////
	//	Private data types
	////////////////////////////////////////////////////////////////
	typedef DynamicVectorClass<TextColumnClass *>	COLUMN_LIST;


	////////////////////////////////////////////////////////////////
	//	Static member data
	////////////////////////////////////////////////////////////////
	static SceneClass *		Scene;

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	int							FirstLineIndex;
	int							CurrentDisplayCount;
	bool							IsDisplayed;
	bool							AreColumnsDisplayed;

	bool							IsViewDirty;
	bool							IsWindowDirty;
	
	COLUMN_LIST					Columns;
	Render2DSentenceClass *	TextRenderers[2];

	Render2DClass				Backdrop;
	RectClass					TextRect;
	
	float							ColumnHeight;
	float							LineSpacing;

	StringClass					HeadingFontName;
	StringClass					TextFontName;
};


////////////////////////////////////////////////////////////////
//
//	TextItemClass
//
////////////////////////////////////////////////////////////////
class TextItemClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	TextItemClass (void) :
		Name (U_CHAR("")),
		Color (1, 1, 1),
		UserData (0)	{}

	TextItemClass (const unichar_t *name) :
		Name (name),
		Color (1, 1, 1),
		UserData (0)	{}

	~TextItemClass (void) {}

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Name access
	//
	const unichar_t *		Get_Name (void) const			{ return Name; }
	void					Set_Name (const unichar_t *name)	{ Name = name; }

	//
	//	Color access
	//
	const Vector3 &	Get_Color (void) const				{ return Color; }
	void					Set_Color (const Vector3 &color)	{ Color = color; }

	//
	//	Color access
	//
	uintptr_t				Get_User_Data (void) const			{ return UserData; }
	void					Set_User_Data (uintptr_t user_data)	{ UserData = user_data; }

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	WideStringClass	Name;
	Vector3				Color;
	uintptr_t				UserData;
};


////////////////////////////////////////////////////////////////
//
//	TextColumnClass
//
////////////////////////////////////////////////////////////////
class TextColumnClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	TextColumnClass (void) :
		Width (0)					{ Reset_Contents (); }

	~TextColumnClass (void)		{ Free_Data (); }

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Name access
	//
	const unichar_t *		Get_Name (void) const			{ return Header.Get_Name (); }
	void					Set_Name (const unichar_t *name)	{ Header.Set_Name (name); }

	//
	//	Width access
	//
	float					Get_Width (void) const			{ return Width; }
	void					Set_Width (float width)			{ Width = width; }

	//
	//	Color access
	//
	const Vector3 &	Get_Color (void) const				{ return Header.Get_Color (); }
	void					Set_Color (const Vector3 &color)	{ Header.Set_Color (color); }

	//
	//	Item access
	//
	int					Insert_Item (int index, const unichar_t *item_name);
	int					Get_Item_Count (void) const								{ return Items.Count (); }
	bool					Delete_Item (int index);
	void					Delete_All_Items (void);
	
	void					Set_Item_Text (int index, const unichar_t *text)			{ Items[index]->Set_Name (text); }
	const unichar_t *		Get_Item_Text (int index) const							{ return Items[index]->Get_Name (); }	
	
	void					Set_Item_Color (int index, const Vector3 &color)	{ Items[index]->Set_Color (color); }
	const Vector3 &	Get_Item_Color (int index) const							{ return Items[index]->Get_Color (); }

	void					Set_Item_Data (int index, uintptr_t data)					{ Items[index]->Set_User_Data (data); }
	uintptr_t				Get_Item_Data (int index) const							{ return Items[index]->Get_User_Data (); }

	//
	//	Cleanup
	//
	void					Reset_Contents (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void					Free_Data (void);

	////////////////////////////////////////////////////////////////
	//	Private data types
	////////////////////////////////////////////////////////////////
	typedef DynamicVectorClass <TextItemClass *> ITEM_LIST;

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	TextItemClass		Header;
	ITEM_LIST			Items;
	float					Width;
};


#endif //__TEXT_WINDOW_H

