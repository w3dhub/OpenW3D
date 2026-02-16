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
 *                     $Archive:: /Commando/Code/Commando/textdisplay.cpp        $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/02/02 2:27a                                               $*
 *                                                                                             *
 *                    $Revision:: 55                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "textdisplay.h"
#include "font3d.h"
#include "assets.h"
#include "render2d.h"
#include "timemgr.h"
#include "_globals.h"
#include "registry.h"
#include "wwprofile.h"
#include "combat.h"
#include "ccamera.h"
#include "render2dsentence.h"
#include "wwmemlog.h"
#include "ConsoleMode.h"

/*
** TextDisplayLine
*/
TextDisplayLine::TextDisplayLine( const WideStringClass & text, unsigned int color ) :
	Time( 0 ),
	LineCount( 0 ),
	Text( text ),
	Color( color )
{
	for( const unichar_t * ch = text; *ch; ch++ ) {
		if ( *ch == (unichar_t)'\n' ) {
			LineCount++;
		}
	}
}

/*
** TextDisplayGameMode
*/
TextDisplayGameModeClass * TextDisplayGameModeClass::Instance = NULL;

void	TextDisplayGameModeClass::Init()
{
	if (!ConsoleBox.Is_Exclusive()) {
		// Build Fonts
		WWASSERT(WW3DAssetManager::Get_Instance() != NULL);
		Font = WW3DAssetManager::Get_Instance()->Get_Font3DInstance( "FONT8x8.TGA" );
   	WWASSERT(Font != NULL);
		SET_REF_OWNER( Font );
		MonoFont = WW3DAssetManager::Get_Instance()->Get_Font3DInstance( "FONT8x8.TGA" );
   	WWASSERT(MonoFont != NULL);
		SET_REF_OWNER( MonoFont );
		MonoFont->Set_Mono_Spaced();


		// Update Instance
		WWASSERT( TextDisplayGameModeClass::Instance == NULL );
		TextDisplayGameModeClass::Instance = this;

		DisplayVisWarning = false;
		MaxScrollLines = 30;
		ScrollLinesPersistTime = 10;

		Load_Registry_Keys();

		Display = new Render2DTextClass(Font);
		Display->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

		VerboseDisplay = new Render2DTextClass(Font);
		VerboseDisplay->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

		StatisticsDisplay = new Render2DTextClass(MonoFont);
		StatisticsDisplay->Set_Coordinate_Range( Render2DClass::Get_Screen_Resolution() );

		RendererLines.Reset_Active();
		RendererColors.Reset_Active();
		TextChanged=true;
		VerboseTextChanged=true;
		DisplayY=0.0f;
	}
	return ;
}

void 	TextDisplayGameModeClass::Shutdown()
{
	if (!ConsoleBox.Is_Exclusive()) {
		Save_Registry_Keys();

   	Flush();

		delete StatisticsDisplay;
		StatisticsDisplay=NULL;

		delete VerboseDisplay;
		VerboseDisplay=NULL;

		delete Display;
		Display=NULL;

		REF_PTR_RELEASE( Font );
		REF_PTR_RELEASE( MonoFont );

		// Update Instance
		WWASSERT( TextDisplayGameModeClass::Instance == this );
		TextDisplayGameModeClass::Instance = NULL;
	}
}

/*
**
*/
void TextDisplayGameModeClass::Load_Registry_Keys(void)
{
	RegistryClass * registry = new RegistryClass( APPLICATION_SUB_KEY_NAME_OPTIONS );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {
		MaxScrollLines = registry->Get_Int( "MaxScrollLines", MaxScrollLines );
		ScrollLinesPersistTime = registry->Get_Float( "ScrollLinesPersistTime", ScrollLinesPersistTime );
	}
	delete registry;
}

void TextDisplayGameModeClass::Save_Registry_Keys(void)
{
	RegistryClass * registry = new RegistryClass( APPLICATION_SUB_KEY_NAME_OPTIONS );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {
		registry->Set_Int( "MaxScrollLines", MaxScrollLines );
		registry->Set_Float( "ScrollLinesPersistTime", ScrollLinesPersistTime );
	}
	delete registry;
}

