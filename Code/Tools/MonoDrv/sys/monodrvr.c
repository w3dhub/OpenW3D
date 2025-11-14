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

/* $Header$ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : NT Device Driver                                             *
 *                                                                                             *
 *                    File Name : MONODRVR.C                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 01/04/97                                                     *
 *                                                                                             *
 *                  Last Update : January 6, 1997 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Display_Signon_Banner -- Display the signon banner at driver start.                       * 
 *   DriverEntry -- Offical driver start entry point.                                          * 
 *   Mono_Bring_To_Top -- Brings the mono page to the visible top of the display stack.        * 
 *   Mono_Claim_Resources -- Claims resources for the MGA driver.                              * 
 *   Mono_Clear_Screen -- Clears the monochrome screen & homes cursor.                         *
 *   Mono_Detect_MGA_Adapter -- Try to detec the MGA adapter.                                  *
 *   Mono_Dispatch -- Main IRP dispatch handler routine.                                       * 
 *   Mono_Fetch_Ptr -- Fetch a USER address space pointer to mono buffer.                      * 
 *   Mono_Fill_Attribute -- Fills rectangle with attribute specified.                          * 
 *   Mono_Get_Address_Ptr -- Converts a physical address into a usable pointer.                *
 *   Mono_Init_Buffer -- Initialize a mono buffer to known (unused) state.                     * 
 *   Mono_Pan -- Pan the mono screen over one column.                                          *
 *   Mono_Print -- Prints text (with formatting) to mono screen.                               *
 *   Mono_Print_Raw -- Print text (without processing) to mono screen.                         *
 *   Mono_Printf -- Print formatted text to the mono device.                                   *
 *   Mono_Scroll -- Scroll the mono screen up one line.                                        *
 *   Mono_Set_View_Pos -- Set the mono display ram view offset.                                *
 *   Mono_Unload -- Unloads the driver from the system.                                        * 
 *   Mono_Unreport_Resource_Usage -- Free reported hardware usage.                             * 
 *   Mono_Update_Cursor -- Updates the CRTC cursor to reflect current visible page.            * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "ntddk.h"
#include "stdarg.h"
#include "stdio.h"
#include "monodrvr.h"


/*
**	Name constants.
*/
#define	DEVICE_NAME			L"\\Device\\Mono"			// Unicode internal device name.
#define	A_DEVICE_NAME		"\\Device\\Mono"			// ASCII internal device name.
#define	SYMBOLIC_NAME		L"\\DosDevices\\MONO"	// Unicode symbolic name.
#define	A_SYMBOLIC_NAME	"\\DosDevices\\MONO"		// ASCII symbolic name.
#define	RESOURCE_NAME		L"LOADED MONO DRIVER RESOURCES"	// Hardware registry name.

/*
**	MGA adapter hardware constants.
*/
#define	CRTC_REGISTER		0x3B4			// CRTC port address.
#define	CRTC_CONTROL		0x3B8			// CRTC control register address.
#define	MONO_MEMORY			0xB0000		// MGA video ram physical address.
#define	DEFAULT_ATTRIBUTE	0x07			// Normal character attribute (white on black).

/*
**	Mono dimension constants.
*/
#define	MONO_WIDTH			80				//	Number of characters wide each page.
#define	MONO_HEIGHT			25				// Number of lines tall each page.
#define	MONO_MEM_LENGTH	(0x10000-1)	// Number of mono video RAM bytes.
#define	MONO_PAGE_SIZE		(MONO_WIDTH * MONO_HEIGHT * sizeof(unsigned short))	// Page size (bytes)
#define	MONO_MAX_PAGES		((MONO_MEM_LENGTH+1) / MONO_PAGE_SIZE)		// Number of mono pages in video RAM.

/*
**	Helper pseudofunctions.
*/
#define	BUILD_MONO_CHAR(c, a)	((unsigned short)(((a) << 8) | ((c) & 0xFF)))
#define	PINDEX_TO_CELL(index)	((index) * MONO_WIDTH * MONO_HEIGHT)
#define	ARRAY_SIZE(x)				((int)(sizeof(x)/sizeof(x[0])))
#define	IS_FLAG_ON(x, flag)		((x->Flags & (1<<(flag))) != 0)
#define	FLAG_SET(x, flag)			x->Flags |= (1<<(flag))
#define	FLAG_CLEAR(x, flag)		x->Flags &= ~(1<<(flag))

/*
**	This controls the state information for each of the
**	display pages.
*/
typedef struct BuffControl
{
	/*
	**	This is the cursor position within the current window
	**	on the mono buffer.
	*/
	int XPos;
	int YPos;

	/*
	**	This specifies the dimensions of the current window
	**	on the mono display.
	*/
	int WinX;
	int WinY;
	int WinW;
	int WinH;

	/*
	**	This is the default attribute to use when displaying text.
	*/
	int Attribute;

	/*
	**	This is the logical address of the display page that this
	**	buffer represents. This pointer is in the address space
	**	of this driver, NOT of the owner of the file object. This
	**	pointer can and does change as the page stack changes.
	*/
	unsigned short * Buffer;

	/*
	**	This is a copy of a pointer to the page memory that is
	**	in mapped to the owning file object's address space. If this
	**	pointer is not NULL, then the page is locked (see LockCount).
	*/
	void * LockPtr;

	/*
	**	Records the number of times this mono page has been locked.
	**	If the page is locked, its position in the display stack cannot
	**	be changed.
	*/
	int LockCount;

	/*
	**	If this page has been allocated, then this flag will
	**	be true.
	*/
	int Allocated;

	/*
	**	These are the behavior flags for this buffer.
	*/
	int Flags;
} BuffControl;


/*
**	This stores the global information for the monochrome driver.
*/
typedef struct MonoGlobals
{
	/*
	**	Syncronizing object so that access to this device is
	**	serialized.
	*/
	KEVENT SyncEvent;

	/*
	**	Logical pointer to the CRTC registers.
	*/
	unsigned char * CRTCRegisters;

	/*
	**	Points to the first character of the MGA display
	**	memory.
	*/
	unsigned short * DisplayMemory;

	/*
	**	Each of the display pages is managed by this buffer
	**	control array.
	*/
	BuffControl Control[MONO_MAX_PAGES];

	/*
	**	This is the index of the currently visible display page. This
	**	means that the BuffControl indicated by this index has its
	**	Buffer pointer pointing to 0xB0000 physical memory.
	*/
	int CurrentVisible;

} MonoGlobals;


/*
**	Describes an MGA resource needed for this driver.
*/
typedef struct MonoResourceType
{
	unsigned int PhysicalAddress;
	unsigned int Length;
	unsigned int AddressSpace;
	unsigned int RangeSharable;
} MonoResourceType;


