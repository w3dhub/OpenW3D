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

/*****************************************************************************\
          C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S
*******************************************************************************
 File:       patch.cpp
 Programmer: Neal Kettler

 StartDate:  Feb  6, 1998
 LastUpdate: Feb 10, 1998
-------------------------------------------------------------------------------

This is where all the code is for applying various types of patches.

\*****************************************************************************/


#include "patch.h"
#include <shellapi.h>
#include <direct.h>


//
// For the text box showing patch info
//
BOOL CALLBACK Update_Info_Proc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

  static int unselectText=0;

  switch(iMsg)
  {
    case WM_INITDIALOG:
    {
      FILE *in = fopen("launcher.txt","r");
      if (in==NULL)
      {
        EndDialog(hwnd,-1);
        return(1);
      }

      char   line[270];
      int    lastsel=0;
      char  *cptr=NULL;
      while(fgets(line,255,in))
      {
        //Get rid of any trailing junk
        while(1)
        {
          if (strlen(line)<1)
            break;
          cptr=line+(strlen(line))-1;
          if ((*cptr=='\r')||(*cptr=='\n'))
            *cptr=0;
          else
            break;
        }
        // ...and add back the gunk that windows likes
        strcat(line,"\r\r\n");

	     SendDlgItemMessage(hwnd, IDC_TEXT, EM_SETSEL, (WPARAM)lastsel, (LPARAM)lastsel );
	     SendDlgItemMessage(hwnd, IDC_TEXT, EM_REPLACESEL, 0, (LPARAM)(line) );
	     SendDlgItemMessage(hwnd, IDC_TEXT, EM_GETSEL, (WPARAM)NULL, (LPARAM)&lastsel );
      }
      unselectText=1;
      fclose(in);

      return(1);   // 1 means windows handles focus issues
    }
    break;

    case WM_PAINT:
      if (unselectText)
        SendDlgItemMessage(hwnd, IDC_TEXT, EM_SETSEL, -1, 0);
      unselectText=0;
      return(0);
    break;


    case WM_COMMAND:
      switch(wParam) {
        case IDOK:
        {
          EndDialog(hwnd,0);
          return(1);
        }
        default:
        break;
      }
      default:
    break;
    case WM_CLOSE:
      EndDialog(hwnd,0);
      return(1);
    break;
  }
  return(FALSE);
}







// Restart the computer for certain types of patches
void Shutdown_Computer_Now(void);


__declspec(dllexport) LPVOID CALLBACK PatchCallBack(UINT ID, LPVOID Param);

typedef LPVOID (CALLBACK* PATCHCALLBACK)(UINT, LPVOID);
typedef	UINT (CALLBACK *PATCHFUNC)( LPSTR, PATCHCALLBACK, BOOL);

int rtpErrCode = -1;


DWORD CALLBACK ApplyPatchThread(LPVOID _file) {

	char *patchfile = (char *)_file;

    HINSTANCE hInst=LoadLibrary("patchw32.dll");
    if (hInst==NULL)
    {
      char message[256];
      LoadString(NULL,IDS_ERR_MISSING_FILE,message,256);
      char string[256];
      sprintf(string,message,"patchw32.dll");
      char title[128];
      LoadString(NULL,IDS_ERROR,title,128);
      MessageBox(NULL,string,title,MB_OK);
      rtpErrCode = -2;
      return -1;
    }

    PATCHFUNC patchFunc;
    patchFunc=(PATCHFUNC)GetProcAddress(hInst,"RTPatchApply32@12");
    if (patchFunc==NULL)
    {
      char message[256];
      LoadString(NULL,IDS_BAD_LIBRARY,message,256);
      char title[128];
      LoadString(NULL,IDS_ERROR,title,128);
      MessageBox(NULL,message,title,MB_OK);
      rtpErrCode = -2;
      FreeLibrary(hInst);     // unload the DLL
      return -1;
    }

    char patchArgs[200];
    sprintf(patchArgs,"\"%s\" .",patchfile);

    rtpErrCode = patchFunc(patchArgs,PatchCallBack,TRUE);

    FreeLibrary(hInst);     // unload the DLL
	return 0;

}

