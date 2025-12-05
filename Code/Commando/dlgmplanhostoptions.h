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
 *                     $Archive:: /Commando/Code/commando/dlgmplanhostoptions.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/27/02 10:25a                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_LAN_HOST_OPTIONS_H
#define __DLG_MP_LAN_HOST_OPTIONS_H


#include "menudialog.h"
#include "childdialog.h"
#include "renegadedialog.h"
#include <wwlib/vector.h>
#include <wwlib/Signaler.h>


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class cGameData;
class WolGameModeClass;
class DlgMsgBoxEvent;
class ModPackageClass;
class MPLanHostMapCycleOptionsTabClass;


////////////////////////////////////////////////////////////////
//
//	MPLanHostOptionsMenuClass
//
////////////////////////////////////////////////////////////////
class MPLanHostOptionsMenuClass :
	public MenuDialogClass,
	protected Signaler<WolGameModeClass>
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MPLanHostOptionsMenuClass (void);
	~MPLanHostOptionsMenuClass();

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	void		On_Periodic(void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

	void		Enable_Mod_Selection (bool onoff);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void ReceiveSignal(WolGameModeClass&) override;
	void Start_Game(cGameData* theGame);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	bool											mStartTheGame;
	unsigned int								mClanID;
	MPLanHostMapCycleOptionsTabClass *	MapCycleDialog;
};



////////////////////////////////////////////////////////////////
//
//	MPLanHostBasicOptionsTabClass
//
////////////////////////////////////////////////////////////////
class MPLanHostBasicOptionsTabClass :
	public ChildDialogClass,
	public Signaler<bool>
{
public:
	static MPLanHostBasicOptionsTabClass* Get_Instance(void);

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MPLanHostBasicOptionsTabClass (void);
	~MPLanHostBasicOptionsTabClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	bool		On_Apply (void) override;

protected:
	void InitSideChoiceCombo(int sidePref);
	void On_EditCtrl_Change(EditCtrlClass* edit, int ctrlID) override;

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	static MPLanHostBasicOptionsTabClass* _mInstance;
	static int BandTestMaxPlayers;
};



////////////////////////////////////////////////////////////////
//
//	MPLanHostAdvancedOptionsTabClass
//
////////////////////////////////////////////////////////////////
class MPLanHostAdvancedOptionsTabClass :
	public ChildDialogClass,
	public Observer<DlgMsgBoxEvent>,
	protected Signaler<bool>
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MPLanHostAdvancedOptionsTabClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	bool		On_Apply (void) override;
	void		On_Command (int ctrl_id, int message_id, unsigned int param) override;
	void		HandleNotification(DlgMsgBoxEvent&) override;

protected:
	void ReceiveSignal(bool&) override;
	void ConfigureWOLControls(void);
	bool IsHostAClanMember(void) const;

private:
	bool mIsWOLGame;
	bool mPassword;
	bool mChangeTeams;
	bool mRemixTeams;
	bool mLaddered;
	bool mClanGame;
	bool mQuickmatch;

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
};



////////////////////////////////////////////////////////////////
//
//	MPLanHostMapCycleOptionsTabClass
//
////////////////////////////////////////////////////////////////
class MPLanHostMapCycleOptionsTabClass : public ChildDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MPLanHostMapCycleOptionsTabClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	bool		On_Apply (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void		On_ListCtrl_DblClk (ListCtrlClass *list_ctrl, int ctrl_id, int item_index) override;
	void		On_ComboBoxCtrl_Sel_Change (ComboBoxCtrlClass *combo_ctrl, int ctrl_id, int old_sel, int new_sel) override;

	void		Enable_Mod_Selection (bool onoff);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void		Build_Map_List (void);
	void		Build_Mod_Package_List (void);
	void		Add_Map (void);
	void		Remove_Map (void);

	void		Fill_Map_Ctrls (void);
	void		Populate_Map_List_Ctrl (void);
	void		Build_Map_List (const ModPackageClass *package);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	DynamicVectorClass<WideStringClass>	MapList;
	DynamicVectorClass<WideStringClass>	MapCycleList;
};



////////////////////////////////////////////////////////////////
//
//	MPLanHostVictoryOptionsTabClass
//
////////////////////////////////////////////////////////////////
class MPLanHostVictoryOptionsTabClass : public ChildDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MPLanHostVictoryOptionsTabClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	bool		On_Apply (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void		Update_Enable_State (void);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
};


/*
////////////////////////////////////////////////////////////////
//
//	MPLanHostCTFOptionsTabClass
//
////////////////////////////////////////////////////////////////
class MPLanHostCTFOptionsTabClass : public ChildDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MPLanHostCTFOptionsTabClass (void)	:
		ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_LAN_HOST_OPTIONS_CTF_SETTINGS))	{}

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void);
	bool		On_Apply (void);
};
*/

////////////////////////////////////////////////////////////////
//
//	MPLanHostCnCOptionsTabClass
//
////////////////////////////////////////////////////////////////
class MPLanHostCnCOptionsTabClass : public ChildDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MPLanHostCnCOptionsTabClass (void)	:
		ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_LAN_HOST_OPTIONS_CNC_SETTINGS))	{}

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	bool		On_Apply (void) override;
};



#endif //__DLG_MP_LAN_HOST_OPTIONS_H










