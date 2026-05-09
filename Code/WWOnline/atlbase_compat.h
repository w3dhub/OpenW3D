/*
**  atlbase_compat.h — ATL COM helper compatibility shim for non-Windows builds.
**
**  On Windows: include the real SDK headers.
**  On non-Windows: provide minimal ATL helpers (CComPtr, AtlAdvise) plus
**  the Windows COM types/macros that WWOnline needs but dxvk doesn't have.
**
**  Include this header INSTEAD of <objbase.h>, <comdef.h>, <windows.h>, etc.
**  in WWOnline source files that need COM support on non-Windows.
**
**  NOTE: This header intentionally does NOT define SYSTEMTIME/LPSYSTEMTIME —
**  those are provided by always.h which must be included first or will
**  define them via its own #ifndef SYSTEMTIME guard.
*/

#ifndef __WWONLINE_ATLBASE_COMPAT_H__
#define __WWONLINE_ATLBASE_COMPAT_H__

#if defined(_WIN32)

// Windows: include the real SDK headers.
#include <objbase.h>
#include <comdef.h>

#else // !_WIN32

//---------------------------------------------------------------------------
// Standard library includes — always needed
//---------------------------------------------------------------------------
#include <cstdint>
#include <sys/stat.h>
#include <errno.h>

//---------------------------------------------------------------------------
// dxvk headers may already be in the include chain (via wwlib/win.h).
// Detect them so we don't redefine their types/macros.
//---------------------------------------------------------------------------
#if defined(__has_include)
#if __has_include("/usr/include/dxvk/windows_base.h")
#define WWOL_DXVK_DETECTED 1
#endif
#else
#define WWOL_DXVK_DETECTED 1
#endif

//---------------------------------------------------------------------------
// Basic types — defined only when dxvk is ABSENT.
// When dxvk IS present, its types are already in the include chain before
// this header is processed (via wwlib/win.h → dxvk_wrapper_compat.h).
//---------------------------------------------------------------------------
#if !defined(WWOL_DXVK_DETECTED)
typedef int32_t HRESULT;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int32_t* LPLONG;
typedef uint32_t* LPDWORD;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef char* LPSTR;
typedef const char* LPCSTR;

#define S_OK               ((HRESULT)0L)
#define S_FALSE            ((HRESULT)1L)
#define E_NOTIMPL          ((HRESULT)0x80004001L)
#define E_NOINTERFACE      ((HRESULT)0x80004002L)
#define E_ABORT            ((HRESULT)0x80004004L)
#define E_FAIL             ((HRESULT)0x80004005L)
#define E_OUTOFMEMORY      ((HRESULT)0x8007000EL)
#define E_INVALIDARG       ((HRESULT)0x80070057L)
#define E_UNEXPECTED       ((HRESULT)0x8000FFFFL)
#define E_POINTER          ((HRESULT)0x80004003L)
#endif // !WWOL_DXVK_DETECTED

//---------------------------------------------------------------------------
// SEVERITY_ERROR, SEVERITY_SUCCESS, FACILITY_ITF
// dxvk's windows_base.h defines MAKE_HRESULT but not these — define always
//---------------------------------------------------------------------------
#ifndef SEVERITY_ERROR
#define SEVERITY_ERROR   1
#endif
#ifndef SEVERITY_SUCCESS
#define SEVERITY_SUCCESS 0
#endif
#ifndef FACILITY_ITF
#define FACILITY_ITF     4
#endif

//---------------------------------------------------------------------------
// Helper macros — dxvk doesn't have these
//---------------------------------------------------------------------------
#ifndef MAKELONG
#define MAKELONG(a, b) ((uint32_t)(((uint16_t)(a)) | ((uint32_t)((uint16_t)(b))) << 16))
#endif
#ifndef MAKEWORD
#define MAKEWORD(a, b) ((uint16_t)(((uint8_t)(a)) | ((uint16_t)((uint8_t)(b))) << 8))
#endif
#ifndef MAKELPARAM
#define MAKELPARAM(a, b) ((uint32_t)(((uint16_t)(a)) | ((uint32_t)((uint16_t)(b))) << 16))
#endif

//---------------------------------------------------------------------------
// STDMETHODIMP — used as simple return-type alias in method definitions:
//   STDMETHODIMP Foo() -> HRESULT Foo()
// dxvk defines STDMETHOD(name) but not STDMETHODIMP
//---------------------------------------------------------------------------
#ifndef STDMETHODIMP
#define STDMETHODIMP HRESULT
#endif

//---------------------------------------------------------------------------
// CLSCTX — dxvk doesn't have these
//---------------------------------------------------------------------------
#ifndef CLSCTX_INPROC_SERVER
#define CLSCTX_INPROC_SERVER   0x1
#endif
#ifndef CLSCTX_INPROC_HANDLER
#define CLSCTX_INPROC_HANDLER  0x2
#endif
#ifndef CLSCTX_LOCAL_SERVER
#define CLSCTX_LOCAL_SERVER    0x4
#endif
#ifndef CLSCTX_REMOTE_SERVER
#define CLSCTX_REMOTE_SERVER   0x10
#endif
#ifndef CLSCTX_ALL
#define CLSCTX_ALL             0x17
#endif

//---------------------------------------------------------------------------
// LCID and DISPID — dxvk doesn't have them
//---------------------------------------------------------------------------
#ifndef LCID
typedef uint32_t LCID;
#endif
#ifndef DISPID
typedef int32_t DISPID;
#endif
typedef DISPID *LPDISPID;

