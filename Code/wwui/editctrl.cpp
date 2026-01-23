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
 *                     $Archive:: /Commando/Code/wwui/editctrl.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/17/02 1:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "editctrl.h"
#include "assetmgr.h"
#include "refcount.h"
#include "font3d.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "stylemgr.h"
#include "dialogbase.h"
#include <limits>
#include <algorithm>


////////////////////////////////////////////////////////////////
//
//	EditCtrlClass
//
////////////////////////////////////////////////////////////////
EditCtrlClass::EditCtrlClass (void)	:
	CaretPos (0),
	ScrollPos (0),
	NumChars(0),
	TextLimit(0xFFFF),
	CaretBlinkDelay (500),
	LastCaretBlink (0),
	HilightStartPos (-1),
	HilightEndPos (-1),
	HilightAnchorPos (-1),
	WasButtonPressedOnMe (false),
	IsCaretDisplayed (false),
	mInComposition(false)

#ifdef SHOW_IME_TYPING
	, mShowIMETypingText(false)
#endif
{	
	//
	//	Set the font for the text renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_CONTROLS);
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	
	StyleMgrClass::Configure_Renderer (&CaretRenderer);
	StyleMgrClass::Configure_Renderer (&HilightRenderer);	

	mIME = DialogMgrClass::Get_IME();
}


