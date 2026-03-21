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

#ifndef _SHELLEXT_H
#define _SHELLEXT_H

#define MAX_TEXTURES_INFILE	32
#define MAX_TEXUTRE_NAME_LEN	32
#define MAX_MESH					128
#define MAX_ANIMS_INFILE		16

//#define ODS(sz) OutputDebugString(sz)
//#define ENABLE_MSG2
//#define ENABLE_MSG
//#define	ENABLE_MSG3
#if defined ENABLE_MSG
#define ODS(sz)  MessageBox(NULL, sz, "Debug Message", MB_OK)
#else
#define ODS(sz)
#endif


#if defined ENABLE_MSG2
#define ODS2(sz)  MessageBox(NULL, sz, "Debug Message", MB_OK)
#else
#define ODS2(sz)
#endif

#if defined ENABLE_MSG3
#define ODS3(sz)  MessageBox(NULL, sz, "Debug Message", MB_OK)
#else
#define ODS3(sz)
#endif

#include "W3D_File.h"
#include "wdump.h"
#include "WdumpDoc.h"
//#include "w3d2dat.h"			/// LFeenanEA: Header file missing, perhaps this tool is outdated?


// {556F8779-49C4-4e88-9CEF-0AC2CFD6B763}
DEFINE_GUID(CLSID_ShellExtension, 0x556f8779L, 0x49c4, 0x4e88, 0x9c, 0xef, 0x0a, 0xc2, 0xcf, 0xd6, 0xb7, 0x63 );
class CShellExtClassFactory : public IClassFactory
{
protected:
	ULONG	m_cRef;

public:
	CShellExtClassFactory();
	~CShellExtClassFactory();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID *) override;
	STDMETHODIMP_(ULONG)	AddRef() override;
	STDMETHODIMP_(ULONG)	Release() override;

	//IClassFactory members
	STDMETHODIMP		CreateInstance(LPUNKNOWN, REFIID, LPVOID *) override;
	STDMETHODIMP		LockServer(BOOL) override;

};
typedef CShellExtClassFactory *LPCSHELLEXTCLASSFACTORY;
class CShellExt : public IContextMenu,
                         IShellExtInit,
                         IExtractIcon,
                         IPersistFile,
                         IShellPropSheetExt,
                         ICopyHook{
public:
    char         m_SelectedFile[MAX_PATH];
void Read_SelectedFile();
protected:
//	ITEMIDLIST m_idFolder;
	ULONG        m_cRef;
	LPDATAOBJECT m_pDataObj;
    char         m_szFileUserClickedOn[MAX_PATH];
	STDMETHODIMP DoW3DMenu1(HWND hParent, LPCSTR pszWorkingDir, LPCSTR pszCmd,LPCSTR pszParam, int iShowCmd);
public:
	bool NotAdded(char* name);
	//void Read_Selection(W3dMeshHeader3Struct&, W3D_HTree&, W3D_HAnim** );
	CShellExt();
	~CShellExt();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IShell members
	STDMETHODIMP			QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) override;
	STDMETHODIMP			InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi) override;
    STDMETHODIMP			GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *reserved, LPSTR pszName, UINT cchMax) override;
	//IShellExtInit methods
	STDMETHODIMP		    Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID) override;
    //IExtractIcon methods
    STDMETHODIMP GetIconLocation(UINT   uFlags,LPSTR  szIconFile,UINT   cchMax,int   *piIndex,UINT  *pwFlags) override;
    STDMETHODIMP Extract(LPCSTR pszFile,UINT   nIconIndex,HICON  *phiconLarge,HICON  *phiconSmall,UINT   nIconSize) override;
    //IPersistFile methods
    STDMETHODIMP GetClassID(LPCLSID lpClassID) override;
    STDMETHODIMP IsDirty() override;
    STDMETHODIMP Load(LPCOLESTR lpszFileName, DWORD grfMode) override;
    STDMETHODIMP Save(LPCOLESTR lpszFileName, BOOL fRemember) override;
    STDMETHODIMP SaveCompleted(LPCOLESTR lpszFileName) override;
    STDMETHODIMP GetCurFile(LPOLESTR * lplpszFileName) override;
    //IShellPropSheetExt methods
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam) override;
    STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam) override;
    //ICopyHook method
    STDMETHODIMP_(UINT) CopyCallback(HWND hwnd, UINT wFunc, UINT wFlags, LPCSTR pszSrcFile, DWORD dwSrcAttribs,LPCSTR pszDestFile, DWORD dwDestAttribs) override;
public:
	W3dAnimHeaderStruct	m_AnimInfos[MAX_ANIMS_INFILE];
	W3dHierarchyStruct	m_Hierarchies[MAX_ANIMS_INFILE];
	CString					m_Textures[MAX_TEXTURES_INFILE];
	int						m_NumAdded;
	bool						m_FileInMemory;
	CWdumpDoc				m_WdumpDocument;
	int						m_FoundMeshes;
	W3dMeshHeader3Struct m_Meshes[MAX_MESH];
};

typedef CShellExt *LPCSHELLEXT;
#endif // _SHELLEXT_H