/*
**	List of MGA resources needed by this driver.
*/
MonoResourceType MonoResources[2] =
{
	{MONO_MEMORY, MONO_MEM_LENGTH, 0, 1},		// MGA memory
	{CRTC_REGISTER, 2, 1, 1}						// CRTC register

#ifdef NEVER
	,{CRTC_CONTROL, 1, 1, 1}		// Mode control register.
#endif
};


/*
**	This is the only public entry point for this driver.
*/
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

/*
**	Internal routines for the driver.
*/
static BOOLEAN Mono_Claim_Resources(PDRIVER_OBJECT DriverObject);
static BOOLEAN Mono_Detect_MGA_Adapter(void);
static NTSTATUS Mono_Dispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);
static void Mono_Fill_Attribute(BuffControl * control, int x, int y, int w, int h, int attrib);
static void * Mono_Fetch_Ptr(MonoGlobals * device, BuffControl * page);
static void * Mono_Get_Address_Ptr(PHYSICAL_ADDRESS PhysicalAddress, unsigned int AddressSpace, unsigned int NumberOfBytes);
static void Display_Signon_Banner(BuffControl * control);
static void Mono_Bring_To_Top(MonoGlobals * device, BuffControl * page);
static void Mono_Clear_Screen(BuffControl * control);
static void Mono_Free_Resources(PDRIVER_OBJECT DriverObject);
static void Mono_Init_Buffer(BuffControl * buffer);
static void Mono_Pan(BuffControl * control);
static void Mono_Print(BuffControl * control, unsigned char * string, unsigned int length);
static void Mono_Print_Raw(BuffControl * control, unsigned char * string, unsigned int length);
static void Mono_Printf(BuffControl * control, char const * DbgMessage, ...);
static void Mono_Scroll(BuffControl * control);
static void Mono_Set_View_Pos(MonoGlobals * device, int pos);
static void Mono_Unload(PDRIVER_OBJECT driver);
static void Mono_Update_Cursor(MonoGlobals * device);



/*********************************************************************************************** 
 * DriverEntry -- Offical driver start entry point.                                            * 
 *                                                                                             * 
 *    This routine is called by the system when the driver is first loaded. It is responsible  * 
 *    for initializing the driver state.                                                       * 
 *                                                                                             * 
 * INPUT:   driverobject   -- Pointer to the loaded driver object.                             * 
 *                                                                                             * 
 *          registry       -- Pointer to the registry string for this driver object.           * 
 *                                                                                             * 
 * OUTPUT:  Returns with the NTSTATUS of the driver initialization. If it is anything but      * 
 *          STATUS_SUCCESS, the the driver will not load.                                      * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	NTSTATUS error_flag = STATUS_UNSUCCESSFUL;

	/*
	**	Check the registry to see if the MGA resources are free to be claimed (claim them
	**	if they are) and also check that the MGA hardware is present. Only if both
	**	are true can the driver be loaded.
	*/
	if (Mono_Claim_Resources(DriverObject) && Mono_Detect_MGA_Adapter())  {
		PDEVICE_OBJECT device_object = NULL;
		UNICODE_STRING device_name;

		/*
		**	Creates the device object.
		*/
		RtlInitUnicodeString(&device_name, DEVICE_NAME);
		error_flag = IoCreateDevice(DriverObject, sizeof(MonoGlobals), &device_name, FILE_DEVICE_MONO, 0, false, &device_object);
		if (NT_SUCCESS(error_flag)) {
			MonoGlobals * mono_globals = (MonoGlobals *)device_object->DeviceExtension;

			/*
			**	This enables the WriteFile/ReadFile Win32 functions.
			*/
			device_object->Flags |= DO_BUFFERED_IO;

			/*
			** Initialize the dispatch event object. This allows us to
			** synchronize access to the h/w registers...
			*/
			KeInitializeEvent(&mono_globals->SyncEvent, SynchronizationEvent, true);

			/*
			** Map all the required resources, save the addresses
			*/
			mono_globals->DisplayMemory = Mono_Get_Address_Ptr(RtlConvertUlongToLargeInteger(MONO_MEMORY), 0, MONO_MEM_LENGTH);
			mono_globals->CRTCRegisters =	Mono_Get_Address_Ptr(RtlConvertUlongToLargeInteger(CRTC_REGISTER), 1, 2);
			if (mono_globals->DisplayMemory != NULL && mono_globals->CRTCRegisters != NULL) {
				UNICODE_STRING link_name;

				/*
				** Create a symbolic link that Win32 apps can specify to gain access
				** to this driver/device
				*/
				RtlInitUnicodeString(&link_name, SYMBOLIC_NAME);
				error_flag = IoCreateSymbolicLink(&link_name, &device_name);
				if (NT_SUCCESS(error_flag)) {
					int index;

					/*
					** Create dispatch points for device control, create, close.
					*/
					DriverObject->MajorFunction[IRP_MJ_CREATE] = Mono_Dispatch;
					DriverObject->MajorFunction[IRP_MJ_CLOSE] = Mono_Dispatch;
					DriverObject->MajorFunction[IRP_MJ_READ] = Mono_Dispatch;
					DriverObject->MajorFunction[IRP_MJ_WRITE] = Mono_Dispatch;
					DriverObject->MajorFunction[IRP_MJ_CLEANUP] = Mono_Dispatch;
					DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Mono_Dispatch;
					DriverObject->DriverUnload = Mono_Unload;

					/*
					** Set the Start Address registers (indicate which part of
					** video buffer is displayed) to 0
					*/
					Mono_Set_View_Pos(mono_globals, 0);

					mono_globals->CurrentVisible = 0;
					for (index = 0; index < MONO_MAX_PAGES; index++) {
						mono_globals->Control[index].Buffer = &mono_globals->DisplayMemory[PINDEX_TO_CELL(index)];
						Mono_Init_Buffer(&mono_globals->Control[index]);
					}

					/*
					**	Display the signon banner.
					*/
					Display_Signon_Banner(&mono_globals->Control[mono_globals->CurrentVisible]);
					Mono_Update_Cursor(mono_globals);
				}
			} else {
				error_flag = STATUS_UNSUCCESSFUL;
			}
		}
	} else {
		error_flag = STATUS_UNSUCCESSFUL;
	}

	/*
	**	If an error was detected during initialization, then unload the
	**	driver before returning the error flag to the system.
	*/
	if (!NT_SUCCESS(error_flag)) {
		Mono_Unload(DriverObject);
	}

	return(error_flag);
}


