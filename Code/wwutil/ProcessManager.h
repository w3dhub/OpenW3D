#pragma once

#if defined(OPENW3D_WIN32)
#include <windows.h>
#elif defined(OPENW3D_SDL3)
#include <SDL3/SDL_process.h>
#endif

#include <optional>

class Process {
public:
	~Process();
	bool Wait(bool block=true);
	bool Kill();
	std::optional<int> Return_Code() const { return mReturnCode; }
	int Pid() const { return mPid; }

private:
#if defined(OPENW3D_WIN32)
	using HandleType = HANDLE;
#elif defined(OPENW3D_SDL3)
	using HandleType = SDL_Process *;
#endif

	Process(HandleType handle, int pid) : mHandle(handle), mPid(pid) {}
	Process(const Process &) = delete;

	friend class ProcessManager;
	HandleType mHandle;
	std::optional<int> mReturnCode;
	int mPid;
};

class ProcessManager {
	ProcessManager() = delete;
public:
	static Process *Create_Process(const char * const *args);
};
