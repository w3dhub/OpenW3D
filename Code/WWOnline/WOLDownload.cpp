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
*     $Archive: /Commando/Code/WWOnline/WOLDownload.cpp $
*
* DESCRIPTION
*     This class specifies patch files to be downloaded from the server.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 8 $
*     $Modtime: 1/12/02 9:26p $
*
******************************************************************************/

#include <wwlib/always.h>
#include "WOLDownload.h"
#include "WOLProduct.h"
#include "WOLErrorUtil.h"
#include "WOLString.h"
#include <wolapi/downloaddefs.h>
#include <wwlib/wwstring.h>
#include <wwdebug/wwdebug.h>

namespace WWOnline {

/******************************************************************************
*
* NAME
*     Download::Create
*
* DESCRIPTION
*     Create a new download.
*
* INPUTS
*     Download - IDownload object.
*     Update   - WOL update description.
*
* RESULT
*     Download - Reference to new download instance.
*
******************************************************************************/

RefPtr<Download> Download::Create(const WOL::Update& update)
	{
	return new Download(update);
	}


/******************************************************************************
*
* NAME
*     Download::Download
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Download - IDownload object.
*     Update   - WOL update description.
*
* RESULT
*     NONE
*
******************************************************************************/

Download::Download(const WOL::Update& update) :
		mDownloadCookie(0),
		mState(DLPending),
		mStatusCode(DOWNLOADSTATUS_DONE),
		mErrorCode(0),
		mErrorText(NULL),
		mBytesRead(0),
		mTotalSize(0),
		mTimeElapsed(0),
		mTimeRemaining(0)
	{
	memcpy(&mWOLUpdate, &update, sizeof(mWOLUpdate));
	mWOLUpdate.next = NULL;

	WWDEBUG_SAY(("WOL: Download instantiated '%s'\n", GetFilename()));
	}


/******************************************************************************
*
* NAME
*     Download::~Download
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

Download::~Download()
	{
	WWDEBUG_SAY(("WOL: Download destructing '%s'\n", GetFilename()));
	Stop();
	}


/******************************************************************************
*
* NAME
*     Download::CreateDownloadObject
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     True if successful.
*
******************************************************************************/

bool Download::CreateDownloadObject(void)
	{
	//---------------------------------------------------------------------------
	// Create Download object
	//---------------------------------------------------------------------------
	WWDEBUG_SAY(("WOL: Creating IID_IDownload object\n"));

	WOL::IDownload* downloadObject = NULL;
	HRESULT hr = CoCreateInstance(WOL::CLSID_Download, NULL, CLSCTX_INPROC_SERVER,
			WOL::IID_IDownload, (void **)&downloadObject);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: Failed to create IID_IDownload\n"));
		return false;
		}

	mDownloadObject = downloadObject;

