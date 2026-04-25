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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/Except.h                               $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/07/02 12:28p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef EXCEPT_H
#define EXCEPT_H

#ifdef _WIN32

#include "win.h"
/*
** Forward Declarations
*/
typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS;
typedef struct _CONTEXT CONTEXT;

int Exception_Handler(int exception_code, EXCEPTION_POINTERS *e_info);
int Stack_Walk(void **return_addresses, int num_addresses, CONTEXT *context = NULL);
bool Lookup_Symbol(void *code_ptr, char *symbol, int &displacement);
void Load_Image_Helper(void);
void Register_Thread_ID(unsigned int thread_id, const char *thread_name, bool main = false);
void Unregister_Thread_ID(unsigned int thread_id, const char *thread_name);
void Register_Application_Exception_Callback(void (*app_callback)(void));
void Register_Application_Version_Callback(const char *(*app_version_callback)(void));
void Set_Exit_On_Exception(bool set);
bool Is_Trying_To_Exit(void);
unsigned int Get_Main_Thread_ID(void);
#if (0)
bool Register_Thread_Handle(unsigned int thread_id, HANDLE thread_handle);
int Get_Num_Thread(void);
HANDLE Get_Thread_Handle(int thread_index);
#endif //(0)

/*
** Register dump variables. These are used to allow the game to restart from an arbitrary
** position after an exception occurs.
*/
extern unsigned int ExceptionReturnStack;
extern unsigned int ExceptionReturnAddress;
extern unsigned int ExceptionReturnFrame;


typedef struct tThreadInfoType {
	char				ThreadName[128];
	unsigned int	ThreadID;
	HANDLE			ThreadHandle;
	bool				Main;
} ThreadInfoType;



#else // !_WIN32
// Linux stubs for exception handling and thread registration

#ifndef HANDLE
typedef void *HANDLE;
#endif
#ifndef UINT
typedef unsigned int UINT;
#endif
#ifndef DWORD
typedef unsigned long DWORD;
#endif

struct _EXCEPTION_POINTERS { void *context; };
struct _CONTEXT { unsigned long regs[32]; unsigned long cpsr; };
typedef _EXCEPTION_POINTERS EXCEPTION_POINTERS;
typedef _CONTEXT CONTEXT;

inline int Exception_Handler(int, EXCEPTION_POINTERS*) { return 0; }
inline int Stack_Walk(void**, int, CONTEXT*) { return 0; }
inline bool Lookup_Symbol(void*, char*, int&) { return false; }
inline void Load_Image_Helper() {}
inline void Register_Thread_ID(unsigned int, const char*, bool = false) {}
inline void Unregister_Thread_ID(unsigned int, const char*) {}
inline void Register_Application_Exception_Callback(void(*)(void)) {}
inline void Register_Application_Version_Callback(const char*(*)(void)) {}
inline void Set_Exit_On_Exception(bool) {}
inline bool Is_Trying_To_Exit() { return false; }
inline unsigned int Get_Main_Thread_ID() { return 1; }

struct tThreadInfoType {
    char ThreadName[128];
    unsigned int ThreadID;
    void *ThreadHandle;
    bool Main;
};

#endif // !_WIN32

#endif // EXCEPT_H