/*
**
*/
void 	TextDisplayGameModeClass::Think()
{
	WWPROFILE( "TextDisplay Think" );

	if (Font == NULL) {
		return;
	}
	// Update time on all scroll lines
	SLNode<TextDisplayLine> *linenode;
	for (	linenode = ScrollLines.Head(); linenode; linenode = linenode->Next() ) {
		TextDisplayLine *line = linenode->Data();
		line->Update_Time( TimeManager::Get_Frame_Seconds() );
	}

	// never have more than 2 over the max number
	while ( ScrollLines.Get_Count() && (Count_Scroll_Lines() >= MaxScrollLines + 2) ) {
		delete ScrollLines.Remove_Head();
	}

	// if there are lines to remove by count or time
	if ( ScrollLines.Get_Count() &&
				((ScrollLines.Head()->Data()->Update_Time(0) > ScrollLinesPersistTime) ||
				 (Count_Scroll_Lines() > MaxScrollLines) ) ) {
		// bump scroll position
		float char_height = Font->Char_Height();
		VerticalScroll += TimeManager::Get_Frame_Seconds() * char_height * 4;
		if ( VerticalScroll >= char_height ) {
			// top line is hidden, remove it
			VerticalScroll -= char_height;
			int lines = Count_Scroll_Lines();
			while ( ScrollLines.Get_Count() && (Count_Scroll_Lines() >= lines) ) {
				delete ScrollLines.Remove_Head();
			}
		}
	}
	// if no more need to be removed, clear scroll
	if ( ScrollLines.Get_Count() &&
				((ScrollLines.Head()->Data()->Update_Time(0) > ScrollLinesPersistTime) ||
				 (Count_Scroll_Lines() > MaxScrollLines) ) ) {
	} else {
		VerticalScroll = 0;
	}
}

void 	TextDisplayGameModeClass::Render()
{
	WWMEMLOG(MEM_GAMEDATA);
	WWPROFILE( "TextDisplay Render" );

	if (Font == NULL) {
		return;
	}

	// Verbose help first
	if (VerboseTextChanged) {
		VerboseTextChanged=false;
		VerboseDisplay->Reset();
		VerboseDisplay->Set_Location( Vector2( 0, 0 ) );
		VerboseDisplay->Draw_Text( VerboseHelpText, 0xFF00FFFF );
	}
	VerboseDisplay->Render();

	// Then statistics

	StatisticsDisplayManager::Render( StatisticsDisplay );

	// And then the rest of the text...

	bool changed=TextChanged;
	TextChanged=false;
	if ( DisplayVisWarning && StatisticsDisplayManager::Is_Current_Display( "fps" ) ) {
		changed=true;
	}

	// never have more than 2 over the max number
	while ( ScrollLines.Get_Count() && (Count_Scroll_Lines() >= MaxScrollLines + 2) ) {
		delete ScrollLines.Remove_Head();
		changed=true;
	}

	// Check if the text lines have changed
	SLNode<TextDisplayLine> *linenode;
	if (!changed) {
		int j=0;
		WWASSERT(RendererLines.Count()==RendererColors.Count());
		for (	linenode = ScrollLines.Head(); linenode; linenode = linenode->Next(), j++ ) {
			if (j>=RendererLines.Count()) {
				changed=true;
				break;
			}
			if (RendererColors[j]!=linenode->Data()->Color || RendererLines[j]!=linenode->Data()->Text) {
				changed=true;
				break;
			}
		}
	}

	float y=WWMath::Floor( -VerticalScroll );
	if (y==0.0f && DisplayY!=0.0f) changed=true;
	float delta=y-DisplayY;
	DisplayY=y;

	if (!changed) {
		if (delta!=0.0f) Display->Move(Vector2(0.0f,delta));
		Display->Render();
		return;
	}

	// Display
	Display->Reset();

	// Cache the lines that are rendered
	RendererLines.Reset_Active();
	RendererColors.Reset_Active();
	for (	linenode = ScrollLines.Head(); linenode; linenode = linenode->Next() ) {
		Display->Draw_Text( linenode->Data()->Text, linenode->Data()->Color );
		RendererLines.Add(linenode->Data()->Text);
		RendererColors.Add(linenode->Data()->Color);
	}

	Display->Draw_Text( InputText );
	Display->Draw_Text( HelpText );

	if ( DisplayVisWarning && StatisticsDisplayManager::Is_Current_Display( "fps" ) ) {
		Vector2 pos = (Render2DClass::Get_Screen_Resolution().Upper_Left() + Render2DClass::Get_Screen_Resolution().Lower_Left()) * 0.5f;
		Display->Set_Location( pos );
		Display->Draw_Text( "Vis Sector Not Found!", 0xFF0000FF );
	}

	Display->Render();
}

void 	TextDisplayGameModeClass::Flush( void )
{
	while ( ScrollLines.Get_Count() ) {
		delete ScrollLines.Remove_Head();
	}
}

int	TextDisplayGameModeClass::Count_Scroll_Lines( void )
{
	int count = 0;
	SLNode<TextDisplayLine> *linenode;
	for (	linenode = ScrollLines.Head(); linenode; linenode = linenode->Next() ) {
		count += linenode->Data()->Get_Line_Count();
	}
	return count;
}

/*
** Printing
*/
void	TextDisplayGameModeClass::Print( const char * string, const Vector4 & color )
{
	if (Font == NULL) {
		return;
	}
	unsigned int col=(unsigned(color[0]*255.0f)<<24)|(unsigned(color[1]*255.0f)<<16)|(unsigned(color[2]*255.0f)<<8)|(unsigned(color[3]*255.0f));
	WideStringClass wide_string;
	wide_string.Convert_From( string );
	TextDisplayLine * line = new TextDisplayLine( wide_string, col );
   ScrollLines.Add_Tail( line );
}

