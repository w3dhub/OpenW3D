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
*     $Archive: /Commando/Code/wwui/IMEManager.cpp $
*
* DESCRIPTION
*     Input Method Editor Manager for input of far east characters.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 6 $
*     $Modtime: 1/12/02 9:44p $
*
******************************************************************************/

#include "IMEManager.h"
#include "wwstring.h"
#include "wwdebug.h"
#include <locale.h>
#include <mbctype.h>
#include <mbstring.h>

#include <algorithm>
#include <memory>

namespace IME {

/******************************************************************************
*
* NAME
*     IMEManager::Create
*
* DESCRIPTION
*     Create an IME manager instance
*
* INPUTS
*     Window - HWND to associate IME manager instance with.
*
* RESULT
*     IMEManager - Pointer to IME manager
*
******************************************************************************/

IMEManager* IMEManager::Create(HWND hwnd)
	{
	if (hwnd)
		{
		IMEManager* ime = new IMEManager;

		if (ime)
			{
			if (ime->FinalizeCreate(hwnd))
				{
				return ime;
				}

			ime->Release_Ref();
			}
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     IMEManager::IMEManager
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

IMEManager::IMEManager() :
		mHWND(NULL),
		mHIMC(NULL),
		mDisabledHIMC(NULL),
		mDisableCount(0),
		mCodePage(CP_ACP),
		mIMEProperties(0),
		mHilite(true),
		mStartCandListFrom1(true),
		mOSCanUnicode(false),
		mUseUnicode(false),
		mInComposition(false)
	{
	WWDEBUG_SAY(("IMEManager: Instantiated\n"));

	mLangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

	ResetComposition();
	mResultString[0] = 0;
	}


/******************************************************************************
*
* NAME
*     IMEManager::~IMEManager
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

IMEManager::~IMEManager()
	{
	WWDEBUG_SAY(("IMEManager: destroyed\n"));

	if (mHIMC)
		{
		ImmAssociateContext(mHWND, mDefaultHIMC);
		ImmDestroyContext(mHIMC);
		}

	mCandidateColl.clear();
	}


/******************************************************************************
*
* NAME
*     IMEManager::FinalizeCreate
*
* DESCRIPTION
*     Post creation finalization.
*
* INPUTS
*     HWND - Window to associate IME context with.
*
* RESULT
*     True if successful
*
******************************************************************************/

bool IMEManager::FinalizeCreate(HWND hwnd)
	{
	if (hwnd == NULL)
		{
		return false;
		}

	mHWND = hwnd;

	// Assume we have at least Win98 or NT4 which is the min spec
	// on the box for the original game anyhow.
	mOSCanUnicode = true;

	// Create new input context for the specified window.
	mHIMC = ImmCreateContext();
	
	if (mHIMC == NULL)
		{
		return false;
		}

	// Associate the new context with this window.
	mDefaultHIMC = ImmAssociateContext(mHWND, mHIMC);

	// Set the language for the current keyboard layout.
	InputLanguageChanged(GetKeyboardLayout(0));

	mCandidateColl.resize(32);

	return true;
	}


/******************************************************************************
*
* NAME
*     IMEManager::Activate
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

void IMEManager::Activate(void)
	{
	WWDEBUG_SAY(("IMEManager: Activate\n"));

	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		BOOL open = ImmGetOpenStatus(imc);

		if (!open)
			{
			ImmSetOpenStatus(imc, true);

			IMEEvent action(IME_ACTIVATED, this);
			NotifyObservers(action);
			}

		ImmReleaseContext(mHWND, imc);
		}
	}


/******************************************************************************
*
* NAME
*     IMEManager::Deactivate
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

void IMEManager::Deactivate(void)
	{
	WWDEBUG_SAY(("IMEManager: Deactivate\n"));

	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		BOOL open = ImmGetOpenStatus(imc);

		if (open)
			{
			ImmSetOpenStatus(imc, false);

			IMEEvent action(IME_DEACTIVATED, this);
			NotifyObservers(action);
			}

		ImmReleaseContext(mHWND, imc);
		}

	ResetComposition();
	}


/******************************************************************************
*
* NAME
*     IMEManager::IsActive
*
* DESCRIPTION
*     Check if IME is active.
*
* INPUTS
*     NONE
*
* RESULT
*     True if IME is currently active.
*
******************************************************************************/

bool IMEManager::IsActive(void) const
	{
	bool isActive = false;

	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		BOOL open = ImmGetOpenStatus(imc);
		isActive = (open != 0);
		ImmReleaseContext(mHWND, imc);
		}

	return isActive;
	}


/******************************************************************************
*
* NAME
*     IMEManager::Disable
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

void IMEManager::Disable(void)
	{
	++mDisableCount;

	// If this is the first disable lock the perform the actuall disabling.
	if (1 == mDisableCount)
		{
		WWDEBUG_SAY(("IMEManager: Disabled\n"));
		mDisabledHIMC = ImmAssociateContext(mHWND, NULL);

		IMEEvent action(IME_DISABLED, this);
		NotifyObservers(action);
		}
	}


/******************************************************************************
*
* NAME
*     IMEManager::Enable
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

void IMEManager::Enable(void)
	{
	if (mDisableCount > 0)
		{
		--mDisableCount;

		// Re-enable when there is no disable locks.
		if (0 == mDisableCount)
			{
			WWDEBUG_SAY(("IMEManager: Enabled\n"));
			ImmAssociateContext(mHWND, mDisabledHIMC);
			mDisabledHIMC = NULL;

			IMEEvent action(IME_ENABLED, this);
			NotifyObservers(action);
			}
		}
	}


/******************************************************************************
*
* NAME
*     IMEManager::IsDisabled
*
* DESCRIPTION
*     Check if IME is turned off.
*
* INPUTS
*     NONE
*
* RESULT
*     True if IME is currently off.
*
******************************************************************************/

bool IMEManager::IsDisabled(void) const
	{
	return (mDisableCount > 0);
	}


/******************************************************************************
*
* NAME
*     IMEManager::ProcessMessage
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     True of message handled and therefore should NOT be passed to the
*     default window procedure.
*
******************************************************************************/

bool IMEManager::ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
	{
	if (hwnd != mHWND)
		{
		return false;
		}

	bool handled = true;
	outResult = 0;

	switch (msg)
		{
		// Request New keyboard layout and / or Input method
		case WM_INPUTLANGCHANGEREQUEST:
			{
			HKL layout = InputLanguageChangeRequest((HKL)lParam);

			if (layout)
				{
				lParam = (LPARAM)layout;
				handled = false;
				}
			}
			break;

		// Input language has changed.
		case WM_INPUTLANGCHANGE:
			InputLanguageChanged((HKL)lParam);
			outResult = true;
			handled = false;
			break;

		// Sent when the system is about to change the current IME.
		case WM_IME_SELECT:
			WWDEBUG_SAY(("IMEManager: WM_IME_SELECT\n"));
			break;

		// We will handle all of the UI so clear all of the flags.
		case WM_IME_SETCONTEXT:
			lParam &= ~(ISC_SHOWUIALL);
			handled = false;
			break;

		// Sent when a composition string is about to be generated in response to a
		// keystroke. This message informs us to prepare for composition.
		case WM_IME_STARTCOMPOSITION:
			StartComposition();
			break;

		// Sent when composition status has changed in response to a keystroke.
		case WM_IME_COMPOSITION:
			DoComposition(wParam, lParam);
			break;

		// Sent when composition has closed.
		case WM_IME_ENDCOMPOSITION:
			EndComposition();
			break;

		// Sent when unable to extend the composition to accomodate any more characters.
		case WM_IME_COMPOSITIONFULL:
			{
			WWDEBUG_SAY(("IMEManager: WM_IME_COMPOSITIONFULL\n"));
			CompositionEvent event(COMPOSITION_FULL, this);
			NotifyObservers(event);
			}
			break;

		// Sent when the IME status has changed.
		case WM_IME_NOTIFY:
			outResult = IMENotify(wParam, lParam);
			break;

		// IMEs send this message when the user accepts the conversion string.
		// wParam contains a single-byte or double-byte character.
		case WM_IME_CHAR:
			handled = IMECharHandler((unsigned short)wParam);

			if (handled)
				{
				outResult = true;
				}
			break;

		case WM_CHAR:
			handled = CharHandler((unsigned short)wParam);

			if (handled)
				{
				outResult = true;
				}
			break;

		case WM_KEYDOWN:
			if (mInComposition)
				{
				outResult = DefWindowProc(hwnd, msg, wParam, lParam);
				}
			else
				{
				handled = false;
				}
			break;

		case WM_KEYUP:
			if (mInComposition)
				{
				#ifdef SHOW_IME_TYPING

				bool typingChanged = false;
				UINT virtualKey = wParam;

				if (VK_BACK == virtualKey)
					{
					if (mTypingCursorPos > 0)
						{
						--mTypingCursorPos;
						mTypingString[mTypingCursorPos] = 0;
						typingChanged = true;
						}
					}
				else if ((0x30 <= virtualKey && 0x39 >= virtualKey) || (0x41 <= virtualKey && 0x5A >= virtualKey))
					{
					if (mTypingCursorPos < IME_MAX_TYPING_LEN)
						{
						mTypingString[mTypingCursorPos] = (unichar_t)virtualKey;
						++mTypingCursorPos;
						mTypingString[mTypingCursorPos] = 0;
						typingChanged = true;
						}
					}

				if (typingChanged)
					{
					CompositionEvent event(COMPOSITION_TYPING, this);
					NotifyObservers(event);
					}
				#endif

				outResult = DefWindowProc(hwnd, msg, wParam, lParam);
				}
			else
				{
				handled = false;
				}
			break;

		case WM_IME_CONTROL:
		case WM_IME_KEYDOWN:
		case WM_IME_KEYUP:
		default:
			handled = false;
			break;
		}

	return handled;
	}


/******************************************************************************
*
* NAME
*     IMEManager::IMENotify
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

LRESULT IMEManager::IMENotify(WPARAM wParam, LPARAM lParam)
	{
	switch (wParam)
		{
		// The open status of the input context has changed.
		case IMN_SETOPENSTATUS:
			WWDEBUG_SAY(("IMEManager: IMN_SETOPENSTATUS\n"));
			{
			HIMC imc = ImmGetContext(mHWND);

			if (imc)
				{
				IMEAction action = IME_DEACTIVATED;

				if (!ImmGetOpenStatus(imc))
					{
					// If the IME conversion engine is closed, we need to
					// erase all currently displayed composition chars and any
					// candidate windows.
					mCandidateColl.clear();
					ResetComposition();
					}
				else
					{
					action = IME_ACTIVATED;
					}

				ImmReleaseContext(mHWND, imc);

				IMEEvent event(action, this);
				NotifyObservers(event);
				}
			}
			break;

		// Open the status window
		case IMN_OPENSTATUSWINDOW:
			WWDEBUG_SAY(("IMEManager: IMN_OPENSTATUSWINDOW\n"));
			break;

		// Close the status window
		case IMN_CLOSESTATUSWINDOW:
			WWDEBUG_SAY(("IMEManager: IMN_CLOSESTATUSWINDOW\n"));
			break;

		// Update the position of the status window.
		case IMN_SETSTATUSWINDOWPOS:
			WWDEBUG_SAY(("IMEManager: IMN_SETSTATUSWINDOWPOS\n"));
			break;

		// The font of the input context has changed.
		case IMN_SETCOMPOSITIONFONT:
			WWDEBUG_SAY(("IMEManager: IMN_SETCOMPOSITIONFONT\n"));
			break;

		// The style or position of the composition window has changed.
		case IMN_SETCOMPOSITIONWINDOW:
			WWDEBUG_SAY(("IMEManager: IMN_SETCOMPOSITIONWINDOW\n"));
			break;

		// The conversion mode of the input context has changed.
		case IMN_SETCONVERSIONMODE:
			WWDEBUG_SAY(("IMEManager: IMN_SETCONVERSIONMODE\n"));

#ifdef WWDEBUG
			{
			HIMC imc = ImmGetContext(mHWND);

			if (imc)
				{
				DWORD conversionMode = 0;
				DWORD sentenceMode = 0;

				BOOL okay = ImmGetConversionStatus(imc, &conversionMode, &sentenceMode);

				if (okay)
					{
					WWDEBUG_SAY(("IMEManager: ConversionMode - "));

					static struct convmodestruct {int flag; const char* ondesc; const char* offdesc;} _convModes[] = 
						{
						{IME_CMODE_CHARCODE, "CharCode:On", "CharCode:Off"},
						{IME_CMODE_EUDC, " EUDC:On", " EUDC:Off"},
						{IME_CMODE_FIXED, " Fixed:On", " Fixed:Off"},
						{IME_CMODE_FULLSHAPE, " FullShape", " HalfShape"},
						{IME_CMODE_HANJACONVERT, " Hanja:On", " Hanja:Off"},
						{IME_CMODE_KATAKANA, " Katakana", " Hiragana"},
						{IME_CMODE_NATIVE, " Native", " Alphanumberic"},
						{IME_CMODE_NOCONVERSION, " Conversion:Off", " Conversion:On"},
						{IME_CMODE_ROMAN, " Roman:On", " Roman:Off"},
						{IME_CMODE_SOFTKBD, " SoftKbd:On", " SoftKbd:Off"},
						{IME_CMODE_SYMBOL, " Symbol:On", " Symbol:Off"},
						{0, ""}
						};

					int flgidx = 0;

					while (_convModes[flgidx].flag)
						{
						if (conversionMode & _convModes[flgidx].flag)
							{
							WWDEBUG_SAY((_convModes[flgidx].ondesc));
							}
						else
							{
							WWDEBUG_SAY((_convModes[flgidx].offdesc));
							}

						flgidx++;
						}

					WWDEBUG_SAY(("\n"));
					}

				ImmReleaseContext(mHWND, imc);
				}
			}
#endif
			break;

		// The sentence mode has changed.
		case IMN_SETSENTENCEMODE:
			WWDEBUG_SAY(("IMEManager: IMN_SETSENTENCEMODE\n"));

#ifdef WWDEBUG
			{
			HIMC imc = ImmGetContext(mHWND);

			if (imc)
				{
				DWORD conversionMode = 0;
				DWORD sentenceMode = 0;

				BOOL okay = ImmGetConversionStatus(imc, &conversionMode, &sentenceMode);

				if (okay)
					{
					WWDEBUG_SAY(("IMEManager: SentenceMode - "));

					static struct smodestruct {int flag; const char* ondesc; const char* offdesc;} _sModes[] = 
						{
						{IME_SMODE_AUTOMATIC, "Automatic:On", "Automatic:Off"},
						{IME_SMODE_NONE, " SentenceInfo:Off", " SentenceInfo:On"},
						{IME_SMODE_PHRASEPREDICT, " PhrasePredict:On", " PhrasePredict:Off"},
						{IME_SMODE_PLAURALCLAUSE, " PluralClause:On", " PluralClause:Off"},
						{IME_SMODE_SINGLECONVERT, " SingleConvert:On", " SingleConvert:Off"},
						{IME_SMODE_CONVERSATION, " Conversation:On", " Conversation:Off"},
						{0, ""}
						};

					int flgidx = 0;

					while (_sModes[flgidx].flag)
						{
						if (sentenceMode & _sModes[flgidx].flag)
							{
							WWDEBUG_SAY((_sModes[flgidx].ondesc));
							}
						else
							{
							WWDEBUG_SAY((_sModes[flgidx].offdesc));
							}

						flgidx++;
						}

					WWDEBUG_SAY(("\n"));
					}

				ImmReleaseContext(mHWND, imc);
				}
			}
#endif
			break;

		// Open the candidate window (lParam = candidate flags)
		case IMN_OPENCANDIDATE:
			OpenCandidate(lParam);
			break;

		// Close the candidate window. (lParam = candidate flags)
		case IMN_CLOSECANDIDATE:
			CloseCandidate(lParam);
			break;

		// Changing the contents of the candidate window (lParam = candidate flags)
		case IMN_CHANGECANDIDATE:
			ChangeCandidate(lParam);
			break;

		// Candidate processing is finished; moving the candidate window
		case IMN_SETCANDIDATEPOS:
			WWDEBUG_SAY(("IMEManager: IMN_SETCANDIDATEPOS\n"));
			break;

		// Show error message or other information
		case IMN_GUIDELINE:
			WWDEBUG_SAY(("IMEManager: IMN_GUIDELINE\n"));
			{
			IMEEvent event(IME_GUIDELINE, this);
			NotifyObservers(event);
			}
			break;

		default:
			break;
		}
	
	return true;
	}


/******************************************************************************
*
* NAME
*     IMEManager::InputLanguageChangeRequest
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

HKL IMEManager::InputLanguageChangeRequest(HKL hkl)
	{
	WWDEBUG_SAY(("IMEManager: Input language change request\n"));

	// Get the number of Keyboard layouts available to the system
	UINT numLayouts = GetKeyboardLayoutList(0, NULL);

	if (numLayouts)
		{
		// Get the list of layouts
		std::vector<HKL> layoutList(numLayouts);
		layoutList.resize(numLayouts);

		numLayouts = GetKeyboardLayoutList(numLayouts, &(*layoutList.begin()));

		// Find the position in the list of the layout which has been requested.
		std::vector<HKL>::iterator iter = std::find(layoutList.begin(), layoutList.end(), hkl);

		if (iter != layoutList.end())
			{
			// Rotate the list so the requested layout is at the head.
			std::rotate(layoutList.begin(), iter, layoutList.end());

			// Look for the layout that doesn't have the  AT_CARET or SPECIAL_UI properties
			iter = layoutList.begin();

			while (iter != layoutList.end())
				{
				DWORD property = ImmGetProperty(*iter, IGP_PROPERTY);

				if ((property & (IME_PROP_AT_CARET | IME_PROP_SPECIAL_UI)) == (IME_PROP_AT_CARET | IME_PROP_SPECIAL_UI))
					{
					iter++;
					}
				else
					{
					return *iter;
					}
				}
			}
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     IMEManager::InputLanguageChanged
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void IMEManager::InputLanguageChanged(HKL hkl)
	{
	mLangID = LOWORD(hkl);
	DWORD lcid = MAKELCID(mLangID, SORT_DEFAULT);

	// Get the default codepage for this input language
	char localeData[8];
	GetLocaleInfoA(lcid, LOCALE_IDEFAULTANSICODEPAGE, localeData, sizeof(localeData));

	// Set the codepage for character conversion
	mCodePage = atoi(localeData);

	// Get properties
	mIMEProperties = ImmGetProperty(hkl, IGP_PROPERTY);

	mStartCandListFrom1 = ((mIMEProperties & IME_PROP_CANDLIST_START_FROM_1) == IME_PROP_CANDLIST_START_FROM_1);
	mUseUnicode = (mOSCanUnicode && (mIMEProperties & IME_PROP_UNICODE));

	// Get IME description
	if (mUseUnicode)
		{
		UINT descSize = ImmGetDescriptionW(hkl, NULL, 0);
		++descSize;
		unichar_t* descPtr = mIMEDescription.Get_Buffer(descSize);

		ImmGetDescriptionW(hkl, reinterpret_cast<wchar_t *>(descPtr), descSize);
		}
	else
		{
		UINT descSize = ImmGetDescriptionA(hkl, NULL, 0);
		++descSize;

		StringClass desc((int)descSize, true);
		char* descPtr = desc.Get_Buffer(descSize);

		ImmGetDescriptionA(hkl, descPtr, descSize);
		mIMEDescription = desc;
		}

	#if(0)
	mHilite = true;

	static const unichar_t _TradChImeName[] = {0x6CE8,0x97F3,0x8F38,0x5165,0x6CD5,0x0020,0x0034,0x002E,0x0031,0x0020,0x7248,0x0000};

	if (mIMEDescription.Compare(_TradChImeName) == 0)
		{
		mHilite = false;
		}
	#endif

	WWDEBUG_SAY(("IMEManager: Language Changed - LangID = %04X, CodePage = %d, Description: '%S'\n",
			mLangID, mCodePage, mIMEDescription));

	WWDEBUG_SAY(("IMEManager: Properties - %s%s%s%s%s\n",
			mIMEProperties & IME_PROP_AT_CARET ? "At Caret" : "",
			mIMEProperties & IME_PROP_SPECIAL_UI ? ", SpecialUI" : "",
			mIMEProperties & IME_PROP_CANDLIST_START_FROM_1 ? ", CandlistFrom1" : "",
			mIMEProperties & IME_PROP_UNICODE ? ", Unicode" : "",
			mIMEProperties & IME_PROP_COMPLETE_ON_UNSELECT ? ", Complete on Unselect" : ""));

	IMEEvent action(IME_LANGUAGECHANGED, this);
	NotifyObservers(action);
	}


/******************************************************************************
*
* NAME
*     IMEManager::ResetComposition
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void IMEManager::ResetComposition(void)
	{
	mInComposition = false;

#ifdef SHOW_IME_TYPING
	mTypingString[0] = 0;
	mTypingCursorPos = 0;
#endif

	mCompositionString[0] = 0;
	memset(mCompositionAttr, 0, sizeof(mCompositionAttr));
	memset(mCompositionClause, 0, sizeof(mCompositionClause));
	mCompositionCursorPos = 0;

	mReadingString[0] = 0;
	}


/******************************************************************************
*
* NAME
*     IMEManager::StartComposition
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void IMEManager::StartComposition(void)
	{
	WWDEBUG_SAY(("IMEManager: StartComposition\n"));

	mInComposition = true;
	mResultString[0] = 0;

	CompositionEvent event(COMPOSITION_START, this);
	NotifyObservers(event);
	}


/******************************************************************************
*
* NAME
*     IMEManager::DoComposition
*
* DESCRIPTION
*     Handle composition message.
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void IMEManager::DoComposition(unsigned int /* dbcs */, int compFlags)
	{
	WWDEBUG_SAY(("IMEManager: DoComposition\n"));

#if(0)
	#ifdef _DEBUG
	static struct flagstruct {int flag; const char* desc;} _gcsFlags[] = 
		{
		{GCS_COMPATTR, "GCS_COMPATTR"},
		{GCS_COMPCLAUSE, " GCS_COMPCLAUSE"},
		{GCS_COMPREADATTR, " GCS_COMPREADATTR"},
		{GCS_COMPREADCLAUSE, " GCS_COMPREADCLAUSE"},
		{GCS_DELTASTART, " GCS_DELTASTART"},
		{GCS_RESULTCLAUSE, " GCS_RESULTCLAUSE"},
		{GCS_RESULTREADCLAUSE, " GCS_RESULTREADCLAUSE"},
		{GCS_RESULTREADSTR, " GCS_RESULTREADSTR"},
		{CS_INSERchar, " CS_INSERchar"},
		{CS_NOMOVECARET, " CS_NOMOVECARET"},
		{0, ""}
		};

	WWDEBUG_SAY(("Flags: "));

	int flgidx = 0;

	while (_gcsFlags[flgidx].flag)
		{
		if (compFlags & _gcsFlags[flgidx].flag)
			{
			WWDEBUG_SAY((_gcsFlags[flgidx].desc));
			}

		flgidx++;
		}

	WWDEBUG_SAY(("\n"));
	#endif
#endif

#ifdef SHOW_IME_TYPING
	// Reset the typing string
	mTypingString[0] = 0;
	mTypingCursorPos = 0;
	CompositionEvent event(COMPOSITION_TYPING, this);
	NotifyObservers(event);
#endif

	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		if (compFlags == 0)
			{
			ResetComposition();
			CompositionEvent event(COMPOSITION_CANCEL, this);
			NotifyObservers(event);
			}
		else if (compFlags & GCS_RESULTSTR)
			{
			// Retrieve the result string
			if (ReadCompositionString(imc, GCS_RESULTSTR, mResultString, sizeof(mResultString)))
				{
				WWDEBUG_SAY(("Result string '%S'\n", mResultString));

				CompositionEvent event(COMPOSITION_RESULT, this);
				NotifyObservers(event);
				}
			}
		else
			{
			CompositionAction action = COMPOSITION_INVALID;

			// Update reading string.
			if (compFlags & GCS_COMPREADSTR)
				{
				if (ReadCompositionString(imc, GCS_COMPREADSTR, mReadingString, sizeof(mReadingString)))
					{
					WWDEBUG_SAY(("Reading string '%S'\n", (const unichar_t*)mReadingString));
					action = COMPOSITION_CHANGE;
					}
				}

#ifdef WWDEBUG
			if (compFlags & GCS_COMPREADATTR)
				{
				unsigned char attr[IME_MAX_STRING_LEN * 2];
				int size = ReadReadingAttr(imc, attr, sizeof(attr));

				WWDEBUG_SAY(("ReadAttr: "));
				
				for (int index = 0; index < size; ++index)
					{
					WWDEBUG_SAY(("%01x", (int)attr[index]));
					}
				
				WWDEBUG_SAY(("\n"));
				}
#endif

			// Update composition string.
			if (compFlags & GCS_COMPSTR)
				{
				if (ReadCompositionString(imc, GCS_COMPSTR, mCompositionString, sizeof(mCompositionString)))
					{
					WWDEBUG_SAY(("Composition string '%S'\n", (const unichar_t*)mCompositionString));
					action = COMPOSITION_CHANGE;
					}
				}

			if (compFlags & GCS_COMPATTR)
				{
				[[maybe_unused]] int size = ReadCompositionAttr(imc, mCompositionAttr, sizeof(mCompositionAttr));

#ifdef WWDEBUG
				WWDEBUG_SAY(("CompAttr: "));

				for (int index = 0; index < size; ++index)
					{
					WWDEBUG_SAY(("%01x", (int)mCompositionAttr[index]));
					}
				
				WWDEBUG_SAY(("\n"));
#endif
				}

			if (compFlags & GCS_COMPCLAUSE)
				{
				mCompositionClause[0] = 0;
				[[maybe_unused]] int size = ReadCompositionClause(imc, mCompositionClause, sizeof(mCompositionClause));

#ifdef WWDEBUG
				WWDEBUG_SAY(("CompClause: "));
				const int count = (size / sizeof(unsigned int));

				for (int index = 0; index < count; ++index)
					{
					WWDEBUG_SAY(("%u ", mCompositionClause[index]));
					}

				WWDEBUG_SAY(("\n"));
#endif
				}

			if (compFlags & GCS_CURSORPOS)
				{
				mCompositionCursorPos = ReadCursorPos(imc);
				action = COMPOSITION_CHANGE;
				}

			if (action != COMPOSITION_INVALID)
				{
				CompositionEvent event(action, this);
				NotifyObservers(event);
				}
			}

		ImmReleaseContext(mHWND, imc);
		}
	}


