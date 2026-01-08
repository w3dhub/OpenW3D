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
 *                     $Archive:: /Commando/Code/Combat/messagewindow.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/13/01 3:07p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MESSAGE_WINDOW_H
#define __MESSAGE_WINDOW_H

#include "wwstring.h"
#include "vector.h"
#include "vector3.h"
#include "textwindow.h"
#include "render2d.h"

////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class SmartGameObj;
class RenderObjClass;

class	SimpleSceneClass;
class	CameraClass;


////////////////////////////////////////////////////////////////
//
//	MessageWindowClass
//
////////////////////////////////////////////////////////////////
class MessageWindowClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MessageWindowClass (void);
	~MessageWindowClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	void				Initialize (void);
	void				Shutdown (void);

	//
	//	Content control
	//
	void				Add_Message (const WideStringClass &message, const Vector3 &color = Vector3 (0, 0.9F, 0.2F), SmartGameObj *game_obj = NULL, float decay_time = 0);
	void				Clear (void);

	//
	//	Render methods
	//
	void				On_Frame_Update (void);
	void				Render (void);

	//
	//	Visibility control
	//
	bool				Has_Data (void) const;
	void				Force_Display (bool onoff);
	
	//
	//	Display rectangle control
	//
	void				Update_Window_Rectangle (void);
	void				Reset_Current_Rect (void);
	void				Set_Window_Dirty (bool onoff)		{ IsRectangleDirty = onoff; }

	//
	//	Decay control
	//
	uint32			Get_Decay_Time (void) const	{ return DecayTime; }
	void				Set_Decay_Time (uint32 time)	{ DecayTime = time; }

	//
	//	Message log support
	//
	void					Clear_Log (void)				{ MessageLog.Reset_Active (); MessageLogColor.Reset_Active (); }
	int					Get_Log_Count (void)			{ return MessageLog.Count (); }
	const unichar_t *		Get_Log_Entry (int index)	{ return MessageLog[index]; }
	const Vector3 &	Get_Log_Color (int index)	{ return MessageLogColor[index]; }

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////	

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	uint32				DecayTime;
	TextWindowClass *	TextWindow;
	Render2DClass		IconRenderer;
	RectClass			CurrentRect;
	RenderObjClass *	HeadModel;
	SimpleSceneClass  * Scene;
	CameraClass			* Camera;
	bool					IsRectangleDirty;
	
	DynamicVectorClass<WideStringClass>	MessageLog;
	DynamicVectorClass<Vector3> MessageLogColor;
};


#endif //__MESSAGE_WINDOW_H

