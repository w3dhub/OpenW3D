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
 *                     $Archive:: /Commando/Code/Commando/serverfps.h                      $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/15/02 7:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __SERVERFPS_H__
#define __SERVERFPS_H__

#include "networkobject.h"

//-----------------------------------------------------------------------------
//
// A S->C mirrored object to inform clients of server fps.
// This singleton object may not be statically allocated.
//
class	cServerFps : public NetworkObjectClass
{
public:

	void						Set_Fps(int fps);
	int						Get_Fps(void)								{return Fps;}

	void						Delete (void) override								{}
	virtual void			Export_Frequent(BitStreamClass &packet) override;
	virtual void			Import_Frequent(BitStreamClass &packet) override;
	virtual void			Set_Delete_Pending (void) override {};		// Never needs deletion since it persists during a game session.

	//
	// Static methods for the global singleton instance
	//
	static void				Create_Instance(void);
	static void				Destroy_Instance(void);
	static cServerFps *	Get_Instance(void);


private:

	//
	// Constructor and destructor are private - static functions are used to
	// manage the global singleton instance.
	//
	cServerFps(void);
	~cServerFps(void) {}

	int						Fps;

	//
	// The global singleton instance
	//
	static cServerFps *	TheInstance;
};

//-----------------------------------------------------------------------------

#endif	// __SERVERFPS_H__

