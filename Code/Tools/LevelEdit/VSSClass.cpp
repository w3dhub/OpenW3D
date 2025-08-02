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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VSSClass.cpp                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/29/02 3:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"

#include "vssclass.h"
#include "utils.h"
#include "filemgr.h"
#include "oleauto.h"
#include "afxdisp.h"
#include "rawfile.h"


typedef struct tagVSS_ERROR
{
	HRESULT hresult;
	LPCTSTR description;
} VSS_ERROR;

VSS_ERROR _ErrorArray[] =
{
	{ -2147164975,	"You cannot change the Admin name or access privileges" },
	{ -2147166139,	"Invalid DOS path: < >" },
	{ -2147166373,	"File < > is currently checked out by < >" },
	{ -2147166374,	"You currently have file < > checked out" },
	{ -2147166386,	"Cannot delete the root project" },
	{ -2147166391, "You do not have access rights to < >" },
	{ -2147166398,	"Cannot move the root project" },
	{ -2147166404,	"File < > is not checked out" },
	{ -2147166405,	"File < > is not shared by any other projects" },
	{ -2147166411,	"Cannot Rename the root project" },
	{ -2147166417,	"A project cannot be shared under a descendant." },
	{ -2147166418,	"File < > is already shared by this project" },
	{ -2147166424,	"The SourceSafe database has been locked by the Administrator." },
	{ -2147166519,	"Invalid password" },
	{ -2147166521,	"Cannot delete the Admin user" },
	{ -2147166522,	"Permission denied " },
	{ -2147166525,	"User ""< >"" already exists" },
	{ -2147166526,	"User ""< >"" not found " },
	{ -2147166572,	"An item with the name < > already exists" },
	{ -2147166574,	"You can not move a project under itself" },
	{ -2147166583,	"Version not found" },
	{ -2147167977,	"The SourceSafe database path < > does not exist. Please select another database." },
	{ -2147210887,	"This command only works on projects." },
	{ -2147210888,	"This command only works on files." },
	{ -2147352566,	"Invalid access code (bad parameter)*" }
};



///////////////////////////////////////////////////////////////////
//
//	Sys_String_To_ANSI
//
LPTSTR
Sys_String_To_ANSI (BSTR bstr)
{
	int len = ::SysStringLen (bstr);

	TCHAR *ansi_string = new TCHAR[len + 1];
	ASSERT(ansi_string != NULL);

	// Convert the BSTR to ANSI using the ANSI code page
	::WideCharToMultiByte (CP_ACP,
								  0,
								  bstr,
								  len,
								  ansi_string,
								  len + 1,
								  NULL,
								  NULL);

	// Ensure the new string is NULL terminated
	ansi_string[len] = 0;

	// Return a pointer to the ANSI string
	return ansi_string;
}


///////////////////////////////////////////////////////////////////
//
//	Alloc_Sys_String
//
BSTR
Alloc_Sys_String (LPCTSTR string)
{
	BSTR sys_string = NULL;

	if (string != NULL) {
		
		// Copy the ascii format string to a wide-character string
		WCHAR *pwide_string = new WCHAR [::lstrlen (string) + 1];
		WCHAR *ptemp_string = pwide_string;
		while (*string) {
			*ptemp_string++ = (WCHAR)*string++;
		}
		*ptemp_string = WCHAR('\0');

		// Allocate a system-string (COM-safe) from the wide-character string.
		sys_string = ::SysAllocString (pwide_string);

		// Free the wide-character string we allocated earlier
		delete [] pwide_string;
		pwide_string = NULL;
	}

	// Return the system string
	return sys_string;
}


