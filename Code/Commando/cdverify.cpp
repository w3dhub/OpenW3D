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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/cdverify.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/03/02 9:57a                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "cdverify.h"
#include "wwstring.h"
#include "popupdialog.h"
#include "renegadedialog.h"
#include <algorithm>


//////////////////////////////////////////////////////////////////////
//	Local constants
//////////////////////////////////////////////////////////////////////
static const char *RENEGADE_MOVIES_VOLUME	= "Renegade Data";


////////////////////////////////////////////////////////////////
//
//	CDVerifyDialogClass
//
////////////////////////////////////////////////////////////////
class CDVerifyDialogClass : public PopupDialogClass
{
public:
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	CDVerifyDialogClass (void)	:
		Object (NULL),
		PopupDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CDVERIFY)) {}

	void	On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void	Set_Object (CDVerifyClass *object)	{ Object = object; }

private:
	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	CDVerifyClass *	Object;
};


//////////////////////////////////////////////////////////////////////
//
//	Get_CD_Path
//
//////////////////////////////////////////////////////////////////////
bool
CDVerifyClass::Get_CD_Path (StringClass &drive_path)
{
	bool retval = false;

	char buffer[1024] = { 0 };
	::GetLogicalDriveStringsA (sizeof (buffer), buffer);

	//
	//	Loop over all the drives
	//
	const char *drive_root_name = buffer;
	while (drive_root_name[0] != 0) {

		//
		//	Only check CD drives
		//
		if (::GetDriveTypeA (drive_root_name) == DRIVE_CDROM) {

			//
			//	Get the name of this volume
			//
			char volume_name[256] = { 0 };
			if (::GetVolumeInformationA (drive_root_name, volume_name, sizeof (volume_name),
						NULL, NULL, NULL, NULL, 0))
			{
				size_t cmp_len = ::strlen (volume_name);
				cmp_len = std::max<size_t> (cmp_len, static_cast<size_t>(11));

				//
				//	Is this the movies CD?
				//
				if (::strnicmp (volume_name, RENEGADE_MOVIES_VOLUME, cmp_len) == 0) {
					retval		= true;
					drive_path	= drive_root_name;
					break;
				}
			}
		}

		//
		//	Advance to the next drive
		//
		drive_root_name += ::strlen (drive_root_name) + 1;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_CD_Path
//
//////////////////////////////////////////////////////////////////////
void
CDVerifyClass::Display_UI (Observer<CDVerifyEvent> *observer)
{
	AddObserver (*observer);

	//
	//	Display the dialog
	//
	CDVerifyDialogClass *dialog = new CDVerifyDialogClass;
	dialog->Set_Object (this);
	dialog->Start_Dialog ();
	REF_PTR_RELEASE (dialog);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
CDVerifyDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	//
	//	Check to see if the CD is in the drive now...
	///
	if (ctrl_id == IDOK) {
		StringClass path;
		if (Object->Get_CD_Path (path)) {

			//
			//	Notify anybody who cares that the CD is in the drive
			//
			CDVerifyEvent event (CDVerifyEvent::VERIFIED, Object);
			Object->NotifyObservers (event);

			//
			//	Close the dialog
			//
			End_Dialog ();
		}
	} else if (ctrl_id == IDCANCEL) {
		
		//
		//	Notify anybody who cares that the user has cancelled the operation
		//
		CDVerifyEvent event (CDVerifyEvent::NOT_VERIFIED, Object);
		Object->NotifyObservers (event);

		//
		//	Close the dialog
		//
		End_Dialog ();		
	}

	PopupDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}
