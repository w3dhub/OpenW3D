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
*     $Archive: /Commando/Code/wwui/IMEManager.h $
*
* DESCRIPTION
*     Input Method Editor Manager for input of far east characters.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 3 $
*     $Modtime: 1/08/02 8:38p $
*
******************************************************************************/

#ifndef __IMEMANAGER_H__
#define __IMEMANAGER_H__

#include "refcount.h"
#include "IMECandidate.h"
#include "Notify.h"
#include "widestring.h"
#include "wwuiplatform.h"

#if defined(OPENW3D_WIN32)
#include "win.h"
#include <imm.h>
#elif defined(OPENW3D_SDL3)
union SDL_Event;
struct SDL_Window;

// SDL has no IME guidelines, GetGuideline() always returns this.
#ifndef GL_LEVEL_NOGUIDELINE
#define GL_LEVEL_NOGUIDELINE 0
#endif
#endif

namespace IME {

#define IME_MAX_STRING_LEN 255
#define IME_MAX_TYPING_LEN 80

class IMEManager;

typedef enum
	{
	IME_ACTIVATED = 1,
	IME_DEACTIVATED,
	IME_LANGUAGECHANGED,
	IME_GUIDELINE,
	IME_ENABLED,
	IME_DISABLED,
	} IMEAction;

typedef TypedActionPtr<IMEAction, IMEManager> IMEEvent;

typedef enum
	{
	COMPOSITION_INVALID = 0,
	COMPOSITION_TYPING,
	COMPOSITION_START,
	COMPOSITION_CHANGE,
	COMPOSITION_FULL,
	COMPOSITION_END,
	COMPOSITION_CANCEL,
	COMPOSITION_RESULT
	} CompositionAction;

typedef TypedActionPtr<CompositionAction, IMEManager> CompositionEvent;

class UnicodeType;
typedef TypedEvent<UnicodeType, unichar_t> UnicodeChar;

class IMEManager :
		public RefCountClass,
		public Notifier<IMEEvent>,
		public Notifier<UnicodeChar>,
		public Notifier<CompositionEvent>,
		public Notifier<CandidateEvent>
	{
	public:
		static IMEManager* Create(WWUIWindowHandle hwnd);

		void Activate(void);
		void Deactivate(void);
		bool IsActive(void) const;

		void Disable(void);
		void Enable(void);
		bool IsDisabled(void) const;

		const unichar_t* GetDescription(void) const
			{return mIMEDescription;}

		uint16_t GetLanguageID(void) const
			{return mLangID;}

		uint32_t GetCodePage(void) const
			{return mCodePage;}

		const unichar_t* GetResultString(void) const
			{return mResultString;}

		const unichar_t* GetCompositionString(void) const
			{return mCompositionString;}

		int GetCompositionCursorPos(void) const
			{return mCompositionCursorPos;}

		const unichar_t* GetReadingString(void) const
			{return mReadingString;}

		#ifdef SHOW_IME_TYPING
		const unichar_t* GetTypingString(void) const
			{return mTypingString;}
		#endif

		void GetTargetClause(unsigned int& start, unsigned int& end);

#if defined(OPENW3D_WIN32)
		bool GetCompositionFont(LPLOGFONT lpFont);
#endif

		const IMECandidateCollection GetCandidateColl(void) const
			{return mCandidateColl;}

		unsigned int GetGuideline(unichar_t* outString, int length);

#if defined(OPENW3D_WIN32)
		bool ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result);
#elif defined(OPENW3D_SDL3)
		// Returns true if the event was consumed by the IME.
		bool ProcessEvent(const SDL_Event& event);
#endif

	protected:
		IMEManager();
		virtual ~IMEManager();

		bool FinalizeCreate(WWUIWindowHandle hwnd);

#if defined(OPENW3D_WIN32)
		LRESULT IMENotify(WPARAM wParam, LPARAM lParam);

		HKL InputLanguageChangeRequest(HKL hkl);
		void InputLanguageChanged(HKL hkl);

		void ResetComposition(void);
		void StartComposition(void);
		void DoComposition(unsigned int dbcsChar, int changeFlag);
		void EndComposition(void);

		bool ReadCompositionString(HIMC imc, unsigned int flag, unichar_t* buffer, int length);
		int ReadReadingAttr(HIMC imc, unsigned char* attr, int length);
		int ReadReadingClause(HIMC imc, unsigned int* clause, int length);
		int ReadCompositionAttr(HIMC imc, unsigned char* attr, int length);
		int ReadCompositionClause(HIMC imc, unsigned int* clause, int length);
		int ReadCursorPos(HIMC imc);

		void OpenCandidate(unsigned int candList);
		void ChangeCandidate(unsigned int candList);
		void CloseCandidate(unsigned int candList);

		bool IMECharHandler(unsigned short dbcs);
		bool CharHandler(unsigned short ch);

		int ConvertAttrForUnicode(unsigned char* mbcs, unsigned char* attr);
		int ConvertClauseForUnicode(unsigned char* mbcs, int length, unsigned int* clause);
#elif defined(OPENW3D_SDL3)
		void ResetComposition(void);
		void StartComposition(void);
		void EndComposition(void);
		void DoComposition(const char* text, int start, int length);
		void DoResult(const char* text);
		void DoCandidates(const char* const* candidates, int count, int selection);

		void UpdateTextInput(void);
#endif

		DECLARE_NOTIFIER(IMEEvent)
		DECLARE_NOTIFIER(UnicodeChar)
		DECLARE_NOTIFIER(CompositionEvent)
		DECLARE_NOTIFIER(CandidateEvent)

		// Prevent copy and assignment
		IMEManager(const IMEManager&);
		const IMEManager& operator=(const IMEManager&);

	private:
#if defined(OPENW3D_WIN32)
		HWND mHWND;
		HIMC mDefaultHIMC;
		HIMC mHIMC;

		HIMC mDisabledHIMC;
#elif defined(OPENW3D_SDL3)
		SDL_Window* mTextInputWindow;
		bool mActive;
		bool mTextInputPassword;
		bool mCandidatesOpen;

		unsigned int mTargetStart;
		unsigned int mTargetEnd;
#endif
		unsigned int mDisableCount;

		uint16_t mLangID;
		uint32_t mCodePage;
		WideStringClass mIMEDescription;
#if defined(OPENW3D_WIN32)
		DWORD mIMEProperties;

		bool mHilite;
#endif
		bool mStartCandListFrom1;
#if defined(OPENW3D_WIN32)
		bool mOSCanUnicode;
		bool mUseUnicode;
#endif
		bool mInComposition;

		#ifdef SHOW_IME_TYPING
		unichar_t mTypingString[IME_MAX_TYPING_LEN];
		int mTypingCursorPos;
		#endif

		unichar_t mCompositionString[IME_MAX_STRING_LEN];
#if defined(OPENW3D_WIN32)
		unsigned char mCompositionAttr[IME_MAX_STRING_LEN];
		unsigned int mCompositionClause[IME_MAX_STRING_LEN / 2];
#endif

		int mCompositionCursorPos;

		unichar_t mReadingString[IME_MAX_STRING_LEN * 2];
		unichar_t mResultString[IME_MAX_STRING_LEN];

		IMECandidateCollection mCandidateColl;
	};

} // namespace IME

#endif //__IMEMANAGER_H__