/******************************************************************************
*
* NAME
*     IMEManager::EndComposition
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void IMEManager::EndComposition(void)
	{
	WWDEBUG_SAY(("IMEManager: EndComposition\n"));

	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		ReadCompositionString(imc, GCS_COMPSTR, mResultString, sizeof(mResultString));
		ImmReleaseContext(mHWND, imc);
		}

	ResetComposition();
	CompositionEvent event(COMPOSITION_END, this);
	NotifyObservers(event);
	}


/******************************************************************************
*
* NAME
*     IMEManager::ReadCompositionString
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

bool IMEManager::ReadCompositionString(HIMC imc, unsigned int flag, unichar_t* buffer, int length)
	{
	if (mUseUnicode)
		{
		LONG size = ImmGetCompositionStringW(imc, flag, buffer, length);

		if (size < 0)
			{
			buffer[0] = 0;
			return false;
			}
		
		// Terminate string
		buffer[(size / sizeof(unichar_t))] = 0;
		}
	else
		{
		// Read the string as multibyte ANSI
		unsigned char string[IME_MAX_STRING_LEN];
		LONG size = ImmGetCompositionString(imc, flag, string, sizeof(string));

		if (size < 0)
			{
			buffer[0] = 0;
			return false;
			}

		// Terminate the string
		string[size] = 0;

		// Convert to Unicode
		MultiByteToWideChar(mCodePage, 0, (const char*)string, -1, reinterpret_cast<wchar_t *>(buffer), (length / sizeof(unichar_t)));
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     IMEManager::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

int IMEManager::ReadReadingAttr(HIMC imc, unsigned char* attr, int length)
	{
	if (mUseUnicode)
		{
		LONG size = ImmGetCompositionStringW(imc, GCS_COMPREADATTR, attr, length);
		return (size / sizeof(unichar_t));
		}

	// Read the string as multibyte ANSI
	unsigned char string[IME_MAX_STRING_LEN];
	LONG size = ImmGetCompositionString(imc, GCS_COMPREADSTR, string, sizeof(string));

	if (size <= 0)
		{
		return 0;
		}

	// Terminate the string
	string[size] = 0;
		
	LONG attrSize = ImmGetCompositionString(imc, GCS_COMPREADATTR, attr, length);
	WWASSERT(size == attrSize);

	if (attrSize <= size)
		{
		return 0;
		}

	return ConvertAttrForUnicode(string, attr);
	}


/******************************************************************************
*
* NAME
*     IMEManager::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

int IMEManager::ReadReadingClause(HIMC imc, unsigned int* clause, int length)
	{
	if (mUseUnicode)
		{
		LONG size = ImmGetCompositionStringW(imc, GCS_COMPREADCLAUSE, clause, length);
		return (size / sizeof(unichar_t));
		}

	// Read the string as multibyte ANSI
	unsigned char string[IME_MAX_STRING_LEN];
	LONG size = ImmGetCompositionString(imc, GCS_COMPREADSTR, string, sizeof(string));

	if (size <= 0)
		{
		return 0;
		}

	// Terminate the string
	string[size] = 0;

	LONG clauseSize = ImmGetCompositionString(imc, GCS_COMPREADCLAUSE, clause, length);

	if (clauseSize <= 0)
		{
		return 0;
		}
	
	return ConvertClauseForUnicode(string, size, clause);
	}


/******************************************************************************
*
* NAME
*     IMEManager::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

int IMEManager::ReadCompositionAttr(HIMC imc, unsigned char* attr, int length)
	{
	if (mUseUnicode)
		{
		return ImmGetCompositionStringW(imc, GCS_COMPATTR, attr, length);
		}

	// Read the string as multibyte ANSI
	unsigned char string[IME_MAX_STRING_LEN];
	LONG size = ImmGetCompositionString(imc, GCS_COMPSTR, string, sizeof(string));

	if (size <= 0)
		{
		return size;
		}

	// Terminate the string
	string[size] = 0;
		
	LONG attrSize = ImmGetCompositionString(imc, GCS_COMPATTR, attr, length);
	WWASSERT(size == attrSize);

	if (attrSize <= size)
		{
		return 0;
		}

	return ConvertAttrForUnicode(string, attr);
	}


/******************************************************************************
*
* NAME
*     IMEManager::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

int IMEManager::ReadCompositionClause(HIMC imc, unsigned int* clause, int length)
	{
	if (mUseUnicode)
		{
		return ImmGetCompositionStringW(imc, GCS_COMPCLAUSE, clause, length);
		}

	// Read the string as multibyte ANSI
	unsigned char string[IME_MAX_STRING_LEN];
	LONG size = ImmGetCompositionString(imc, GCS_COMPSTR, string, sizeof(string));

	if (size <= 0)
		{
		return size;
		}

	// Terminate the string
	string[size] = 0;

	LONG clauseSize = ImmGetCompositionString(imc, GCS_COMPCLAUSE, clause, length);

	if (clauseSize <= 0)
		{
		return 0;
		}
	
	return ConvertClauseForUnicode(string, size, clause);
	}


/******************************************************************************
*
* NAME
*     IMEManager::ReadCursorPos
*
* DESCRIPTION
*     Read the composition string cursor position.
*
* INPUTS
*
* RESULT
*
******************************************************************************/

