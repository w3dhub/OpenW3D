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

// VisWindowDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "viswindowdialog.h"
#include "visrasterizer.h"
#include "visrendercontext.h"
#include "pscene.h"
#include "phys.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// VisWindowDialogClass
//
/////////////////////////////////////////////////////////////////////////////
VisWindowDialogClass::VisWindowDialogClass(CWnd* pParent /*=NULL*/)	:
	BitmapBits (NULL),
	Bitmap (NULL),
	Width (0),
	Height (0),
	MemDC (NULL),
	CurToolTipVisId(unsigned(-1)),
	CDialog(VisWindowDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(VisWindowDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//
	//	Create a screen compatible DC
	//
	MemDC = ::CreateCompatibleDC (NULL);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~VisWindowDialogClass
//
/////////////////////////////////////////////////////////////////////////////
VisWindowDialogClass::~VisWindowDialogClass (void)
{
	Free_DIB_Section ();

	::DeleteDC (MemDC);
	MemDC = NULL;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
VisWindowDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VisWindowDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(VisWindowDialogClass, CDialog)
	//{{AFX_MSG_MAP(VisWindowDialogClass)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// Create
//
/////////////////////////////////////////////////////////////////////////////
void
VisWindowDialogClass::Create (void)
{
	CDialog::Create (VisWindowDialogClass::IDD, NULL);
	return ;
}

/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL VisWindowDialogClass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// create the tool-tip
	if (ToolTip.Create(this, TTS_ALWAYSTIP) && ToolTip.AddTool(this))
	{
		ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
		ToolTip.SendMessage(TTM_SETDELAYTIME, TTDT_AUTOPOP, SHRT_MAX);
		ToolTip.SendMessage(TTM_SETDELAYTIME, TTDT_INITIAL, 200);
		ToolTip.SendMessage(TTM_SETDELAYTIME, TTDT_RESHOW, 200);
	}
	else
	{
		TRACE("Error in creating ToolTip");
	}
 	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Display
//
/////////////////////////////////////////////////////////////////////////////
void
VisWindowDialogClass::Update_Display (VisRasterizerClass &rasterizer)
{
	//
	// Don't do anything if the user has not enabled the vis window
	//
	if (m_hWnd == NULL || IsWindowVisible () == false) {
		return;
	}

	//
	//	Get the resolution of rasterizer
	//
	int width	= 0;
	int height	= 0;
	rasterizer.Get_Resolution (&width, &height);

	//
	//	Recreate DIB section
	//
	Create_DIB_Section (width, height);

	//
	//	Loop over all the rows in the bitmap
	//
	int index	= 0;
	int stride	= (((Width * 3) + 3) & ~3);
	for (int row = 0; row < Height; row ++) {

		//
		//	DIB sections are aligned on DWORD boundaries, so ensure
		// our starting index for this row is mapped appropriately.
		//
		index = row * stride;
		
		//
		//	Get a pointer to the raw vis-render data for this row of the bitmap
		//
		const uint32 *pixel_ptr = rasterizer.Get_Pixel_Row (row, 0, Width - 1);

		//
		//	Loop over all the columns of the bitmap
		//		
		for (int col = 0; col < Width; col ++) {

			unsigned int id			= pixel_ptr[col];// + 1;
			unsigned int pixel		= Id_To_Pixel(id);
			
			//
			//	Convert the pixel into red, green, and blue components
			//
			BYTE red		= BYTE(pixel & 0x000000FF);
			BYTE green	= BYTE((pixel & 0x0000FF00) >> 8);
			BYTE blue	= BYTE((pixel & 0x00FF0000) >> 16);			
			
			//
			//	Store the pixel in the bitmap
			//
			BitmapBits[index ++] = blue;
			BitmapBits[index ++] = green;
			BitmapBits[index ++] = red;
		}
	}

	//
	//	Update the window
	//
	Paint_Display ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Free_DIB_Section
//
/////////////////////////////////////////////////////////////////////////////
void
VisWindowDialogClass::Free_DIB_Section (void)
{
	Width		= 0;
	Height	= 0;

	//
	//	Free the bitmap
	//
	if (Bitmap != NULL) {
		::DeleteObject (Bitmap);
		Bitmap = NULL;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Create_DIB_Section
//
/////////////////////////////////////////////////////////////////////////////
void
VisWindowDialogClass::Create_DIB_Section (int width, int height)
{
	if (Width == width && Height == height) {
		return ;
	}

	//
	//	Calculate what size to make the window
	//
	RECT rect	= { 0, 0, width * 2, height * 2 };
	LONG style	= ::GetWindowLong (m_hWnd, GWL_STYLE);
	::AdjustWindowRect (&rect, style, false);

	//
	//	Resize the window
	//
	SetWindowPos (NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE);

	//
	//	Release our old DIB section
	//
	Free_DIB_Section ();

	//
	//	Cache the new width and height
	//
	Width		= width;
	Height	= height;

	//
	// Set-up the fields of the BITMAPINFOHEADER
	//	Note: Top-down DIBs use negative height in Win32.
	//
	BITMAPINFOHEADER bitmap_info = { 0 };
	bitmap_info.biSize				= sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth				= Width;
	bitmap_info.biHeight				= -Height;
	bitmap_info.biPlanes				= 1;
	bitmap_info.biBitCount			= 24;
	bitmap_info.biCompression		= BI_RGB;
	bitmap_info.biSizeImage			= ((Width * Height) * 3);
	bitmap_info.biXPelsPerMeter	= 0;
	bitmap_info.biYPelsPerMeter	= 0;
	bitmap_info.biClrUsed			= 0;
	bitmap_info.biClrImportant		= 0;

	//
	// Create a bitmap that we can access the bits directly of
	//
	HDC screen_dc = ::GetDC (NULL);
	Bitmap = ::CreateDIBSection (	screen_dc,
											(const BITMAPINFO *)&bitmap_info,
											DIB_RGB_COLORS,
											(void **)&BitmapBits,
											NULL,
											0L);

	//
	// Release our temporary screen DC
	//
	::ReleaseDC (NULL, screen_dc);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_Display
//
/////////////////////////////////////////////////////////////////////////////
void
VisWindowDialogClass::Paint_Display (void)
{
	if (m_hWnd == NULL || Bitmap == NULL) {
		return ;
	}

	//
	//	Setup the DCs for blitting
	//
	HDC wnd_dc			= ::GetDC (m_hWnd);
	HBITMAP old_bmp	= (HBITMAP)::SelectObject (MemDC, Bitmap);

	//
	//	Copy the contents of the bitmap to the screen
	//
	::StretchBlt (wnd_dc, 0, 0, Width * 2, Height * 2, MemDC, 0, 0, Width, Height, SRCCOPY);

	//
	//	Restore the DCs
	//
	::SelectObject (MemDC, old_bmp);
	::ReleaseDC (m_hWnd, wnd_dc);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnPaint
//
/////////////////////////////////////////////////////////////////////////////
void
VisWindowDialogClass::OnPaint (void)
{
	CPaintDC dc (this);
	Paint_Display ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Hit_Test - determines the vis-id under the cursor
//
/////////////////////////////////////////////////////////////////////////////
int 
VisWindowDialogClass::Hit_Test(CPoint point) const
{
	// 
	// If we don't have a bitmap, just return
	//
	if (BitmapBits == NULL) {
		return -1;
	}

	// 
	// Look up the pixel, we are StretchBlt'ing the bitmap to 2x the normal size
	// so we divide the coordinates by two
	//
	int x = point.x / 2;
	int y = point.y / 2;

	if ((x <= 0) || (x >= Width) || (y <= 0) || (y >= Height)) {
		return -1;
	}

	int stride	= (((Width * 3) + 3) & ~3);
	int pixel_address = x * 3 + stride * y;

	BYTE blue = BitmapBits[pixel_address ++];
	BYTE green = BitmapBits[pixel_address ++];
	BYTE red = BitmapBits[pixel_address ++];

	// 
	// Map the color back to a vis ID
	// 
	unsigned int color = red | (green<<8) | (blue<<16);
	unsigned int id = Pixel_To_Id(color);
	return id;
}

/////////////////////////////////////////////////////////////////////////////
//
// Id_To_Pixel
//
/////////////////////////////////////////////////////////////////////////////
unsigned int	
VisWindowDialogClass::Id_To_Pixel(unsigned int id) const		
{ 
	uint32 pixel = 0;
	pixel |= ((id & 0x0000000F) >>  0) << 20;		// LSN (Least Significant Nibble) goes into MSN of red
	pixel |= ((id & 0x000000F0) >>  4) << 12;		// next LSN goes into MSN of green
	pixel |= ((id & 0x00000F00) >>  8) << 4;		// next LSN goes into MSN of blue
	pixel |= ((id & 0x0000F000) >> 12) << 16;		// put into LSN of red
	pixel |= ((id & 0x000F0000) >> 16) << 8;		// put into LSN of green
	pixel |= ((id & 0x00F00000) >> 20) << 0;		// put into LSN of blue

	return pixel;
}

/////////////////////////////////////////////////////////////////////////////
//
// Pixel_To_Id
//
/////////////////////////////////////////////////////////////////////////////
unsigned int	
VisWindowDialogClass::Pixel_To_Id(unsigned int pixel) const
{ 
	uint32 id = 0;

	id |= ((pixel & 0x0000000F) >>  0) << 20;		
	id |= ((pixel & 0x000000F0) >>  4) << 8;		
	id |= ((pixel & 0x00000F00) >>  8) << 16;		
	id |= ((pixel & 0x0000F000) >> 12) << 4;
	id |= ((pixel & 0x000F0000) >> 16) << 12;
	id |= ((pixel & 0x00F00000) >> 20) << 0;

	return id;
}


/////////////////////////////////////////////////////////////////////////////
//
// PreTranslateMessage
//
/////////////////////////////////////////////////////////////////////////////
BOOL 
VisWindowDialogClass::PreTranslateMessage(MSG* pMsg) 
{
	if (::IsWindow(ToolTip.m_hWnd) && pMsg->hwnd == m_hWnd)
	{
		switch(pMsg->message)
		{
		case WM_LBUTTONDOWN:    
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:    
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:    
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			ToolTip.RelayEvent(pMsg);
			break;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
//
// OnMouseMove
//
/////////////////////////////////////////////////////////////////////////////
void VisWindowDialogClass::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (::IsWindow(ToolTip.m_hWnd))
	{
		unsigned int vis_id = Hit_Test(point);

		if ((vis_id == -1) || (vis_id != CurToolTipVisId)) {
			// Use Activate() to hide the tooltip.
			ToolTip.Activate(false);        
		}

		if (vis_id != -1) {
			ToolTip.Activate(true);

			if (vis_id == BACKFACE_VIS_ID) {

				CString tooltip_string;
				tooltip_string.Format("Vis Id: %d, Backface!",vis_id);
				ToolTip.UpdateTipText(tooltip_string,this);

			} else {

				PhysClass * obj = NULL;
				RefPhysListIterator	it = PhysicsSceneClass::Get_Instance()->Get_Static_Object_Iterator();
				while ((!it.Is_Done()) && (obj == NULL)) {
					if (it.Peek_Obj()->Get_Vis_Object_ID()==(int)vis_id) {
						obj = it.Peek_Obj();
					}
					it.Next();
				}

				if (obj != NULL) {
					CString tooltip_string;
					tooltip_string.Format("Vis Id: %d, Model Name: %s",vis_id,obj->Peek_Model()->Get_Name());
					ToolTip.UpdateTipText(tooltip_string,this);
				} else {
					CString tooltip_string;
					tooltip_string.Format("Vis Id: %d, Unknown Model",vis_id);
					ToolTip.UpdateTipText(tooltip_string,this);
				}
			}
			CurToolTipVisId = vis_id;
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}

