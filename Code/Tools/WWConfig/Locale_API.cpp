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

/*****************************************************************************
*   C O N F I D E N T I A L --- W E S T W O O D   A S S O C I A T E S		 *
******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Tools/WWConfig/Locale_API.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Ian_l $
*
* VERSION INFO
*     $Modtime: 1/09/02 11:02a $
*     $Revision: 2 $
*
*****************************************************************************/

#include <windows.h>
#include <wtypes.h>
#include "locale.h"
#include "locale_api.h"
#include "rawfile.h"
#include "wwconfig_ids.h"


/****************************************************************************/
/* DEFINES			                                                        */
/****************************************************************************/

//----------------------------------------------------------------------------
// NOTE:	if USE_MULTI_FILE_FORMAT is "true", then a .lOC file must be in 
//			the same directory as this file.
//----------------------------------------------------------------------------
#define		USE_MULTI_FILE_FORMAT		FALSE

#define		LANGUAGE_IS_DBCS(l)	(((l)==IDL_JAPANESE)||((l)==IDL_KOREAN)||((l)==IDL_CHINESE))		// [OYO]
#define		CODEPAGE_IS_DBCS(C)	((C==932)||(C==949)||(C==950))										// [OYO]


/****************************************************************************/
/* GLOBAL VARIABLES                                                         */
/****************************************************************************/
CHAR		LanguageFile[ _MAX_PATH ];
void *	LocaleFile		= NULL;
int		CodePage			= GetACP();
int		LanguageID		= 0;

/****************************************************************************/
/* LOCALE API                                                               */
/****************************************************************************/
WCHAR *		Remove_Quotes_Around_String ( WCHAR *old_string );
void *	 	Load_File ( const CHAR *filename, long *filesize );

//=============================================================================
// These are wrapper functions around the LOCALE_ functions.  I made these to 
// make using the single vs. multi language files more transparent to the program.
//=============================================================================

bool Locale_Use_Multi_Language_Files ( void )
{
#if( USE_MULTI_FILE_FORMAT )
	return true;
#else
	return false;
#endif
}


/****************************************************************************/
/* initialization															*/
/****************************************************************************/

int Locale_Init	( int language, const char *file )
{
	int	result = 0;

	//-------------------------------------------------------------------------
	// Check for a file passed in.
	//-------------------------------------------------------------------------
	if( file == NULL || file[0] == '/0' ) {
		return 0;
	}

	//-------------------------------------------------------------------------
	// Initialize settings.
	//-------------------------------------------------------------------------
	strcpy( LanguageFile, file );
	LanguageID = language;

	//-------------------------------------------------------------------------
	// If language == -1, then set based on CodePage.
	//-------------------------------------------------------------------------
	if( LanguageID == -1 ) {

		switch( CodePage ) {

			// Japanese
			case 932:
				LanguageID	= IDL_JAPANESE;
				break;

			// Korean
			case 949:
				LanguageID	= IDL_KOREAN;
				break;

			// Chinese
			case 950:
				LanguageID	= IDL_CHINESE;
				break;

			// English, French, and German
			case 1252:
				{
					LANGID	langid	= GetSystemDefaultLangID();
					WORD	plangid = PRIMARYLANGID( langid );

					switch( plangid ) {

						// German
						case 0x07:
							LanguageID 	= IDL_GERMAN;
							break;

						// French
						case 0x0c:
							LanguageID 	= IDL_FRENCH;
							break;

						// English
						case 0x09:
						default:
							LanguageID	= IDL_ENGLISH;
							break;
					}
				}
				break;
							
			default:
				LanguageID	= IDL_ENGLISH;
				break;
		}
	}

	//-------------------------------------------------------------------------
	// Check for valid LanguageID range.
	//-------------------------------------------------------------------------
	if( LanguageID < 0 || LanguageID >= LOCALE_LANGUAGE_COUNT ) {
		LanguageID = 0;
	}

	//-------------------------------------------------------------------------
	// Use English in the situation where Chinese, Korean, or Japanese was 
	// requested but not available.
	//-------------------------------------------------------------------------
	if( LANGUAGE_IS_DBCS( LanguageID ) && !CODEPAGE_IS_DBCS( CodePage )) {
		LanguageID = IDL_ENGLISH;
	}

	//-------------------------------------------------------------------------
	// Initialize the lx object.
	//-------------------------------------------------------------------------
	LOCALE_init();

	#if( USE_MULTI_FILE_FORMAT )

		//---------------------------------------------------------------------
		// Set bank to use and load the appropriate table.
		//---------------------------------------------------------------------
		LOCALE_setbank(0);
		result = LOCALE_loadtable( LanguageFile, LanguageID );

	#else

		//---------------------------------------------------------------------
		// Create a file buffer that holds all the strings in the file.
		//---------------------------------------------------------------------
		long		filesize;
		HRSRC 		hRsrc;
		HGLOBAL		hGlobal;
		int			PrimaryLanguage = LANG_NEUTRAL;
		int			SubLanguage		= SUBLANG_DEFAULT;
		HMODULE		module			= GetModuleHandle( NULL );

		//-------------------------------------------------------------------------
		// Find the string file in this program's resources.
		//-------------------------------------------------------------------------
		switch( LanguageID ) {

			// Japanese
			case IDL_JAPANESE:
				PrimaryLanguage = LANG_JAPANESE;
				SubLanguage		= SUBLANG_DEFAULT;
				break;

			// Korean
			case IDL_KOREAN:
				PrimaryLanguage = LANG_KOREAN;
				SubLanguage		= SUBLANG_DEFAULT;
				break;

			// Chinese
			case IDL_CHINESE:
				PrimaryLanguage = LANG_CHINESE;
				SubLanguage		= SUBLANG_DEFAULT;
				break;

			// German
			case IDL_GERMAN:
				PrimaryLanguage = LANG_GERMAN;
				SubLanguage		= SUBLANG_GERMAN;
				break;

			// French
			case IDL_FRENCH:
				PrimaryLanguage = LANG_FRENCH;
				SubLanguage		= SUBLANG_FRENCH;
				break;

			// English
			case IDL_ENGLISH:
				PrimaryLanguage = LANG_ENGLISH;
				SubLanguage		= SUBLANG_ENGLISH_US;
				break;

			default:
				PrimaryLanguage = LANG_ENGLISH;
				SubLanguage		= SUBLANG_ENGLISH_US;
				break;
		}

		hRsrc = FindResourceEx( module, RT_RCDATA, "STRINGS", MAKELANGID( PrimaryLanguage, SubLanguage ));
		if ( hRsrc == NULL ) {
			hRsrc = FindResourceEx( module, RT_RCDATA, "STRINGS", MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ));
		}
		if ( hRsrc ) {

			//---------------------------------------------------------------------
			// Load the resource, lock the memory, grab a DC.
			//---------------------------------------------------------------------
			hGlobal		= LoadResource( module, hRsrc );
			filesize	= SizeofResource( module, hRsrc );

			LocaleFile = (void*)malloc( filesize + 1 );
			memset( LocaleFile, '\0', filesize + 1 );
			memcpy( LocaleFile, (const void *)hGlobal, filesize );

			//---------------------------------------------------------------------
			// Free DS and memory used.
			//---------------------------------------------------------------------
			UnlockResource( hGlobal );
			FreeResource( hGlobal );
		}

		if( LocaleFile == NULL ) {
			LocaleFile = Load_File( LanguageFile, &filesize );
		}

		if( LocaleFile != NULL ) {
			result = 1;
		}

	#endif

	return result;	
}