int IMEManager::ReadCursorPos(HIMC imc)
	{
	if (mUseUnicode)
		{
		int cursorPos = ImmGetCompositionStringW(imc, GCS_CURSORPOS, NULL, 0);
		return (cursorPos & 0x0000FFFF);
		}

	// Get the multibyte string
	char string[IME_MAX_STRING_LEN];
	LONG size = ImmGetCompositionString(imc, GCS_COMPSTR, string, sizeof(string));

	if (size < 0)
		{
		return 0;
		}

	string[size] = 0;

	int cursorPos = ImmGetCompositionString(imc, GCS_CURSORPOS, NULL, 0);
	cursorPos = (cursorPos & 0x0000FFFF);

	// Convert multibyte character position in unicode position.
	return static_cast<long>(_mbsnccnt(reinterpret_cast<const unsigned char*>(string), cursorPos));
	}


/******************************************************************************
*
* NAME
*     IMEManager::GetTargetClause
*
* DESCRIPTION
*     Get the composition string conversion target range. This is the characters
*     that are currently be considered for conversion.
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void IMEManager::GetTargetClause(unsigned int& start, unsigned int& end)
	{
	int index = 0;
	const size_t compLength = ::u_strlen(mCompositionString);

	while (mCompositionClause[index] < compLength)
		{
		unsigned int offset = mCompositionClause[index];

		if (ATTR_TARGET_CONVERTED == mCompositionAttr[offset])
			{
			start = offset;
			end = mCompositionClause[index + 1];
			return;
			}

		++index;
		}

	start = 0;
	end = 0;
	}


/******************************************************************************
*
* NAME
*     IMEManager::GetCompositionFont
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

bool IMEManager::GetCompositionFont(LPLOGFONT lpFont)
	{
	BOOL success = false;
	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		success = ImmGetCompositionFont(imc, lpFont);
		ImmReleaseContext(mHWND, imc);
		}

	return (success != 0);
	}


/******************************************************************************
*
* NAME
*     IMEManager::OpenCandidate
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void IMEManager::OpenCandidate(unsigned int candList)
	{
	WWDEBUG_SAY(("IMEManager: OpenCandidate\n"));

	for (int index = 0; index < 32; index++)
		{
		if (candList & (1 << index))
			{
			IMECandidate& candidate = mCandidateColl[index];
			candidate.Open(index, mHWND, mCodePage, mUseUnicode, mStartCandListFrom1);
			candidate.Read();

			CandidateEvent event(CANDIDATE_OPEN, &candidate);
			NotifyObservers(event);
			}
		}
	}


/******************************************************************************
*
* NAME
*     IMEManager::ChangeCandidate
*
* DESCRIPTION
*     The contents of the candidate list has changed.
*
* INPUTS
*     Changed - Bitfield of the candidate lists that have changed.
*
* RESULT
*     NONE
*
******************************************************************************/

