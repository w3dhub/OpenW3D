#pragma once

#ifdef ZeroMemory
#undef ZeroMemory
#endif

#include <windows_base.h>
#include <windows.h>

#define LF_FACESIZE 32
#define EXTERN_C extern "C"

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

#define HIWORD(W) (((W) & 0xffff) >> 8)
#define LOWORD(W) ((W) & 0xff)
