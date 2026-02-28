#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

class SharedObject {
public:
	using FunctionPointer = void (*)();

	~SharedObject();

	static SharedObject *LoadObject(const char *path);

	FunctionPointer LoadFunction(const char *name) const;
protected:
#ifdef _WIN32
	using SoType = HMODULE;
#else
	using SoType = void *;
#endif
	SharedObject(SoType so) : m_so(so) {}
	SharedObject(SharedObject &) = delete;

	SoType m_so;
};
