#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wtsapi32.h>
#include <userenv.h>
#include <cstdio>

#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "userenv.lib")
#pragma comment(lib, "advapi32.lib")

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: launcher2.exe <exe_path> [args...]\n");
        return 1;
    }

    // Get the active console session ID
    DWORD sessionId = WTSGetActiveConsoleSessionId();
    if (sessionId == 0xFFFFFFFF) {
        printf("No active console session\n");
        return 1;
    }
    printf("Active session ID: %lu\n", sessionId);

    // Get the user token for the active session
    HANDLE hToken = NULL;
    if (!WTSQueryUserToken(sessionId, &hToken)) {
        printf("WTSQueryUserToken failed: %lu\n", GetLastError());
        return 1;
    }
    printf("WTSQueryUserToken succeeded\n");

    // Duplicate the token to create a primary token
    HANDLE hNewToken = NULL;
    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hNewToken)) {
        printf("DuplicateTokenEx failed: %lu\n", GetLastError());
        CloseHandle(hToken);
        return 1;
    }
    CloseHandle(hToken);

    // Create environment block for the user
    LPVOID pEnv = NULL;
    if (!CreateEnvironmentBlock(&pEnv, hNewToken, FALSE)) {
        printf("CreateEnvironmentBlock failed: %lu\n", GetLastError());
        // Continue anyway - environment is not strictly required
    }

    // Set up startup info
    STARTUPINFO si = { sizeof(si) };
    si.lpDesktop = "winsta0\\default";
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    PROCESS_INFORMATION pi = { 0 };

    // Build command line
    char cmdLine[1024] = { 0 };
    strncpy(cmdLine, argv[1], sizeof(cmdLine) - 1);
    for (int i = 2; i < argc && strlen(cmdLine) < sizeof(cmdLine) - 2; i++) {
        strncat(cmdLine, " ", sizeof(cmdLine) - strlen(cmdLine) - 1);
        strncat(cmdLine, argv[i], sizeof(cmdLine) - strlen(cmdLine) - 1);
    }

    printf("Launching: %s\n", cmdLine);
    printf("Session: %lu, Desktop: %s\n", sessionId, si.lpDesktop);

    DWORD flags = CREATE_UNICODE_ENVIRONMENT;
    if (pEnv) flags |= CREATE_NEW_CONSOLE;

    if (!CreateProcessAsUserA(hNewToken, NULL, cmdLine, NULL, NULL, FALSE, 
        flags, pEnv, NULL, &si, &pi)) {
        printf("CreateProcessAsUser failed: %lu\n", GetLastError());
        if (pEnv) DestroyEnvironmentBlock(pEnv);
        CloseHandle(hNewToken);
        return 1;
    }

    printf("Process launched with PID: %lu\n", pi.dwProcessId);
    CloseHandle(pi.hThread);
    
    // Wait for process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    printf("Process exited with code: %lu\n", exitCode);
    
    CloseHandle(pi.hProcess);
    if (pEnv) DestroyEnvironmentBlock(pEnv);
    CloseHandle(hNewToken);
    return exitCode;
}