/*********************************************************************************************** 
 * Mono_Dispatch -- Main IRP dispatch handler routine.                                         * 
 *                                                                                             * 
 *    This routine is called whenever an IRP occurs. It is passed the driver object pointer    * 
 *    and the IRP info.                                                                        * 
 *                                                                                             * 
 * INPUT:   driver   -- Pointer to the device object.                                          * 
 *                                                                                             * 
 *          irp      -- Pointer to the IRP object.                                             * 
 *                                                                                             * 
 * OUTPUT:  Returns with the NTSTATUS of this IRP handler.                                     * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
NTSTATUS Mono_Dispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PIO_STACK_LOCATION irpStack;
	MonoGlobals *  mono_globals;
	void * ioBuffer;
	unsigned int inputBufferLength;
	unsigned int outputBufferLength;
	unsigned int ioControlCode;
	NTSTATUS ntStatus;
	FILE_OBJECT * fileobject = NULL;
	BuffControl * control = NULL;
	int currentindex = -1;

	/*
	**	Presume success in the command processing.
	*/
	Irp->IoStatus.Status      = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;		// Bytes read/written or other return value.

	/*
	** Get a pointer to the current location in the Irp. This is where
	**     the function codes and parameters are located.
	*/
	irpStack = IoGetCurrentIrpStackLocation(Irp);

	/*
	** Get a pointer to the device extension
	*/
	mono_globals = DeviceObject->DeviceExtension;

	/*
	**	Fetch the file object for this I/O request.
	*/
	fileobject = irpStack->FileObject;
	if (fileobject != NULL)  {
		currentindex = (int)fileobject->FsContext;
	}

	/*
	** Get the pointer to the input/output buffer and it's length
	*/
	ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
	inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

	/*
	** Synchronize execution of the dispatch routine by acquiring the device
	** event object. This ensures all request are serialized.
	*/
	KeWaitForSingleObject(&mono_globals->SyncEvent, Executive, KernelMode, false, NULL);

	/*
	**	Get a working pointer to the display page.
	*/
	if (currentindex == -1) {
		currentindex == mono_globals->CurrentVisible;
	}
	control = &mono_globals->Control[currentindex];

	switch (irpStack->MajorFunction) {
		case IRP_MJ_READ:
			break;

		/*
		**	Process WriteFile request to display text.
		*/
		case IRP_MJ_WRITE:
			if (ioBuffer != NULL && irpStack->Parameters.Write.Length > 0)  {
				Mono_Print(control, ioBuffer, irpStack->Parameters.Write.Length);
				Irp->IoStatus.Information = irpStack->Parameters.Write.Length;
			}
			break;

		/*
		**	The create event is called when a file object is first created. At that
		**	time, designate a context buffer and inialize it.
		*/
		case IRP_MJ_CREATE: {
				int j;
				int avail = -1;

				/*
				**	Search for a free page to use.
				*/
				for (j = 0; j < MONO_MAX_PAGES; j++) {
					if (!mono_globals->Control[j].Allocated) {
						avail = j;
						break;
					}
				}

				/*
				**	If one is not found, then return with error.
				*/
				if (avail == -1) {
					Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				} else {
					BuffControl * newcon = &mono_globals->Control[avail];
					Mono_Init_Buffer(newcon);
					newcon->Allocated = true;
					Mono_Clear_Screen(newcon);
					Mono_Bring_To_Top(mono_globals, newcon);
					fileobject->FsContext = (void*)avail;
				}
			}
			break;

		/*
		**	Closing the file occurs when the last reference to the object has been
		**	severed.
		*/
		case IRP_MJ_CLOSE:
			if (currentindex != -1) {
				Mono_Init_Buffer(&mono_globals->Control[currentindex]);
				fileobject->FsContext = NULL;
			}
			break;

		/*
		**	This is called prior to closing the file.
		*/
		case IRP_MJ_CLEANUP:
			break;

		/*
		**	All the special I/O control functions route through this
		**	IRP message.
		*/
		case IRP_MJ_DEVICE_CONTROL:
			ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

			switch (ioControlCode) {

				/*
				**	Set a driver flag.
				*/
				case IOCTL_MONO_SET_FLAG:
					if (inputBufferLength == sizeof(int))  {
						FLAG_SET(control, *(int*)ioBuffer);
					} else {
						Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
					}
					break;

				/*
				**	Clear a driver flag.
				*/
				case IOCTL_MONO_CLEAR_FLAG:
					if (inputBufferLength == sizeof(int))  {
						FLAG_CLEAR(control, *(int*)ioBuffer);
					} else {
						Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
					}
					break;

				/*
				**	Fill a region of the screen with a specific attribute.
				*/
				case IOCTL_MONO_FILL_ATTRIB:  
					if (inputBufferLength == sizeof(int)*5) {
						int x,y,w,h,a;

						x = *(((int*)ioBuffer) + 0);
						y = *(((int*)ioBuffer) + 1);
						w = *(((int*)ioBuffer) + 2);
						h = *(((int*)ioBuffer) + 3);
						a = *(((int*)ioBuffer) + 4);
						Mono_Fill_Attribute(control, x, y, w, h, a);
					} else {
						Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
					}
					break;

				/*
				**	Reset the sub-window back to full screen.
				*/
				case IOCTL_MONO_RESET_WINDOW:
					control->WinX = 0;
					control->WinY = 0;
					control->WinW = MONO_WIDTH;
					control->WinH = MONO_HEIGHT;
					control->XPos = 0;
					control->YPos = 0;
					break;

				/*
				**	Create a sub-window that restricts mono output to.
				*/
				case IOCTL_MONO_SET_WINDOW:
					if (inputBufferLength == sizeof(int)*4)  {
						control->WinX = *(((int*)ioBuffer) + 0);
						if (control->WinX < 0) control->WinX = 0;
						if (control->WinX >= MONO_WIDTH) control->WinX = MONO_WIDTH-1;

						control->WinY = *(((int*)ioBuffer) + 1);
						if (control->WinY < 0) control->WinY = 0;
						if (control->WinY >= MONO_HEIGHT) control->WinY = MONO_HEIGHT-1;
						
						control->WinW = *(((int*)ioBuffer) + 2);
						if (control->WinW < 1) control->WinW = 1;
						if (control->WinW > MONO_WIDTH-control->WinX) control->WinW = MONO_WIDTH-control->WinX;

						control->WinH = *(((int*)ioBuffer) + 3);
						if (control->WinH < 1) control->WinH = 1;
						if (control->WinH > MONO_HEIGHT-control->WinY) control->WinH = MONO_HEIGHT-control->WinY;
						
						control->XPos = 0;
						control->YPos = 0;
					} else {
						Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
					}
					break;

				/*
				**	Display the help screen (signon banner).
				*/
				case IOCTL_MONO_HELP_SCREEN:
					Display_Signon_Banner(control);
					break;

				/*
				**	Bring the mono page to the visible top, but only if it
				**	or the current visible top is not currently locked.
				*/
				case IOCTL_MONO_BRING_TO_TOP:
					Mono_Bring_To_Top(mono_globals, control);
					break;

				/*
				**	Scroll the screen up one row.
				*/
				case IOCTL_MONO_SCROLL:
					Mono_Scroll(control);
					break;

				/*
				**	Scroll the screen left one column.
				*/
				case IOCTL_MONO_PAN:
					Mono_Pan(control);
					break;

				/*
				**	Set the cursor position to the coordinates specified.
				*/
				case IOCTL_MONO_SET_CURSOR:
					if (inputBufferLength == sizeof(int)*2) {
						control->XPos = *(int*)ioBuffer;
						if (control->XPos < 0) control->XPos = 0;
						if (control->XPos >= control->WinW) control->XPos = control->WinW-1;

						control->YPos = *(((int*)ioBuffer) + 1);
						if (control->YPos < 0) control->YPos = 0;
						if (control->YPos >= control->WinH) control->YPos = control->WinH-1;
					} else {
						Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
					}
					break;

				/*
				**	Print text without any formatting processing.
				*/
				case IOCTL_MONO_PRINT_RAW:
					Mono_Print_Raw(control, ioBuffer, inputBufferLength);
					break;

				/*
				**	Clear the screen to blank (using the default attribute).
				*/
				case IOCTL_MONO_CLEAR_SCREEN:
					Mono_Clear_Screen(control);
					break;

				/*
				**	Set the default attribute to use for subsiquent output.
				*/
				case IOCTL_MONO_SET_ATTRIBUTE:
					control->Attribute = *(char *)ioBuffer;
					break;

				/*
				**	Lock the mono page and return a user-space mapped pointer to
				**	it.
				*/
				case IOCTL_MONO_LOCK:
					if (control->LockPtr == NULL)  {
						control->LockPtr = Mono_Fetch_Ptr(mono_globals, control);
					}
					if (control->LockPtr != NULL) {
						*(void**)ioBuffer = control->LockPtr;
						control->LockCount++;
						Irp->IoStatus.Information = sizeof(void *);
					} else {
						Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
					}
					break;

				/*
				**	Unlock the mono page.
				*/
				case IOCTL_MONO_UNLOCK:
					if (control->LockCount > 0) {
						control->LockCount--;

						if (control->LockCount == 0) {
							Irp->IoStatus.Status = ZwUnmapViewOfSection((HANDLE) -1, control->LockPtr);
							control->LockPtr = NULL;
						}
					}
					break;

				/*
				**	Unrecognized control code results in an error return value.
				*/
				default:
					Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
					break;
			}

			break;
	}

	/*
	**	Only if the context drawn to is the currently visible one, will the
	**	cursor position be updated.
	*/
	if (currentindex == mono_globals->CurrentVisible)  {
		Mono_Update_Cursor(mono_globals);
	}

	KeSetEvent(&mono_globals->SyncEvent, 0, false);

	ntStatus = Irp->IoStatus.Status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return(ntStatus);
}


