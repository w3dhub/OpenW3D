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

/**********************************************************************
 *<
	FILE: FPMatNav.cpp

	DESCRIPTION:	Function published access for material navigator messages

	CREATED BY:		Michael Russo

	HISTORY:		02-08-2002	Created

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#if defined W3D_GMAXDEV

#include "Max.h"
#include "GMaxMtlDlg.h"
#include "GameMtlForm.h"
#include "GameMtl.h"
#include "resource.h"

extern GMaxMtlDlg		* GMaxMaterialDialog;
extern GameMtlActionCB* Game_Mtl_ActionCB;


//============================================================================================
FPMatNav fpMatNav(FPMATNAV_INTERFACE, _T("MaterialNavNotify"), 0, NULL, FP_CORE,
	IFPMatNav::idLaunch, _T("Launch"), 0, TYPE_VOID, 0, 0,
	IFPMatNav::idSetMultiMaterialTabBySlot, _T("SetMultiMaterialTabBySlot"), 0, TYPE_VOID, 0, 3,
		_T("slot"), 0, TYPE_INT,
		_T("SubMtlTexIndex"), 0, TYPE_INT,
		_T("Element"), 0, TYPE_INT,
	IFPMatNav::idSetMaterialTabBySlot, _T("SetMaterialTabBySlot"), 0, TYPE_VOID, 0, 2,
		_T("slot"), 0, TYPE_INT,
		_T("Element"), 0, TYPE_INT,
	IFPMatNav::idSetMultiMaterialBySlot, _T("SetMultiMaterialBySlot"), 0, TYPE_VOID, 0, 2,
		_T("slot"), 0, TYPE_INT,
		_T("Element"), 0, TYPE_INT,
	IFPMatNav::idSetMaterialBySlot, _T("SetMaterialBySlot"), 0, TYPE_VOID, 0, 1,
		_T("slot"), 0, TYPE_INT,
end
);

IFPMatNav *FPMatNav::m_pIFPMatNavCallback = NULL;

//============================================================================================
void FPMatNav::Launch(){
	if( m_pIFPMatNavCallback ) {
		m_pIFPMatNavCallback->Launch();
	}
}
//============================================================================================
void FPMatNav::SetMultiMaterialTabBySlot( int iSlot, int iSubMtlTexIndex, int iElement ){
	if( m_pIFPMatNavCallback ) {
		m_pIFPMatNavCallback->SetMultiMaterialTabBySlot(iSlot, iSubMtlTexIndex, iElement);
	}
}
//============================================================================================
void FPMatNav::SetMaterialTabBySlot( int iSlot, int iElement ){
	if( m_pIFPMatNavCallback ) {
		m_pIFPMatNavCallback->SetMaterialTabBySlot(iSlot, iElement);
	}
}
//============================================================================================
void FPMatNav::SetMultiMaterialBySlot( int iSlot, int iIndex ){
	if( m_pIFPMatNavCallback ) {
		m_pIFPMatNavCallback->SetMultiMaterialBySlot(iSlot, iIndex);
	}
}
//============================================================================================
void FPMatNav::SetMaterialBySlot(int iSlot ){
	if( m_pIFPMatNavCallback ) {
		m_pIFPMatNavCallback->SetMaterialBySlot(iSlot);
	}else{
		if(NULL == GMaxMaterialDialog){
			Game_Mtl_ActionCB->ExecuteAction(IDA_GAMEMTL_DODLG);
		}
		//Retry
		if( NULL != GMaxMaterialDialog && m_pIFPMatNavCallback ) {
			m_pIFPMatNavCallback->SetMaterialBySlot(iSlot);
		}
	}
}
#endif