	// Register this download as an event sink for download events
	hr = AtlAdvise(mDownloadObject, this, WOL::IID_IDownloadEvent, &mDownloadCookie);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: AltAdvise failed\n"));
		SetError(-1, "WOL_NOTINITIALIZED");
		mDownloadCookie = 0;
		return false;
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     Download::ReleaseDownloadObject
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Download::ReleaseDownloadObject(void)
	{
	// No longer listen to download events.
	if (mDownloadObject && mDownloadCookie != 0)
		{
		HRESULT hr = AtlUnadvise(mDownloadObject, WOL::IID_IDownloadEvent, mDownloadCookie);
		mDownloadCookie = 0;

		if (FAILED(hr))
			{
			WWDEBUG_SAY(("WOLERROR: Download AltUnadvise failed\n"));
			}
		}

	mDownloadObject.Release();
	}


/******************************************************************************
*
* NAME
*     Download::Start
*
* DESCRIPTION
*     Start downloading file.
*
* INPUTS
*     NONE
*
* RESULT
*     True if download started successfully.
*
******************************************************************************/

bool Download::Start(void)
	{
	// If the file was already download successfully then we are done.
	if (DLComplete == mState)
		{
		OnEnd();
		return true;
		}

	// Verify product is initialized
	RefPtr<Product> product = Product::Current();

	if (!product.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: Product not initialized\n"));
		SetError(-1, "WOL_NOTINITIALIZED");
		return false;
		}

	bool downloadCreated = CreateDownloadObject();

	if (!downloadCreated)
		{
		WWDEBUG_SAY(("WOLERROR: IDownload object not initialized\n"));
		SetError(-1, "WOL_NOTINITIALIZED");
		return false;
		}

	// Attempt to create the target path for the download file.
	const char* localPath = GetLocalPath();
	int dirCreated = CreateDirectoryA(localPath, NULL);

	if (!dirCreated && (ERROR_ALREADY_EXISTS != GetLastError()))
		{
		WWDEBUG_SAY(("WOLERROR: Failed to create download directory '%s'\n", localPath));
		Print_Win32Error(GetLastError());
		SetError(DOWNLOADEVENT_LOCALFILEOPENFAILED, GetOnErrorText(DOWNLOADEVENT_LOCALFILEOPENFAILED));
		return false;
		}

	// Generate full pathname for source and target
	const char* downloadPath = GetDownloadPath();
	const char* filename = GetFilename();

	StringClass localFile(true);
	localFile.Format("%s\\%s", localPath, filename);

	StringClass downloadFile(true);
	downloadFile.Format("%s\\%s", downloadPath, filename);
	
	// Initiate download of file.
	const char* server = GetServerName();
	const char* login = GetLoginName();
	const char* password = GetPassword();
	const char* regPath = product->GetRegistryPath();

	WWDEBUG_SAY(("WOL: Downloading '%s' to '%s'\n", (const char*)downloadFile, (const char*)localFile));
	HRESULT hr = mDownloadObject->DownloadFile(server, login, password, downloadFile, localFile, regPath);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: DownloadFile() HRESULT = %s\n", GetDownloadErrorString(hr)));
		AtlUnadvise(mDownloadObject, WOL::IID_IDownloadEvent, mDownloadCookie);
		SetError(DOWNLOADEVENT_COULDNOTCONNECT, GetOnErrorText(DOWNLOADEVENT_COULDNOTCONNECT));
		return false;
		}

	mState = DLDownloading;

	DownloadEvent event(DownloadEvent::DOWNLOAD_BEGIN, this);
	NotifyObservers(event);

	return true;
	}


/******************************************************************************
*
* NAME
*     Download::Stop
*
* DESCRIPTION
*     Stop the download in progress.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Download::Stop(void)
	{
	if (mDownloadObject)
		{
		// If the download is in progress then stop it.
		if (DLDownloading == mState)
			{
			mState = DLAborted;

			HRESULT hr = mDownloadObject->Abort();

			if (FAILED(hr))
				{
				WWDEBUG_SAY(("WOLERROR: WOL::IDownload::Abort() HRESULT = %s\n", GetDownloadErrorString(hr)));
				}

			DownloadEvent event(DownloadEvent::DOWNLOAD_STOPPED, this);
			NotifyObservers(event);
			}

		ReleaseDownloadObject();
		}
	}


/******************************************************************************
*
* NAME
*     Download::IsDone
*
* DESCRIPTION
*     Check if the download has finished.
*
* INPUTS
*     NONE
*
* RESULT
*     True if download is finished.
*
******************************************************************************/

bool Download::IsDone(void) const
	{
	return (DLComplete == mState);
	}


/******************************************************************************
*
* NAME
*     Download::Process
*
* DESCRIPTION
*     Yield time for the download to process.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Download::Process(void)
	{
	if (mDownloadObject)
		{
		mDownloadObject->PumpMessages();
		}
	}


/******************************************************************************
*
* NAME
*     Download::GetStatusText
*
* DESCRIPTION
*     Get a text description of the current download status.
*
* INPUTS
*     NONE
*
* RESULT
*     Status - Text description of status.
*
******************************************************************************/

const unichar_t* Download::GetStatusText(void) const
	{
	static const char* _statusText[] =
		{
		// DOWNLOADSTATUS_DONE
		"WOL_DOWNLOADDONE",

		// DOWNLOADSTATUS_GO
		"WOL_DOWNLOADGO",

		// DOWNLOADSTATUS_CONNECTING
		"WOL_DOWNLOADCONNECT",
		
		// DOWNLOADSTATUS_LOGGINGIN
		"WOL_DOWNLOADLOGIN",
		
		// DOWNLOADSTATUS_FINDINGFILE
		"WOL_DOWNLOADFIND",
		
		// DOWNLOADSTATUS_QUERYINGRESUME
		"WOL_DOWNLOADRESUME",
		
		// DOWNLOADSTATUS_DOWNLOADING
		"WOL_DOWNLOADING",
		
		// DOWNLOADSTATUS_DISCONNECTING
		"WOL_DOWNLOADDISCONNECT"
		};

	if ((mStatusCode >= 0) && (mStatusCode < ARRAY_SIZE(_statusText)))
		{
		return WOLSTRING(_statusText[mStatusCode]);
		}

	return U_CHAR("");
	}