/*********************************************************************************************** 
 * Mono_Unload -- Unloads the driver from the system.                                          * 
 *                                                                                             * 
 *    This will free any resources allocated by this driver, break any symbolic link, and      * 
 *    delete the driver object from the system. Effectively, this is the suicide function      * 
 *    for the MGA driver.                                                                      * 
 *                                                                                             * 
 * INPUT:   driver   -- Pointer to the driver object to delete.                                * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   Don't use the driver after calling this function.                               * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void Mono_Unload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING link_string;

	Mono_Free_Resources(DriverObject);

	RtlInitUnicodeString(&link_string, SYMBOLIC_NAME);
	IoDeleteSymbolicLink(&link_string);
	
	if (DriverObject->DeviceObject != NULL)  {
		IoDeleteDevice(DriverObject->DeviceObject);
	}
}


/***********************************************************************************************
 * Mono_Detect_MGA_Adapter -- Try to detec the MGA adapter.                                    *
 *                                                                                             *
 *    Tries to find the MGA adaptor by writing a few bytes to the CRTC register and checking   *
 *    if they return a corresponding value.                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the MGA card deteced?                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/05/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
BOOLEAN Mono_Detect_MGA_Adapter(void)
{
	unsigned char * crtc_registerss;

	/*
	**	Get address to the CRTC registers.
	*/
	crtc_registerss = Mono_Get_Address_Ptr(RtlConvertUlongToLargeInteger(CRTC_REGISTER), 1, 2);
	if (crtc_registerss != NULL)  {

		/*
		**	Set the mono cursor position and see if it changed. If so, then
		**	presume that the MGA adapter is present.
		*/
		WRITE_PORT_UCHAR(crtc_registerss, 0x0F);
		WRITE_PORT_UCHAR(crtc_registerss+1, 0x55);
		if (READ_PORT_UCHAR(crtc_registerss+1) != 0x55) {
			return(false);
		}
		WRITE_PORT_UCHAR(crtc_registerss+1, 0xAA);
		if (READ_PORT_UCHAR(crtc_registerss+1) != 0xAA) {
			return(false);
		}

		WRITE_PORT_UCHAR(crtc_registerss+1, 0x00);
		return(true);
	}

	return(false);
}


/***********************************************************************************************
 * Mono_Get_Address_Ptr -- Converts a physical address into a usable pointer.                  *
 *                                                                                             *
 *    This routine will take the physical address specified and convert it into a pointer that *
 *    can be used to reference the address specified.                                          *
 *                                                                                             *
 * INPUT:   address  -- The physical address to convert.                                       *
 *                                                                                             *
 *          space    -- The address space that the specified address is in. 0 means memory,    *
 *                      1 means I/O space (ports).                                             *
 *                                                                                             *
 *          length   -- The length of the memory that will be referenced.                      *
 *                                                                                             *
 * OUTPUT:  Returns with a usable pointer to the memory specified or NULL if it could not be   *
 *          processed.                                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/05/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void * Mono_Get_Address_Ptr(PHYSICAL_ADDRESS address, unsigned int space, unsigned int length)
{
	PHYSICAL_ADDRESS logical_address;
	void * usable_ptr = NULL;

	/*
	**	Translate a bus specific address into a logical system address.
	*/
	if (HalTranslateBusAddress(Isa, 0, address, &space, &logical_address)) {

		/*
		**	If the space value is zero, then this idicates that a call to
		**	MmMapIoSpace is required. This is usually required for port addresses.
		*/
		if (space == 0) {
			usable_ptr = MmMapIoSpace(logical_address, length, false);
		} else {
			usable_ptr = (void *)logical_address.LowPart;
		}
	}

	return(usable_ptr);
}


