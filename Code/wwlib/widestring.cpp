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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/widestring.cpp            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/06/02 5:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "widestring.h"
#include "win.h"
#include <stdio.h>


///////////////////////////////////////////////////////////////////
//	Static member initialzation
///////////////////////////////////////////////////////////////////

int		WideStringClass::m_UsedTempStringCount	= 0;

FastCriticalSectionClass WideStringClass::m_TempMutex;

unichar_t		WideStringClass::m_NullChar				= 0;
unichar_t *	WideStringClass::m_EmptyString			= &m_NullChar;

//
// A trick to optimize strings that are allocated from the stack and used only temporarily
//
char		WideStringClass::m_TempString1[WideStringClass::MAX_TEMP_BYTES];
char		WideStringClass::m_TempString2[WideStringClass::MAX_TEMP_BYTES];
char		WideStringClass::m_TempString3[WideStringClass::MAX_TEMP_BYTES];
char		WideStringClass::m_TempString4[WideStringClass::MAX_TEMP_BYTES];

unichar_t *	WideStringClass::m_FreeTempPtr[MAX_TEMP_STRING] = {
	reinterpret_cast<unichar_t *> (m_TempString1 + sizeof (WideStringClass::_HEADER)),
	reinterpret_cast<unichar_t *> (m_TempString2 + sizeof (WideStringClass::_HEADER)),
	reinterpret_cast<unichar_t *> (m_TempString3 + sizeof (WideStringClass::_HEADER)),
	reinterpret_cast<unichar_t *> (m_TempString4 + sizeof (WideStringClass::_HEADER))
};

unichar_t *	WideStringClass::m_ResTempPtr[MAX_TEMP_STRING] = {
	NULL,
	NULL,
	NULL,
	NULL
};


