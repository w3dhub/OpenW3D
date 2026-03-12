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
 *                 Project Name : ENBAssetMgr                                                  *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/ww3d2/hueshift/enbassetmgr.h                      $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:35p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef ENBASSETMGR_H
#define ENBASSETMGR_H

#include <assetmgr.h>

class Vector3;
class VertexMaterialClass;

class ENBAssetManager: public WW3DAssetManager
{
public:
	ENBAssetManager(void);
	virtual ~ENBAssetManager(void);

	// unique to ENBAssetManager
	virtual RenderObjClass * Create_Render_Obj(const char * name,float Scale,Vector3 &HSV_shift);

private:
	void Make_Mesh_Unique(RenderObjClass *robj,bool geometry, bool colors);
	void Make_HLOD_Unique(RenderObjClass *robj,bool geometry, bool colors);
	void Make_Unique(RenderObjClass *robj,bool geometry, bool colors);
	void Recolor_Vertex_Material(VertexMaterialClass *vmat,Vector3 &hsv_shift);
	void Recolor_Vertices(unsigned int *color, int count, Vector3 &hsv_shift);
	void Recolor_Mesh(RenderObjClass *robj,Vector3 &hsv_shift);
	TextureClass * Recolor_Texture(TextureClass *texture, Vector3 &hsv_shift);
	void Recolor_HLOD(RenderObjClass *robj,Vector3 &hsv_shift);
	void Recolor_ParticleEmitter(RenderObjClass *robj,Vector3 &hsv_shift);
	void Recolor_Asset(RenderObjClass *robj,Vector3 &hsv_shift);
};

#endif

