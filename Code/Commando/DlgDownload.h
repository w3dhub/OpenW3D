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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Commando/DlgDownload.h $
*
* DESCRIPTION
*     File download dialog.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 5 $
*     $Modtime: 12/20/01 9:43a $
*
******************************************************************************/

#ifndef __DLGDOWNLOAD_H__
#define __DLGDOWNLOAD_H__

#include <wwui/popupdialog.h>
#include <WWOnline/WOLDownload.h>

class DlgDownload :
		public PopupDialogClass
	{
	public:
		static bool DoDialog(const WCHAR* title, const WWOnline::DownloadList& files, bool quiet = false);

	protected:
		DlgDownload();
		~DlgDownload();

		// Prevent copy and assignment
		DlgDownload(const DlgDownload&);
		const DlgDownload& operator=(const DlgDownload&);

		bool FinalizeCreate(const WWOnline::DownloadList& files);

		void On_Init_Dialog(void);
		void On_Command(int ctrl, int message, DWORD param);
		void On_Periodic(void);

		void UpdateProgress(WWOnline::DownloadEvent& event);
		static void HandleCallback(WWOnline::DownloadEvent& event, unsigned long userdata);

	private:
		RefPtr<WWOnline::DownloadWait> mWait;

		unsigned long mStartTime;
		bool mDownloading;
		static bool mQuietMode;
	};

#endif // __DLGDOWNLOAD_H__