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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgconfigaudiotab.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/11/02 5:22p                                                $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCONFIGAUDIOTAB_H
#define __DLGCONFIGAUDIOTAB_H


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class AudibleSoundClass;


#include "childdialog.h"


//////////////////////////////////////////////////////////////////////
//
//	DlgConfigAudioTabClass
//
//////////////////////////////////////////////////////////////////////
class DlgConfigAudioTabClass : public ChildDialogClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	DlgConfigAudioTabClass  (void);
	~DlgConfigAudioTabClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Command (int ctrl_id, int message_id, DWORD param) override;
	bool		On_Apply (void) override;
	void		On_Frame_Update (void) override;
	void		On_SliderCtrl_Pos_Changed (SliderCtrlClass *slider_ctrl, int ctrl_id, int new_pos) override;
	
protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	void		Update_Slider_Enable_State (void);
	void		Configure_Quality_Combobox (void);
	void		Configure_Rate_Combobox (void);
	void		Configure_Speaker_Combobox (void);
	void		Configure_Driver_List (void);
	void		Set_Default_Volumes (void);

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	int		InitialDeviceIndex;
	int		InitialHertz;
	int		InitialBits;
	bool		InitialIsStereo;

	AudibleSoundClass *	SoundVolumeTestSound;
	AudibleSoundClass *	MusicVolumeTestSound;
	AudibleSoundClass *	DialogVolumeTestSound;
	AudibleSoundClass *	CinematicVolumeTestSound;

	int						SoundVolumeTestSoundStartTime;
	int						MusicVolumeTestSoundStartTime;
	int						DialogVolumeTestSoundStartTime;
	int						CinematicVolumeTestSoundStartTime;
};


#endif //__DLGCONFIGAUDIOTAB_H
