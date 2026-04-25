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
typedef struct IStream IStream;
typedef struct HFONT HFONT;
typedef struct TEXTMETRICA TEXTMETRICA;
typedef struct TEXTMETRICW TEXTMETRICW;
typedef double DOUBLE;
typedef long HRESULT;
#define STDAPI HRESULT WINAPI
#define DECLSPEC_UUID(X)
#else
// Stub types for non-Windows builds (Linux/macOS).
// The dxvk_wrapper provides D3D9 type definitions only —
// the actual renderer backend must still be implemented per platform.
typedef void *LPGUID;
typedef long HRESULT;            // NOTE: codebase uses int for HRESULT in practice
#define STDAPI HRESULT
#define DECLSPEC_UUID(X)
typedef void *HFONT;
typedef void *HBITMAP;  // Windows GDI bitmap handle
typedef void *HDC;      // Windows GDI device context handle
typedef void *HANDLE;
typedef struct { void *lpVtbl; } IStream;
#endif

#define HIWORD(W) (((W) & 0xffff) >> 8)
#define LOWORD(W) ((W) & 0xff)
