#pragma once

#ifdef _WIN32
#include <windows_base.h>
#include <windows.h>
#endif

#define LF_FACESIZE 32
#define EXTERN_C extern "C"

#ifdef _WIN32
typedef struct GLYPHMETRICSFLOAT GLYPHMETRICSFLOAT;
typedef GUID *LPGUID;
typedef GUID CLSID;
#define DECLSPEC_UUID(X)
typedef struct IStream IStream;
typedef struct HFONT HFONT;
typedef struct TEXTMETRICA TEXTMETRICA;
typedef struct TEXTMETRICW TEXTMETRICW;
typedef double DOUBLE;
#define STDAPI HRESULT WINAPI
typedef long HRESULT;
#else
// Stub types for non-Windows builds.
// The dxvk_wrapper provides D3D9 type shims only; actual rendering
// requires a platform-specific backend implementation.
typedef void *LPGUID;
typedef long HRESULT;
#define STDAPI HRESULT
#define DECLSPEC_UUID(X)
typedef void *HFONT;
typedef void *HBITMAP;
typedef void *HDC;
typedef void *HANDLE;
typedef struct { void *lpVtbl; } IStream;
#endif

#define HIWORD(W) (((W) & 0xffff) >> 8)
#define LOWORD(W) ((W) & 0xff)