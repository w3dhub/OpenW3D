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
 *                     $Archive:: /Commando/Code/Commando/dialogtests.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/16/02 3:55p                                               $*
 *                                                                                             *
 *                    $Revision:: 37                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DIALOG_TESTS_H
#define __DIALOG_TESTS_H

#include "popupdialog.h"
#include "menudialog.h"
#include "childdialog.h"
#include "renegadedialog.h"
#include "DlgWOLWait.h"
#include <wwlib/Notify.h>
#include <WWOnline/RefPtr.h>


class WheeledVehicleDefClass;
class TrackedVehicleDefClass;


////////////////////////////////////////////////////////////////
//
//	SplashIntroMenuDialogClass
//
////////////////////////////////////////////////////////////////
class SplashIntroMenuDialogClass : public MenuDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	SplashIntroMenuDialogClass (void);
	~SplashIntroMenuDialogClass (void);
	
	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Frame_Update (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

	//
	//	Static accessors
	//
	static bool	Is_Complete (void)	{ return IsComplete; }

private:
	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	float				Timer;
	static bool		IsComplete;
};


////////////////////////////////////////////////////////////////
//
//	SplashOutroMenuDialogClass
//
////////////////////////////////////////////////////////////////
class SplashOutroMenuDialogClass : public MenuDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	SplashOutroMenuDialogClass (void);
	~SplashOutroMenuDialogClass (void);
	
	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Frame_Update (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

private:
	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	float		Timer;
};


////////////////////////////////////////////////////////////////
//
//	GameSpyMainDialogClass
//
////////////////////////////////////////////////////////////////
class GameSpyMainDialogClass : public MenuDialogClass,
public Observer<DlgWOLWaitEvent>

{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	GameSpyMainDialogClass (void);
	~GameSpyMainDialogClass (void);
	
	//
	//	Inherited
	//
	void		On_Init_Dialog(void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void		On_Last_Menu_Ending (void) override;
	void		On_Frame_Update (void) override;

	//
	//	Singleton access
	//
	static void								Display (void);
	static GameSpyMainDialogClass *	Get_Instance (void)	{ return _TheInstance; }

private:

	void HandleNotification(DlgWOLWaitEvent& event) override;
	static void Host_Game(void);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	static GameSpyMainDialogClass *	_TheInstance;
	static bool	DetectingBandwidth;

};


////////////////////////////////////////////////////////////////
//
//	GameSpyOptionsDialogClass
//
////////////////////////////////////////////////////////////////
class GameSpyOptionsDialogClass : public MenuDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	GameSpyOptionsDialogClass (void);
	~GameSpyOptionsDialogClass (void);
	
	//
	//	Inherited
	//
	void		On_Init_Dialog(void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	//void		On_Last_Menu_Ending (void);

	//
	//	Singleton access
	//
	static void								Display (void);
	static GameSpyOptionsDialogClass *	Get_Instance (void)	{ return _TheInstance; }

private:
	void		Init_Connection_Speed_Combo(void);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	static GameSpyOptionsDialogClass *	_TheInstance;
};


////////////////////////////////////////////////////////////////
//
//	InternetMainDialogClass
//
////////////////////////////////////////////////////////////////
class InternetMainDialogClass : public MenuDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	InternetMainDialogClass (void);
	~InternetMainDialogClass (void);
	
	//
	//	Inherited
	//
	void		On_Init_Dialog(void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	//void		On_Last_Menu_Ending (void);

	//
	//	Singleton access
	//
	static void								Display (void);
	static InternetMainDialogClass *	Get_Instance (void)	{ return _TheInstance; }

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	static InternetMainDialogClass *	_TheInstance;
};


////////////////////////////////////////////////////////////////
//
//	StartSPGameDialogClass
//
////////////////////////////////////////////////////////////////
class StartSPGameDialogClass : public MenuDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	StartSPGameDialogClass (void);	
	
	void			On_Init_Dialog (void) override;
	void			On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

	//void	On_TreeCtrl_Needs_Children (TreeCtrlClass *list_ctrl, int ctrl_id, TreeItemClass *parent_item);
};


////////////////////////////////////////////////////////////////
//
//	DifficultyMenuClass
//
////////////////////////////////////////////////////////////////
class DifficultyMenuClass : public MenuDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	DifficultyMenuClass (void);

	//
	//	Inherited
	//
	void					On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void					On_Menu_Activate (bool onoff) override;
	void					On_Frame_Update (void) override;

	void					Set_Replay( const char * filename )	{ ReplayFilename = filename; }

private:
	StringClass				ReplayFilename;
	int						CurrSel;
};


////////////////////////////////////////////////////////////////
//
//	MultiplayOptionsMainMenuClass
//
////////////////////////////////////////////////////////////////
class MultiplayOptionsMainMenuClass : public MenuDialogClass
{
public:
	MultiplayOptionsMainMenuClass (void)	:
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_OPTION_MAIN_MULTIPLAY)) {}

	
	//
	//	Inherited
	//
	void			On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
};



////////////////////////////////////////////////////////////////
//
//	CampaignScoreTabClass
//
////////////////////////////////////////////////////////////////
class CampaignScoreTabClass : public ChildDialogClass
{
public:
	CampaignScoreTabClass (void)	:
		ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_SCORE_CAMPAIGN_TAB))	{}
};


