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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/widgetuser.cpp                        $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 10/11/01 2:29p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "widgetuser.h"
#include "widgets.h"
#include "mempool.h"
#include "vertmaterial.h"
#include "matinfo.h"
#include "boxrobj.h"
#include "ww3d.h"


/*
** WidgetRenderOpClass
** This class encapsulates the instructions from a physics object to render
** a debug widget.  Each physics object has a list of these hanging off it 
** for rendering vectors, points, etc.
** Since instances of this class are constantly allocated and deallocated
** each frame they are derived from AutoPoolClass.
*/
class WidgetRenderOpClass : public AutoPoolClass<WidgetRenderOpClass,256>
{
public:

	WidgetRenderOpClass(void);
	
	void	Set_Color(const Vector3 & color);
	void	Set_Opacity(float opacity);
	void	Init_Point(const Vector3 & point);
	void	Init_Vector(const Vector3 & point,const Vector3 & vector);
	void	Init_AABox(const AABoxClass & box);
	void	Init_OBBox(const OBBoxClass & box);
	void	Init_Coord_Axes(const Matrix3D & tm);
		
	void	Render(RenderInfoClass & rinfo);

	void	Set_Next(WidgetRenderOpClass * next)		{ Next = next; }
	WidgetRenderOpClass * Get_Next(void)				{ return Next; }

protected:

	void	render_point(RenderInfoClass & rinfo);
	void	render_vector(RenderInfoClass & rinfo);
	void	render_aabox(RenderInfoClass & rinfo);
	void	render_obbox(RenderInfoClass & rinfo);
	void	render_axes(RenderInfoClass & rinfo);

	enum { RENDER_NONE = 0, RENDER_POINT, RENDER_VECTOR, RENDER_AABOX, RENDER_OBBOX, RENDER_AXES };

	int								RenderOp;
	Vector3							Color;
	float								Opacity;

	Vector3							V0;				// these are all re-used depending on the render op
	Vector3							V1;
	Vector3							V2;
	Matrix3D							Transform;

	WidgetRenderOpClass *		Next;
};

/*
** declare the instance of the pool object for all WidgetRenderOpClass's
*/
DEFINE_AUTO_POOL(WidgetRenderOpClass,256);


/*******************************************
**
** WidgetRenderOpClass Implementation
**
*******************************************/
WidgetRenderOpClass::WidgetRenderOpClass(void) :
	RenderOp(RENDER_NONE),
	Next(NULL)
{
}

void WidgetRenderOpClass::Set_Color(const Vector3 & color)
{
	Color = color;
}

void WidgetRenderOpClass::Set_Opacity(float opacity)
{
	Opacity = opacity;
}

void WidgetRenderOpClass::Init_Point(const Vector3 & p0)
{
	RenderOp = RENDER_POINT;
	V0 = p0;
}

void WidgetRenderOpClass::Init_Vector(const Vector3 & point,const Vector3 & vector)
{
	RenderOp = RENDER_VECTOR;
	V0 = point;
	V1 = vector;
}

void WidgetRenderOpClass::Init_AABox(const AABoxClass & box)
{
	RenderOp = RENDER_AABOX;
	V0 = box.Center;
	V1 = box.Extent;
}

void WidgetRenderOpClass::Init_OBBox(const OBBoxClass & box)
{
	RenderOp = RENDER_OBBOX;
	V0 = Vector3(0,0,0);
	V1 = box.Extent;
	Transform.Set(box.Basis,box.Center);
}

void WidgetRenderOpClass::Init_Coord_Axes(const Matrix3D & tm)
{
	RenderOp = RENDER_AXES;
	Transform = tm;
}

void WidgetRenderOpClass::Render(RenderInfoClass & rinfo)
{
	switch(RenderOp)
	{
	case RENDER_NONE:
		break;
	case RENDER_POINT:
		render_point(rinfo);
		break;
	case RENDER_VECTOR:
		render_vector(rinfo);
		break;
	case RENDER_AABOX:
		render_aabox(rinfo);
		break;
	case RENDER_OBBOX:
		render_obbox(rinfo);
		break;
	case RENDER_AXES:
		render_axes(rinfo);
		break;
	};
}

