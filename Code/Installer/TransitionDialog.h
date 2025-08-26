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
 *                 Project Name : Installer                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Installer/TransitionDialog.h $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 12/15/01 5:08p                $* 
 *                                                                                             * 
 *                    $Revision:: 4                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _TRANSITION_DIALOG_H
#define _TRANSITION_DIALOG_H

// Includes.
#include "menudialog.h"
#include "Random.h"

				  
// Defines.
#define LABEL_COUNT 7
#define MESH_COUNT  5	 


// Forward declarations.
class Render2DSentenceClass;
class VertexMaterialClass;


// Classes.
class TransitionDialogClass : public MenuDialogClass
{
	public:
		 TransitionDialogClass();
		~TransitionDialogClass();

		void End_Dialog (void);
		void Render (void);

	protected:

		void	  On_Init_Dialog (void);
		void	  On_Frame_Update (void);
		Vector2 Position (int controlid, const WideStringClass &text);

		float														Time;								// Time (in seconds) that the dialog has been running.
		Render2DSentenceClass							  *TextRenderers [LABEL_COUNT];
		DynamicVectorClass <Render2DSentenceClass*>  Digits; 							
		VertexMaterialClass								  *Materials [MESH_COUNT];
};


#endif // _TRANSITION_DIALOG_H