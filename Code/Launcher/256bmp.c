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

#include <windows.h>
#include <windowsx.h>
#include <alloc.h>

DWORD GetDibInfoHeaderSize (BYTE huge *);
WORD GetDibWidth (BYTE huge *);
WORD GetDibHeight (BYTE huge *);
BYTE huge * GetDibBitsAddr (BYTE huge *);
BYTE huge * ReadDib(char *);

//-------------------------------------------------------------//

DWORD GetDibInfoHeaderSize (BYTE huge * lpDib)
{
	return ((BITMAPINFOHEADER huge *) lpDib)->biSize ;
}

WORD GetDibWidth (BYTE huge * lpDib)
{
	if (GetDibInfoHeaderSize (lpDib) == sizeof (BITMAPCOREHEADER))
		return (WORD) (((BITMAPCOREHEADER huge *) lpDib)->bcWidth) ;
	else
		return (WORD) (((BITMAPINFOHEADER huge *) lpDib)->biWidth) ;
}

WORD GetDibHeight (BYTE huge * lpDib)
{
	if (GetDibInfoHeaderSize (lpDib) == sizeof (BITMAPCOREHEADER))
		return (WORD) (((BITMAPCOREHEADER huge *) lpDib)->bcHeight) ;
	else
		return (WORD) (((BITMAPINFOHEADER huge *) lpDib)->biHeight) ;
}

BYTE huge * GetDibBitsAddr (BYTE huge * lpDib)
{
	DWORD dwNumColors, dwColorTableSize ;
	WORD  wBitCount ;

	if (GetDibInfoHeaderSize (lpDib) == sizeof (BITMAPCOREHEADER))
	{
		wBitCount = ((BITMAPCOREHEADER huge *) lpDib)->bcBitCount ;

		if (wBitCount != 24)
			dwNumColors = 1L << wBitCount ;
		else
			dwNumColors = 0 ;

		dwColorTableSize = dwNumColors * sizeof (RGBTRIPLE) ;
	}
	else
	{
		wBitCount = ((BITMAPINFOHEADER huge *) lpDib)->biBitCount ;

		if (GetDibInfoHeaderSize (lpDib) >= 36)
			dwNumColors = ((BITMAPINFOHEADER huge *) lpDib)->biClrUsed ;
		else
			dwNumColors = 0 ;

		if (dwNumColors == 0)
		{
			if (wBitCount != 24)
				dwNumColors = 1L << wBitCount ;
			else
				dwNumColors = 0 ;
		}

		dwColorTableSize = dwNumColors * sizeof (RGBQUAD) ;
	}

	return lpDib + GetDibInfoHeaderSize (lpDib) + dwColorTableSize ;
}

// Read a DIB from a file into memory
BYTE huge * ReadDib (char * szFileName)
{
	BITMAPFILEHEADER bmfh ;
	BYTE huge *      lpDib ;
	DWORD            dwDibSize, dwOffset, dwHeaderSize ;
	int              hFile ;
	WORD             wDibRead ;

	if (-1 == (hFile = _lopen (szFileName, OF_READ | OF_SHARE_DENY_WRITE)))
		return NULL ;

	if (_lread (hFile, (LPSTR) &bmfh, sizeof (BITMAPFILEHEADER)) !=
									   sizeof (BITMAPFILEHEADER))
	{
		_lclose (hFile) ;
		return NULL ;
	}

	if (bmfh.bfType != * (WORD *) "BM")
	{
		  _lclose (hFile) ;
		  return NULL ;
	}

	dwDibSize = bmfh.bfSize - sizeof (BITMAPFILEHEADER) ;

	lpDib = (BYTE huge * ) GlobalAllocPtr (GMEM_MOVEABLE, dwDibSize) ;

	if (lpDib == NULL)
	{
		_lclose (hFile) ;
		return NULL ;
	}

	dwOffset = 0 ;

	while (dwDibSize > 0)
	{
		wDibRead = (WORD) std::min (32768ul, dwDibSize) ;

		if (wDibRead != _lread (hFile, (LPSTR) (lpDib + dwOffset), wDibRead))
		{
			_lclose (hFile) ;
			return NULL ;
		}

		dwDibSize -= wDibRead ;
		dwOffset  += wDibRead ;
	}

	_lclose (hFile) ;

	dwHeaderSize = GetDibInfoHeaderSize (lpDib) ;

	if (dwHeaderSize < 12 || (dwHeaderSize > 12 && dwHeaderSize < 16))
		return NULL ;
	return lpDib ;
}

long FAR PASCAL _export MainWndProc(HWND hWnd,UINT message,UINT wParam,LONG lParam)
{
	PAINTSTRUCT	ps;
	HDC hdc;
	RECT rect;
	unsigned char r,g,b,x;
	FILE *fi;
	int i;

	static BYTE huge *lpDib;
	static BYTE huge *lpDibBits;
	static int cxDib, cyDib;
	static LPLOGPALETTE LogPal;
	static HPALETTE hLogPal;

	switch(message)
	{
		case WM_CREATE:
			fi=fopen("somefile.bmp","r");
			if(fi==NULL)
				return 0;

			LogPal=(LPLOGPALETTE)farmalloc(sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*256);
			LogPal->palVersion=0x300;
			LogPal->palNumEntries=256;

			fseek(fi,54L,0);
			for(i=0;i<256;i++)
			{
				fscanf(fi,"%c%c%c%c",&b,&g,&r,&x);
				LogPal->palPalEntry[i].peRed=r;
				LogPal->palPalEntry[i].peGreen=g;
				LogPal->palPalEntry[i].peBlue=b;
				LogPal->palPalEntry[i].peFlags=x;
			}
			fclose(fi);
			hLogPal=CreatePalette(LogPal);
			return 0;
		case WM_PAINT:
			hdc=BeginPaint(hWnd,&ps);
			GetClientRect(hWnd,&rect);
			SelectPalette(hdc,hLogPal,0);
			RealizePalette(hdc);

			SetStretchBltMode(hdc,COLORONCOLOR);
			lpDib=ReadDib(bmp_fname);
			lpDibBits=GetDibBitsAddr(lpDib);
			cxDib=GetDibWidth(lpDib);
			cyDib=GetDibHeight(lpDib);
			StretchDIBits(hdc,0,0,rect.right,rect.bottom,0,0,cxDib,cyDib,(LPSTR)lpDibBits,
				(LPBITMAPINFO)lpDib,DIB_RGB_COLORS,SRCCOPY);
			EndPaint(hWnd,&ps);
			break;
		case WM_DESTROY:
			farfree(LogPal);
			DeleteObject(hLogPal);
			PostQuitMessage(0);
			break;
		default:
			return (DefWindowProc(hWnd,message,wParam,lParam));
	}
	return (DefWindowProc(hWnd,message,wParam,lParam));
}