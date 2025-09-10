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
 *                     $Archive:: /Commando/Code/Installer/TransitionDialog.c $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 12/15/01 6:37p                $* 
 *                                                                                             * 
 *                    $Revision:: 6                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "TransitionDialog.h"
#include "AssetMgr.h"
#include "DialogText.h"
#include "HAnim.h"
#include "MenuBackDrop.h"
#include "Mesh.h"
#include "MeshMdl.h"
#include "MessageBox.h"
#include "mousemgr.h"
#include "RendObj.h"
#include "resource.h"
#include "stylemgr.h"
#include "Translator.h"
#include "Scene.h"
#include "TooltipMgr.h"
#include "Ww3D.h"
 

/***********************************************************************************************
 * TransitionDialogClass::TransitionDialogClass -- Constructor											  *		
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
TransitionDialogClass::TransitionDialogClass()
	: MenuDialogClass (IDD_DIALOG_TRANSITION),
	  Time (0.0f)
{
	const char *modelname = "IN_STATICTRANS2";
	const char *animname  = "IN_STATICTRANS2.IN_STATICTRANS2";
	
	Get_BackDrop()->Set_Model (modelname);
	Get_BackDrop()->Set_Animation (animname);

	// Null materials.
	for (unsigned mesh = 0; mesh < MESH_COUNT; mesh++) {
		Materials [mesh] = NULL;
	}
}


/***********************************************************************************************
 * TransitionDialogClass::~TransitionDialogClass -- Destructor											  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
TransitionDialogClass::~TransitionDialogClass()
{
	Get_BackDrop()->Remove_Model();
}


/***********************************************************************************************
 * TransitionDialogClass::On_Init_Dialog --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void TransitionDialogClass::On_Init_Dialog (void)
{
	const int textstringids [LABEL_COUNT]  = {IDS_INSTALLER_TEXT094,
															IDS_INSTALLER_TEXT070,
															IDS_SCANNING_LOCAL_RADAR,
															IDS_BIO_MONITOR_ACTIVATING,
															IDS_UPDATING_WEAPON_STATUS,
															IDS_INITIALIZING_OBJECTIVES_SYSTEM,
															IDS_TARGETING_SYSTEM_ONLINE};

	const int textcontrolids [LABEL_COUNT] = {IDC_EVA_UPDATING1,
															IDC_PLEASE_STANDBY,
															IDC_SCANNING_LOCAL_RADAR,
															IDC_BIO_MONITOR_ACTIVATING,
															IDC_UPDATING_WEAPON_STATUS,
															IDC_INITIALIZING_OBJECTIVES_SYSTEM,
															IDC_TARGETING_SYSTEM_ONLINE};

	const char *meshnames [MESH_COUNT]		= {"RADAR",
															"HEALTH",
															"WEAPONS",
															"POGS",
															"TARGETING"};

	MouseMgrClass::Show_Cursor (false);

	// Create labels.
	for (unsigned label = 0; label < LABEL_COUNT; label++) {
		
		TxWideStringClass text (textstringids [label]);
		
		TextRenderers [label] = new Render2DSentenceClass;
		StyleMgrClass::Assign_Font (TextRenderers [label], StyleMgrClass::FONT_CREDITS);
		TextRenderers [label]->Build_Sentence (text);
		TextRenderers [label]->Set_Location (Position (textcontrolids [label], text));
	}

	Digits.Set_Growth_Step (1024);

	// For each model...
	for (unsigned mesh = 0; mesh < MESH_COUNT; mesh++) {
		for (int index = 0; index < Get_BackDrop()->Peek_Model()->Get_Num_Sub_Objects(); index++) {
			
			RenderObjClass *sub_obj = Get_BackDrop()->Peek_Model()->Get_Sub_Object (index);
			
			if (sub_obj->Class_ID() == RenderObjClass::CLASSID_MESH) {
				if (::strstr (sub_obj->Get_Name(), meshnames [mesh]) != 0) {
					Materials [mesh] = ((MeshClass*) sub_obj)->Peek_Model()->Get_Single_Material();
				}
			}
			REF_PTR_RELEASE (sub_obj);
		}
	}
	MenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * TransitionDialogClass::End_Dialog --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void TransitionDialogClass::End_Dialog (void)
{
	MouseMgrClass::Show_Cursor (true);

	// Remove materials.
	for (unsigned mesh = 0; mesh < MESH_COUNT; mesh++) {
		REF_PTR_RELEASE (Materials [mesh]);
	}

	// Remove all digits.
	for (int d = 0; d < Digits.Count(); d++) {
		delete Digits [d];
	}

	// Remove labels.
	for (unsigned label = 0; label < LABEL_COUNT; label++) {
		delete TextRenderers [label];
	}

	MenuDialogClass::End_Dialog();
}


/***********************************************************************************************
 * TransitionDialogClass::On_Frame_Update --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void TransitionDialogClass::On_Frame_Update (void)
{
	static Random2Class _randomnumber (0x236ae371);
	
	const float labelcontroltimes [LABEL_COUNT][4] = {{ 1.67f,  1.73f,  6.60f,  6.67f},
																	  { 6.86f,  6.93f, 17.59f, 17.67f},
																	  {17.97f, 17.97f, 38.97f, 38.97f},
																	  {21.97f, 21.97f, 38.97f, 38.97f},
																	  {25.97f, 25.97f, 38.97f, 38.97f},
																	  {29.97f, 29.97f, 38.97f, 38.97f},
																	  {33.97f, 33.97f, 38.97f, 38.97f}};

	const float digitstarttime =  7.00f;
	const float digitdecaytime = 14.00f;
	const float digitendtime	= 15.00f;

	const float meshcontroltimes [MESH_COUNT][4] = {{18.97f, 20.97f, 38.97f, 38.97f},
																	{22.97f, 24.97f, 38.97f, 38.97f},
																	{26.97f, 28.97f, 38.97f, 38.97f},
																	{30.97f, 32.97f, 38.97f, 38.97f},
																	{34.97f, 36.97f, 38.97f, 38.97f}};

	Time += WW3D::Get_Frame_Time() * 0.001f;

	// For each label...
	for (unsigned label = 0; label < LABEL_COUNT; label++) {

		uint32 color;
		float	 f;	

		if ((Time < labelcontroltimes [label][0]) || (Time > labelcontroltimes [label][3])) {
			f = 0.0f;
		} else {
			if (Time < labelcontroltimes [label][1]) {
				f = (Time - labelcontroltimes [label][0]) / (labelcontroltimes [label][1] - labelcontroltimes [label][0]);
			} else {
				if (Time < labelcontroltimes [label][2]) {
					f = 1.0f;
				} else {
					f = 1.0f - (Time - labelcontroltimes [label][2]) / (labelcontroltimes [label][3] - labelcontroltimes [label][2]);
				}
			}
		}
		color = ((uint32)(f * 0xff)) << 24;

		TextRenderers [label]->Reset_Polys();
		TextRenderers [label]->Draw_Sentence (color | (RGBA_TO_INT32 (255, 213, 40, 0)));
	}

	// Process binary digits.
	if (Time > digitstarttime) {

		if (Time < digitendtime) {

			const int digitthreshhold = 32;

			if (Time < digitdecaytime) {

				const unsigned  randomness = 32;
				const wchar_t		*text [2]	= {L"0", L"1"};
				
				Render2DSentenceClass *digit;
				int						  w, h, bits;
				bool						  windowed;
				float						  oow, ooh;

				// Remove a digit?
				if (Digits.Count() > digitthreshhold) {
					delete Digits [0];
					Digits.Delete (0);
				}


				// Add another digit.
				WW3D::Get_Render_Target_Resolution (w, h, bits, windowed);
				oow = ((float) w) / randomness;
				ooh = ((float) h) / randomness;
				digit = new Render2DSentenceClass;
				StyleMgrClass::Assign_Font (digit, StyleMgrClass::FONT_CREDITS);
				digit->Build_Sentence (text [_randomnumber (0, 1)]);
				digit->Set_Location (Vector2 (floorf (_randomnumber (0, randomness) * oow), floorf (_randomnumber (0, randomness) * ooh)));
				digit->Reset_Polys();
				digit->Draw_Sentence (RGBA_TO_INT32 (255, 213, 40, 255));
				Digits.Add (digit);
			
			} else {

				// Remove a digit?
				if (Digits.Count() > 0) {
					delete Digits [0];
					Digits.Delete (0);
				}
			}

		} else {

			// Remove all digits.
			for (int d = 0; d < Digits.Count(); d++) {
				delete Digits [d];
			}
			Digits.Clear();
 		}
	}

	// For each mesh...
	for (unsigned mesh = 0; mesh < MESH_COUNT; mesh++) {

		float	 f;	

		if ((Time < meshcontroltimes [mesh][0]) || (Time > meshcontroltimes [mesh][3])) {
			f = 0.0f;
		} else {
			if (Time < meshcontroltimes [mesh][1]) {
				f = (Time - meshcontroltimes [mesh][0]) / (meshcontroltimes [mesh][1] - meshcontroltimes [mesh][0]);
			} else {
				if (Time < meshcontroltimes [mesh][2]) {
					f = 1.0f;
				} else {
					f = 1.0f - (Time - meshcontroltimes [mesh][2]) / (meshcontroltimes [mesh][3] - meshcontroltimes [mesh][2]);
				}
			}
		}
		Materials [mesh]->Set_Opacity (f);
	}

	MenuDialogClass::On_Frame_Update();
	
	// Has the animation played out?
	if (Time > Get_BackDrop()->Peek_Model()->Peek_Animation()->Get_Total_Time()) {
		End_Dialog();
	}
}


/***********************************************************************************************
 * TransitionDialogClass::Position --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
Vector2 TransitionDialogClass::Position (int controlid, const WideStringClass &text)
{
	DialogTextClass *textcontrol;
	RectClass		  rect;	
	Vector2			  textextent;	
	DWORD				  style;
	Vector2			  position;

	textcontrol	= Get_Dlg_Item (controlid)->As_DialogTextClass();
	rect			= textcontrol->Get_Client_Rect();
	textextent	= TextRenderers [0]->Get_Text_Extents (text);
	style			= textcontrol->Get_Style();

	if ((style & 0xF) == SS_RIGHT) {
		position.X = rect.Right - textextent.X;
	} else {
		if ((style & 0xF) == SS_CENTER) {
			position.X = rect.Left + (0.5f * (rect.Width() - textextent.X));
		} else {
			position.X = rect.Left;
		}
	}
	position.Y = rect.Top;

	position.X = (int) position.X;
	position.Y = (int) position.Y;
	return (position);
}


/***********************************************************************************************
 * TransitionDialogClass::Render --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void TransitionDialogClass::Render (void)
{
	MenuDialogClass::Render();

	// Render the binary digits.
	for (int d = 0; d < Digits.Count(); d++) {
		Digits [d]->Render();
	}

	// Render the labels.
	for (unsigned label = 0; label < LABEL_COUNT; label++) {
		TextRenderers [label]->Render();
	}
}