void WidgetRenderOpClass::render_point([[maybe_unused]] RenderInfoClass & rinfo)
{
#ifdef WWDEBUG
	// Get the point model
	RenderObjClass * model = WidgetSystem::Get_Debug_Widget(WidgetSystem::WIDGET_POINT);
	if (model == NULL) return;
	if (model->Class_ID() != RenderObjClass::CLASSID_MESH) {
		model->Release_Ref();
		return;
	}

	// Set its color
	MaterialInfoClass * matinfo = model->Get_Material_Info();
	if (matinfo) {
		VertexMaterialClass * vmat = matinfo->Peek_Vertex_Material(0);
		if (vmat) {
			vmat->Set_Emissive(Color);
		}
		matinfo->Release_Ref();
	}

	// Render it
	model->Set_Transform(Matrix3D(V0));
	model->Render(rinfo);
	model->Release_Ref();
#endif
}

void WidgetRenderOpClass::render_vector([[maybe_unused]] RenderInfoClass & rinfo)
{
#ifdef WWDEBUG
	if (V1.Length2() < WWMATH_EPSILON) return;
	
	RenderObjClass * vecmodel = WidgetSystem::Get_Debug_Widget(WidgetSystem::WIDGET_VECTOR);
	if (vecmodel) {

		// set the color, the vector model is a skin so we have to get the
		// first sub-object, get its material info, then get the first vertex material
		RenderObjClass * subobj = vecmodel->Get_Sub_Object(0);
		if (subobj) {
			MaterialInfoClass * matinfo = subobj->Get_Material_Info();
			if (matinfo) {
				VertexMaterialClass * vmat = matinfo->Peek_Vertex_Material(0);
				if (vmat) {
					vmat->Set_Emissive(Color);
				}
				matinfo->Release_Ref();
			}
			subobj->Release_Ref();
		}

		// set the transform to be at p0, pointing -z at p1
		Matrix3D tm0;
		Matrix3D tm1(1);
		tm0.Look_At(V0,V0+V1,0.0f);	// point -z from pt along vec
		tm1.Translate(Vector3(0,0,1.0f - V1.Length()));	// p1 is along -z (Point1 starts at -1)
		vecmodel->Set_Transform(tm0);
	
		// set bone "Point1" to be at p1
		int p1index = vecmodel->Get_Bone_Index("Point1");
		vecmodel->Capture_Bone(p1index);
		vecmodel->Control_Bone(p1index,tm1);

		vecmodel->Render(rinfo);
		vecmodel->Release_Ref();
	}
#endif
}

void WidgetRenderOpClass::render_aabox([[maybe_unused]] RenderInfoClass & rinfo)
{
#ifdef WWDEBUG
	RenderObjClass * model = WidgetSystem::Get_Debug_Widget(WidgetSystem::WIDGET_AABOX);
	if (model == NULL) return;
	if (model->Class_ID() != RenderObjClass::CLASSID_AABOX) {
		model->Release_Ref();
		return;
	}

	// force this box to get rendered (bypass the built in collision bit masking)
	int oldmask = WW3D::Get_Collision_Box_Display_Mask();
	int newmask = oldmask | 0x01;
	WW3D::Set_Collision_Box_Display_Mask(newmask);

	AABoxRenderObjClass * boxmodel = (AABoxRenderObjClass *)model;
	boxmodel->Set_Local_Center_Extent(V0,V1);
	boxmodel->Set_Transform(Matrix3D(1));
	boxmodel->Set_Color(Color);	
	boxmodel->Set_Opacity(Opacity);
	boxmodel->Render(rinfo);
	boxmodel->Release_Ref();

	// restore the old mask
	WW3D::Set_Collision_Box_Display_Mask(oldmask);
#endif
}

void WidgetRenderOpClass::render_obbox([[maybe_unused]] RenderInfoClass & rinfo)
{
#ifdef WWDEBUG
	RenderObjClass * model = WidgetSystem::Get_Debug_Widget(WidgetSystem::WIDGET_OBBOX);
	if (model == NULL) return;
	if (model->Class_ID() != RenderObjClass::CLASSID_OBBOX) {
		model->Release_Ref();
		return;
	}

	// force this box to get rendered (bypass the built in collision bit masking)
	int oldmask = WW3D::Get_Collision_Box_Display_Mask();
	int newmask = oldmask | 0x01;
	WW3D::Set_Collision_Box_Display_Mask(newmask);

	OBBoxRenderObjClass * boxmodel = (OBBoxRenderObjClass *)model;
	boxmodel->Set_Collision_Type(0xFF);
	boxmodel->Set_Transform(Transform);
	boxmodel->Set_Local_Center_Extent(V0,V1);
	boxmodel->Set_Color(Color);	
	boxmodel->Set_Opacity(Opacity);
	boxmodel->Render(rinfo);
	boxmodel->Release_Ref();

	// restore the old mask
	WW3D::Set_Collision_Box_Display_Mask(oldmask);
#endif
}

