#include "soutil.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

SharedObject *SharedObject::LoadObject(const char *path)
{
	SoType so;
#ifdef _WIN32
	so = LoadLibraryA(path);
#else
	so = dlopen(path, RTLD_NOW | RTLD_LOCAL);
#endif
	if (so == nullptr) {
		return nullptr;
	}
	auto result = new SharedObject(so);
	if (result == nullptr) {
#ifdef _WIN32
		FreeLibrary(so);
#else
		dlclose(so);
#endif
	}
	return result;
}

SharedObject::~SharedObject()
{
#ifdef _WIN32
		FreeLibrary(m_so);
#else
		dlclose(m_so);
#endif
}

SharedObject::FunctionPointer SharedObject::LoadFunction(const char *name) const
{
#ifdef _WIN32
	return reinterpret_cast<FunctionPointer>(GetProcAddress(m_so, name));
#else
	return reinterpret_cast<FunctionPointer>(dlsym(m_so, name));
#endif
}
