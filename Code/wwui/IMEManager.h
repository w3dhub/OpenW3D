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
#include "win.h"
#include <imm.h>

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
typedef TypedEvent<UnicodeType, wchar_t> UnicodeChar;

class IMEManager :
		public RefCountClass,
		public Notifier<IMEEvent>,
		public Notifier<UnicodeChar>,
		public Notifier<CompositionEvent>,
		public Notifier<CandidateEvent>
	{
	public:
		static IMEManager* Create(HWND hwnd);

		void Activate(void);
		void Deactivate(void);
		bool IsActive(void) const;

		void Disable(void);
		void Enable(void);
		bool IsDisabled(void) const;

		const wchar_t* GetDescription(void) const
			{return mIMEDescription;}

		WORD GetLanguageID(void) const
			{return mLangID;}

		UINT GetCodePage(void) const
			{return mCodePage;}

		const wchar_t* GetResultString(void) const
			{return mResultString;}

		const wchar_t* GetCompositionString(void) const
			{return mCompositionString;}

		long GetCompositionCursorPos(void) const
			{return mCompositionCursorPos;}

		const wchar_t* GetReadingString(void) const
			{return mReadingString;}

		#ifdef SHOW_IME_TYPING
		const wchar_t* GetTypingString(void) const
			{return mTypingString;}
		#endif

		void GetTargetClause(unsigned long& start, unsigned long& end);

		bool GetCompositionFont(LPLOGFONT lpFont);

		const IMECandidateCollection GetCandidateColl(void) const
			{return mCandidateColl;}

		unsigned long GetGuideline(wchar_t* outString, int length);

		bool ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result);

	protected:
		IMEManager();
		virtual ~IMEManager();

		bool FinalizeCreate(HWND hwnd);

		LRESULT IMENotify(WPARAM wParam, LPARAM lParam);
		
		HKL InputLanguageChangeRequest(HKL hkl);
		void InputLanguageChanged(HKL hkl);

		void ResetComposition(void);
		void StartComposition(void);
		void DoComposition(unsigned int dbcsChar, long changeFlag);
		void EndComposition(void);

		bool ReadCompositionString(HIMC imc, unsigned long flag, wchar_t* buffer, int length);
		long ReadReadingAttr(HIMC imc, unsigned char* attr, int length);
		long ReadReadingClause(HIMC imc, unsigned long* clause, int length);
		long ReadCompositionAttr(HIMC imc, unsigned char* attr, int length);
		long ReadCompositionClause(HIMC imc, unsigned long* clause, int length);
		long ReadCursorPos(HIMC imc);

		void OpenCandidate(unsigned long candList);
		void ChangeCandidate(unsigned long candList);
		void CloseCandidate(unsigned long candList);

		bool IMECharHandler(unsigned short dbcs);
		bool CharHandler(unsigned short ch);

		long ConvertAttrForUnicode(unsigned char* mbcs, unsigned char* attr);
		long ConvertClauseForUnicode(unsigned char* mbcs, long length, unsigned long* clause);

		DECLARE_NOTIFIER(IMEEvent)
		DECLARE_NOTIFIER(UnicodeChar)
		DECLARE_NOTIFIER(CompositionEvent)
		DECLARE_NOTIFIER(CandidateEvent)

		// Prevent copy and assignment
		IMEManager(const IMEManager&);
		const IMEManager& operator=(const IMEManager&);

	private:
		HWND mHWND;
		HIMC mDefaultHIMC;
		HIMC mHIMC;

		HIMC mDisabledHIMC;
		unsigned long mDisableCount;

		WORD mLangID;
		UINT mCodePage;
		WideStringClass mIMEDescription;
		DWORD mIMEProperties;

		bool mHilite;
		bool mStartCandListFrom1;
		bool mOSCanUnicode;
		bool mUseUnicode;
		bool mInComposition;

		#ifdef SHOW_IME_TYPING
		wchar_t mTypingString[IME_MAX_TYPING_LEN];
		long mTypingCursorPos;
		#endif
		
		wchar_t mCompositionString[IME_MAX_STRING_LEN];
		unsigned char mCompositionAttr[IME_MAX_STRING_LEN];
		unsigned long mCompositionClause[IME_MAX_STRING_LEN / 2];

		long mCompositionCursorPos;

		wchar_t mReadingString[IME_MAX_STRING_LEN * 2];
		wchar_t mResultString[IME_MAX_STRING_LEN];

		IMECandidateCollection mCandidateColl;
	};

} // namespace IME

#endif //__IMEMANAGER_H__
