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

/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Oct 22 14:51:58 2001
 */
/* Compiler settings for .\Code\WOLBrowser.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_IWOLBrowserEvent = {0x28CC9F2D,0x1F5F,0x4d3f,{0xA8,0xC6,0x0C,0x8C,0x21,0x72,0xD6,0x8D}};


const IID IID_IWOLBrowser = {0x0315484A,0xDC04,0x47a5,{0x8F,0x50,0xCD,0xB7,0x40,0x24,0x25,0xC5}};


const IID IID_IWOLBrowserScript = {0x4A992E36,0x766A,0x4ff9,{0xAF,0x04,0x85,0x41,0xDC,0x43,0xB2,0xDD}};


const IID LIBID_WOLBrowserLib = {0x3F326056,0x5B6D,0x4ECB,{0x86,0x03,0xB3,0x61,0x16,0xD3,0xF8,0x57}};


const CLSID CLSID_WOLBrowser = {0xE6F33E57,0xC630,0x4EAF,{0xAA,0x4A,0x43,0xF3,0xA5,0x9E,0xA6,0x08}};


#ifdef __cplusplus
}
#endif

