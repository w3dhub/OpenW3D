#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: launcher.exe <exe_path> [args...]\n");
        return 1;
    }

    // Find explorer.exe process
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed: %lu\n", GetLastError());
        return 1;
    }

    PROCESSENTRY32 pe32 = { sizeof(pe32) };
    DWORD explorerPid = 0;
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, "explorer.exe") == 0) {
                explorerPid = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    if (explorerPid == 0) {
        printf("explorer.exe not found\n");
        return 1;
    }
    printf("Found explorer.exe PID: %lu\n", explorerPid);

    // Open explorer's process token
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, explorerPid);
    if (!hProcess) {
        printf("OpenProcess failed: %lu\n", GetLastError());
        return 1;
    }

    HANDLE hToken = NULL;
    if (!OpenProcessToken(hProcess, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &hToken)) {
        printf("OpenProcessToken failed: %lu\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }
    CloseHandle(hProcess);

    // Duplicate token for CreateProcessAsUser
    HANDLE hNewToken = NULL;
    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hNewToken)) {
        printf("DuplicateTokenEx failed: %lu\n", GetLastError());
        CloseHandle(hToken);
        return 1;
    }
    CloseHandle(hToken);

    // Set desktop info
    STARTUPINFO si = { sizeof(si) };
    si.lpDesktop = "winsta0\\default";
    PROCESS_INFORMATION pi = { 0 };

    // Build command line
    char cmdLine[1024] = { 0 };
    strncpy(cmdLine, argv[1], sizeof(cmdLine) - 1);
    for (int i = 2; i < argc && strlen(cmdLine) < sizeof(cmdLine) - 2; i++) {
        strncat(cmdLine, " ", sizeof(cmdLine) - strlen(cmdLine) - 1);
        strncat(cmdLine, argv[i], sizeof(cmdLine) - strlen(cmdLine) - 1);
    }

    printf("Launching: %s\n", cmdLine);

    if (!CreateProcessAsUserA(hNewToken, NULL, cmdLine, NULL, NULL, FALSE, 
        CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi)) {
        printf("CreateProcessAsUser failed: %lu\n", GetLastError());
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
    CloseHandle(hNewToken);
    return exitCode;
}