void IMEManager::ChangeCandidate(unsigned int candList)
	{
	WWDEBUG_SAY(("IMEManager: ChangeCandidate\n"));

	for (int index = 0; index < 32; index++)
		{
		if (candList & (1 << index))
			{
			IMECandidate& candidate = mCandidateColl[index];
			candidate.Read();
			CandidateEvent event(CANDIDATE_CHANGE, &candidate);
			NotifyObservers(event);
			}
		}
	}

	
/******************************************************************************
*
* NAME
*     IMEManager::CloseCandidate
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void IMEManager::CloseCandidate(unsigned int candList)
	{
	WWDEBUG_SAY(("IMEManager: CloseCandidate\n"));

	for (int index = 0; index < 32; index++)
		{
		if (candList & (1 << index))
			{
			IMECandidate& candidate = mCandidateColl[index];
			CandidateEvent event(CANDIDATE_CLOSE, &candidate);
			NotifyObservers(event);
			candidate.Close();
			}
		}
	}


/******************************************************************************
*
* NAME
*     IMEManager::GetGuideLine
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

unsigned int IMEManager::GetGuideline(unichar_t* outString, int length)
	{
	unsigned int level = GL_LEVEL_NOGUIDELINE;

	HIMC imc = ImmGetContext(mHWND);

	if (imc)
		{
		level = ImmGetGuideLineA(imc, GGL_LEVEL, NULL, 0);

		if ((GL_LEVEL_NOGUIDELINE != level) && outString)
			{
			if (mUseUnicode)
				{
				DWORD size = ImmGetGuideLineW(imc, GGL_STRING, reinterpret_cast<wchar_t *>(outString), (length * sizeof(unichar_t)));
				WWASSERT(size <= (DWORD)length);
				outString[size / sizeof(unichar_t)] = 0;
				}
			else
				{
				char temp[512];
				DWORD size = ImmGetGuideLineA(imc, GGL_STRING, temp, sizeof(temp));
				temp[size] = 0;

				MultiByteToWideChar(mCodePage, 0, temp, -1, reinterpret_cast<wchar_t *>(outString), length);
				outString[length] = 0;
				}
			}

		ImmReleaseContext(mHWND, imc);
		}

	return level;
	}


/******************************************************************************
*
* NAME
*     IMEManager::IMECharHandler
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     True if character converted.
*
******************************************************************************/

