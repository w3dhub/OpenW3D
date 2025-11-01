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

#include "StdAfx.h"
#include "priv.h"
#include "shellext.h"
#include "resource.h"
#include <stdio.h>
#include "wdump.h"
#include "WdumpDoc.h"
#include "W3D_File.h"
#include <commctrl.h>





//===============================================================
//          ***********   G L O B A L S  ************************
extern  UINT g_DllRefCount;         // Reference count of this DLL.
extern	HINSTANCE g_DllInstance; // Handle to this DLL itself.
//===============================================================
//          ***********   F U N C T I O N S  *********************

///////////////////////////////////////
UINT CALLBACK W3DPageCallback(HWND hWnd,
                UINT uMessage,
                LPPROPSHEETPAGE  ppsp){
    switch(uMessage){
        case PSPCB_CREATE:
            return true;

        case PSPCB_RELEASE:{
            if (ppsp->lParam){
               ((LPCSHELLEXT)(ppsp->lParam))->Release();
            }
            return true; 
			}
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowHideControls(HWND hDlg, bool show){
	HWND hControl = GetWindow(hDlg, GW_CHILD);
	while(hControl != NULL){
		ShowWindow(hControl,show?SW_SHOW:SW_HIDE);
		hControl = GetWindow(hControl, GW_HWNDNEXT);
	}
	ShowWindow(GetDlgItem(hDlg, IDC_NODATA),show?SW_HIDE:SW_SHOW );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GetItemName(ChunkItem *pItem, int id_of_interest, void* pInfo, int sizeof_struct, int& found ){
	if(pItem->ID == id_of_interest){
		BYTE *byte_ptr = (BYTE*) pInfo;
		byte_ptr += sizeof_struct* found;
		memcpy(byte_ptr,pItem->Data,pItem->Length);
		found ++;
	}
	//Get all Sibitems for this item
	POSITION p = pItem->Chunks.GetHeadPosition();
	while(p != 0) {
		ChunkItem *subitem = pItem->Chunks.GetNext(p); 
		GetItemName(subitem, id_of_interest, pInfo, sizeof_struct,found);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK AnimPageDlgProc(HWND hDlg,UINT uMessage, WPARAM wParam, LPARAM lParam){
	LPPROPSHEETPAGE psp=(LPPROPSHEETPAGE)GetWindowLong(hDlg, DWL_USER);
	UINT iIndex=0;
	LPCSHELLEXT lpcs;
	char buf[MAX_PATH];
    switch (uMessage){
		case WM_INITDIALOG:{
            SetWindowLong(hDlg, DWL_USER, lParam);
            psp = (LPPROPSHEETPAGE)lParam;
            lpcs = (LPCSHELLEXT)psp->lParam;
				if(!lpcs->m_FileInMemory){
					lpcs->Read_SelectedFile();
				}
				ChunkData*	pData	= &(lpcs->m_WdumpDocument.m_ChunkData);
				assert(pData);
				POSITION		ptr = pData->Chunks.GetHeadPosition();;
				ASSERT(ptr);
				W3dAnimHeaderStruct* pAnim = lpcs->m_AnimInfos;
				int id_of_interest = W3D_CHUNK_ANIMATION_HEADER;
				int sizeof_struct = sizeof W3dAnimHeaderStruct;

				int found_animations(0);
				while(ptr){
					ChunkItem *pItem = pData->Chunks.GetNext(ptr);
					GetItemName(pItem, id_of_interest, (void*)lpcs->m_AnimInfos, sizeof_struct, found_animations );
				}
				if(found_animations){
					//So far assuming only one animation per file
					SetDlgItemText(hDlg, IDC_ANIMNAME, pAnim->Name);
					SetDlgItemText(hDlg, IDC_HNAME, pAnim->HierarchyName);
					SetDlgItemInt(hDlg, IDC_NUMFRAMES, pAnim->NumFrames,false);
					//FrameRate
					sprintf(buf,"%i fps",pAnim->FrameRate,false);
					SetDlgItemText(hDlg, IDC_FRAMERATE, buf);
					//SetDlgItemInt(hDlg, IDC_FRAMERATE, pAnim->FrameRate,false);
					//version
					sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(pAnim->Version),W3D_GET_MINOR_VERSION(pAnim->Version));
					SetDlgItemText(hDlg, IDC_ANIMVERSION, buf);
				}
				//Hierarchies
				ptr = pData->Chunks.GetHeadPosition();;
				ASSERT(ptr);
				id_of_interest = W3D_CHUNK_HIERARCHY_HEADER;
				//So far assuming only one hierarchy per file
				W3dHierarchyStruct* pHInfo = lpcs->m_Hierarchies;
				ASSERT(pHInfo);
				sizeof_struct = sizeof W3dHierarchyStruct;
				int found_hierarchies(0);
				while(ptr){
					ChunkItem *pItem = pData->Chunks.GetNext(ptr);
					GetItemName(pItem, id_of_interest, (void*)pHInfo, sizeof_struct, found_hierarchies );
				}
				if(found_hierarchies){
					SetDlgItemText(hDlg, IDC_HIERARCHYNAME, pHInfo->Name);
					SetDlgItemInt(hDlg, IDC_NUMPIVOTS, pHInfo->NumPivots,false);
					sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(pHInfo->Version),W3D_GET_MINOR_VERSION(pHInfo->Version));
					SetDlgItemText(hDlg, IDC_HIERARCHYVERSION, buf);
					sprintf(buf, "(%.3f, %.3f, %.3f)", pHInfo->Center.X, pHInfo->Center.Y, pHInfo->Center.Z);
					SetDlgItemText(hDlg, IDC_HCENTER, buf);
				}
			ShowHideControls(hDlg, (found_animations + found_hierarchies)!= 0);
         break;
			}
			case WM_DESTROY:{
				RemoveProp(hDlg, "ID");
				 break;
			}
        case WM_COMMAND:
            switch (LOWORD(wParam)){
                case IDC_TOP3D:
                    //SetProp(hDlg, "ID", (HANDLE)lParam);
                    break;
                default:
                    break;
            }
					break;
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code){
                case PSN_SETACTIVE:
                    break;
                case PSN_APPLY:
                    //User has clicked the OK or Apply button so we'll
                    //update the icon information in the .W3D file
                    lpcs = (LPCSHELLEXT)psp->lParam;
                    //Ask the shell to refresh the icon list...
                    //SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSHNOWAIT, 0, 0);
                    break;
                default:
                    break;
            }
            break;
        default:
            return false;
    }
    return true;
}
//=====================================================================================================
void SetDlgMeshParams(HWND hDlg, W3dMeshHeader3Struct*pInfo){
	SetDlgItemText(hDlg,IDC_MESHNAME,pInfo->MeshName);
	SetDlgItemText(hDlg,IDC_CONTAINER,pInfo->ContainerName);
	SetDlgItemInt(hDlg,IDC_NUM_MATERIALS, pInfo->NumMaterials,false);

	SetDlgItemInt(hDlg,IDC_NUM_POLYS, pInfo->NumTris,false);
	SetDlgItemInt(hDlg,IDC_NUM_VERTICES, pInfo->NumVertices, false);
	SetDlgItemInt(hDlg,IDC_NUM_MATERIALS, pInfo->NumMaterials,false);
	char msg[MAX_PATH];
	sprintf(msg, "%.3f", pInfo->SphRadius);
	SetDlgItemText(hDlg,IDC_SPHERERADIUS,msg);
	//3D Coords
	//Center
	sprintf(msg, "(%.3f, %.3f, %.3f)", pInfo->SphCenter.X, pInfo->SphCenter.Y, pInfo->SphCenter.Z);
	SetDlgItemText(hDlg, IDC_SPHERECENTER, msg);
	//Min
	sprintf(msg, "(%.3f, %.3f, %.3f)", pInfo->Min.X, pInfo->Min.Y, pInfo->Min.Z);
	SetDlgItemText(hDlg, IDC_MINDIM, msg);
	//Max
	sprintf(msg, "(%.3f, %.3f, %.3f)", pInfo->Max.X, pInfo->Max.Y, pInfo->Max.Z);
	SetDlgItemText(hDlg, IDC_MAXDIM, msg);

	sprintf(msg,"%d.%d",W3D_GET_MAJOR_VERSION(pInfo->Version),W3D_GET_MINOR_VERSION(pInfo->Version));
	SetDlgItemText(hDlg, IDC_VERSION, msg);
}
//==========================================================================================================
BOOL CALLBACK MeshPageDlgProc(HWND hDlg,UINT uMessage, WPARAM wParam, LPARAM lParam){
	LPPROPSHEETPAGE psp=(LPPROPSHEETPAGE)GetWindowLong(hDlg, DWL_USER);
	UINT iIndex=0;
	LPCSHELLEXT lpcs;

    switch (uMessage){
		case WM_INITDIALOG:{
            SetWindowLong(hDlg, DWL_USER, lParam);
            psp = (LPPROPSHEETPAGE)lParam;
            lpcs = (LPCSHELLEXT)psp->lParam;
				if(!lpcs->m_FileInMemory){
					lpcs->Read_SelectedFile();
				}
				ChunkData*	pData	= &(lpcs->m_WdumpDocument.m_ChunkData);
				assert(pData);
				POSITION		ptr = pData->Chunks.GetHeadPosition();
				ASSERT(ptr);
				//Look for mesh headers
				W3dMeshHeader3Struct* pInfo = lpcs->m_Meshes;
				int id_of_interest = W3D_CHUNK_MESH_HEADER3;
				int sizeof_struct = sizeof W3dMeshHeader3Struct;
				while(ptr){
					ChunkItem *pItem = pData->Chunks.GetNext(ptr);
					ASSERT(lpcs->m_FoundMeshes < MAX_MESH);
					GetItemName(pItem, id_of_interest, (void*)(pInfo),sizeof_struct, lpcs->m_FoundMeshes );
				}
				if(lpcs->m_FoundMeshes){
					SetDlgItemInt(hDlg,IDC_MESHNUMBER, 0,false);
					//Look for Textures
					char pTextureInfo[MAX_TEXUTRE_NAME_LEN * MAX_MESH];
					id_of_interest = W3D_CHUNK_TEXTURE_NAME;
					int found_textures(0);
					int sizeof_struct = MAX_TEXUTRE_NAME_LEN;
					ptr = pData->Chunks.GetHeadPosition();
					while(ptr){
						ChunkItem *pItem = pData->Chunks.GetNext(ptr);
						GetItemName(pItem, id_of_interest, (void*)pTextureInfo, sizeof_struct, found_textures );
					}
					for(int t_count(0); t_count < found_textures; t_count ++){
						if(lpcs->NotAdded(&(pTextureInfo[t_count*MAX_TEXUTRE_NAME_LEN]))){
							char* text = lpcs->m_Textures[lpcs->m_NumAdded-1].LockBuffer();
							SendDlgItemMessage(hDlg, IDC_TEXTURELIST, LB_ADDSTRING,(WPARAM) 0L, (LPARAM)text);//(pTextureInfo[t_count].name)) ;
							lpcs->m_Textures[lpcs->m_NumAdded-1].ReleaseBuffer();
						}
					}
					SetDlgItemInt(hDlg,IDC_NUM_MESHES, lpcs->m_FoundMeshes,false);
					SetDlgMeshParams(hDlg, pInfo);
					//Set Spin range
					SendDlgItemMessage(hDlg, IDC_MESHSPIN, UDM_SETRANGE, (WPARAM)0L, (LPARAM)MAKELONG(lpcs->m_FoundMeshes-1,0));
				}
				ShowHideControls(hDlg, lpcs->m_FoundMeshes != 0);
         break;
		 }
		case WM_DESTROY:
			RemoveProp(hDlg, "ID");
			break;
		case WM_COMMAND:

			switch (LOWORD(wParam)){
				case IDC_TOP3D:
					//SetProp(hDlg, "ID", (HANDLE)lParam);
					break;
				default:
					break;
			}
			break;
		case WM_NOTIFY:
			lpcs = (LPCSHELLEXT)psp->lParam;
			switch (((NMHDR FAR *)lParam)->code){
				case PSN_SETACTIVE:
					break;
				case PSN_APPLY:
					//Ask the shell to refresh the icon list...
					SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSHNOWAIT, 0, 0);
					break;
				case UDN_DELTAPOS:{
					lpcs = (LPCSHELLEXT)psp->lParam;
					W3dMeshHeader3Struct* pInfo = lpcs->m_Meshes;
					LPNMUPDOWN lpUD = (LPNMUPDOWN) lParam;
					int mesh_num = lpUD->iDelta + lpUD->iPos;
					if(mesh_num >= 0 && mesh_num < lpcs->m_FoundMeshes){
						SetDlgMeshParams(hDlg, pInfo+mesh_num);
					}
					break;
				}
				default:
					break;
			}
		break;
			default:
		return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////
BOOL CALLBACK PreviewPageDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam){

	LPPROPSHEETPAGE psp=(LPPROPSHEETPAGE)GetWindowLong(hDlg, DWL_USER);
    UINT iIndex(0);
    LPCSHELLEXT lpcs;
    switch (uMessage){
		case WM_INITDIALOG:{
            SetWindowLong(hDlg, DWL_USER, lParam);
            psp = (LPPROPSHEETPAGE)lParam;
            lpcs = (LPCSHELLEXT)psp->lParam;
				if(!lpcs->m_FileInMemory){
					lpcs->Read_SelectedFile();
				}
         break;
		 }
		case WM_DESTROY:{
            RemoveProp(hDlg, "ID");
            break;
		  }
        case WM_COMMAND:
            switch (LOWORD(wParam)){
                case IDC_TOP3D:
                    //SetProp(hDlg, "ID", (HANDLE)lParam);
                    break;
                default:
                    break;
            }

			break;
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code){
                case PSN_SETACTIVE:
                    break;
                case PSN_APPLY:
                    //User has clicked the OK or Apply button so we'll
                    //update the icon information in the .W3D file
                    lpcs = (LPCSHELLEXT)psp->lParam;
                    //Ask the shell to refresh the icon list...
                    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSHNOWAIT, 0, 0);
                    break;
                default:
                    break;
            }
            break;
        default:
            return false;
    }
    return true;
}



