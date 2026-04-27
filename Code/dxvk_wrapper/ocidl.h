#pragma once

#include "windows.h"

#ifdef _WIN32
#include <dxvk/ocidl.h>
#endif

struct IConnectionPoint : public IUnknown {
	virtual HRESULT Advise(IUnknown *pUnkSink, DWORD *dwCookie) = 0;
	virtual HRESULT Unadvise(DWORD dwCookie) = 0;
};
struct IConnectionPointContainer : public IUnknown {
	virtual HRESULT FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP) = 0;
};