void	TextDisplayGameModeClass::Print( const WideStringClass & string, const Vector4 & color )
{
	if (Font == NULL) {
		return;
	}
	unsigned int col=(unsigned(color[0]*255.0f)<<24)|(unsigned(color[1]*255.0f)<<16)|(unsigned(color[2]*255.0f)<<8)|(unsigned(color[3]*255.0f));
	TextDisplayLine * line = new TextDisplayLine( string, col );
   ScrollLines.Add_Tail( line );
}

void	TextDisplayGameModeClass::Print( const char * string, const Vector3 & color )
{
	if (Font == NULL) {
		return;
	}
	//TSS unsigned int col=(unsigned(color[0]*255.0f)<<24)|(unsigned(color[1]*255.0f)<<16)|(unsigned(color[2]*255.0f)<<8)|0xff;
	unsigned int col=(unsigned(color[0]*255.0f)<<16)|(unsigned(color[1]*255.0f)<<8)|(unsigned(color[2]*255.0f))|0xFF000000;
	WideStringClass wide_string;
	wide_string.Convert_From( string );
	TextDisplayLine * line = new TextDisplayLine( wide_string, col );
   ScrollLines.Add_Tail( line );
}


void	TextDisplayGameModeClass::Print( const WideStringClass & string, const Vector3 & color )
{
	if (Font == NULL) {
		return;
	}
	//TSS unsigned int col=(unsigned(color[0]*255.0f)<<24)|(unsigned(color[1]*255.0f)<<16)|(unsigned(color[2]*255.0f)<<8)|0xff;
	unsigned int col=(unsigned(color[0]*255.0f)<<16)|(unsigned(color[1]*255.0f)<<8)|(unsigned(color[2]*255.0f))|0xFF000000;
	TextDisplayLine * line = new TextDisplayLine( string, col );
   ScrollLines.Add_Tail( line );
}

void	TextDisplayGameModeClass::Print_System( const char * format, ... )
{
	va_list arg_list;
	va_start (arg_list, format);
	StringClass string;
	string.Format_Args( format, arg_list );
	va_end (arg_list);

	Print( string, Vector4( 1,1,0,1 ) );
}

void	TextDisplayGameModeClass::Print_System( const WideStringClass & string )
{
	Print( string, Vector4( 1,1,0,1 ) );
}


void	TextDisplayGameModeClass::Print_Informational( const char * format, ... )
{
	va_list arg_list;
	va_start (arg_list, format);
	StringClass string;
	string.Format_Args( format, arg_list );
	va_end (arg_list);

	Print( string, Vector4( 0.7f,0.7f,0.7f,1.0f ) );
}

void	TextDisplayGameModeClass::Print_Informational( const WideStringClass & string )
{
	Print( string, Vector4( 0.7f,0.7f,0.7f,1.0f ) );
}

/*
** TextDebugDisplayHandler
*/
void	TextDebugDisplayHandlerClass::Display_Text( const char * string, const Vector4 & color )
{
	if (Get_Text_Display() != NULL) {
		Get_Text_Display()->Print( string, color );
	}
}

void	TextDebugDisplayHandlerClass::Display_Text( const WideStringClass & string, const Vector4 & color )
{
	if (Get_Text_Display() != NULL) {
		Get_Text_Display()->Print( string, color );
	}
}


/*
** StatisticsDisplay
*/
static float													_StatsScrollOffset = 0;
static StringClass											_StatsTitle;
static StringClass											_StatsText;
static unsigned int											_StatsColor = 0xFFFFFFFF;
static Vector2													_StatsLocation( 0,0 );

void	StatisticsDisplayManager::Render(Render2DTextClass * renderer)
{
	if (WWDEBUG_TRIGGER(WWDEBUG_TRIGGER_GENERIC0)) {
		_StatsScrollOffset -= 32.0f;
	}
	if (WWDEBUG_TRIGGER(WWDEBUG_TRIGGER_GENERIC1)) {
		_StatsScrollOffset += 32.0f;
	}

	renderer->Reset();
	renderer->Set_Location( _StatsLocation );
	renderer->Draw_Text( _StatsText, _StatsColor );
	renderer->Render();
}

void	StatisticsDisplayManager::Set_Display( const char * title )
{
	_StatsScrollOffset = 0;
	_StatsTitle = title;
	_StatsText = "";
}

bool StatisticsDisplayManager::Is_Current_Display( const char* title) // Returns true if "title" is currently active
{
	return !stricmp( _StatsTitle, title );
}

void	StatisticsDisplayManager::Set_Stat( const char * title, const char * text, unsigned int color, const Vector2& location )
{
	if ( Is_Current_Display( title ) ) {
		_StatsText = text;
		_StatsColor = color;

		if ( location.Y == -240 ) {
			// Handle default case
			Vector2 pos = (Render2DClass::Get_Screen_Resolution().Upper_Left() + Render2DClass::Get_Screen_Resolution().Upper_Right()) * 0.5f;
			_StatsLocation = pos;
		} else {
			_StatsLocation = location;
		}

	}
}