/************************************************************************/
/* restore	 															*/
/************************************************************************/

void Locale_Restore ( void )
{
	#if( USE_MULTI_FILE_FORMAT )
		LOCALE_freetable();
		LOCALE_restore();
	#else
		if( LocaleFile ) {
			free( LocaleFile );
			LocaleFile = NULL;
		}
	#endif
}


/********************************************************************************
 * Load_File -- Allocates a buffer and loads the file into it.					*
 *																				*
 *		Get a string usine Resource ID and a buffer.							*
 *																				*
 * INPUT:   int StringID  -- The resource number of the string.					*
 *          CHAR *String  -- The buffer to place the text in.					*
 *																				*
 * OUTPUT:  Returns with a pointer to the buffer.								*
 *																				*
 * WARNINGS:   none																*
 *																				*
 * HISTORY:																		*
 *   12/10/2001  MML : Created.													*
 *==============================================================================*/

const CHAR* Locale_GetString( int StringID, CHAR *String )
{
	static	CHAR		buffer[ _MAX_PATH ];
	static	WCHAR		wide_buffer[ _MAX_PATH ];

	memset( buffer,		 '\0', _MAX_PATH );
	memset(	wide_buffer, '\0', _MAX_PATH );

	#if( USE_MULTI_FILE_FORMAT )
		wcscpy( wide_buffer, (WCHAR *)LOCALE_getstring( StringID ));
	#else									  
		wcscpy( wide_buffer, (WCHAR *)LOCALE_getstr( LocaleFile, StringID ));
	#endif

	Remove_Quotes_Around_String( wide_buffer );
	WideCharToMultiByte( CodePage, 0, wide_buffer, _MAX_PATH, buffer, _MAX_PATH, NULL, NULL );

	if( String != NULL ) {
		strncpy( String, buffer, _MAX_PATH );
	}
	return buffer;
}

/********************************************************************************
 * Load_File -- Allocates a buffer and loads the file into it.					*
 *																				*
 *		Get a string usine Resource ID and a buffer.							*
 *																				*
 * INPUT:   int StringID  -- The resource number of the string.					*
 *          WCHAR *String  -- The buffer to place the text in.					*
 *																				*
 * OUTPUT:  Returns with a pointer to the buffer.								*
 *																				*
 * WARNINGS:   none																*
 *																				*
 * HISTORY:																		*
 *   12/10/2001  MML : Created.													*
 *==============================================================================*/