//---------------------------------------------------------------------------
// COM init/shutdown — dxvk doesn't define CoInitialize/CoUninitialize
//---------------------------------------------------------------------------
#ifndef CoInitialize
inline int32_t CoInitialize(void* /*pvReserved*/) {
    return (int32_t)0; // S_OK
}
#endif

#ifndef CoUninitialize
inline void CoUninitialize(void) { }
#endif

//---------------------------------------------------------------------------
// CoCreateInstance — stub that always returns E_NOINTERFACE on non-Windows
//---------------------------------------------------------------------------
inline int32_t CoCreateInstance_Compat(
    const void *rclsid,
    void* pUnkOuter,
    uint32_t dwClsContext,
    const void *riid,
    void **ppv)
{
    (void)rclsid;
    (void)pUnkOuter;
    (void)dwClsContext;
    (void)riid;
    if (ppv) *ppv = nullptr;
    return (int32_t)0x80004002L; // E_NOINTERFACE
}

#ifndef CoCreateInstance
#define CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv) \
    CoCreateInstance_Compat(&(rclsid), pUnkOuter, dwClsContext, &(riid), ppv)
#endif

//---------------------------------------------------------------------------
// Interlocked operations — use int32_t directly
//---------------------------------------------------------------------------
#ifndef InterlockedIncrement
inline int32_t InterlockedIncrement_Compat(int32_t *lpAddend) {
    return ++(*lpAddend);
}
#define InterlockedIncrement(x) InterlockedIncrement_Compat((int32_t*)(x))
#endif

#ifndef InterlockedDecrement
inline int32_t InterlockedDecrement_Compat(int32_t *lpAddend) {
    return --(*lpAddend);
}
#define InterlockedDecrement(x) InterlockedDecrement_Compat((int32_t*)(x))
#endif

#ifndef InterlockedExchange
inline int32_t InterlockedExchange_Compat(int32_t *lpAddend, int32_t val) {
    int32_t old = *lpAddend;
    *lpAddend = val;
    return old;
}
#define InterlockedExchange InterlockedExchange_Compat
#endif

//---------------------------------------------------------------------------
// File system helpers — dxvk doesn't have these
//---------------------------------------------------------------------------
#ifndef CreateDirectoryA
inline int CreateDirectoryA(const char* lpPathName, void* /*lpSecurityAttributes*/) {
    return (mkdir(lpPathName, 0755) == 0) ? 1 : 0;
}
#endif

#ifndef GetLastError
inline uint32_t GetLastError_Compat(void) {
    return (uint32_t)errno;
}
#define GetLastError() GetLastError_Compat()
#endif

#ifndef ERROR_ALREADY_EXISTS
#define ERROR_ALREADY_EXISTS  183
#endif

//---------------------------------------------------------------------------
// Socket address struct — only define if system headers haven't already
//---------------------------------------------------------------------------
#if !defined(_NETINET_IN_H) && !defined(_SYS_INADDR_DEFINED) && !defined(in_addr)
#define _SYS_INADDR_DEFINED
struct in_addr {
    union {
        struct { uint8_t s_b1, s_b2, s_b3, s_b4; } S_un_b;
        struct { uint16_t s_w1, s_w2; } S_un_w;
        uint32_t S_addr;
    } S_un;
};
#endif

//---------------------------------------------------------------------------
// AtlAdvise / AtlUnadvise — stubs that always return E_NOINTERFACE
//---------------------------------------------------------------------------
#ifndef AtlAdvise
inline int32_t AtlAdvise_Compat(
        void* /*pUnkCP*/,
        void* /*pUnk*/,
        const void */*iid*/,
        uint32_t* /*pdw*/)
{
    return (int32_t)0x80004002L; // E_NOINTERFACE
}
#define AtlAdvise(pCP, pUnk, iid, pdw) \
    AtlAdvise_Compat(pCP, pUnk, &(iid), pdw)
#endif

#ifndef AtlUnadvise
inline int32_t AtlUnadvise_Compat(
        void* /*pUnkCP*/,
        const void */*iid*/,
        uint32_t /*dw*/)
{
    return (int32_t)0x80004002L; // E_NOINTERFACE
}
#define AtlUnadvise(pCP, iid, dw) \
    AtlUnadvise_Compat(pCP, &(iid), dw)
#endif

//---------------------------------------------------------------------------
// CComPtr — ATL smart pointer (always provided on non-Windows)
//---------------------------------------------------------------------------
#ifdef __cplusplus

template<typename T>
class CComPtr {
public:
    CComPtr() : m_p(nullptr) {}
    CComPtr(T* lp) : m_p(lp) { if (m_p) m_p->AddRef(); }
    CComPtr(const CComPtr<T>& lp) : m_p(lp.m_p) { if (m_p) m_p->AddRef(); }
    ~CComPtr() { if (m_p) m_p->Release(); }

    void Release() { if (m_p) { m_p->Release(); m_p = nullptr; } }
    void Attach(T* lp) {
        if (m_p) m_p->Release();
        m_p = lp;
    }
    T* Detach() { T* p = m_p; m_p = nullptr; return p; }

    T* m_p; // public for raw access

    T* operator->() const { return m_p; }
    T** operator&() { return &m_p; }
    operator T*() const { return m_p; }

    CComPtr<T>& operator=(T* lp) {
        if (lp) lp->AddRef();
        if (m_p) m_p->Release();
        m_p = lp;
        return *this;
    }

    bool operator!() const { return m_p == nullptr; }
};

#endif // __cplusplus

#endif // !_WIN32

#endif // __WWONLINE_ATLBASE_COMPAT_H__