/*********************************************************************************************** 
 * Mono_Claim_Resources -- Claims resources for the MGA driver.                                * 
 *                                                                                             * 
 *    This routine will scan the registry in order to claim the hardware resources needed by   * 
 *    this MGA display driver. If any of the required resources have already been claimed      * 
 *    in an exclusive mode, then the driver should not load.                                   * 
 *                                                                                             * 
 * INPUT:   driver   -- Pointer to the driver object.                                          * 
 *                                                                                             * 
 * OUTPUT:  bool; Were the required resources claimed successfully?                            * 
 *                                                                                             * 
 * WARNINGS:   If the resources were not claimed, then the driver should not load.             * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
BOOLEAN Mono_Claim_Resources(PDRIVER_OBJECT driver)
{
	unsigned int resource_list_size = 0;
	PCM_RESOURCE_LIST resource_list = NULL;

	/*
	**	Allocate a resource descriptor list. This must be allocated because it is a variable
	**	size structure and the CM_RESOURCE_LIST is typedef'd as single entry lists (which
	**	is insufficient for our purposes).
	*/
	resource_list_size = sizeof(CM_RESOURCE_LIST) + (sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * (ARRAY_SIZE(MonoResources) - 1));
	resource_list = ExAllocatePool(PagedPool, resource_list_size);
	if (resource_list != NULL) {
		UNICODE_STRING class_name;
		PCM_PARTIAL_RESOURCE_DESCRIPTOR partial_resource_list;
		BOOLEAN conflict_flag;
		unsigned int index;
	
		RtlZeroMemory(resource_list, resource_list_size);

		/*
		**	There is one resource list (although multiple resources). They are
		**	all located on the Isa bus number 0.
		*/
		resource_list->Count = 1;
		resource_list->List[0].InterfaceType = Isa;
		resource_list->List[0].BusNumber     = 0;
		resource_list->List[0].PartialResourceList.Count = ARRAY_SIZE(MonoResources);

		/*
		**	Fill in the resrouce list with the resources required by the MGA
		**	device driver.
		*/
		partial_resource_list = &resource_list->List[0].PartialResourceList.PartialDescriptors[0];
		for (index = 0; index < ARRAY_SIZE(MonoResources); index++) {
			if (MonoResources[index].AddressSpace) {
				partial_resource_list->Type          = CmResourceTypePort;
				partial_resource_list->Flags         = CM_RESOURCE_PORT_IO;
				partial_resource_list->u.Port.Start  = RtlConvertUlongToLargeInteger(MonoResources[index].PhysicalAddress);
				partial_resource_list->u.Port.Length = MonoResources[index].Length;
			} else {
				partial_resource_list->Type            = CmResourceTypeMemory;
				partial_resource_list->Flags           = CM_RESOURCE_MEMORY_READ_WRITE;
				partial_resource_list->u.Memory.Start  = RtlConvertUlongToLargeInteger(MonoResources[index].PhysicalAddress);
				partial_resource_list->u.Memory.Length = MonoResources[index].Length;
			}

			if (MonoResources[index].RangeSharable) {
				partial_resource_list->ShareDisposition = CmResourceShareShared;
			} else {
				partial_resource_list->ShareDisposition = CmResourceShareDeviceExclusive;
			}

			partial_resource_list++;
		}

		/*
		**	Now that the resource list has been filled in, try to request the resources
		**	from the system. It does this by comparing the requested resources with those
		**	that have been claimed in the registry.
		*/
		RtlInitUnicodeString(&class_name, RESOURCE_NAME);
		IoReportResourceUsage(&class_name, driver, resource_list, resource_list_size, NULL, NULL, 0, false, &conflict_flag);

		ExFreePool(resource_list);
		
		return(conflict_flag == false);
	}
	return(false);
}