//
//  Apply any type of patch.  Filename in patchfile.  Product base registry
//    (eg: "SOFTWARE\Westwood\Red Alert") should be in the config file as
//  SKUX SKU base reg dir   where X = index
//
void Apply_Patch(char *patchfile,ConfigFile &config,int skuIndex, bool show_dialog)
{
  DBGMSG("PATCHFILE : "<<patchfile);
  char cwdbuf[256];
  _getcwd(cwdbuf,255);
  DBGMSG("CWD : "<<cwdbuf);

  //
  // If patch is a .exe type patch
  //
  if (strcasecmp(patchfile+strlen(patchfile)-strlen(".exe"),".exe")==0)
  {
    // Set this as a run once service thing
    HKEY   regKey;
    LONG   regRetval;
    DWORD  regPrevious;
    regRetval=RegCreateKeyEx(
      HKEY_CURRENT_USER,
      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
      0,
      "",
      REG_OPTION_NON_VOLATILE,
      KEY_ALL_ACCESS,
      NULL,
      &regKey,
      &regPrevious);

    if (regRetval==ERROR_SUCCESS)
    {
      RegSetValueEx(regKey,"EXEPatch",0,REG_SZ,(const uint8*)patchfile,strlen(patchfile)+1);

      char message[256];
      LoadString(NULL,IDS_SYS_RESTART,message,256);
      char title[128];
      LoadString(NULL,IDS_SYS_RESTART_TITLE,title,128);

      MessageBox(NULL,message,title,MB_OK);

      Shutdown_Computer_Now();
    }
    else
    {
      char message[256];
      LoadString(NULL,IDS_RUNONCE_ERR,message,256);
      char string[256];
      sprintf(string,message,patchfile);
      MessageBox(NULL,string,"ERROR",MB_OK);
    }
  }
  //
  // RTPatch type patch
  //
  else if (strcasecmp(patchfile+strlen(patchfile)-strlen(".rtp"),".rtp")==0)
  {
    MSG msg;
    HWND dialog=Create_Patch_Dialog();
    while(PeekMessage(&msg,NULL,0,0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    DBGMSG("Patch SKU = "<<skuIndex);

	DWORD patchID = 0;
	HANDLE patchThread = CreateThread(NULL, 0, ApplyPatchThread, (LPVOID)patchfile, 0, &patchID);

	int dotcnt = 9;
	DWORD last_time = GetTickCount();
	BOOL forward = FALSE;
	char worktext[256];
    LoadString(NULL,IDS_WORKING_TEXT,worktext,sizeof(worktext)-12);
	int scnt = strlen(worktext)+1;
	strcat(worktext, " ..........");
	// Wait for the thread to finish
	while (rtpErrCode == -1) {
		if (GetTickCount() - last_time > 100) {
			if (forward) {
				worktext[scnt+dotcnt] = '.';
				if (dotcnt == 9) forward = FALSE;
				else dotcnt++;
			} else {
				worktext[scnt+dotcnt] = 0;
				if (dotcnt == 0) forward = TRUE;
				else dotcnt--;
			}
			SetWindowText(GetDlgItem(PatchDialog,IDC_WORKING_BOX),(char *)worktext);
			last_time = GetTickCount();
		}
		while(PeekMessage(&msg,NULL,0,0, PM_REMOVE))
		{
		  TranslateMessage(&msg);
		  DispatchMessage(&msg);
		}
		Sleep(5);
	}
	CloseHandle(patchThread);

    _unlink(patchfile);     // delete the patch

    DestroyWindow(dialog);  // get rid of the dialog

	// Don't update the registry if the patch failed to update.
	if (rtpErrCode != 0)
	{
		ERRMSG("Patch error: " << rtpErrCode);
		return;
	}

    // Now we have to update the registry so the version is correct
    // The version is the first integer in the filename
    // Eg: 22456_patch.rtp  means version 22456 goes into the registry

    // The version# starts after the last '\' char
    char   *cptr=patchfile;
    char   *tempPtr;
    DWORD   version;
    while( (tempPtr=strchr(cptr,'\\')) !=NULL)
      cptr=tempPtr+1;
    if (cptr)
      version=atol(cptr);
    DBGMSG("VERSION TO = "<<version);


    char      string[256];
    Wstring   key;
    // Get the InstallPath from the specified registry key
    sprintf(string,"SKU%d",skuIndex);
    if (config.getString(string,key)==FALSE)
    {
      ERRMSG("SKU is missing from config file!");
      return;
    }

    int       temp=0;
    Wstring   sku;
    Wstring   path;
    temp=key.getToken(temp," ",sku);
    path=key;
    path.remove(0,temp);
    while((*(path.get()))==' ')  // remove leading spaces
      path.remove(0,1);
    // Open the registry key for modifying now...
    HKEY regKey;
    LONG regRetval;
    regRetval=RegOpenKeyEx(HKEY_CURRENT_USER,path.get(),0,
        KEY_ALL_ACCESS,&regKey);
    if (regRetval!=ERROR_SUCCESS)
      DBGMSG("Can't open reg key for writing");
    regRetval=RegSetValueEx(regKey,"Version",0,REG_DWORD,(uint8 *)&version,
        sizeof(version));


		// Create blocking DLG for update info
		if (show_dialog) {
			DialogBox(Global_instance,MAKEINTRESOURCE(IDD_CHANGELOG),NULL,(DLGPROC)Update_Info_Proc);
		}
  }
  //
  // Execute now (without rebooting) type patch
  //
  else if (strcasecmp(patchfile+strlen(patchfile)-strlen(".exn"),".exn")==0)
  {
    Process proc;
    strcpy(proc.directory,".");
    strcpy(proc.command,patchfile);
    Create_Process(proc);
    Wait_Process(proc);
    _unlink(patchfile);
  }
  //
  // Web link type patch
  //
  // Im about 99.44% sure that this is completely useless.
  //
  else if (strcasecmp(patchfile+strlen(patchfile)-strlen(".web"),".web")==0)
  {
    char message[256];
    LoadString(NULL,IDS_WEBPATCH,message,256);
    char title[128];
    LoadString(NULL,IDS_WEBPATCH_TITLE,title,128);
    MessageBox(NULL,message,title,MB_OK);

    FILE *in=fopen(patchfile,"r");
    if (in!=NULL)
    {
      char URL[256];
      fgets(URL,255,in);
      fclose(in);
      ShellExecute(NULL,NULL,URL,NULL,".",SW_SHOW);
      _unlink(patchfile);
      //// This is somewhat skanky, but we can't wait
      //// for the viewer to exit (I tried).
      exit(0);
    }
    else
    {
      MessageBox(NULL,patchfile,"Patchfile vanished?",MB_OK);
    }
  }
}



void Shutdown_Computer_Now(void)
{
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;

  // Get a token for this process.
  if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
  {
    //error("OpenProcessToken");
  }

  // Get the LUID for the shutdown privilege.
  LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
        &tkp.Privileges[0].Luid);

  tkp.PrivilegeCount = 1;  // one privilege to set
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  // Get the shutdown privilege for this process.
  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
        (PTOKEN_PRIVILEGES)NULL, 0);

  // Cannot test the return value of AdjustTokenPrivileges.
  if (GetLastError() != ERROR_SUCCESS)
  {
    //error("AdjustTokenPrivileges");
  }

  // Shut down the system and force all applications to close.
  if (!ExitWindowsEx(EWX_REBOOT, 0))
  {
    // Should never happen
    char restart[128];
    LoadString(NULL,IDS_MUST_RESTART,restart,128);
    MessageBox(NULL,restart,"OK",MB_OK);
    exit(0);
  }

  MSG     msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage( &msg );
    DispatchMessage( &msg );
  }
}



