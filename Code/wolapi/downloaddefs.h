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

#ifndef _DOWNLOADDEFS_H
#define _DOWNLOADDEFS_H

#include "wolcommondefs.h"

// CDownload statuses

#define DOWNLOADSTATUS_NONE				0
#define DOWNLOADSTATUS_GO				1
#define DOWNLOADSTATUS_CONNECTING		2
#define DOWNLOADSTATUS_LOGGINGIN		3
#define DOWNLOADSTATUS_FINDINGFILE		4
#define DOWNLOADSTATUS_QUERYINGRESUME	5
#define DOWNLOADSTATUS_DOWNLOADING		6
#define DOWNLOADSTATUS_DISCONNECTING	7
#define	DOWNLOADSTATUS_DONE				0

// CDownload return codes

#define DOWNLOAD_SUCCEEDED		S_OK
#define	DOWNLOAD_PARAMERROR		WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 1 )
#define DOWNLOAD_STATUSERROR	WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 2 )
#define DOWNLOAD_NETWORKERROR	WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 3 )
#define DOWNLOAD_FILEERROR		WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 4 )
#define DOWNLOAD_REENTERERROR	WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 4 )

// CDownloadEvent return codes.

#define DOWNLOADEVENT_SUCCEEDED		WOLAPI_OK
#define	DOWNLOADEVENT_FAILED		WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 5 )
#define	DOWNLOADEVENT_RESUME		WOLAPI_OK
#define	DOWNLOADEVENT_DONOTRESUME	WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 6 )

// CDownloadEvent error codes for OnError()

#define DOWNLOADEVENT_NOSUCHSERVER			1
#define DOWNLOADEVENT_COULDNOTCONNECT		2
#define DOWNLOADEVENT_LOGINFAILED			3
#define DOWNLOADEVENT_NOSUCHFILE			4
#define DOWNLOADEVENT_LOCALFILEOPENFAILED	5
#define DOWNLOADEVENT_TCPERROR				6
#define DOWNLOADEVENT_DISCONNECTERROR		7

#endif
