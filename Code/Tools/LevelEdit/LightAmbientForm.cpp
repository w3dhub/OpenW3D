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

// LightAmbientFormClass.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "LightAmbientForm.H"
#include "vector3.h"
#include "Utils.h"
#include "SceneEditor.H"
#include "rendobj.h"
#include "phys.h"
#include "colorpickerdialogclass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////////
void ColorUpdateCallback (int red, int green, int blue, void *arg);


/////////////////////////////////////////////////////////////////////////////
//
// LightAmbientFormClass
//
/////////////////////////////////////////////////////////////////////////////
LightAmbientFormClass::LightAmbientFormClass()
	:	m_ColorForm (NULL),
		DockableFormClass(LightAmbientFormClass::IDD)
{
	//{{AFX_DATA_INIT(LightAmbientFormClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~LightAmbientFormClass
//
LightAmbientFormClass::~LightAmbientFormClass()
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
LightAmbientFormClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LightAmbientFormClass)
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(LightAmbientFormClass, DockableFormClass)
	//{{AFX_MSG_MAP(LightAmbientFormClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LightAmbientFormClass diagnostics

#ifdef _DEBUG
void LightAmbientFormClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void LightAmbientFormClass::Dump(CDumpContext& dc) const
{
	DockableFormClass::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
/////////////////////////////////////////////////////////////////////////////
void
LightAmbientFormClass::HandleInitDialog (void)
{	
	//
	//	Create the color picker
	//
	m_ColorForm = Create_Color_Picker_Form (m_hWnd, 0, 0, 0);
	Set_Update_Callback (m_ColorForm, ColorUpdateCallback);
	::ShowWindow (m_ColorForm, SW_SHOW);

	Update_Settings ();
	return ;
}


//////////////////////////////////////////////////////////////
//
//  Update_Settings
//
//////////////////////////////////////////////////////////////
void
LightAmbientFormClass::Update_Settings (void)
{
	//
	// Get the current ambient light settings
	//
	Vector3 light_settings = ::Get_Scene_Editor()->Get_Ambient_Light ();

	int red		= light_settings.X * 255;
	int green	= light_settings.Y * 255;
	int blue		= light_settings.Z * 255;

	//
	//	Pass these colors onto the form
	//
	Set_Form_Original_Color (m_ColorForm, red, green, blue);
	Set_Form_Color (m_ColorForm, red, green, blue);

	::Get_Scene_Editor ()->Update_Lighting ();
	return ;
}


//////////////////////////////////////////////////////////////
//
//  ColorUpdateCallback
//
//////////////////////////////////////////////////////////////
void
ColorUpdateCallback (int red, int green, int blue, void * /* arg */)
{
	//
	//	Convert the settings to a vector3
	//
	Vector3 light;
	light.X = ((float)red) / 255;
	light.Y = ((float)green) / 255;
	light.Z = ((float)blue) / 255;

	//
	//	Update the scene's abmient light
	//
	::Get_Scene_Editor()->Set_Ambient_Light (light);
	::Get_Scene_Editor()->Update_Lighting ();
	::Refresh_Main_View ();
	return ;
}