/*********************************************************************************************** 
 * Mono_Free_Resources -- Free reported hardware usage.                                        * 
 *                                                                                             * 
 *    This routine will clear the registry of the reported hardware usage by the mono          * 
 *    display driver. It should be called only when the driver is being unloaded.              * 
 *                                                                                             * 
 * INPUT:   driver   -- Pointer to the driver object.                                          * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void Mono_Free_Resources(PDRIVER_OBJECT driver)
{
	UNICODE_STRING class_name;
	BOOLEAN found_conflict;
	CM_RESOURCE_LIST resource_list =  {0};

	RtlInitUnicodeString(&class_name, RESOURCE_NAME);
	IoReportResourceUsage(&class_name, driver, &resource_list, sizeof(resource_list), NULL, NULL, 0, false, &found_conflict);
}



/*********************************************************************************************** 
 * Mono_Update_Cursor -- Updates the CRTC cursor to reflect current visible page.              * 
 *                                                                                             * 
 *    Use this routine to update the display cursor to match the recorded cursor position      * 
 *    for the currently visible page.                                                          * 
 *                                                                                             * 
 * INPUT:   device   -- Pointer to the mono driver globals.                                    * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void Mono_Update_Cursor(MonoGlobals * device)
{
	int pos;
	BuffControl * control = &device->Control[device->CurrentVisible];

	pos = control->XPos + control->WinX + ((control->YPos+control->WinY) * MONO_WIDTH);

	WRITE_PORT_UCHAR(device->CRTCRegisters, 0x0E);
	WRITE_PORT_UCHAR(device->CRTCRegisters+1, (unsigned char)(pos>>8));
	WRITE_PORT_UCHAR(device->CRTCRegisters, 0x0F);
	WRITE_PORT_UCHAR(device->CRTCRegisters+1, (unsigned char)(pos & 0xFF));
}


/***********************************************************************************************
 * Mono_Print -- Prints text (with formatting) to mono screen.                                 *
 *                                                                                             *
 *    This routine will take the text specified and print it to the monochrome screen. If      *
 *    there are any formatting characters present, then they will be processed accordingly.    *
 *                                                                                             *
 * INPUT:   control  -- Poitner to the buffer to print to.                                     *
 *                                                                                             *
 *          string   -- Pointer to the string to print.                                        *
 *                                                                                             *
 *          length   -- The length of the string to print.                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Mono_Print(BuffControl * control, unsigned char * string, unsigned int length)
{
	if (control != NULL)  {
		unsigned char space = ' ';
		int original_x = control->XPos;

		while (length > 0) {
			int x,y;
			unsigned char * blockstart;
			int blocklen;
			unsigned char bchar;		// Breaking character.

			/*
			**	Scan for a block of contiguous non-formatting characters to print.
			*/
			blockstart = string;
			blocklen = 0;
			bchar = '\0';
			x = control->XPos;
			while (length > 0 && bchar == '\0') {
				bchar = *string++;
				length--;

				/*
				**	Detect if this character is special and requires
				**	formatting control rather than simple displaying.
				*/
				switch (bchar) {
					case '\b':		// Backspace
					case '\a':		// Audible bell
					case '\t':		// Horizontal tab
					case '\f':		// Formfeed
					case '\n':		// Linefeed
					case '\v':		// Vertical tab
						break;

					/*
					**	If a <CR><LF> pair is detected, then treat it
					**	just as if it were a '\n'. This is for backward
					**	compatibility. If it is just a '\r', then it 
					**	functions similarly to the '\n' code.
					*/
					case '\r':
						if (length > 0 && *string == '\n')  {
							length--;
							string++;
							bchar = '\n';
						}
						break;

					/*
					**	If not a valid formatting character, then clear out the break
					**	character variable so that processing will continue.
					*/
					default:
						blocklen++;
						bchar = '\0';
						break;
				}
				
				/*
				**	If wrapping at right margin is desired, then check for 
				**	this condition and force a line break if margin is reached.
				*/
				if (bchar == '\0' && IS_FLAG_ON(control, MONOFLAG_WRAP))  {
					x++;
					if (x >= control->WinW)  {
						bchar = '\n';
					}
				}
			}

			/*
			**	If a block has been scanned, then print it at this time.
			*/
			if (blocklen > 0) {
				Mono_Print_Raw(control, blockstart, blocklen);
			}

			/*
			**	Conveniently get the cursor position into working variables.
			**	This helps because many of the formatting characters change
			**	the cursor position.
			*/
			x = control->XPos;
			y = control->YPos;

			/*
			**	If a formatting character was found, then process it at this
			**	time.
			*/
			switch (bchar) {
				/*
				**	Backspace moves the cursor back one space (erasing what was there).
				*/
				case '\b':
					if (x > 0) {
						x--;
						control->XPos = x;
						Mono_Print_Raw(control, &space, 1);
						control->XPos = x;
					}
					break;

				/*
				**	Formfeed -- clears the screen.
				*/
				case '\f':
					Mono_Clear_Screen(control);
					break;

				/*
				**	Linefeed -- in the tradition of C, this actually translates
				**	into a functional <CR><LF> pair.
				*/
				case '\n':
					x = 0;
					y++;
					if (y >= control->WinH) {
						Mono_Scroll(control);
						y--;
					}
					control->XPos = x;
					control->YPos = y;
					break;

				/*
				**	A <CR> will move the cursor down one row and back to the column
				**	position that the print started at.
				*/
				case '\r':
					x = original_x;
					y++;
					if (y >= control->WinH) {
						Mono_Scroll(control);
						y--;
					}
					control->XPos = x;
					control->YPos = y;
					break;

				/*
				**	Horizontal tab
				*/
				case '\t':
					Mono_Print_Raw(control, &space, 1);
					break;

				/*
				**	Unrecognized formatting character -- ignore it.
				*/
				default:
					break;
			}
		}
	}
}


/***********************************************************************************************
 * Mono_Scroll -- Scroll the mono screen up one line.                                          *
 *                                                                                             *
 *    Use this routine to scroll the mono screen up one line. The bottom line will be filled   *
 *    with blanks. The cursor position is moved up one line as well.                           *
 *                                                                                             *
 * INPUT:   control  -- Pointer to the buffer to scroll.                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Mono_Scroll(BuffControl * control)
{
	if (control != NULL)  {
		int j;
		unsigned short * vidmem = control->Buffer;
		unsigned short blank = BUILD_MONO_CHAR(' ', control->Attribute);
		unsigned short * lineptr = vidmem + (control->WinY*MONO_WIDTH) + control->WinX;

		/*
		**	Move every line of the sub-window.
		*/
		for (j = 0; j < control->WinH-1; j++) {
			RtlCopyMemory(lineptr, lineptr+MONO_WIDTH, control->WinW * sizeof(unsigned short));
			lineptr += MONO_WIDTH;
		}

		/*
		**	Fill the bottom row with blanks.
		*/
		for (j = 0; j < control->WinW; j++) {
			*lineptr++ = blank;
		}

		/*
		**	The cursor position moves with the scroll, but limited to the
		**	edge of the window.
		*/
		control->YPos -= 1;
		if (control->YPos < 0)  {
			control->YPos = 0;
		}
	}
}


/***********************************************************************************************
 * Mono_Pan -- Pan the mono screen over one column.                                            *
 *                                                                                             *
 *    This will scroll (horizontally) the monochrome screen. The new column will be filled     *
 *    with blank spaces.                                                                       *
 *                                                                                             *
 * INPUT:   control  -- Pointer to the buffer to pan.                                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Mono_Pan(BuffControl * control)
{
	if (control != NULL)  {
		unsigned short * vidmem = control->Buffer;
		unsigned short blank = BUILD_MONO_CHAR(' ', control->Attribute);
		int j;

		/*
		**	Move the video memory over by one column.
		*/
		for (j = 0; j < control->WinH; j++) {
			unsigned short * linestart = vidmem + ((control->WinY+j)*MONO_WIDTH) + control->WinX;
			RtlMoveMemory(linestart, linestart+1, (control->WinW-1) * sizeof(unsigned short));
			*(linestart+control->WinW-1) = blank;
		}

		/*
		**	The cursor position moves as well, but limited to the window
		**	margin.
		*/
		control->XPos -= 1;
		if (control->XPos < 0)  {
			control->XPos = 0;
		}
	}
}


/***********************************************************************************************
 * Mono_Print_Raw -- Print text (without processing) to mono screen.                           *
 *                                                                                             *
 *    This is a low level routine that will not process the characters submitted, but merely   *
 *    draw them to the monochrome screen. The output will not wrap at right margin nor will    *
 *    it scroll the screen when it tries to print past the bottom right character position.    *
 *    This routine can be used to output characters that have a visual image even though they  *
 *    line up with formatting control characters.                                              *
 *                                                                                             *
 * INPUT:   control  -- Pointer to the buffer to print to.                                     *
 *                                                                                             *
 *          string   -- Pointer to the string to output.                                       *
 *                                                                                             *
 *          length   -- The length of the string in characters.                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Mono_Print_Raw(BuffControl * control, unsigned char * string, unsigned int length)
{
	if (control != NULL)  {
		unsigned short * vidmem = control->Buffer + (control->XPos+control->WinX) + ((control->YPos+control->WinY)*MONO_WIDTH);
		int x = control->XPos;
		
		while (length > 0) {
			*vidmem++ = BUILD_MONO_CHAR(*string++, control->Attribute);
			x++;
			length--;

			/*
			**	Don't allow the cursor to extend past the right margin.
			*/
			if (x >= control->WinW) {
				x--;
				vidmem--;
			}
		}
		control->XPos = x;
	}
}


