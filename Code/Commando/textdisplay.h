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
 *                     $Archive:: /Commando/Code/Commando/textdisplay.h                       $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 9/27/01 9:41a                                               $* 
 *                                                                                             * 
 *                    $Revision:: 29                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	TEXTDISPLAY_H
#define	TEXTDISPLAY_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	GAMEMODE_H
	#include "gamemode.h"
#endif

#ifndef	SLIST_H
	#include "slist.h"
#endif

#ifndef	WWSTRING_H
	#include "wwstring.h"
#endif

#ifndef	DEBUG_H
	#include "debug.h"
#endif

#ifndef VECTOR2_H
	#include "vector2.h"
#endif

#ifndef VECTOR4_H
	#include "vector4.h"
#endif
#include "vector.h"

class	Font3DInstanceClass;
class	Render2DTextClass;

/*
**
*/
class	TextDebugDisplayHandlerClass : public DebugDisplayHandlerClass {
public:
	virtual	void	Display_Text( const char * string, const Vector4 & color = Vector4( 1,1,1,1 ) ) override;
	virtual	void	Display_Text( const WideStringClass & string, const Vector4 & color = Vector4( 1,1,1,1 ) ) override;
};

/*
** TextDisplayLine
*/
class	TextDisplayLine {

public:
	TextDisplayLine( const WideStringClass & text, unsigned int color );

	float	Update_Time( float seconds )	{ Time += seconds; return Time; }
	int	Get_Line_Count( void )			{ return LineCount; }

	WideStringClass	Text;
	unsigned int		Color;
	float					Time;
	int					LineCount;
};

/*
** TextDisplayGameMode - Game (Sub) Mode to display text
*/
#define	Get_Text_Display()		TextDisplayGameModeClass::Get_Instance()

class	TextDisplayGameModeClass : public GameModeClass {
public:
	virtual	const char *Name() override	{ return "TextDisplay"; }	// the name of this mode
	virtual	void	Init() override;		 	// called when the mode is activated
	virtual	void 	Shutdown() override; 	// called when the mode is deactivated
	virtual	void 	Think() override;			// called each time through the main loop
	virtual	void 	Render() override;		// called each time through the main loop

	static	TextDisplayGameModeClass * Get_Instance() { return Instance; }

	void 		Flush( void );

	/*
	** Scrolling Lines
	*/
	void		Set_Max_Scroll_Lines( int count )				{ MaxScrollLines = count; }
	int		Get_Max_Scroll_Lines( void )						{ return MaxScrollLines; }
	void		Set_Scroll_Lines_Persist_Time( float time )	{ ScrollLinesPersistTime = time; }
	float		Get_Scroll_Lines_Persist_Time( void )			{ return ScrollLinesPersistTime; }

	void		Print( const char * text, const Vector4 & color = Vector4( 1,1,1,1 ) );
	void		Print( const WideStringClass & text, const Vector4 & color = Vector4( 1,1,1,1 ) );
	void		Print( const char * text, const Vector3 & color = Vector3( 1,1,1 ) );
	void		Print( const WideStringClass & text, const Vector3 & color = Vector3( 1,1,1 ) );

	void		Print_System( const char * text, ... );
	void		Print_System( const WideStringClass & string );
	void		Print_Informational( const char * text, ... );
	void		Print_Informational( const WideStringClass & string );

	void		Set_Input_Text( const char * text )
	{
		TextChanged|=InputText!=text; 
		InputText = text; 
	}
	void		Set_Help_Text( const char * text )
	{
		TextChanged|=HelpText!=text; 
		HelpText = text; 
	}
	void		Set_Verbose_Help_Text( const char * text )
	{
		VerboseTextChanged|=VerboseHelpText!=text; 
		VerboseHelpText = text; 
	}
	void		Display_Vis_Warning( bool on )					{ DisplayVisWarning = on; }

private:
	Font3DInstanceClass	*	Font;
	Font3DInstanceClass	*	MonoFont;
	Render2DTextClass	*		Display;
	Render2DTextClass	*		VerboseDisplay;
	Render2DTextClass	*		StatisticsDisplay;
	DynamicVectorClass<WideStringClass> RendererLines;
	DynamicVectorClass<unsigned int> RendererColors;
	float							DisplayY;

	SList<TextDisplayLine>	ScrollLines;
	StringClass					InputText;
	StringClass					HelpText;
	StringClass					VerboseHelpText;
	bool							TextChanged;
	bool							VerboseTextChanged;
	bool							DisplayVisWarning;

	int							MaxScrollLines;
	float							ScrollLinesPersistTime;
	float							VerticalScroll;

	static TextDisplayGameModeClass * Instance;

	/*
	*/
	int							Count_Scroll_Lines( void );
	void							Load_Registry_Keys(void);
	void							Save_Registry_Keys(void);
};


/*
** Statistics Display
*/
class	StatisticsDisplayManager {
public:
	static	void	Render( Render2DTextClass * renderer );
	static	void	Set_Display( const char * title );
	static	bool	Is_Current_Display( const char* title); // Returns true if "title" is currently active
	static	void	Set_Stat( const char * title, const char * text, unsigned int color = 0xffffffff, const Vector2& location = Vector2( 0, -240 ) );
};

#endif
