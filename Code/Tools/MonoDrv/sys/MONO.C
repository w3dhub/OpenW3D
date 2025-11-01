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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MONO.C                                                       *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 01/04/97                                                     *
 *                                                                                             *
 *                  Last Update : January 5, 1997 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Mono_Clear_Screen -- Clears the monochrome screen & homes cursor.                         *
 *   Mono_Print -- Prints text (with formatting) to mono screen.                               *
 *   Mono_Print_Raw -- Print text (without processing) to mono screen.                         *
 *   Mono_Printf -- Print formatted text to the mono device.                                   *
 *   Mono_Scroll -- Scroll the mono screen up one line.                                        *
 *   Mono_Set_View_Pos -- Set the mono display ram view offset.                                *
 *   Mono_Pan -- Pan the mono screen over one column.                                          *
 *   Mono_Detect_MGA_Adapter -- Try to detec the MGA adapter.                                  *
 *   Mono_Get_Address_Ptr -- Converts a physical address into a usable pointer.                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "ntddk.h"
#include "stdarg.h"
#include "stdio.h"
#include "monopub.h"


//
// Indices into & count of the above array
//

#define MONO_VIDEO_BUFFER  0
#define MONO_CRTC_REG      1
#define MONO_MODE_CTL_REG  2

#define MONO_NUMBER_RESOURCE_ENTRIES 2
//#define MONO_NUMBER_RESOURCE_ENTRIES 3


//
// Our user mode app will pass an initialized structure like this
//     down to the kernel mode driver
//

//typedef struct
//{
//    INTERFACE_TYPE   InterfaceType; // Isa, Eisa, etc....
//    ULONG            BusNumber;     // Bus number
//    PHYSICAL_ADDRESS BusAddress;    // Bus-relative address
//    ULONG            AddressSpace;  // 0 is memory, 1 is I/O
//    ULONG            Length;        // Length of section to map
//
//} PHYSICAL_MEMORY_INFO, *PPHYSICAL_MEMORY_INFO;



#define	DEVICE_NAME			L"\\Device\\Mono"
#define	A_DEVICE_NAME		"\\Device\\Mono"
#define	SYMBOLIC_NAME		L"\\DosDevices\\MONO"
#define	A_SYMBOLIC_NAME	"\\DosDevices\\MONO"


#define	CRTC_REGISTER		0x3B4
#define	MONO_MEMORY			0xB0000
#define	MONO_MEM_LENGTH	(0x10000-1)

#define	MAX_PAGES			16

#define	MONO_WIDTH	80
#define	MONO_HEIGHT	25

#define	SIZE_OF_SCREEN	(MONO_WIDTH * MONO_HEIGHT * sizeof(unsigned short))

#define	BUILD_CHAR(c, a)	((unsigned short)(((a) << 8) | ((c) & 0xFF)))

#define	PINDEX_TO_CELL(index)	((index) * MONO_WIDTH * MONO_HEIGHT)

/*
**	This controls the state information for each of the
**	display pages.
*/
typedef struct BuffControl
{
	int XPos;
	int YPos;

	/*
	**	This is the default attribute to use when displaying text.
	*/
	int Attribute;

	/*
	**	This is the logical address of the display page that this
	**	buffer represents. This pointer is in the address space
	**	of this driver, NOT of the owner of the file object.
	*/
	unsigned short * Buffer;

	/*
	**	This is a copy of a pointer to the page memory that is
	**	in mapped to the owning file object's address space.
	*/
	void * LockPtr;

	/*
	**	If this page has been allocated, then this flag will
	**	be true.
	*/
	int Allocated;
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
	BuffControl Control[MAX_PAGES];

	/*
	**	This is the index of the currently visible display page.
	*/
	int CurrentVisible;
} MonoGlobals;


//
// A structure decribing a particular device resource
//
typedef struct _MONO_RESOURCE
{
    PHYSICAL_ADDRESS  PhysicalAddress;
    unsigned long             Length;
    unsigned long             AddressSpace;
    unsigned long             RangeSharable;
}   MONO_RESOURCE, *PMONO_RESOURCE;



//
// A global that we keep around for use by the MonoDbgPrint function
// (when other drivers call MonoDbgPrint we need to be able to access
// the info stored in the device extension).
//

MonoGlobals * GlobalDeviceExtension = NULL;



//
// A variable which determines the verboseness of the messages printed by
// MonoDbgPrint.
//

