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

#pragma once

#include "always.h"

#include <atomic>

#if defined(OPENW3D_WIN32)
#include <windows.h>
#elif defined(OPENW3D_SDL3)
#include <SDL3/SDL_thread.h>
#endif


// ****************************************************************************
//
// To create a new thread just derive a new class from this and define
// Thread_Function. Creating the TheadClass object doesn't start the
// thread. To start the thread you must call Execute().
//
// In your own thread remember to check for "running" flag of the base class.
// If the flag is false you must exit the asap. Stop() is the function that
// will clear the flag and expect you to exit from the thread. If you are
// not exiting in certain time (defined as a parameter to Stop()) it will
// force-kill the thread to prevent the program from halting.
//
// ****************************************************************************

class ThreadClass
{
public:
	ThreadClass(const char *name = NULL);
	virtual ~ThreadClass();

	// Execute Thread_Function(). Note that only one instance can be executed at a time.
	void Execute();

	// Thread priority 0 is normal, positive numbers are higher and normal and negative are lower.
	void Set_Priority(int priority);

	// Signal thread to stop.
	void Stop();

	// Put current thread sleep for ms milliseconds (can be called from any thread, ThreadClass or other)
	static void Sleep_Ms(unsigned ms=0);

	// Put current thread in sleep and switch to next one (Useful for balansing the thread switches with game update)
	static void Switch_Thread();

	// Return calling thread's unique thread id
	static unsigned Get_Current_Thread_ID();

	// Returns true if the thread is running.
	bool Is_Running();

	// Gets the name of the thread.
	const char *Get_Name(void) { return(ThreadName); }

	// Get info about a registered thread by it's index.
	static int Get_Thread_By_Index(int index, char *name_ptr = NULL);

protected:

	// User defined thread function. The thread function should check for "running" flag every now and then
	// and exit the thread if running is false.
	virtual void Thread_Function() = 0;

	std::atomic<bool> mRunning;

	// Name of thread.
	char ThreadName[64];

	// ID of thread.
	unsigned mThreadID;

private:
#if defined(OPENW3D_WIN32)
	using ThreadHandle = HANDLE;
	using InternalThreadFunctionReturnType = DWORD;
#define INTERNAL_THREAD_FUNCTION_CALL_CONVENTION WINAPI
#elif defined(OPENW3D_SDL3)
	using ThreadHandle = SDL_Thread *;
	using InternalThreadFunctionReturnType = int;
#define INTERNAL_THREAD_FUNCTION_CALL_CONVENTION SDLCALL
#endif
	static InternalThreadFunctionReturnType INTERNAL_THREAD_FUNCTION_CALL_CONVENTION Internal_Thread_Function(void *param);
	ThreadHandle mHandle;
	int mThread_priority;
};
