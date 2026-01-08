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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/excel.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/17/02 1:43p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
///#include <comutil.h>

#include "excel.h"
#include <stdio.h>
#include <assert.h>

#include <comutil.h>
#include <comdef.h>
#include <afxdisp.h>
#include "excel8.h"
#include "Utils.h"


typedef enum {
    xlAll = -4104,
    xlAutomatic = -4105,
    xlBoth = 1,
    xlCenter = -4108,
    xlChecker = 9,
    xlCircle = 8,
    xlCorner = 2,
    xlCrissCross = 16,
    xlCross = 4,
    xlDiamond = 2,
    xlDistributed = -4117,
    xlDoubleAccounting = 5,
    xlFixedValue = 1,
    xlFormats = -4122,
    xlGray16 = 17,
    xlGray8 = 18,
    xlGrid = 15,
    xlHigh = -4127,
    xlInside = 2,
    xlJustify = -4130,
    xlLightDown = 13,
    xlLightHorizontal = 11,
    xlLightUp = 14,
    xlLightVertical = 12,
    xlLow = -4134,
    xlManual = -4135,
    xlMinusValues = 3,
    xlModule = -4141,
    xlNextToAxis = 4,
    xlNone = -4142,
    xlNotes = -4144,
    xlOff = -4146,
    xlOn = 1,
    xlPercent = 2,
    xlPlus = 9,
    xlPlusValues = 2,
    xlSemiGray75 = 10,
    xlShowLabel = 4,
    xlShowLabelAndPercent = 5,
    xlShowPercent = 3,
    xlShowValue = 2,
    xlSimple = -4154,
    xlSingle = 2,
    xlSingleAccounting = 4,
    xlSolid = 1,
    xlSquare = 1,
    xlStar = 5,
    xlStError = 4,
    xlToolbarButton = 2,
    xlTriangle = 3,
    xlGray25 = -4124,
    xlGray50 = -4125,
    xlGray75 = -4126,
    xlBottom = -4107,
    xlLeft = -4131,
    xlRight = -4152,
    xlTop = -4160,
    xl3DBar = -4099,
    xl3DSurface = -4103,
    xlBar = 2,
    xlColumn = 3,
    xlCombination = -4111,
    xlCustom = -4114,
    xlDefaultAutoFormat = -1,
    xlMaximum = 2,
    xlMinimum = 4,
    xlOpaque = 3,
    xlTransparent = 2,
    xlBidi = -5000,
    xlLatin = -5001,
    xlContext = -5002,
    xlLTR = -5003,
    xlRTL = -5004,
    xlVisualCursor = 2,
    xlLogicalCursor = 1,
    xlSystem = 1,
    xlPartial = 3,
    xlHindiNumerals = 3,
    xlBidiCalendar = 3,
    xlGregorian = 2,
    xlComplete = 4,
    xlScale = 3,
    xlClosed = 3,
    xlColor1 = 7,
    xlColor2 = 8,
    xlColor3 = 9,
    xlConstants = 2,
    xlContents = 2,
    xlBelow = 1,
    xlCascade = 7,
    xlCenterAcrossSelection = 7,
    xlChart4 = 2,
    xlChartSeries = 17,
    xlChartShort = 6,
    xlChartTitles = 18,
    xlClassic1 = 1,
    xlClassic2 = 2,
    xlClassic3 = 3,
    xl3DEffects1 = 13,
    xl3DEffects2 = 14,
    xlAbove = 0,
    xlAccounting1 = 4,
    xlAccounting2 = 5,
    xlAccounting3 = 6,
    xlAccounting4 = 17,
    xlAdd = 2,
    xlDebugCodePane = 13,
    xlDesktop = 9,
    xlDirect = 1,
    xlDivide = 5,
    xlDoubleClosed = 5,
    xlDoubleOpen = 4,
    xlDoubleQuote = 1,
    xlEntireChart = 20,
    xlExcelMenus = 1,
    xlExtended = 3,
    xlFill = 5,
    xlFirst = 0,
    xlFloating = 5,
    xlFormula = 5,
    xlGeneral = 1,
    xlGridline = 22,
    xlIcons = 1,
    xlImmediatePane = 12,
    xlInteger = 2,
    xlLast = 1,
    xlLastCell = 11,
    xlList1 = 10,
    xlList2 = 11,
    xlList3 = 12,
    xlLocalFormat1 = 15,
    xlLocalFormat2 = 16,
    xlLong = 3,
    xlLotusHelp = 2,
    xlMacrosheetCell = 7,
    xlMixed = 2,
    xlMultiply = 4,
    xlNarrow = 1,
    xlNoDocuments = 3,
    xlOpen = 2,
    xlOutside = 3,
    xlReference = 4,
    xlSemiautomatic = 2,
    xlShort = 1,
    xlSingleQuote = 2,
    xlStrict = 2,
    xlSubtract = 3,
    xlTextBox = 16,
    xlTiled = 1,
    xlTitleBar = 8,
    xlToolbar = 1,
    xlVisible = 12,
    xlWatchPane = 11,
    xlWide = 3,
    xlWorkbookTab = 6,
    xlWorksheet4 = 1,
    xlWorksheetCell = 3,
    xlWorksheetShort = 5,
    xlAllExceptBorders = 6,
    xlLeftToRight = 2,
    xlTopToBottom = 1,
    xlVeryHidden = 2,
    xlDrawingObject = 14
} Constants;

