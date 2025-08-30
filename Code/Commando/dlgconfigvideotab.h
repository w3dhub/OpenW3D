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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgconfigvideotab.h                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/21/01 5:52p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCONFIGVIDEOTAB_H
#define __DLGCONFIGVIDEOTAB_H


// Includes.
#include "childdialog.h"
#include "dx8wrapper.h"


// Defines.
#define GAMMA_SLIDER_MIN				 60
#define GAMMA_SLIDER_DEFAULT			 90
#define GAMMA_SLIDER_MAX				210
#define BRIGHTNESS_SLIDER_MIN		   -45
#define BRIGHTNESS_SLIDER_DEFAULT	  0
#define BRIGHTNESS_SLIDER_MAX			 45
#define CONTRAST_SLIDER_MIN			 50
#define CONTRAST_SLIDER_DEFAULT		130
#define CONTRAST_SLIDER_MAX			200


//////////////////////////////////////////////////////////////////////
//
//	DlgConfigVideoTabClass
//
//////////////////////////////////////////////////////////////////////
class DlgConfigVideoTabClass : public ChildDialogClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	DlgConfigVideoTabClass  (void);
	~DlgConfigVideoTabClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	
	//
	//	Inherited
	//
	void On_Init_Dialog (void) override;
	void On_Destroy (void) override;
	void On_SliderCtrl_Pos_Changed (SliderCtrlClass *slider_ctrl, int ctrl_id, int new_pos) override;

	//
	//	Gamma adjustments
	//
	static int	Get_Gamma (void)					{ return GammaLevel; }
	static void	Set_Gamma (int level)			{ GammaLevel = level; Update_Gamma();}
	static int	Get_Brightness (void)			{ return BrightnessLevel; }
	static void	Set_Brightness (int level)		{ BrightnessLevel = level; Update_Gamma();}
	static int	Get_Contrast (void)				{ return ContrastLevel; }
	static void	Set_Contrast (int level)		{ ContrastLevel = level; Update_Gamma();}

protected:
	
	bool UpdateGamma;

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////	
	static float Gamma_Scale (int level);
	static void  Update_Gamma();
	static void	 Update_Gamma (int g, int b, int c);
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	static int GammaLevel;
	static int BrightnessLevel;
	static int ContrastLevel;
};


inline float DlgConfigVideoTabClass::Gamma_Scale (int level)
{
	return (level * 0.01f);
}


inline void	DlgConfigVideoTabClass::Update_Gamma()
{
	DX8Wrapper::Set_Gamma (Gamma_Scale (GammaLevel), Gamma_Scale (BrightnessLevel), Gamma_Scale (ContrastLevel), true, false);
}


inline void	DlgConfigVideoTabClass::Update_Gamma (int g, int b, int c)
{
	DX8Wrapper::Set_Gamma (Gamma_Scale (g), Gamma_Scale (b), Gamma_Scale (c), true, false);
}


#endif //__DLGCONFIGVIDEOTAB_H