//
// Callback during the patching process
//
__declspec(dllexport) LPVOID CALLBACK PatchCallBack(UINT Id, LPVOID Param)
{
  char         string[128];
  static int   fileCount=0;    // number of files to be patched
  static int   currFile=0;

  // Make sure our windows get updated
//  MSG     msg;
  int     counter=0;


// Let the main thread do this now.
//  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE ))
//  {
//    TranslateMessage( &msg );
//    DispatchMessage( &msg );
//    counter++;
//    if (counter==100)     // just in case...
//      break;
//  }

  LPVOID RetVal="";
  bit8 Abort=FALSE;

  //// using the global Dialog pointer, set the current "error" code
  //g_DlgPtr->SetRTPErrCode(Id);

  int percent;

  switch( Id )
  {
   case 1:
	case 2:
      // Warning message header/text
	  DBGMSG("P_MSG: "<<((char *)Param));
	break;

	case 3:
      // Error message header
     DBGMSG("P_MSG: "<<((char *)Param));
	break;

	case 4:
	  // Error message header/text
      ///////*g_LogFile << (char *)Parm << endl;
      char errmsg[256];
      LoadString(NULL,IDS_ERR_PATCH,errmsg,256);
      MessageBox(NULL,(char *)Param,errmsg,MB_OK);
      {
        FILE *out=fopen("patch.err","a");
        time_t  timet=time(NULL);
        fprintf(out,"\n\nPatch Error: %s\n",ctime(&timet));
        fprintf(out,"%s\n",(char *)Param);
        fclose(out);
      }
	break;

    case 9:
      // progress message
    break;

    case 0xa:
	  // help message
	break;

	case 0xb:
	  // patch file comment
	break;

	case 0xc:
	  // copyright message
	break;  // these just display text

    case 5:
	  // % completed
	  // so adjust the progress bar using the global Dialog pointer
	  /////////g_DlgPtr->SetProgressBar((int)((float)(*(UINT *)Parm)/(float)0x8000*(float)100));
      percent=((*(UINT *)Param)*100)/0x8000;
      PostMessage(GetDlgItem(PatchDialog,IDC_PROGRESS2),PBM_SETPOS,percent,0);
	break;

	case 6:
	  // Number of patch files
      DBGMSG("6: "<<*((uint32 *)Param));
      fileCount=*((uint32 *)Param);
      currFile=0;
	break;

	case 7:
	  //// begin patch
	  //LoadString(g_AppInstance, IDS_PROCESSING, lpcBuf, 256);
	  //strcpy(buf,lpcBuf);
	  //strcat(buf,(char *)Parm);
	  //g_DlgPtr->SetProgressText(buf);
	  //*g_LogFile << buf << " : ";
	  //fileModified = true;

      DBGMSG("7: "<<(char *)Param);
//      PostMessage(GetDlgItem(PatchDialog,IDC_FILENAME),WM_SETTEXT,0,(LPARAM)Param);
      SetWindowText(GetDlgItem(PatchDialog,IDC_FILENAME),(char *)Param);
      percent=0;
      PostMessage(GetDlgItem(PatchDialog,IDC_PROGRESS2),PBM_SETPOS,percent,0);

      currFile++;
      char xofy[64];
      LoadString(NULL,IDS_FILE_X_OF_Y,xofy,64);
      sprintf(string,xofy,currFile,fileCount);
      SetWindowText(GetDlgItem(PatchDialog,IDC_CAPTION),string);
//      PostMessage(GetDlgItem(PatchDialog,IDC_CAPTION),WM_SETTEXT,0,(LPARAM)string);

	break;

	case 8:
	  //// end patch
	  //LoadString(g_AppInstance, IDS_PROCCOMPLETE, lpcBuf, 256);
	  //g_DlgPtr->SetProgressText(lpcBuf);
	  //*g_LogFile << " complete" << endl;
      percent=100;
      PostMessage(GetDlgItem(PatchDialog,IDC_PROGRESS2),PBM_SETPOS,percent,0);
		DBGMSG("P_DONE");
	break;

	case 0xd:
	  //// this one shouldn't happen (only occurs if the command line
	  ////   doesn't have a patch file in it, and we insure that it does).
	  //Abort = TRUE;
	  //*g_LogFile << "Incorrect (or none) patch file specified in command line." << endl;
	break;

	case 0xe:
	  //// this one shouldn't happen either (same reason)
	  //Abort = TRUE;
	  //*g_LogFile << "Incorrect (or none) path specified in command line." << endl;
	break;

	case 0xf:
	  //// Password Dialog
	break;

	case 0x10:
	  //// Invalid Password Alert
	break;

	case 0x11:
	  //// Disk Change Dialog
	break;

	case 0x12:
	  //// Disk Change Alert
	break;

	case 0x13:
	  //// Confirmation Dialog
	break;

	case 0x14:
	  //// Location Dialog
	  //Abort = TRUE;
      //*g_LogFile << "Specified path is incorrect." << endl;
	break;

	case 0x16:
	  //// Searching Call-back
	break;

	case 0x15:
	 //// Idle...
    break;

	default:
	break;
  }

  if(Abort)
    return (NULL);
  else
	return (RetVal);
}