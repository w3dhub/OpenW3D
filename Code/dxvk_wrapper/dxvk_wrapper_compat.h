#pragma once

#ifdef _WIN32

#include <windows_base.h>
#include <windows.h>

typedef struct GLYPHMETRICSFLOAT GLYPHMETRICSFLOAT;
typedef GUID *LPGUID;
typedef GUID CLSID;
typedef struct IStream IStream;
typedef struct HFONT HFONT;
typedef struct TEXTMETRICA TEXTMETRICA;
typedef struct TEXTMETRICW TEXTMETRICW;
typedef double DOUBLE;
typedef long HRESULT;
#define STDAPI HRESULT WINAPI
#define DECLSPEC_UUID(X)

#else // !_WIN32

// Linux stubs for Windows types needed by the codebase
// These are safe to include at any point - they use #ifndef guards
// BUT note: #ifndef only works for macros, not typedefs.
// For typedefs that might conflict with win.h, we just don't redefine.

#include <stdint.h>

#ifndef DWORD
typedef uint32_t DWORD;
#endif

#ifndef HRESULT
typedef int32_t HRESULT;
#endif

#ifndef HANDLE
typedef void *HANDLE;
#endif

#ifndef LPVOID
typedef void *LPVOID;
#endif

#ifndef ULONG_PTR
typedef uintptr_t ULONG_PTR;
#endif

#ifndef LONG_PTR
typedef intptr_t LONG_PTR;
#endif

#ifndef LONG
typedef int32_t LONG;
#endif

#ifndef UINT
typedef uint32_t UINT;
#endif

#ifndef WORD
typedef uint16_t WORD;
#endif

#ifndef BOOL
typedef int32_t BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef BYTE
typedef uint8_t BYTE;
#endif

#ifndef WCHAR
typedef wchar_t WCHAR;
#endif

// Note: LPSTR and LPWSTR are intentionally omitted because win.h defines them
// as void* which is incompatible with CHAR*/WCHAR* definitions

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif

#ifndef LF_FACESIZE
#define LF_FACESIZE 32
#endif

// MAKE_HRESULT - construct HRESULT from severity, facility, and code
#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev, fac, code) \
    ((HRESULT)(((unsigned long)(sev) << 31) | ((unsigned long)(fac) << 16) | ((unsigned long)(code))))
#endif

#ifndef EXTERN_C
#define EXTERN_C extern "C"
#endif

#ifndef STDAPI
#define STDAPI HRESULT
#endif

#ifndef DECLSPEC_UUID
#define DECLSPEC_UUID(X)
#endif

#ifndef WINAPI
#define WINAPI __attribute__((ms_abi))
#endif

// Stub types for non-Windows builds (only if not already defined)
#ifndef LPGUID
typedef void *LPGUID;
#endif

#ifndef HFONT
typedef void *HFONT;
#endif

#ifndef HBITMAP
typedef void *HBITMAP;
#endif

#ifndef HDC
typedef void *HDC;
#endif

#ifndef HGLOBAL
typedef void *HGLOBAL;
#endif

#ifndef HRSRC
typedef void *HRSRC;
#endif

// Note: IStream is intentionally omitted - win.h doesn't define it,
// but refcount.cpp defines it differently. Let the first definition win.

#ifndef FARPROC
typedef long (*FARPROC)();
#endif

#ifndef PROC
typedef long (*PROC)();
#endif

#ifndef HIWORD
#define HIWORD(W) (((W) & 0xffff) >> 8)
#endif

#ifndef LOWORD
#define LOWORD(W) ((W) & 0xff)
#endif

// Windows File API stubs for Linux
#ifndef GENERIC_READ
#define GENERIC_READ             (0x80000000L)
#endif

#ifndef GENERIC_WRITE
#define GENERIC_WRITE            (0x40000000L)
#endif

#ifndef GENERIC_EXECUTE
#define GENERIC_EXECUTE          (0x20000000L)
#endif

#ifndef GENERIC_ALL
#define GENERIC_ALL              (0x10000000L)
#endif

#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ          0x00000001
#endif

#ifndef FILE_SHARE_WRITE
#define FILE_SHARE_WRITE         0x00000002
#endif

#ifndef FILE_SHARE_DELETE
#define FILE_SHARE_DELETE        0x00000004
#endif

#ifndef FILE_ATTRIBUTE_READONLY
#define FILE_ATTRIBUTE_READONLY  0x00000001
#endif

#ifndef FILE_ATTRIBUTE_HIDDEN
#define FILE_ATTRIBUTE_HIDDEN    0x00000002
#endif

#ifndef FILE_ATTRIBUTE_SYSTEM
#define FILE_ATTRIBUTE_SYSTEM    0x00000004
#endif

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif

#ifndef FILE_ATTRIBUTE_ARCHIVE
#define FILE_ATTRIBUTE_ARCHIVE   0x00000020
#endif

#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL    0x00000080
#endif

#ifndef FILE_ATTRIBUTE_TEMPORARY
#define FILE_ATTRIBUTE_TEMPORARY 0x00000100
#endif

#ifndef CREATE_NEW
#define CREATE_NEW              1
#endif

#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS           2
#endif

#ifndef OPEN_EXISTING
#define OPEN_EXISTING           3
#endif

#ifndef OPEN_ALWAYS
#define OPEN_ALWAYS             4
#endif

#ifndef TRUNCATE_EXISTING
#define TRUNCATE_EXISTING       5
#endif

#ifndef FILE_BEGIN
#define FILE_BEGIN              0
#endif

#ifndef FILE_CURRENT
#define FILE_CURRENT            1
#endif

#ifndef FILE_END
#define FILE_END                2
#endif

#ifndef FILE_FLAG_WRITE_THROUGH
#define FILE_FLAG_WRITE_THROUGH  0x80000000
#endif

#ifndef FILE_FLAG_NO_BUFFERING
#define FILE_FLAG_NO_BUFFERING  0x20000000
#endif

#ifndef FILE_FLAG_RANDOM_ACCESS
#define FILE_FLAG_RANDOM_ACCESS  0x10000000
#endif

#ifndef FILE_FLAG_SEQUENTIAL_SCAN
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#endif

// File time structure
#ifndef FILETIME
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;
#endif

// By-handle file information
#ifndef BY_HANDLE_FILE_INFORMATION
typedef struct _BY_HANDLE_FILE_INFORMATION {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD dwVolumeSerialNumber;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD nNumberOfLinks;
    DWORD nFileIndexHigh;
    DWORD nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION;
#endif

// Security attributes (simplified)
#ifndef SECURITY_ATTRIBUTES
typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES;
#endif

// HANDLE is already defined above as void*
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif // !_WIN32
