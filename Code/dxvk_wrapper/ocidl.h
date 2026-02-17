#pragma once

#include "windows.h"
#include <dxvk/ocidl.h>

struct IConnectionPoint : public IUnknown {
	virtual HRESULT Advise(IUnknown *pUnkSink, DWORD *dwCookie) = 0;
	virtual HRESULT Unadvise(DWORD dwCookie) = 0;
};
struct IConnectionPointContainer : public IUnknown {
	virtual HRESULT FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP) = 0;
};