bool IMEManager::IMECharHandler(unsigned short dbcs)
	{
	unsigned int mbcs = dbcs;

	// If this char has a lead byte then it is double byte. Swap the bytes
	// for generate string order
	if (dbcs & 0xFF00)
		{
		mbcs = (((dbcs & 0xFF) << 8) | (dbcs >> 8));
		}

	// Convert char to unicode
	unichar_t unicode = 0;
	MultiByteToWideChar(mCodePage, 0, (const char*)&mbcs, -1, reinterpret_cast<wchar_t *>(&unicode), 1);

	UnicodeChar event(unicode);
	NotifyObservers(event);

	return true;
	}


/******************************************************************************
*
* NAME
*     IMEManager::CharHandler
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     True if character handled.
*
******************************************************************************/

bool IMEManager::CharHandler(unsigned short ch)
	{
	// Because DBCS characters are usually generated by IMEs (as two PostMessages),
	// if a lead byte comes in, the trail byte should arrive very soon after.
	// We wait here for the trail byte and store them into the text buffer together.
	if (!IsDBCSLeadByte((unsigned char)ch))
		{
		return false;
		}

	// Wait an arbitrary amount of time for the trail byte to arrive.
	// If it doesn't, then discard the lead byte. This could happen if the IME
	// screwed up.  Or, more likely, the user generated the lead byte through ALT-numpad.
	MSG msg;
	int i = 10;

	while (!PeekMessage(&msg, mHWND, WM_CHAR, WM_CHAR, PM_REMOVE))
		{
		if (--i == 0)
			{
			return true;
			}

		Sleep(0);
		}

	// Convert char to unicode.
	unsigned int dbcs = (unsigned int)(((unsigned)msg.wParam << 8) | ch);
	unichar_t unicode = 0;
	MultiByteToWideChar(mCodePage, 0, (const char*)&dbcs, 2, reinterpret_cast<wchar_t *>(&unicode), 1);

	UnicodeChar event(unicode);
	NotifyObservers(event);

	return true;
	}