///////////////////////////////////////////////////////////////////
//
//	Get_String
//
///////////////////////////////////////////////////////////////////
void
WideStringClass::Get_String (size_t length, bool is_temp)
{
	if (!is_temp && length <= 1) {
		m_Buffer = m_EmptyString;
	} else {

		unichar_t *string = NULL;

		//
		//	Should we attempt to use a temp buffer for this string?
		//
		if (is_temp && length < MAX_TEMP_LEN && m_UsedTempStringCount < MAX_TEMP_STRING) {

			//
			//	Make sure no one else is requesting a temp pointer
			// at the same time we are.
			//
			FastCriticalSectionClass::LockClass lock(m_TempMutex);

			//
			//	Try to find an available temporary buffer
			//
			for (int index = 0; index < MAX_TEMP_STRING; index ++) {
				if (m_FreeTempPtr[index] != NULL) {
					
					//
					//	Grab this unused buffer for our string
					//
					string					= m_FreeTempPtr[index];
					m_ResTempPtr[index]	= m_FreeTempPtr[index];
					m_FreeTempPtr[index]	= NULL;					
					Set_Buffer_And_Allocated_Length (string, static_cast<size_t>(MAX_TEMP_LEN));

					//
					//	Increment the count of used buffers
					//
					m_UsedTempStringCount ++;
					break;
				}
			}
		}

		if (string == NULL) {
			Set_Buffer_And_Allocated_Length (Allocate_Buffer (length), length);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////
//
//	Resize
//
///////////////////////////////////////////////////////////////////
void
WideStringClass::Resize (size_t new_len)
{
	size_t allocated_len = Get_Allocated_Length ();
	if (new_len > allocated_len) {

		//
		//	Allocate the new buffer and copy the contents of our current
		// string.
		//
		unichar_t *new_buffer = Allocate_Buffer (new_len);
		u_strcpy (new_buffer, m_Buffer);

		//
		//	Switch to the new buffer
		//
		Set_Buffer_And_Allocated_Length (new_buffer, new_len);
	}

	return ;
}


///////////////////////////////////////////////////////////////////
//
//	Uninitialised_Grow
//
///////////////////////////////////////////////////////////////////
void
WideStringClass::Uninitialised_Grow (size_t new_len)
{
	size_t allocated_len = Get_Allocated_Length ();
	if (new_len > allocated_len) {
		
		//
		//	Switch to a newly allocated buffer
		//
		unichar_t *new_buffer = Allocate_Buffer (new_len);
		Set_Buffer_And_Allocated_Length (new_buffer, new_len);	
	}

	//
	// Whenever this function is called, clear the cached length 
	//
	Store_Length (0);
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	Uninitialised_Grow
//
///////////////////////////////////////////////////////////////////
void
WideStringClass::Free_String (void)
{
	if (m_Buffer != m_EmptyString) {

		//
		//	Check to see if this string was a temporary string
		//
		bool found = false;
		for (int index = 0; index < MAX_TEMP_STRING; index ++) {
			if (m_Buffer == m_ResTempPtr[index]) {
				//
				//	Make sure no one else is modifying a temp pointer
				// at the same time we are.
				//
				FastCriticalSectionClass::LockClass lock(m_TempMutex);
				
				//
				//	Release our hold on this temporary buffer
				//
				m_Buffer[0]				= 0;
				m_FreeTempPtr[index]	= m_Buffer;
				m_ResTempPtr[index]	= 0;
				m_UsedTempStringCount --;
				found = true;
				break;
			}
		}

		//
		//	String wasn't temporary, so free the memory
		//
		if (found == false) {
			char *buffer = ((char *)m_Buffer) - sizeof (WideStringClass::_HEADER);
			delete [] buffer;
		}

		//
		//	Reset the buffer
		//
		m_Buffer = m_EmptyString;
	}

	return ;
}


///////////////////////////////////////////////////////////////////
//
//	Format
//
///////////////////////////////////////////////////////////////////
int
WideStringClass::Format_Args (const unichar_t *format, va_list arg_list )
{
	if (format == NULL) {
		return 0;
	}

	//
	// Make a guess at the maximum length of the resulting string
	//
	unichar_t temp_buffer[512] = { 0 };

	//
	//	Format the string
	//
	int retval = u_vsnprintf_u (temp_buffer, 512, format, arg_list);
	
	//
	// Ensure null termination if an error occurred
	//
	if (retval < 0 || retval >= 512) {
		temp_buffer[511] = U_CHAR('\0');
	}
	
	//
	//	Copy the string into our buffer
	//	
	(*this) = temp_buffer;

	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Format
//
///////////////////////////////////////////////////////////////////
int
WideStringClass::Format (const unichar_t *format, ...)
{
	if (format == NULL) {
		return 0;
	}

	va_list arg_list;
	va_start (arg_list, format);

	//
	// Make a guess at the maximum length of the resulting string
	//
	unichar_t temp_buffer[512] = { 0 };

	//
	//	Format the string
	//
	int retval = u_vsnprintf_u (temp_buffer, 512, format, arg_list);
	
	//
	// Ensure null termination if an error occurred
	//
	if (retval < 0 || retval >= 512) {
		temp_buffer[511] = U_CHAR('\0');
	}

	//
	//	Copy the string into our buffer
	//	
	(*this) = temp_buffer;

	va_end (arg_list);
	return retval;
}


///////////////////////////////////////////////////////////////////
//
//	Release_Resources
//
///////////////////////////////////////////////////////////////////
void
WideStringClass::Release_Resources (void)
{
	return ;
}

///////////////////////////////////////////////////////////////////
// Convert_From
///////////////////////////////////////////////////////////////////
bool WideStringClass::Convert_From (const char *text)
{
	if (text != NULL) {
#ifdef W3D_USING_ICU
		int32_t length;
		UErrorCode error = U_ZERO_ERROR;
		u_strFromUTF8(nullptr, 0, &length, text, -1, &error);

		if (length > 0) {
			++length; // Add space for null termination as ICU does not include that in calculated length.
			error = U_ZERO_ERROR;
			u_strFromUTF8(Get_Buffer(length), length, nullptr, text, -1, &error);

			if (U_SUCCESS(error)) {
				Store_Length(length - 1);
				return true;
			}
		}

		WWDEBUG_SAY(("Conversion from utf-8 to utf-16 failed"));
#else
		int length;

		length = MultiByteToWideChar (CP_UTF8, 0, text, -1, NULL, 0);
		if (length > 0) {

			size_t wide_length = static_cast<size_t>(length);
			Uninitialised_Grow(wide_length);
			Store_Length(wide_length - 1);

			// Convert.
			MultiByteToWideChar (CP_UTF8, 0, text, -1, m_Buffer, length);

			// Success.
			return (true);
		}
#endif
   }

	// Failure.
	return (false);
}

///////////////////////////////////////////////////////////////////
// Test if a Unicode string is within the ANSI range. (0 - 255)
///////////////////////////////////////////////////////////////////
bool WideStringClass::Is_ANSI(void)
{
	if (m_Buffer) {
		for (int index = 0; m_Buffer[index] != 0; index++) {
			unichar_t value = m_Buffer[index];

			//
			// This is not strictly ANSI, but rather Windows-1252 code page which aligns with UTF16
			//
			if (value > U_CHAR('\xFF')) {
				return false;
			}
		}
	}

	return true;
}