//  PURPOSE: Called by the shell just before the property sheet is displayed.
STDMETHODIMP CShellExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage,	//Pointer to the Shell's AddPage function
											LPARAM lParam){							//Passed as second parameter to lpfnAddPage
	 m_FileInMemory = false;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hpage;
    FORMATETC fmte = {CF_HDROP,(DVTARGETDEVICE FAR *)NULL,DVASPECT_CONTENT,-1, TYMED_HGLOBAL };
    STGMEDIUM medium;
	HRESULT hres = 0;
//	char buf[MAX_PATH];
	if (m_pDataObj){  //Paranoid check, m_pDataObj should have something by now...
       hres = m_pDataObj->GetData(&fmte, &medium);
	}
    if (SUCCEEDED(hres)){
        //Find out how many files the user has selected...
        UINT cbFiles = 0;
        LPCSHELLEXT lpcsext = this;

        if (medium.hGlobal){
            cbFiles = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, 0, 0);
		  }
        if (cbFiles < 2){
			  if (cbFiles){
               DragQueryFile((HDROP)medium.hGlobal,0, m_SelectedFile, sizeof(m_SelectedFile));
			  }
				psp.dwSize      = sizeof(psp);	// no extra data.
            psp.dwFlags     = PSP_USEREFPARENT | PSP_USETITLE | PSP_USECALLBACK;
            psp.hInstance   = g_DllInstance;
            psp.pszTemplate = MAKEINTRESOURCE(IDD_MESH);
            psp.hIcon       = 0;
            psp.pszTitle    = "   Meshes   ";
            psp.pfnDlgProc  = MeshPageDlgProc;
            psp.pcRefParent = &g_DllRefCount;
            psp.pfnCallback = W3DPageCallback;
            psp.lParam      = (LPARAM)lpcsext;
            AddRef();
            hpage = CreatePropertySheetPage(&psp);
             if(hpage){
                if (!lpfnAddPage(hpage, lParam)){
                    DestroyPropertySheetPage(hpage);
                    Release();
                }
            }
				 //Animation Page
            psp.pszTemplate = MAKEINTRESOURCE(IDD_ANIMPAGE);
            psp.pszTitle    = "Animations";
            psp.pfnDlgProc  = AnimPageDlgProc;
            AddRef();
            hpage = CreatePropertySheetPage(&psp);
             if(hpage){
                if (!lpfnAddPage(hpage, lParam)){
                    DestroyPropertySheetPage(hpage);
                    Release();
                }
            }
				 //Preview Page
            psp.pszTemplate = MAKEINTRESOURCE(IDD_PREVIEW);
            psp.pszTitle    = " Preview ";
            psp.pfnDlgProc  = PreviewPageDlgProc;
            AddRef();
            hpage = CreatePropertySheetPage(&psp);
             if(hpage){
                if (!lpfnAddPage(hpage, lParam)){
                    DestroyPropertySheetPage(hpage);
                    Release();
                }
            }
        }
    }
    return NOERROR;
}
//  PURPOSE: Called by the shell only for Control Panel property sheet 
STDMETHODIMP CShellExt::ReplacePage(UINT uPageID,									//ID of page to be replaced
                                    LPFNADDPROPSHEETPAGE lpfnReplaceWith,  //Pointer to the Shell's Replace function
                                    LPARAM lParam){									//Passed as second parameter to lpfnReplaceWith
    return E_FAIL;//we don't support this function.  It should never be
}
