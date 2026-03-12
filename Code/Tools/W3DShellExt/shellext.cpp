/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdAfx.h"
#include "priv.h"
#include "Chunkio.h"
#include "io.h"
#include "wdump.h"
#include "WdumpDoc.h"
//#include "w3d2dat.h"			/// LFeenanEA: Header file missing, perhaps this tool is outdated?
//
// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
//
#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#include "shellext.h"


#pragma data_seg()
// Global variables
//
UINT      g_DllRefCount = 0;    // Reference count of this DLL.
HINSTANCE g_DllInstance = NULL;	// Handle to this DLL itself.

//===============================================================
extern "C" int APIENTRY
	DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved){
	if (dwReason == DLL_PROCESS_ATTACH){
		g_DllInstance = hInstance;
	}else
	if (dwReason == DLL_PROCESS_DETACH){
		ODS3("Detaching Process");
	}
	return 1;   // ok
}

//---------------------------------------------------------------------------
// DllCanUnloadNow
//---------------------------------------------------------------------------

STDAPI DllCanUnloadNow(void)
{
    return (g_DllRefCount == 0 ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
    *ppvOut = NULL;
    if (IsEqualIID(rclsid, CLSID_ShellExtension)) {
     CShellExtClassFactory* pcf = new CShellExtClassFactory;
     return pcf->QueryInterface(riid, ppvOut);
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

//======================================================================================
CShellExtClassFactory::CShellExtClassFactory()
{
    m_cRef = 0L;
    g_DllRefCount++;
}

//======================================================================================
CShellExtClassFactory::~CShellExtClassFactory(){
    g_DllRefCount--;
}

//======================================================================================
STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid, LPVOID FAR *ppv){
    *ppv = NULL;
    // Any interface on this object is the object pointer

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory)){
        *ppv = (LPCLASSFACTORY)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}
//======================================================================================
STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;
    delete this;
    return 0L;
}

STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,REFIID riid,LPVOID *ppvObj){
    *ppvObj = NULL;
    // Shell extensions typically don't support aggregation (inheritance)
    if (pUnkOuter)
    	return CLASS_E_NOAGGREGATION;
    // Create the main shell extension object.  The shell will then call
    // QueryInterface with IID_IShellExtInit--this is how shell extensions are
    // initialized.

    LPCSHELLEXT pShellExt = new CShellExt();  //Create the CShellExt object
    if (NULL == pShellExt)
    	return E_OUTOFMEMORY;
    return pShellExt->QueryInterface(riid, ppvObj);
}


STDMETHODIMP CShellExtClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

// *********************** CShellExt *************************
CShellExt::CShellExt(): m_FileInMemory(false),
						m_NumAdded(0),
						m_FoundMeshes(0){
    m_cRef = 0L;
    m_pDataObj = NULL;
    g_DllRefCount++;
}

CShellExt::~CShellExt()
{
    if (m_pDataObj)
        m_pDataObj->Release();

    g_DllRefCount--;
}

STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;
    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown)) {
    	*ppv = (LPSHELLEXTINIT)this;
    }else {
		 if (IsEqualIID(riid, IID_IContextMenu)) {
	        *ppv = (LPCONTEXTMENU)this;
		 }else{
			 if (IsEqualIID(riid, IID_IExtractIcon)){
				*ppv = (LPEXTRACTICON)this;
			 }else {
				 if (IsEqualIID(riid, IID_IPersistFile)){
					*ppv = (LPPERSISTFILE)this;
				 }else {
					 if (IsEqualIID(riid, IID_IShellPropSheetExt)){
							*ppv = (LPSHELLPROPSHEETEXT)this;
					 }else {
						 if (IsEqualIID(riid, IID_IShellCopyHook)){
								*ppv = (LPCOPYHOOK)this;
						 }
					 }
				 }
			 }
		 }
	 }
    if (*ppv){
        AddRef();
        return NOERROR;
    }
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release()
{
	if (--m_cRef){
       return m_cRef;
	}
    delete this;
    return 0L;
}
void CShellExt::Read_SelectedFile(){
	m_WdumpDocument.Read_File(m_SelectedFile);
	m_FileInMemory = true;
}
bool CShellExt::NotAdded(char* name){
	CString str = name;
	for(int pos(0); pos < m_NumAdded; pos ++){
		if(str == m_Textures[pos]){
			return false;
		}
	}
	m_Textures[m_NumAdded ++ ] = str;
	return true;
}
/////////////////////////////////////////////////////////////