typedef enum {
    xlHairline = 1,
    xlMedium = -4138,
    xlThick = 4,
    xlThin = 2
} XlBorderWeight;

typedef enum {
    xlContinuous = 1,
    xlDash = -4115,
    xlDashDot = 4,
    xlDashDotDot = 5,
    xlDot = -4118,
    xlDouble = -4119,
    xlSlantDashDot = 13,
    xlLineStyleNone = -4142
} XlLineStyle;

typedef enum {
    xlInsideHorizontal = 12,
    xlInsideVertical = 11,
    xlDiagonalDown = 5,
    xlDiagonalUp = 6,
    xlEdgeBottom = 9,
    xlEdgeLeft = 7,
    xlEdgeRight = 10,
    xlEdgeTop = 8
} XlBordersIndex;


static const int xlWorkbookNormal = -4143;
static const int xlNoChange = 1;
static const int xlLocalSessionChanges = 2;
static const int xlWBATWorksheet = -4167;

static VARIANT no, yes, dummy, dummy0, nullstring, empty;
static VARIANT continuous, automatic, medium, thin, none;
static VARIANT yellow, solid;



/////////////////////////////////////////////////////////////////////////
//	Static member initialization
/////////////////////////////////////////////////////////////////////////
_Application *		ExcelClass::Application		= NULL;
Workbooks *			ExcelClass::WorkbooksObj	= NULL;
_Worksheet *		ExcelClass::WorksheetObj	= NULL;
_Workbook *			ExcelClass::WorkbookObj		= NULL;
Range *				ExcelClass::RangeObj			= NULL;
StringClass			ExcelClass::CurrPath;