const unichar_t* Download::GetErrorText(void) const
	{
	return WOLSTRING(mErrorText);
	}


/******************************************************************************
*
* NAME
*     Download::GetOnErrorText
*
* DESCRIPTION
*     Get a text description of the current error condition.
*
* INPUTS
*     NONE
*
* RESULT
*     Error - Text description of error.
*
******************************************************************************/

const char* Download::GetOnErrorText(int onErrorCode) const
	{
	static const char* _onErrorText[] =
		{
		// DOWNLOADEVENT_NOSUCHSERVER
		"WOL_DOWNLOADNOSERVER",

		// DOWNLOADEVENT_COULDNOTCONNECT
		"WOL_DOWNLOADCONNECTERROR",

		// DOWNLOADEVENT_LOGINFAILED
		"WOL_DOWNLOADLOGINERROR",
		
		// DOWNLOADEVENT_NOSUCHFILE
		"WOL_DOWNLOADFNF",
		
		// DOWNLOADEVENT_LOCALFILEOPENFAILED
		"WOL_DOWNLOADOPENERROR",
		
		// DOWNLOADEVENT_TCPERROR
		"WOL_DOWNLOADIOERROR",
		
		// DOWNLOADEVENT_DISCONNECTERROR
		"WOL_DOWNLOADDISCONNECTERROR"
		};

	if ((onErrorCode >= 1) && (onErrorCode < ARRAY_SIZE(_onErrorText)))
		{
		return _onErrorText[onErrorCode - 1];
		}

	return "";
	}


/******************************************************************************
*
* NAME
*     Download::SetError
*
* DESCRIPTION
*
* INPUTS
*     ErrorCode - Numerical error code.
*     ErrorText - Text description of error code.
*
* RESULT
*     NONE
*
******************************************************************************/

void Download::SetError(int errorCode, const char* errorText)
	{
	mState = DLError;
	mErrorCode = errorCode;
	mErrorText = errorText;
	}


/******************************************************************************
*
* NAME
*     Download::GetProgress
*
* DESCRIPTION
*     Get the progress of the download.
*
* INPUTS
*     BytesRead     - On Return; Number of bytes read.
*     TotalSize     - On Return; Total number of bytes to download.
*     TimeElapsed   - On Return; Time elapsed since download began.
*     TimeRemaining - On Return; Estimated time remaining for download.
*
* RESULT
*     NONE
*
******************************************************************************/

void Download::GetProgress(int& bytesRead, int& totalSize, int& timeElapsed, int& timeRemaining) const
	{
	bytesRead = mBytesRead;
	totalSize = mTotalSize;
	timeElapsed = mTimeElapsed;
	timeRemaining = mTimeRemaining;
	}


/****************************************************************************
*
* NAME
*     IUnknown::QueryInterface
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
****************************************************************************/

STDMETHODIMP Download::QueryInterface(const IID& iid, void** ppv)
	{
	if ((iid == IID_IUnknown) || (iid == WOL::IID_IDownloadEvent))
		{
		*ppv = static_cast<WOL::IDownloadEvent*>(this);
		}
	else
		{
		*ppv = NULL;
		return E_NOINTERFACE;
		}

	static_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
	}


/****************************************************************************
*
* NAME
*     IUnknown::AddRef
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
****************************************************************************/

ULONG STDMETHODCALLTYPE Download::AddRef(void)
	{
	RefCounted::AddReference();
	return RefCounted::ReferenceCount();
	}


/****************************************************************************
*
* NAME
*     IUnknown::Release
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
****************************************************************************/

ULONG STDMETHODCALLTYPE Download::Release(void)
	{
	ULONG refCount = RefCounted::ReferenceCount();
	RefCounted::ReleaseReference();
	return --refCount;
	}


