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
 *                     $Archive:: /Commando/Code/Combat/specialbuilds.h                           $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/18/02 11:54a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef	SPECIALBUILDS_H
#define	SPECIALBUILDS_H

//
// N.B. Any file that references any of the following special build defines
// must include this file.
//

//-----------------------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------------------

// Uncomment the BETASERVER define to build a BETASERVER
// this is actually the same as a FULL build, just points to 
// different registry entries
//
// #define BETASERVER


//
// Uncomment the BETACLIENT define to omit single-player and
// hosting functionality.
//
// #define BETACLIENT


//
// Uncomment the FREEDEDICATEDSERVER define to permit only dedicated
// hosting.
//
// #define FREEDEDICATEDSERVER


//
// Uncomment the MULTIPLAYERDEMO define to build the MP demo.
//
// #define MULTIPLAYERDEMO


//-----------------------------------------------------------------------------
// COMPILE-TIME WARNINGS
//-----------------------------------------------------------------------------

#ifdef BETASERVER
// FIXME *** BETASERVER is defined ***
#endif // BETACLIENT

#ifdef BETACLIENT
// FIXME *** BETACLIENT is defined ***
#endif // BETACLIENT

#ifdef FREEDEDICATEDSERVER
// FIXME *** FREEDEDICATEDSERVER is defined ***
#endif // FREEDEDICATEDSERVER

#ifdef MULTIPLAYERDEMO
// FIXME *** MULTIPLAYERDEMO is defined ***
#endif // MULTIPLAYERDEMO

//
// Disallow specialbuild defines to be used together.
//
#if defined(BETASERVER) && defined(FREEDEDICATEDSERVER)
#error "*** ERROR: specialbuild defines should be mutually exclusive.***"
#endif

#if defined(BETASERVER) && defined(MULTIPLAYERDEMO)
#error "*** ERROR: specialbuild defines should be mutually exclusive.***"
#endif

#if defined(BETASERVER) && defined(BETACLIENT)
#error "*** ERROR: specialbuild defines should be mutually exclusive.***"
#endif

#if defined(BETACLIENT) && defined(FREEDEDICATEDSERVER)
#error "*** ERROR: specialbuild defines should be mutually exclusive.***"
#endif

#if defined(BETACLIENT) && defined(MULTIPLAYERDEMO)
#error "*** ERROR: specialbuild defines should be mutually exclusive.***"
#endif

#if defined(FREEDEDICATEDSERVER) && defined(MULTIPLAYERDEMO)
#error "*** ERROR: specialbuild defines should be mutually exclusive.***"
#endif
//-----------------------------------------------------------------------------

#endif // SPECIALBUILDS_H
















/*
//
// 11/07/01 - BETASERVER define no longer needed.
//
// If you uncomment the BETASERVER define, the server will perform exe matching based only
// on the exe resource version number and the strings.tdb internal version number.
// This is a less stringent compatibility test than normal.
// Do not use unless absolutely necessary.
// Only compatible with clients that have BETACLIENT defined.
//
//#define BETASERVER
*/


/*
#ifdef BETASERVER
// FIXME *** BETASERVER is defined ***
#endif // BETASERVER
*/

// Define BETASERVER to create a server and then define BETACLIENT to create a client.