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
 *                     $Archive:: /Commando/Code/Combat/diaglog.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/02/02 3:35p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 5                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "diaglog.h"
#include "ffactory.h"
#include "wwfile.h"
#include "timemgr.h"

#include <wtypes.h>	// for SYSTEMTIME

FileClass * _DiagLogFile = NULL;

/*
**
*/
void	DiagLogClass::Init( void )
{
	FileClass * file = NULL;
	for ( int i = 0; i < 100; i++ ) {
		StringClass	filename;
		filename.Format( "DIAGLOG%03d.TXT", i );
		file = _TheWritingFileFactory->Get_File( filename );
		if ( file == NULL || !file->Is_Available() ) {
			break;
		}
		_TheWritingFileFactory->Return_File( file );
		file = NULL;
	}

	if ( file != NULL ) {
		file->Close();
		file->Open(FileClass::WRITE);
		_DiagLogFile = file;
	}

	SYSTEMTIME dt;
	::GetSystemTime( &dt );
	StringClass dt_string;
	dt_string.Format( "%02d/%02d/%02d %02d:%02d:%02d", dt.wMonth, dt.wDay, dt.wYear, dt.wHour, dt.wMinute, dt.wSecond );
	DIAG_LOG(( "OPEN", "%s", (const char *)dt_string ));
}

void	DiagLogClass::Shutdown( void )
{
	if ( _DiagLogFile != NULL ) {
		SYSTEMTIME dt;
		::GetSystemTime( &dt );
		StringClass dt_string;
		dt_string.Format( "%02d/%02d/%02d %02d:%02d:%02d", dt.wMonth, dt.wDay, dt.wYear, dt.wHour, dt.wMinute, dt.wSecond );
		DIAG_LOG(( "CLOS", "%s", (const char *)dt_string ));

		_DiagLogFile->Close();
		_TheWritingFileFactory->Return_File( _DiagLogFile );
		_DiagLogFile = NULL;
	}
}

void	DiagLogClass::Log_Timed( const char * type, const char * format, ... )
{
	if ( _DiagLogFile != NULL ) {

		va_list arg_list;
		va_start (arg_list, format);
		StringClass data;
		data.Format_Args( format, arg_list );
		va_end (arg_list);

		StringClass line;
		float time = TimeManager::Get_Total_Seconds();
		line.Format( "%s; %1.2f; %s%c%c", type, time, data, 0x0D, 0x0A );
		_DiagLogFile->Write( line, ::strlen( line ) );
	}
}

