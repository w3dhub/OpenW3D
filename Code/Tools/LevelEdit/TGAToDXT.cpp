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
 *                 Project Name : LevelEdit																	  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TGAToDXT.cpp       $*
 *                                                                                             *
 *                       Author:: Ian Leslie			                                            *
 *                                                                                             *
 *                     $Modtime:: 8/29/01 5:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "Targa.h"
#include "TGAToDXT.H"
#include <crnlib.h>
#include <io.h>
#include <stdlib.h>

// Singletons.
TGAToDXTClass _TGAToDXTConverter;


///////////////////////////////////////////////////////////////////////////////
//
//	TGAToDXTClass
//
///////////////////////////////////////////////////////////////////////////////
TGAToDXTClass::TGAToDXTClass()
	: WriteTimePtr (NULL),
		Buffer(NULL),
	  BufferSize (1024),
	  BufferCount (0)
{
}


///////////////////////////////////////////////////////////////////////////////
//
//	~TGAToDXTClass
//
///////////////////////////////////////////////////////////////////////////////
TGAToDXTClass::~TGAToDXTClass()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//	Convert
//
///////////////////////////////////////////////////////////////////////////////
bool TGAToDXTClass::Convert (const char *inputpathname, const char *outputpathname, FILETIME *writetimeptr, bool &redundantalpha)
{
	bool  success;
	Targa targa;
	int	error;

	WriteTimePtr = writetimeptr;
	redundantalpha = false;

	success = false;
	error = targa.Load (inputpathname, TGAF_IMAGE, false);
	if (error == 0) {

		bool validbitdepth, validsize, validaspect;

		// Check that the targa is in the right format.
		// In order to be valid it must adhere to the following:
		// 1. Pixel depth must be 24 or 32 (compressor has no support for lower bit depths).
		// 2. Dimensions >= 4 (DDS block size is 4x4).
		// 3. Aspect ratio <= 1:8 (some H/W will not render textures above this ratio).
		// 4. Dimensions must be power of 2 (see below).
		validbitdepth = ((targa.Header.PixelDepth == 24) || (targa.Header.PixelDepth == 32));
		validsize	  = (targa.Header.Width >= 4) && (targa.Header.Height >= 4);
		validaspect	  = ((float) MAX (targa.Header.Width, targa.Header.Height)) / ((float) MIN (targa.Header.Width, targa.Header.Height)) <= 8.0f; 
		if (validbitdepth && validsize && validaspect) {
			crn_comp_params comp_params;
      comp_params.m_width = targa.Header.Width;
      comp_params.m_height = targa.Header.Height;
      comp_params.set_flag(cCRNCompFlagHierarchical, false);
      comp_params.m_file_type = cCRNFileTypeDDS;
      comp_params.m_format = targa.Header.PixelDepth == 32 ? cCRNFmtDXT5 : cCRNFmtDXT1;

			targa.YFlip();
      comp_params.m_pImages[0][0] = reinterpret_cast<crn_uint32 *>(targa.GetImage());

      crn_mipmap_params mip_params;
      // mip_params.m_gamma_filtering = true;
      mip_params.m_mode = cCRNMipModeGenerateMips;
      mip_params.m_min_mip_size = 4;

			crn_uint32 output_file_size;
			void *output_file_data = crn_compress(comp_params, mip_params, output_file_size);
			// Was the image compressed successfully?
			// NOTE: Any image that does not have power of 2 dimensions will not be compressed.
			if (output_file_data) {
				Buffer = static_cast<unsigned char *>(output_file_data);
				BufferCount = output_file_size;
				Write (outputpathname);
				Buffer = NULL;
				crn_free_block(output_file_data);
				success = true;
			}
		}
	}

	return (success);
}


///////////////////////////////////////////////////////////////////////////////
//
//	Write
//
///////////////////////////////////////////////////////////////////////////////
void TGAToDXTClass::Write (const char *outputpathname)
{
	if (!Buffer) {
		return;
	}
	
	HANDLE hfile;
	DWORD  bytecountwritten;
	
	hfile = ::CreateFileA (outputpathname, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0L, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
      LockFile (hfile, 0, 0, BufferCount, 0); 
      WriteFile (hfile, Buffer, BufferCount, &bytecountwritten, NULL);
      UnlockFile (hfile, 0, 0, BufferCount, 0); 
		
		// Stamp the write time (if one has been supplied).
		if (WriteTimePtr != NULL) {
			SetFileTime (hfile, NULL, NULL, WriteTimePtr);
		}

		CloseHandle (hfile);
	}

	// Reset buffer.
	BufferCount = 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//	ReadDTXnFile
//
///////////////////////////////////////////////////////////////////////////////
void ReadDTXnFile (DWORD /* datacount */, void * /* data */)
{
	// Not implemented.
	ASSERT (false);
}


///////////////////////////////////////////////////////////////////////////////
//
//	WriteDTXnFile
//
///////////////////////////////////////////////////////////////////////////////
void WriteDTXnFile (DWORD datacount, void *data)
{
	// Ensure that the buffer is large enough.
	if (_TGAToDXTConverter.BufferSize < _TGAToDXTConverter.BufferCount + datacount) {

		unsigned			newbuffersize;
		unsigned char *newbuffer;

		newbuffersize = MAX (_TGAToDXTConverter.BufferSize * 2, _TGAToDXTConverter.BufferCount + datacount);
		newbuffer	  = new unsigned char [newbuffersize];
		ASSERT (newbuffer != NULL);
		memcpy (newbuffer, _TGAToDXTConverter.Buffer, _TGAToDXTConverter.BufferCount);
		delete [] _TGAToDXTConverter.Buffer;
		_TGAToDXTConverter.Buffer = newbuffer;
		_TGAToDXTConverter.BufferSize = newbuffersize;
	}

	// Write new data to buffer.
	memcpy (_TGAToDXTConverter.Buffer + _TGAToDXTConverter.BufferCount, data, datacount);
	_TGAToDXTConverter.BufferCount += datacount;
}