///////////////////////////////////////////////////////////////////
//
//	IDispatch_Get_Property
//
bool
IDispatch_Get_Property
(
	LPDISPATCH pdispatch,
	DISPID dispid,
	VARIANT *presult
)
{
	// Assume failure
	bool retval = false;

	// Params OK?
	ASSERT (pdispatch != NULL);
	if (pdispatch != NULL) {

		// Get this property w/o passing any args
		DISPPARAMS no_args = {NULL, NULL, 0, 0};			
		retval = SUCCEEDED (pdispatch->Invoke (dispid,
															IID_NULL,
															NULL,
															DISPATCH_PROPERTYGET,
															&no_args,
															presult,
															NULL,
															NULL));
	}
	
	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	IDispatch_Get_Property
//
bool
IDispatch_Get_Property
(
	LPDISPATCH pdispatch,
	DISPID dispid,
	VARIANT &var_arg,
	VARIANT *presult
)
{
	// Assume failure
	bool retval = false;

	// Params OK?
	ASSERT (pdispatch != NULL);
	if (pdispatch != NULL) {

		// Fill in the dispatch-params structure with our single param
		DISPPARAMS args = {NULL, NULL, 0, 0};
		args.rgvarg = &var_arg;
		args.cArgs = 1;		
		
		// Get this property an pass in the single arguement
		EXCEPINFO excep_info = { 0 };
		retval = SUCCEEDED (pdispatch->Invoke (dispid,
															IID_NULL,
															NULL,
															DISPATCH_PROPERTYGET,
															&args,
															presult,
															&excep_info,
															NULL));
	}
	
	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get_VSS_Interface
//
void
VSSClass::Get_VSS_Interface (void)
{
	if (m_pIVSSDatabase == NULL) {
	
		// Attempt to create an instance of the VSS Database interface
		LPUNKNOWN punknown = NULL;
		HRESULT hresult = ::CoCreateInstance (CLSID_VSSDatabase,
														  NULL,
														  CLSCTX_INPROC_SERVER,
														  IID_IVSSDatabase,
														  (LPVOID *)&punknown);
		if (SUCCEEDED (hresult)) {
			
			// Get a pointer to the 'newest' interface we know about
			punknown->QueryInterface (IID_IVSSDatabase, (LPVOID *)&m_pIVSSDatabase);

			// Release our hold on the original IUnknown interface
			COM_RELEASE (punknown);
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	Open_Database
//
bool
VSSClass::Open_Database
(
	LPCTSTR ini_filename,
	LPCTSTR username,
	LPCTSTR password
)
{
	// Make sure we have an interface to work with
	Get_VSS_Interface ();

	// Assume failure
	bool retval = false;

	m_bReadOnly = TRUE;
	ASSERT (m_pIVSSDatabase != NULL);
	if (m_pIVSSDatabase != NULL) {
		
		// Convert the string params to COM-safe system strings
		BSTR bstr_filename = ::Alloc_Sys_String (ini_filename);
		BSTR bstr_username = ::Alloc_Sys_String (username != NULL ? username : "");
		BSTR bstr_password = ::Alloc_Sys_String (password != NULL ? password : "");

		// Ask VSS to open this database
		retval = SUCCEEDED (m_pIVSSDatabase->Open (bstr_filename, bstr_username, bstr_password));
		if (retval) {
			
			//
			//	Determine if the user is read-only
			//
			IVSSUser *user = NULL;
			if (SUCCEEDED (m_pIVSSDatabase->get_User (bstr_username, &user))) {
				VARIANT_BOOL readonly = 0;
				user->get_ReadOnly (&readonly);
				m_bReadOnly = (readonly != 0);
				COM_RELEASE (user);
			}

			//
			//	Set the working directory
			//

			/*IVSSItem *root_item = Get_VSS_Item ("$/");
			if (root_item != NULL) {
				
				LPCTSTR local_path	= ::Get_File_Mgr ()->Get_Base_Path ();
				BSTR bstr_local_path	= ::Alloc_Sys_String (local_path);
				HRESULT result			= root_item->put_LocalSpec (bstr_local_path);
				ASSERT (SUCCEEDED (result));
				::SysFreeString (bstr_local_path);

				COM_RELEASE (root_item);
			}*/
		}

		// Free the system strings we allocated earlier
		::SysFreeString (bstr_filename);
		::SysFreeString (bstr_username);
		::SysFreeString (bstr_password);
	}

	// Free our inteface pointer if the function wasn't successfull
	if (retval == false) {
		COM_RELEASE (m_pIVSSDatabase);
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get_VSS_Item_From_Local_Path
//
IVSSItem *
VSSClass::Get_VSS_Item_From_Local_Path (LPCTSTR local_filename)
{
	// Assume failure
	IVSSItem *pitem = NULL;

	ASSERT (m_pIVSSDatabase != NULL);
	ASSERT (local_filename != NULL);

	// State OK?
	if ((m_pIVSSDatabase != NULL) &&
		 (local_filename != NULL)) {
		
		// Lookup the relative path
		TCHAR vss_path[MAX_PATH];
		CString rel_path = ::Get_File_Mgr ()->Make_Relative_Path (local_filename);
		::wsprintf (vss_path, "$/%s", (LPCTSTR)rel_path);

		// Now, lookup the VSS item
		pitem = Get_VSS_Item (vss_path);
	}

	// Return a pointer to the IVSSItem interface
	return pitem;
}


///////////////////////////////////////////////////////////////////
//
//	Get_VSS_Item
//
IVSSItem *
VSSClass::Get_VSS_Item (LPCTSTR vss_path)
{
	// Assume failure
	IVSSItem *pitem = NULL;

	ASSERT (m_pIVSSDatabase != NULL);
	ASSERT (vss_path != NULL);

	// State OK?
	if ((m_pIVSSDatabase != NULL) &&
		 (vss_path != NULL)) {
		
		// Convert the ascii string to a COM-safe BSTR
		BSTR bstr_vss_path = ::Alloc_Sys_String (vss_path);

		/*VARIANT var_arg;		
		var_arg.vt = VT_BSTR;
		var_arg.bstrVal = bstr_vss_path;
		VARIANT result = { 0 };

		// Use the IDispatch interface to get the VSSItem property
		bool success = ::IDispatch_Get_Property (m_pIVSSDatabase,
															  0x00000006,
															  var_arg,
															  &result);		*/
	
		HRESULT result = m_pIVSSDatabase->get_VSSItem (bstr_vss_path, 0, &pitem);
		if (FAILED (result)) {
			pitem = NULL;
		}
		
		// Was the property-get successful?
		/*if (success) {

			// The IVSSItem pointer should be passed back in the result
			pitem =  (IVSSItem *)result.ppdispVal;
		}*/

		// Free the system string we allocated earlier
		::SysFreeString (bstr_vss_path);
	}

	// Return a pointer to the IVSSItem interface
	return pitem;
}


///////////////////////////////////////////////////////////////////
//
//	Does_File_Exist
//
bool
VSSClass::Does_File_Exist (LPCTSTR local_filename)
{
	// Assume failure
	bool retval = false;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);

	// If we successfully got the VSS item, then it exists in the DB
	if (pitem != NULL) {
		retval = true;
				
		// Release our hold on the item
		COM_RELEASE (pitem);
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get_Type
//
VSSItemType
VSSClass::Get_Type (LPCTSTR local_filename)
{
	VSSItemType type = VSSITEM_FILE;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);
	ASSERT (pitem != NULL);
	if (pitem != NULL) {
		int temp = 0;
		pitem->get_Type (&temp);
		type = (VSSItemType)temp;

		// Release our hold on the item
		COM_RELEASE (pitem);
	}

	// Return the type (either project or file)
	return type;
}


///////////////////////////////////////////////////////////////////
//
//	Get
//
bool
VSSClass::Get (LPCTSTR local_filename)
{
	// Assume failure
	bool retval = false;

	//
	// If this 'file' is a directory, then get files recursively, otherwise
	//	simply 'get' the desired file.
	//
	if (Get_Type (local_filename) == VSSITEM_PROJECT) {
		retval = Get_Subproject (local_filename);
	} else {

		// Get a pointer to this 'item' inside the VSS database.
		IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);
		ASSERT (pitem != NULL);
		if (pitem != NULL) {
			
			// Do the actual file 'get' from VSS.
			retval = Get (local_filename, pitem);
			COM_RELEASE (pitem);
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get
//
bool
VSSClass::Get
(
	LPCTSTR local_filename,
	IVSSItem *pitem
)
{
	// Assume failure
	bool retval = false;

	// Params OK?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		if (Get_File_Status (local_filename, NULL, 0, pitem) != VSSFILE_CHECKEDOUT_ME) {

			// Convert the ascii string to a COM-safe BSTR
			BSTR bstr_filename = ::Alloc_Sys_String (local_filename);

			//
			//	Get the file
			//
			VARIANT_BOOL is_different = 0;
			HRESULT hresult = pitem->get_IsDifferent (bstr_filename, &is_different);
			if (!SUCCEEDED (hresult) || (is_different != 0)) {
				retval = SUCCEEDED (pitem->Get (&bstr_filename, VSSFLAG_TIMEMOD | VSSFLAG_REPREPLACE));
			}

			// Free the system string we allocated earlier
			::SysFreeString (bstr_filename);

		} else {
			retval = true;
		}

		//
		//	Only do the 'get' if the vss-version and local versions are different.
		//
		/*VARIANT_BOOL is_different = 0;
		HRESULT hresult = pitem->get_IsDifferent (bstr_filename, &is_different);
		if (!SUCCEEDED (hresult) || (is_different != 0)) {

			//	Get the last mod time of the version in VSS
			SYSTEMTIME vss_file_time = { 0 };		
			if (Get_File_Date (pitem, vss_file_time)) {

				//	Get the last mod time of the local version
				FILETIME local_file_time = { 0 };
				Get_File_Time (local_filename, NULL, NULL, &local_file_time);

				//
				// Should this file be overwritten?
				//
				CTime vss_time (vss_file_time);
				CTime local_time (local_file_time);
				if (vss_time > local_time) {
					retval = SUCCEEDED (pitem->Get (&bstr_filename, VSSFLAG_TIMEMOD | VSSFLAG_REPREPLACE));
				}
			}
		}*/
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get_Subproject
//
bool
VSSClass::Get_Subproject (LPCTSTR local_filename)
{
	// Assume failure
	bool retval = false;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);

	// Did we succesfully get the VSS item?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		Get_Recursive (local_filename, pitem);
		retval = true;

		// Convert the ascii string to a COM-safe BSTR		
		//BSTR bstr_filename = ::Alloc_Sys_String (::Get_File_Mgr ()->Make_Full_Path (local_filename));

		// Ask the VSS interface to recursively 'get' this file for us
		//retval = SUCCEEDED (pitem->Get (&bstr_filename, VSSFLAG_TIMEMOD | VSSFLAG_RECURSYES));

		// Free the system string we allocated earlier
		//::SysFreeString (bstr_filename);
		
		// Release our hold on the item
		COM_RELEASE (pitem);
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Check_Out
//
bool
VSSClass::Check_Out (LPCTSTR local_filename, bool get_locally)
{
	// Assume failure
	bool retval = false;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);

	// Did we succesfully get the VSS item?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		// Convert the ascii string to a COM-safe BSTR
		BSTR bstr_filename = ::Alloc_Sys_String (local_filename);

		// Ask the VSS interface to 'checkout' this file for us
		retval = SUCCEEDED (pitem->Checkout (NULL, bstr_filename, (get_locally ? VSSFLAG_TIMEMOD : VSSFLAG_GETNO) | VSSFLAG_CHKEXCLUSIVENO));

		// Free the system string we allocated earlier
		::SysFreeString (bstr_filename);
		
		// Release our hold on the item
		COM_RELEASE (pitem);
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Check_In
//
bool
VSSClass::Check_In
(
	LPCTSTR local_filename,
	LPCTSTR comment
)
{
	// Assume failure
	bool retval = false;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);

	// Did we succesfully get the VSS item?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		// Convert the ascii string to a COM-safe BSTR
		BSTR bstr_filename = ::Alloc_Sys_String (local_filename);
		BSTR bstr_comment = (comment != NULL) ? ::Alloc_Sys_String (comment) : NULL;

		// Ask the VSS interface to 'Checkin' this file for us
		retval = SUCCEEDED (pitem->Checkin (bstr_comment, bstr_filename, VSSFLAG_CMPCHKSUM | VSSFLAG_RECURSNO | VSSFLAG_UPDUPDATE));

		// Free the system strings we allocated earlier
		::SysFreeString (bstr_filename);	
		if (bstr_comment) {
			::SysFreeString (bstr_filename);
		}
		
		// Release our hold on the item
		COM_RELEASE (pitem);
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Add_File
//
bool
VSSClass::Add_File
(
	LPCTSTR local_filename,
	LPCTSTR comment
)
{
	// Assume failure
	bool retval = false;

	if (Build_Tree (local_filename)) {

		TCHAR parent_dir[MAX_PATH];
		::lstrcpy (parent_dir, ::Strip_Filename_From_Path (local_filename));

		// Get a pointer to this item's parent inside the VSS database.
		IVSSItem *pitem = Get_VSS_Item_From_Local_Path (parent_dir);

		// Did we succesfully get the VSS item?
		ASSERT (pitem != NULL);
		if (pitem != NULL) {

			// Convert the ascii string to a COM-safe BSTR
			BSTR bstr_filename = ::Alloc_Sys_String (local_filename);
			BSTR bstr_comment = (comment != NULL) ? ::Alloc_Sys_String (comment) : NULL;

			// Ask the VSS interface to add this file for us
			IVSSItem *pnew_item = NULL;
			retval = SUCCEEDED (pitem->Add (bstr_filename, bstr_comment, 0, &pnew_item));

			// Free the system strings we allocated earlier
			::SysFreeString (bstr_filename);	
			if (bstr_comment) {
				::SysFreeString (bstr_filename);
			}
			
			// Release our hold on these items
			COM_RELEASE (pitem);
			COM_RELEASE (pnew_item);
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Build_Tree
//
bool
VSSClass::Build_Tree (LPCTSTR local_filename)
{
	// Assume failure
	bool retval = true;

	// Create a relative path to use
	CString rel_path = ::Get_File_Mgr ()->Make_Relative_Path (local_filename);

	// Generate a list of folders from the path
	CString *pfolder_list = NULL;
	int count = ::Build_List_From_String (::Strip_Filename_From_Path (rel_path), "\\", &pfolder_list);
	
	// Loop through all the folders in the path and make sure they
	// exist in VSS
	CString vss_path = "$";
	CString parent = "$";
	for (int ifolder = 0; (ifolder < count) && retval; ifolder ++) {
		
		// See if this path exists in the VSS database
		vss_path += CString ("\\") + pfolder_list[ifolder];
		IVSSItem *pitem = Get_VSS_Item (vss_path);
		if (pitem == NULL) {

			// Get a pointer to the parent so we can create a new folder
			pitem = Get_VSS_Item (parent + CString ("\\"));
			if (pitem != NULL) {

				// Convert the ascii string to a COM-safe BSTR
				BSTR bstr_foldername = ::Alloc_Sys_String (pfolder_list[ifolder]);

				// Ask the VSS interface to add this folder for us
				IVSSItem *pfolder = NULL;
				retval &= (bool)SUCCEEDED (pitem->NewSubproject (bstr_foldername, NULL, &pfolder));
				
				// Free any memory we allocated
				::SysFreeString (bstr_foldername);	
				COM_RELEASE (pfolder);
			}			
		}
		
		COM_RELEASE (pitem);
		parent += CString ("\\") + pfolder_list[ifolder];
	}

	// Free the list
	SAFE_DELETE_ARRAY (pfolder_list);

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Undo_Check_Out
//
bool
VSSClass::Undo_Check_Out (LPCTSTR local_filename)
{
	// Assume failure
	bool retval = false;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);

	// Did we succesfully get the VSS item?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		// Convert the ascii string to a COM-safe BSTR
		BSTR bstr_filename = ::Alloc_Sys_String (local_filename);

		// Ask the VSS interface to undo our 'checkout' on this file for us
		retval = SUCCEEDED (pitem->UndoCheckout (bstr_filename, 0));

		// Free the system string we allocated earlier
		::SysFreeString (bstr_filename);
		
		// Release our hold on the item
		COM_RELEASE (pitem);
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Destroy
//
bool
VSSClass::Destroy (LPCTSTR local_filename)
{
	// Assume failure
	bool retval = false;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);

	// Did we succesfully get the VSS item?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		// Ask the VSS interface to completely destroy this file for us
		retval = SUCCEEDED (pitem->Destroy ());

		// Release our hold on the item
		COM_RELEASE (pitem);
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get_File_Version
//
long
VSSClass::Get_File_Version (LPCTSTR local_filename)
{
	long version = 0L;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);

	// Did we succesfully get the VSS item?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		// Get the version number for this item
		VARIANT result = { 0 };
		::IDispatch_Get_Property (pitem, 0x00000008, &result);
		
		// Pass the result back to the caller
		version = result.lVal;

		// Release our hold on the item
		COM_RELEASE (pitem);
	}

	// Return the version ID
	return version;
}


///////////////////////////////////////////////////////////////////
//
//	Is_File_Different
//
bool
VSSClass::Is_File_Different (LPCTSTR local_filename)
{
	bool different = false;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = Get_VSS_Item_From_Local_Path (local_filename);

	// Did we succesfully get the VSS item?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		// Convert the ansi string to a system string
		BSTR bstr_filename = ::Alloc_Sys_String (local_filename);

		VARIANT var_arg = { 0 };
		var_arg.vt = VT_BSTR;
		var_arg.bstrVal = bstr_filename;

		// Ask VSS if the local and checked-in copy are different
		VARIANT result = { 0 };
		::IDispatch_Get_Property (pitem, 0x00000010, var_arg, &result);
		
		// Convert the COM-bool to a C++ bool
		different = (result.boolVal == 0 ? false : true);

		// Free the system strings we allocated earlier
		::SysFreeString (bstr_filename);

		// Release our hold on the item
		COM_RELEASE (pitem);
	}

	// Return the true/false result code
	return different;
}


///////////////////////////////////////////////////////////////////
//
//	Get_File_Date
//
bool
VSSClass::Get_File_Date
(
	//LPCTSTR local_filename,
	IVSSItem *pitem,
	SYSTEMTIME &system_time
)
{
	// Params OK?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		// Determine which version number we want the date for.
		long current_ver_num = 0;
		pitem->get_VersionNumber (&current_ver_num);

		//
		//	Get the 'versions' list for this item
		//
		IVSSVersions *pversions = NULL;
		HRESULT hresult = pitem->get_Versions (VSSFLAG_HISTIGNOREFILES, &pversions);
		if (SUCCEEDED (hresult) && (pversions != NULL)) {

			//
			// Get the version enumerator for this item
			//
			IUnknown *penum_variant = NULL;
			HRESULT hresult = pversions->_NewEnum (&penum_variant);
			if (SUCCEEDED (hresult)) {
				
				//
				// Convert the IUnknown version enumerator to a standard
				// VARIANT enumerator.
				//
				IEnumVARIANT *pversion_enum = NULL;
				hresult = penum_variant->QueryInterface (IID_IEnumVARIANT, (LPVOID *)&pversion_enum);
				if (SUCCEEDED (hresult) && (pversion_enum != NULL)) {

					//
					//	Loop through all the versions until we've found the right one.
					//
					bool found = false;
					VARIANT version_info = { 0 };
					while ((pversion_enum->Next (1, &version_info, NULL) == S_OK) && !found) {
						
						IVSSVersionOld *pversion = (IVSSVersionOld *)version_info.pdispVal;
						if (pversion != NULL) {
							
							// Get this version's number
							long ver_num = 0L;
							pversion->get_VersionNumber (&ver_num);

							// Is this the version we were looking for?
							if (ver_num == current_ver_num) {
								
								// Get the date from the version
								DATE file_date = { 0 };
								pversion->get_Date (&file_date);

								// Convert the OLE date to a SYSTEMTIME structure
								COleDateTime date_obj (file_date);
								date_obj.GetAsSystemTime (system_time);

								// Success!
								found = true;
							}

							COM_RELEASE (pversion);
						}
					}

					COM_RELEASE (pversion_enum);
				}

				// Get the date of this file
				/*if (::IDispatch_Get_Property (pcheckout, 0x00000004, &result)) {
					COleDateTime date_obj (result.dateVal);
					SYSTEMTIME system_time;
					date_obj.GetAsSystemTime (system_time);

					LPTSTR username = ::Sys_String_To_ANSI (result.bstrVal);

					// Copy the username back to the buffer the caller supplied
					::lstrcpyn (checked_out_username, username, buffer_size);
					checked_out_username[buffer_size-1] = 0;

					// Free the temporary buffer we used to hold the username
					delete [] username;
					username = NULL;
				}*/
				
				COM_RELEASE (penum_variant);
			}
											
			// Release our hold on the interface
			COM_RELEASE (pversions);
		}
	}

	// Return the current file status
	return true;
}


///////////////////////////////////////////////////////////////////
//
//	Get_File_Status
//
VSSFileStatus
VSSClass::Get_File_Status
(
	LPCTSTR		local_filename,
	LPTSTR		checked_out_username,
	DWORD			buffer_size,
	IVSSItem *	item_to_use
)
{
	// Assume not checked out
	VSSFileStatus status = VSSFILE_NOTCHECKEDOUT;

	// Get a pointer to this 'item' inside the VSS database.
	IVSSItem *pitem = item_to_use;
	if (pitem == NULL) {
		pitem = Get_VSS_Item_From_Local_Path (local_filename);
	}

	// Did we succesfully get the VSS item?
	ASSERT (pitem != NULL);
	if (pitem != NULL) {

		// Get the current check-out status for this item
		VARIANT result = { 0 };
		::IDispatch_Get_Property (pitem, 0x0000000e, &result);
		
		// Pass the result back to the caller
		status = (VSSFileStatus)result.lVal;

		// Should we lookup the name of user who has this file checked out?
		if ((status != VSSFILE_NOTCHECKEDOUT) &&
		    (checked_out_username != NULL)) {

			// Get a pointer to the 'checkouts' interface for this item
			bool success = ::IDispatch_Get_Property (pitem, 0x0000000f, &result);
			IVSSCheckouts *pcheckouts = (IVSSCheckouts *)result.ppdispVal;

			if (success && (pcheckouts != NULL)) {
				
				// Get the count of 'checkouts' on this file (should be 1 for our purposes)
				if (::IDispatch_Get_Property (pcheckouts, 0x00000001, &result)) {
					int count = result.lVal;
					if (count > 0) {						
						
						VARIANT var_test = { 0 };
						var_test.vt = VT_I4;
						var_test.lVal = 1;

						VARIANT var_arg = { 0 };
						var_arg.vt = VT_VARIANT | VT_BYREF;
						var_arg.pvarVal = &var_test;

						// Attempt to lookup an interface that represents the first 'checkout'.
						// Remember, VSS supports multiple checkouts per file.
						if (::IDispatch_Get_Property (pcheckouts, 00000000, var_arg, &result)) {
							IVSSCheckout *pcheckout = (IVSSCheckout *)result.ppdispVal;

							// Get the username of this 'checkout'
							if (::IDispatch_Get_Property (pcheckout, 0x00000001, &result)) {
								LPTSTR username = ::Sys_String_To_ANSI (result.bstrVal);

								// Copy the username back to the buffer the caller supplied
								::lstrcpyn (checked_out_username, username, buffer_size);
								checked_out_username[buffer_size-1] = 0;

								// Free the temporary buffer we used to hold the username
								delete [] username;
								username = NULL;
							}
							
							// Release our hold on the checkout interface
							COM_RELEASE (pcheckout);
						}
					}
				}
				
				// Release our hold on the interface
				COM_RELEASE (pcheckouts);
			}
		}

		// Release our hold on the item
		if (item_to_use == NULL) {
			COM_RELEASE (pitem);
		}
	}

	// Return the current file status
	return status;
}


///////////////////////////////////////////////////////////////////
//
//	Get_Error_Description
//
LPCTSTR
VSSClass::Get_Error_Description (HRESULT hresult)
{
	LPCTSTR description = NULL;

	// Loop through all the VSS specific errors that we know about
	int error_count = sizeof (_ErrorArray) / sizeof (VSS_ERROR);
	for (int error = 0;
		  (error < error_count) && (description == NULL);
		  error ++) {

		// Is this the error we are looking for?
		if (_ErrorArray[error].hresult == hresult) {
			description = _ErrorArray[error].description;
		}
	}

	// Return a string containing the error description.
	return description;
}


///////////////////////////////////////////////////////////////////
//
//	Retry_Check_Out
//
bool
VSSClass::Retry_Check_Out
(
	LPCTSTR local_filename,
	int iattempts,
	int idelay
)
{
	// Assume failure
	bool retval = false;

	// Try for up to 'iattempts' times to check-out the file
	for (int iattempt = 0; (iattempt < iattempts) && !retval; iattempt ++) {
		
		// Attempt to checkout the file, if unsucccessful, then wait for a second
		retval = Check_Out (local_filename);
		if (retval == false) {
			::Sleep (idelay);
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Retry_Check_In
//
bool
VSSClass::Retry_Check_In
(
	LPCTSTR local_filename,
	int iattempts,
	int idelay
)
{
	// Assume failure
	bool retval = false;

	// Try for up to 'iattempts' times to check-in the file
	for (int iattempt = 0; (iattempt < iattempts) && !retval; iattempt ++) {
		
		// Attempt to checkin the file, if unsucccessful, then wait for a second
		retval = Check_In (local_filename);
		if (retval == false) {
			::Sleep (idelay);
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get_Recursive
//
void
VSSClass::Get_Recursive
(
	LPCTSTR local_path,
	IVSSItem *pparent
)
{
	// Get a pointer to the 'IVSSItems' interface for this item
	VARIANT result = { 0 };
	//bool success = ::IDispatch_Get_Property (pparent, 0x00000009, &result);

	IVSSItems *pitems = NULL;
	pparent->get_Items (0, &pitems);
	//IVSSItems *pitems = (IVSSItems *)result.ppdispVal;

	if (pitems != NULL) {

		// Get the sub-item count
		if (::IDispatch_Get_Property (pitems, 0x00000001, &result)) {
			int count = result.lVal;
			for (int index = 0; index < count; index ++) {

				VARIANT var_index = { 0 };
				var_index.vt = VT_I4;
				var_index.lVal = index + 1;

				VARIANT var_arg = { 0 };
				var_arg.vt = VT_VARIANT | VT_BYREF;
				var_arg.pvarVal = &var_index;

				// Get the subitem at this index
				if (::IDispatch_Get_Property (pitems, 0x00000000, var_arg, &result)) {
					IVSSItem *psub_item = (IVSSItem *)result.ppdispVal;
					if (psub_item != NULL) {

						// Get the sub-item's name
						TCHAR sub_item_name[MAX_PATH] = { 0 };
						if (::IDispatch_Get_Property (psub_item, 0x00000006, &result)) {
							char *ansi_name = ::Sys_String_To_ANSI (result.bstrVal);
							::lstrcpy (sub_item_name, ansi_name);
							SAFE_DELETE (ansi_name);
						}

						// Build a filename from the path and the subitem name
						TCHAR sub_item_path[MAX_PATH];
						::lstrcpy (sub_item_path, local_path);
						if (sub_item_path[::lstrlen (sub_item_path)-1] != '\\') {
							::lstrcat (sub_item_path, "\\");
						}
						::lstrcat (sub_item_path, sub_item_name);

						// Get the sub-item's type
						VSSItemType sub_item_type = VSSITEM_PROJECT;
						if (::IDispatch_Get_Property (psub_item, 0x00000004, &result)) {
							sub_item_type = (VSSItemType)result.lVal;
						}

						// Recursively 'get' items for this subitem
						if (sub_item_type == VSSITEM_PROJECT) {
							Get_Recursive (sub_item_path, psub_item);
						} else {
							Get (sub_item_path, psub_item);
						}

						// Release our hold on this subitem's interface
						COM_RELEASE (psub_item);
					}
				}				
			}
		}

		// Release our hold on the interface
		COM_RELEASE (pitems);
	}

	return ;
}


///////////////////////////////////////////////////////////////////
//
//	Check_Out_Ex
//
///////////////////////////////////////////////////////////////////
bool
VSSClass::Check_Out_Ex (LPCTSTR local_filename, HWND parent_wnd)
{
	bool retval = false;

	//
	//	Determine the checkout status of the file
	//
	CString user_name;
	VSSFileStatus status = Get_File_Status (local_filename, user_name.GetBufferSetLength (64), 64);
	if (status == VSSFILE_NOTCHECKEDOUT) {
		
		//
		//	Check out the file
		//
		retval = Retry_Check_Out (local_filename, 10, 250);
		if (retval == false) {
			CString message;
			message.Format ("Unable to check out %s.", local_filename);
			::MessageBox (parent_wnd, message, "VSS Error", MB_ICONERROR | MB_OK);
		}

	} else if (status == VSSFILE_CHECKEDOUT) {
		
		//
		//	Let the user know who has the file checked out.
		//
		CString message;
		message.Format ("User %s already has this file checked out.", user_name);
		::MessageBox (parent_wnd, message, "Checkout Error", MB_ICONEXCLAMATION | MB_OK);
	} else if (status == VSSFILE_CHECKEDOUT_ME) {
		retval = true;
	}
	
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Check_In_Ex
//
///////////////////////////////////////////////////////////////////
bool
VSSClass::Check_In_Ex (LPCTSTR local_filename, HWND parent_wnd)
{
	bool retval = false;

	//
	//	Make sure the file is checked out to the current user
	//
	if (Get_File_Status (local_filename) == VSSFILE_CHECKEDOUT_ME) {

		//
		//	Check in the file
		//		
		retval = Retry_Check_In (local_filename, 10, 250);
		if (retval == false) {
			CString message;
			message.Format ("Unable to check in file: %s.", local_filename);
			::MessageBox (parent_wnd, message, "VSS Error", MB_ICONERROR | MB_OK);
		}

	} else if (Does_File_Exist (local_filename) == false) {

		//
		//	If the file did not exist, then add it to the database.
		//
		retval = Add_File (local_filename);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get_File_Status
//
///////////////////////////////////////////////////////////////////
AssetDatabaseClass::FILE_STATUS
VSSClass::Get_File_Status (LPCTSTR local_filename, StringClass *checked_out_user_name)
{
	AssetDatabaseClass::FILE_STATUS retval = AssetDatabaseClass::UNKNOWN;

	LPTSTR username		= NULL;
	DWORD buffer_size		= 0;

	if (checked_out_user_name != NULL) {
		username		= checked_out_user_name->Get_Buffer (64);
		buffer_size	= 64;
	}

	//
	//	Lookup the vss status of this file
	//
	VSSFileStatus vss_status = Get_File_Status (local_filename, username, buffer_size);

   //
	//	Map the return value to a generic
	//
	if (vss_status == VSSFILE_CHECKEDOUT_ME) {
		retval = AssetDatabaseClass::CHECKED_OUT_TO_ME;
	} else if (vss_status == VSSFILE_NOTCHECKEDOUT) {
		retval = AssetDatabaseClass::NOT_CHECKED_OUT;
	} else if (vss_status == VSSFILE_CHECKEDOUT) {
		retval = AssetDatabaseClass::CHECKED_OUT;
	}

	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Get_File
//
///////////////////////////////////////////////////////////////////
FileClass *
VSSClass::Get_File (LPCTSTR local_filename)
{
	//
	//	Do this to get around an oddity of the class when you pass in
	// the filename to the constructor
	//
	RawFileClass *file_obj = new RawFileClass;
	file_obj->Set_Name (local_filename);
	return file_obj;
}

