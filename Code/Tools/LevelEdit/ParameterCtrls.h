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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ParameterCtrls.h             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/22/01 3:14p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PARAMETER_CTRLS_H
#define __PARAMETER_CTRLS_H

#include "presetpicker.h"
#include "filepicker.h"
#include "presetpicker.h"
#include "stringpicker.h"
#include "parameter.h"
#include "simpleparameter.h"
#include "ntree.h"
#include "bittype.h"
#include "definitionclassids.h"
#include "icons.h"
#include "combatchunkid.h"


///////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////
class PresetClass;
class SpecSheetClass;


///////////////////////////////////////////////////////////////////////
//
//	ParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	ParameterCtrlClass (void)
		:	m_IsTemp (false),
			m_IsReadOnly (false)				{ }
	virtual ~ParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	virtual ParameterClass::Type	Get_Type (void) const = 0;
	virtual int							Create (HWND parent_wnd, int id_start, LPRECT pos) = 0;
	virtual void						Resize (const CRect &rect) = 0;
	virtual void						Read_Data (HWND /* parent_wnd */) { };
	virtual bool						On_Command (HWND /* parent_wnd */, WPARAM /* wparam */, LPARAM /* lparam */) { return false; }
	virtual bool						On_DrawItem (HWND /* parent_wnd */, WPARAM /* wparam */, LPARAM /* lparam */) { return false; }
	virtual void						On_Destroy (void) { };

	virtual ParameterClass *		Get_Parameter (void) const = 0;

	// Accessors
	bool									Is_Temp (void) const			{ return m_IsTemp; }
	void									Set_Is_Temp (bool is_temp)	{ m_IsTemp = is_temp; }

	bool									Is_Read_Only (void) const	{ return m_IsReadOnly; }
	virtual void						Set_Read_Only (bool onoff)	{ m_IsReadOnly = onoff; }

	virtual bool						Is_Modified (void) const	{ return true; }

	virtual void						Set_Spec_Sheet (SpecSheetClass *sheet)	{ m_SpecSheet = sheet; }
	virtual SpecSheetClass *		Get_Spec_Sheet (void) const				{ return m_SpecSheet; }

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////////
	bool					m_IsTemp;
	bool					m_IsReadOnly;	
	
	SpecSheetClass *	m_SpecSheet;
};


///////////////////////////////////////////////////////////////////////
//
//	StringParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class StringParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	StringParameterCtrlClass (StringParameterClass *param)
		:	m_Parameter (param)				{ }
	virtual ~StringParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_STRING; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);	

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_StaticText;
	CEdit							m_EditCtrl;
	StringParameterClass *	m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	FileParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class FileParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	FileParameterCtrlClass (FilenameParameterClass *param)
		:	m_Parameter (param)				{ }
	virtual ~FileParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_FILENAME; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };

	void							Get_Current_Value (CString &value) const;
	bool							Is_Modified (void) const;

	FilePickerClass &			Get_File_Picker (void) { return m_FilePicker; }
	
protected:

	////////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////////
	CStatic							m_StaticText;
	FilePickerClass				m_FilePicker;
	FilenameParameterClass *	m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	StringsDBEntryParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class StringsDBEntryParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	StringsDBEntryParameterCtrlClass (StringsDBEntryParameterClass *param)
		:	m_Parameter (param)				{ }
	virtual ~StringsDBEntryParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_STRINGSDB_ID; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////////
	CStatic									m_StaticText;
	StringPickerClass						m_StringPicker;
	StringsDBEntryParameterClass *	m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	SoundFileParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class SoundFileParameterCtrlClass : public FileParameterCtrlClass 
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	SoundFileParameterCtrlClass (SoundFilenameParameterClass *param)
		:	FileParameterCtrlClass (param)			{ }
	virtual ~SoundFileParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_SOUND_FILENAME; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);	
};