////////////////////////////////////////////////////////////////
//
//	~EditCtrlClass
//
////////////////////////////////////////////////////////////////
EditCtrlClass::~EditCtrlClass (void)
{
	if (mIME) {
		mIME->Release_Ref();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderers
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Create_Text_Renderers (void)
{
	HilightRenderer.Reset ();
	HilightRenderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());		
	StyleMgrClass::Configure_Hilighter (&HilightRenderer);

	//
	//	Get the string we'll be displaying
	//
	WideStringClass display_string(0, true);
	Get_Display_Text(display_string);

	//
	//	Index into the text buffer
	//
	const unichar_t* text = display_string.Peek_Buffer();

	if (ScrollPos >= 0 && ScrollPos < display_string.Get_Length()) {
		text += ScrollPos;
	}

	//
	//	Start fresh
	//
	TextRenderer.Reset();

	//
	//	Draw the text
	//
	uint32 color = StyleMgrClass::Get_Text_Color();
	uint32 shadowColor = StyleMgrClass::Get_Text_Shadow_Color();

	if (IsEnabled == false) {
		color = StyleMgrClass::Get_Disabled_Text_Color();
		shadowColor = StyleMgrClass::Get_Disabled_Text_Shadow_Color();
	}

	StyleMgrClass::Render_Text(text, &TextRenderer, color, shadowColor,
			ClientRect, true, true, StyleMgrClass::LEFT_JUSTIFY, IsEnabled);

	// Draw the composition markers
	if (mIME && mInComposition) {
		const unichar_t* compString = mIME->GetCompositionString();
		Vector2 compSize = TextRenderer.Get_Text_Extents(compString);

		if (compSize.X > 0.0f) {
			// Underline the composition string
			Vector2 startPos;
			startPos.X = Pos_From_Character(CaretPos);
			startPos.Y = ClientRect.Bottom - 2;

			Vector2 stopPos(startPos);
			stopPos.X += compSize.X;

			unsigned int underlineColor = StyleMgrClass::Get_Disabled_Line_Color();
			ControlRenderer.Add_Line(startPos, stopPos, 1.0f, underlineColor);

			// Hilight the conversion target
			unsigned int start = 0;
			unsigned int end = 0;
			mIME->GetTargetClause(start, end);

			if (end > 0) {
				HilightStartPos = (CaretPos + start);
				HilightEndPos = (CaretPos + end);
			}
		}
	}

	//
	//	Do the hilight
	//
	if (HilightStartPos >= 0 && HilightStartPos != HilightEndPos && IsEnabled) {
		RectClass hilight_rect;
		hilight_rect.Top		= ClientRect.Top + 1;
		hilight_rect.Bottom	= ClientRect.Bottom - 1;
		hilight_rect.Left		= Pos_From_Character (HilightStartPos);
		hilight_rect.Right	= Pos_From_Character (HilightEndPos);

		StyleMgrClass::Render_Hilight (&HilightRenderer, hilight_rect);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderers
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Create_Control_Renderers (void)
{
	//
	//	Configure this renderer
	//
	ControlRenderer.Reset ();
	ControlRenderer.Enable_Texturing (false);

	//
	//	Determine which color to draw the outline in
	//
	int color	= StyleMgrClass::Get_Line_Color ();
	int bkcolor	= StyleMgrClass::Get_Bk_Color ();

	if (IsEnabled == false) {
		color		= StyleMgrClass::Get_Disabled_Line_Color ();
		bkcolor	= StyleMgrClass::Get_Disabled_Bk_Color ();
	}

	//
	//	Draw the control outline
	//
	RectClass rect = Rect;
	ControlRenderer.Add_Outline (rect, 1.0F, color);

	//
	//	Now draw the client area
	//
	rect.Right	-= 1;
	rect.Bottom -= 1;
	ControlRenderer.Add_Quad (rect, bkcolor);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::On_Set_Cursor (const Vector2 &mouse_pos)
{
	//
	//	Change the mouse cursor if necessary
	//
	if (ClientRect.Contains (mouse_pos)) {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_TEXT);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Update_Client_Rect (void)
{
	//
	//	Determine what one character spacing would be
	//
	float char_width = TextRenderer.Get_Text_Extents (U_CHAR("I")).X;

	//
	//	Shrink the client area
	//
	ClientRect = Rect;
	ClientRect.Inflate (Vector2 (-char_width, -2));

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {
		Create_Control_Renderers ();
		Create_Text_Renderers ();
		Create_Caret_Renderer ();
	}

	//
	//	Render the background and text for the current state
	//
	ControlRenderer.Render ();
	TextRenderer.Render ();
	HilightRenderer.Render ();

	//
	//	Render the caret (if necessary)
	//
	Update_Caret ();

	if (HasFocus && IsCaretDisplayed) {
		CaretRenderer.Render ();
	}

	#ifdef SHOW_IME_TYPING
	if (mShowIMETypingText) {
		mIMETypingTip.Render();
	}
	#endif

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//	Get_Display_Text
////////////////////////////////////////////////////////////////
void EditCtrlClass::Get_Display_Text(WideStringClass &text)
{
	// Resize to accomodate entire string
	size_t length = Title.Get_Length();

	if (mIME && mInComposition) {
		const unichar_t* compString = mIME->GetCompositionString();

		if (compString) {
			length += ::u_strlen(compString);
		}
	}
	WWASSERT(length <= static_cast<size_t>(std::numeric_limits<int>::max()));
		const int buffer_length = static_cast<int>(length);
		text.Get_Buffer(buffer_length);

	// If password then replace text with astrisks
	if ((Style & ES_PASSWORD) != 0) {
		int len = static_cast<int>(Title.Get_Length());
		unichar_t* buffer = text.Peek_Buffer();
		int index;

		for (index = 0; index < len; ++index) {
			buffer[index] = U_CHAR('*');
		}

		buffer[index] = 0;
	} else {
		text = Title;
	}

	// Insert IME composition at cursor position
	if (mIME && mInComposition) {
		const unichar_t* compString = mIME->GetCompositionString();

		if (compString) {
			WideStringClass temp(buffer_length, true);
				temp = (text.Peek_Buffer() + CaretPos);

			text.Erase(CaretPos, (text.Get_Length() - CaretPos));
			text += compString;
			text += temp;
		}
	}
}

////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	if (mInComposition) {
		return;
	}

	Set_Capture ();

	if (HasFocus) {
		
		//
		//	Update the caret position
		//
		Set_Caret_Pos (Character_From_Pos (mouse_pos));
		HilightAnchorPos	= CaretPos;
		HilightStartPos	= -1;
		HilightEndPos		= -1;

		//
		//	Force a repaint
		//
		Set_Dirty ();
		WasButtonPressedOnMe = true;
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	if (mInComposition) {
		return;
	}

	Release_Capture ();

	//
	//	Reset our flags
	//
	WasButtonPressedOnMe	= false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::On_Mouse_Move (const Vector2 &mouse_pos)
{
	if (mInComposition) {
		return;
	}

	//
	//	Is the user "dragging" inside the control
	//
	if (HasFocus && WasButtonPressedOnMe) {
		//
		//	Has the caret position changed?
		//
		int new_caret_pos = Character_From_Pos (mouse_pos);

		if (new_caret_pos != CaretPos) {
			//
			//	Calculate the new caret and hilight positions
			//
			Set_Caret_Pos (new_caret_pos);
			HilightStartPos	= std::min (HilightAnchorPos, CaretPos);
			HilightEndPos		= std::max (HilightAnchorPos, CaretPos);

			//
			//	Force a repaint
			//
			Set_Dirty ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Caret
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Update_Caret (void)
{
	//
	//	Blink the caret if necessary
	//
	uint32 curr_time = DialogMgrClass::Get_Time ();

	if ((curr_time - LastCaretBlink) > CaretBlinkDelay) {
		IsCaretDisplayed	= !IsCaretDisplayed;
		LastCaretBlink		= curr_time;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Caret_Renderer
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Create_Caret_Renderer (void)
{
	CaretRenderer.Reset ();

	//
	//	Configure this renderer
	//
	CaretRenderer.Enable_Texturing (false);
	CaretRenderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution());

	//
	//	Determine how many characters we should render the caret after
	//
	WideStringClass temp_copy(0, true);
	Get_Display_Text(temp_copy);

	unichar_t *text = temp_copy.Peek_Buffer();
	int caretPos = Get_Caret_Pos();
	text[caretPos] = 0;
	text = &text[ScrollPos];

	//
	//	Calculate where in the client area we should build the caret
	//
	Vector2 text_extent	= TextRenderer.Get_Text_Extents (text);
	float space_extent	= 1;

	//
	//	Create a bounding rectangle for the caret
	//
	RectClass rect;
	rect.Left	= int(ClientRect.Left + text_extent.X);
	rect.Top		= int(ClientRect.Top + 1);
	rect.Right	= int(rect.Left + std::max (space_extent, 1.0F));
	rect.Bottom	= int(ClientRect.Bottom - 1);

	//
	//	Draw the caret
	//
	CaretRenderer.Add_Quad(rect, StyleMgrClass::Get_Text_Color());
}


////////////////////////////////////////////////////////////////
//
//	Character_From_Pos
//
////////////////////////////////////////////////////////////////
int
EditCtrlClass::Character_From_Pos (const Vector2 &mouse_pos)
{
	//
	//	Get the text we will be displaying
	//
	WideStringClass display_text(0, true);
	Get_Display_Text(display_text);

	//
	//	Index into the buffer
	//
	const unichar_t *text		= display_text.Peek_Buffer () + ScrollPos;	
	int char_index			= static_cast<int>(display_text.Get_Length ());

	float x_pos				= mouse_pos.X - ClientRect.Left;
	float curr_x_pos		= 0;

	//
	//	Loop over all the characters in the remainder of the string until
	// we've moved past the x-position we we're looking for.
	//
	int count = static_cast<int>(::u_strlen (text));	
	for (int index = 0; index < count; index ++) {
		
		//
		//	Get the width of the character
		//
		unichar_t char_string[2] = { text[index], 0 };
		float char_width = TextRenderer.Get_Text_Extents (char_string).X;
		
		//
		//	Did we move past the position we were looking for?
		//
		if (curr_x_pos + (char_width / 2) >= x_pos) {
			char_index = index + ScrollPos;
			break;
		}

		//
		//	Increment our x-position
		//
		curr_x_pos += char_width;
	}

	//
	//	Return the character index
	//
	return char_index;
}


////////////////////////////////////////////////////////////////
//
//	Pos_From_Character
//
////////////////////////////////////////////////////////////////
float
EditCtrlClass::Pos_From_Character (int char_index)
{
	//
	//	Ensure the character isn't scrolled off the screen
	//
	char_index = std::max (ScrollPos, char_index);

	//
	//	Get the width of the string up to this character index
	//
	WideStringClass temp_copy(0, true);
	Get_Display_Text (temp_copy);
	unichar_t *text						= temp_copy.Peek_Buffer ();
	text[char_index]				= 0;
	text								= &text[ScrollPos];
	float width						= TextRenderer.Get_Text_Extents (text).X;

	//
	//	Clip the position to the right side of the control
	//
	float position = std::min (ClientRect.Left + width, ClientRect.Right);
	return position;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::On_Set_Focus (void)
{
	//
	//	Hilight the contents of the window
	//
	Set_Caret_Pos (Title.Get_Length ());
	HilightAnchorPos	= 0;
	HilightStartPos	= 0;
	HilightEndPos		= CaretPos;
	Set_Dirty ();

	if (mIME) {
		if (IsIMEAllowed()) {
			Observer<IME::CompositionEvent>::NotifyMe(*mIME);
			Observer<IME::CandidateEvent>::NotifyMe(*mIME);
			mIME->Activate();
		} else {
			mIME->Disable();
		}
	}

	DialogControlClass::On_Set_Focus ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{
	if (mIME) {
		if (IsIMEAllowed()) {
			mIME->Deactivate();
			Observer<IME::CompositionEvent>::StopObserving();
			Observer<IME::CandidateEvent>::StopObserving();
		} else {
			mIME->Enable();
		}
	}

	WasButtonPressedOnMe	= false;

	//
	//	Remove any hilight
	//
	Set_Caret_Pos (0);
	HilightStartPos	= -1;
	HilightEndPos		= -1;
	Set_Dirty ();

	DialogControlClass::On_Kill_Focus (focus);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool 
EditCtrlClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	bool handled = false;

	//
	//	Check to see if the advise sinks are processing this input...
	//
	if (AdviseSink != NULL) {
		handled = AdviseSink->On_EditCtrl_Key_Down (this, key_id, key_data);
	}
	if (handled == false) {
		handled = Parent->On_EditCtrl_Key_Down (this, key_id, key_data);
	}

	if (handled) {
		return true;
	}

	handled			= true;
	bool changed	= false;

	bool is_dirty			= true;
	int old_caret			= CaretPos;
	bool update_hilight	= ((DialogMgrClass::Get_VKey_State (VK_SHIFT) & VKEY_PRESSED) == VKEY_PRESSED);

	switch (key_id)
	{
		case VK_BACK:
			if (HilightStartPos >= 0) {
				changed = Delete_Selection ();
			} else if (CaretPos > 0) {
				
				//
				//	Delete the previous character
				//
				Title.Erase (CaretPos - 1, 1);
				Set_Caret_Pos (CaretPos - 1);

				assert(NumChars > 0);
				NumChars--;
				
				changed = true;
			}
			update_hilight = false;
			break;

		case VK_DELETE:
			if (HilightStartPos >= 0) {
				changed = Delete_Selection ();
			} else if (CaretPos < Title.Get_Length ()) {
				
				//
				//	Delete the next character
				//
				Title.Erase (CaretPos, 1);
				assert(NumChars > 0);
				NumChars--;
				
				changed = true;
			}
			update_hilight = false;
			break;

		case VK_HOME:			
			Set_Caret_Pos (0);
			break;

		case VK_END:
			Set_Caret_Pos (Title.Get_Length ());
			break;

		case VK_LEFT:

			//
			//	Should snap the caret to words, or just increment it?
			//
			if (DialogMgrClass::Get_VKey_State (VK_CONTROL) & VKEY_PRESSED) {
				Set_Caret_Pos (Find_Word_Start (CaretPos, -1));
			} else {
				Set_Caret_Pos (std::max (0, CaretPos - 1));
			}
			break;

		case VK_RIGHT:
			
			//
			//	Should snap the caret to words, or just increment it?
			//
			if (DialogMgrClass::Get_VKey_State (VK_CONTROL) & VKEY_PRESSED) {
				Set_Caret_Pos (Find_Word_Start (CaretPos, 1));
			} else {
				Set_Caret_Pos (std::min (Title.Get_Length (), CaretPos + 1));
			}
			break;

		case VK_RETURN:
			//
			//	Notify the parent that the user pressed enter
			//
			ADVISE_NOTIFY (On_EditCtrl_Enter_Pressed (this, Get_ID ()));
			is_dirty = false;
			break;

		case VK_UP:
		case VK_DOWN:
			//
			//	Don't do anything
			//
			is_dirty = false;
			handled = false;
			break;

		default:
			is_dirty	= false;
			handled = false;
			break;
	}

	if (update_hilight) {

		//
		//	Update the hilight position
		//
		Update_Hilight (CaretPos, old_caret);

	} else if (handled) {
		
		//
		//	Reset the hilight position
		//
		HilightStartPos	= -1;
		HilightAnchorPos	= -1;
	}

	if (changed) {
		ADVISE_NOTIFY(On_EditCtrl_Change(this, Get_ID()));
	}

	//
	//	Repaint the control if necessary
	//
	if (is_dirty) {
		Set_Dirty ();
	}

	return handled;
}


void EditCtrlClass::On_Unicode_Char(unichar_t unicode)
{
	if (unicode >= 32) {
		//	Delete the old selection
		Delete_Selection ();

		if (NumChars < TextLimit) {
			NumChars++;

			//	Build the new string
			WideStringClass new_title = Title;
			new_title.Erase (CaretPos, new_title.Get_Length () - CaretPos);
			new_title += unicode;
			new_title += Title.Peek_Buffer () + CaretPos;
			Title = new_title;
		
			//	Move the caret to the end of the text the user just typed
			Set_Caret_Pos (CaretPos + 1);

			//
			//	Reset the hilight position
			//
			HilightStartPos	= -1;
			HilightAnchorPos	= -1;
		
			ADVISE_NOTIFY(On_EditCtrl_Change(this, Get_ID()));

			//	Force a repaint
			Set_Dirty ();
		}
	}
}


void EditCtrlClass::Insert_String(const unichar_t* string)
{
	int count = static_cast<int>(::u_strlen(string));

	if (count > 0) {
		//	Delete the old selection
		Delete_Selection();

		WideStringClass new_title = Title;
		new_title.Erase(CaretPos, new_title.Get_Length () - CaretPos);

		// If the string exceed the text limit then only insert characters upto
		// the text limit.
		if ((NumChars + count) > TextLimit) {
			// Truncate the insert string so that it will fit.
			WideStringClass insert(count, true);
			insert = string;

			count = (TextLimit - NumChars);
			insert.Erase(count, insert.Get_Length() - count);
			new_title += insert;
		} else {
			new_title += string;
		}

		new_title += Title.Peek_Buffer() + CaretPos;
		Title = new_title;

		NumChars += count;

		//	Move the caret to the end of the text the user just typed
		Set_Caret_Pos(CaretPos + count);

		ADVISE_NOTIFY(On_EditCtrl_Change(this, Get_ID()));

		//	Force a repaint
		Set_Dirty();
	}
}


////////////////////////////////////////////////////////////////
//
//	Delete_Selection
//
////////////////////////////////////////////////////////////////
bool
EditCtrlClass::Delete_Selection (void)
{
	if (HilightStartPos >= 0) {

		//
		//	Delete the characters and update the caret
		//
		int count = (HilightEndPos - HilightStartPos);
		Title.Erase (HilightStartPos, count);
		Set_Caret_Pos (HilightStartPos);

		assert(NumChars >= count);
		NumChars -= count;

		//
		//	Remove any hilighting
		//
		HilightStartPos	= -1;
		HilightEndPos		= -1;

		//
		//	Force a repaint
		//
		Set_Dirty ();
		return true;
	}

	return false;
}


////////////////////////////////////////////////////////////////
//
//	On_Create
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::On_Create (void)
{
	//TextColor.Set (0.35F, 1.0F, 0.35F);
	//Set_Text (U_CHAR("This is a test..."));	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Caret_Pos
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Set_Caret_Pos (int new_pos)
{
	//
	//	Remove the hilight
	//
	HilightStartPos = -1;	
	
	//
	//	Did the caret position change?
	//
	if (CaretPos != new_pos) {
		CaretPos = new_pos;

		//
		//	Force the caret to be displayed (i.e. no blink)
		//
		IsCaretDisplayed	= true;
		LastCaretBlink		= DialogMgrClass::Get_Time ();

		//
		//	Handle scrolling
		//
		Update_Scroll_Pos ();
		
		//
		//	Force a repaint
		//
		Set_Dirty ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Hilight
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Update_Hilight (int new_pos, int anchor_pos)
{
	HilightAnchorPos	= (HilightAnchorPos >= 0) ? HilightAnchorPos : anchor_pos;
	HilightStartPos	= std::min (new_pos, HilightAnchorPos);
	HilightEndPos		= std::max (new_pos, HilightAnchorPos);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Word_Start
//
////////////////////////////////////////////////////////////////
int
EditCtrlClass::Find_Word_Start (int pos, int increment)
{
	int count = Title.Get_Length ();	

	//
	//	Determine what the extreme end posiiton should be
	//
	int extreme	= pos + increment * 1000;
	int retval	= 0;	
	if (extreme > count) {
		retval = count;
	}
	
	//
	//	Loop over all the  characters until we've found the word break
	//	
	for (int index = pos + increment; index < count && index >= 0; index += increment) {
		
		//
		//	Is this a space character?
		//
		bool is_space	= (Title[index] == U_CHAR(' '));

		//
		//	If we've already found the word break and this is the
		// start of a new word, then return its index to the caller
		//
		if (!is_space && ((index == 0) || (Title[index - 1] == U_CHAR(' ')))) {
			retval = index;
			break;			
		}		
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Update_Scroll_Pos
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Update_Scroll_Pos (void)
{
	int old_scroll = ScrollPos;
	int caretPos = Get_Caret_Pos();

	//
	//	Find the current x-position of the caret
	//
	float x_pos = Pos_From_Character(caretPos);

	if (x_pos <= ClientRect.Left) {
		//
		//	Simply ensure the caret is scrolled into view
		//
		ScrollPos = caretPos - 2;
	} else if (x_pos >= ClientRect.Right) {

		//
		// Make a temporary copy of the text so we can
		// modify it
		//
		WideStringClass temp_string(0, true);
		Get_Display_Text(temp_string);
		unichar_t *text = temp_string.Peek_Buffer();
		text[caretPos] = 0;

		//
		//	Find how far we'd have to scroll in order to bring the
		// caret back into view.
		//
		for (int index = 0; index < caretPos; ++index) {
			float width = TextRenderer.Get_Text_Extents (text + index).X;

			if (width < ClientRect.Width ()) {
				ScrollPos = index + 2;
				break;
			}
		} 
	}
	
	//
	//	Adjust the scroll range to stay within our boundaries
	//	
	ScrollPos = std::min (ScrollPos, Title.Get_Length () - 1);
	ScrollPos = std::max (ScrollPos, 0);

	//
	//	Repaint if necessary
	//
	if (old_scroll != ScrollPos) {
		Set_Dirty ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Int
//
////////////////////////////////////////////////////////////////
int
EditCtrlClass::Get_Int (void)
{
	int retval;
	u_sscanf_u(Get_Text (), U_CHAR("%d"), &retval);
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Int
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Set_Int (int value)
{
	WideStringClass text(64, true);
	text.Format (U_CHAR("%d"), value);
	Set_Text (text);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Text
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Set_Text (const unichar_t *title)
{
	int count = static_cast<int>(::u_strlen(title));

	// If the string is too long then truncate it so that it will fit.
	if (count > TextLimit) {
		WideStringClass text(count, true);
		text = title;
		text.Erase(TextLimit, text.Get_Length() - TextLimit);
		DialogControlClass::Set_Text (text);

		count = TextLimit;
	} else {
		DialogControlClass::Set_Text (title);
	}

	NumChars = count;

	Set_Caret_Pos (Title.Get_Length ());
	HilightAnchorPos	= CaretPos;
	HilightStartPos	= -1;
	HilightEndPos		= -1;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Text_Length
//
////////////////////////////////////////////////////////////////
int EditCtrlClass::Get_Text_Length(void) const
{
	return NumChars;
}


////////////////////////////////////////////////////////////////
//
//	Set_Text_Limit
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Set_Text_Limit (int numChars)
{
	if (numChars > 0) {
		TextLimit = numChars;
	} else {
		TextLimit = 0xFFFF;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Text_Limit
//
////////////////////////////////////////////////////////////////
int
EditCtrlClass::Get_Text_Limit (void) const
{
	return TextLimit;
}




////////////////////////////////////////////////////////////////
//
//	Get_Caret_Pos
//
////////////////////////////////////////////////////////////////
int
EditCtrlClass::Get_Caret_Pos (void) const
{
	if (mIME && mInComposition) {
		return (CaretPos + mIME->GetCompositionCursorPos());
	}

	return CaretPos;
}


////////////////////////////////////////////////////////////////
//
//	Set_Sel
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Set_Sel (int start_index, int end_index)
{
	HilightAnchorPos	= start_index;
	HilightStartPos	= start_index;
	HilightEndPos		= end_index;
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Sel
//
////////////////////////////////////////////////////////////////
void
EditCtrlClass::Get_Sel (int &start_index, int &end_index) const
{
	start_index = HilightStartPos;
	end_index	= HilightEndPos;
	return ;
}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

bool EditCtrlClass::IsIMEAllowed(void) const
{
	// We use the ES_OEMCONVERT style to indicate that IME conversion
	// is not allowed for this control. This style bit is set in the
	// resource editor.
	return ((Style & (ES_NUMBER|ES_OEMCONVERT)) == 0);
}


#ifdef SHOW_IME_TYPING
/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void EditCtrlClass::Set_IME_Typing_Text_Pos(void)
{
	Vector2 charExtent = TextRenderer.Get_Text_Extents(U_CHAR("W"));
	int caretPos = Get_Caret_Pos();

	Vector2 pos;
	pos.X = (Pos_From_Character(caretPos) + (charExtent.X * 0.25f));
	pos.Y = (ClientRect.Top + ((ClientRect.Bottom - ClientRect.Top) * 0.25f));

	mIMETypingTip.Set_Position(pos);
}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void EditCtrlClass::Show_IME_Typing_Text(const unichar_t* text)
{
	mShowIMETypingText = (text && u_strlen(text) > 0);

	if (mShowIMETypingText) {
		Set_IME_Typing_Text_Pos();
		mIMETypingTip.Set_Text(text);
	} else {
		mIMETypingTip.Set_Text(U_CHAR(""));
	}
}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void EditCtrlClass::Hide_IME_Typing_Text(void)
{
	mShowIMETypingText = false;
	mIMETypingTip.Set_Text(U_CHAR(""));
}
#endif


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void EditCtrlClass::PositionCandidateList(void)
{
	if (mIME) {
		//-------------------------------------------------------------------------
		// Position the candidate window under the edit control
		//-------------------------------------------------------------------------
		unsigned int start = 0;
		unsigned int end = 0;
		mIME->GetTargetClause(start, end);
		int caretPos = CaretPos + start;

		Vector2 pos;
		pos.X = Pos_From_Character(caretPos);
		pos.Y = (Rect.Bottom + 2.0f);
		mCandidateList.Set_Window_Pos(pos);

		//-------------------------------------------------------------------------
		// Reposition the candidate list if it will go off the screen.
		//-------------------------------------------------------------------------
		bool reposition = false;
		const RectClass& screen = Render2DClass::Get_Screen_Resolution();
		const RectClass& ctrlRect = mCandidateList.Get_Window_Rect();

		// If the list will go off the bottom of the screen move it above
		// the edit control.
		if (ctrlRect.Bottom > screen.Bottom) {
			pos.Y = ((Rect.Top - 2.0f) - ctrlRect.Height());
			WWASSERT((pos.Y >= 0.0f) && "CandidateCtrl off the top of the screen");
			reposition = true;
		}

		// Do not allow the control to go off the right of the screen.
		if (ctrlRect.Right > screen.Right) {
			pos.X -= ((ctrlRect.Right - screen.Right) - 1);
			WWASSERT((pos.X >= 0.0f) && "CandidateCtrl of the left of the screen");
			reposition = true;
		}

		if (reposition) {
			mCandidateList.Set_Window_Pos(pos);
		}
	}
}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void EditCtrlClass::HandleNotification(IME::CompositionEvent& imeEvent)
{
	switch (imeEvent.GetAction()) {
		#ifdef SHOW_IME_TYPING
		case IME::COMPOSITION_TYPING:
			Show_IME_Typing_Text(imeEvent.Subject()->GetTypingString());
			break;
		#endif

		case IME::COMPOSITION_START:
			mInComposition = true;
			Delete_Selection();

			#ifdef SHOW_IME_TYPING
			Show_IME_Typing_Text(imeEvent.Subject()->GetTypingString());
			#endif

			Set_Dirty(true);
			break;

		case IME::COMPOSITION_CHANGE:
			Update_Scroll_Pos();
			Set_Dirty(true);
			break;

		case IME::COMPOSITION_CANCEL:
		case IME::COMPOSITION_END:
			mInComposition = false;

			#ifdef SHOW_IME_TYPING
			Hide_IME_Typing_Text();
			#endif

			// Remove hilight
			HilightStartPos = -1;
			HilightEndPos = -1;

			Set_Dirty(true);
			break;

		case IME::COMPOSITION_RESULT:
			HilightStartPos = -1;
			HilightEndPos = -1;

			Insert_String(imeEvent.Subject()->GetResultString());

			Update_Scroll_Pos();
			break;

		default:
			break;
	}
}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void EditCtrlClass::HandleNotification(IME::CandidateEvent& imeEvent)
{
	switch (imeEvent.GetAction()) {
		case IME::CANDIDATE_OPEN: {
			#ifdef SHOW_IME_TYPING
			Hide_IME_Typing_Text();
			#endif

			mCandidateList.Init(imeEvent.Subject());
			PositionCandidateList();
			Parent->Add_Control(&mCandidateList);
			}
			break;

		case IME::CANDIDATE_CHANGE:
			mCandidateList.Changed(imeEvent.Subject());
			break;

		case IME::CANDIDATE_CLOSE:
			mCandidateList.Reset();
			Parent->Remove_Control(&mCandidateList);
			break;

		default:
			break;
	}
}
