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
 *                     $Archive:: /Commando/Code/Commando/ConsoleMode.h                       $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/14/02 12:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include <win.h>

class WideStringClass;
class StringClass;
class Vector3;

/*
** This is a console interface for the game so you can do stuff even if you can't display the main game window.
**
**
*/
class ConsoleModeClass
{
	public:
		/*
		** Constructor, destructor.
		*/
		ConsoleModeClass(void);
		~ConsoleModeClass(void);

		/*
		** Init.
		*/
		void Init(void);
		void Set_Title(char *name, char *settings);

		/*
		** Input processing.
		*/
		void Think();
		void Wait_For_Keypress(void);

		/*
		** Console output.
		*/
		void Print(char const * string, ...);
		void Print_Maybe(char const * string, ...);
		static void Static_Print_Maybe(char const * string, ...);
		void Add_Message(WideStringClass *formatted_text, Vector3 *text_color, bool forced = false);

		/*
		** Profiling support.
		*/
		void Update_Profile(StringClass profile_string);
		void Set_Profile_Mode(bool set) {ProfileMode = set; LastProfileCRC = 0; LastProfilePrint = 0;}
		void Handle_Profile_Key(int key);

		/*
		** Master/Slave console window settings/support.
		*/
		void Set_Exclusive(bool set) {IsExclusive = set;}
		bool Is_Exclusive(void) {return(IsExclusive);}
		HWND Get_Slave_Window_By_Title(char *name, char *settings);
		StringClass Compose_Window_Title(char *name, char *settings, bool slave);
		void cprintf(char const * string, ...);

		/*
		** File logging.
		*/
		void Log_To_Disk(const char *string);
		const char *Get_Log_File_Name(void);

	private:
		/*
		** Private functions.
		*/
		void Apply_Attributes(void);

		/*
		** Input and output handles.
		*/
		HANDLE	ConsoleInputHandle;
		HANDLE	ConsoleOutputHandle;

		/*
		** Window handle.
		*/
		HWND		ConsoleWindow;

		/*
		** Input parsing variables.
		*/
		unsigned long LastKeypressTime;
		int		Pos;

		/*
		** Console title bar contents.
		*/
		char		Title[256];

		/*
		** Is console mode the only mode?
		*/
		bool		IsExclusive;

		/*
		** Is the console in profile mode?
		*/
		bool		ProfileMode;

		/*
		** Last profile text CRC.
		*/
		unsigned long LastProfileCRC;

		/*
		** Last time we printed the profile.
		*/
		unsigned long LastProfilePrint;

};

/*
** Single instance of console.
*/
extern ConsoleModeClass ConsoleBox;