///////////////////////////////////////////////////////////////////////
//
//	IntParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class IntParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	IntParameterCtrlClass (IntParameterClass *param)
		:	m_Parameter (param)				{ }
	virtual ~IntParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_INT; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_StaticText1;
	CStatic						m_StaticText2;
	CEdit							m_EditCtrl;
	CSpinButtonCtrl			m_SpinCtrl;
	IntParameterClass *		m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	FloatParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class FloatParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	FloatParameterCtrlClass (FloatParameterClass *param)
		:	m_Parameter (param)				{ }
	virtual ~FloatParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_FLOAT; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_StaticText1;
	CStatic						m_StaticText2;
	CEdit							m_EditCtrl;
	CSpinButtonCtrl			m_SpinCtrl;
	FloatParameterClass *	m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	AngleParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class AngleParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	AngleParameterCtrlClass (AngleParameterClass *param)
		:	m_Parameter (param)						{ }
	virtual ~AngleParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_ANGLE; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_StaticText1;
	CStatic						m_StaticText2;
	CEdit							m_EditCtrl;
	CSpinButtonCtrl			m_SpinCtrl;
	AngleParameterClass *	m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	BoolParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class BoolParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	BoolParameterCtrlClass (BoolParameterClass *param)
		:	m_Parameter (param)				{ }
	virtual ~BoolParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_BOOL; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CButton						m_CheckBoxCtrl;
	BoolParameterClass *		m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	Vector3ParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class Vector3ParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	Vector3ParameterCtrlClass (Vector3ParameterClass *param)
		:	m_Parameter (param)				{ }
	virtual ~Vector3ParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_VECTOR3; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_TextCtrls[3];
	CEdit							m_EditCtrls[3];
	CSpinButtonCtrl			m_SpinCtrls[3];
	CButton						m_GroupBox;
	Vector3ParameterClass *	m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	Vector2ParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class Vector2ParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	Vector2ParameterCtrlClass (Vector2ParameterClass *param)
		:	m_Parameter (param)				{ }
	virtual ~Vector2ParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_VECTOR2; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_TextCtrls[2];
	CEdit							m_EditCtrls[2];
	CSpinButtonCtrl			m_SpinCtrls[2];
	CButton						m_GroupBox;
	Vector2ParameterClass *	m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	RectParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class RectParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	RectParameterCtrlClass (RectParameterClass *param)
		:	m_Parameter (param)					{}
	virtual ~RectParameterCtrlClass (void)	{}

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_RECT; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_TextCtrls[4];
	CEdit							m_EditCtrls[4];
	CSpinButtonCtrl			m_SpinCtrls[4];
	CButton						m_GroupBox;
	RectParameterClass *		m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	EnumParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class EnumParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	EnumParameterCtrlClass (EnumParameterClass *param)
		:	m_Parameter (param)					{ }
	virtual ~EnumParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_ENUM; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_StaticText;
	CComboBox					m_ComboBox;
	EnumParameterClass *		m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	ColorParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class ColorParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	ColorParameterCtrlClass (ColorParameterClass *param)
		:	m_Parameter (param),
			m_ButtonID (0)								{ }
	virtual ~ColorParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_COLOR; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);
	bool							On_Command (HWND parent_wnd, WPARAM wparam, LPARAM lparam);
	bool							On_DrawItem (HWND parent_wnd, WPARAM wparam, LPARAM lparam);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_StaticText;
	CButton						m_Button;
	ColorParameterClass *	m_Parameter;
	int							m_ButtonID;
};


///////////////////////////////////////////////////////////////////////
//
//	PhysDefParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class PhysDefParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	PhysDefParameterCtrlClass (PhysDefParameterClass *param)
		:	m_Parameter (param),
			m_ButtonID (0)								{ }
	virtual ~PhysDefParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_PHYSDEFINITIONID; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);
	bool							On_Command (HWND parent_wnd, WPARAM wparam, LPARAM lparam);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_StaticText;
	CButton						m_Button;
	PhysDefParameterClass *	m_Parameter;
	int							m_ButtonID;
};


///////////////////////////////////////////////////////////////////////
//
//	DefParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class DefParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	DefParameterCtrlClass (DefParameterClass *param)
		:	m_Parameter (param),
			m_ButtonID (0)							{ }
	virtual ~DefParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							On_Destroy (void);
	void							Read_Data (HWND parent_wnd);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	virtual uint32				Get_Def_Class_ID (void) = 0;
	virtual int					Get_Def_Icon (void) = 0;
			
	////////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////////
	CStatic						m_StaticText;
	PresetPickerClass			m_PresetPicker;
	CComboBoxEx					m_ComboBox;
	int							m_ButtonID;
	DefParameterClass *		m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	GenericDefParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class GenericDefParameterCtrlClass : public DefParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	GenericDefParameterCtrlClass (GenericDefParameterClass *param)
		:	DefParameterCtrlClass (param)				{ }
	virtual ~GenericDefParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const	{ return ParameterClass::TYPE_GENERICDEFINITIONID; }

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	virtual uint32				Get_Def_Class_ID (void) { return ((GenericDefParameterClass *)m_Parameter)->Get_Class_ID (); }
	virtual int					Get_Def_Icon (void)		{ return OBJECT_ICON; }		
};


///////////////////////////////////////////////////////////////////////
//
//	GameObjDefParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class GameObjDefParameterCtrlClass : public DefParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	GameObjDefParameterCtrlClass (GameObjDefParameterClass *param)
		:	DefParameterCtrlClass (param)				{ }
	virtual ~GameObjDefParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_GAMEOBJDEFINITIONID; }

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	virtual uint32				Get_Def_Class_ID (void) { return CLASSID_GAME_OBJECTS; }
	virtual int					Get_Def_Icon (void)		{ return OBJECT_ICON; }		
};


///////////////////////////////////////////////////////////////////////
//
//	WeaponObjDefParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class WeaponObjDefParameterCtrlClass : public GameObjDefParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	WeaponObjDefParameterCtrlClass (WeaponObjDefParameterClass *param)
		:	GameObjDefParameterCtrlClass (param)		{ }
	virtual ~WeaponObjDefParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_WEAPONOBJDEFINITIONID; }

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	virtual uint32				Get_Def_Class_ID (void) { return CLASSID_DEF_WEAPON; }
	virtual int					Get_Def_Icon (void)		{ return OBJECT_ICON; }
};


