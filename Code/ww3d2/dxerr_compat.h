#pragma once

#ifdef _MSC_VER
#include <dxerr.h>
#else

static inline const char*  DXGetErrorStringA(HRESULT hr) {
    static char text[32];
    sprintf(text, "DXERR:0x%08x", hr);
    return text;
}

#endif
