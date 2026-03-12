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
	FILE: FPMatNav.h

	DESCRIPTION:	Function published access for material navigator messages

	CREATED BY:		Michael Russo

	HISTORY:		02-08-2002	Created

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#if !defined __FPMATNAV__
#define __FPMATNAV__


#include "max.h"
#include "iFnPub.h"

#define FPMATNAV_INTERFACE    Interface_ID(0x60151ad4, 0x3b325af3)

inline class IFPMatNav* GetFPMatNav() {
	return (IFPMatNav*)GetCOREInterface(FPMATNAV_INTERFACE);
}


#define MATNAV_ELEMENT_UNKNOWN		-1
#define MATNAV_ELEMENT_AMBIENT		0
#define MATNAV_ELEMENT_DIFFUSE		1
#define MATNAV_ELEMENT_COLOR		2
#define MATNAV_ELEMENT_LEVEL		3
#define MATNAV_ELEMENT_GLOSSINESS	4
#define MATNAV_ELEMENT_SELFILLUM	5
#define MATNAV_ELEMENT_BUMP			6

///////////////////////////////////////////////////////////////////////////////
//
// IFPMatNav  fSetMultiMaterialBySlot
//
///////////////////////////////////////////////////////////////////////////////
class IFPMatNav
{
public:
	virtual void Launch(void) = 0;
	virtual void SetMultiMaterialTabBySlot( int iSlot, int iSubMtlTexIndex, int iElement ) = 0;
	virtual void SetMaterialTabBySlot( int iSlot, int iElement ) = 0;
	virtual void SetMultiMaterialBySlot( int iSlot, int iIndex ) = 0;
	virtual void SetMaterialBySlot( int iSlot ) = 0;

	// function IDs
	enum {	idLaunch, idSetMultiMaterialTabBySlot,
			idSetMaterialTabBySlot, idSetMultiMaterialBySlot,
			idSetMaterialBySlot	};
};

class FPMatNav : public IFPMatNav, public FPStaticInterface
{
public:
	//
	// Setup Callback
	//
	void SetIFPMatNavCallback( IFPMatNav *pIFPMatNav ) { m_pIFPMatNavCallback = pIFPMatNav; };

	//
	// IFPMatNav Methods
	//
	void Launch(void);
	void SetMultiMaterialTabBySlot( int iSlot, int iSubMtlTexIndex, int iElement );
	void SetMaterialTabBySlot( int iSlot, int iElement );
	void SetMultiMaterialBySlot( int iSlot, int iIndex );
	void SetMaterialBySlot( int iSlot );

	DECLARE_DESCRIPTOR(FPMatNav)
	BEGIN_FUNCTION_MAP
		VFN_0(IFPMatNav::idLaunch, Launch);
		VFN_3(IFPMatNav::idSetMultiMaterialTabBySlot, SetMultiMaterialTabBySlot, TYPE_INT, TYPE_INT, TYPE_INT);
		VFN_2(IFPMatNav::idSetMaterialTabBySlot, SetMaterialTabBySlot, TYPE_INT, TYPE_INT);
		VFN_2(IFPMatNav::idSetMultiMaterialBySlot, SetMultiMaterialBySlot, TYPE_INT, TYPE_INT);
		VFN_1(IFPMatNav::idSetMaterialBySlot, SetMaterialBySlot, TYPE_INT);
	END_FUNCTION_MAP

public:
	static IFPMatNav *m_pIFPMatNavCallback;

};

extern FPMatNav fpMatNav;

#endif