const WCHAR* Locale_GetString( int StringID, WCHAR *String )
{
	static WCHAR wide_buffer[ _MAX_PATH ];

	memset(	wide_buffer, '\0', _MAX_PATH );

	#if( USE_MULTI_FILE_FORMAT )
		wcscpy( wide_buffer, (WCHAR *)LOCALE_getstring( StringID ));
	#else									  
		wcscpy( wide_buffer, (WCHAR *)LOCALE_getstr( LocaleFile, StringID ));
	#endif

	Remove_Quotes_Around_String( wide_buffer );

	if( String != NULL ) {
		wcsncpy( String, wide_buffer, _MAX_PATH );
	}
	return wide_buffer;
}

/********************************************************************************
 * Load_File -- Allocates a buffer and loads the file into it.					*
 *																				*
 *		Get a string usine Resource ID and a buffer.							*
 *																				*
 * INPUT:   int StringID  -- The resource number of the string.					*
 *																				*
 * OUTPUT:  Returns with a pointer to the buffer.								*
 *																				*
 * WARNINGS:   none																*
 *																				*
 * HISTORY:																		*
 *   12/10/2001  MML : Created.													*
 *==============================================================================*/

const WCHAR* Locale_GetString( int StringID )
{
	static WCHAR wide_buffer[ _MAX_PATH ];

	memset(	wide_buffer, '\0', _MAX_PATH );

	#if( USE_MULTI_FILE_FORMAT )
		wcscpy( wide_buffer, (WCHAR *)LOCALE_getstring( StringID ));
	#else									  
		wcscpy( wide_buffer, (WCHAR *)LOCALE_getstr( LocaleFile, StringID ));
	#endif

	Remove_Quotes_Around_String( wide_buffer );

	return wide_buffer;
}


/****************************************************************************/
/* formating strings   														*/
/****************************************************************************/

WCHAR *Remove_Quotes_Around_String ( WCHAR *old_string )
{
	WCHAR	wide_buffer[ _MAX_PATH * 3 ];
	WCHAR * letter = old_string;
	int		length;

	//----------------------------------------------------------------------
	// If string is not NULL...
	//----------------------------------------------------------------------
	if ( *letter == '"' ) {

		letter++;
		wcscpy( wide_buffer, letter );

		length = wcslen( wide_buffer );

		if ( wide_buffer[ wcslen( wide_buffer )-1 ] == '"' ) {
			wide_buffer[ wcslen( wide_buffer )-1 ] = '\0';
		}
		wcscpy( old_string, wide_buffer );
	}

	return( old_string );
}


/********************************************************************************
 * Load_File -- Allocates a buffer and loads the file into it.					*
 *																				*
 *	This is the C++ replacement for the Load_Alloc_Data function. It will		*
 *	allocate the memory big enough to hold the file & read the file into it.	*
 *																				*
 * INPUT:   file  -- The file to read.											*
 *          mem   -- The memory system to use for allocation.					*
 *																				*
 * OUTPUT:  Returns with a pointer to the allocated and filled memory block.	*
 *																				*
 * WARNINGS:   none																*
 *																				*
 * HISTORY:																		*
 *   10/17/1994 JLB : Created.													*
 *==============================================================================*/

void * Load_File ( const char *filename, long *filesize )
{
	int					size, bytes_read;
	void				*ptr = NULL;
//	StandardFileClass	file;
	RawFileClass		file;

	if ( filename == NULL || filename[0] == '\0' ) {
		return( NULL );
	}

	//-------------------------------------------------------------------------
	// Open file in READ ONLY mode.  If fails, exit.
	//-------------------------------------------------------------------------
//	file.Open( filename, MODE_READ_ONLY );
	file.Open( filename, FileClass::READ );
//	if( !file.Query_Open()) {
	if( !file.Is_Open()) {
		return( NULL );
	}

	//-------------------------------------------------------------------------
	// Get filesize and create a buffer.
	//-------------------------------------------------------------------------
// 	size = file.Query_Size();
	size = file.Size();
	ptr = (void*)malloc(size + 1);
	if ( !ptr ) {
		return( NULL );
	}

	//-------------------------------------------------------------------------
	// Read data into the buffer, close the file.
	//-------------------------------------------------------------------------
	memset( ptr, '\0', size + 1 );
//  bytes_read = file.Read( ptr, size );
	bytes_read = file.Read( ptr, size );
// 	file.Close();
	file.Close();

	//-------------------------------------------------------------------------
	// Check return bytes.  It should match the file size.
	//-------------------------------------------------------------------------
	assert( bytes_read == size );
	if ( bytes_read != size ) {
		free(ptr);
		return( NULL );
	}

	if ( filesize != NULL ) {
		*filesize = (long)size;
	}
	return( ptr );
}

