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
 *                     $Archive:: /Commando/Code/Commando/sctextobj.h                           $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 11/30/01 12:52p                                             $* 
 *                                                                                             * 
 *                    $Revision:: 5                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __SCTEXTOBJ_H__
#define __SCTEXTOBJ_H__

#include "netevent.h"
#include "netclassids.h"

enum TextMessageEnum
{
	TEXT_MESSAGE_PUBLIC,
	TEXT_MESSAGE_TEAM,
	TEXT_MESSAGE_PRIVATE,
};

enum 
{
	HOST_TEXT_SENDER = -1
};

//-----------------------------------------------------------------------------
//
// A S->C mirrored object for ingame text communications.
//
class	cScTextObj : public cNetEvent
{
public:
   cScTextObj(void);

	void						Init(const WideStringClass & text, TextMessageEnum type, bool is_host_admin_message, int sender_id, int recipient_id = -1);

	virtual void			Export_Creation(BitStreamClass &packet) override;
	virtual void			Import_Creation(BitStreamClass &packet) override;
	virtual uint32			Get_Network_Class_ID(void) const override				{return NETCLASSID_SCTEXTOBJ;}

private:

	virtual void			Act(void) override;
	void						Set_Dirty_Bit_For_Team(DIRTY_BIT bit, int team);

	int						SenderId;
	int						RecipientId;
	TextMessageEnum		Type;
	WideStringClass		Text;
	bool						IsHostAdminMessage;
};

//-----------------------------------------------------------------------------

#endif	// __SCTEXTOBJ_H__
