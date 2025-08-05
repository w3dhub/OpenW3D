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
* NAME
*     $Archive: /Commando/Code/Commando/WebBrowser.h $
*
* DESCRIPTION
*     Web Browser
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 7 $
*     $Modtime: 1/15/02 3:06p $
*
******************************************************************************/

#ifndef __WEBBROWSER_H__
#define __WEBBROWSER_H__

#include "WOLBrowser\WOLBrowser.h"
#include <WWLib\Notify.h>
#include <atlbase.h>
#include <windows.h>

class WebBrowser;

class WebEvent :
		public TypedEventPtr<WebEvent, WebBrowser>
	{
	public:
		typedef enum
			{
			None = 0, // NULL event
			Quit, // User initiated quit
			CertificationFailed, // Requested page failed certification.
			} EventID;

		//! Retrieve event
		inline EventID Event(void) const
			{return mEvent;}

		WebEvent(EventID event, WebBrowser* object) :
				TypedEventPtr<WebEvent, WebBrowser>(object),
			  mEvent(event)
			{}

	protected:
		// Prevent copy and assignment
		WebEvent(const WebEvent&);
		const WebEvent& operator=(const WebEvent&);

	private:
		EventID mEvent;
	};


class WebBrowser :
		public IWOLBrowserEvent,
		public Notifier<WebEvent>
	{
	public:
		// Initialize browser prerequisites.
		// NOTE: This is for development purpose only; The game installer should handle
		//       these tasks.
		#ifdef _DEBUG
		static bool InstallPrerequisites(void);
		#endif

		//! Test if a web page is currently displayed
		static bool IsWebPageDisplayed(void);

		//! Create an instance of the embedded browser for Dune Emperor.
		static WebBrowser* CreateInstance(HWND window);

		//! Check if browser is embedded or external (True if embedded)
		bool UsingEmbeddedBrowser(void) const
			{return (mWOLBrowser != NULL);}

		//! Test if the external browser is running
		bool IsExternalBrowserRunning(void) const;

		//! Display the specified web content.
		bool ShowWebPage(const char* page);

		//! Launch the external browser
		bool LaunchExternal(const char* url);

		//! Show the browser
		void Show(void);

		//! Hide the browser
		void Hide(void);

		//! Test if the browser is visible
		bool IsVisible(void) const
			{return mVisible;}

	protected:
		// Protected to prevent direct construction via new, use CreateInstance() instead.
		WebBrowser();
		virtual ~WebBrowser();

		// Protected to prevent copy and assignment
		WebBrowser(const WebBrowser&);
		const WebBrowser& operator=(const WebBrowser&);

		bool FinalizeCreate(HWND window);

		bool RetrievePageURL(const char* page, char* url, int size);
		bool RetrieveHTMLPath(char* path, int size);

		DECLARE_NOTIFIER(WebEvent)

	private:
		static WebBrowser* _mInstance;

		ULONG mRefCount;
		CComPtr<IWOLBrowser> mWOLBrowser;
		wchar_t mPendingURL[512];
		bool mVisible;

		PROCESS_INFORMATION mProcessInfo;

		bool mSwitchedMode;
		int mRestoreWidth;
		int mRestoreHeight;
		int mRestoreBits;

	//---------------------------------------------------------------------------
	// IUnknown methods
	//---------------------------------------------------------------------------
	public:
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
		ULONG STDMETHODCALLTYPE AddRef(void);
		ULONG STDMETHODCALLTYPE Release(void);

	//---------------------------------------------------------------------------
	// IWOLBrowserEvent methods
	//---------------------------------------------------------------------------
	private:
		STDMETHOD(OnScriptQuit)(void);
		STDMETHOD(OnBeforeNavigate)(const wchar_t* url, const wchar_t* targetFrame);
		STDMETHOD(OnDocumentComplete)(const wchar_t* url, BOOL topFrame);
		STDMETHOD(OnDownloadBegin)(void);
		STDMETHOD(OnProgressChange)(LONG progress, LONG progressMax);
		STDMETHOD(OnDownloadComplete)(void);
		STDMETHOD(OnNavigateComplete)(const wchar_t* url);
		STDMETHOD(OnStatusTextChange)(const wchar_t* statusText);
		STDMETHOD(OnTitleChange)(const wchar_t* title);
		STDMETHOD(OnNewWindow)(void);
		STDMETHOD(OnShowMessage)(const wchar_t* text, const wchar_t* caption, ULONG type, LONG* result);
		STDMETHOD(OnFailedPageCertification)(void);
		STDMETHOD(OnErrorMsg)(const wchar_t* error);
		STDMETHOD(OnRegisterLogin)(const wchar_t* nick, const wchar_t* pass);
	};

#endif // __WEBBROWSER_H__
