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
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GameMtlForm.h                  $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/17/02 3:14p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#ifndef GAMEMTLFORM_H
#define GAMEMTLFORM_H

#include "FormClass.h"

class GameMtl;


#if defined W3D_GMAXDEV

class FakeIMtlParams : public IMtlParams{
public:
	TimeValue GetTime(){ return GetCOREInterface()->GetTime();}
	virtual void MtlChanged(){};
	virtual HWND AddRollupPage( HINSTANCE hInst, DLGTEMPLATE *dlgTemplate,
		DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD ){return NULL;}
	virtual HWND AddRollupPage( HINSTANCE hInst, TCHAR *dlgTemplate,
		DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD ){return NULL;};

	virtual HWND ReplaceRollupPage( HWND hOldRollup, HINSTANCE hInst, TCHAR *dlgTemplate,
		DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD  ){return NULL;}

	virtual HWND ReplaceRollupPage( HWND hOldRollup, HINSTANCE hInst, DLGTEMPLATE *dlgTemplate,
		DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD  ){return NULL;};

	virtual void DeleteRollupPage( HWND hRollup ){};


	virtual void RollupMouseMessage( HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam ){};
	virtual void RegisterTimeChangeCallback(TimeChangeCallback *tc){}
	virtual void UnRegisterTimeChangeCallback(TimeChangeCallback *tc){}

	virtual void RegisterDlgWnd( HWND hDlg ){};
	virtual int UnRegisterDlgWnd( HWND hDlg ){return 0;};

	// get the current time.

	// Pick an object from the scene
	virtual void SetPickMode(PickObjectProc *proc){};
	virtual void EndPickMode(){};

	// JBW 10/19/98: get interface to mtl editor rollup
	virtual IRollupWindow *GetMtlEditorRollup(){return NULL;}

	virtual int IsRollupPanelOpen(HWND hwnd){return 0;}
	virtual int GetRollupScrollPos(){return 0;}
	virtual void SetRollupScrollPos(int spos){}
	Mtl*	Multi_mtl_list;
};
#endif



class GameMtlFormClass : public FormClass
{
public:
	GameMtlFormClass(IMtlParams * imtl_params,GameMtl * mtl,int pass);

	void					SetThing(ReferenceTarget *m);
	ReferenceTarget*	GetThing(void);
	void					DeleteThis(void);
	Class_ID				ClassID(void);
	void					SetTime(TimeValue t);

protected:
#if !defined W3D_GMAXDEV
	IMtlParams *		IParams;			// interface to the material editor
#else
	FakeIMtlParams *		IParams;			// interface to the material editor
#endif
	GameMtl *			TheMtl;			// current mtl being edited.
	int					PassIndex;		// material pass that this form edits
};

#endif