////////////////////////////////////////////////////////////////
//
//	MissionsScoreTabClass
//
////////////////////////////////////////////////////////////////
class MissionsScoreTabClass : public ChildDialogClass
{
public:
	MissionsScoreTabClass (void)	:
		ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_SCORE_MISSIONS_TAB))	{}
};



////////////////////////////////////////////////////////////////
//
//	OptionsMenuClass
//
////////////////////////////////////////////////////////////////
class OptionsMenuClass : public MenuDialogClass
{
public:
	OptionsMenuClass (void)	:
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_OPTIONS))	{}
};


////////////////////////////////////////////////////////////////
//
//	QuitVerificationDialogClass
//
////////////////////////////////////////////////////////////////
class QuitVerificationDialogClass : public PopupDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	QuitVerificationDialogClass (void);
	~QuitVerificationDialogClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	static QuitVerificationDialogClass *		Get_Instance (void)	{ return _TheInstance; }

private:
	
	////////////////////////////////////////////////////////////////
	//	Static member data
	////////////////////////////////////////////////////////////////
	static QuitVerificationDialogClass *	_TheInstance;
};


////////////////////////////////////////////////////////////////
//
//	MPMainMenuClass
//
////////////////////////////////////////////////////////////////
class MPMainMenuClass : public MenuDialogClass
{
public:
	MPMainMenuClass (void)	:
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_MAIN_MULTIPLAY))	{}

	void	On_Init_Dialog (void) override;
};


////////////////////////////////////////////////////////////////
//
//	MPLanMenuClass
//
////////////////////////////////////////////////////////////////
class MPLanMenuClass : public MenuDialogClass
{
public:
	MPLanMenuClass (void)	:
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_CS_SEL))	{}

	void	On_Init_Dialog (void) override;
	void	On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void	On_Destroy (void) override;
};


////////////////////////////////////////////////////////////////
//
//	MPInternetCSMenuClass
//
////////////////////////////////////////////////////////////////
class MPInternetCSMenuClass : public MenuDialogClass
{
public:
	MPInternetCSMenuClass (void)	:
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_WWOL_CS_SEL))	{}
};


/*
////////////////////////////////////////////////////////////////
//
//	MPGameMenuClass
//
////////////////////////////////////////////////////////////////
class MPGameMenuClass : public MenuDialogClass
{
public:
	MPGameMenuClass (void)	:
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_GAME_TYPE))	{}

	void	On_Command (int ctrl_id, int mesage_id, unsigned int param);
};


////////////////////////////////////////////////////////////////
//
//	MPServerConfigClass
//
////////////////////////////////////////////////////////////////
class MPServerConfigClass : public MenuDialogClass
{
public:
	MPServerConfigClass (void)	:
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_SERVER_CONFIG))	{}

	void	On_Init_Dialog (void);
	void	On_Command (int ctrl_id, int mesage_id, unsigned int param);
};


////////////////////////////////////////////////////////////////
//
//	MPServerStartMenuClass
//
////////////////////////////////////////////////////////////////
class MPServerStartMenuClass : public MenuDialogClass
{
public:
	MPServerStartMenuClass (void)	:
		//IsServer (true),
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_START_GAME))	{}
			

	void	On_Init_Dialog (void);
	void	On_Command (int ctrl_id, int mesage_id, unsigned int param);
	void	Start_Game (void);

	//bool	IsServer;
};
*/


////////////////////////////////////////////////////////////////
//
//	MPJoinMenuClass
//
////////////////////////////////////////////////////////////////
class MPJoinMenuClass : public MenuDialogClass
{
public:
	MPJoinMenuClass (void)	:
		MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_JOIN_GAME))	{}

	void	On_Init_Dialog (void) override;
	void	On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void	Update_Game_List (void);
	void	On_Frame_Update (void) override;
};


////////////////////////////////////////////////////////////////
//
//	DeathOptionsPopupClass
//
////////////////////////////////////////////////////////////////
class DeathOptionsPopupClass : public PopupDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	DeathOptionsPopupClass (void)	:
		PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_DEATH_OPTIONS))	{}

	void	On_Init_Dialog (void) override;
	void	On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
};

////////////////////////////////////////////////////////////////
//
//	FailedOptionsPopupClass
//
////////////////////////////////////////////////////////////////
class FailedOptionsPopupClass : public PopupDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	FailedOptionsPopupClass (void)	:
		PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_FAILED_OPTIONS))	{}

	void	On_Init_Dialog (void) override;
	void	On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
};


////////////////////////////////////////////////////////////////
//
//	EditWheeledVehicleDialogClass
//
////////////////////////////////////////////////////////////////
class EditWheeledVehicleDialogClass : public PopupDialogClass
{
public:
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	EditWheeledVehicleDialogClass (WheeledVehicleDefClass * def,float wheel_radius);
	~EditWheeledVehicleDialogClass(void);

	void	On_Init_Dialog (void) override;
	void	On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

protected:

	WheeledVehicleDefClass *	VehicleDef;
	float								WheelRadius;
};

////////////////////////////////////////////////////////////////
//
//	EditTrackedVehicleDialogClass
//
////////////////////////////////////////////////////////////////
class EditTrackedVehicleDialogClass : public PopupDialogClass
{
public:
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	EditTrackedVehicleDialogClass (TrackedVehicleDefClass * def,float wheel_radius);
	~EditTrackedVehicleDialogClass(void);

	void	On_Init_Dialog (void) override;
	void	On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

protected:

	TrackedVehicleDefClass *	VehicleDef;
	float								WheelRadius;

};



#endif //__DIALOG_TESTS_H