unsigned long MonoDbgLevel = 3;



//
// Resources used by the monochrome video adapter
//

MONO_RESOURCE MonoResources[] =
{
    { MONO_MEMORY, 0x00000000,   // the video buffer
      MONO_MEM_LENGTH,               // length
      0,                        // in memory space
      1 },                      // shared with other drivers/devices (vga)

    { CRTC_REGISTER, 0x00000000,   // the 6845 index & data regs
      0x00000002,               // length
      1,                        // in I/O space
      1 },                      // shared with other drivers/devices (vga)

    { 0x000b03b8, 0x00000000,   // mode control register
      0x00000001,               // length
      1,                        // in I/O space
      1 }                       // shared with other drivers/devices (vga)
};



/*
**	Each cell is constructed of the actual character that is displayed and the
**	attribute to use. This character pair is located at every position on the
**	display (80 x 25). Since this cell pair can be represented by a "short"
**	integer, certain speed optimizations are taken in the monochrome drawing
**	code.
*/
typedef struct CellType {
	unsigned char Character;	// Character to display.
	unsigned char Attribute;	// Attribute.
} CellType;



NTSTATUS MonoDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);
void MonoUnload(PDRIVER_OBJECT DriverObject);
BOOLEAN pMonoReportResourceUsage(PDRIVER_OBJECT DriverObject, PMONO_RESOURCE MonoResources, unsigned long NumberOfResources);
//NTSTATUS MapMemMapTheMemory(PDEVICE_OBJECT DeviceObject,PVOID IoBuffer, ULONG OutputBufferLength);

void * Mono_Get_Address_Ptr(PHYSICAL_ADDRESS PhysicalAddress, unsigned long AddressSpace, unsigned long NumberOfBytes);
void Mono_Set_View_Pos(MonoGlobals * device, int pos);
NTSTATUS Mono_Detect_MGA_Adapter(void);
void * Mono_Fetch_Ptr(PDEVICE_OBJECT DeviceObject);
void Mono_Printf(BuffControl * control, char const * DbgMessage, ...);
void Mono_Clear_Screen(BuffControl * control);
void Mono_Print_Raw(BuffControl * control, unsigned char * string, unsigned long length);
void Mono_Scroll(BuffControl * control);
void Mono_Print(BuffControl * control, unsigned char * string, unsigned long length);
void Mono_Bring_To_Top(MonoGlobals * device, int index);
void Mono_Pan(BuffControl * control);
void Display_Signon_Banner(BuffControl * control);
void Mono_Update_Cursor(MonoGlobals * device);



/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path
                   to driver-specific key in the registry

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise

--*/
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	PDEVICE_OBJECT         deviceObject        = NULL;
	NTSTATUS               ntStatus;
	UNICODE_STRING         deviceNameUnicodeString;
	MonoGlobals * deviceExtension;
	BOOLEAN                symbolicLinkCreated = false;
	UNICODE_STRING         deviceLinkUnicodeString;
	int j;

	//
	// Check to see if anybody else has claim the resources we want to
	// used. We don't want to be partying on someone else's hardware,
	// even when trying to locate our devicc.
	//
	if (!pMonoReportResourceUsage(DriverObject, MonoResources, MONO_NUMBER_RESOURCE_ENTRIES)) {
		return STATUS_UNSUCCESSFUL;
	}

	//
	// Try to locate a monochrome display adapter (MDA)
	//
	ntStatus = Mono_Detect_MGA_Adapter();
	if (!NT_SUCCESS(ntStatus)) {
		ntStatus = STATUS_UNSUCCESSFUL;
		goto done_DriverEntry;
	}

	//
	// OK, we've claimed our resources & found our h/w, so create
	// a device and initialize stuff...
	//
	RtlInitUnicodeString(&deviceNameUnicodeString, DEVICE_NAME);
