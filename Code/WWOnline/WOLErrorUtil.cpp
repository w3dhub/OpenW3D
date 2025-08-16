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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/WWOnline/WOLErrorUtil.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 6 $
*     $Modtime: 1/11/02 2:23p $
*
******************************************************************************/

#include "WOLErrorUtil.h"
#include <wolapi/ChatDefs.h>
#include <wolapi/NetUtilDefs.h>
#include <wolapi/DownloadDefs.h>

namespace WWOnline {

#define ADD_CASE(exp, msg) case exp: return msg;

/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

const char* const GetChatErrorString(HRESULT hr)
	{
	switch (hr)
		{
		ADD_CASE(CHAT_E_NICKINUSE, "WOL_NICKINUSE");
		ADD_CASE(CHAT_E_BADPASS, "WOL_BADPASSWORD");
		ADD_CASE(CHAT_E_NONESUCH, "WOL_NONESUCH");
		ADD_CASE(CHAT_E_CON_NETDOWN, "WOL_NETDOWN");
		ADD_CASE(CHAT_E_CON_LOOKUP_FAILED, "WOL_FAILEDLOOKUP");
		ADD_CASE(CHAT_E_CON_ERROR, "WOL_CONNECTERROR");
		ADD_CASE(CHAT_E_TIMEOUT, "WOL_TIMEDOUT");
		ADD_CASE(CHAT_E_MUSTPATCH, "WOL_PATCHREQUIRED");
		ADD_CASE(CHAT_E_STATUSERROR, "WOL_STATUSERROR");
		ADD_CASE(CHAT_E_UNKNOWNRESPONSE, "WOL_UNKNOWNERROR");
		ADD_CASE(CHAT_E_CHANNELFULL, "WOL_CHANNELFULL");
		ADD_CASE(CHAT_E_CHANNELEXISTS, "WOL_CHANNELCREATEERROR");
		ADD_CASE(CHAT_E_CHANNELDOESNOTEXIST, "WOL_CHANNELNOTFOUND");
		ADD_CASE(CHAT_E_BADCHANNELPASSWORD, "WOL_BADPASSWORD");
		ADD_CASE(CHAT_E_BANNED, "WOL_BANNED");
		ADD_CASE(CHAT_E_NOT_OPER, "WOL_NOTCHANOP");
		ADD_CASE(CHAT_E_DISABLED, "WOL_ACCOUNTDISABLED");
		ADD_CASE(CHAT_E_SERIALBANNED, "WOL_SERIALBANNED");
		ADD_CASE(CHAT_E_SERIALDUP, "WOL_SERIALDUPLICATE");
		ADD_CASE(CHAT_E_SERIALUNKNOWN, "WOL_SERIALUNKNOWN");
		ADD_CASE(CHAT_E_SKUSERIALMISMATCH, "WOL_SERIALSKUMISMATCH");
		ADD_CASE(CHAT_E_NOTCONNECTED, "WOL_NOTCONNECTED");
		ADD_CASE(CHAT_E_NOCHANNEL, "WOL_CHANNELNOTFOUND");
		ADD_CASE(CHAT_E_NOTIMPLEMENTED, "WOL_NOTIMPLEMENTED");
		ADD_CASE(CHAT_E_PENDINGREQUEST, "WOL_PENDINGREQUEST");
		ADD_CASE(CHAT_E_PARAMERROR, "WOL_PARAMERROR");
		ADD_CASE(CHAT_E_LEAVECHANNEL, "WOL_CHANNELLEAVEERROR");
		ADD_CASE(CHAT_E_JOINCHANNEL, "WOL_CHANNELJOINERROR");
		ADD_CASE(CHAT_E_UNKNOWNCHANNEL, "WOL_CHANNELNOTFOUND");

		default: 
			return "WOL_UNKNOWNERROR";
			break;
		}
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

const char* const GetNetUtilErrorString(HRESULT hr)
	{
	switch (hr)
		{
		ADD_CASE(NETUTIL_E_ERROR, "WOL_ERROR");
		ADD_CASE(NETUTIL_E_BUSY, "WOL_BUSY");
		ADD_CASE(NETUTIL_E_TIMEOUT, "WOL_TIMEDOUT");
		ADD_CASE(NETUTIL_E_INVALIDFIELD, "WOL_PARAMERROR");
		ADD_CASE(NETUTIL_E_CANTVERIFY, "WOL_CANTVERIFY");

		default: 
			return "WOL_UNKNOWNERROR";
   }
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

const char* const GetDownloadErrorString(HRESULT hr)
	{
	switch (hr)
		{
		ADD_CASE(DOWNLOADEVENT_NOSUCHSERVER, "WOL_DOWNLOADNOSERVER");
		ADD_CASE(DOWNLOADEVENT_COULDNOTCONNECT, "WOL_DOWNLOADCONNECTERROR");
		ADD_CASE(DOWNLOADEVENT_LOGINFAILED, "WOL_DOWNLOADLOGINERROR");
		ADD_CASE(DOWNLOADEVENT_NOSUCHFILE, "WOL_DOWNLOADFNF");
		ADD_CASE(DOWNLOADEVENT_LOCALFILEOPENFAILED, "WOL_DOWNLOADOPENERROR");
		ADD_CASE(DOWNLOADEVENT_TCPERROR, "WOL_DOWNLOADIOERROR");
		ADD_CASE(DOWNLOADEVENT_DISCONNECTERROR, "WOL_DOWNLOADDISCONNECTERROR");

		default: 
			return "WOL_UNKNOWNERROR";
		}
	}

}