///////////////////////////////////////////////////////////////////////
//
//	AmmoObjDefParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class AmmoObjDefParameterCtrlClass : public GameObjDefParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	AmmoObjDefParameterCtrlClass (AmmoObjDefParameterClass *param)
		:	GameObjDefParameterCtrlClass (param)	{ }
	virtual ~AmmoObjDefParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_AMMOOBJDEFINITIONID; }

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	virtual uint32				Get_Def_Class_ID (void) { return CLASSID_DEF_AMMO; }
	virtual int					Get_Def_Icon (void)		{ return OBJECT_ICON; }
};


///////////////////////////////////////////////////////////////////////
//
//	ExplosionObjDefParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class ExplosionObjDefParameterCtrlClass : public GameObjDefParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	ExplosionObjDefParameterCtrlClass (ExplosionObjDefParameterClass *param)
		:	GameObjDefParameterCtrlClass (param)	{ }
	virtual ~ExplosionObjDefParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_EXPLOSIONDEFINITIONID; }

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	virtual uint32				Get_Def_Class_ID (void) { return CLASSID_DEF_EXPLOSION; }
	virtual int					Get_Def_Icon (void)		{ return OBJECT_ICON; }
};


///////////////////////////////////////////////////////////////////////
//
//	SoundDefParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class SoundDefParameterCtrlClass : public DefParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	SoundDefParameterCtrlClass (SoundDefParameterClass *param)
		:	m_Parameter (param),
			DefParameterCtrlClass (param)				{ }
	virtual ~SoundDefParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_SOUNDDEFINITIONID; }

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	virtual uint32				Get_Def_Class_ID (void) { return CLASSID_SOUND; }
	virtual int					Get_Def_Icon (void)		{ return SOUND_ICON; }
		
	////////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////////
	SoundDefParameterClass *	m_Parameter;
};


///////////////////////////////////////////////////////////////////////
//
//	ScriptParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class ScriptParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	ScriptParameterCtrlClass (ScriptParameterClass *param)
		:	m_Parameter (param),
			m_ButtonID (0)								{ }
	virtual ~ScriptParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_SCRIPT; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);
	bool							On_Command (HWND parent_wnd, WPARAM wparam, LPARAM lparam);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	void							Fill_Combo (void);
		
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CButton							m_Button;
	CButton							m_CheckBoxCtrl;
	ScriptParameterClass *		m_Parameter;
	int								m_ButtonID;
};


///////////////////////////////////////////////////////////////////////
//
//	DefIDListParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class DefIDListParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	DefIDListParameterCtrlClass (DefIDListParameterClass *param)
		:	m_Parameter (param),
			m_ButtonID (0)								{ }
	virtual ~DefIDListParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_DEFINITIONIDLIST; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);
	bool							On_Command (HWND parent_wnd, WPARAM wparam, LPARAM lparam);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CButton							m_Button;
	CStatic							m_StaticText;
	DefIDListParameterClass *	m_Parameter;
	int								m_ButtonID;
};


///////////////////////////////////////////////////////////////////////
//
//	ZoneParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class ZoneParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	ZoneParameterCtrlClass (ZoneParameterClass *param)
		:	m_Parameter (param),
			m_ButtonID (0)								{ }
	virtual ~ZoneParameterCtrlClass (void)		{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_ZONE; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);
	bool							On_Command (HWND parent_wnd, WPARAM wparam, LPARAM lparam);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CButton						m_Button;
	CStatic						m_StaticText;
	ZoneParameterClass *		m_Parameter;
	int							m_ButtonID;
};


///////////////////////////////////////////////////////////////////////
//
//	FilenameListParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class FilenameListParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	FilenameListParameterCtrlClass (FilenameListParameterClass *param)
		:	m_Parameter (param),
			m_ButtonID (0)										{ }
	virtual ~FilenameListParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_FILENAMELIST; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);
	void							Read_Data (HWND parent_wnd);
	bool							On_Command (HWND parent_wnd, WPARAM wparam, LPARAM lparam);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CButton								m_Button;
	CStatic								m_StaticText;
	FilenameListParameterClass *	m_Parameter;
	int									m_ButtonID;
};


///////////////////////////////////////////////////////////////////////
//
//	SeparatorParameterCtrlClass
//
///////////////////////////////////////////////////////////////////////
class SeparatorParameterCtrlClass : public ParameterCtrlClass
{
public:
	
	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	SeparatorParameterCtrlClass (SeparatorParameterClass *param)
		:	m_Parameter (param)							{ }
	virtual ~SeparatorParameterCtrlClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	ParameterClass::Type		Get_Type (void) const { return ParameterClass::TYPE_SEPARATOR; }
	int							Create (HWND parent_wnd, int id_start, LPRECT pos);
	void							Resize (const CRect &rect);

	ParameterClass *			Get_Parameter (void) const { return m_Parameter; };
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	CStatic								m_FrameCtrl;
	CStatic								m_TextCtrl;
	SeparatorParameterClass *		m_Parameter;
};


#endif //__PARAMETER_CTRLS_H