/***********************************************************************************************
 * Mono_Clear_Screen -- Clears the monochrome screen & homes cursor.                           *
 *                                                                                             *
 *    This will erase the monochrome screen and move the output cursor to the upper left       *
 *    corner.                                                                                  *
 *                                                                                             *
 * INPUT:   control  -- Pointer to the buffer to clear.                                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Mono_Clear_Screen(BuffControl * control)
{
	if (control != NULL)  {
		int yindex;
		unsigned short blank = BUILD_MONO_CHAR(' ', control->Attribute);

		unsigned short * rowptr = control->Buffer + (control->WinY * MONO_WIDTH) + control->WinX;
		for (yindex = 0; yindex < control->WinH; yindex++) {
			int xindex;
			unsigned short * lineptr = rowptr;
			for (xindex = 0; xindex < control->WinW; xindex++) {
				*lineptr++ = blank;
			}
			rowptr += MONO_WIDTH;
		}

		control->XPos = 0;
		control->YPos = 0;
	}
}


/***********************************************************************************************
 * Mono_Printf -- Print formatted text to the mono device.                                     *
 *                                                                                             *
 *    Prints formatted text to the monochrome device specified.                                *
 *                                                                                             *
 * INPUT:   control  -- Pointer to the buffer to print to.                                     *
 *                                                                                             *
 *          text     -- Pointer to the text to display.                                        *
 *                                                                                             *
 *          ...      -- Any optional parameters needed by the string.                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The final formatted string is limited to 1024 characters long.                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Mono_Printf(BuffControl * control, char const * text, ...)
{
	if (control != NULL)  {
		char buf[MONO_WIDTH*MONO_HEIGHT + 1];

		va_list ap;
		va_start(ap, text);
		vsprintf(buf, text, ap);
		Mono_Print(control, buf, strlen(buf));
		va_end(ap);
	}
}


/***********************************************************************************************
 * Mono_Set_View_Pos -- Set the mono display ram view offset.                                  *
 *                                                                                             *
 *    Set the CRTC register for the monochrome display to the offset specified. The card       *
 *    defaults to viewing offset 0 (i.e. $B0000 absolute RAM address).                         *
 *                                                                                             *
 * INPUT:   device   -- The monochrome device global data.                                     *
 *                                                                                             *
 *          pos      -- The offset position to set the view to.                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Mono_Set_View_Pos(MonoGlobals * device, int pos)
{
	if (device != NULL)  {
		WRITE_PORT_UCHAR(device->CRTCRegisters, 0x0C);
		WRITE_PORT_UCHAR(device->CRTCRegisters+1, (unsigned char)(pos >> 8));
		WRITE_PORT_UCHAR(device->CRTCRegisters, 0x0D);
		WRITE_PORT_UCHAR(device->CRTCRegisters+1, (unsigned char)(pos & 0xFF));
	}
}


/*********************************************************************************************** 
 * Mono_Bring_To_Top -- Brings the mono page to the visible top of the display stack.          * 
 *                                                                                             * 
 *    This routine will take the page specified and bring it to the top of the display stack.  * 
 *    Effectively, this makes it visible.                                                      * 
 *                                                                                             * 
 * INPUT:   device   -- Pointer to the mono device globals.                                    * 
 *                                                                                             * 
 *          page     -- Pointer to the page that is to be displayed.                           * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   If either the visible page or the one specified is locked, then no action       * 
 *             can be performed.                                                               * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void Mono_Bring_To_Top(MonoGlobals * device, BuffControl * page)
{
	BuffControl * visible_page = &device->Control[device->CurrentVisible];

	if (page == NULL || page->Buffer == visible_page->Buffer) return;

	/*
	**	If either the one trying to be brought to the front or the
	**	one already displayed is locked, then the display stack
	**	cannot be modified.
	*/
	if (page->LockCount > 0 || visible_page->LockCount > 0) return;

	/*
	**	Swap the contents of the page buffers and then swap the page buffer
	**	pointers.
	*/ 
	{
		unsigned short temp[MONO_WIDTH*MONO_HEIGHT];
		void * tempptr;
		int tempindex;
	
		RtlCopyMemory(temp, page->Buffer, MONO_PAGE_SIZE);
		RtlCopyMemory(page->Buffer, visible_page->Buffer, MONO_PAGE_SIZE);
		RtlCopyMemory(visible_page->Buffer, temp, MONO_PAGE_SIZE);

		tempptr = page->Buffer;
		page->Buffer = visible_page->Buffer;
		visible_page->Buffer = tempptr;
	}

	device->CurrentVisible = (page - &device->Control[0]);
	Mono_Update_Cursor(device);
}


