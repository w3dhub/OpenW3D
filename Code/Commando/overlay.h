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
 *                     $Archive:: /Commando/Code/Commando/overlay.h                           $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 5/14/01 4:19p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 4                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	OVERLAY_H
#define	OVERLAY_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	GAMEMODE_H
	#include "gamemode.h"
#endif

class	CameraClass;
class	SimpleSceneClass;
class	RenderObjClass;

#define		OVERLAY_SCENE		OverlayGameModeClass::Scene		
#define		OVERLAY_CAMERA		OverlayGameModeClass::Camera

/*
** Game Mode to display screen images
*/
class	OverlayGameModeClass : public GameModeClass {
public:
	virtual	const char *Name() override	{ return "Overlay"; }	// the name of this mode
	virtual	void	Init() override;		 	// called when the mode is activated
	virtual	void 	Shutdown() override; 	// called when the mode is deactivated
	virtual	void 	Think() override {}		// called each time through the main loop
	virtual	void 	Render() override;		// called each time through the main loop

	static	SimpleSceneClass  * Scene;
	static	CameraClass			* Camera;
};


/*
** Game Mode to display W3D overlay files
*/
class	Overlay3DGameModeClass : public GameModeClass {
public:
	virtual	const char *Name() override	{ return "Overlay3D"; }	// the name of this mode
	virtual	void	Init() override;		 	// called when the mode is activated
	virtual	void 	Shutdown() override; 	// called when the mode is deactivated
	virtual	void 	Think() override;			// called each time through the main loop
	virtual	void 	Render() override;		// called each time through the main loop

	void 		Start_Intro( void );
	void 		Start_End_Screen( void );

private:

	void 	Release_Model( void );
	void 	Set_Model( const char * name, const char * anim_name );

	static	SimpleSceneClass  * Scene;
	static	CameraClass			* Camera;
	static	RenderObjClass		* Model;
	static	float					  ModelTimer;
	static	bool					  EndScreen;
};

#endif
