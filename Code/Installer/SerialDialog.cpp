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
 *                 Project Name : Installer                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Installer/SerialDialog.cpp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/15/02 11:09a                $* 
 *                                                                                             * 
 *                    $Revision:: 10                    $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "SerialDialog.h"
#include "CodeControl.h"
#include "editctrl.h"
#include "Installer.h"
#include "resource.h"
#include "Translator.h"


// Defines.
#define EDIT_CONTROL_COUNT	4


// Forward declarations.
unsigned int Check_Sum (char *str);


// Static data.
static const int _EditControlIds		 [EDIT_CONTROL_COUNT] = {IDC_SERIAL_EDIT1,
																				 IDC_SERIAL_EDIT2,
																				 IDC_SERIAL_EDIT3,
																				 IDC_SERIAL_EDIT4};

static const int _EditControlLengths [EDIT_CONTROL_COUNT] = {6, 6, 6, 4};

static char *_SerialNumberLowRange;
static char *_SerialNumberHighRange;


/***********************************************************************************************
 * SerialDialogClass::On_Init_Dialog --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void SerialDialogClass::On_Init_Dialog (void)
{
	if (!_Installer.Beta_Test()) {
		#if USE_QA_SERIAL_NOS
		_SerialNumberLowRange  = "0661";
		_SerialNumberHighRange = "0661";
		#else
		_SerialNumberLowRange  = "0559";
		_SerialNumberHighRange = "0660";
		#endif
	} else {
		_SerialNumberLowRange  = "0663";
		_SerialNumberHighRange = "0663";
	}

	#if NDEBUG
	#if USE_QA_SERIAL_NOS
	Set_Dlg_Item_Text (IDC_SERIAL_EDIT1, WideStringClass (_SerialNumberLowRange));
	#endif
	#else
	// Initialize with a valid serial number.
	if (!_Installer.Beta_Test()) {
		#if USE_QA_SERIAL_NOS
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT1, WideStringClass ("066146"));
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT2, WideStringClass ("321463"));
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT3, WideStringClass ("028258"));
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT4, WideStringClass ("8591"));
		#else
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT1, WideStringClass ("065906"));
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT2, WideStringClass ("044792"));
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT3, WideStringClass ("767144"));
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT4, WideStringClass ("4915"));
		#endif
	} else {
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT1, WideStringClass ("066390"));
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT2, WideStringClass ("274825"));
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT3, WideStringClass ("508134"));
		Set_Dlg_Item_Text (IDC_SERIAL_EDIT4, WideStringClass ("6328"));
	}
	#endif

	InstallMenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * SerialDialogClass::On_Unicode_Char --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void SerialDialogClass::On_Unicode_Char (uint16 unicode)
{

	// Scan the edit controls for input focus...
	for (unsigned i = 0; i < EDIT_CONTROL_COUNT; i++) {

		EditCtrlClass *editcontrol = Get_Dlg_Item (_EditControlIds [i])->As_EditCtrlClass();

		// Does this control have focus?
		if (editcontrol->Has_Focus()) {

			int textlength = wcslen (editcontrol->Get_Text());

			// Is this key one of the edit keys?
			switch (unicode) {

				case VK_BACK:
					if (textlength == 0) {
						if (i > 0) {
						 
							EditCtrlClass *preveditcontrol = Get_Dlg_Item (_EditControlIds [i - 1])->As_EditCtrlClass();
							preveditcontrol->Set_Focus();
							preveditcontrol->Set_Sel (-1, -1);
						}
					
					} else {
						InstallMenuDialogClass::On_Unicode_Char (unicode);
					}
					break;

				case VK_DELETE:
				case VK_HOME:
				case VK_END:
				case VK_LEFT:
				case VK_RIGHT:
				case VK_RETURN:
				case VK_UP:
				case VK_DOWN:
				case VK_TAB:

					// Process the key.
					InstallMenuDialogClass::On_Unicode_Char (unicode);
					break;

				default:
					
					// Is it a numeric key?
					if ((unicode >= L'0') && (unicode <= L'9')) {

						if (textlength < _EditControlLengths [i]) {
							
							// Process the key.
							InstallMenuDialogClass::On_Unicode_Char (unicode);
							textlength = wcslen (editcontrol->Get_Text());

							// If the edit control is now full and the caret is at the end...
							if ((textlength == _EditControlLengths [i]) && (textlength == editcontrol->Get_Caret_Pos())) {

								// If the next edit control is empty switch focus to it.
								if (i < EDIT_CONTROL_COUNT - 1) {

									EditCtrlClass *nexteditcontrol = Get_Dlg_Item (_EditControlIds [i + 1])->As_EditCtrlClass();

									if (wcslen (nexteditcontrol->Get_Text()) == 0) {
										nexteditcontrol->Set_Focus();
									}
								}
							}

						} else {

	 						if ((textlength == _EditControlLengths [i]) && (textlength == editcontrol->Get_Caret_Pos())) {
							
								// If the next edit control is empty switch focus to it and process the key.
								if (i < EDIT_CONTROL_COUNT - 1) {

									EditCtrlClass *nexteditcontrol = Get_Dlg_Item (_EditControlIds [i + 1])->As_EditCtrlClass();

									if (wcslen (nexteditcontrol->Get_Text()) == 0) {
										nexteditcontrol->Set_Focus();
										InstallMenuDialogClass::On_Unicode_Char (unicode);
									}
								}
							}
						}
					}
			}
			break;
		}
	}
}


/***********************************************************************************************
 * SerialDialogClass::On_Command --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void SerialDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	bool valid = true;

	switch (ctrl_id) {

		case IDOK:
		{
			StringClass serialnumber;

			valid = Get_Serial_Number (serialnumber);
			if (valid) {

				#if NDEBUG
				StringClass lowrange  (_SerialNumberLowRange);
				StringClass highrange (_SerialNumberHighRange);
			
				// Limit range of serial number.
				valid = !(((_strnicmp (serialnumber, lowrange, 4) < 0) || (_strnicmp (serialnumber, highrange, 4) > 0)));
				#endif

				if (valid) {

					unsigned i, v;
					
					// Scan the string for all zeros.
					v = 0;
					for (i = 0; i < (unsigned) serialnumber.Get_Length(); i++) {
						v += *(serialnumber.Peek_Buffer() + i) - '0';
					}
					valid = (v > 0);

					if (valid) {
	
						const unsigned offset = 18;

						StringClass checksumstring (serialnumber);
						unsigned		checksum;

						// Apply checksum test.
						*(checksumstring.Peek_Buffer() + offset) = '\0';
						checksum = Check_Sum (checksumstring.Peek_Buffer());
						valid = (checksum == (uint32) atol (serialnumber.Peek_Buffer() + offset));
					}
				}
			}
			break;
		}

		default:
			// Do nothing.
			break;
	}
	
	if (valid) {
		InstallMenuDialogClass::On_Command (ctrl_id, message_id, param);
	} else {
		MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_INCORRECT_SERIAL), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
	}
}


/***********************************************************************************************
 * SerialDialogClass::Get_Serial_Number --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool SerialDialogClass::Get_Serial_Number (StringClass &serialnumber)
{
	bool valid = false;

	for (unsigned i = 0; i < EDIT_CONTROL_COUNT; i++) {

		EditCtrlClass *editcontrol = Get_Dlg_Item (_EditControlIds [i])->As_EditCtrlClass();

		// Extract the serial number.
		WideStringClass widedigits (editcontrol->Get_Text());
		StringClass		 digits;		

		// Ensure that the text contains correct no. of characters.
		// NOTE: Non-numeric characters have already been filtered out, so no need to check for this.
		valid = (widedigits.Get_Length() == _EditControlLengths [i]);
		if (!valid) {
			break;
		}
		widedigits.Convert_To (digits);
		serialnumber += digits;
	}

	return (valid);
}


/***********************************************************************************************
 * SerialDialogClass::CheckSum -- Generate a checksum number between 0...9999 from a null		  *
 *											 terminated string of numbers.										  *
 *																															  *
 * INPUT:		none.																									  *
 *																															  *
 * OUTPUT:		none.																									  *
 *																															  *
 * WARNINGS:																											  *
 *																															  *
 *		The checksum is divided into 2 parts.  The first is computed by grabbing 7 digits at a   *
 *		grabbing 7 digits at a time and summing them mod 97. The second part is computed by		  *
 *		grabbing 5 digits at a time and The second part is computed by grabbing 5 digits at a	  *
 *		time and	summing them mod 93.																				  *
 *																															  *
 *		Note that it's important that 97 & 93 are prime (or at least relatively prime...) and	  *
 *		less than 100.	7 & 5 were chosen because they can stay out of phase for the length of	  *
 *		the input string.																								  *
 *																															  *
 *		The two components together make the checksum. This will catch	all errors of 1 & 2		  *
 *		digits and many other errors, but the proof is left as an exercise to the reader.		  *
 *																															  *
 *		Note, if you try and checksum ammounts over 30 chars the above	guarantees will fail.	  *
 *		(That's the number of chars that the skip	& offset values will stay out of phase).		  *														  *
 *																															  *
 *		Note: The checksum of all zeros will be 0 so you need to special case the check to not	  *
 *		allow this (since it's a pretty obvious thing for a user to type all 0's to try and		  *
 *		cheat the system).																							  *
 *																															  *
 * HISTORY:																												  *
 *		10/30/1995	MML : Created.																					  *
 *=============================================================================================*/