/*********************************************************************************************** 
 * Display_Signon_Banner -- Display the signon banner at driver start.                         * 
 *                                                                                             * 
 *    This displays the signon banner that the driver displays when it first starts.           * 
 *                                                                                             * 
 * INPUT:   control  -- Pointer to the buffer that the signon banner will be displayed to.     * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void Display_Signon_Banner(BuffControl * control)
{
	Mono_Printf(control,
		"\fMono Display Driver [monodrvr.sys] -- NT Kernel Mode Device Driver\n"
		"Version: 1.0  -- (" __DATE__ "  " __TIME__ ")\n"
		"Device Name: " A_DEVICE_NAME "\n"
		"Symbolic Link: " A_SYMBOLIC_NAME " (\"\\\\.\\MONO\")\n\n");

	Mono_Printf(control, "Supporting IOCTL Services:\n");
	Mono_Printf(control, "--------------------------\n");
	Mono_Printf(control, " 0x%08X IOCTL_MONO_HELP_SCREEN\n", IOCTL_MONO_HELP_SCREEN);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_CLEAR_SCREEN\n", IOCTL_MONO_CLEAR_SCREEN);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_PRINT_RAW(char * text, int length)\n", IOCTL_MONO_PRINT_RAW);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_SET_CURSOR(int x, int y)\n", IOCTL_MONO_SET_CURSOR);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_SCROLL\n", IOCTL_MONO_SCROLL);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_BRING_TO_TOP\n", IOCTL_MONO_BRING_TO_TOP);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_SET_ATTRIBUTE(char attrib)\n", IOCTL_MONO_SET_ATTRIBUTE);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_PAN\n", IOCTL_MONO_PAN);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_LOCK [returns mono memory address]\n", IOCTL_MONO_LOCK);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_UNLOCK\n", IOCTL_MONO_UNLOCK);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_SET_WINDOW(int x, int y, int w, int h)\n", IOCTL_MONO_SET_WINDOW);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_RESET_WINDOW\n", IOCTL_MONO_RESET_WINDOW);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_SET_FLAG(int flag)\n", IOCTL_MONO_SET_FLAG);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_CLEAR_FLAG(int flag)\n", IOCTL_MONO_CLEAR_FLAG);
	Mono_Printf(control, " 0x%08X IOCTL_MONO_FILL_ATTRIB(int x, int y, int w, int h, int attrib)\n", IOCTL_MONO_FILL_ATTRIB);
}


/*********************************************************************************************** 
 * Mono_Fetch_Ptr -- Fetch a USER address space pointer to mono buffer.                        * 
 *                                                                                             * 
 *    This routine will create a user address space pointer (one that the ring 3 program can   * 
 *    use).                                                                                    * 
 *                                                                                             * 
 * INPUT:   device   -- Pointer to the mono device globals.                                    * 
 *                                                                                             * 
 *          page     -- Pointer to the page that a pointer will be generated for.              * 
 *                                                                                             * 
 * OUTPUT:  Returns with a user pointer to the monochrome card memory that is represented      * 
 *          by the page specified.                                                             * 
 *                                                                                             * 
 * WARNINGS:   This effectively locks the buffer so that it cannot be swapped.                 * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void * Mono_Fetch_Ptr(MonoGlobals * device, BuffControl * page)
{
	UNICODE_STRING     memory_unicode_string;
	OBJECT_ATTRIBUTES  object_attribute;
	HANDLE             section_handle  = NULL;
	NTSTATUS           error;
	void * retval = NULL;

	/*
	**	Try to open the PhysicalMemory section. This will be needed when mapping the
	**	physical memory 'file' to an accessable address range. First create a physical
	**	memory object and then convert that object into a handle value.
	*/
	RtlInitUnicodeString(&memory_unicode_string, L"\\Device\\PhysicalMemory");
	InitializeObjectAttributes(&object_attribute, &memory_unicode_string, OBJ_CASE_INSENSITIVE, (HANDLE)NULL, (PSECURITY_DESCRIPTOR)NULL);
	error = ZwOpenSection(&section_handle, SECTION_ALL_ACCESS, &object_attribute);
	if (NT_SUCCESS(error)) {
		PHYSICAL_ADDRESS logical_base_address;
		ULONG in_io_space = 0;

		//
		// Translate the physical addresses.
		//
		if (HalTranslateBusAddress(Isa, 0, RtlConvertUlongToLargeInteger(MONO_MEMORY + (page->Buffer - device->DisplayMemory) * sizeof(unsigned short)), &in_io_space, &logical_base_address)) {
			PVOID virtual_address = NULL;
			ULONG viewlength = MONO_MEM_LENGTH;
			PHYSICAL_ADDRESS view_base = logical_base_address;

			//
			// Map the section
			//
			error = ZwMapViewOfSection(section_handle, (HANDLE) -1, &virtual_address, 0L, viewlength, &view_base, &viewlength, ViewShare, 0, PAGE_READWRITE | PAGE_NOCACHE);
			if (NT_SUCCESS(error)) {

				//
				// Mapping the section above rounded the physical address down to the
				// nearest 64 K boundary. Now return a virtual address that sits where
				// we want by adding in the offset from the beginning of the section.
				//
				(ULONG) virtual_address += (ULONG)logical_base_address.LowPart - (ULONG)view_base.LowPart;
				retval = virtual_address;
			}
		}

		/*
		**	Close the zone handle before exiting.
		*/
		ZwClose(section_handle);
	}
	return(retval);
}


/*********************************************************************************************** 
 * Mono_Init_Buffer -- Initialize a mono buffer to known (unused) state.                       * 
 *                                                                                             * 
 *    This is equivalent to a constructor for the BuffControl object. It resets all values     * 
 *    that it can to a known state.                                                            * 
 *                                                                                             * 
 * INPUT:   buffer   -- Pointer to the buffer object to initialize.                            * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   The internal MGA RAM pointer is not reset by this routine. This value is        * 
 *             managed at the driver level, not the buffer object level.                       * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void Mono_Init_Buffer(BuffControl * buffer)
{
	buffer->XPos = 0;
	buffer->YPos = 0;
	buffer->Attribute = DEFAULT_ATTRIBUTE;
	buffer->LockPtr = NULL;
	buffer->LockCount = 0;
	buffer->Allocated = false;
	buffer->WinX = 0;
	buffer->WinY = 0;
	buffer->WinW = MONO_WIDTH;
	buffer->WinH = MONO_HEIGHT;
	buffer->Flags = 0;
}	


/*********************************************************************************************** 
 * Mono_Fill_Attribute -- Fills rectangle with attribute specified.                            * 
 *                                                                                             * 
 *    This routine will set the attribute for the region specified. It can be used to          * 
 *    hightlight or otherwise perform some cheesy effects without altering the text that       * 
 *    is already present.                                                                      * 
 *                                                                                             * 
 * INPUT:   x,y,w,h  -- Coordinates and dimensions of the rectangle to alter.                  * 
 *                                                                                             * 
 *          attrib   -- The attribute to set for the rectangle specified.                      * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   It is possible to set an attribute such that the text 'disappears'.             * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void Mono_Fill_Attribute(BuffControl * control, int x, int y, int w, int h, int attrib)
{
	if (control != NULL)  {
		int yindex;
		unsigned short blank = BUILD_MONO_CHAR(' ', control->Attribute);
		unsigned short * rowptr;

		/*
		**	Clip the parameters to legal values.
		*/
		if (x < 0) x = 0;
		if (x >= control->WinW) x = control->WinW-1;
		if (y < 0) y = 0;
		if (y >= control->WinH) y = control->WinH-1;
		if (w < 1) w = 1;
		if (w > control->WinW-x) w = control->WinW-x;
		if (h < 1) h = 1;
		if (h > control->WinH-y) h = control->WinH-y;

		rowptr = control->Buffer + ((control->WinY + y) * MONO_WIDTH) + control->WinX + x;
		for (yindex = 0; yindex < h; yindex++) {
			int xindex;
			unsigned short * lineptr = rowptr;
			for (xindex = 0; xindex < w; xindex++) {
				*lineptr = BUILD_MONO_CHAR(*lineptr, attrib);
				lineptr++;
			}
			rowptr += MONO_WIDTH;
		}
	}
}	
