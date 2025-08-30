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
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/clientcontrol.h                        $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 9/17/01 4:18p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __CLIENTCONTROL_H__
#define __CLIENTCONTROL_H__

#include "networkobject.h"
#include "netclassids.h"
#include "control.h"

//-----------------------------------------------------------------------------
//
// A C-S mirrored object to represent client control and targeting data
//
class	CClientControl : public NetworkObjectClass
{
public:

	CClientControl();
	~CClientControl();

	void					Init(void);
	virtual uint32		Get_Network_Class_ID(void) const override					{return NETCLASSID_CLIENTCONTROL;}
	virtual void		Delete(void) override											{delete this;}

	void					Set_Update_Flag(int id);

	virtual void		Export_Creation(BitStreamClass &packet) override;
	virtual void		Import_Creation(BitStreamClass &packet) override;

	virtual void		Export_Frequent(BitStreamClass &packet) override;
	virtual void		Import_Frequent(BitStreamClass &packet) override;

private:

	int					ClientId;
	int					SmartObjId;
};

//-----------------------------------------------------------------------------

extern CClientControl *		PClientControl;

//-----------------------------------------------------------------------------

#endif	// __CLIENTCONTROL_H__