unsigned int Check_Sum (char *str)
{
	unsigned int counter	= 0;
	unsigned int retval	= 0;
	unsigned int group	= 0;
	int	len	= strlen( str );
	int	mult	= 0;
	int	i;
	int	j;
 
	const int skipa	=  7;		// part A's skip value
	const int skipb	=  5;		// ... part B
	const int offsa	= -5;		// Everywhere the skips are in phase is a place where an 2nd error can occur and not be detected.

	//---------------------------------------------------------------------------
	// Part1
	//---------------------------------------------------------------------------
	counter = 0;
	for ( i = offsa; i < len; i += skipa ) {

		mult = 1;
		for ( j = 1; j < skipa; j++ ) {
			mult *= 10;
		}
 
		group = 0;
		for ( j = i; j < MIN( len, i+skipa ); j++ ) {

			if ( j >= 0 ) {
				group += ((int)( str[j] - '0' )) * mult;
			}
			mult /= 10;
		}
		counter += group;
		counter %= 97;
	}
	retval = counter * 100;
 
	//---------------------------------------------------------------------------
	// Part2
	//---------------------------------------------------------------------------
	counter = 0;
	for ( i = 0; i < len; i += skipb ) {

		mult = 1;
		for ( j = 1; j < skipb; j++ ) {
			mult *= 10;
		}
 
		group = 0;
		for ( j = i; j < MIN( len, i+skipb ); j++ ) {

			if ( j >= 0 ) {
				group += ((int)( str[j] - '0' )) * mult;
			}
         mult /= 10;
    }
    counter += group;
    counter %= 93;
  }
  retval += counter;
 
  return (retval);
}