/******************************************************************************
*
* NAME
*     IMEManager::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

int IMEManager::ConvertAttrForUnicode(unsigned char* mbcs, unsigned char* attr)
	{
	// Scale the attributes for unicode string length
	unsigned char* mbsPtr = mbcs;
	unsigned char* attrPtr = attr;

	while (*mbsPtr != 0)
		{
		*attrPtr = attr[mbsPtr - mbcs];
		++attrPtr;
		mbsPtr = _mbsinc(mbsPtr);
		}

	return (attrPtr - attr);
	}


/******************************************************************************
*
* NAME
*     IMEManager::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

int IMEManager::ConvertClauseForUnicode(unsigned char* mbcs, int length, unsigned int* clause)
	{
	//---------------------------------------------------------------------------
	// Scale the clause offsets for unicode string
	//---------------------------------------------------------------------------
	unsigned char* mbsPtr = mbcs;
	unsigned int offset = 0;

	// The first clause is always zero so there is no need to adjust it.
	int index = 1;

	// The clause is terminated with the size of the string
	while (clause[index] < (unsigned int)length)
		{
		// Count the number of characters in this clause
		unsigned char* mbsStop = (mbcs + clause[index]);

		while (mbsPtr < mbsStop)
			{
			++offset;
			mbsPtr = _mbsinc(mbsPtr);
			}

		clause[index] = offset;
		++index;
		}

	// Terminate the unicode adjusted clause with the string length
	clause[index] = static_cast<unsigned long>(_mbslen(mbcs));
	++index;

	return (&clause[index] - clause);
	}

} // namespace IME
