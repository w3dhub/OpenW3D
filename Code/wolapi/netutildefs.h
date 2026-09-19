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

#ifndef NETUTILDEFS_HEADER
#define NETUTILDEFS_HEADER

#include "wolcommondefs.h"

#define NETUTIL_E_ERROR       WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 100)
#define NETUTIL_E_BUSY        WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 101)
#define NETUTIL_E_TIMEOUT     WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 102)

#define NETUTIL_E_INVALIDFIELD  WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 256)
#define NETUTIL_E_CANTVERIFY    WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_ERROR, WOLAPI_FACILITY_ITF, 257)

#define NETUTIL_S_FINISHED    WOLAPI_MAKE_HRESULT( WOLAPI_SEVERITY_SUCCESS, WOLAPI_FACILITY_ITF, 500)

#endif
