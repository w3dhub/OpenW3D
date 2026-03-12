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
 *                     $Archive:: /Commando/Code/Commando/consolefunction.h                   $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/14/02 3:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	CONSOLEFUNCTION_H
#define	CONSOLEFUNCTION_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	VECTOR_H
	#include "vector.h"
#endif


/*
** Console Function
*/
class ConsoleFunctionClass {
public:
	virtual	const char *	Get_Name( void )	= 0;
   virtual	const char *	Get_Alias( void )	{ return NULL; }//Get_Name(); }
	virtual	const char *	Get_Help( void ) = 0;
	virtual	void				Activate( const char * input ) = 0;
	static	void				Print( const char *format, ... );

	virtual ~ConsoleFunctionClass() {}
};

/*
** Console Function Manager
*/
class ConsoleFunctionManager {

public:
	static	void	Init( void );
	static	void	Shutdown( void );

	static	void	Help(	const char * function = NULL );
	static	int	Get_Function_Count(void);
	static	void	Next_Verbose_Help_Screen(void);
	static	void	Verbose_Help_File(void);
	static	void	Parse_Input( const char * input_string );

	static	bool	Get_Command_Suggestion(const char * input,const char * cur_suggestion,char * suggestion,char * help,int len);

	static	void	Print( const char *format, ... );

private:

	static	int									 Find_Function_Node(const char * name);
	static	ConsoleFunctionClass *			 Find_Command_Suggestion(const char * input, int index );
	static	void									 Sort_Function_List( void );

	static	DynamicVectorClass<ConsoleFunctionClass *>	FunctionList;	// list of all console functions
};

#endif // CONSOLEFUNCTION_H
