#pragma once

#ifdef _MSC_VER

#include <atlbase.h>

#else

#include <windows.h>
#include <ocidl.h>
#include <unknwn.h>

template <typename T>
class CComPtr final
{
public:
    CComPtr()
    : p(nullptr) {
    }
    explicit CComPtr(T *object) {
        Attach(object);
    }
    CComPtr(CComPtr &other)
    : p(other.p) {
        if (p) {
            p->AddRef();
        }
    }
    CComPtr(CComPtr &&other)
    : p(other.p) {
        other.p = nullptr;
    }
    CComPtr & operator=(CComPtr &other) {
        if (other.p != this->p) {
            Attach(other.p);
            if (p) {
                p->AddRef();
            }
        }
        return *this;
    }
    CComPtr & operator=(CComPtr &&other) {
        T *object = p;
        p = other.p;
        other.p = object;
        return *this;
    }
    CComPtr & operator=(T *object) {
        if (object != this->p) {
            Release();
            p = object;
            p->AddRef();
        }
        return *this;
    }

    ~CComPtr() {
        Release();
    }

    void Release() {
        if (p) {
            p->Release();
            p = nullptr;
        }
    }

    void Attach(T* p) {
        Release();
        p = p;
    }

    T * Detach() {
        T *obj = p;
        p = nullptr;
        return obj;
    }

    bool operator==(T* object) const
    {
        return p == object;
    }

    bool operator!=(T* object) const
    {
        return p != object;
    }

    T ** operator &() {
        return &p;
    }

    operator T*() const {
        return p;
    }

    T *operator->() const {
        return p;
    }

    T *p;
};


inline HRESULT AtlAdvise(
        IUnknown* pUnkCP,
        IUnknown* pUnk,
        const IID& iid,
        LPDWORD pdw)
{
    if(pUnkCP == NULL) {
        return E_INVALIDARG;
    }

    CComPtr<IConnectionPointContainer> pCPC;
    CComPtr<IConnectionPoint> pCP;
    HRESULT hRes = pUnkCP->QueryInterface(__uuidof(IConnectionPointContainer), (void**)&pCPC);
    if (SUCCEEDED(hRes)) {
        hRes = pCPC->FindConnectionPoint(iid, &pCP);
    }
    if (SUCCEEDED(hRes)) {
        hRes = pCP->Advise(pUnk, pdw);
    }
    return hRes;
}

inline HRESULT AtlUnadvise(
        IUnknown* pUnkCP,
        const IID& iid,
        DWORD dw)
{
    if (pUnkCP == NULL) {
        return E_INVALIDARG;
    }

    CComPtr<IConnectionPointContainer> pCPC;
    CComPtr<IConnectionPoint> pCP;
    HRESULT hRes = pUnkCP->QueryInterface(__uuidof(IConnectionPointContainer), (void**)&pCPC);
    if (SUCCEEDED(hRes)) {
        hRes = pCPC->FindConnectionPoint(iid, &pCP);
    }
    if (SUCCEEDED(hRes)) {
        hRes = pCP->Unadvise(dw);
    }
    return hRes;
}

#endif
