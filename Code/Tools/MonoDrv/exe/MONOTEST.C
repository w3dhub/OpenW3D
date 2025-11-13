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

#include "windows.h"
#include "winioctl.h"
#include "stdio.h"
#include "stdlib.h"
#include "monodrvr.h"
#include	"conio.h"


#define	MESSAGE1	"*** Page 1 ***\n"
#define	MESSAGE2	"*** Page 2 ***\n"
#define	MESSAGE3	"Another line of text.\n"


int Print(HANDLE handle, char const * text)
{
	int retval = 0;
	if (text != NULL)  {
		WriteFile(handle, text, strlen(text), &retval, NULL);
		printf(text);
	}
	return(retval);
}	


int __cdecl main(int argc, char *argv[])
{
	HANDLE handle1;
	HANDLE handle2;
	char attrib = 0x70;

	handle1 = CreateFile("\\\\.\\MONO", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle1 != INVALID_HANDLE_VALUE)  {
		int retval;
		struct  {
			int X,Y,W,H;
		} windowcontrol;
		int value;
		struct  {
			int X,Y,W,H,A;
		} fillcontrol;

		Print(handle1, MESSAGE1);

		printf("Press <ENTER> to proceed.\n");
		getchar();

		handle2 = CreateFile("\\\\.\\MONO", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle2 != INVALID_HANDLE_VALUE)  {
			Print(handle2, MESSAGE2);

			printf("Press <ENTER> to proceed again.\n");getchar();

			DeviceIoControl(handle1, (DWORD)IOCTL_MONO_BRING_TO_TOP, NULL, 0, NULL, 0, &retval, 0);
			printf("First page should reappear.\n");getchar();

			DeviceIoControl(handle2, (DWORD)IOCTL_MONO_BRING_TO_TOP, NULL, 0, NULL, 0, &retval, 0);
			printf("Second page should reappear.\n");getchar();
			{
				unsigned short * ptr = NULL;
				unsigned short * ptr2 = NULL;

				DeviceIoControl(handle2, (DWORD)IOCTL_MONO_LOCK, NULL, 0, &ptr, sizeof(ptr), &retval, 0);
				if (ptr != NULL) {
					*ptr = 0x0720;
				}
				printf("Upper left character should blank out.\n");getchar();

				DeviceIoControl(handle1, (DWORD)IOCTL_MONO_BRING_TO_TOP, NULL, 0, NULL, 0, &retval, 0);
				DeviceIoControl(handle1, (DWORD)IOCTL_MONO_LOCK, NULL, 0, &ptr2, sizeof(ptr2), &retval, 0);
				if (ptr2 != NULL) {
					*ptr2 = 0x0720;
				}
				DeviceIoControl(handle1, (DWORD)IOCTL_MONO_UNLOCK, NULL, 0, NULL, 0, &retval, 0);
				printf("First page should NOT reappear (but it has been modified).\n");getchar();

				DeviceIoControl(handle2, (DWORD)IOCTL_MONO_BRING_TO_TOP, NULL, 0, NULL, 0, &retval, 0);
				if (ptr != NULL) {
					*ptr = 0x0721;
				}
				DeviceIoControl(handle2, (DWORD)IOCTL_MONO_UNLOCK, NULL, 0, NULL, 0, &retval, 0);
				printf("Second page is now modified after after a call to unlock for page 1.\n");getchar();
			}
			CloseHandle(handle2);
		} else {
			printf("Unable to open second device handle.\n");getchar();
		}

		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_BRING_TO_TOP, NULL, 0, NULL, 0, &retval, 0);
		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_SET_ATTRIBUTE, &attrib, 1, NULL, 0, &retval, 0);
		Print(handle1, MESSAGE3);
		printf("First page should reappear with new text.\n");getchar();

		windowcontrol.X = 10;
		windowcontrol.Y = 10;
		windowcontrol.W = 20;
		windowcontrol.H = 10;
		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_SET_WINDOW, &windowcontrol, sizeof(windowcontrol), NULL, 0, &retval, 0);
		Print(handle1, "This text should appear in a window that is of limited size. You should notice that the text wraps at the right margin.\n");
		printf("Sub window printing -- no wrap at right margin.\n");getchar();

		value = MONOFLAG_WRAP;
		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_SET_FLAG, &value, sizeof(value), NULL, 0, &retval, 0);
		Print(handle1, "\fThis text should appear in a window that is of limited size. You should notice that the text wraps at the right margin.\n");
		printf("Sub window printing -- wrapping at right margin.\n");getchar();


		fillcontrol.X = 0;
		fillcontrol.Y = 0;
		fillcontrol.W = 100;
		fillcontrol.H = 100;
		fillcontrol.A = 0x0F;
		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_FILL_ATTRIB, &fillcontrol, sizeof(fillcontrol), NULL, 0, &retval, 0);
		printf("Screen attribute change.\n");getchar();

		fillcontrol.X = 0;
		fillcontrol.Y = 0;
		fillcontrol.W = 100;
		fillcontrol.H = 100;
		fillcontrol.A = 0x07;
		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_FILL_ATTRIB, &fillcontrol, sizeof(fillcontrol), NULL, 0, &retval, 0);
		printf("Screen attribute restore.\n");getchar();


		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_PAN, NULL, 0, NULL, 0, &retval, 0);
		printf("Now it should pan over one column.\n");getchar();

		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_SCROLL, NULL, 0, NULL, 0, &retval, 0);
		printf("Now it should scroll up one row.\n");getchar();

		attrib = 0x07;
		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_SET_ATTRIBUTE, &attrib, 1, NULL, 0, &retval, 0);
		DeviceIoControl(handle1, (DWORD)IOCTL_MONO_CLEAR_SCREEN, NULL, 0, NULL, 0, &retval, 0);

		CloseHandle(handle1);
	}

#ifdef NEVER
	HANDLE  hDriver;
	UCHAR   outputString[] = "Test Message\nfor the monochrome device.\n";
	UCHAR   altString[] = "'\t','\n'";
	UCHAR topline[] = "Top line of screen.\n\n\n\n";
	DWORD   cbReturned;
	struct {
		int X;
		int Y;
	} cursorpos;

	if ((hDriver = CreateFile("\\\\.\\MONO", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != ((HANDLE)-1))  {
		printf("\nRetrieved valid handle for MONO driver\n");
	} else {
		printf("Can't get a handle to MONO driver\n");
		return 1;
	}

	WriteFile(hDriver, outputString, strlen(outputString), &cbReturned, NULL);
	printf("WriteFile says it wrote out %d bytes.\n", cbReturned);


	if (DeviceIoControl(hDriver, (DWORD) IOCTL_MONO_PRINT_RAW, altString, sizeof(altString)-1, NULL, 0, &cbReturned, 0)) {
		printf("DeviceIoControl worked\n\n");

		cursorpos.X = 0;
		cursorpos.Y = 0;
		DeviceIoControl(hDriver, (DWORD)IOCTL_MONO_SET_CURSOR, (char*)&cursorpos, sizeof(cursorpos), NULL, 0, &cbReturned, 0);
		WriteFile(hDriver, topline, strlen(topline), &cbReturned, NULL);

		printf("Hit <Enter> to clear the mono display: \n");
		getchar();

		DeviceIoControl(hDriver, (DWORD) IOCTL_MONO_CLEAR_SCREEN, NULL, 0, NULL, 0, &cbReturned, 0);

		printf("'Bye\n");
	} else {
		printf("DeviceIoControl failed\n");
	}

	CloseHandle(hDriver);
#endif
	return 0;
}