/////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
/////////////////////////////////////////////////////////////////////////
bool
ExcelClass::Initialize (void)
{
	//
	//	Don't reinitialize
	//
	if (Application != NULL) {
		return true;
	}

	bool retval = false;

	//
	//	Allocate the application object
	//
	Application = new _Application;
	
	//
	//	Attempt to start excel
	//
	if (Application->CreateDispatch ("Excel.Application")) {

		//
		//	Get the workbook interface
		//
		LPDISPATCH dispatch = Application->GetWorkbooks ();
		if (dispatch != NULL) {
			retval = true;

			//
			//	Create the neccessary interfaces
			//
			WorkbooksObj	= new Workbooks (dispatch);
			WorksheetObj	= new _Worksheet;
			RangeObj			= new Range;

			//
			//	Dunno
			//
			V_VT ( &no ) = VT_BOOL;
			V_VT ( &yes ) = VT_BOOL;
			V_VT ( &dummy ) = VT_I4;
			V_VT ( &dummy0 ) = VT_I4;
			V_VT ( &nullstring ) = VT_BSTR ;
			V_VT ( &empty ) = VT_EMPTY;
			V_VT ( &continuous ) = VT_I4;
			V_VT ( &automatic ) = VT_I4;
			V_VT ( &medium ) = VT_I4;
			V_VT ( &thin ) = VT_I4;
			V_VT ( &none ) = VT_I4;
			V_VT ( &solid ) = VT_I4;
			V_VT ( &yellow ) = VT_I4;

			V_BOOL ( &no ) = false;
			V_BOOL ( &yes ) = true;
			V_I4 ( &dummy ) = 1;
			V_I4 ( &dummy0 ) = 0;
			V_BSTR ( &nullstring ) = SysAllocString ( OLESTR ("") );

			V_I4 ( &continuous ) = xlContinuous;
			V_I4 ( &automatic ) = xlAutomatic;
			V_I4 ( &medium ) = xlMedium;
			V_I4 ( &thin ) = xlThin;
			V_I4 ( &none ) = xlThin;
			V_I4 ( &solid ) = xlSolid;
			V_I4 ( &yellow ) = 6;
		}
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
/////////////////////////////////////////////////////////////////////////
void
ExcelClass::Shutdown (void)
{
	Close_Workbook ();


	//
	//	Free our interfaces
	//
	SAFE_DELETE (RangeObj);
	SAFE_DELETE (WorksheetObj);

	if (WorkbooksObj != NULL) {		
		WorkbooksObj->Close ();
		SAFE_DELETE (WorksheetObj);
	}

	if (Application != NULL) {
		Application->Quit ();
		Application->ReleaseDispatch ();
		SAFE_DELETE (Application);
	}

	//
	//	Dunno
	//
	VariantClear (&nullstring);
	return ;
}

#define WIDETOOLE(x) (reinterpret_cast<const wchar_t*>(static_cast<const unichar_t*>(x)))
/////////////////////////////////////////////////////////////////////////
//
//	New_Workbook
//
/////////////////////////////////////////////////////////////////////////
void
ExcelClass::New_Workbook (const char *template_filename)
{
	if (template_filename == NULL) {
		return ;
	}

	//
	//	Configure a variant with the filename we wish to add
	//
	WideStringClass wide_filename;
	wide_filename.Convert_From (template_filename);

	VARIANT temp;
	V_VT (&temp)	= VT_BSTR;
	V_BSTR (&temp)	= ::SysAllocString (WIDETOOLE(wide_filename));

	//
	//	Create the new workbook
	//
	LPDISPATCH dispatch = WorkbooksObj->Add (temp);	
	if (dispatch != NULL) {

		//
		//	Create a new wrapper object for this workbook
		//
		WorkbookObj = new _Workbook (dispatch);
		Select_Active_Sheet ();
	}

	//
	//	Free the variant's data
	//
	::VariantClear (&temp);
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Open_Workbookk
//
/////////////////////////////////////////////////////////////////////////
void
ExcelClass::Open_Workbook (const char *filename)
{
	//
	//	Attempt to open this workbook
	//
	LPDISPATCH dispatch = WorkbooksObj->Open (filename, dummy0, yes, dummy, nullstring,
														nullstring, yes, dummy, dummy, no, no, dummy, no);

	if (dispatch != NULL) {
		
		//
		//	Wrap the dispatch pointer in a friendlier object
		//
		WorkbookObj = new _Workbook (dispatch);
		Select_Active_Sheet ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Save_Workbook
//
/////////////////////////////////////////////////////////////////////////
void
ExcelClass::Save_Workbook (const char *filename)
{
	if (WorkbookObj == NULL) {
		return ;
	}

	VARIANT name, fileformat, rc;

	WideStringClass wide_filename;
	wide_filename.Convert_From (filename);

	V_VT (&name)	= VT_BSTR;
	V_BSTR (&name)	= SysAllocString (WIDETOOLE(wide_filename));

	V_VT (&fileformat) = VT_I4;
	V_I4 (&fileformat) = xlWorkbookNormal;

	V_VT (&rc) = VT_I4;
	V_I4 (&rc) = xlLocalSessionChanges;

	WorkbookObj->SaveAs ( name, fileformat, nullstring, nullstring, no, no, 
					xlNoChange, rc, no, empty, empty );

	VariantClear (&name);

	/*if (WorkbookObj != NULL) {
		WorkbookObj->Save ();
	}*/

	/*dispatch;
	VARIANT temp;

	V_VT ( &temp ) = VT_I4;
	V_I4 ( &temp ) = xlWBATWorksheet;

	if (CurrPath.Is_Empty () == false) {

		//
		//	Check to see if the file still exists
		//
		if (::GetFileAttributes (CurrPath) != 0xFFFFFFFF) {
				
				WideStringClass wide_filename;
				wide_filename.Convert_From (CurrPath);

				//
				//	Fill in the path of the file
				//
				V_VT (&temp)	= VT_BSTR;
				V_BSTR (&temp) = ::SysAllocString (wide_filename);
			}
		}
	}

	//
	//	Save the workbook and refresh our workbook object
	//
	LPDISPATCH dispatch = WorkbooksObj->Add (temp);
	if (dispatch != NULL) {
		WorkbookObj = new _Workbook (dispatch);
	}

	Select_Active_Sheet ();
	VariantClear (&temp);*/
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Close_Workbook
//
/////////////////////////////////////////////////////////////////////////
void
ExcelClass::Close_Workbook (void)
{
	//
	//	Simply close and delete the workbook
	//
	if (WorkbookObj != NULL) {
		WorkbookObj->SetSaved (true);
		WorkbookObj->Close (no, nullstring, no);
		SAFE_DELETE (WorkbookObj);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Get_String
//
/////////////////////////////////////////////////////////////////////////
bool
ExcelClass::Get_String (int row, int col, WideStringClass &string)
{
	bool retval = false;

	//
	//	Get the cell's contents
	//
	VARIANT variant_value;
	if (Get_Cell (row, col, variant_value)) {
		
		//
		//	Is this a string?
		//
		if (V_VT (&variant_value) == VT_BSTR) {
			string = reinterpret_cast<unichar_t*>(V_BSTR (&variant_value));
			retval = true;
		} else if (V_VT (&variant_value) == VT_R4) {
			int value = (int)variant_value.fltVal;
			string.Format (U_CHAR("%d"), value);
			retval = true;
		} else if (V_VT (&variant_value) == VT_R8) {
			int value = (int)variant_value.dblVal;
			string.Format (U_CHAR("%d"), value);
			retval = true;
		} else if (	V_VT (&variant_value) == VT_I2 || V_VT (&variant_value) == VT_I4 ||
						V_VT (&variant_value) == VT_UI2 || V_VT (&variant_value) == VT_UI4)
		{
			string.Format (U_CHAR("%d"), variant_value.iVal);
			retval = true;
		}
		
		//
		//	Free the variant data
		//
		::VariantClear (&variant_value);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Set_String
//
/////////////////////////////////////////////////////////////////////////
bool
ExcelClass::Set_String (int row, int col, const WideStringClass &value)
{
	//
	//	Configure a variant with the string data we want to set
	//
	VARIANT variant_value;	
	V_VT (&variant_value)	= VT_BSTR;
	V_BSTR (&variant_value) = ::SysAllocString (WIDETOOLE(value));

	//
	//	Put the string into the cell
	//
	bool retval = Set_Cell (row, col, variant_value);

	//
	//	Free the variant data
	//
	::VariantClear (&variant_value);
	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Get_Int
//
/////////////////////////////////////////////////////////////////////////
bool
ExcelClass::Get_Int (int row, int col, int &value)
{
	bool retval = false;

	//
	//	Get the cell's contents
	//
	VARIANT variant_value;
	if (Get_Cell (row, col, variant_value)) {
		
		//
		//	Is this an integer?
		//
		if (V_VT (&variant_value) == VT_I4) {
			value = V_I4(&variant_value);
			retval = true;
		}
		
		//
		//	Free the variant data
		//
		::VariantClear (&variant_value);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Set_Int
//
/////////////////////////////////////////////////////////////////////////
bool
ExcelClass::Set_Int (int row, int col, int value)
{
	//
	//	Configure a variant with the integer data we want to set
	//
	VARIANT variant_value;	
	V_VT (&variant_value)	= VT_I4;
	V_I4 (&variant_value)	= value;

	//
	//	Put the integer into the cell
	//
	bool retval = Set_Cell (row, col, variant_value);

	//
	//	Free the variant data
	//
	::VariantClear (&variant_value);
	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Select_Active_Sheet
//
/////////////////////////////////////////////////////////////////////////
void
ExcelClass::Select_Active_Sheet (void)
{
	LPDISPATCH dispatch = Application->GetActiveSheet ();

	//
	//	If possible, attach the new worksheet
	//
	if (dispatch != NULL) {
		WorksheetObj->ReleaseDispatch ();
		WorksheetObj->AttachDispatch (dispatch);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Get_Cell
//
/////////////////////////////////////////////////////////////////////////
bool
ExcelClass::Get_Cell (int row, int col, VARIANT &result)
{
	if (WorksheetObj == NULL) {
		return false;
	}

	bool retval = false;
	
	//
	//	Generate the name of the cell we'll be using
	//
	WideStringClass cell_name;
	cell_name.Format (U_CHAR("%c%d"), 'A'+col , row + 1);

 	//
	//	Configure a variant object for use as a cell ID
	//
	VARIANT cell;
	V_VT (&cell)	= VT_BSTR;
 	V_BSTR (&cell)	= ::SysAllocString (WIDETOOLE(cell_name));

	//
	//	Get the data
	//
	LPDISPATCH dispatch = WorksheetObj->GetRange (cell, cell);
	if (dispatch != NULL) {

		//
		//	Retrieve the data from the range object
		//
		RangeObj->AttachDispatch (dispatch);
		result = RangeObj->GetValue ();
		RangeObj->ReleaseDispatch ();
		retval = true;
	}

	//
	//	Free the variant data
	//
	::VariantClear (&cell);
	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Set_Cell
//
/////////////////////////////////////////////////////////////////////////
bool
ExcelClass::Set_Cell (int row, int col, const VARIANT &data)
{
	if (WorksheetObj == NULL) {
		return false;
	}

	bool retval = false;
	
	//
	//	Generate the name of the cell we'll be using
	//
	WideStringClass cell_name;
	cell_name.Format (U_CHAR("%c%d"), 'A'+col, row + 1);

 	//
	//	Configure a variant object for use as a cell ID
	//
	VARIANT cell;
	V_VT (&cell)	= VT_BSTR;
 	V_BSTR (&cell)	= ::SysAllocString (WIDETOOLE(cell_name));

	//
	//	Get the cell range
	//
	LPDISPATCH dispatch = WorksheetObj->GetRange (cell, cell);
	if (dispatch != NULL) {
		
		//
		//	Shove our new data into this range
		//
		RangeObj->AttachDispatch (dispatch);
		RangeObj->SetValue (data);
		RangeObj->ReleaseDispatch ();
		retval = true;
	}

	//
	//	Free the variant data
	//
	::VariantClear (&cell);
	return retval;
}