/******************************************************************************
*
* NAME
*     Download::OnEnd
*
* DESCRIPTION
*     Notification that the download has completed.
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

STDMETHODIMP Download::OnEnd(void)
	{
	WWDEBUG_SAY(("WOL: Download End '%s'\n", GetFilename()));

	SetError(0, NULL);
	mState = DLComplete;

	DownloadEvent event(DownloadEvent::DOWNLOAD_END, this);
	NotifyObservers(event);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     Download::OnError
*
* DESCRIPTION
*     Notification that the download has encountered an error.
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP Download::OnError(int error)
	{
	WWDEBUG_SAY(("WOLERROR: Download '%s'\n", GetFilename()));

	#ifdef WWDEBUG
	static const char* _errors[] =
		{
		"DOWNLOADEVENT_NOSUCHSERVER",
		"DOWNLOADEVENT_COULDNOTCONNECT",
		"DOWNLOADEVENT_LOGINFAILED",
		"DOWNLOADEVENT_NOSUCHFILE",
		"DOWNLOADEVENT_LOCALFILEOPENFAILED",
		"DOWNLOADEVENT_TCPERROR",
		"DOWNLOADEVENT_DISCONNECTERROR"
		};

	WWDEBUG_SAY(("WOLERROR: Download Error: %s\n", _errors[error - 1]));
	#endif

	SetError(error, GetOnErrorText(error));

	DownloadEvent event(DownloadEvent::DOWNLOAD_ERROR, this);
	NotifyObservers(event);
	
	return S_OK;
	}

        
/******************************************************************************
*
* NAME
*     Download::OnProgressUpdate
*
* DESCRIPTION
*     Notification of the downloads progress.
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP Download::OnProgressUpdate(int bytesRead, int totalSize,
		int timeElapsed, int timeRemaining)
	{
	mBytesRead = bytesRead;
	mTotalSize = totalSize;
	mTimeElapsed = timeElapsed;
	mTimeRemaining = timeRemaining;

	DownloadEvent event(DownloadEvent::DOWNLOAD_PROGRESS, this);
	NotifyObservers(event);
	
	return S_OK;
	}

        
/******************************************************************************
*
* NAME
*     DownloadObserver::OnQueryResume
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP Download::OnQueryResume(void)
	{
	WWDEBUG_SAY(("WOL: Download QueryResume '%s'\n", GetFilename()));
	
	DownloadEvent event(DownloadEvent::DOWNLOAD_RESUME, this);
	NotifyObservers(event);

	return DOWNLOADEVENT_RESUME;
	}

        
/******************************************************************************
*
* NAME
*     DownloadObserver::OnStatusUpdate
*
* DESCRIPTION
*     Notification of the downloads status change.
*
* INPUTS
*
* RESULT
*
******************************************************************************/