//	RtlInitUnicodeString(&deviceNameUnicodeString, L"\\Device\\Mono");

	//
	// Create an EXCLUSIVE device, i.e. only 1 thread at a time can send
	// i/o requests. If opened as non-exclusive, then we would need to
	// implement a more robust synchronization scheme than the event
	// mechanism we utilize below.
	//
	ntStatus = IoCreateDevice(DriverObject, sizeof(MonoGlobals), &deviceNameUnicodeString, FILE_DEVICE_MONO, 0, false, &deviceObject);

	if (NT_SUCCESS(ntStatus)) {

		deviceObject->Flags |= DO_BUFFERED_IO;

		GlobalDeviceExtension = deviceExtension = (MonoGlobals *) deviceObject->DeviceExtension;

		//
		// Initialize the dispatch event object. This allows us to
		// synchronize access to the h/w registers...
		//
		KeInitializeEvent(&deviceExtension->SyncEvent, SynchronizationEvent, true);

		//
		// Map all the required resources, save the addresses
		//
		deviceExtension->DisplayMemory =
				(PUSHORT) Mono_Get_Address_Ptr(MonoResources[MONO_VIDEO_BUFFER].PhysicalAddress,
				MonoResources[MONO_VIDEO_BUFFER].AddressSpace,
				MonoResources[MONO_VIDEO_BUFFER].Length
				);

		deviceExtension->CRTCRegisters =
				(unsigned char *) Mono_Get_Address_Ptr(MonoResources[MONO_CRTC_REG].PhysicalAddress,
				MonoResources[MONO_CRTC_REG].AddressSpace,
				MonoResources[MONO_CRTC_REG].Length
				);

		//        deviceExtension->ModeControlRegister =
		//            (unsigned char *) Mono_Get_Address_Ptr (MonoResources[MONO_MODE_CTL_REG].PhysicalAddress,
		//                                            MonoResources[MONO_MODE_CTL_REG].AddressSpace,
		//                                            MonoResources[MONO_MODE_CTL_REG].Length
		//                                            );

		if (deviceExtension->DisplayMemory == NULL || deviceExtension->CRTCRegisters == NULL)  {
			ntStatus = STATUS_UNSUCCESSFUL;
			goto done_DriverEntry;
		}

		//
		// Create a symbolic link that Win32 apps can specify to gain access
		// to this driver/device
		//
		RtlInitUnicodeString(&deviceLinkUnicodeString, SYMBOLIC_NAME);
//		RtlInitUnicodeString(&deviceLinkUnicodeString, L"\\DosDevices\\MONO");
		ntStatus = IoCreateSymbolicLink(&deviceLinkUnicodeString, &deviceNameUnicodeString);
		if (!NT_SUCCESS(ntStatus)) {
		} else {
			symbolicLinkCreated = true;
		}

		//
		// Create dispatch points for device control, create, close.
		//
		DriverObject->MajorFunction[IRP_MJ_CREATE] = MonoDispatch;
		DriverObject->MajorFunction[IRP_MJ_CLOSE] = MonoDispatch;
		DriverObject->MajorFunction[IRP_MJ_READ] = MonoDispatch;
		DriverObject->MajorFunction[IRP_MJ_WRITE] = MonoDispatch;
		DriverObject->MajorFunction[IRP_MJ_CLEANUP] = MonoDispatch;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MonoDispatch;
		DriverObject->DriverUnload = MonoUnload;

		//
		// Set the Start Address registers (indicate which part of
		// video buffer is displayed) to 0
		//
		Mono_Set_View_Pos(deviceExtension, 0);

		deviceExtension->CurrentVisible = 0;
		for (j = 0; j < MAX_PAGES; j++) {
			deviceExtension->Control[j].Buffer = &deviceExtension->DisplayMemory[PINDEX_TO_CELL(j)];
			deviceExtension->Control[j].XPos = 0;
			deviceExtension->Control[j].YPos = 0;
			deviceExtension->Control[j].Attribute = 0x07;
			deviceExtension->Control[j].LockPtr = NULL;
			deviceExtension->Control[j].Allocated = false;
		}

		/*
		**	Display the signon banner.
		*/
		Display_Signon_Banner(&deviceExtension->Control[deviceExtension->CurrentVisible]);
		Mono_Update_Cursor(deviceExtension);
	}


done_DriverEntry:

	if (!NT_SUCCESS(ntStatus)) {
		//
		// Something went wrong, so clean up
		//
		pMonoReportResourceUsage(DriverObject, NULL, 0);

		if (symbolicLinkCreated)  {
			IoDeleteSymbolicLink(&deviceLinkUnicodeString);
		}

		if (deviceObject)  {
			IoDeleteDevice(deviceObject);
		}

		GlobalDeviceExtension = NULL;
	}

	return ntStatus;
}



