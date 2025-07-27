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
 *                     $Archive:: /Commando/Code/Commando/bandwidth.cpp                       $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/18/02 9:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "bandwidth.h"  // I WANNA BE FIRST!

#include "miscutil.h"
#include "wwdebug.h"
#include "translatedb.h"
#include "string_ids.h"
#include "useroptions.h"

#include "bandwidthcheck.h"

//-----------------------------------------------------------------------------
ULONG cBandwidth::Get_Bandwidth_Bps_From_Type(BANDWIDTH_TYPE_ENUM bandwidth_type)
{
	/*
	WWASSERT(bandwidth_type >= BANDWIDTH_FIRST &&
		      bandwidth_type <= BANDWIDTH_LAST);
	*/

//	WWASSERT(bandwidth_type != BANDWIDTH_CUSTOM);

	switch (bandwidth_type) {
		case BANDWIDTH_CUSTOM:
			return cUserOptions::BandwidthBps.Get();
		case BANDWIDTH_MODEM_288:
			return 28800;
		case BANDWIDTH_MODEM_336:
			return 33600;
		case BANDWIDTH_MODEM_56:
			return 56000;
		case BANDWIDTH_ISDN:
			return 64000;
		case BANDWIDTH_CABLE:
			return 128000;
		case BANDWIDTH_LANT1:
			return 2000000;
		case BANDWIDTH_AUTO:
		{
			ULONG bps = BandwidthCheckerClass::Get_Upstream_Bandwidth();
//			WWASSERT(bps > 0);
			return bps;
		}
		default:
			DIE;
			return 0xffffffff; // to avoid compiler warning
   }
}

//-----------------------------------------------------------------------------
const wchar_t *cBandwidth::Get_Bandwidth_String_From_Type(BANDWIDTH_TYPE_ENUM bandwidth_type)
{
	static char _bandwidth_auto_txt[128];
	static WideStringClass s;
	/*
	WWASSERT(bandwidth_type >= BANDWIDTH_FIRST &&
				bandwidth_type <= BANDWIDTH_LAST);
	*/

	switch (bandwidth_type) {
		case BANDWIDTH_MODEM_288:
			return TRANSLATE(IDS_MP_CONNECTION_288);		//"BANDWIDTH_MODEM_288";
		case BANDWIDTH_MODEM_336:
			return TRANSLATE(IDS_MP_CONNECTION_336);		//"BANDWIDTH_MODEM_336";
		case BANDWIDTH_MODEM_56:
			return TRANSLATE(IDS_MP_CONNECTION_56);		//"BANDWIDTH_MODEM_56";
		case BANDWIDTH_ISDN:
			return TRANSLATE(IDS_MP_CONNECTION_ISDN);		//"BANDWIDTH_ISDN";
		case BANDWIDTH_CABLE:
			return TRANSLATE(IDS_MP_CONNECTION_CABLE);	//"BANDWIDTH_CABLE";
		case BANDWIDTH_LANT1:
			return TRANSLATE(IDS_MP_CONNECTION_T1);		//"BANDWIDTH_LANT1";
		case BANDWIDTH_CUSTOM:
			return L"BANDWIDTH_CUSTOM";
		case BANDWIDTH_AUTO:
		{
			//sprintf(_bandwidth_auto_txt, "BANDWIDTH_AUTO (%s)", BandwidthCheckerClass::Get_Bandwidth_As_String());
			s.Format(TRANSLATE(IDS_MP_CONNECTION_T1), Get_Bandwidth_Bps_From_Type(bandwidth_type));
			//wsprintf(_bandwidth_auto_txt, TRANSLATE(IDS_MP_CONNECTION_T1), Get_Bandwidth_Bps_From_Type(bandwidth_type));
			//return ((const)_bandwidth_auto_txt);
			return(s);
		}
		default:
			DIE;
			return L"ERROR"; // to avoid compiler warning
   }
}

//-----------------------------------------------------------------------------
BANDWIDTH_TYPE_ENUM cBandwidth::Get_Bandwidth_Type_From_String(LPCSTR bandwidth_string)
{
	WWASSERT(bandwidth_string != NULL);

   if (cMiscUtil::Is_String_Same(bandwidth_string, "BANDWIDTH_MODEM_288")) {
      return BANDWIDTH_MODEM_288;
   } else if (cMiscUtil::Is_String_Same(bandwidth_string, "BANDWIDTH_MODEM_336")) {
      return BANDWIDTH_MODEM_336;
   } else if (cMiscUtil::Is_String_Same(bandwidth_string, "BANDWIDTH_MODEM_56")) {
      return BANDWIDTH_MODEM_56;
   } else if (cMiscUtil::Is_String_Same(bandwidth_string, "BANDWIDTH_ISDN")) {
      return BANDWIDTH_ISDN;
   } else if (cMiscUtil::Is_String_Same(bandwidth_string, "BANDWIDTH_CABLE")) {
      return BANDWIDTH_CABLE;
   } else if (cMiscUtil::Is_String_Same(bandwidth_string, "BANDWIDTH_LANT1")) {
      return BANDWIDTH_LANT1;
   } else if (cMiscUtil::Is_String_Same(bandwidth_string, "BANDWIDTH_CUSTOM")) {
      return BANDWIDTH_CUSTOM;
   } else if (strnicmp(bandwidth_string, "BANDWIDTH_AUTO ", 15) == 0) {
      return BANDWIDTH_AUTO;
   } else {
      DIE;
		return BANDWIDTH_CUSTOM; // to avoid compiler warning
   }
}