#include "Locale_API.h"

#include "locale.h"
#include "wwconfig_ids.h"

#include <cstdlib>
#include <cstring>
#include <cwchar>

namespace
{
constexpr size_t kMaxLocalePath = 260;

bool g_localeInitialized = false;

int DefaultLanguageFromLocale()
{
    const char *locale = std::getenv("LC_ALL");
    if (!locale || locale[0] == '\0') {
        locale = std::getenv("LC_MESSAGES");
    }
    if (!locale || locale[0] == '\0') {
        locale = std::getenv("LANG");
    }
    if (!locale) {
        return IDL_ENGLISH;
    }

    if (std::strncmp(locale, "de", 2) == 0) {
        return IDL_GERMAN;
    }
    if (std::strncmp(locale, "fr", 2) == 0) {
        return IDL_FRENCH;
    }
    if (std::strncmp(locale, "ja", 2) == 0) {
        return IDL_JAPANESE;
    }
    if (std::strncmp(locale, "ko", 2) == 0) {
        return IDL_KOREAN;
    }
    if (std::strncmp(locale, "zh", 2) == 0) {
        return IDL_CHINESE;
    }

    return IDL_ENGLISH;
}

wchar_t *Remove_Quotes_Around_String(wchar_t *old_string)
{
    if (!old_string || *old_string != L'"') {
        return old_string;
    }

    wchar_t wide_buffer[kMaxLocalePath * 3] = {};
    const wchar_t *letter = old_string + 1;
    std::wcsncpy(wide_buffer, letter, (kMaxLocalePath * 3) - 1);

    const size_t length = std::wcslen(wide_buffer);
    if (length > 0 && wide_buffer[length - 1] == L'"') {
        wide_buffer[length - 1] = L'\0';
    }

    std::wcsncpy(old_string, wide_buffer, (kMaxLocalePath * 3) - 1);
    return old_string;
}

const wchar_t *LookupWideString(int stringId)
{
    if (!g_localeInitialized) {
        return L"";
    }

    const char *text = LOCALE_getstring(stringId);
    if (!text) {
        return L"";
    }

    return reinterpret_cast<const wchar_t *>(text);
}
} // namespace

char LanguageFile[kMaxLocalePath] = {};
void *LocaleFile = nullptr;
int CodePage = 65001;
int LanguageID = IDL_ENGLISH;

bool Locale_Use_Multi_Language_Files(void)
{
    return true;
}

int Locale_Init(int language, const char *file)
{
    if (!file || file[0] == '\0') {
        return 0;
    }

    if (g_localeInitialized) {
        Locale_Restore();
    }

    std::strncpy(LanguageFile, file, sizeof(LanguageFile) - 1);
    LanguageFile[sizeof(LanguageFile) - 1] = '\0';

    LanguageID = language;
    if (LanguageID == -1) {
        LanguageID = DefaultLanguageFromLocale();
    }
    if (LanguageID < 0 || LanguageID >= LOCALE_LANGUAGE_COUNT) {
        LanguageID = IDL_ENGLISH;
    }

    if (!LOCALE_init()) {
        return 0;
    }

    LOCALE_setbank(0);
    bool loaded = LOCALE_loadtable(LanguageFile, LanguageID) != 0;
    if (!loaded && LanguageID != IDL_ENGLISH) {
        LOCALE_restore();
        if (!LOCALE_init()) {
            return 0;
        }
        LOCALE_setbank(0);
        loaded = LOCALE_loadtable(LanguageFile, IDL_ENGLISH) != 0;
        if (loaded) {
            LanguageID = IDL_ENGLISH;
        }
    }

    if (!loaded) {
        LOCALE_restore();
        return 0;
    }

    g_localeInitialized = true;
    LocaleFile = reinterpret_cast<void *>(1);
    return 1;
}

void Locale_Restore(void)
{
    if (!g_localeInitialized) {
        return;
    }

    LOCALE_freetable();
    LOCALE_restore();
    g_localeInitialized = false;
    LocaleFile = nullptr;
}

const char *Locale_GetString(int stringId, char *string)
{
    static char buffer[kMaxLocalePath] = {};
    static wchar_t wide_buffer[kMaxLocalePath] = {};

    std::memset(buffer, '\0', sizeof(buffer));
    std::wmemset(wide_buffer, L'\0', sizeof(wide_buffer) / sizeof(wide_buffer[0]));

    std::wcsncpy(wide_buffer, LookupWideString(stringId), (kMaxLocalePath - 1));
    Remove_Quotes_Around_String(wide_buffer);

    const size_t converted = std::wcstombs(buffer, wide_buffer, sizeof(buffer) - 1);
    if (converted == static_cast<size_t>(-1)) {
        buffer[0] = '\0';
    }

    if (string) {
        std::strncpy(string, buffer, kMaxLocalePath - 1);
        string[kMaxLocalePath - 1] = '\0';
    }

    return buffer;
}

const wchar_t *Locale_GetString(int stringId, wchar_t *string)
{
    static wchar_t wide_buffer[kMaxLocalePath] = {};

    std::wmemset(wide_buffer, L'\0', sizeof(wide_buffer) / sizeof(wide_buffer[0]));
    std::wcsncpy(wide_buffer, LookupWideString(stringId), kMaxLocalePath - 1);
    Remove_Quotes_Around_String(wide_buffer);

    if (string) {
        std::wcsncpy(string, wide_buffer, kMaxLocalePath - 1);
        string[kMaxLocalePath - 1] = L'\0';
    }

    return wide_buffer;
}

const wchar_t *Locale_GetString(int stringId)
{
    static wchar_t wide_buffer[kMaxLocalePath] = {};

    std::wmemset(wide_buffer, L'\0', sizeof(wide_buffer) / sizeof(wide_buffer[0]));
    std::wcsncpy(wide_buffer, LookupWideString(stringId), kMaxLocalePath - 1);
    Remove_Quotes_Around_String(wide_buffer);

    return wide_buffer;
}