/*++

Routine Description:

    Process the IRPs sent to this device.

Arguments:

    DeviceObject - pointer to a device object

    Irp          - pointer to an I/O Request Packet

Return Value:


--*/
NTSTATUS MonoDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PIO_STACK_LOCATION irpStack;
	MonoGlobals *  deviceExtension;
	void * ioBuffer;
	unsigned long inputBufferLength;
	unsigned long outputBufferLength;
	unsigned long ioControlCode;
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
	deviceExtension = DeviceObject->DeviceExtension;

	/*
	**	Fetch the file object for this I/O request.
	*/
	fileobject = irpStack->FileObject;
	if (fileobject != NULL)  {
		currentindex = (long)fileobject->FsContext;
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
	KeWaitForSingleObject(&deviceExtension->SyncEvent, Executive, KernelMode, false, NULL);

	/*
	**	Get a working pointer to the display page.
	*/
	if (currentindex == -1) {
		currentindex == deviceExtension->CurrentVisible;
	}
	control = &deviceExtension->Control[currentindex];

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
				for (j = 0; j < MAX_PAGES; j++) {
					if (!deviceExtension->Control[j].Allocated) {
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
					BuffControl * newcon = &deviceExtension->Control[avail];
					newcon->XPos = 0;
					newcon->YPos = 0;
					newcon->Attribute = 0x07;
					newcon->Allocated = true;

					Mono_Bring_To_Top(deviceExtension, avail);
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
				deviceExtension->Control[currentindex].Allocated = false;
				fileobject->FsContext = NULL;
			}
			break;

		case IRP_MJ_CLEANUP:
			break;

		case IRP_MJ_DEVICE_CONTROL:
			ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

			switch (ioControlCode) {
				case IOCTL_MONO_BRING_TO_TOP:
					Mono_Bring_To_Top(deviceExtension, currentindex);
					break;

				case IOCTL_MONO_SCROLL:
					Mono_Scroll(control);
					break;

				case IOCTL_MONO_PAN:
					Mono_Pan(control);
					break;

				case IOCTL_MONO_SET_CURSOR:
					if (inputBufferLength == sizeof(int)*2) {
						control->XPos = *(int*)ioBuffer;
						control->YPos = *(((int*)ioBuffer) + 1);
					}
					break;

				case IOCTL_MONO_PRINT_RAW:
					Mono_Print_Raw(control, ioBuffer, inputBufferLength);
					break;

				case IOCTL_MONO_SET_POS:
					if (inputBufferLength == sizeof(int)) {
						Mono_Set_View_Pos(deviceExtension, *(int*)ioBuffer);
					}
					break;

				case IOCTL_MONO_CLEAR_SCREEN:
					Mono_Clear_Screen(control);
					break;

				case IOCTL_MONO_SET_ATTRIBUTE:
					control->Attribute = *(char *)ioBuffer;
					break;

#ifdef NEVER
				case IOCTL_MONO_LOCK:
					if (deviceExtension->LockPtr != NULL) {
						*(void**)ioBuffer = deviceExtension->LockPtr;
						deviceExtension->LockCount++;
					} else {
						*(void**)ioBuffer = Mono_Fetch_Ptr(DeviceObject);
						if (*(void**)ioBuffer != NULL) {
							deviceExtension->LockPtr = *(void**)ioBuffer;
							deviceExtension->LockCount++;
							Irp->IoStatus.Information = sizeof(PVOID);
						} else {
							Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
						}
					}
					break;

				case IOCTL_MONO_UNLOCK:
					if (deviceExtension->LockCount > 0) {
						deviceExtension->LockCount--;

						if (deviceExtension->LockCount == 0) {
							Irp->IoStatus.Status = ZwUnmapViewOfSection((HANDLE) -1, deviceExtension->LockPtr);
							deviceExtension->LockPtr = NULL;
						}
					}
					break;
#endif

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
	if (currentindex == deviceExtension->CurrentVisible)  {
		Mono_Update_Cursor(deviceExtension);
	}

	KeSetEvent(&deviceExtension->SyncEvent, 0, false);

	ntStatus = Irp->IoStatus.Status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return(ntStatus);
}



/*++

Routine Description:

    Free all the allocated resources, etc.

Arguments:

    DriverObject - pointer to a driver object

Return Value:


--*/
void MonoUnload(PDRIVER_OBJECT DriverObject)
{
//	wchar_t                  deviceLinkBuffer[]  = L"\\DosDevices\\MONO";
	UNICODE_STRING         deviceLinkUnicodeString;

	//
	// Unreport the resources
	//
	pMonoReportResourceUsage(DriverObject, NULL, 0);

	//
	// Delete the symbolic link
	//
	RtlInitUnicodeString(&deviceLinkUnicodeString, L"\\DosDevices\\MONO");
	IoDeleteSymbolicLink(&deviceLinkUnicodeString);

	//
	// Delete the device object
	//
	IoDeleteDevice(DriverObject->DeviceObject);
}


/***********************************************************************************************
 * Mono_Detect_MGA_Adapter -- Try to detec the MGA adapter.                                    *
 *                                                                                             *
 *    Tries to find the MGA adaptor by writing a few bytes to the CRTC register and checking   *
 *    if they return a corresponding value.                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the NT status code of the check. STATUS_SUCCESS if the MGA adapter    *
 *          was found.                                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/05/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
NTSTATUS Mono_Detect_MGA_Adapter(void)
{
	PHYSICAL_ADDRESS physicalAddress;
	unsigned char * crtcRegisters;

	/*
	**	Get address to the CRTC registers.
	*/
	physicalAddress.LowPart  = CRTC_REGISTER;
	physicalAddress.HighPart = 0;
	if ((crtcRegisters = Mono_Get_Address_Ptr(physicalAddress, 1, 2))) {

		/*
		**	Set the mono cursor position and see if it changed. If so, then
		**	presume that the MGA adapter is present.
		*/
		WRITE_PORT_UCHAR(crtcRegisters, 0x0F);
		WRITE_PORT_UCHAR(crtcRegisters+1, 0x55);
		if (READ_PORT_UCHAR(crtcRegisters+1) != 0x55) {
			return(STATUS_UNSUCCESSFUL);
		}
		WRITE_PORT_UCHAR(crtcRegisters+1, 0xAA);
		if (READ_PORT_UCHAR(crtcRegisters+1) != 0xAA) {
			return(STATUS_UNSUCCESSFUL);
		}

		WRITE_PORT_UCHAR(crtcRegisters+1, 0x00);

	} else {
		return(STATUS_UNSUCCESSFUL);
	}

	return(STATUS_SUCCESS);
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
void * Mono_Get_Address_Ptr(PHYSICAL_ADDRESS address, unsigned long space, unsigned long length)
{
	PHYSICAL_ADDRESS translatedAddress;
	void * usable_ptr = NULL;

	/*
	**	Translate a bus specific address into a logical system address.
	*/
	if (HalTranslateBusAddress(Isa, 0, address, &space, &translatedAddress)) {

		/*
		**	If the space value is zero, then this idicates that a call to
		**	MmMapIoSpace is required. This is usually required for port addresses.
		*/
		if (space == 0) {
			usable_ptr = MmMapIoSpace(translatedAddress, length, false);
		} else {
			usable_ptr = (void *)translatedAddress.LowPart;
		}
	}

	return(usable_ptr);
}



//
// NOTE: The following is commented out because some of the video miniport
//       drivers provided with the system (and in the NT DDK) regsister
//       their resources as exclusive;  if these resources overlap with
//       the resources used by the mono driver, then this driver will fail
//       to load.  This can be solved in one of two ways: by commenting
//       out the code below, and using the resources without reporting them
//       (not safe to do, but the easiest for this simple example); or, by
//       editing the source file(s) of the appropriate miniport driver
//       such that the VIDEO_ACCESS_RANGE.RangeSharable element is set to
//       true, and rebuilding the driver.
//
//       A real driver should *always* report it's resources.
//
/*++

Routine Description:

    Reports the resources used by a device.

Arguments:

    DriverObject      - pointer to a driver object

    MonoResources     - pointer to an array of resource information, or
                        NULL is unreporting resources for this driver

    NumberOfResources - number of entries in the resource array, or
                        0 if unreporting resources for this driver

Return Value:

    true if resources successfully report (and no conflicts),
    false otherwise.

--*/
BOOLEAN pMonoReportResourceUsage(PDRIVER_OBJECT DriverObject, PMONO_RESOURCE MonoResources, unsigned long NumberOfResources)
{
	unsigned long                           sizeOfResourceList = 0;
	PCM_RESOURCE_LIST               resourceList       = NULL;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
	unsigned long                           i;
	UNICODE_STRING                  className;
	BOOLEAN                         conflictDetected;


	if (NumberOfResources > 0) {
		//
		// Alloc enough memory to build a resource list & zero it out
		//
		sizeOfResourceList = sizeof(CM_RESOURCE_LIST) + (sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)*(NumberOfResources - 1));

		resourceList = ExAllocatePool(PagedPool, sizeOfResourceList);

		if (!resourceList) {
			return false;
		}

		RtlZeroMemory(resourceList, sizeOfResourceList);

		//
		// Fill in the reosurce list
		//
		// NOTE: Assume Isa, bus # 0
		//
		resourceList->Count = 1;

		resourceList->List[0].InterfaceType = Isa;
		resourceList->List[0].BusNumber     = 0;

		resourceList->List[0].PartialResourceList.Count = NumberOfResources;

		partial = &resourceList->List[0].PartialResourceList.PartialDescriptors[0];

		//
		// Account for the space used by the controller.
		//

		for (i = 0; i < NumberOfResources; i++) {
			if (MonoResources[i].AddressSpace) {
				partial->Type          = CmResourceTypePort;
				partial->Flags         = CM_RESOURCE_PORT_IO;
				partial->u.Port.Start  = MonoResources[i].PhysicalAddress;
				partial->u.Port.Length = MonoResources[i].Length;
			} else {
				partial->Type            = CmResourceTypeMemory;
				partial->Flags           = CM_RESOURCE_MEMORY_READ_WRITE;
				partial->u.Memory.Start  = MonoResources[i].PhysicalAddress;
				partial->u.Memory.Length = MonoResources[i].Length;
			}

			if (MonoResources[i].RangeSharable) {
				partial->ShareDisposition = CmResourceShareShared;
			} else {
				partial->ShareDisposition = CmResourceShareDeviceExclusive;
			}

			partial++;
		}
	}

	RtlInitUnicodeString(&className, L"LOADED MONO DRIVER RESOURCES");

	IoReportResourceUsage(&className, DriverObject, resourceList, sizeOfResourceList, NULL, NULL, 0, false, &conflictDetected);

	if (resourceList) {
		ExFreePool(resourceList);

		if (conflictDetected)  {
			return false;
		} else {
			return true;
		}
	}

	return true;
}


void Mono_Update_Cursor(MonoGlobals * device)
{
	int pos;

	pos = device->Control[device->CurrentVisible].XPos + (device->Control[device->CurrentVisible].YPos * MONO_WIDTH);

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
void Mono_Print(BuffControl * control, unsigned char * string, unsigned long length)
{
	if (control != NULL)  {
		int x,y;
		unsigned char space = ' ';

		x = control->XPos;
		y = control->YPos;

		while (length > 0) {
			unsigned char * blockstart;
			int blocklen;
			unsigned char bchar;		// Breaking character.

			/*
			**	Scan for a block of contiguous non-formatting characters to print.
			*/
			blockstart = string;
			blocklen = 0;
			bchar = '\0';
			while (length > 0 && bchar == '\0') {
				bchar = *string++;
				length--;

				switch (bchar) {
					case '\b':		// Backspace
					case '\a':		// Audible bell
					case '\t':		// Horizontal tab
					case '\f':		// Formfeed
					case '\n':		// Linefeed
					case '\v':		// Vertical tab
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
					if (y >= MONO_HEIGHT) {
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
				**	Unrecognized formatting character.
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
		unsigned short * vidmem;
		unsigned short blank = BUILD_CHAR(' ', control->Attribute);

		vidmem = control->Buffer;

		RtlMoveMemory(vidmem, vidmem+MONO_WIDTH, (MONO_HEIGHT-1) * MONO_WIDTH * sizeof(unsigned short));

		/*
		**	Fill the bottom row with blanks.
		*/
		for (j = 0; j < MONO_WIDTH; j++) {
			*(vidmem + j + ((MONO_HEIGHT-1) * MONO_WIDTH)) = blank;
		}

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
		int j;
		unsigned short * vidmem;
		unsigned short blank = BUILD_CHAR(' ', control->Attribute);

		vidmem = control->Buffer;

		for (j = 0; j < MONO_HEIGHT; j++) {
			RtlMoveMemory(vidmem+j*MONO_WIDTH, vidmem+j*MONO_WIDTH+1, (MONO_WIDTH-1) * sizeof(unsigned short));
			*(vidmem + j*MONO_WIDTH + MONO_WIDTH-1) = blank;
		}
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
void Mono_Print_Raw(BuffControl * control, unsigned char * string, unsigned long length)
{
	if (control != NULL)  {
		unsigned short * vidmem;
		int x,y;
		unsigned long i;

		vidmem = control->Buffer;
		x = control->XPos;
		y = control->YPos;

		for (i = 0; i < length; i++) {
			unsigned char letter;
			letter = *string++;

			*(vidmem + x + (y * MONO_WIDTH)) = BUILD_CHAR(letter, control->Attribute);
			x++;

			/*
			**	Don't allow the cursor to extend past the right margin.
			*/
			if (x >= MONO_WIDTH) {
				x = MONO_WIDTH-1;
			}
		}
		control->XPos = x;
		control->YPos = y;
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
		int i;
		unsigned short * vidptr = control->Buffer;
		unsigned short blank = BUILD_CHAR(' ', control->Attribute);

		for (i = 0; i < MONO_HEIGHT*MONO_WIDTH; i++) {
			*vidptr++ = blank;
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
		char buf[1024];
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


void Mono_Bring_To_Top(MonoGlobals * device, int index)
{
	if (index == -1 || index == device->CurrentVisible) return;

	Mono_Set_View_Pos(device, index * 100);
//	Mono_Set_View_Pos(device, index * MONO_WIDTH * MONO_HEIGHT);
	device->CurrentVisible = index;
	Mono_Update_Cursor(device);
}


void Display_Signon_Banner(BuffControl * control)
{
	Mono_Printf(control,
		"\fMono Display Driver [mono.sys] -- NT Kernel Mode Device Driver\n"
		"Version: 1.0  -- (" __DATE__ "  " __TIME__ ")\n"
		"Device Name: " A_DEVICE_NAME "\n"
		"Symbolic Link: " A_SYMBOLIC_NAME " (\"\\\\.\\MONO\")\n\n");

	Mono_Printf(control, "Supporting IOCTL Services:\n");
	Mono_Printf(control, "--------------------------\n");
	Mono_Printf(control, " [%08X] IOCTL_MONO_PRINT_RAW(char * text, int length)\n", IOCTL_MONO_PRINT_RAW);
	Mono_Printf(control, " [%08X] IOCTL_MONO_CLEAR_SCREEN(void)\n", IOCTL_MONO_CLEAR_SCREEN);
	Mono_Printf(control, " [%08X] IOCTL_MONO_SET_CURSOR(int x, int y)\n", IOCTL_MONO_SET_CURSOR);
	Mono_Printf(control, " [%08X] IOCTL_MONO_SCROLL(void)\n", IOCTL_MONO_SCROLL);
	Mono_Printf(control, " [%08X] IOCTL_MONO_BRING_TO_TOP(void)\n", IOCTL_MONO_BRING_TO_TOP);
	Mono_Printf(control, " [%08X] IOCTL_MONO_SET_ATTRIBUTE(char attrib)\n", IOCTL_MONO_SET_ATTRIBUTE);
	Mono_Printf(control, " [%08X] IOCTL_MONO_PAN(void)\n", IOCTL_MONO_PAN);
	Mono_Printf(control, " [%08X] IOCTL_MONO_LOCK(void) [returns mono memory address]\n", IOCTL_MONO_LOCK);
	Mono_Printf(control, " [%08X] IOCTL_MONO_UNLOCK(void)\n", IOCTL_MONO_UNLOCK);
}



/*++

Routine Description:

    Given a physical address, maps this address into a user mode process's
    address space

Arguments:

    DeviceObject       - pointer to a device object

    IoBuffer           - pointer to the I/O buffer

    InputBufferLength  - input buffer length

    OutputBufferLength - output buffer length

Return Value:

    STATUS_SUCCESS if sucessful, otherwise
    STATUS_UNSUCCESSFUL,
    STATUS_INSUFFICIENT_RESOURCES,
    (other STATUS_* as returned by kernel APIs)

--*/

void * Mono_Fetch_Ptr(PDEVICE_OBJECT DeviceObject)
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
		PHYSICAL_ADDRESS physical_address;
		ULONG in_io_space = 0;

		physical_address.HighPart = 0;
		physical_address.LowPart = MONO_MEMORY;

		//
		// Translate the physical addresses.
		//
		if (HalTranslateBusAddress(Isa, 0, physical_address, &in_io_space, &logical_base_address)) {
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

