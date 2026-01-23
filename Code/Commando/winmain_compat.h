#pragma once

#ifdef _WIN32
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

// Taken from https://github.com/thpatch/win32_utf8/blob/master/src/shell32_dll.c
// Get the command line as UTF-8 as it would be on other platforms.
static char **CommandLineToArgvU(LPCWSTR lpCmdLine, int *pNumArgs)
{
	int cmd_line_pos; // Array "index" of the actual command line string
	// int lpCmdLine_len = wcslen(lpCmdLine) + 1;
	int lpCmdLine_len = WideCharToMultiByte(CP_UTF8, 0, lpCmdLine, -1, NULL, 0, NULL, NULL) + 1;
	char **argv_u;

	wchar_t **argv_w = CommandLineToArgvW(lpCmdLine, pNumArgs);

	if (!argv_w) {
		return NULL;
	}

	cmd_line_pos = *pNumArgs + 1;

	// argv is indeed terminated with an additional sentinel NULL pointer.
	argv_u = (char **)LocalAlloc(LMEM_FIXED, cmd_line_pos * sizeof(char *) + lpCmdLine_len);

	if (argv_u) {
		int i;
		char *cur_arg_u = (char *)&argv_u[cmd_line_pos];

		for (i = 0; i < *pNumArgs; i++) {
			size_t cur_arg_u_len;
			int conv_len;
			argv_u[i] = cur_arg_u;
			conv_len = WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1, cur_arg_u, lpCmdLine_len, NULL, NULL);

			cur_arg_u_len = argv_w[i] != NULL ? conv_len : conv_len + 1;
			cur_arg_u += cur_arg_u_len;
			lpCmdLine_len -= static_cast<int>(cur_arg_u_len);
		}

		argv_u[i] = NULL;
	}

	LocalFree(argv_w);

	return argv_u;
}

int __stdcall WinMain(HINSTANCE /* hInstance */, HINSTANCE /* hPrevInstance */, LPSTR /* lpCmdLine */, int /* nCmdShow */)
{
	// Get args as UTF-8
	int argc;
	char **argv = CommandLineToArgvU(GetCommandLineW(), &argc);

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
	int ret = main(argc, argv);
	LocalFree(argv);

	return ret;
}
#endif