void WidgetRenderOpClass::render_axes([[maybe_unused]] RenderInfoClass & rinfo)
{
#ifdef WWDEBUG
	RenderObjClass * model = WidgetSystem::Get_Debug_Widget(WidgetSystem::WIDGET_AXES);
	if (model == NULL) return;
	if (model->Class_ID() != RenderObjClass::CLASSID_MESH) {
		model->Release_Ref();
		return;
	}

	model->Set_Transform(Transform);
	model->Render(rinfo);
	model->Release_Ref();
#endif
}


/*******************************************
**
** WidgetUserClass Implementation
**
*******************************************/
WidgetUserClass::WidgetUserClass(void)
{
#ifdef WWDEBUG
	WidgetRenderOpList = NULL;
#endif
}

WidgetUserClass::~WidgetUserClass(void)
{
	Reset_Debug_Widget_List();
}

#ifdef WWDEBUG
void WidgetUserClass::Reset_Debug_Widget_List(void)
{
	WidgetRenderOpClass * op = WidgetRenderOpList;
	while (op) {
		WidgetRenderOpClass * nextop = op->Get_Next();
		delete op;
		op = nextop;
	}
	WidgetRenderOpList = NULL;
}
#endif

#ifdef WWDEBUG
void WidgetUserClass::Add_Debug_Widget(WidgetRenderOpClass * op)
{
	WWASSERT(op->Get_Next() == NULL);
	op->Set_Next(WidgetRenderOpList);
	WidgetRenderOpList = op;
}
#endif

#ifdef WWDEBUG
void WidgetUserClass::Add_Debug_Point(const Vector3 & p,const Vector3 & color)
{
	WidgetRenderOpClass * op = new WidgetRenderOpClass;
	op->Set_Color(color);
	op->Init_Point(p);
	Add_Debug_Widget(op);
}
#endif

#ifdef WWDEBUG
void WidgetUserClass::Add_Debug_Vector(const Vector3 & p,const Vector3 & v,const Vector3 & color)
{
	if (v.Length2() > 0.0f) {
		WidgetRenderOpClass * op = new WidgetRenderOpClass;
		op->Set_Color(color);
		op->Init_Vector(p,v);
		Add_Debug_Widget(op);
	}
}
#endif

#ifdef WWDEBUG
void WidgetUserClass::Add_Debug_AABox(const AABoxClass & box,const Vector3 & color,float opacity)
{
	WidgetRenderOpClass * op = new WidgetRenderOpClass;
	op->Set_Color(color);
	op->Set_Opacity(opacity);
	op->Init_AABox(box);
	Add_Debug_Widget(op);
}
#endif

#ifdef WWDEBUG
void WidgetUserClass::Add_Debug_OBBox(const OBBoxClass & box,const Vector3 & color,float opacity)
{
	WidgetRenderOpClass * op = new WidgetRenderOpClass;
	op->Set_Color(color);
	op->Set_Opacity(opacity);
	op->Init_OBBox(box);
	Add_Debug_Widget(op);
}
#endif

#ifdef WWDEBUG
void WidgetUserClass::Add_Debug_Axes(const Matrix3D & transform,const Vector3 & color)
{
	WidgetRenderOpClass * op = new WidgetRenderOpClass;
	op->Set_Color(color);
	op->Init_Coord_Axes(transform);
	Add_Debug_Widget(op);
}
#endif

#ifdef WWDEBUG
void WidgetUserClass::Render_Debug_Widgets(RenderInfoClass & rinfo)
{
	WW3D::Flush(rinfo);

	WidgetRenderOpClass * op = WidgetRenderOpList;
	while (op) {
		op->Render(rinfo);
		op = op->Get_Next();
		WW3D::Flush(rinfo);
	}
}
#endif



