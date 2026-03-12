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
 *                     $Archive:: /Commando/Code/Combat/assets.h                              $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/31/01 8:03p                                               $*
 *                                                                                             *
 *                    $Revision:: 31                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	ASSETS_H
#define	ASSETS_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef ASSETMGR_H
	#include "assetmgr.h"
#endif

#ifndef INI_H
	#include "ini.h"
#endif

/*
** INI File Access
*/
void			Set_INI_Path( const char * path );
INIClass *	Get_INI( const char * filename );
void			Save_INI( INIClass * p_ini, const char * filename );
void			Release_INI( INIClass * ini );

/*
** Path Striping
*/
void Strip_Path_From_Filename( StringClass& new_name, const char * filename );
void Get_Render_Obj_Name_From_Filename( StringClass& new_name, const char * filename );

/*
** Asset access - these functions strips the path off of the filenames; for use
** with filenames that come from preset definitions.
*/
RenderObjClass * Create_Render_Obj_From_Filename( const char * filename );
TextureClass * Get_Texture_From_Filename(	const char * filename,
														TextureClass::MipCountType mip_level_count=TextureClass::MIP_LEVELS_ALL );

/*
** Filenames
*/
void Create_Animation_Name( StringClass& anim_name, const char * anim_filename, const char * model_name );

#endif	// ASSETS_H

