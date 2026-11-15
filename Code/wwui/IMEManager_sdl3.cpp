/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D Developers
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
* DESCRIPTION
*     Input Method Editor Manager implemented on top of SDL3 text input.
*
*     This mirrors the notifications sent by the Win32 IMM implementation in
*     IMEManager.cpp so EditCtrlClass and IMECandidateCtrl work unchanged:
*
*       SDL_EVENT_TEXT_EDITING            -> COMPOSITION_START / CHANGE / END
*       SDL_EVENT_TEXT_INPUT (composing)  -> COMPOSITION_RESULT, COMPOSITION_END
*       SDL_EVENT_TEXT_EDITING_CANDIDATES -> CANDIDATE_OPEN / CHANGE / CLOSE
*
*     Text input outside of a composition is left to WWUIInputClass, the same
*     way WM_CHAR is in the Win32 implementation.
*
*     The application must set SDL_HINT_IME_IMPLEMENTED_UI to
*     "composition,candidates" so SDL sends the composition and candidate
*     events rather than showing the system IME windows.
*
******************************************************************************/

#include "IMEManager.h"
#include "wwdebug.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_properties.h>

#include <algorithm>

namespace IME {

// UTF-8 code page identifier (CP_UTF8)
static const uint32_t SDL_IME_CODE_PAGE = 65001;


/******************************************************************************
*
* NAME
*     Copy_String
*
* DESCRIPTION
*     Copy a UTF-8 string into a fixed size unichar_t buffer.
*
******************************************************************************/

static void Copy_String(const char* utf8, unichar_t* buffer, size_t length)
{
	std::basic_string<unichar_t> text = WWUIPlatform::UTF8_To_Unichar(utf8);

	size_t count = std::min(text.length(), length - 1);
	std::copy(text.begin(), text.begin() + count, buffer);
	buffer[count] = 0;
}


IMEManager* IMEManager::Create(WWUIWindowHandle hwnd)
{
	IMEManager* ime = new IMEManager;

	if (ime->FinalizeCreate(hwnd))
	{
		return ime;
	}

	ime->Release_Ref();
	return nullptr;
}


IMEManager::IMEManager() :
		mTextInputWindow(nullptr),
		mActive(false),
		mTextInputPassword(false),
		mCandidatesOpen(false),
		mTargetStart(0),
		mTargetEnd(0),
		mDisableCount(0),
		mLangID(0),
		mCodePage(SDL_IME_CODE_PAGE),
		mStartCandListFrom1(true),
		mInComposition(false)
{
	WWDEBUG_SAY(("IMEManager[SDL3]: Instantiated\n"));

	ResetComposition();
	mResultString[0] = 0;
}


IMEManager::~IMEManager()
{
	WWDEBUG_SAY(("IMEManager[SDL3]: destroyed\n"));

	if (mTextInputWindow)
	{
		SDL_StopTextInput(mTextInputWindow);
		mTextInputWindow = nullptr;
	}

	mCandidateColl.clear();
}


bool IMEManager::FinalizeCreate(WWUIWindowHandle /* hwnd */)
{
	// SDL tracks text input per SDL_Window. The window is looked up when text
	// input is started, so the native handle is not needed.
	mCandidateColl.resize(1);
	return true;
}


void IMEManager::Activate(void)
{
	WWDEBUG_SAY(("IMEManager[SDL3]: Activate\n"));

	if (!mActive)
	{
		mActive = true;
		UpdateTextInput();

		IMEEvent action(IME_ACTIVATED, this);
		NotifyObservers(action);
	}
}


void IMEManager::Deactivate(void)
{
	WWDEBUG_SAY(("IMEManager[SDL3]: Deactivate\n"));

	if (mActive)
	{
		// Close any open composition so the observers clean up their state.
		DoCandidates(nullptr, 0, -1);

		if (mInComposition)
		{
			EndComposition();
		}

		mActive = false;
		UpdateTextInput();

		IMEEvent action(IME_DEACTIVATED, this);
		NotifyObservers(action);
	}

	ResetComposition();
}


bool IMEManager::IsActive(void) const
{
	return mActive;
}


void IMEManager::Disable(void)
{
	++mDisableCount;

	if (1 == mDisableCount)
	{
		WWDEBUG_SAY(("IMEManager[SDL3]: Disabled\n"));
		UpdateTextInput();

		IMEEvent action(IME_DISABLED, this);
		NotifyObservers(action);
	}
}


void IMEManager::Enable(void)
{
	if (mDisableCount > 0)
	{
		--mDisableCount;

		if (0 == mDisableCount)
		{
			WWDEBUG_SAY(("IMEManager[SDL3]: Enabled\n"));
			UpdateTextInput();

			IMEEvent action(IME_ENABLED, this);
			NotifyObservers(action);
		}
	}
}


bool IMEManager::IsDisabled(void) const
{
	return (mDisableCount > 0);
}


/******************************************************************************
*
* NAME
*     IMEManager::UpdateTextInput
*
* DESCRIPTION
*     Start or stop SDL text input to match the IME state.
*
*     On Win32 typed characters always arrive as WM_CHAR, the IME only changes
*     how they are composed. SDL only sends text events while text input is
*     started, so it is kept running while the IME is active (an IME enabled
*     edit control has focus) or disabled (an edit control that does not
*     allow the IME has focus). While disabled, text input is started as a
*     password field which tells the platform not to use the IME.
*
******************************************************************************/

void IMEManager::UpdateTextInput(void)
{
	const bool wanted = (mActive || mDisableCount > 0);
	const bool password = (mDisableCount > 0);

	if (mTextInputWindow && (!wanted || password != mTextInputPassword))
	{
		SDL_StopTextInput(mTextInputWindow);
		mTextInputWindow = nullptr;
	}

	if (wanted && mTextInputWindow == nullptr)
	{
		SDL_Window* window = SDL_GetKeyboardFocus();

		if (window == nullptr)
		{
			WWDEBUG_SAY(("IMEManager[SDL3]: No window has keyboard focus\n"));
			return;
		}

		SDL_PropertiesID props = SDL_CreateProperties();
		SDL_SetNumberProperty(props, SDL_PROP_TEXTINPUT_TYPE_NUMBER,
			password ? SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_VISIBLE : SDL_TEXTINPUT_TYPE_TEXT);
		SDL_SetNumberProperty(props, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_NONE);
		SDL_SetBooleanProperty(props, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, false);

		if (SDL_StartTextInputWithProperties(window, props))
		{
			mTextInputWindow = window;
			mTextInputPassword = password;
		}
		else
		{
			WWDEBUG_SAY(("IMEManager[SDL3]: SDL_StartTextInput failed: %s\n", SDL_GetError()));
		}

		SDL_DestroyProperties(props);
	}
}


/******************************************************************************
*
* NAME
*     IMEManager::ProcessEvent
*
* DESCRIPTION
*     Process an SDL event.
*
* INPUTS
*     Event - SDL event
*
* RESULT
*     True if the event was consumed by the IME.
*
******************************************************************************/

bool IMEManager::ProcessEvent(const SDL_Event& event)
{
	switch (event.type)
	{
		case SDL_EVENT_TEXT_EDITING:
			if (!mActive)
			{
				return false;
			}

			if ((event.edit.text == nullptr) || (event.edit.text[0] == 0))
			{
				if (mInComposition)
				{
					EndComposition();
				}
			}
			else
			{
				if (!mInComposition)
				{
					StartComposition();
				}

				DoComposition(event.edit.text, event.edit.start, event.edit.length);
			}
			return true;

		case SDL_EVENT_TEXT_EDITING_CANDIDATES:
			if (!mActive)
			{
				return false;
			}

			DoCandidates(event.edit_candidates.candidates, event.edit_candidates.num_candidates,
				event.edit_candidates.selected_candidate);
			return true;

		case SDL_EVENT_TEXT_INPUT:
			// Text committed while composing is the composition result. Anything
			// else is plain typing and is handled by WWUIInputClass.
			if (mInComposition)
			{
				DoResult(event.text.text);
				EndComposition();
				return true;
			}
			return false;

		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			// Keys belong to the IME while composing.
			return mInComposition;

		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			// Text input may have been requested before the window had focus.
			UpdateTextInput();
			return false;

		case SDL_EVENT_KEYMAP_CHANGED:
			WWDEBUG_SAY(("IMEManager[SDL3]: Keymap changed\n"));
			return false;

		default:
			break;
	}

	return false;
}


void IMEManager::ResetComposition(void)
{
	mInComposition = false;

#ifdef SHOW_IME_TYPING
	mTypingString[0] = 0;
	mTypingCursorPos = 0;
#endif

	mCompositionString[0] = 0;
	mCompositionCursorPos = 0;
	mTargetStart = 0;
	mTargetEnd = 0;

	mReadingString[0] = 0;
}


void IMEManager::StartComposition(void)
{
	WWDEBUG_SAY(("IMEManager[SDL3]: StartComposition\n"));

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
*     Update the composition string.
*
* INPUTS
*     Text   - UTF-8 composition string
*     Start  - Cursor position in code points, or -1 if not set
*     Length - Length of the selected (target) text in code points, or -1
*
* RESULT
*     NONE
*
******************************************************************************/

void IMEManager::DoComposition(const char* text, int start, int length)
{
	Copy_String(text, mCompositionString, IME_MAX_STRING_LEN);

	const int compLength = (int)std::basic_string<unichar_t>(mCompositionString).length();

	if (start >= 0)
	{
		mCompositionCursorPos = std::min(WWUIPlatform::UTF8_Index_To_Unichar_Index(text, start), compLength);
	}
	else
	{
		mCompositionCursorPos = compLength;
	}

	if ((start >= 0) && (length > 0))
	{
		mTargetStart = (unsigned int)mCompositionCursorPos;
		mTargetEnd = (unsigned int)std::min(WWUIPlatform::UTF8_Index_To_Unichar_Index(text, start + length), compLength);
	}
	else
	{
		mTargetStart = 0;
		mTargetEnd = 0;
	}

	CompositionEvent event(COMPOSITION_CHANGE, this);
	NotifyObservers(event);
}


void IMEManager::DoResult(const char* text)
{
	Copy_String(text, mResultString, IME_MAX_STRING_LEN);

	CompositionEvent event(COMPOSITION_RESULT, this);
	NotifyObservers(event);
}


void IMEManager::EndComposition(void)
{
	WWDEBUG_SAY(("IMEManager[SDL3]: EndComposition\n"));

	ResetComposition();
	CompositionEvent event(COMPOSITION_END, this);
	NotifyObservers(event);
}


/******************************************************************************
*
* NAME
*     IMEManager::DoCandidates
*
* DESCRIPTION
*     Update the candidate list. SDL only has a single candidate list.
*
* INPUTS
*     Candidates - UTF-8 candidate strings
*     Count      - Number of candidates, 0 closes the list
*     Selection  - Selected candidate or -1
*
* RESULT
*     NONE
*
******************************************************************************/

void IMEManager::DoCandidates(const char* const* candidates, int count, int selection)
{
	IMECandidate& candidate = mCandidateColl[0];

	if ((candidates != nullptr) && (count > 0))
	{
		CandidateAction action = CANDIDATE_CHANGE;

		if (!mCandidatesOpen)
		{
			candidate.Open(0, mStartCandListFrom1);
			mCandidatesOpen = true;
			action = CANDIDATE_OPEN;
		}

		candidate.Set(candidates, count, selection);

		CandidateEvent event(action, &candidate);
		NotifyObservers(event);
	}
	else if (mCandidatesOpen)
	{
		mCandidatesOpen = false;

		CandidateEvent event(CANDIDATE_CLOSE, &candidate);
		NotifyObservers(event);
		candidate.Close();
	}
}


void IMEManager::GetTargetClause(unsigned int& start, unsigned int& end)
{
	start = mTargetStart;
	end = mTargetEnd;
}


unsigned int IMEManager::GetGuideline(unichar_t* outString, int /* length */)
{
	if (outString)
	{
		outString[0] = 0;
	}

	return GL_LEVEL_NOGUIDELINE;
}

} // namespace IME
