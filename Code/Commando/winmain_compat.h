#pragma once

#if defined(_WIN32) && !defined(FREEDEDICATEDSERVER)
#include <windows.h>

#include <io.h>
#include <shellapi.h>
#include <stdio.h>

// main actually called something else when we wrap it to avoid linker issues.
#define main local_main

#ifdef _MSC_VER
#pragma comment(linker, "/ENTRY:WinMainCRTStartup")
#endif

// Forward declare main so WinMain can call it.
int main(int argc, char *argv[]);

#include "console/Utf8Arguments.h"

int __stdcall WinMain(HINSTANCE /* hInstance */, HINSTANCE /* hPrevInstance */, LPSTR /* lpCmdLine */, int /* nCmdShow */)
{
	// Get args as UTF-8
	OpenW3D::Utf8Arguments arguments;
	if (!arguments.Args) return 1;

	// Attach to the console we were run from if we were.
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		// We attached successfully, lets redirect IO to the consoles handles
		if (_fileno(stdout) == -2 || _get_osfhandle(fileno(stdout)) == -2) {
			freopen("CONOUT$", "w", stdout);
		}

		if (_fileno(stderr) == -2 || _get_osfhandle(fileno(stderr)) == -2) {
			freopen("CONOUT$", "w", stderr);
		}

		if (_fileno(stdin) == -2 || _get_osfhandle(fileno(stdin)) == -2) {
			freopen("CONIN$", "r", stdin);
		}
	}

	// Call the real main function.
	int ret = main(arguments.Count, arguments.Args);

	return ret;
}
#endif