STDMETHODIMP Download::OnStatusUpdate(int status)
	{
	#ifdef WWDEBUG
	static const char* _status[] =
		{
		"DOWNLOADSTATUS_DONE",
		"DOWNLOADSTATUS_GO",
		"DOWNLOADSTATUS_CONNECTING",
		"DOWNLOADSTATUS_LOGGINGIN",
		"DOWNLOADSTATUS_FINDINGFILE",
		"DOWNLOADSTATUS_QUERYINGRESUME",
		"DOWNLOADSTATUS_DOWNLOADING",
		"DOWNLOADSTATUS_DISCONNECTING",
		};

	WWDEBUG_SAY(("WOL: Download Status: %s\n", _status[status]));
	#endif

	mStatusCode = status;

	DownloadEvent event(DownloadEvent::DOWNLOAD_STATUS, this);
	NotifyObservers(event);

	return S_OK;
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     DownloadWait::Create
*
* DESCRIPTION
*     Create a wait condition to process downloads.
*
* INPUTS
*     List of download files.
*
* RESULT
*     Wait condition to process.
*
******************************************************************************/

RefPtr<DownloadWait> DownloadWait::Create(const DownloadList& files)
	{
	if (files.empty())
		{
		return NULL;
		}

	return new DownloadWait(files);
	}


/******************************************************************************
*
* NAME
*     DownloadWait::DownloadWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     List of download files.
*
* RESULT
*     NONE
*
******************************************************************************/

DownloadWait::DownloadWait(const DownloadList& files) :
		SingleWait(WOLSTRING("WOL_DOWNLOADING")),
		mFiles(files),
		mFileIndex(-1),
		mCallback(NULL)
	{
	WWDEBUG_SAY(("WOL: DownloadWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     DownloadWait::~DownloadWait
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DownloadWait::~DownloadWait()
	{
	WWDEBUG_SAY(("WOL: DownloadWait End '%S'\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     DownloadWait::WaitBeginning
*
* DESCRIPTION
*     Begin download process.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DownloadWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: DownloadWait Begin\n"));

	// If there are no files to download then just say we are done.
	if (mFiles.empty())
		{
		SingleWait::EndWait(ConditionMet, WOLSTRING("WOL_DOWNLOADDONE"));
		return;
		}

	// WaitBeginning should only be called once.
	if (mCurrentDownload.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: DownloadWait::WaitBeginning() called more that once"));
		WWASSERT("DownloadWait::WaitBeginning() called more that once");
		return;
		}
	
	// Start the first download
	mFileIndex = 0;
	mCurrentDownload = mFiles[0];

	Observer<DownloadEvent>::NotifyMe(*mCurrentDownload);
	bool started = mCurrentDownload->Start();

	if (!started)
		{
		SingleWait::EndWait(Error, mCurrentDownload->GetErrorText());
		}
	}


/******************************************************************************
*
* NAME
*     DownloadWait::GetResult
*
* DESCRIPTION
*     Check the status of the download.
*     This must be called regularly for the wait condition to be processed.
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

WaitCondition::WaitResult DownloadWait::GetResult(void)
	{
	if (mFileIndex == -1)
		{
		return Waiting;
		}

	// Process the current download
	if (mEndResult == Waiting)
		{
		mCurrentDownload->Process();

		// If the current download is complete then start the next one.
		if (mCurrentDownload->IsDone())
			{
			Observer<DownloadEvent>::StopObserving();

			mCurrentDownload->Stop();
			mCurrentDownload.Release();

			// Advance to the next file.
			++mFileIndex;

			// If there are more files then start the next one in sequence.
			if ((unsigned)mFileIndex < mFiles.size())
				{
				mCurrentDownload = mFiles[mFileIndex];
				WWASSERT(mCurrentDownload.IsValid());
				Observer<DownloadEvent>::NotifyMe(*mCurrentDownload);

				bool started = mCurrentDownload->Start();

				if (!started)
					{
					EndWait(Error, mCurrentDownload->GetErrorText());
					}
				}
			else 
				{
				// If all of the files have downloaded successfully then the wait
				// condition has been met.
				EndWait(ConditionMet, WOLSTRING("WOL_DOWNLOADCOMPLETE"));
				}
			}
		}

	return mEndResult;
	}


/******************************************************************************
*
* NAME
*     DownloadWait::EndWait
*
* DESCRIPTION
*     Terminate the wait condition.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DownloadWait::EndWait(WaitResult endResult, const unichar_t* endText)
	{
	if ((ConditionMet != endResult) && mCurrentDownload.IsValid())
		{
		mCurrentDownload->Stop();
		}

	SingleWait::EndWait(endResult, endText);
	}


/******************************************************************************
*
* NAME
*     DownloadWait::SetCallback
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void DownloadWait::SetCallback(DownloadWaitCallback callback, void *userdata)
	{
	mCallback = callback;
	mUserdata = userdata;
	}


/******************************************************************************
*
* NAME
*     DownloadWait::DoCallback
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void DownloadWait::DoCallback(DownloadEvent& event)
	{
	if (mCallback)
		{
		mCallback(event, mUserdata);
		}
	}


/******************************************************************************
*
* NAME
*     DownloadWait::HandleNotification(DownloadEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void DownloadWait::HandleNotification(DownloadEvent& event)
	{
	if (mEndResult == Waiting)
		{
		const RefPtr<Download>& download = event.GetDownload();
		WWASSERT(download.IsValid());
		WWASSERT(mCurrentDownload == download);

		switch (event.GetEvent())
			{
			case DownloadEvent::DOWNLOAD_ERROR:
				WWDEBUG_SAY(("WOLERROR: Download Error %S\n", download->GetErrorText()));
				EndWait(Error, download->GetErrorText());
				break;
			
			case DownloadEvent::DOWNLOAD_STATUS:
				mWaitText = download->GetStatusText();
				break;
			
			case DownloadEvent::DOWNLOAD_BEGIN:
				break;

			case DownloadEvent::DOWNLOAD_PROGRESS:
				break;

			case DownloadEvent::DOWNLOAD_END:
				WWDEBUG_SAY(("WOL: Download complete\n"));
				break;

			case DownloadEvent::DOWNLOAD_STOPPED:
				WWDEBUG_SAY(("WOL: Download stopped\n"));
				EndWait(UserCancel, WOLSTRING("WOL_DOWNLOADSTOPPED"));
				break;
			
			case DownloadEvent::DOWNLOAD_RESUME:
				WWDEBUG_SAY(("WOL: Download resumed\n"));
				mWaitText = WOLSTRING("WOL_DOWNLOADRESUME");
				break;

			default:
				break;
			}

		DoCallback(event);
		}
	}

} // namespace WWOnline
